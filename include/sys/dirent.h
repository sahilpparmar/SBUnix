#include <defs.h>

#define MAXLEN 20
#define MAXCHILD 10

enum ftype {DIRECTORY, FILE};
enum perm {O_RDONLY, O_WRONLY, O_RDWR, O_APPEND, O_CREAT, O_TRUNC};
enum seekop {SEEK_SET, SEEK_CUR, SEEK_END};

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
    uint64_t f_inode_no;
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
    uint64_t f_perm;
    uint64_t inode_struct;
};


fnode_t* root_node;

void printnode(fnode_t *node);
DIR* opendir(char *dir_path);
struct dirent* readdir(DIR* node);
int closedir(DIR* node);
int open(char *path, int flags);
void* file_lookup(char *dir_path);
void make_node(struct file *node, struct file *parent, char *name, uint64_t start, uint64_t end, int type, uint64_t f_inode_no);
fnode_t* get_root_node();
