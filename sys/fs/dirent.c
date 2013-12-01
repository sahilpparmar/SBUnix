#include <sys/dirent.h>
#include <sys/tarfs.h>
#include <defs.h>
#include <sys/virt_mm.h>
#include <sys/kstring.h>
#include <sys/kmalloc.h>
#include <stdio.h>

fnode_t* root_node;

void* file_lookup(char *dir_path)
{

    char* file_path = (char *)dir_path;

    fnode_t *aux_node, *currnode = root_node;

    char *temp = NULL; 
    int i;
    char *path = (char *)kmalloc(sizeof(char) * kstrlen(file_path));
    kstrcpy(path, file_path); 

    temp = kstrtok(path, "/");  
    
    if (temp == NULL)
        return NULL;
    
    //kprintf("\n step1 %s", temp);

    while (temp != NULL) {
        
        aux_node = currnode;
        for (i = 2; i < currnode->end; ++i) {
            if (kstrcmp(temp, currnode->f_child[i]->f_name) == 0) {
                currnode = (fnode_t *)currnode->f_child[i];
                break;       
            }        
        }

        if (i == aux_node->end) {
            return NULL;
        }

        temp = kstrtok(NULL, "/");          
    }

    if (currnode->f_type == FILE)
        return (void *)currnode->start; 
    else
        return NULL;
}

void make_node(struct file *node, struct file *parent, char *name, uint64_t start, uint64_t end, int type)
{
    kstrcpy(node->f_name, name);
    node->start = start;
    node->end   = end;
    node->curr  = start;
    node->f_type  = type;
    
    node->f_child[0] = node;
    node->f_child[1] = parent;    
    
    //return (void *)node;
}

void parse(char *dir_path, int type, uint64_t start, uint64_t end)
{
    fnode_t *temp_node, *aux_node, *currnode = root_node->f_child[2];
    char *temp; 
    int i = 0;

    char *path = (char *)kmalloc(sizeof(char) * kstrlen(dir_path));
    kstrcpy(path, dir_path); 

    temp = kstrtok(path, "/");  

    while (temp != NULL) {
        aux_node = currnode; 
        //kprintf("%s \n", temp);
        //iterate through all childrens of currnode        
        for(i = 2; i < currnode->end; ++i){
            if(kstrcmp(temp, currnode->f_child[i]->f_name) == 0) {
                currnode = (fnode_t *)currnode->f_child[i];
                break;       
            }        
        }

        //kprintf("\n....%s...%s...", currnode->f_name, temp);
        //if no child has been found
        //add this as child of current
        if (i == aux_node->end) {

            temp_node = (fnode_t *)kmalloc(sizeof(struct file));
            make_node(temp_node, currnode, temp, start, end, type);  

            currnode->f_child[currnode->end] = temp_node;
            currnode->end += 1; 
        } 

        //kprintf("....%d...%s...", currnode->end, temp);
        //while(1); 

        temp = kstrtok(NULL, "/");          

    }
}

void printnode(fnode_t *node)
{
    int i = 0; 
    //kprintf("\n%s\t%p", node->f_name, &node);

    for (i = 2; i < node->end; ++i) {
        if (node->f_child[i]->f_type == FILE) {
            kprintf("\nfile %s \t %p \t %p", node->f_child[i]->f_name, node->f_child[i]->start, node->f_child[i]->end);
        } else { 
            printnode(node->f_child[i]); 
        }
    }
}


void* init_tarfs()
{
    HEADER *header = (HEADER*) &_binary_tarfs_start;
    int size_of_file = 0;
    fnode_t *temp_node;

    root_node = (fnode_t *)kmalloc(sizeof(struct file));
    make_node(root_node, root_node, "/", 0, 2, DIRECTORY);  

    temp_node = (fnode_t *)kmalloc(sizeof(struct file)); 
    make_node(temp_node, root_node, "rootfs", 0, 2, DIRECTORY);
    root_node->f_child[2] = temp_node; 
    root_node->end += 1;

    do {
        size_of_file = oct_to_dec(atoi(header->size));

        if (kstrcmp(header->typeflag, "5") == 0) {
            parse(header->name, DIRECTORY, 0, 2);
        } else {
            parse(header->name, FILE, (uint64_t)(header + 1), (uint64_t)((void *)header + 512 + size_of_file));
        }

        if (size_of_file > 0) {
            header = header + size_of_file / (sizeof(HEADER) + 1) + 2;
        } else {
            header = header + 1;
        }
    } while(header < (HEADER*)&_binary_tarfs_end);

    return (void *)&root_node; 
}

