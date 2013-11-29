#include <defs.h>
#define MAXLEN 20
#define MAXCHILD 10

enum ftype {DIRECTORY, FILE};

typedef struct file fnode_t;
typedef struct file_dir DIR;
typedef struct dirent dirent;
typedef struct file_descp FD;


struct file {

    char f_name[MAXLEN];
    uint64_t start, end, curr;
    fnode_t* f_child[MAXCHILD];
    uint64_t curr_child;
    bool f_type;
};

struct dirent {
    
    uint64_t ino;
    char name[MAXLEN];
};

struct file_dir {

    fnode_t* filenode;
    uint64_t curr;
    dirent curr_dirent;

};

struct file_descp {
    fnode_t* filenode;
    uint64_t curr;
};

void* init_tarfs();
fnode_t* root_node;

DIR* opendir(char *dir_path);
struct dirent* readdir(DIR* node);
int closedir(DIR* node);
int open(char *path, int flags);
//fnode_t* opendir(char *dir);
//fnode_t* readdir(fnode_t* node);
//int closedir(fnode_t* node);
