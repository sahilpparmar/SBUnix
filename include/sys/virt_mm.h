#include <defs.h>

#define KERNEL_START_VADDR 0xFFFFFFFF80000000UL

uint64_t get_top_virtaddr();
void set_top_virtaddr(uint64_t vaddr);

void* virt_alloc_pages (uint32_t no_of_vpages);
void free_virt_page(void *vaddr);
void zero_out_phys_block(uint64_t paddr);

