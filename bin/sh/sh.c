#include <stdlib.h>
#include <defs.h>

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

int main(int argc, char **argv)
{
    //buffer is to hold the commands that the user will type in
    char /*str[25], *newstr,*/ ptr[20]; //= "Hello World Program\nls -l\nls -a\n\0";
    int i, j=0, k=0;//, count;
    char* path = "bin/";
    while(1)
    {
        char args[20][20];
        
        printf("\n"); 
        printf("<shell>");
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


        char prog[20];
        strcpy(prog, path);
        //printf("\nprog:%s", prog);
        strcat(prog, args[0]);
        //printf("\nfinal prog:%s", prog);
        //int l;
        //printf("\t%s\t%d", str, argsCount(str));
        
        //printf("\tnumber of args:%d \targs:", count);
        //for (l=1; l<= count; l++)
        //printf("\t%s", args[l]);
   
        int pid = fork();
        //Error checking to see if fork works
        //If pid !=0 then it's the parent
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

    /*
    newstr = ptr;
    //For parsing a script file and extracting the commands from the file
    while (*newstr != '\0')
    {
        char args[20][20];
        int count;
        newstr = getLine(newstr, str);
        j=0;
        k=0;
        count = argsCount(str);
        for (i = 0; i < ustrlen(str); i++)
        {
            if(str[i] == ' ') {
                args[j][k]= '\0';
                j++;
                k=0;
            } else 
                args[j][k++] = str[i];
            
            
        }
        *str = NULL;

        args[j][k]='\0';

        
        for(i = 0; i <= j; i++);
            //printf("\t%s",args[i]);


        char prog[20];
        strcpy(prog, path);
        printf("\nprog:%s", prog);
        strcat(prog, args[0]);
        printf("\nfinal prog:%s", prog);
        int l;
        printf("\t%s\t%d", str, argsCount(str));
        printf("\targs:");
        for (l=1; l< count; l++)
        printf("\t\t%s", args[l]);
        
    }
   */ 
    return 0;
}
