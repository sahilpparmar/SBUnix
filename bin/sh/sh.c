#include <stdlib.h>
#include <defs.h>
#include <sys/dirent.h>

char currdir[1024], args[10][100];
char temp[512];
DIR *curr_dir_ptr;
static char bg_flag, prog[100];
char *execargs[10], path[100] = "/rootfs/bin/";

static char *getLine(char *ptr, char *str, int limit)
{
    while (limit && *ptr != '\n') {
        *str++ = *ptr++;
        limit--;
    }
    *str = '\0';

    return ++ptr;
}

static bool Is_file_exist()
{
    int fd  = open(prog, 0);
    if (fd == -1)
        return FALSE;
    close(fd);
    return TRUE;
}

static void copy_args_to_execargs()
{
    int i = 0;
    for (i = 0; args[i+1][0] != '\0'; i++)
        execargs[i] = (char*)args[i+1];
    execargs[i] = NULL;
}

static void export_to_path()
{
    char *path_str;

    if (args[1][0] == 'P' && args[1][1] == 'A' && args[1][2] == 'T' && args[1][3] == 'H') {

        if (args[1][strlen(args[1])-1] != '/') {
            args[1][strlen(args[1])] = '/';
            args[1][strlen(args[1])+1] = '\0';

        }
        path_str = args[1];
        path_str += 5;
        strcpy(path, path_str);
    }
}

