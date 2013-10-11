#include <defs.h>

void pmmngr_init (uint64_t physSize, uint64_t physBase);
uint64_t pmmngr_alloc_block(); 
void pmmngr_free_block (uint64_t p);

