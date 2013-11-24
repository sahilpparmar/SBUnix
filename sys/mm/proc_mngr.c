#include <sys/kmalloc.h>
#include <sys/virt_mm.h>
#include <sys/phys_mm.h>
#include <sys/proc_mngr.h>
#include <stdio.h>
#include <screen.h>
#include <io_common.h>
#include <sys/paging.h>
#include <sys/types.h>

// The process lists. The task at the head of the READY_LIST should always be executed next
task_struct* READY_LIST = NULL;
task_struct* CURRENT_TASK = NULL;
task_struct* idle_task = NULL;

static task_struct* prev = NULL;
static task_struct* next = NULL;

// Whether scheduling has been initiated
uint8_t IsInitScheduler;

// Idle kernel thread
static void idle_process(void)
{
    kprintf("\nInside Idle Process %d", sys_getpid());
    while(1);
}

void create_idle_process()
{
    idle_task = alloc_new_task(FALSE);
    idle_task->task_state = IDLE_STATE;
    schedule_process(idle_task, (uint64_t)idle_process, (uint64_t)&idle_task->kernel_stack[KERNEL_STACK_SIZE-1]);
}

static void add_to_ready_list(task_struct* new_task)
{
    task_struct* ready_list_ptr = READY_LIST;

    if (new_task->task_state == IDLE_STATE) {
        return;
    } else if (new_task->task_state == EXIT_STATE) {
        add_to_task_free_list(new_task);
        return;
    } else if (new_task->task_state == RUNNING_STATE) {
        new_task->task_state = READY_STATE;
    }

    if (ready_list_ptr == NULL) {
        READY_LIST = new_task;
    } else {
        while (ready_list_ptr->next != NULL) {
            ready_list_ptr = ready_list_ptr->next;
        }
        ready_list_ptr->next = new_task;
        new_task->next = NULL;
    }
}

static task_struct* get_next_ready_task()
{
    task_struct* last, *next;

    CURRENT_TASK = READY_LIST;
    last = next = NULL;

    // Find a process with READY_STATE
    while (CURRENT_TASK != NULL) {
        if (CURRENT_TASK->task_state == READY_STATE) {
            next = CURRENT_TASK;
            next->task_state = RUNNING_STATE;
            break;
        }
        last = CURRENT_TASK;
        CURRENT_TASK = CURRENT_TASK->next;
    }

    if (CURRENT_TASK == NULL) {
        // Schedule IDLE process if no READY process found
        CURRENT_TASK = next = idle_task;
    } else {
        // READY process found
        if (last != NULL) {
            last->next = CURRENT_TASK->next;
            CURRENT_TASK->next = READY_LIST;
        } else {
            READY_LIST = READY_LIST->next;
        }
    }
    return next;
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
        "movq $0x23, %rax;"\
        "movq %rax,  %ds;"\
        "movq %rax,  %es;"\
        "movq %rax,  %fs;"\
        "movq %rax,  %gs;")

void timer_handler()
{
    print_timer();

    if (!IsInitScheduler) {
        IsInitScheduler = TRUE;

        // Get the first process to be scheduled
        prev = get_next_ready_task();

        LOAD_CR3(prev->mm->pml4_t);

        // Switch the kernel stack to that of the first process
        __asm__ __volatile__("movq %[prev_rsp], %%rsp" : : [prev_rsp] "m" (prev->rsp_register));

        if (prev->IsUserProcess) {
            set_tss_rsp0((uint64_t)&prev->kernel_stack[KERNEL_STACK_SIZE-1]);
            switch_to_ring3;
        }

#if DEBUG_SCHEDULING
        kprintf("\nScheduler Initiated with PID: %d[%d]", prev->pid, prev->task_state);
#endif

    } else {
        uint64_t cur_rsp;
        __asm__ __volatile__("movq %%rsp, %[cur_rsp]": [cur_rsp] "=r"(cur_rsp));

        prev = CURRENT_TASK;
        prev->rsp_register = cur_rsp;

        // Add prev to the end of the READY_LIST for states other than EXIT
        add_to_ready_list(prev);

        // Schedule next READY process
        next = get_next_ready_task();

        LOAD_CR3(next->mm->pml4_t);

        __asm__ __volatile__("movq %[next_rsp], %%rsp" : : [next_rsp] "m" (next->rsp_register));

        if (next->IsUserProcess) {
            set_tss_rsp0((uint64_t)&next->kernel_stack[KERNEL_STACK_SIZE-1]);
            switch_to_ring3;
        }

#if DEBUG_SCHEDULING
        //kprintf(" %d[%d]", next->pid, next->task_state);
#endif
    }
    __asm__ __volatile__("mov $0x20, %al;" "out %al, $0x20");
}

extern void irq0();

