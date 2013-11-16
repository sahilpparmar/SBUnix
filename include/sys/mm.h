#include <sys/mm_types.h>
#include <defs.h>


void* mmap(uint64_t virt_addr, int size);
void create_proc(char *filename);

struct vm_area_struct* get_vma(uint64_t start_addr, uint64_t end_addr);
/* To do:
 * 1) add the protection flag in remap_pfn_range function
 * */
