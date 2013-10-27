#include <defs.h>
#include <sys/phys_mm.h>
#include <sys/virt_mm.h>
#include <sys/paging.h>
#include <sys/types.h>


uint64_t topVirtAddr;

void virt_init (uint64_t virtBase)
{
    topVirtAddr = virtBase;
}

void set_top_virtaddr(uint64_t value)
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
       
        for(i = 0; i < no_of_vpages; ++i)
        {
             physadd = (uint64_t*) phys_alloc_block();
             map_virt_phys_addr(topVirtAddr, (uint64_t)physadd, 1);
             set_top_virtaddr(PAGESIZE);     
                  
        } 
        
    return ret_addr;
}

