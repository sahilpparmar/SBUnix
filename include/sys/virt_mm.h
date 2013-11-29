#include <defs.h>

#define KERNEL_START_VADDR 0xFFFFFFFF80000000UL

uint64_t get_top_virtaddr();
void set_top_virtaddr(uint64_t vaddr);

void* virt_alloc_pages(uint32_t no_of_vpages, uint64_t flags);
void free_virt_page(void *vaddr);
void zero_out_phys_block(uint64_t paddr);

uint64_t get_temp_vaddr(uint64_t paddr);
void free_temp_vaddr(uint64_t vaddr);
