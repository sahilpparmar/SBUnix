void init_paging(uint64_t, uint64_t, uint64_t);
void map_virt_phys_addr(uint64_t,uint64_t,uint64_t);

void set_pml4_t(uint64_t *addr);
uint64_t* get_pml4_t();
uint64_t* get_ker_pml4_t();
