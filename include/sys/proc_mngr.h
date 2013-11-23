#ifndef _PROC_MNGR_H
#define _PROC_MNGR_H

#include <sys/types.h>

#define KERNEL_STACK_SIZE 128
#define DEBUG_SCHEDULING 0

typedef struct vm_area_struct vma_struct;
typedef struct mm_struct mm_struct;
typedef struct task_struct task_struct;

struct vm_area_struct {
    mm_struct *vm_mm;               // The address space we belong to.
    uint64_t vm_start;              // Our start address within vm_mm
    uint64_t vm_end;                // The first byte after our end address within vm_mm
    vma_struct *vm_next;            // linked list of VM areas per task, sorted by address
    uint64_t vm_flags;              // Flags, see mm.h
};

struct mm_struct {
    vma_struct *vma_list;           // list of VMAs
    uint64_t pml4_t;                // Actual physical base addr for PML4 table
    uint32_t vma_count;             // number of VMAs
    uint64_t hiwater_vm;            // High-water virtual memory usage
    uint64_t total_vm;
    uint64_t stack_vm;
    uint64_t start_code, end_code, start_data, end_data;
    uint64_t start_brk, end_brk, start_stack;
    uint64_t arg_start, arg_end, env_start, env_end;
};

struct task_struct
{
    pid_t pid;
    pid_t ppid;
    bool IsUserProcess;
    uint64_t kernel_stack[KERNEL_STACK_SIZE];
    uint64_t rip_register;
    uint64_t rsp_register;
    mm_struct* mm; 
    task_struct* next;      // The next process in the process list
    task_struct* last;      // The process that ran last
    task_struct* parent;    // Keep track of parent process on fork
    task_struct* children;  // Keep track of its children on fork
    task_struct* sibling;   // Keep track of its siblings (children of same parent)
};

extern task_struct* CURRENT_TASK;

void* mmap(uint64_t virt_addr, int bytes);
task_struct* alloc_new_task(bool IsUserProcess);
vma_struct* alloc_new_vma(uint64_t start_addr, uint64_t end_addr);
pid_t create_elf_proc(char *filename);
void schedule_process(task_struct* new_task, uint64_t entry_point, uint64_t stack_top);
void set_tss_rsp0(uint64_t rsp);
void add_to_ready_list(task_struct* new_task);
void increment_brk(task_struct *proc, uint64_t bytes);
pid_t sys_getpid();
pid_t sys_getppid();

#endif
