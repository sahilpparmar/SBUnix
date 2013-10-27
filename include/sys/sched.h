#include <sys/mm_types.h>
#include <sys/types.h>

typedef struct proc_struct_t proc_struct;

struct proc_struct_t {
    uint64_t kernel_stack[512];
    uint64_t flags;    
    
    //struct mm_struct mm;
    uint64_t pml4_t;

    proc_struct *parent;
    proc_struct *children;
    proc_struct *next;

    int prio;
    int state;
    pid_t pid;
};

