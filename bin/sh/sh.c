#include <stdlib.h>
#include <defs.h>
#include <sys/dirent.h>

char currdir[1024];
char temp[512];
DIR *tp;

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
            for(k = j; temp[k] != '/'; k--)
            {

            }
            j=k+1;
        }
    }
    temp[j]='\0';
    
    strcpy(currdir, temp);
}


int main(int argc, char **argv)
{
    //buffer is to hold the commands that the user will type in
    char /*str[25], *newstr,*/ ptr[20]; //= "Hello World Program\nls -l\nls -a\n\0";
    int i, j=0, k=0;//, count;
    // /bin/program_name is the arguments to pass to execv
    //if we want to run ls, "/bin/ls" is required to be passed to execv()
    tp = opendir("/");
    strcpy(currdir, "/"); 
    //volatile int exit_now = 0;
    char* path = "bin/";
    
    while(1)
    {
        char args[20][20];

        printf("\n"); 
        printf("[user@SBUnix ~%s]$", currdir);
        scanf("%s", ptr);
        //newstr = ptr;
        j=0;
        k=0;
        //count = argsCount(ptr); 


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


        //for(i = 0; i <= j; i++);
        //printf("\t%s",args[i]);
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
            }
            
            //function to strip and clean the dir if it has .. 
            modify_string(currdir);

        } else if (strcmp(args[0], "ls") == 0) {
            //TODO: convert this ls into binary 
             
            printf("\n");
            struct dirent* temp; 
            while((temp = readdir(tp)) != NULL) {
                printf("\t%s", temp->name);
            }

        } else {

            char prog[20];
            strcpy(prog, path);
            //printf("\nprog:%s", prog);
            strcat(prog, args[0]);
            int pid = fork();
            
            if(pid!=0)
            {
                wait(NULL);
            }
            else
            {
                execvpe(prog, NULL, NULL);
                exit(1);
            }

        } 
    }

    return 0;
}
