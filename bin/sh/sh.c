#include <stdlib.h>
#include <defs.h>
#include <sys/dirent.h>

char currdir[1024], args[20][20];
char temp[512];
DIR *curr_dir_ptr;
static char bg_flag, prog[20];
char *tempargs[10], *path = "/rootfs/bin/";

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

void *kmemset(void *ptr, uint8_t value, uint64_t num)
{
    uint8_t *temp = (uint8_t *)ptr;

    while(num--) {
        *temp++ = value; 
    }
    return ptr;
}


void export_to_path()
{
    char *path_str;

    if (args[1][0] == 'P' && args[1][1] == 'A' && args[1][2] == 'T' && args[1][3] == 'H') {

        if (args[1][ustrlen(args[1])-1] != '/') {
            args[1][ustrlen(args[1])] = '/';
        }
        
        path_str = args[1];
        path_str += 5;
        strcpy(path, path_str);
    }
    //printf("\n path: %s \t str: %s args: %s", path, path_str, args[1]);
}

void modify_string(char *currdir)
{

    int i = 0, j = 0, count = -1, currlen = ustrlen(currdir);
    int indx[50];

    for(i = 0; i < currlen;) {

        if(currdir[i] == '/')
            indx[++count] = j;

        if(i+1 < currlen && currdir[i] == '.' && currdir[i+1] != '.') {  //if path ends with dot dot

            i = i + 2;
        } else if(i+1 >= currlen && currdir[i+1] == '.') {                //if path ends with single dot

            break; 
        } else if(currdir[i] == '.' && currdir[i+1] == '.') {           //path has two consecutive dots in middle 

            j           = indx[count - 1] + 1;
            i           = i + 3;
            indx[count] = 0;
            count       = count - 1;
            continue;
        }

        temp[j++] = currdir[i++];             
    }

    //if second last character is '/'
    if(temp[j-1] == '/')
        temp[j-1] = '\0';
    else 
        temp[j] = '\0';

    strcpy(currdir, temp);
}


void fork_and_execvpe()
{
    
    int pid = fork();

    if (pid!=0) {
        if (bg_flag != '&')
            waitpid(pid, NULL, 0);
    } else {
        execvpe(prog, tempargs, NULL);
        exit(1);
    }
}

void *umemset(void *ptr, uint8_t value, uint64_t num)
{
    uint8_t *temp = (uint8_t *)ptr;

    while(num--) {
        *temp++ = value; 
    }
    return ptr;
}

