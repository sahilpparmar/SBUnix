#include <stdio.h>
#include <sys/proc_mngr.h>
#include <sys/paging.h>
#include <sys/virt_mm.h>
#include <sys/tarfs.h>
#include <sys/elf.h>
#include <sys/kmalloc.h>
#include <string.h>

#define USER_STACK_TOP 0xF0000000

uint64_t next_pid = 0;
task_struct *task_free_list = NULL;
vma_struct *vma_free_list = NULL;

task_struct* get_free_task_struct()
{
    task_struct *ret = NULL;
    if (task_free_list != NULL) {
        ret = task_free_list;
        task_free_list = task_free_list->next;
    }
    return ret;
}

vma_struct* get_free_vma_struct()
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
    //kprintf("\n new heapend: %p", addr);
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

vma_struct* alloc_new_vma(uint64_t start_addr, uint64_t end_addr)
{
    vma_struct *vma = NULL;

    if ((vma = get_free_vma_struct()) == NULL) {
        vma = (vma_struct*) kmalloc(sizeof(vma_struct));
    }
    vma->vm_start = start_addr;
    vma->vm_end   = end_addr; 
    vma->vm_next  = NULL;
    return vma;
}

task_struct* alloc_new_task(bool IsUserProcess)
{
    mm_struct* mms = NULL;
    task_struct* new_proc = NULL;

    if ((new_proc = get_free_task_struct()) == NULL) {
        new_proc = (task_struct*) kmalloc(sizeof(task_struct));
        mms      = (mm_struct *) kmalloc(sizeof(mm_struct));
        new_proc->mm = mms;
    } else {
        mms = new_proc->mm;
    }

    // Initialize mm_struct
    mms->pml4_t     = create_new_pml4();
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

void empty_vma_list(vma_struct *vma_list)
{
    vma_struct *cur_vma  = vma_list;
    vma_struct *last_vma = NULL;

    while (cur_vma) {
        uint64_t start, end;

        start = cur_vma->vm_start;
        end   = cur_vma->vm_end;

        // Deallocate all used pages
        if (end - start) {
            uint64_t vaddr = PAGE_ALIGN(start); 
            while (vaddr < end) {
                free_virt_page((void*)vaddr);
                vaddr = vaddr + PAGESIZE;
            }
        }

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

// Loads CS and DS into VMAs and returns the entry point into process
uint64_t load_elf(Elf64_Ehdr* header, task_struct *proc)
{
    Elf64_Phdr* program_header;
    mm_struct *mms = proc->mm;
    vma_struct *node, *iter;
    uint64_t start_vaddr, end_vaddr, cur_pml4_t, max_addr = 0;
    int i, size;

    // Save current PML4 table
    READ_CR3(cur_pml4_t);

    // Offset at which program header table starts
    program_header = (Elf64_Phdr*) ((void*)header + header->e_phoff);  
    
    for (i = 0; i < header->e_phnum; ++i) {

        if ((int)program_header->p_type == 1) {           // this is loadable section
            
            
            start_vaddr    = program_header->p_vaddr;
            size           = program_header->p_memsz;
            end_vaddr      = start_vaddr + size;    
            node           = alloc_new_vma(start_vaddr, end_vaddr); 
            node->vm_flags = program_header->p_type; 
             
            if(program_header->p_type == 5)
                node->vm_type = TEXT;
            else if(program_header->p_type == 6)
                node->vm_type = DATA;

            mms->vma_count++;
            mms->total_vm += size;

            //kprintf("\nstart:%p end:%p size:%p", start_vaddr, end_vaddr, size);

            if (max_addr < end_vaddr) {
                max_addr = end_vaddr;
            }
            
            // Load ELF sections into new Virtual Memory Area
            LOAD_CR3(mms->pml4_t);

            kmmap(start_vaddr, size); 

            if (mms->vma_list == NULL) {
                mms->vma_list = node;
            } else {
                for (iter = mms->vma_list; iter->vm_next != NULL; iter = iter->vm_next);
                iter->vm_next = node;
            }

            //kprintf("\nVaddr = %p, ELF = %p, size = %p",(void*) start_vaddr, (void*) header + program_header->p_offset, size);
            memcpy((void*) start_vaddr, (void*) header + program_header->p_offset, program_header->p_filesz);

            // Set .bss section with zero
            // Note that, only in case of segment containing .bss will filesize and memsize differ 
            memset((void *)start_vaddr + program_header->p_filesz, 0, size - program_header->p_filesz);
            
            // Restore parent CR3
            LOAD_CR3(cur_pml4_t);
        }
        // Go to next program header
        program_header = program_header + 1;
    }
 
    // Traverse the vmalist to reach end vma and allocate a vma for the heap at 4k align
    for (iter = mms->vma_list; iter->vm_next != NULL; iter = iter->vm_next);
    start_vaddr    = end_vaddr = ((((max_addr - 1) >> 12) + 1) << 12);
    node           = alloc_new_vma(start_vaddr, end_vaddr); 
    node->vm_flags = RW;
    node->vm_type  = HEAP; 
    iter->vm_next  = node;
    
    mms->vma_count++;
    mms->start_brk = start_vaddr;
    mms->end_brk   = end_vaddr; 
    //kprintf("\tHeap Start:%p", mms->start_brk);

    // Stack VMA of one page (TODO: Need to allocate a dynamically growing stack)
    for (iter = mms->vma_list; iter->vm_next != NULL; iter = iter->vm_next);
    end_vaddr = USER_STACK_TOP;
    start_vaddr = USER_STACK_TOP - PAGESIZE;
    node           = alloc_new_vma(start_vaddr, end_vaddr); 
    node->vm_flags = RW;
    node->vm_type  = STACK;
    iter->vm_next  = node;
    
    // Map a physical page
    LOAD_CR3(mms->pml4_t);
    kmmap(start_vaddr, PAGESIZE);
    LOAD_CR3(cur_pml4_t);

    mms->vma_count++;
    mms->stack_vm  = PAGESIZE;
    mms->total_vm += PAGESIZE;
    mms->start_stack = end_vaddr - 8;

    return header->e_entry;
}

pid_t create_elf_proc(char *filename)
{
    HEADER *header;
    Elf64_Ehdr* elf_header;
    task_struct* new_proc;
    uint64_t entrypoint;

    // lookup for the file in tarfs
    header = (HEADER*) lookup(filename); 
    
    if (header == NULL) {
        return -1;
    }

    // Check if file is an ELF executable by checking the magic bits
    elf_header = (Elf64_Ehdr *)header;
    
    if (elf_header->e_ident[1] == 'E' && elf_header->e_ident[2] == 'L' && elf_header->e_ident[3] == 'F') {                
        new_proc = alloc_new_task(TRUE);
        kstrcpy(new_proc->comm, filename);
        entrypoint = load_elf(elf_header, new_proc);
        schedule_process(new_proc, entrypoint, (uint64_t)new_proc->mm->start_stack);
        
        return new_proc->pid;
    }
    return -1;
}

