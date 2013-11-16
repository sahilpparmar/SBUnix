#include <stdio.h>
#include <sys/mm.h>
#include <sys/proc_mngr.h>
#include <sys/mm_types.h>
#include <sys/paging.h>
#include <sys/virt_mm.h>
#include <sys/tarfs.h>
#include <sys/elf.h>
#include <sys/kmalloc.h>

vma_struct* get_vma(uint64_t start_addr, uint64_t end_addr) {

    vma_struct *vma = (struct vm_area_struct *) kmalloc(sizeof(struct vm_area_struct));
    vma->vm_start = start_addr;
    vma->vm_end = end_addr; 
    vma->vm_next = NULL;
    return vma;

}

void* mmap(uint64_t virt_addr, int size)
{  
    int no_pages = 0;   
    uint64_t vaddr_aligned, end_vaddr;
    
    vaddr_aligned = virt_addr << 12 >> 12;
    set_top_virtaddr(vaddr_aligned);
    end_vaddr = virt_addr + size;

    no_pages = (end_vaddr >> 12) - (virt_addr >> 12) + 1;
    virt_alloc_pages(no_pages);

    return (void*)virt_addr;
}

void create_proc(char *filename) {
    mm_struct* mms;
    int i, size;
    uint64_t virt_addr, end_vaddr;
    task_struct* new_proc;
    vma_struct *node, *iter;
    Elf64_Ehdr* header = (Elf64_Ehdr*)lookup(filename); //lookup for the file in tarfs

    if(header == NULL)
        return;
    
    mms = (struct mm_struct *) kmalloc(sizeof(struct mm_struct));
    mms->mmap = NULL;
    mms->pml4_t = (uint64_t *)create_new_pml4();

    Elf64_Phdr* program_header = (Elf64_Phdr*)((void*)header + header->e_phoff);  //offset at which program header table starts
    
    // kprintf("\n entrypoint: %p", header->e_entry);
    for(i = 0; i < header->e_phnum; ++i){

    // kprintf("\n Type of Segment: %x", program_header->p_type);
        if((int)program_header->p_type == 1) {           // this is loadable section
            
            virt_addr = program_header->p_vaddr;
            size = program_header->p_memsz;
 
            end_vaddr = virt_addr + size;    
            node = get_vma(virt_addr, end_vaddr); 

            mmap(virt_addr, size); 

            if(mms->mmap == NULL) {
                mms->mmap = node;
            } else {
                for(iter = mms->mmap; iter->vm_next != NULL; iter = iter->vm_next) 
                ;
                iter->vm_next = node;
            }

            //kprintf("\nVaddr = %p, ELF = %p, size = %p",(void*) virt_addr, (void*) header + program_header->p_offset, size);
            memcpy((void*) virt_addr, (void*) header + program_header->p_offset, size);
        }
        program_header = program_header + 1;             //go to next program header
    }

    new_proc = (task_struct*)kmalloc(sizeof(task_struct));
    new_proc->mm = mms;
    create_new_process(new_proc, header->e_entry);
}

