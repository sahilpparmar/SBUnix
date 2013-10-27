#include <defs.h>

void virt_init (uint64_t virtBase);
void set_top_virtaddr(uint64_t value);
void* virt_alloc_pages (uint32_t no_of_vpages);