static void modify_string(char *currdir)
{
    int i = 0, j = 0, count = -1, currlen = strlen(currdir);
    int indx[50];

    for(i = 0; i < currlen;) {

        if(currdir[i] == '/')
            indx[++count] = j;

        if(i+1 < currlen && currdir[i] == '.' && currdir[i+1] != '.') {     
            // if path ends with dot dot
            i = i + 2;
        } else if(i+1 >= currlen && currdir[i+1] == '.') {                  
            // if path ends with single dot
            break; 
        } else if(currdir[i] == '.' && currdir[i+1] == '.') {               
            // path has two consecutive dots in middle 
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

static void fork_and_execvpe()
{
    int pid = fork();

    if (pid != 0) {
        if (bg_flag != '&') {
            waitpid(pid, NULL, 0);
        } else {
            printf("Added [%d] to BG", pid);
        }

    } else {
        execvpe(prog, execargs, NULL);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    char str[1024], *newstr, ptr[1024], path_cmd[1024];
    int i, j=0, k=0, file_descp, ptr_length, lendir = 0, str_length;
    char* exec_path;

    printf("\n\t\t\t\t*******NEW SHELL*******");
    //By default current directory stream will be pointing to DIR stream of '/'
    curr_dir_ptr = opendir("/");
    while(1) {
        j = 0, k = 0;

        printf("\n[user@SBUnix ~%s]$", currdir);

        scanf("%s", ptr);
        ptr_length = strlen(ptr);

        memset(args, 0, sizeof(args));
        execargs[0] = NULL;

        if (ptr_length == 0) {
            continue;
        } else if (strcmp(ptr, "echo $PATH")==0) {
            printf("PATH: %s", path);
            continue;
        }
        /*****1) To check if process is to be run in background ****/
        bg_flag = ptr[ptr_length - 1];
        if (bg_flag == '&') {
            ptr[ptr_length - 1] = '\0';
            ptr_length -= 1;
            if (ptr_length == 0)
                continue;
        }
        //collect the arguments entered by user in 2D array args
        for (i = 0; i < ptr_length; i++) {
            if(ptr[i] == ' ') {
                args[j][k]= '\0';
                j++;
                k=0;
            } else {
                args[j][k++] = ptr[i];
            }
        }
        args[j][k]='\0';

        char help_str[2048], help_ptr[2048];
        memset(help_ptr, 0, 2048);
        memset(help_str, 0, 2048);

        if (strcmp(args[0], "ulimit") == 0) {

            char *new_pointer; 
            file_descp  = open("/rootfs/etc/ulimit", 0);
            if (file_descp != -1) {
                read(file_descp, help_ptr, 2048); 
                close(file_descp);
                new_pointer = help_ptr;
                cls();
                while (*new_pointer != '\0') {

                    new_pointer = getLine(new_pointer, help_str, 1023);
                    printf("\n%s", help_str);
                }

                printf("\n");
            }

        } else if (strcmp(args[0], "help") == 0) {

            char *new_pointer; 
            file_descp  = open("/rootfs/etc/help", 0);
            if (file_descp != -1) {
                read(file_descp, help_ptr, 2048); 
                close(file_descp);
                new_pointer = help_ptr;
                cls();
                while (*new_pointer != '\0') {

                    new_pointer = getLine(new_pointer, help_str, 2047);
                    printf("\n%s", help_str);
                }

                printf("\n");
            }
        } else if (strcmp(args[0], "cat") == 0) {

            char *new_pointer; 
            file_descp  = open(args[1], 0);
            if (file_descp != -1) {
                read(file_descp, help_ptr, 2048); 
                close(file_descp);
                new_pointer = help_ptr;
                while (*new_pointer != '\0') {

                    new_pointer = getLine(new_pointer, help_str, 2047);
                    printf("\n%s", help_str);
                }

                printf("\n");
            } else { 
                printf("\nInvalid Path : Please enter absolute path of the file.\nRefer to help.");
            }

        } else if (strcmp(args[0], "shutdown") == 0) {
            shutdown();
        } else if (strcmp(args[0], "cls") == 0) {
            cls();
        } else if(strcmp(args[0], "export") == 0) {
            /****1) export path****/   
            export_to_path();

        } else if (strcmp(args[0], "pwd") == 0) {
            /****3) To handle PWD command ****/
            if (strlen(currdir) == 0)
                printf("/");
            else
                printf("%s", currdir);
        } else if (strcmp(args[0], "cd") == 0) {
            /****4) To handle CD command ****/
            lendir  = strlen(currdir);

            //check if args[1] is a absolute path 
            if (args[1][0] == '/') {

                curr_dir_ptr = opendir(args[1]);

                if (curr_dir_ptr == NULL) {

                    printf("%s: No such directory.\nRefer to help.", args[1]); 
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

                    printf("%s: No such directory.\nRefer to help.", currdir); 
                    currdir[lendir] = '\0';
                    curr_dir_ptr    = opendir(currdir); 

                } else {

                    //If it is relative correct path, modify it to remove dots
                    modify_string(currdir);
                }
            } 

        } else if (strcmp(args[0], "/rootfs/bin/ls") == 0 || strcmp(args[0], "ls") == 0) {
            /****5) To handle LS command ****/

            // To list contents of directory specified by user
            if (args[1][0] != '\0') {

                //if argument to ls is relative path, append it to currdir
                if (args[1][0] != '/') {

                    int lendir  = strlen(currdir);

                    strcat(currdir, "/");
                    strcat(currdir, args[1]);
                    strcpy(args[1], currdir);
                    //restore orignal currdir 
                    currdir[lendir] = '\0';

                }                
                execargs[0] = (char *)args[1]; 
            } else {            
                //No argument by user
                //List contents of current directory
                execargs[0] = (char *)currdir; 
            }

            execargs[1] = NULL;
            strcpy(prog, path);
            strcat(prog, "ls");

            if (Is_file_exist()) {
                fork_and_execvpe();
            } else {
                printf("CMD does not exist.\nRefer to help.");
            }

        } else if (ptr[0] == 's' && ptr[1] == 'h' && ptr[2] == ' ' && ptr_length > 3)  {              /****6) To check for executable ****/

            // Extracting the scan from shell into a 2d array: row 0 = command, other rows = arguments to the cmd
            exec_path = ptr;

            //ignore the 'sh' part. 
            exec_path +=3;
            strcpy(path_cmd, exec_path);
            //strcat(path_cmd, "\0");
            file_descp  = open(path_cmd, 0);
            if (file_descp != -1) {
                //check if path is valid path

                read(file_descp, ptr, 1024); 
                close(file_descp);
                if (ptr[0] == '#' && ptr[1] == '!') {
                    newstr = ptr;
                    newstr = getLine(newstr, str, 1023);
                    // For parsing a script file and extracting the commands from the file
                    newstr = getLine(newstr, str, 1023);
                    while (*newstr != '\0') {
                        str_length = strlen(str); 
                        bg_flag = str[str_length - 1];

                        if (bg_flag == '&')
                            str[str_length - 1] = '\0';

                        j = 0, k = 0;

                        memset(args, 0, sizeof(args));
                        execargs[0] = NULL;

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

                        if (Is_file_exist()) {
                            copy_args_to_execargs();
                            fork_and_execvpe();
                            yield();
                        } else { 
                            printf("CMD does not exist.\nRefer to help.");
                        }
                        newstr = getLine(newstr, str, 1023);
                    }

                } else {
                    printf("File not an executable.\nRefer to help.");
                }

            } else {
                printf("File does not exist.\nRefer to help.");
            }

        } else {
            /****7) Run a binary ****/
            char *cmd = NULL;
            cmd = args[0];

            if (args[0][0] == '.' && args[0][1] == '/')
                cmd += 2;

            if (args[0][0] == '/' && strlen(args[0]) > 1) {
                strcpy(prog, args[0]);
            } else { 
                strcpy(prog, path);
                strcat(prog, cmd);
            }
            if (Is_file_exist() || strcmp(prog, "ls") == 0) {
                copy_args_to_execargs();
                fork_and_execvpe();
            } else { 
                printf("CMD does not exist.\nRefer to help.");
            }
        }
        *ptr = NULL;
    } 

    return 0;
}

