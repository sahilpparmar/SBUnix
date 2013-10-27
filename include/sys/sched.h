#include <sys/mm_types.h>
#include <sys/types.h>

#define PROCESS_STATE_SLEEP  0
#define PROCESS_STATE_ACTIVE 1

typedef struct _regs_t {
    uint64_t esp;
    uint64_t ebp;
    uint64_t eip;
    uint64_t edi;
    uint64_t esi;
    uint64_t eax;
    uint64_t ebx;
    uint64_t ecx;
    uint64_t edx;
    uint64_t flags;
} regs_t;

typedef struct proc_struct_t proc_struct;

struct proc_struct_t {
    uint64_t kernel_stack[512];
    regs_t regs;    
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

