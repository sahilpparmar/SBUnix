#include <defs.h>
#include <stdio.h>
#include <sys/paging_tables.h>
#include <sys/paging.h>
#include <sys/phys_mm.h>

#define ENTRIES_PER_PTE  512
#define ENTRIES_PER_PDE  512
#define ENTRIES_PER_PDPE 512
#define ENTRIES_PER_PML4 512

// For accessing Page table addresses, we need to first convert PhyADDR to VirADDR
// So need to add below kernel base address
#define KERNEL_START_VADDR 0xFFFFFFFF80000000
#define ALIGN(ADDR) ((ADDR) >> 12 << 12)
#define VADDR(PADDR) ((KERNEL_START_VADDR) + ALIGN(PADDR))
#define PADDR(VADDR) (ALIGN(VADDR) - (KERNEL_START_VADDR))

#define I86_PRESENT_WRITABLE I86_PRESENT | I86_WRITABLE; 

static uint64_t *cur_pml4_t;
static uint64_t *ker_pml4_t;
static bool pre_paging = FALSE;

uint64_t* get_pml4_t()
{
    return cur_pml4_t;
}

void set_pml4_t(uint64_t *addr)
{
    cur_pml4_t = addr;
}

uint64_t* get_ker_pml4_t()
{
    return ker_pml4_t;
}


// TO DO : Put condition check for non existing pte table entry in pde_table

uint64_t* alloc_pte(uint64_t *pde_table,int pde_off)
{
    uint64_t pte_table = phys_alloc_block();
    pde_table[pde_off] = pte_table | I86_PRESENT_WRITABLE;   
    return (uint64_t*)VADDR(pte_table);
} 

uint64_t* alloc_pde(uint64_t *pdpe_table,int pdpe_off)
{
    uint64_t pde_table = phys_alloc_block();
    pdpe_table[pdpe_off] = pde_table | I86_PRESENT_WRITABLE;   
    return (uint64_t*)VADDR(pde_table);
}

uint64_t* alloc_pdpe(uint64_t *cur_pml4_t,int pml4_off)
{
    uint64_t pdpe_table = phys_alloc_block();
    cur_pml4_t[pml4_off] = pdpe_table | I86_PRESENT_WRITABLE;   
    return (uint64_t*)VADDR(pdpe_table);
}


//------------self referencing

void alloc_pte_entry_s(uint64_t vaddr, uint64_t paddr)
{
    //printf("in pte entry self ref");
    uint64_t tvaddr;
    uint64_t *addr;

    tvaddr  = ((uint64_t)vaddr << 16 >> 16);
    tvaddr  = tvaddr >> 12 << 3;

    vaddr = ((uint64_t)tvaddr | 0xFFFFFF0000000000);
    addr = (uint64_t *)vaddr; 
    
    *addr = paddr | 0x03; //phys_alloc_block();
} 

void alloc_pte_s(uint64_t vaddr)
{

    //printf("in pte self ref");
    uint64_t tvaddr;
    uint64_t *addr;

    tvaddr  = ((uint64_t)vaddr << 16 >> 16);
    tvaddr  = tvaddr >> 21 << 3;

    vaddr = ((uint64_t)tvaddr | 0xFFFFFF7F80000000);
    addr = (uint64_t *)vaddr; 
    *addr = phys_alloc_block() | 0x03; //phys_alloc_block();
    
} 


void alloc_pde_s(uint64_t vaddr)
{
    
    //printf("in pde self ref");
    uint64_t tvaddr;
    uint64_t *addr;

    tvaddr  = ((uint64_t)vaddr << 16 >> 16);
    tvaddr  = tvaddr >> 30 << 3;

    vaddr = ((uint64_t)tvaddr | 0xFFFFFF7FBFC00000);
    addr = (uint64_t *)vaddr; 
    *addr = phys_alloc_block()| 0x03;               //phys_alloc_block();

}


void alloc_pdpe_s(uint64_t vaddr)
{
    
    // printf("in pdpe self ref");
    uint64_t tvaddr;
    uint64_t *addr;

    tvaddr  = ((uint64_t)vaddr << 16 >> 16);
    tvaddr  = tvaddr >> 39 << 3;

    vaddr = ((uint64_t)tvaddr | 0xFFFFFF7FBFDFE000);
    addr = (uint64_t *)vaddr; 
    
    *addr = phys_alloc_block() | 0x03;               //phys_alloc_block();

}


void self_ref(uint64_t vaddr, uint64_t paddr)
{
    uint64_t *pdpe_table = NULL, *pde_table = NULL, *pte_table = NULL;

    uint64_t v_addr;
    int phys_addr, p_addr, pde_off, pdpe_off, pml4_off; 

//  pte_off  = (vaddr >> 12) & 0x1FF;
    pde_off  = (vaddr >> 21) & 0x1FF;
    pdpe_off = (vaddr >> 30) & 0x1FF;
    pml4_off = (vaddr >> 39) & 0x1FF;
    
    p_addr = paddr;  
    v_addr = vaddr;

    phys_addr = (uint64_t) *(cur_pml4_t + pml4_off);
    if (phys_addr & I86_PRESENT) {
        pdpe_table =(uint64_t*) VADDR(phys_addr); 
        phys_addr = (uint64_t) *(pdpe_table + pdpe_off);
        if (phys_addr & I86_PRESENT)
        { 
            pde_table =(uint64_t*) VADDR(phys_addr); 
            phys_addr  = (uint64_t) *(pde_table + pde_off);
            if (phys_addr & I86_PRESENT) { 
                pte_table =(uint64_t*) VADDR(phys_addr);
                alloc_pte_entry_s(v_addr,p_addr); 
            } 
            else 
                alloc_pte_s(v_addr);
        } 
        
        else {
                alloc_pde_s(v_addr);
                alloc_pte_s(v_addr);
        }
    } 
    else {
            alloc_pdpe_s(v_addr);
            alloc_pde_s(v_addr);
            alloc_pte_s(v_addr);
    }
    
    alloc_pte_entry_s(vaddr, p_addr);
    printf(" --------- PML4 Address = %p, PDPE Address = %p, PDE Address = %p, PTE Address = %p --------------", cur_pml4_t, pdpe_table ,pde_table, pte_table);
}


