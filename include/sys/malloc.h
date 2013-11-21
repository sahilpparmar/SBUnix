#include <defs.h>

char *mem_start_p;
int max_mem;
int allocated_mem; 
int mcb_count;
char *heap_end;

typedef struct
{
    int is_available;
    int size;

} MCB, *MCB_P;

enum {NEW_MCB=0,NO_MCB,REUSE_MCB};
enum {FREE,IN_USE};

//void initMem(char *ptr, int size_in_bytes);
void *myalloc(int elem_size);
void myfree(void *p);
//void init_malloc(uint64_t addr);
void make_head(char *addr, int size);
void *alloc_new(int aligned_size);
