#include <defs.h>
#include <stdio.h>
#include <sys/paging.h>
#include <sys/phys_mm.h>
#include <sys/virt_mm.h>
#include <sys/kmalloc.h>

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

#define PAGING_PRESENT_WRITABLE PAGING_PRESENT | PAGING_WRITABLE; 

static uint64_t *ker_pml4_t;

uint64_t* get_ker_pml4_t()
{
    return ker_pml4_t;
}

/******************** Functions for Existing KERNEL VADDR to PADDR Mapping ******************/

static uint64_t* alloc_pte(uint64_t *pde_table, int pde_off)
{
    uint64_t pte_table = phys_alloc_block();
    pde_table[pde_off] = pte_table | PAGING_PRESENT_WRITABLE;   
    return (uint64_t*)VADDR(pte_table);
} 

static uint64_t* alloc_pde(uint64_t *pdpe_table, int pdpe_off)
{
    uint64_t pde_table = phys_alloc_block();
    pdpe_table[pdpe_off] = pde_table | PAGING_PRESENT_WRITABLE;   
    return (uint64_t*)VADDR(pde_table);
}

static uint64_t* alloc_pdpe(uint64_t *cur_pml4_t, int pml4_off)
{
    uint64_t pdpe_table = phys_alloc_block();
    ker_pml4_t[pml4_off] = pdpe_table | PAGING_PRESENT_WRITABLE;   
    return (uint64_t*)VADDR(pdpe_table);
}

static void map_kernel_virt_phys_addr(uint64_t vaddr, uint64_t paddr, uint64_t size)
{
    uint64_t *pdpe_table = NULL, *pde_table = NULL, *pte_table = NULL;

    int i, j, k, phys_addr, pde_off, pdpe_off, pml4_off , pte_off; 

    pte_off  = (vaddr >> 12) & 0x1FF;
    pde_off  = (vaddr >> 21) & 0x1FF;
    pdpe_off = (vaddr >> 30) & 0x1FF;
    pml4_off = (vaddr >> 39) & 0x1FF;

    // kprintf(" $$ NEW MAPPING $$ OFF => %d %d %d %d ", pml4_off, pdpe_off, pde_off, pte_off);

    phys_addr = (uint64_t) *(ker_pml4_t + pml4_off);
    // kprintf("%p",phys_addr);
    if (phys_addr & PAGING_PRESENT) {
        pdpe_table =(uint64_t*) VADDR(phys_addr); 

        phys_addr = (uint64_t) *(pdpe_table + pdpe_off);
        if (phys_addr & PAGING_PRESENT) {
            pde_table =(uint64_t*) VADDR(phys_addr); 

            phys_addr  = (uint64_t) *(pde_table + pde_off);
            if (phys_addr & PAGING_PRESENT) {
                pte_table =(uint64_t*) VADDR(phys_addr); 
            } else {
                pte_table = alloc_pte(pde_table, pde_off);
            }
        } else {
            pde_table = alloc_pde(pdpe_table, pdpe_off);
            pte_table = alloc_pte(pde_table, pde_off);
        }
    } else {
        pdpe_table = alloc_pdpe(ker_pml4_t, pml4_off);
        pde_table = alloc_pde(pdpe_table, pdpe_off);
        pte_table = alloc_pte(pde_table, pde_off);
    }

    phys_addr = paddr;  

    if (size + pte_off <= ENTRIES_PER_PTE) {
        for (i = pte_off; i < (pte_off + size); i++) {
            pte_table[i] = phys_addr | PAGING_PRESENT_WRITABLE; 
            phys_addr += PAGESIZE;
        }
        // kprintf(" SIZE = %d PDPE Address %p, PDE Address %p, PTE Address %p", size, pdpe_table , pde_table, pte_table);
    } else {
        int lsize = size, no_of_pte_t;

        // kprintf(" SIZE = %d PDPE Address %p, PDE Address %p, PTE Address %p", lsize, pdpe_table ,pde_table, pte_table);
        for ( i = pte_off ; i < ENTRIES_PER_PTE; i++) {
            pte_table[i] = phys_addr | PAGING_PRESENT_WRITABLE;
            phys_addr += PAGESIZE;
        }
        lsize = lsize - (ENTRIES_PER_PTE - pte_off);
        no_of_pte_t = lsize/ENTRIES_PER_PTE;

        for (j = 1; j <= no_of_pte_t; j++) {   
            pte_table = alloc_pte(pde_table, pde_off+j);
            // kprintf(" SIZE = %d PDPE Address %p, PDE Address %p, PTE Address %p", lsize, pdpe_table ,pde_table, pte_table);
            for(k = 0; k < ENTRIES_PER_PTE; k++ ) { 
                pte_table[k] = phys_addr | PAGING_PRESENT_WRITABLE;
                phys_addr += PAGESIZE;
            }
        }
        lsize = lsize - (ENTRIES_PER_PTE * pte_off);
        pte_table = alloc_pte(pde_table, pde_off+j);
        
        // kprintf(" SIZE = %d PDPE Address %p, PDE Address %p, PTE Address %p", lsize, pdpe_table ,pde_table, pte_table);
        for(k = 0; k < lsize; k++ ) { 
            pte_table[k] = phys_addr | PAGING_PRESENT_WRITABLE;
            phys_addr += PAGESIZE;
        }
    }
}