void schedule_process(task_struct* new_task, uint64_t entry_point, uint64_t stack_top)
{
    // 1) Set up kernel stack => ss, rsp, rflags, cs, rip
    if (new_task->IsUserProcess) {
        new_task->kernel_stack[KERNEL_STACK_SIZE-1] = 0x23;
        new_task->kernel_stack[KERNEL_STACK_SIZE-4] = 0x1b;
    } else {
        new_task->kernel_stack[KERNEL_STACK_SIZE-1] = 0x10;
        new_task->kernel_stack[KERNEL_STACK_SIZE-4] = 0x08;
    }
    new_task->kernel_stack[KERNEL_STACK_SIZE-2] = stack_top;
    new_task->kernel_stack[KERNEL_STACK_SIZE-3] = 0x200202UL;
    new_task->kernel_stack[KERNEL_STACK_SIZE-5] = entry_point;
    new_task->rip_register = entry_point;

#if DEBUG_SCHEDULING
    kprintf("\tEntry Point:%p", entry_point);
    kprintf("\tStackTop:%p", stack_top);
#endif

    // 2) Leave 9 spaces for POPA => KERNEL_STACK_SIZE-6 to KERNEL_STACK_SIZE-14

    // 3) Set return address to POPA in irq0()
    new_task->kernel_stack[KERNEL_STACK_SIZE-15] = (uint64_t)irq0 + 0x14;

    // 4) Set rsp to KERNEL_STACK_SIZE-16
    new_task->rsp_register = (uint64_t)&new_task->kernel_stack[KERNEL_STACK_SIZE-16];

    // 5) Add to the READY_LIST 
    add_to_ready_list(new_task);
}

task_struct* copy_task_struct(task_struct* parent_task)
{
    task_struct* child_task = alloc_new_task(TRUE);
    uint64_t parent_pml4_t = parent_task->mm->pml4_t;
    uint64_t child_pml4_t = child_task->mm->pml4_t;
    vma_struct *parent_vma_l = parent_task->mm->vma_list;
    vma_struct *child_vma_l = NULL;

    // Copy contains of parent mm_struct, except pml4_t and vma_list
    memcpy((void*)child_task->mm, (void*)parent_task->mm, sizeof(mm_struct));
    child_task->mm->pml4_t = child_pml4_t;
    child_task->mm->vma_list = NULL; 

    child_task->ppid   = parent_task->pid;
    child_task->parent = parent_task;
    parent_task->children = child_task;

    while (parent_vma_l) {
        uint64_t start, end;

        start = parent_vma_l->vm_start;
        end   = parent_vma_l->vm_end;  

        if (child_task->mm->vma_list == NULL) {
            child_task->mm->vma_list = alloc_new_vma(start, end);
            child_vma_l = child_task->mm->vma_list;
        } else {
            child_vma_l->vm_next = alloc_new_vma(start, end);
            child_vma_l = child_vma_l->vm_next;
        }

        // Allocate page tables if physical memory is allocated
        if (end - start) {
            uint64_t vaddr = PAGE_ALIGN(start); 
            uint64_t pte_entry, paddr, page_flags;

            // Allocate separate physical memory for only stack VMA
            if (start < child_task->mm->start_stack && child_task->mm->start_stack < end) {
                uint64_t k_vaddr = get_top_virtaddr();
                uint64_t *k_pte_entry;

                while (vaddr < end) {
                    LOAD_CR3(parent_pml4_t);

                    // Allocate a new page in kernel
                    paddr = phys_alloc_block();
                    map_virt_phys_addr(k_vaddr, paddr, PAGING_PRESENT_WRITABLE);

                    // Copy parent page in kernel space
                    memcpy((void*)k_vaddr, (void*)vaddr, PAGESIZE);

                    // Map paddr with child vaddr
                    LOAD_CR3(child_pml4_t);
                    map_virt_phys_addr(vaddr, paddr, PAGING_PRESENT_WRITABLE);

                    // Unmap k_vaddr
                    k_pte_entry = get_pte_entry(k_vaddr);
                    *k_pte_entry = 0UL;

                    vaddr = vaddr + PAGESIZE;
                }
            } else {
                while (vaddr < end) {

                    //TODO: Need to check if page is "present" and set COW bit to parent page tables
                    LOAD_CR3(parent_pml4_t);

                    pte_entry  = *get_pte_entry(vaddr);
                    page_flags = pte_entry & 0xFFF;
                    paddr      = PAGE_ALIGN(pte_entry);

                    LOAD_CR3(child_pml4_t);
                    map_virt_phys_addr(vaddr, paddr, page_flags | PAGING_COW);

                    vaddr = vaddr + PAGESIZE;
                }
            }
        }
        parent_vma_l = parent_vma_l->vm_next;
    }
    LOAD_CR3(parent_pml4_t);

    return child_task;
}

pid_t sys_fork()
{
    // Take a pointer to this process' task struct for later reference.
    task_struct *parent_task = CURRENT_TASK; 

    // Create a new process.
    task_struct* child_task = copy_task_struct(parent_task); 

    // Add it to the end of the ready queue
    schedule_process(child_task, parent_task->kernel_stack[KERNEL_STACK_SIZE-6], parent_task->kernel_stack[KERNEL_STACK_SIZE-3]);
    
    // Set return (rax) for child process to be 0
    child_task->kernel_stack[KERNEL_STACK_SIZE-7] = 0UL;

    return child_task->pid;
}

uint64_t sys_execvpe(char *filename, char *argv[])
{
    return -1;
}

uint64_t sys_exit()
{
    task_struct *cur_task = CURRENT_TASK;
    mm_struct *mms = cur_task->mm;

    empty_vma_list(mms->vma_list);
    empty_page_tables(mms->pml4_t);

    memset((void*)cur_task->kernel_stack, 0, KERNEL_STACK_SIZE);
    cur_task->task_state = EXIT_STATE;

    // Enable interrupt for scheduling next process
    sti;

    while(1);

    return 0;
}

