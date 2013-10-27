#include <defs.h>
#include <sys/phys_mm.h>

uint64_t topVirtAddr;

void virt_init (uint64_t virtBase)
{
    topVirtAddr = virtBase;
}

void* virt_alloc_pages (uint32_t no_of_vpages)
{
    void *ret_addr = NULL;
     
     
    return ret_addr;
}

