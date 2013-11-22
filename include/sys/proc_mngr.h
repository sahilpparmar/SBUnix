#include <sys/mm_types.h>
#include <sys/types.h>

#define KERNEL_STACK_SIZE 512

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
uint64_t get_brk_top(task_struct *proc);
void increment_brk(task_struct *proc, uint64_t addr);
