#include <sys/mm_types.h>
#include <sys/types.h>

#define KERNEL_STACK_SIZE 128

typedef struct task_struct task_struct;

struct task_struct
{
    uint64_t proc_id;
    uint64_t kernel_stack[KERNEL_STACK_SIZE];
    uint64_t rip_register;
    uint64_t rsp_register;
    mm_struct* mm; 
    task_struct* next;      // The next process in the process list
    task_struct* last;      // The process that ran last
};

void* mmap(uint64_t virt_addr, int bytes);
task_struct* alloc_new_task();
void create_elf_proc(char *filename);
void schedule_process(task_struct* new_task, uint64_t func_addr);
void set_tss_rsp0(uint64_t);

#if PREMPTIVE_OS

#else

void init_schedule();
void schedule();
void read_ip();

// Switching the context between prev and next processes
#define switch_to(prev, next) \
    __asm__ __volatile__(\
        /* Push all Registers into Stack */\
        "pushq %%rax;"\
        "pushq %%rbx;"\
        "pushq %%rcx;"\
        "pushq %%rdx;"\
        "pushfq;"\
        "pushq %[my_last];"\
        "pushq %%rbp;"\
        \
        "movq %%rsp, %[prev_sp];"    /* Store current Stack Pointer */\
        "movq %[next_sp], %%rsp;"    /* Restore Stack Pointer of next process */\
        "movq $label, %[prev_ip];"   /* Assign IP for current process to label */\
        "pushq %[next_ip];"          /* Push IP of next process to be switched */\
        "jmp read_ip;"               /* Call read_ip function to read next_ip on stack */\
    "label:\t"\
        /* Pop all Registers from Stack */\
        "popq %%rbp;"\
        "popq %[new_last];"\
        "popfq;"\
        "popq %%rdx;"\
        "popq %%rcx;"\
        "popq %%rbx;"\
        "popq %%rax;"\
        :[prev_sp]  "=m" (prev->rsp_register),\
         [prev_ip]  "=m" (prev->rip_register),\
         [new_last] "=m" (prev)\
        :[next_sp]  "m"  (next->rsp_register),\
         [next_ip]  "m"  (next->rip_register),\
         [my_last]  "m"  (prev));\

#endif