void init_paging(uint64_t kernmem, uint64_t physbase, uint64_t k_size)
{
    // Allocate free memory for PML4 table 
    uint64_t pml4_paddr = phys_alloc_block();

    ker_pml4_t = (uint64_t*) VADDR(pml4_paddr);
    kprintf("\tKernel PML4t:%p", ker_pml4_t);

    ker_pml4_t[510] = pml4_paddr | PAGING_PRESENT_WRITABLE;   
    
    // Kernal Memory Mapping 
    // Mappings for virtual address range [0xFFFFFFFF80200000, 0xFFFFFFFF80406000]
    // to physical address range [0x200000, 0x406000]
    // 2 MB for Kernal + 6 Pages for PML4, PDPE, PDE, PTE(3) tables
    map_kernel_virt_phys_addr(kernmem, physbase, k_size);

    // Use existing Video address mapping: Virtual memory 0xFFFFFFFF800B8000 to Physical memory 0xB8000
    map_kernel_virt_phys_addr(0xFFFFFFFF800B8000, 0xB8000, 1);
    
    // Set CR3 register to address of PML4 table
    __asm__ __volatile__("movq %0, %%cr3;" :: "r"(PADDR((uint64_t)(ker_pml4_t))));
    
    // Set value of top virtual address
    set_top_virtaddr(kernmem + (k_size * PAGESIZE));
           
    // Setting available free memory for kmalloc() to zero
    init_kmalloc();
}

/***************** Map VAddr to PAddr using Self Referencing Technique **************/

uint64_t* get_pte_entry(uint64_t vaddr)
{
    uint64_t tvaddr;
    uint64_t *addr;

    tvaddr  = vaddr << 16 >> 28 << 3;

    tvaddr = tvaddr | 0xFFFFFF0000000000UL;
    addr = (uint64_t *)tvaddr; 
    return addr;
} 

static uint64_t* get_pde_entry(uint64_t vaddr)
{
    uint64_t tvaddr;
    uint64_t *addr;

    tvaddr  = vaddr << 16 >> 37 << 3;

    tvaddr = tvaddr | 0xFFFFFF7F80000000UL;
    addr = (uint64_t *)tvaddr; 
    
    return addr;
} 

static uint64_t* get_pdpe_entry(uint64_t vaddr)
{
    uint64_t tvaddr;
    uint64_t *addr;

    tvaddr  = vaddr << 16 >> 46 << 3;

    tvaddr = tvaddr | 0xFFFFFF7FBFC00000UL;
    addr = (uint64_t *)tvaddr; 

    return addr;

}

static uint64_t* get_pml4_entry(uint64_t vaddr)
{
    uint64_t tvaddr;
    uint64_t *addr;

    tvaddr  = vaddr << 16 >> 55 << 3;

    tvaddr = tvaddr | 0xFFFFFF7FBFDFE000UL;
    addr = (uint64_t *)tvaddr; 
    
    return addr;
}

void map_virt_phys_addr(uint64_t vaddr, uint64_t paddr)
{
    uint64_t *pml4_entry, *pdpe_entry, *pde_entry, *pte_entry;
    uint64_t entry; 

    pml4_entry = get_pml4_entry(vaddr);
    pdpe_entry = get_pdpe_entry(vaddr); 
    pde_entry = get_pde_entry(vaddr);   
    pte_entry = get_pte_entry(vaddr);

    entry = (uint64_t) *(pml4_entry);

    if (entry & PAGING_PRESENT) {
        entry = (uint64_t) *(pdpe_entry);
        // kprintf("\tInside pdpe available");

        if (entry & PAGING_PRESENT) { 
            entry  = (uint64_t) *(pde_entry);
            // kprintf("\tInside pde available");

            if (entry & PAGING_PRESENT) { 
                // kprintf("\tInside pte available");
                *pte_entry = paddr | PAGING_PRESENT_WRITABLE;
            } else {
                // kprintf("\tInside pte creation");
                *pde_entry = phys_alloc_block() | PAGING_PRESENT_WRITABLE;
                *pte_entry = paddr | PAGING_PRESENT_WRITABLE;
            }

        } else {
            // kprintf("\tInside pde and pte creation");
            *pdpe_entry = phys_alloc_block() | PAGING_PRESENT_WRITABLE;
            *pde_entry = phys_alloc_block() | PAGING_PRESENT_WRITABLE;
            *pte_entry = paddr | PAGING_PRESENT_WRITABLE;
        }
    } else {
        // kprintf("\tInside pdpe, pde and pte creation");
        *pml4_entry = phys_alloc_block() | PAGING_PRESENT_WRITABLE;
        *pdpe_entry = phys_alloc_block() | PAGING_PRESENT_WRITABLE;
        *pde_entry = phys_alloc_block() | PAGING_PRESENT_WRITABLE;
        *pte_entry = paddr | PAGING_PRESENT_WRITABLE;
    }

    // kprintf("\nEntries: PML4: %p, PDPE: %p, PDE: %p, PTE: %p ", pml4_entry, pdpe_entry ,pde_entry, pte_entry);
}

uint64_t create_new_pml4()
{
    uint64_t virtAddr, physAddr, *cur_pml4_t;

    // Get a free virtual and physical page
    virtAddr = get_top_virtaddr();
    set_top_virtaddr(virtAddr + PAGESIZE);
    physAddr = phys_alloc_block();

    map_virt_phys_addr(virtAddr, physAddr);
    cur_pml4_t = (uint64_t *) virtAddr;    
      
    // Reserve mapping for kernel page tables
    cur_pml4_t[511] = ker_pml4_t[511];
    
    // Self referencing entry
    cur_pml4_t[510] = physAddr | PAGING_PRESENT_WRITABLE;

    asm volatile ("movq %0, %%cr3;" :: "r"(physAddr));
    
    // kprintf("\nNew PML4t: %p", physAddr);
    
    return physAddr;
}

