#include <defs.h>
#include <stdio.h>
#include <sys/proc_mngr.h>
#include <sys/paging.h>
#include <sys/init_desc_table.h>
#include <sys/irq_common.h>
#include <sys/types.h>
#include <sys/kmalloc.h>
#include <stdlib.h>

extern int read_rip();

int sys_fork()
{

    // We are modifying kernel structures, and so cannot be interrupted.
    __asm__ __volatile__("cli");

    // Take a pointer to this process' task struct for later reference.
    task_struct *parent_task = CURRENT_TASK; 

    // Create a new process.
    task_struct* new_task = alloc_new_task(TRUE);

    // Set Copy on Write for all level page tables
    copy_vma(new_task, parent_task);

    // Clone the task struct of parent onto child process.
    //new_task->mm = parent_task->mm;
    //new_task->next = parent_task->next;
    //new_task->last = parent_task->last;
    
    uint64_t rsp, rip; 
    __asm__ __volatile__("mov %%rsp, %0" : "=r"(rsp));

    uint64_t rsp_ind = ((uint64_t)&parent_task->kernel_stack[KERNEL_STACK_SIZE-1] - rsp)/8;
    rsp_ind = KERNEL_STACK_SIZE - rsp_ind - 1;

    // Copy parent's kernel stack to child's kernel stack
    memcpy8(new_task->kernel_stack+rsp_ind, parent_task->kernel_stack+rsp_ind, KERNEL_STACK_SIZE-rsp_ind);

    new_task->parent = parent_task;
    
    rip = read_rip() + 17;
    // Add it to the end of the ready queue.
    schedule_process(new_task, rip, rsp_ind);
    
    // We could be the parent or the child here - check.
    if (CURRENT_TASK == parent_task)
    {
        // We are the parent, so set up the rsp/rbp/rip for our child.
        
        //new_task->rsp_register = parent_task->rsp_register;
        
        // All finished: Reenable interrupts.
        __asm__ __volatile__("sti");
        return new_task->pid;
    }
    else
    {
        // We are the child - by convention return 0.
        __asm__ __volatile__("sti");
        return 0;
    }
    //kprintf(" Fork called %d", current_task->pid);
    //return 1;
}

