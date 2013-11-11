#include <defs.h>
#include <sys/virt_mm.h>
#include <sys/types.h>

// 2 ^ POWER2_ALIGN = BLOCK_ALIGN
#define BLOCK_ALIGN  16
#define POWER2_ALIGN  4

static uint32_t free_mem_avail;
static uint64_t currptr;

void init_kmalloc()
{
    free_mem_avail = 0;
    currptr = 0;
}

void* kmalloc(uint32_t size)
{
    void* retaddr = NULL; 
    int no_of_pages = 0;
    // Align size to BLOCK_ALIGN
    if (size % BLOCK_ALIGN) {
        size = size >> POWER2_ALIGN << POWER2_ALIGN;
        size += BLOCK_ALIGN;
    }

    if (size > free_mem_avail) {
        no_of_pages = size/(PAGESIZE + 1) + 1;     
        
        retaddr = virt_alloc_pages(no_of_pages);         
        if (retaddr != NULL) {
            free_mem_avail = (no_of_pages * PAGESIZE) - size;
            currptr = (uint64_t)retaddr + (uint64_t)size;
        }

    } else {
        retaddr = (void*)currptr;
        currptr = currptr + (uint64_t)size;
        free_mem_avail = free_mem_avail - size;
    }

    return retaddr;
}


