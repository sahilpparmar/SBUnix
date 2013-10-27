#include <defs.h>
#include <sys/virt_mm.h>
#include <sys/types.h>

uint32_t free_mem_avail;
uint64_t currptr;

void init_kmalloc()
{
    free_mem_avail = 0;
}

void* kmalloc(uint32_t size)
{
    void* retaddr = NULL; 
    int no_of_pages = 0;

    if(size > free_mem_avail)
    {
        no_of_pages = size/PAGESIZE + 1;     
        retaddr = virt_alloc_pages(no_of_pages);         
        free_mem_avail = (no_of_pages * PAGESIZE) - size;
        currptr = (uint64_t)retaddr + (uint64_t)size;

    } else {

        retaddr = (void*)currptr;
        currptr = currptr + (uint64_t)size;
        free_mem_avail = free_mem_avail - size;
    }

    return retaddr;
}


