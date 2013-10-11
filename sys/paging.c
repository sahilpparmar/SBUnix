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

static void init() 
{
    pml_table  = (uint64_t*) pmmngr_alloc_block();
    pdpe_table = (uint64_t*) pmmngr_alloc_block();
    pde_table  = (uint64_t*) pmmngr_alloc_block();
    pte_table  = (uint64_t*) pmmngr_alloc_block();

    set_cursor_pos(17, 5);
    printf("%p..%p..%p..%p", pml_table, pdpe_table, pde_table, pte_table);
}

void kernel_allocate()
{
    int i, phys_addr; 

    init(); 

    pml_table[511]  = (uint64_t) pdpe_table | 0x3; 
    pdpe_table[510] = (uint64_t) pde_table  | 0x3;
    pde_table[1]    = (uint64_t) pte_table  | 0x3; 

    phys_addr = 0x200000;

    for(i = 0; i< ENTRIES_PER_PTE; ++i)
    {
        pte_table[i] = phys_addr | 0x3;
        phys_addr += 0x1000;
    }

    pte_table  = (uint64_t*) pmmngr_alloc_block();
    pde_table[2] = (uint64_t) pte_table | 0x3; 

    phys_addr = 0x400000;

    for(i = 0; i< ENTRIES_PER_PTE; ++i)
    {
        pte_table[i] = phys_addr | 0x3;
        phys_addr += 0x1000;
    }
    
    // Remap Video address
    pte_table[184] = 0xB8000 | 0x3;

    __asm__(
        "movq $0x400000, %rax;"
        "movq %rax, %cr3;"
    );

    // Change Video base address to current virtual address
    init_screen(0xFFFFFFFF804B8000);
}

