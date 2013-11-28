#include <stdlib.h>
#include <defs.h>
#include <sys/dirent.h>

char currdir[1024];
char temp[512];
DIR *tp;
static char bg, prog[20];

int strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2++)
        if (*s1++ == 0)
            return (0);

    return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}


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

void modify_string(char *currdir)
{
    int i=0,j=0,k=0;

    for(i = 0 ; currdir[i] != '\0' ; )
    {

        if (currdir[i] != '.')
        {
            temp[j++] = currdir[i++];
        }
        else if(currdir[i] == '.' && currdir[i+1] == '.')
        {
            i+=3;
            j-=2;
            for(k = j; temp[k] != '/'; k--);
            j=k+1;
        }
    }
    temp[j]='\0';

    strcpy(currdir, temp);
}

int main(int argc, char **argv)
{
    //buffer is to hold the commands that the user will type in
    char str[25], *newstr, ptr[20], args[20][20];
    int i, j=0, k=0, fd;
    char* path = "bin/";

    tp = opendir("/");
    strcpy(currdir, "/"); 

    while(1)
    {
        printf("\n"); 
        printf("[user@SBUnix ~%s]$", currdir);
        scanf("%s", ptr);

        bg = ptr[ustrlen(ptr)-1];
        if (bg == '&')
            ptr[ustrlen(ptr)-1] = '\0';

        j=0;
        k=0;

        for (i = 0; i < ustrlen(ptr); i++) {
            if(ptr[i] == ' ') {
                args[j][k]= '\0';
                j++;
                k=0;
            } else 
                args[j][k++] = ptr[i];
        }

        args[j][k]='\0';

        if (strcmp(args[0], "pwd") == 0) {
            
            printf("\n%s", currdir); 

        } else if (strcmp(args[0], "cd") == 0) {

            int lendir  = ustrlen(currdir);

                strcat(currdir, "/");
                strcat(currdir, args[1]);

                tp = opendir(currdir); 

                if(tp == NULL) {
                    printf("\n Invalid path entered"); 
                    currdir[lendir] = '\0';
                }//function to strip and clean the dir if it has .. 
                modify_string(currdir);
                
        } else if (strcmp(args[0], "ls") == 0) {
            //TODO: convert this ls into binary 
            printf("\n");
            struct dirent* temp; 
            while((temp = readdir(tp)) != NULL) {
                
                printf("\t%s", temp->name);
            }

        } else if (ptr[0] == 's' && ptr[1] == 'h' && ptr[2] == ' ')  {
            
            // Extracting the scan from shell into a 2d array: row 0 = command, other rows = arguments to the cmd
            char *tmp = ptr;

            tmp +=3;
            fd = open(tmp, 0);

            if (fd != -1) {
                read(fd, ptr, 100); 
                
                if (ptr[0] == '#' && ptr[1] == '~') {
                    newstr = ptr;
                    newstr += 2;
                    //For parsing a script file and extracting the commands from the file
                    
                    while (*newstr != '\0')
                    {
                        newstr = getLine(newstr, str);

                        bg = str[ustrlen(str)-1];
                        if (bg == '&')
                            str[ustrlen(str)-1] = '\0';

                        j=0;
                        k=0;

                        for (i = 0; i < ustrlen(str); i++) {
                            if (str[i] == ' ') {
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
        
        } else {
            
            strcpy(prog, path);
            strcat(prog, args[0]);
            fork_and_execvpe();
        
        }
        *ptr = NULL;
    } 


    return 0;
}
