#include <sys/mm_types.h>
#include <sys/types.h>

#define KERNEL_STACK_SIZE 128

#define cli __asm__ __volatile__("cli");
#define sti __asm__ __volatile__("sti");

typedef struct task_struct task_struct;

struct task_struct
{
    char identifier;
    uint64_t proc_id;
    uint64_t pml4_entry_base;
    uint64_t kernel_stack[KERNEL_STACK_SIZE];
    uint64_t rip_register;
    uint64_t rsp_register;
    mm_struct* mm; 
    task_struct* next;       // The next process in the process list
    task_struct* last;   // The process that ran last
};

void create_new_process(task_struct* new_task, uint64_t func_addr);

#if PREMPTIVE_OS

#else

void init_schedule();
void schedule();
void read_ip();

// Switching the context between prev and next processes
#define switch_to(prev, next) \
    __asm__ __volatile__(\
        /* Push all Registers into Stack */\
        "pushq %%rax\n\t"\
        "pushq %%rbx\n\t"\
        "pushq %%rcx\n\t"\
        "pushq %%rdx\n\t"\
        "pushfq\n\t"\
        "pushq %[my_last]\n\t"\
        \
        "pushq %%rbp\n\t"\
        "movq %%rsp, %[prev_sp]\n\t"    /* Store current Stack Pointer */\
        "movq %[next_sp], %%rsp\n\t"    /* Restore Stack Pointer of next process */\
        "movq $label, %[prev_ip]\n\t"   /* Assign IP for current process to label */\
        "pushq %[next_ip]\n\t"          /* Push IP of next process to be switched */\
        "jmp read_ip\n\t"               /* Call read_ip function to read next_ip on stack */\
    "label:\t"\
        /* Pop all Registers from Stack */\
        "popq %%rbp\n\t"\
        "popq %[new_last]\n\t"\
        "popfq\n\t"\
        "popq %%rdx\n\t"\
        "popq %%rcx\n\t"\
        "popq %%rbx\n\t"\
        "popq %%rax\n\t"\
        :[prev_sp]  "=m" (prev->rsp_register),\
         [prev_ip]  "=m" (prev->rip_register),\
         [new_last] "=m" (prev)\
        :[next_sp]  "m"  (next->rsp_register),\
         [next_ip]  "m"  (next->rip_register),\
         [my_last]  "m"  (prev));\

#endif

