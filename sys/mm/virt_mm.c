#include <defs.h>
#include <stdio.h>
#include <sys/phys_mm.h>
#include <sys/virt_mm.h>
#include <sys/types.h>
#include <sys/paging.h>

#define PAGING_PRESENT_WRITABLE PAGING_PRESENT | PAGING_WRITABLE | PAGING_USER

uint64_t topVirtAddr;

uint64_t get_top_virtaddr()
{
    return topVirtAddr;
}

void set_top_virtaddr(uint64_t vaddr)
{
    topVirtAddr = vaddr;
    //kprintf("\nTopVaddr = %p", topVirtAddr);
}

void* virt_alloc_pages(uint32_t no_of_vpages)
{
    void *ret_addr = NULL; 
    uint64_t* physaddr = NULL;
    int i = 0;

    if (no_of_vpages > phys_get_free_block_count()) {
        return NULL;
    } else {
        ret_addr = (void*)topVirtAddr; 
    }

    for(i = 0; i < no_of_vpages; ++i) {
        physaddr = (uint64_t*) phys_alloc_block();
        map_virt_phys_addr(topVirtAddr, (uint64_t)physaddr, PAGING_PRESENT_WRITABLE);
        topVirtAddr += PAGESIZE;     
    }

    // kprintf("\tNew Vaddr = %p", ret_addr);
    return ret_addr;
}

void free_virt_page(void *vaddr)
{
    uint64_t *pte_entry = NULL;
    uint64_t physaddr = NULL;

    // Zero out the contents of page
    memset8(vaddr, 0x0, PAGESIZE/8);
    // Get address of PTE entry
    pte_entry = get_pte_entry((uint64_t)vaddr);
    // Get physical address of the page
    physaddr = (*pte_entry) >> 12 << 12;     
    // Free the physical page
    phys_free_block (physaddr);
    // Empty PTE entry
    *pte_entry = 0;
}