int main(int argc, char **argv)
{
    char str[25], *newstr, ptr[20], path_to_cmd[20];
    int i, j=0, k=0, file_descp, ptr_length, lendir = 0, str_length;
    char* exec_path;
    //path = "bin/";

    printf("\n\t\t\t\t*******NEW SHELL*******");
    //By default current directory stream will be pointing to DIR stream of '/'
    curr_dir_ptr = opendir("/");
    while(1)
    {
        j = 0, k = 0;
        umemset(args, 0, 400);

        printf("\n"); 
        printf("[user@SBUnix ~%s]$", currdir);

        scanf("%s", ptr);
        ptr_length = ustrlen(ptr);

        if (ptr_length == 0)
            continue;

        /*****1) To check if process is to be run in background ****/
        bg_flag = ptr[ptr_length - 1];
        if (bg_flag == '&')
            ptr[ptr_length - 1] = '\0';


        //collect the arguments entered by user in 2D array args
        for (i = 0; i < ptr_length; i++) {
            if(ptr[i] == ' ') {
                args[j][k]= '\0';
                j++;
                k=0;
            } else 
                args[j][k++] = ptr[i];
        }

        args[j][k]='\0';

        
        
        if(strcmp(args[0], "export") == 0) {
            /****1) export path****/   
               //printf("b4:%s",args[1]);
               export_to_path();
        
        } else if (strcmp(args[0], "help") == 0) {             
            /****2) To handle help command ****/
            printf("\nps\ncls\nls\ncd\nexport PATH");

        } else if (strcmp(args[0], "pwd") == 0) {
            /****3) To handle PWD command ****/
            printf("\n%s", currdir); 

        } else if (strcmp(args[0], "cd") == 0) {
            /****4) To handle CD command ****/
            lendir  = ustrlen(currdir);

            //check if args[1] is a absolute path 
            if(args[1][0] == '/') {

                curr_dir_ptr = opendir(args[1]);

                if (curr_dir_ptr == NULL) {

                    printf("\n %s: No such file or directory", args[1]); 
                    currdir[lendir] = '\0';
                    curr_dir_ptr    = opendir(currdir); 

                } else {

                    //If it is absoulte correct path, modify it to remove dots
                    strcpy(currdir, args[1]); 
                    modify_string(currdir);
                }

            } else {
                //If it is a relative path
                //concatenate to existing dir

                strcat(currdir, "/");
                strcat(currdir, args[1]);

                curr_dir_ptr = opendir(currdir); 

                if (curr_dir_ptr == NULL) {

                    printf("\n %s: No such file or directory", currdir); 
                    currdir[lendir] = '\0';
                    curr_dir_ptr    = opendir(currdir); 

                } else {

                    //If it is relative correct path, modify it to remove dots
                    modify_string(currdir);
                }
            } 

        } else if (strcmp(args[0], "ls") == 0) {
            /****5) To handle LS command ****/

            // To list contents of directory specified by user
            if(args[1][0] != '\0') {

                //if argument to ls is relative path, append it to currdir
                if(args[1][0] != '/') {

                    int lendir  = ustrlen(currdir);

                    strcat(currdir, "/");
                    strcat(currdir, args[1]);
                    strcpy(args[1], currdir);
                    //restore orignal currdir 
                    currdir[lendir] = '\0';

                }                

                //If argument is absolute path, pass it directly to execvpe
                tempargs[0] = (char *)malloc(ustrlen(args[1]) + 1); 
                strcpy(tempargs[0], args[1]);

            } else {            
                //No argument by user
                //List contents of current directory

                tempargs[0] = (char *)malloc(ustrlen(currdir) + 1); 
                strcpy(tempargs[0], currdir);
                //printf("\n tempargs: %s, currir %s", tempargs[0], currdir);
            }

            tempargs[1] = NULL;
            strcpy(prog, "/rootfs/bin/ls");

            fork_and_execvpe();


        } else if (ptr[0] == 's' && ptr[1] == 'h' && ptr[2] == ' ' && ptr_length > 3)  {              /****6) To check for executable ****/

            // Extracting the scan from shell into a 2d array: row 0 = command, other rows = arguments to the cmd
            exec_path = ptr;

            //ignore the 'sh' part. 
            exec_path +=3;
            file_descp  = open(exec_path, 0);


            if (file_descp != -1) {
                //check if path is valid path

                read(file_descp, ptr, 100); 

                if (ptr[0] == '#' && ptr[1] == '~') {
                    newstr = ptr;
                    newstr += 2;
                    //For parsing a script file and extracting the commands from the file

                    while (*newstr != '\0')
                    {
                        newstr = getLine(newstr, str);
                        str_length = ustrlen(newstr); 
                        bg_flag = str[str_length - 1];

                        if (bg_flag == '&')
                            str[str_length - 1] = '\0';

                        j = 0, k = 0;

                        for (i = 0; i < str_length ; i++) {
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

                        //strcpy(path_to_cmd, "/rootfs/");
                        strcat(path_to_cmd, prog);

                        file_descp = open(path_to_cmd, 0);
                        close(file_descp);
                        
                        if (file_descp != -1) {
                            fork_and_execvpe();
                        } else { 
                            printf("\t CMD does not exist");
                        }
                    }

                } else {
                    printf("\nFile not an executable");
                }

            } else {
                printf("\nFile does not exist");
            }

        } else {
            /****7) Run a binary ****/

            strcpy(prog, path);
            strcat(prog, args[0]);
            //strcpy(path_to_cmd, "/rootfs/");
            strcat(path_to_cmd, prog);

            file_descp = open(path_to_cmd, 0);
            close(file_descp);
            if (file_descp != -1) {
                fork_and_execvpe();
            } else { 
                printf("\tCMD does not exist");
            }
        }
        *ptr = NULL;
    } 

    //exit(1);
    return 0;
}
