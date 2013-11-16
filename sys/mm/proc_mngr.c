#include <sys/kmalloc.h>
#include <sys/proc_mngr.h>
#include <stdio.h>

// The process lists. The task at the head of the READY_LIST should always be executed next.
task_struct* READY_LIST = NULL;
task_struct* CURRENT_TASK = NULL;
task_struct* prev = NULL;
task_struct* next = NULL;

// Whether scheduling has been initiated
uint8_t IsInitScheduler;

void read_ip()
{
}

void add_to_ready_list(task_struct* new_task)
{
    task_struct* ready_list_ptr = READY_LIST;
    // kprintf("\nThe READY_LIST: %p", READY_LIST);
    if(ready_list_ptr == NULL){
        READY_LIST = new_task;
    } else {
        // kprintf("\nThe READY_LIST->next= %p", ready_list_ptr->next);
        while(ready_list_ptr->next != NULL) {
            ready_list_ptr = ready_list_ptr->next;
        }
        // kprintf("\nGoing in to add with ready_list_ptr=%p", ready_list_ptr);
        ready_list_ptr->next = new_task;
        new_task->next = NULL;
    }
    // kprintf("\nLeaving add_to_ready_list with READY_LIST=%p", READY_LIST);
}

#if PREMPTIVE_OS

// Push all Registers into Stack
#define PUSHA \
    __asm__ __volatile__(\
        "pushq %rax\n\t"\
        "pushq %rbx\n\t"\
        "pushq %rcx\n\t"\
        "pushq %rdx\n\t"\
        "pushq %rsp\n\t"\
        "pushq %rbp\n\t"\
        "pushq %rsi\n\t"\
        "pushq %rdi\n\t")

// Pop all Registers from Stack
#define POPA \
    __asm__ __volatile__(\
        "popq %rdi\n\t"\
        "popq %rsi\n\t"\
        "popq %rbp\n\t"\
        "popq %rsp\n\t"\
        "popq %rdx\n\t"\
        "popq %rcx\n\t"\
        "popq %rbx\n\t"\
        "popq %rax\n\t")

void timer_handler1()
{
    __asm__ __volatile__("cli");
    // kprintf("\tHello");
    __asm__ __volatile__("mov $0x20, %al\n\t"
            "out %al, $0x20\n\t"
            "sti\n\t"
            "iretq\n\t");
}

void timer_handler()
{
    uint64_t old_rsp;
//    __asm__ __volatile__("cli");
//    PUSHA;
    __asm__ __volatile__("movq %%rsp, %[old_rsp]": [old_rsp] "=r"(old_rsp));

    if (READY_LIST != NULL) {
        if (!IsInitScheduler) {
            kprintf("\nNot Initiated");
            /* The first time schedule gets invoked - additional processing needs done. */
            prev = READY_LIST;
            READY_LIST = READY_LIST->next;
            CURRENT_TASK = prev;
            IsInitScheduler = TRUE;
            // Need to switch the kernel stack to that of the first process
            // IRETQ pops in the order of rip, cs, rflags, rsp and ss
            __asm__ __volatile__(
                    "movq %[prev_rsp], %%rsp\n\t"
                    :
                    :[prev_rsp] "m" (prev->rsp_register));

        //    POPA;
            __asm__ __volatile__("mov $0x20, %al\n\t"
                                 "out %al, $0x20\n\t"
                                 "out %al, $0xA0\n\t"
        //                         "sti\n\t"
                                 "iretq\n\t");
        } else {
            kprintf("\nInitiated");
            while(1);
            prev = CURRENT_TASK;
            next = READY_LIST;
            __asm__ __volatile__(
                    "movq %[old_rsp], %[prev_rsp]\n\t"
                    "movq %[next_rsp], %%rsp\n\t"
                    :[prev_rsp] "=m" (prev->rsp_register)
                    :[old_rsp] "r" (old_rsp),
                    [next_rsp] "m" (next->rsp_register));

            CURRENT_TASK = READY_LIST;
            // Add prev to the end of the READY_LIST
            add_to_ready_list(prev);

            READY_LIST = READY_LIST->next;
        //    POPA;
            __asm__ __volatile__("mov $0x20, %al\n\t"
                                 "out %al, $0x20\n\t"
                                 "out %al, $0xA0\n\t"
        //                         "sti\n\t"
                                 "iretq\n\t");
        }
    } else {
        // Code should never reach here, as READY_LIST should never be null
        kprintf("\nReady List Empty");
    //    POPA;
        __asm__ __volatile__("mov $0x20, %al\n\t"
                           "out %al, $0x20\n\t"
                           "out %al, $0xA0\n\t"
    //                       "sti\n\t"
                           "iretq\n\t");
    }
}

#else

void schedule()
{
    if (IsInitScheduler == FALSE) {
        if (READY_LIST == NULL) {
            kprintf("\nReady List Empty");
            return;
        }
        prev = READY_LIST;
        READY_LIST = READY_LIST->next;
        CURRENT_TASK = prev;
    } else {
        prev = CURRENT_TASK;
        CURRENT_TASK = READY_LIST;
        next = READY_LIST;
        // Add prev to the end of the READY_LIST
        // kprintf("\nGoing in to add");
        add_to_ready_list(prev);
        // kprintf("\nThe task to be scheduled next is %p", READY_LIST);
        READY_LIST = READY_LIST->next;
        // kprintf("\nThe task to be scheduled after that is %p", READY_LIST);
        switch_to(prev, next);
    }
}

void init_schedule()
{
    IsInitScheduler = FALSE;
    schedule();

    // Need to switch the kernel stack to that of the first process
    __asm__ __volatile("movq %[prev_sp], %%rsp"::[prev_sp] "m" (prev->rsp_register));
    __asm__ __volatile("movq %[prev_bp], %%rbp"::[prev_bp] "m" (prev->rsp_register));

    IsInitScheduler = TRUE;
}

#endif

void create_new_process(task_struct* new_task, uint64_t func_addr)
{
    int stackind = KERNEL_STACK_SIZE-1;
#if PREMPTIVE_OS
    // Set up kernel stack => ss, rsp, rflags, cs, rip
    new_task->kernel_stack[127] = 0x10;
    new_task->kernel_stack[126] = (uint64_t)&new_task->kernel_stack[127];
    new_task->kernel_stack[125] = 0x200202;
    new_task->kernel_stack[124] = 0x08;
    new_task->kernel_stack[123] = func_addr;
    new_task->rsp_register = (uint64_t)&new_task->kernel_stack[123];
#else
    new_task->rsp_register = (uint64_t)&new_task->kernel_stack[stackind];
#endif

    new_task->rip_register = func_addr;
    new_task->next = NULL;
    new_task->last = NULL;

    // Add to the ready list 
    add_to_ready_list(new_task);
}

