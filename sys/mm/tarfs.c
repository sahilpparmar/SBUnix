#include <sys/tarfs.h>
#include <defs.h>
#include <string.h>
#include <stdio.h>


void lookup(char *file_name) {


    HEADER *header = (HEADER*) &_binary_tarfs_start;
    int size_of_file = 0; 
    int i;
    char *p;
    
    do {
        /*convert header->size string to octal to integer*/
        size_of_file = octal_decimal(atoi(header->size));
        p = (char*)(header + 1);
        
        if(strcmp(file_name, header->name) == 0)
        {   
            printf("...File Name: %s..File Size: %d bytes..Contents of file: ", header->name, size_of_file);

            for (i = 0; i < size_of_file-1; i++) {
            putchar(*p++);
            } 
        }
        
        if(size_of_file > 0) 
            header = header + 1 + (size_of_file / 513 + 1);
        else
            header = header + 1;
    } while(header < (HEADER*)&_binary_tarfs_end);
    
}

