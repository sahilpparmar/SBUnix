#include <stdio.h>
#include <sys/proc_mngr.h>
#include <sys/paging.h>
#include <sys/virt_mm.h>
#include <sys/tarfs.h>
#include <sys/elf.h>
#include <sys/kmalloc.h>
#include <sys/types.h>
#include <string.h>

static uint64_t next_pid = 0;

void set_next_pid(pid_t fnext_pid)
{
    next_pid = fnext_pid;
}

static task_struct *task_free_list = NULL;
static vma_struct *vma_free_list = NULL;

static task_struct* get_free_task_struct()
{
    task_struct *ret = NULL;
    if (task_free_list != NULL) {
        ret = task_free_list;
        task_free_list = task_free_list->next;
    }
    return ret;
}

static vma_struct* get_free_vma_struct()
{
    vma_struct *ret = NULL;
    if (vma_free_list != NULL) {
        ret = vma_free_list;
        vma_free_list = vma_free_list->vm_next;
    }
    return ret;
}

void add_to_task_free_list(task_struct* free_task)
{
    free_task->next = task_free_list;
    task_free_list = free_task;
}

void add_to_vma_free_list(vma_struct* free_vma)
{
    free_vma->vm_next = vma_free_list;
    vma_free_list = free_vma;
}

task_struct* alloc_new_task(bool IsUserProcess)
{
    mm_struct* mms = NULL;
    task_struct* new_proc = NULL;

    if ((new_proc = get_free_task_struct()) == NULL) {
        new_proc    = (task_struct*) kmalloc(sizeof(task_struct));
        mms         = (mm_struct *) kmalloc(sizeof(mm_struct));
        mms->pml4_t = create_new_pml4();
        new_proc->mm = mms;
    } else {
        mms = new_proc->mm;
    }

    // Initialize mm_struct
    mms->vma_list   = NULL;
    mms->vma_count  = NULL;
    mms->hiwater_vm = NULL;
    mms->total_vm   = NULL;
    mms->stack_vm   = NULL;

    // Initialize new process
    new_proc->pid           = next_pid++;
    new_proc->ppid          = 0;
    new_proc->IsUserProcess = IsUserProcess;
    new_proc->task_state    = READY_STATE;
    new_proc->next          = NULL;
    new_proc->last          = NULL;
    new_proc->parent        = NULL;
    new_proc->children      = NULL;
    new_proc->sibling       = NULL;

#if DEBUG_SCHEDULING
    kprintf("\nPID:%d\tCR3: %p", new_proc->pid, mms->pml4_t);
#endif

    return new_proc;
}

vma_struct* alloc_new_vma(uint64_t start_addr, uint64_t end_addr)
{
    vma_struct *vma = NULL;

    if ((vma = get_free_vma_struct()) == NULL) {
        vma = (vma_struct*) kmalloc(sizeof(vma_struct));
    }
    vma->vm_start = start_addr;
    vma->vm_end   = end_addr; 
    vma->vm_flags = NULL;
    vma->vm_type  = NULL; 
    vma->vm_next  = NULL;
    return vma;
}

void exit_task_struct(task_struct *new_task)
{
    mm_struct *mms = new_task->mm;

    empty_vma_list(mms->vma_list);
    empty_page_tables(mms->pml4_t);
    
    memset((void*)new_task->kernel_stack, 0, KERNEL_STACK_SIZE);
    new_task->task_state = EXIT_STATE;
}

void empty_vma_list(vma_struct *vma_list)
{
    vma_struct *cur_vma  = vma_list;
    vma_struct *last_vma = NULL;

    while (cur_vma) {

        cur_vma->vm_mm    = NULL;
        cur_vma->vm_start = NULL;
        cur_vma->vm_end   = NULL;
        cur_vma->vm_flags = NULL;

        last_vma = cur_vma;
        cur_vma = cur_vma->vm_next;
    }

    // Add vma_list to vma_free_list
    if (last_vma) {
        last_vma->vm_next = vma_free_list;
        vma_free_list = vma_list;
    }
}


bool verify_addr(task_struct *proc, uint64_t addr, uint64_t size)
{
    //traverse vma list and check for addr
    
    mm_struct *mms = proc->mm;
    vma_struct *iter;

    for (iter = mms->vma_list; iter != NULL; iter = iter->vm_next) {

        if (addr < iter->vm_start && (addr + size) > iter->vm_start)
            return 0;

        if (addr < iter->vm_end && (addr + size) > iter->vm_end)
            return 0;
    }
    
    if (addr >= KERNEL_START_VADDR)
        return 0;

    return 1;
}

void increment_brk(task_struct *proc, uint64_t bytes)
{   
    mm_struct *mms = proc->mm;
    vma_struct *iter;
    
    for (iter = mms->vma_list; iter != NULL; iter = iter->vm_next) {
        //kprintf("\n vm_start %p\t start_brk %p\t end_brk %p", iter->vm_start, mms->start_brk, mms->end_brk);

        // this vma is pointing to heap
        if (iter->vm_start == mms->start_brk) {
            iter->vm_end  += bytes;
            mms->end_brk  += bytes; 
            mms->total_vm += bytes;
            break;
        }
    }
}

void* kmmap(uint64_t start_addr, int bytes)
{
    int no_of_pages = 0;
    uint64_t end_vaddr;
    uint64_t cur_top_vaddr = get_top_virtaddr();

    // Use new Vaddr
    set_top_virtaddr(PAGE_ALIGN(start_addr));

    // Find no of pages to be allocated
    end_vaddr = start_addr + bytes;
    no_of_pages = (end_vaddr >> 12) - (start_addr >> 12) + 1;

    // Allocate VPages
    virt_alloc_pages(no_of_pages);

    // Restore old top Vaddr
    set_top_virtaddr(cur_top_vaddr);

    return (void*)start_addr;
}

