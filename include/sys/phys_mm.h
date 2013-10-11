#include <stdio.h>
#include <defs.h>

typedef uint64_t physical_addr;

void mmap_set (int bit);
void mmap_unset (int bit);
int mmap_test (int bit);
int mmap_first_free ();


void pmmngr_init (uint64_t memSize, physical_addr* bitmap, uint64_t base); 
physical_addr pmmngr_alloc_block (); 
void pmmngr_free_block (uint64_t p);
uint64_t  pmmngr_get_memory_size ();
uint64_t pmmngr_get_block_count ();
uint64_t pmmngr_get_use_block_count ();
uint64_t pmmngr_get_free_block_count ();
uint64_t pmmngr_get_block_size ();

