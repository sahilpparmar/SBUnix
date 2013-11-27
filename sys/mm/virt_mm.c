#include <defs.h>
#include <stdio.h>
#include <sys/phys_mm.h>
#include <sys/virt_mm.h>
#include <sys/types.h>
#include <sys/paging.h>
#include <io_common.h>

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
    uint64_t physaddr = NULL;
    int i = 0;

    if (no_of_vpages > phys_get_free_block_count()) {
        panic("\nNO FREE Pages!! Please Reboot your System");
        return NULL;
    } else {
        ret_addr = (void*)topVirtAddr; 
    }

    for (i = 0; i < no_of_vpages; ++i) {
        physaddr = phys_alloc_block();
        map_virt_phys_addr(topVirtAddr, physaddr, PAGING_PRESENT_WRITABLE);
        topVirtAddr += PAGESIZE;     
    }

    //kprintf("\tNew:%p", ret_addr);
    return ret_addr;
}

void free_virt_page(void *vaddr)
{
    uint64_t *pte_entry = NULL;
    uint64_t physaddr = NULL;

    // Get address of PTE entry
    pte_entry = get_pte_entry((uint64_t)vaddr);
    // Get physical address of the page
    physaddr = *pte_entry & PAGING_ADDR;     
    // Free the physical page
    phys_free_block(physaddr);
    // Empty PTE entry
    *pte_entry = 0;
}

void zero_out_phys_block(uint64_t paddr)
{
    uint64_t vaddr = get_top_virtaddr();
    uint64_t *k_pte_entry;

    map_virt_phys_addr(vaddr, paddr, PAGING_PRESENT_WRITABLE);

    // Copy parent page in kernel space
    memset8((uint64_t*)vaddr, 0, PAGESIZE/8);

    // Unmap k_vaddr
    k_pte_entry = get_pte_entry(vaddr);
    *k_pte_entry = 0UL;
}