//------------self referencing


void map(uint64_t vaddr, uint64_t paddr, uint64_t size)
{
    if (pre_paging)
        map_virt_phys_addr(vaddr, paddr, size);
    else
        self_ref(vaddr, paddr);
}

void map_virt_phys_addr(uint64_t vaddr, uint64_t paddr, uint64_t size)
{
    uint64_t *pdpe_table = NULL, *pde_table = NULL, *pte_table = NULL;

    int i, j, k, phys_addr, pde_off, pdpe_off, pml4_off , pte_off; 

    pte_off  = (vaddr >> 12) & 0x1FF;
    pde_off  = (vaddr >> 21) & 0x1FF;
    pdpe_off = (vaddr >> 30) & 0x1FF;
    pml4_off = (vaddr >> 39) & 0x1FF;

    //printf(" $$ NEW MAPPING $$ OFF => %d %d %d %d ", pml4_off, pdpe_off, pde_off, pte_off);

    phys_addr = (uint64_t) *(cur_pml4_t + pml4_off);
    //printf("%d",phys_addr & I86_PRESENT);
    if (phys_addr != 0x0) {
        pdpe_table =(uint64_t*) VADDR(phys_addr); 

        phys_addr = (uint64_t) *(pdpe_table + pdpe_off);
        if (phys_addr != 0x0) {
            pde_table =(uint64_t*) VADDR(phys_addr); 

            phys_addr  = (uint64_t) *(pde_table + pde_off);
            if (phys_addr != 0x0) {
                pte_table =(uint64_t*) VADDR(phys_addr); 
            } else {
                pte_table = alloc_pte(pde_table, pde_off);
            }
        } else {
            pde_table = alloc_pde(pdpe_table, pdpe_off);
            pte_table = alloc_pte(pde_table, pde_off);
        }
    } else {
        pdpe_table = alloc_pdpe(cur_pml4_t, pml4_off);
        pde_table = alloc_pde(pdpe_table, pdpe_off);
        pte_table = alloc_pte(pde_table, pde_off);
    }

    phys_addr = paddr;  

    if (size + pte_off <= ENTRIES_PER_PTE) {
        for (i = pte_off; i < (pte_off + size); i++) {
            pte_table[i] = phys_addr | 0x3;
            phys_addr += 0x1000;
        }
        //printf(" SIZE = %d PDPE Address %p, PDE Address %p, PTE Address %p", size, pdpe_table , pde_table, pte_table);
    } else {
        int lsize = size, no_of_pte_t;

       // printf(" SIZE = %d PDPE Address %p, PDE Address %p, PTE Address %p", lsize, pdpe_table ,pde_table, pte_table);
        for ( i = pte_off ; i < ENTRIES_PER_PTE; i++) {
            pte_table[i] = phys_addr | 0x3;
            phys_addr += 0x1000;
        }
        lsize = lsize - (ENTRIES_PER_PTE - pte_off);
        no_of_pte_t = lsize/ENTRIES_PER_PTE;

        for (j = 1; j <= no_of_pte_t; j++) {   
            pte_table = alloc_pte(pde_table, pde_off+j);
         //   printf(" SIZE = %d PDPE Address %p, PDE Address %p, PTE Address %p", lsize, pdpe_table ,pde_table, pte_table);
            for(k = 0; k < ENTRIES_PER_PTE; k++ ) { 
                pte_table[k] = phys_addr | 0x3;
                phys_addr += 0x1000;
            }
        }
        lsize = lsize - (ENTRIES_PER_PTE * pte_off);
        pte_table = alloc_pte(pde_table, pde_off+j);
        
        //printf(" SIZE = %d PDPE Address %p, PDE Address %p, PTE Address %p", lsize, pdpe_table ,pde_table, pte_table);
        for(k = 0; k < lsize; k++ ) { 
            pte_table[k] = phys_addr | 0x3;
            phys_addr += 0x1000;
        }
    }
}

void init_paging(uint64_t kernmem,uint64_t physbase, uint64_t k_size)
{
    // Allocate free memory for PML4 table 
    uint64_t pml4_paddr = phys_alloc_block();

    ker_pml4_t = cur_pml4_t = (uint64_t*) VADDR(pml4_paddr);
    printf(" $$ PML4 Address %p", cur_pml4_t);

    cur_pml4_t[510] = pml4_paddr | I86_PRESENT_WRITABLE;   
    printf(" $$ pml4[510]= %p", cur_pml4_t[510]);
    
    pre_paging = TRUE;

    // Kernal Memory Mapping 
    // Mappings for virtual address range [0xFFFFFFFF80200000, 0xFFFFFFFF80406000]
    // to physical address range [0x200000, 0x406000]
    // 2 MB for Kernal + 6 Pages for PML4, PDPE, PDE, PTE(3) tables
    
    map(kernmem, physbase, k_size);

    // Use existing Video address mapping: Virtual memory 0xFFFFFFFF800B8000 to Physical memory 0xB8000
    map(0xFFFFFFFF800B8000, 0xB8000, 1);
    
    // Set CR3 register to address of PML4 table
    asm volatile ("movq %0, %%cr3;" :: "r"(PADDR((uint64_t)(cur_pml4_t))));
    pre_paging = FALSE;
}
