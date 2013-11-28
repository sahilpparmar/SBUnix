#include <stdlib.h>
#include <defs.h>

static char bg, prog[20];

int ustrlen(const char *str)
{
    int len=0;
    while (*str++ != '\0')
        len += 1;
    return len;
}

char* strcat(char *str1, const char *str2)
{
    uint64_t len1 = ustrlen(str1);
    uint64_t len2 = ustrlen(str2);
    uint64_t i = 0;
    
    for(i = 0; i < len2 ; i++)
        str1[len1 + i] = str2[i];
    str1[len1 + i] = '\0';

    return str1;    
}

char *strcpy(char *dest, const char *src)
{
    char *str = dest;
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
    return str;
}

char *getLine(char *ptr, char *str)
{
    while (*ptr != '\n')
        *str++ = *ptr++;
    *str = '\0';
    
    return ++ptr;
}

int argsCount(char *str)
{
    int num=0;

    while (*str != '\0')
    {
        if( *str == ' ')
            num++;

        str++;
    }
    return num;
}

void fork_and_execvpe()
{

    int pid = fork();

    if(pid!=0) {
        if (bg != '&')
            wait(NULL);
    } else {
        execvpe(prog, NULL, NULL);
        exit(1);
    }

}

int main(int argc, char **argv)
{
    //buffer is to hold the commands that the user will type in
    char str[25], *newstr, ptr[20], args[20][20];
    int i, j=0, k=0, fd;
    char* path = "bin/";

    while(1)
    {

        printf("\n"); 
        printf("<shell>");
        scanf("%s", ptr);
        
        bg = ptr[ustrlen(ptr)-1];
        if (bg == '&')
            ptr[ustrlen(ptr)-1] = '\0';

        if (ptr[0] != '.') {
            j=0;
            k=0;
            


            // Extracting the scan from shell into a 2d array: row 0 = command, other rows = arguments to the cmd
            for (i = 0; i < ustrlen(ptr); i++)
            {
                if(ptr[i] == ' ') {
                    args[j][k]= '\0';
                    j++;
                    k=0;
                } else 
                    args[j][k++] = ptr[i];
            }

            *ptr = NULL;
            args[j][k]='\0';
            //char prog[20];
            strcpy(prog, path);
            strcat(prog, args[0]);
            
            fork_and_execvpe();

        } else {

            char *tmp = ptr;

            tmp +=1;
            fd = open(tmp, 0);

            if(fd != -1) {
                read(fd, ptr, 100); 
                if (ptr[0] == '#' && ptr[1] == '~') {
                    newstr = ptr;
                    newstr += 2;
                    //For parsing a script file and extracting the commands from the file
                    while (*newstr != '\0')
                    {
                        newstr = getLine(newstr, str);

                        if (str[ustrlen(str)-1] == '&')
                            str[ustrlen(str)-1] = '\0';

                        j=0;
                        k=0;

                        for (i = 0; i < ustrlen(str); i++) {
                            if(str[i] == ' ') {
                                args[j][k]= '\0';
                                j++;
                                k=0;
                            } else 
                                args[j][k++] = str[i];
                        }
                        *str = NULL;

                        args[j][k]='\0';

                        strcpy(prog, path);
                        strcat(prog, args[0]);

                        fork_and_execvpe();
                    }
                } else {
                    printf("\nNX");
                }
            } else {
                printf("File does not exist");
            }
        }
    }

    return 0;
}
