#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <sys/paging_tables.h>
#include <sys/paging.h>
#include <sys/phys_mm.h>

#define ENTRIES_PER_PTE  512
#define ENTRIES_PER_PDE  512
#define ENTRIES_PER_PDPE 512
#define ENTRIES_PER_PML4 512

#define I86_PDPE_PRESENT_WRITABLE I86_PDPE_PRESENT | I86_PDPE_WRITABLE; 
#define I86_PDE_PRESENT_WRITABLE I86_PDE_PRESENT | I86_PDE_WRITABLE; 
#define I86_PTE_PRESENT_WRITABLE I86_PTE_PRESENT | I86_PTE_WRITABLE; 

static uint64_t *pml4_table;

uint64_t* alloc_pte(uint64_t *pde_table,int pde_off)
{
    uint64_t *pte_table;
    pte_table = (uint64_t *) pmmngr_alloc_block();
    pde_table[pde_off] = (uint64_t) pte_table | I86_PTE_PRESENT_WRITABLE;   
    return pte_table;
}

uint64_t* alloc_pde(uint64_t *pdpe_table,int pdpe_off)
{
    uint64_t *pde_table;
    pde_table = (uint64_t *) pmmngr_alloc_block();
    pdpe_table[pdpe_off] = (uint64_t) pde_table | I86_PDE_PRESENT_WRITABLE;   
    return pde_table;
}

uint64_t* alloc_pdpe(uint64_t *pml4_table,int pml4_off)
{
    uint64_t *pdpe_table;
    pdpe_table = (uint64_t *) pmmngr_alloc_block();
    pml4_table[pml4_off] = (uint64_t) pdpe_table | I86_PDPE_PRESENT_WRITABLE;   
    return pdpe_table;
}

void init_mapping(uint64_t vaddr, uint64_t paddr, uint64_t size)
{
    uint64_t *pdpe_table, *pde_table, *pte_table;

    int i, j, k, phys_addr, pde_off, pdpe_off, pml4_off , pte_off; 
    uint64_t v_addr = vaddr, addr; 

    pte_off = (v_addr >> 12) & 0x1FF;
    pde_off = (v_addr >> 21) & 0x1FF;
    pdpe_off = (v_addr >> 30) & 0x1FF;
    pml4_off = (v_addr >> 39) & 0x1FF;

    if (*(pml4_table + pml4_off) != NULL)
    {
        addr = (uint64_t) *(pml4_table + pml4_off);
        pdpe_table =(uint64_t*) (addr & 0xFFFFFFFFFF000); 
        if  (*(pdpe_table + pdpe_off) != NULL)
        {
            addr  = (uint64_t) *(pdpe_table + pdpe_off);
            pde_table =(uint64_t*) (addr & 0xFFFFFFFFFF000); 
            if  (*(pde_table + pde_off) != NULL)
            {
                addr  = (uint64_t) *(pde_table + pde_off);
                pte_table =(uint64_t*) (addr & 0xFFFFFFFFFF000); 
            }
            else
            {
                pte_table = alloc_pte(pde_table,pde_off);
            }
        }
        else
        {
            pde_table = alloc_pde(pdpe_table,pdpe_off);
            pte_table = alloc_pte(pde_table,pde_off);
        }
    }
    else
    {
        pdpe_table = alloc_pdpe(pml4_table,pml4_off);
        pde_table = alloc_pde(pdpe_table,pdpe_off);
        pte_table = alloc_pte(pde_table,pde_off);
    }

    phys_addr = paddr;  

    if (size + pte_off <= ENTRIES_PER_PTE)
    {
        for ( i = pte_off ; i < (pte_off + size) ; i++)
        {
            pte_table[i] = phys_addr | 0x3;
            phys_addr += 0x1000;
        }
        printf("PDPE Address %p, PDE Address %p, PTE Address %p", pdpe_table ,pde_table,pte_table);
    }
    else
    {
        for ( i = pte_off ; i < ENTRIES_PER_PTE ; i++)
        {
            pte_table[i] = phys_addr | 0x3;
            phys_addr += 0x1000;
        }
        printf("PDPE Address %p, PDE Address %p, PTE Address %p", pdpe_table ,pde_table,pte_table);

        for ( j = 0 ; j < (size - (ENTRIES_PER_PTE - pte_off))/ENTRIES_PER_PTE ; j++ )
        {   
            printf("PDPE Address %p, PDE Address %p, PTE Address %p", pdpe_table ,pde_table,pte_table);
            pte_table = alloc_pte(pde_table,pde_off+j+1);
            for(k = 0; k < ENTRIES_PER_PTE; k++ )
            {
                pte_table[k] = phys_addr | 0x3;
                phys_addr += 0x1000;
            }

            //TO DO : Put condition check for non existing pte table entry in pde_table
            pte_table = alloc_pte(pde_table,pde_off+j+1);
        }
    }
}

void init_paging(uint64_t kernmem,uint64_t physbase, uint64_t k_size)
{
    //Allocate free memory for PML4 table 
    pml4_table = (uint64_t*) pmmngr_alloc_block();

    // Kernal Memory Mapping 
    // Mappings for virtual address range [0xFFFFFFFF80200000, 0xFFFFFFFF80406000]
    // to physical address range [0x200000, 0x406000]
    // 2 MB for Kernal + 6 Pages for PML4, PDPE, PDE, PTE(3) tables
    init_mapping(kernmem, physbase, k_size);

    // Remap Video base address: Virtual memory 0xFFFFFFFF800B8000 to Physical memory 0xB8000
    init_mapping(0xFFFFFFFF800B8000,0xB8000,1);

    // Set CR3 register to address of PML4 table
    asm volatile ("movq %0, %%cr3;" :: "r"((uint64_t)pml4_table));

    // Change Video base address
    init_screen(0xFFFFFFFF800B8000);
    
}

