#include <defs.h>
#include <stdio.h>
#include <sys/phys_mm.h>
#include <sys/virt_mm.h>
#include <sys/paging.h>
#include <sys/paging_tables.h>
#include <sys/types.h>

uint64_t topVirtAddr;

uint64_t get_top_virtaddr()
{
    topVirtAddr += 0x1000;
    return topVirtAddr;
}

void virt_init (uint64_t virtBase)
{
    topVirtAddr = virtBase;
}

void add_to_virtaddr(uint64_t value)
{
    topVirtAddr += value;
}

void* virt_alloc_pages (uint32_t no_of_vpages)
{
    void *ret_addr = NULL; 
    uint64_t* physadd = NULL;
    int i = 0;

    if(no_of_vpages >  phys_get_free_block_count()) {
        return NULL;
    } else {
        ret_addr = (void*)topVirtAddr; 
    }

    for(i = 0; i < no_of_vpages; ++i) {
        physadd = (uint64_t*) phys_alloc_block();
        map_virt_phys_addr(topVirtAddr, (uint64_t)physadd);
        add_to_virtaddr(PAGESIZE);     
    } 

    return ret_addr;
}

void free_virt_addr(uint64_t *vaddr)
{
    uint64_t *pte_entry = NULL;
    uint64_t physaddr = NULL;

    // Get address of PTE entry
    pte_entry = get_pte_entry((uint64_t)vaddr);
    // Get physical address of the page
    physaddr = (*pte_entry) >> 12 << 12;     
    // Free the physical page
    phys_free_block (physaddr);
    // Empty PTE entry
    *pte_entry = 0;
}
