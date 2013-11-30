#include <defs.h>
#include <sys/proc_mngr.h>
#include <sys/paging.h>
#include <sys/elf.h>
#include <sys/tarfs.h>
#include <sys/kstring.h>
#include <stdio.h>
#include <sys/dirent.h>

void readelf(char* filename)
{
    char *p;
    int i = 0;    
    Elf64_Ehdr* header = (Elf64_Ehdr*)lookup(filename);

    Elf64_Phdr* program_header = (Elf64_Phdr*)((void*)header + header->e_phoff);  //offset at which program header table starts

    for (i = 0; i < header->e_phnum; ++i) {
        kprintf("\n Type of Segment: %x",program_header->p_type);

        if ((int)program_header->p_type == 1) {           // this is loadable section

        }
        program_header = program_header + 1;             //go to next program header
    }
    
    Elf64_Shdr* section_header =  (Elf64_Shdr*)((void *) header + header->e_shoff);   //offset at which section header table starts
    //getting section header corresponding to .shstrtab
    Elf64_Shdr* string_table_header = (Elf64_Shdr*)((void *) section_header + (header->e_shentsize * header->e_shstrndx)); 
    char* string_table = (char*)((void *) header + string_table_header->sh_offset);

    for(i = 0; i < header->e_shnum; ++i) {
        kprintf("\n...%x....%x",section_header->sh_type, section_header->sh_offset);         
        p = (char*)((void *) string_table + section_header->sh_name);   //index into string table to get name of section
        kprintf("\n....%s", p); 
        section_header = section_header + 1;
    }
}

static bool is_file_elf_exec(Elf64_Ehdr* header)
{
    if (header == NULL)
        return FALSE;

    // Check if file is an ELF executable by checking the magic bits
    if (header->e_ident[1] == 'E' && header->e_ident[2] == 'L' && header->e_ident[3] == 'F')
        return TRUE;

    return FALSE;
}

static char args[10][100];

static void copy_arg_to_stack(task_struct *task, int argc)
{
    uint64_t cur_pml4, *user_stack, *argv[10];
    int len, i;

    READ_CR3(cur_pml4);
    LOAD_CR3(task->mm->pml4_t);

    user_stack = (uint64_t*) task->mm->start_stack;
    // Store the argument values
    for (i = argc-1; i >= 0; i--) {
        len = kstrlen(args[i]) + 1;
        user_stack = (uint64_t*)((void*)user_stack - len);
        memcpy((char*)user_stack, args[i], len);
        argv[i] = user_stack;
    }
    // Store the argument pointers
    for (i = argc-1; i >= 0; i--) {
        user_stack--;
        *user_stack = (uint64_t)argv[i];
        //kprintf("\t%p=%s", user_stack, *user_stack);
    }
    // Store the arg count
    user_stack--;
    *user_stack = (uint64_t)argc;
    //kprintf("\t%p=%d", user_stack, *user_stack);

    // Reset stack top below arguments
    task->mm->start_stack = (uint64_t)user_stack;

    LOAD_CR3(cur_pml4);
}

// Loads CS and DS into VMAs and returns the entry point into process
static task_struct* load_elf(Elf64_Ehdr* header, task_struct *proc, char *filename, char *argv[])
{
    Elf64_Phdr* program_header;
    mm_struct *mms = proc->mm;
    vma_struct *node, *iter;
    uint64_t start_vaddr, end_vaddr, cur_pml4_t, max_addr, vm_type;
    int i, size;

    // Save current PML4 table
    READ_CR3(cur_pml4_t);

    kstrcpyn(proc->comm, filename, sizeof(proc->comm)-1);

    // Offset at which program header table starts
    program_header = (Elf64_Phdr*) ((void*)header + header->e_phoff);
    
    max_addr = 0; 
    for (i = 0; i < header->e_phnum; ++i) {

        if ((int)program_header->p_type == 1) {           // this is loadable section
            
            start_vaddr    = program_header->p_vaddr;
            size           = program_header->p_memsz;
            end_vaddr      = start_vaddr + size;    
             
            if (program_header->p_flags == 5) {
                vm_type = TEXT;
            } else if (program_header->p_flags == 6) {
                vm_type = DATA;
            } else {
                vm_type = NOTYPE;
            }   

            // Allocate a new vma
            node = alloc_new_vma(start_vaddr, end_vaddr, program_header->p_type, vm_type); 

            mms->vma_count++;
            mms->total_vm += size;

            //kprintf("\nstart:%p end:%p size:%p", start_vaddr, end_vaddr, size);

            if (max_addr < end_vaddr) {
                max_addr = end_vaddr;
            }
            
            // Load ELF sections into new Virtual Memory Area
            LOAD_CR3(mms->pml4_t);

            kmmap(start_vaddr, size, vm_type == TEXT ? RX_USER_FLAGS : RW_USER_FLAGS);

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
    
    iter->vm_next  = alloc_new_vma(start_vaddr, end_vaddr, RW, HEAP);
    
    mms->vma_count++;
    mms->start_brk = start_vaddr;
    mms->end_brk   = end_vaddr; 
    //kprintf("\tHeap Start:%p", mms->start_brk);

    // Allocate Stack VMA
    for (iter = mms->vma_list; iter->vm_next != NULL; iter = iter->vm_next);
    end_vaddr = USER_STACK_TOP;
    start_vaddr = USER_STACK_TOP - USER_STACK_SIZE;
    iter->vm_next = alloc_new_vma(start_vaddr, end_vaddr, RW, STACK);

    // For now map only a single physical page
    LOAD_CR3(mms->pml4_t);
    kmmap(end_vaddr-PAGESIZE, PAGESIZE, RW_USER_FLAGS);
    LOAD_CR3(cur_pml4_t);

    mms->vma_count++;
    mms->stack_vm  = USER_STACK_SIZE;
    mms->total_vm += USER_STACK_SIZE;
    
    // Initialize stack top to end of stack VMA
    mms->start_stack = end_vaddr - 0x8;
    
    int argc = 0;
    kstrcpy(args[argc++], filename);
    if (argv) {
        while (argv[argc-1]) {
            kstrcpy(args[argc], argv[argc-1]);
            argc++;
        } 
    }
    copy_arg_to_stack(proc, argc);

    schedule_process(proc, header->e_entry, mms->start_stack);

    return proc;
}

task_struct* create_elf_proc(char *filename, char *argv[])
{
    HEADER *header;
    Elf64_Ehdr* elf_header;

    // lookup for the file in tarfs
    header = (HEADER*) file_lookup(filename); 

    elf_header = (Elf64_Ehdr *)header;
    
    if (is_file_elf_exec(elf_header)) {

        task_struct* new_proc = alloc_new_task(TRUE);

        return load_elf(elf_header, new_proc, filename, argv);
    }
    return NULL;
}

