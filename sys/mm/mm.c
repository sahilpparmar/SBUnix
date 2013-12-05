#include <sys/common.h>
#include <sys/proc_mngr.h>
#include <sys/paging.h>
#include <sys/virt_mm.h>
#include <sys/tarfs.h>
#include <sys/elf.h>
#include <sys/kmalloc.h>
#include <sys/types.h>
#include <io_common.h>
#include <sys/kstring.h>

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
    task_struct* new_task = NULL;

    if ((new_task = get_free_task_struct()) == NULL) {
        
        mm_struct* mms = (mm_struct *) kmalloc(sizeof(mm_struct));
        // Initialize mm_struct
        mms->pml4_t = create_new_pml4();
        mms->vma_list   = NULL;
        mms->vma_count  = NULL;
        mms->hiwater_vm = NULL;
        mms->total_vm   = NULL;
        mms->stack_vm   = NULL;

        new_task    = (task_struct*) kmalloc(sizeof(task_struct));
        // Initialize task_struct
        new_task->mm = mms;
        new_task->next          = NULL;
        new_task->last          = NULL;
        new_task->parent        = NULL;
        new_task->childhead     = NULL;
        new_task->siblings      = NULL;
        new_task->no_children   = 0;
        new_task->wait_on_child_pid = 0;
        memset((void*)new_task->kernel_stack, 0, KERNEL_STACK_SIZE);
        memset((void*)new_task->file_descp, 0, MAXFD*8);
    }

    // Initialize new process
    new_task->pid           = next_pid++;
    new_task->ppid          = 0;
    new_task->IsUserProcess = IsUserProcess;
    new_task->task_state    = READY_STATE;

#if DEBUG_SCHEDULING
    kprintf("\nPID:%d\tCR3: %p", new_task->pid, new_task->mm->pml4_t);
#endif

    return new_task;
}

vma_struct* alloc_new_vma(uint64_t start_addr, uint64_t end_addr, uint64_t flags, uint64_t type, uint64_t file_d)
{
    vma_struct *vma = NULL;

    if ((vma = get_free_vma_struct()) == NULL) {
        vma = (vma_struct*) kmalloc(sizeof(vma_struct));
    }
    vma->vm_start       = start_addr;
    vma->vm_end         = end_addr; 
    vma->vm_flags       = flags;
    vma->vm_type        = type; 
    vma->vm_next        = NULL;
    vma->vm_file_descp  = file_d;
    return vma;
}

void empty_task_struct(task_struct *cur_task)
{
    mm_struct *mms = cur_task->mm;

    empty_vma_list(mms->vma_list);
    empty_page_tables(mms->pml4_t);
    mms->vma_list   = NULL;
    mms->vma_count  = NULL;
    mms->hiwater_vm = NULL;
    mms->total_vm   = NULL;
    mms->stack_vm   = NULL;
    
    memset((void*)cur_task->kernel_stack, 0, KERNEL_STACK_SIZE);
    memset((void*)cur_task->file_descp, 0, MAXFD*8);
    cur_task->next        = NULL;
    cur_task->last        = NULL;
    cur_task->parent      = NULL;
    cur_task->childhead   = NULL;
    cur_task->siblings    = NULL;
    cur_task->no_children = 0;
    cur_task->wait_on_child_pid = 0;
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
        cur_vma->vm_file_descp = NULL;

        last_vma = cur_vma;
        cur_vma = cur_vma->vm_next;
    }

    // Add vma_list to vma_free_list
    if (last_vma) {
        last_vma->vm_next = vma_free_list;
        vma_free_list = vma_list;
    }
}

void add_child_to_parent(task_struct *child_task)
{
    task_struct *parent_task = child_task->parent;

    if (parent_task->childhead) {
        // Prepend new child_task to sibling list
        child_task->siblings   = parent_task->childhead;
    }
    // assign task as childhead
    parent_task->childhead = child_task;
    parent_task->no_children++;
}

void remove_parent_from_child(task_struct *parent_task)
{
    task_struct *child_next = parent_task->childhead;
    task_struct *child_cur = NULL;

    while (child_next) {
        child_cur = child_next;
        child_cur->task_state = ZOMBIE_STATE;
        child_next = child_next->siblings;
        child_cur->siblings = NULL;
    }
}

void remove_child_from_parent(task_struct *child_task)
{
    task_struct *parent_task = child_task->parent;
    task_struct *sibling_l, *last_sibling;

    // Find the child_task in childlist
    sibling_l = parent_task->childhead;
    last_sibling = NULL;
    while (sibling_l) {
        if (sibling_l == child_task) {
            break;
        }
        last_sibling = sibling_l;
        sibling_l = sibling_l->siblings;
    }

    if (!sibling_l) return; 

    // Remove from sibling list
    if (last_sibling) {
        last_sibling->siblings = sibling_l->siblings;    
    } else {
        parent_task->childhead = sibling_l->siblings;
    }

    parent_task->no_children--;
    if (parent_task->task_state == WAIT_STATE) {
        if (!parent_task->wait_on_child_pid || parent_task->wait_on_child_pid == child_task->pid) {
            parent_task->wait_on_child_pid = child_task->pid;
            parent_task->task_state = READY_STATE;
        }
    }
}

void replace_child_task(task_struct *old_task, task_struct *new_task)
{
    task_struct *parent_task = old_task->parent;
    task_struct *sibling_l, *last_sibling;

    // Find the old child task in childlist
    sibling_l = parent_task->childhead;
    last_sibling = NULL;
    while (sibling_l) {
        if (sibling_l == old_task) {
            break;
        }
        last_sibling = sibling_l;
        sibling_l = sibling_l->siblings;
    }

    if (!sibling_l) return; 
    
    // Replace old_task with new_task
    if (last_sibling) {
        last_sibling->siblings = new_task;
        new_task->siblings = sibling_l->siblings;
    } else {
        new_task->siblings = sibling_l->siblings;
        parent_task->childhead = new_task;
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

        // this vma is pointing to heap
        if (iter->vm_type == HEAP) {
            //kprintf("\n vm_start %p\t start_brk %p\t end_brk %p", iter->vm_start, mms->start_brk, mms->end_brk);
            iter->vm_end  += bytes;
            mms->end_brk  += bytes; 
            mms->total_vm += bytes;
            break;
        }
    }
}

void* kmmap(uint64_t start_addr, int bytes, uint64_t flags)
{
    int no_of_pages = 0;
    uint64_t end_vaddr;
    uint64_t cur_top_vaddr = get_top_virtaddr();

    // Use new Vaddr
    set_top_virtaddr(PAGE_ALIGN(start_addr));

    // Find no of pages to be allocated
    end_vaddr = start_addr + bytes;
    no_of_pages = ((end_vaddr-1) >> 12) - ((start_addr) >> 12) + 1;

    // Allocate VPages
    virt_alloc_pages(no_of_pages, flags);

    // Restore old top Vaddr
    set_top_virtaddr(cur_top_vaddr);

    return (void*)start_addr;
}

