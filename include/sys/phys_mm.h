#include <defs.h>

void phys_init(uint64_t physBase, uint64_t physfree, uint64_t physSize);
uint64_t phys_alloc_block(); 
void phys_free_block (uint64_t p);
uint64_t phys_get_free_block_count ();
void phys_inc_block_ref(uint64_t paddr);
void phys_dec_block_ref(uint64_t paddr);
int phys_get_block_ref(uint64_t paddr);
void mmap_set(int bit);

