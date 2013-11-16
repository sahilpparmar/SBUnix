#include <sys/elf.h>
#include <sys/tarfs.h>
#include <stdio.h>

void readelf(char* filename) {

    char *p;
    int i = 0;    
    Elf64_Ehdr* header = (Elf64_Ehdr*)lookup(filename);

    Elf64_Phdr* program_header = (Elf64_Phdr*)((void*)header + header->e_phoff);  //offset at which program header table starts

    for(i = 0; i < header->e_phnum; ++i){
        //kprintf("\n Type of Segment: %x",program_header->p_type);
        if((int)program_header->p_type == 1) {           // this is loadable section
        }
        program_header = program_header + 1;             //go to next program header
    }

    
    Elf64_Shdr* section_header =  (Elf64_Shdr*)((void *) header + header->e_shoff);   //offset at which section header table starts
    //getting section header corresponding to .shstrtab
    Elf64_Shdr* string_table_header = (Elf64_Shdr*)((void *) section_header + (header->e_shentsize * header->e_shstrndx)); 
    char* string_table = (char*)((void *) header + string_table_header->sh_offset);

    for(i = 0; i < header->e_shnum; ++i) {
        //printf("\n...%x....%x",section_header->sh_type, section_header->sh_offset);         
        p = (char*)((void *) string_table + section_header->sh_name);   //index into string table to get name of section
        kprintf("\n....%s", p); 
        section_header = section_header + 1;
    }
}

