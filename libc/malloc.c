#include <syscall.h>
#include <defs.h>
#include <stdlib.h>

#define PAGESIZE 4096

static char *mem_start_p;
static char *heap_end;
static int max_mem;
static int allocated_mem; 
static int mcb_count;

typedef struct MCB_header_t {
    int is_available;
    int size;
} MCB_t, *MCB_P;

enum {NEW_MCB, NO_MCB, REUSE_MCB};
enum {FREE, IN_USE};

void make_head(char *addr, int size) 
{
    MCB_P head         = (MCB_P)addr;
    head->is_available = FREE;
    head->size         = size;
}

void *alloc_new(int aligned_size)
{
    char *add;
    uint64_t no_of_pages = 0, sz;    
    MCB_P p_mcb;
    
    sz          = sizeof(MCB_t);
    no_of_pages = (aligned_size + sz) /(PAGESIZE + 1) + 1;
    add = (char*) __syscall1(BRK, (uint64_t)no_of_pages);    

    if (heap_end == 0) {
        mem_start_p   = add;
        mcb_count     = 0;
        allocated_mem = 0;
        heap_end = add; 
    }
    
    heap_end = (char*)((uint64_t)add + (uint64_t)(PAGESIZE * no_of_pages));
    max_mem += PAGESIZE * no_of_pages; 
    
    p_mcb               = (MCB_P)add; 
    p_mcb->is_available = IN_USE;
    p_mcb->size         = aligned_size + sz; 
    mcb_count++;    

    if (PAGESIZE * no_of_pages > aligned_size + sz) {
        make_head(((char *)p_mcb + aligned_size + sz), (PAGESIZE * no_of_pages - aligned_size - sz));
    }

    allocated_mem += aligned_size; 
    
    return ((void *) p_mcb + sz);
}

void* malloc(int elem_size)
{
    MCB_P p_mcb;
    int flag, sz, temp = 0, aligned_size;

    // Align elem_size to header size
    aligned_size = ((((elem_size - 1) >> 3) + 1) << 3);

    if (heap_end == 0) {
        /*no heap has been assigned yet*/
        return alloc_new(aligned_size);
    } else {

        flag  = NO_MCB;
        p_mcb = (MCB_P)mem_start_p;
        sz    = sizeof(MCB_t);

        //printf("\nheap_end : %p\treqd size: %p",heap_end, ((char *)p_mcb + aligned_size + sz));
        while (heap_end >= ((char *)p_mcb + aligned_size + sz)) {
            if (p_mcb->is_available == FREE) {
                if (p_mcb->size >= (aligned_size + sz)) {
                    flag = REUSE_MCB;
                    break;
                }
            }
            p_mcb = (MCB_P) ((char *)p_mcb + p_mcb->size);
        }

        if (flag != NO_MCB) {
            p_mcb->is_available = IN_USE;

            if (flag == REUSE_MCB) {
                if (p_mcb->size > aligned_size + sz) {
                    temp        = p_mcb->size; 
                    p_mcb->size = aligned_size + sz;

                    make_head(((char *)p_mcb + aligned_size + sz),(temp - aligned_size - sz));
                }        
                mcb_count++;
            }
            allocated_mem += aligned_size;
            return ((char *) p_mcb + sz);
        }

        /*when no hole is found to match the request*/
        return alloc_new(aligned_size); 
    }

}

void free(void *p)
{
    MCB_P ptr = (MCB_P)p;
    ptr--;

    mcb_count--;
    ptr->is_available = FREE;
    allocated_mem    -= (ptr->size - sizeof(MCB_t));

}

