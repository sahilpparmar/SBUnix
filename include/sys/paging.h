void init_paging();
void init_mapping(uint64_t,uint64_t,uint64_t);
uint64_t* alloc_pte(uint64_t *pde_table,int pde_off);
uint64_t* alloc_pde(uint64_t *pdpe_table,int pdpe_off);
uint64_t* alloc_pdpe(uint64_t *pml4_table,int pml4_off);
