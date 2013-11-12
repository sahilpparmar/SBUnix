#include <defs.h>

uint64_t get_top_virtaddr();
void virt_init (uint64_t virtBase);
void add_to_virtaddr(uint64_t value);
void* virt_alloc_pages (uint32_t no_of_vpages);
void free_virt_page(uint64_t *vaddr);

