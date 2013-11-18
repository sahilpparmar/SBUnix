#include <stdio.h>
#include <sys/proc_mngr.h>
#include <sys/mm_types.h>
#include <sys/paging.h>
#include <sys/virt_mm.h>
#include <sys/tarfs.h>
#include <sys/elf.h>
#include <sys/kmalloc.h>

// Start from pid = 1
uint64_t next_pid = 1;

static vma_struct* alloc_new_vma(uint64_t start_addr, uint64_t end_addr)
{
    vma_struct *vma = (struct vm_area_struct *) kmalloc(sizeof(vma_struct));
    vma->vm_start = start_addr;
    vma->vm_end = end_addr; 
    vma->vm_next = NULL;
    return vma;
}

task_struct* alloc_new_task()
{
    mm_struct* mms;
    task_struct* new_proc;

    // Allocate a new mm_struct
    mms = (mm_struct *) kmalloc(sizeof(mm_struct));

    mms->pml4_t = create_new_pml4();

    // Initialize to NULL
    mms->vma_list = NULL;
    mms->vma_cache = NULL;
    mms->mmap_base = NULL;
    mms->task_size = NULL;
    mms->cached_hole_size = NULL;
    mms->free_area_cache = NULL;
    mms->vma_count = NULL;
    mms->hiwater_vm = NULL;
    mms->total_vm = NULL;
    mms->stack_vm = NULL;
    mms->flags = NULL;

    // Allocate a new task struct
    new_proc = (task_struct*) kmalloc(sizeof(task_struct));
    new_proc->proc_id = next_pid++;
    new_proc->mm = mms;

#if DEBUG_SCHEDULING
    kprintf("\nPID:%d\tCR3: %p", new_proc->proc_id, mms->pml4_t);
#endif

    return new_proc;
}

void* mmap(uint64_t start_addr, int bytes)
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
    uint64_t start_vaddr, end_vaddr, cur_pml4_t;
    int i, size;

    // Save current PML4 table
    READ_CR3(cur_pml4_t);

    // Offset at which program header table starts
    program_header = (Elf64_Phdr*) ((void*)header + header->e_phoff);  
    
    for (i = 0; i < header->e_phnum; ++i) {

        // kprintf("\n Type of Segment: %x", program_header->p_type);
        if ((int)program_header->p_type == 1) {           // this is loadable section

            start_vaddr = program_header->p_vaddr;
            size = program_header->p_memsz;
 
            end_vaddr = start_vaddr + size;    
            node = alloc_new_vma(start_vaddr, end_vaddr); 
            mms->vma_count++;
            mms->total_vm += size;

            // Load ELF sections into new Virtual Memory Area
            LOAD_CR3(mms->pml4_t);

            mmap(start_vaddr, size); 

            if (mms->vma_list == NULL) {
                mms->vma_list = node;
            } else {
                for (iter = mms->vma_list; iter->vm_next != NULL; iter = iter->vm_next) 
                ;
                iter->vm_next = node;
            }

            //kprintf("\nVaddr = %p, ELF = %p, size = %p",(void*) start_vaddr, (void*) header + program_header->p_offset, size);
            memcpy((void*) start_vaddr, (void*) header + program_header->p_offset, size);

            // Load parent CR3
            LOAD_CR3(cur_pml4_t);
        }
        // Go to next program header
        program_header = program_header + 1;
    }
    return header->e_entry;
}

void create_elf_proc(char *filename)
{
    HEADER *header;
    task_struct* new_proc;
    uint64_t entrypoint;

    // lookup for the file in tarfs
    header = (HEADER*) lookup(filename); 
    
    //TODO: Also check for ELF Magic bits
    if (header == NULL) {
        return;
    }

    new_proc = alloc_new_task();
    //TODO: Need to properly initialize kmalloc and virt_alloc_pages for user process

    entrypoint = load_elf((Elf64_Ehdr*) header, new_proc);

    schedule_process(new_proc, entrypoint);
}

