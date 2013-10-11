#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <sys/paging.h>
#include <sys/phys_mm.h>

#define ENTRIES_PER_PTE  512
#define ENTRIES_PER_PDE  512
#define ENTRIES_PER_PDPE 512
#define ENTRIES_PER_PML  512

static uint64_t *pml_table;
static uint64_t *pdpe_table;
static uint64_t *pde_table;
static uint64_t *pte_table;

void init_paging()
{
    int i, phys_addr; 

    pml_table  = (uint64_t*) pmmngr_alloc_block();
    pdpe_table = (uint64_t*) pmmngr_alloc_block();
    pde_table  = (uint64_t*) pmmngr_alloc_block();
    

    // Mappings for virtual address range [0xFFFFFFFF80200000, 0xFFFFFFFF80600000]
    // to physical address range [0x200000, 0x600000]
    pml_table[511]  = (uint64_t) pdpe_table | 0x3;
    pdpe_table[510] = (uint64_t) pde_table  | 0x3;

    pte_table = (uint64_t*) pmmngr_alloc_block();
    pde_table[1] = (uint64_t) pte_table  | 0x3; 
    
    // Assigning physical addresses 0x200000 to 0x400000 in PTE
    phys_addr = 0x200000;
    for(i = 0; i< ENTRIES_PER_PTE; ++i)
    {
        pte_table[i] = phys_addr | 0x3;
        phys_addr += 0x1000;
    }

    pte_table = (uint64_t*) pmmngr_alloc_block();
    pde_table[2] = (uint64_t) pte_table | 0x3; 
    
    // Assigning physical addresses 0x400000 to 0x600000 in PTE
    phys_addr = 0x400000;
    for(i = 0; i< ENTRIES_PER_PTE; ++i)
    {
        pte_table[i] = phys_addr | 0x3;
        phys_addr += 0x1000;
    }
    
    // Remap Video base address: Virtual memory 0xFFFFFFFF804B8000 to Physical memory 0xB8000
    pte_table[184] = 0xB8000 | 0x3;

    // Set CR3 register to address of PML4 table
    __asm__(
        "movq $0x400000, %rax;"
        "movq %rax, %cr3;"
    );

    // Change Video base address
    init_screen(0xFFFFFFFF804B8000);
}

