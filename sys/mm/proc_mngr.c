#include <sys/kmalloc.h>
#include <sys/proc_mngr.h>
#include <stdio.h>
#include <screen.h>
#include <io_common.h>
#include <sys/paging.h>
#include <sys/types.h>

#define PAGING_PRESENT_WRITABLE PAGING_PRESENT | PAGING_WRITABLE | PAGING_USER
// The process lists. The task at the head of the READY_LIST should always be executed next.
task_struct* READY_LIST = NULL;
//task_struct* CURRENT_TASK = NULL;
task_struct* prev = NULL;
task_struct* next = NULL;

// Whether scheduling has been initiated
uint8_t IsInitScheduler;

void add_to_ready_list(task_struct* new_task)
{
    task_struct* ready_list_ptr = READY_LIST;
    // kprintf("\nThe READY_LIST: %p", READY_LIST);
    if (ready_list_ptr == NULL) {
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

static uint32_t sec, min, hr, tick;

void init_timer(uint32_t freq)
{
    uint32_t divisor = 1193180 / freq;
    uint8_t lower, upper;

    // Send the command byte.
    outb(0x43, 0x36);

    // Split into upper/lower bytes
    lower = (uint8_t)(divisor & 0xFF);
    upper = (uint8_t)((divisor >> 8) & 0xFF);

    // Send the frequency divisor
    outb(0x40, lower);
    outb(0x40, upper);

    // Initialize timer to 0
    sec = min = hr = tick = 0;
}

void print_timer()
{
    uint64_t cur_video_addr;

    // Save current video address
    cur_video_addr = get_video_addr();

    /* tick: counts the PC timer ticks at the rate of 1.18 MHz.
     * sec : counter for counting number of seconds completed. 1 sec = 100 ticks.
     * min : counter for counting number of minutes completed. 1 min = 60 seconds.
     * hr  : counter for counting number of hours completed.
     */
    tick++;
    if (tick%100 == 0) {
        sec++;
        if (sec == 60) {
            min++;
            sec = 0;
            if (min == 60) {
                hr++;
                min = 0;
                if (hr == 24) {
                    hr = 0;
                }
            }
        }
    }

    set_cursor_pos(24, 55);
    kprintf("         ");
    set_cursor_pos(24, 55);
    kprintf("%d:%d:%d", hr, min, sec);

    // Restore video address
    set_video_addr(cur_video_addr);
}

#define switch_to_ring3 \
    __asm__ __volatile__(\
        "mov $0x23, %rax;"\
        "mov %rax,  %ds;"\
        "mov %rax,  %es;"\
        "mov %rax,  %fs;"\
        "mov %rax,  %gs;")

void timer_handler()
{
    print_timer();

    if (READY_LIST != NULL) {
        if (!IsInitScheduler) {
            // The first time schedule gets invoked - additional processing needs done
            prev = READY_LIST;
            READY_LIST = READY_LIST->next;
            CURRENT_TASK = prev;
            IsInitScheduler = TRUE;

#if DEBUG_SCHEDULING
            kprintf("\nScheduler Initiated with PID: %d", prev->proc_id);
#endif

            LOAD_CR3(prev->mm->pml4_t);

            // set_tss_rsp0(prev->rsp_register);
            
            // Switch the kernel stack to that of the first process
            __asm__ __volatile__("movq %[prev_rsp], %%rsp" : : [prev_rsp] "m" (prev->rsp_register));
            //switch_to_ring3;

        } else {
            uint64_t cur_rsp;
            __asm__ __volatile__("movq %%rsp, %[cur_rsp]": [cur_rsp] "=r"(cur_rsp));

            prev = CURRENT_TASK;
            next = READY_LIST;

            prev->rsp_register = cur_rsp;

            LOAD_CR3(next->mm->pml4_t);

            // set_tss_rsp0(next->rsp_register);
            __asm__ __volatile__("movq %[next_rsp], %%rsp" : : [next_rsp] "m" (next->rsp_register));
            //switch_to_ring3;

            CURRENT_TASK = READY_LIST;
            
            // Add prev to the end of the READY_LIST
            add_to_ready_list(prev);

            READY_LIST = READY_LIST->next;

#if DEBUG_SCHEDULING
            //kprintf("\nPID:%d", next->proc_id);
#endif
        }
    }
    __asm__ __volatile__("mov $0x20, %al;" "out %al, $0x20");
}

extern void irq0();

void schedule_process(task_struct* new_task, uint64_t func_addr, uint64_t stack_ind)
{
    // Set up kernel stack => ss, rsp, rflags, cs, rip
    new_task->kernel_stack[stack_ind-1] = 0x10;
    new_task->kernel_stack[stack_ind-2] = (uint64_t)&new_task->kernel_stack[stack_ind-1];
    new_task->kernel_stack[stack_ind-3] = 0x200202UL;
    new_task->kernel_stack[stack_ind-4] = 0x08;
    new_task->kernel_stack[stack_ind-5] = func_addr;

    // Leave 9 spaces for POPA => stack_ind-6 to stack_ind-14
    
    // Set return address to POPA in irq0()
    new_task->kernel_stack[stack_ind-15] = (uint64_t)irq0 + 0x14;

    // Set rsp to stack_ind-16
    new_task->rsp_register = (uint64_t)&new_task->kernel_stack[stack_ind-16];

    new_task->rip_register = func_addr;
    new_task->next = NULL;
    new_task->last = NULL;

#if DEBUG_SCHEDULING
    kprintf("\tEntry Point:%p", func_addr);
#endif

    // Add to the READY_LIST 
    add_to_ready_list(new_task);
}

void copy_vma(task_struct* child_task, task_struct* parent_task)
{
    uint64_t start, end, vaddr, paddr;
    uint64_t *pte_entry;
    int no_of_pages = 0, count = 0;
    vma_struct *parent_vma = parent_task->mm->vma_list;
    

    while (parent_vma) {
        start = parent_vma->vm_start;
        end = parent_vma->vm_end;  
        vaddr = PAGE_ALIGN(start); 
        
        // Reset vma start address to 4k page align
        no_of_pages = (end >> 12) - (start >> 12) + 1;
        
        for (count = 0; count < no_of_pages; count++) {
        
            LOAD_CR3(parent_task->mm->pml4_t);
            pte_entry = get_pte_entry(vaddr);
        
            paddr = PAGE_ALIGN(*pte_entry);
    kprintf("\tPaddr:%p", paddr);
            LOAD_CR3(child_task->mm->pml4_t);
            
            map_virt_phys_addr(vaddr, paddr, PAGING_PRESENT_WRITABLE | PAGING_COW);
            vaddr = vaddr + PAGESIZE;
        
        }
        parent_vma = parent_vma->vm_next;
    }
    LOAD_CR3(parent_task->mm->pml4_t);
}
