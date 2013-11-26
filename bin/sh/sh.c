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
    int i, j=0, k=0, count;
    // /bin/program_name is the arguments to pass to execv
    //if we want to run ls, "/bin/ls" is required to be passed to execv()
    char* path = "/bin/";
    while(1)
    {
        char args[20][20];
        
        printf("\n"); 
        printf("<shell>");
        scanf("%s", ptr);
        //newstr = ptr;
        j=0;
        k=0;
        count = argsCount(ptr);
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
        printf("\nprog:%s", prog);
        strcat(prog, args[0]);
        printf("\nfinal prog:%s", prog);
        int l;
        //printf("\t%s\t%d", str, argsCount(str));
        
        printf("\tnumber of args:%d \targs:", count);
        for (l=1; l<= count; l++)
        printf("\t%s", args[l]);
        
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
    /*
    while(1)
    {
    //print the prompt
        printf("<shell>");
        //get input
        read(stdin, buffer, 512);
        //fork!
        //int pid = fork();
        //Error checking to see if fork works
        //If pid !=0 then it's the parent
        if(pid!=0)
        {
            wait(NULL);
        }
        else
        {
            //if pid = 0 then we're at teh child
            //Count the number of arguments
            int num_of_args = argCount(buffer);
            //create an array of pointers for the arguments to be passed to execcv.
            char *arguments[num_of_args+1];
            //parse the input and arguments will have all the arguments to be passed to the program
            parseArgs(buffer, num_of_args, arguments);
            //set the last pointer in the array to NULL. Requirement of execv
            arguments[num_of_args] = NULL;
            //This will be the final path to the program that we will pass to execv
            char prog[512];
            //First we copy a /bin/ to prog
            strcpy(prog, path);
            //Then we concancate the program name to /bin/
            //If the program name is ls, then it'll be /bin/ls
            strcat(prog, arguments[0]);
            //pass the prepared arguments to execv and we're done!
            int rv = execv(prog, arguments);
        }
    }
    */
    while(1);
    return 0;
}
