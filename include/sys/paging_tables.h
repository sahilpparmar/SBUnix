enum PAGE_PML4_FLAGS {

    I86_PML4_PRESENT         =   1,                                //00000000000000000000000000000000 00000000000000000000000000000001
    I86_PML4_WRITABLE        =   2,                                //00000000000000000000000000000000 00000000000000000000000000000010
    I86_PML4_USER            =   4,                                //00000000000000000000000000000000 00000000000000000000000000000100
    I86_PML4_PWT             =   8,                                //00000000000000000000000000000000 00000000000000000000000000001000
    I86_PML4_PCD             =   0x10,                             //00000000000000000000000000000000 00000000000000000000000000010000
    I86_PML4_ACCESSED        =   0x20,                             //00000000000000000000000000000000 00000000000000000000000000100000
    I86_PML4_IGN             =   0x40,                             //00000000000000000000000000000000 00000000000000000000000001000000
    I86_PML4_MBZ             =   0x180,                            //00000000000000000000000000000000 00000000000000000000000110000000
    I86_PML4_AVL             =   0xD00,                            //00000000000000000000000000000000 00000000000000000000111000000000
    I86_PML4_PDPB_ADDR       =   0x000FFFFFFFFFF000,               //00000000000011111111111111111111 11111111111111111111000000000000
    I86_PML4_AVAILABLE       =   0x7FF0000000000000,               //01111111111100000000000000000000 00000000000000000000000000000000
    I86_PML4_NX              =   0x8000000000000000,               //10000000000000000000000000000000 00000000000000000000000000000000
};


    
enum PAGE_PDPE_FLAGS {

    I86_PDPE_PRESENT         =   1,                                //00000000000000000000000000000000 00000000000000000000000000000001
    I86_PDPE_WRITABLE        =   2,                                //00000000000000000000000000000000 00000000000000000000000000000010
    I86_PDPE_USER            =   4,                                //00000000000000000000000000000000 00000000000000000000000000000100
    I86_PDPE_PWT             =   8,                                //00000000000000000000000000000000 00000000000000000000000000001000
    I86_PDPE_PCD             =   0x10,                             //00000000000000000000000000000000 00000000000000000000000000010000
    I86_PDPE_ACCESSED        =   0x20,                             //00000000000000000000000000000000 00000000000000000000000000100000
    I86_PDPE_IGN             =   0x40,                             //00000000000000000000000000000000 00000000000000000000000001000000
    I86_PDPE_ZERO            =   0,                                //00000000000000000000000000000000 00000000000000000000000000000000
    I86_PDPE_MBZ             =   0x100,                            //00000000000000000000000000000000 00000000000000000000000100000000
    I86_PDPE_AVL             =   0xD00,                            //00000000000000000000000000000000 00000000000000000000111000000000
    I86_PDPE_PDB_ADDR       =   0x000FFFFFFFFFF000,               //00000000000011111111111111111111 11111111111111111111000000000000
    I86_PDPE_AVAILABLE       =   0x7FF0000000000000,               //01111111111100000000000000000000 00000000000000000000000000000000
    I86_PDPE_NX              =   0x8000000000000000,               //10000000000000000000000000000000 00000000000000000000000000000000
};


enum PAGE_PDE_FLAGS {

    I86_PDE_PRESENT         =   1,                                //00000000000000000000000000000000 00000000000000000000000000000001
    I86_PDE_WRITABLE        =   2,                                //00000000000000000000000000000000 00000000000000000000000000000010
    I86_PDE_USER            =   4,                                //00000000000000000000000000000000 00000000000000000000000000000100
    I86_PDE_PWT             =   8,                                //00000000000000000000000000000000 00000000000000000000000000001000
    I86_PDE_PCD             =   0x10,                             //00000000000000000000000000000000 00000000000000000000000000010000
    I86_PDE_ACCESSED        =   0x20,                             //00000000000000000000000000000000 00000000000000000000000000100000
    I86_PDE_IGN1            =   0x40,                             //00000000000000000000000000000000 00000000000000000000000001000000
    I86_PDE_ZERO            =   0,                                //00000000000000000000000000000000 00000000000000000000000000000000
    I86_PDE_IGN2            =   0x100,                            //00000000000000000000000000000000 00000000000000000000000100000000
    I86_PDE_AVL             =   0xD00,                            //00000000000000000000000000000000 00000000000000000000111000000000
    I86_PDE_PTB_ADDR        =   0x000FFFFFFFFFF000,               //00000000000011111111111111111111 11111111111111111111000000000000
    I86_PDE_AVAILABLE       =   0x7FF0000000000000,               //01111111111100000000000000000000 00000000000000000000000000000000
    I86_PDE_NX              =   0x8000000000000000,               //10000000000000000000000000000000 00000000000000000000000000000000
};

enum PAGE_PTE_FLAGS {

    I86_PTE_PRESENT         =   1,                                //00000000000000000000000000000000 00000000000000000000000000000001
    I86_PTE_WRITABLE        =   2,                                //00000000000000000000000000000000 00000000000000000000000000000010
    I86_PTE_USER            =   4,                                //00000000000000000000000000000000 00000000000000000000000000000100
    I86_PTE_PWT             =   8,                                //00000000000000000000000000000000 00000000000000000000000000001000
    I86_PTE_PCD             =   0x10,                             //00000000000000000000000000000000 00000000000000000000000000010000
    I86_PTE_ACCESSED        =   0x20,                             //00000000000000000000000000000000 00000000000000000000000000100000
    I86_PTE_IGN1            =   0x40,                             //00000000000000000000000000000000 00000000000000000000000001000000
    I86_PTE_ZERO            =   0,                                //00000000000000000000000000000000 00000000000000000000000000000000
    I86_PTE_IGN2            =   0x100,                            //00000000000000000000000000000000 00000000000000000000000100000000
    I86_PTE_AVL             =   0xD00,                            //00000000000000000000000000000000 00000000000000000000111000000000
    I86_PTE_PTB_ADDR        =   0x000FFFFFFFFFF000,               //00000000000011111111111111111111 11111111111111111111000000000000
    I86_PTE_AVAILABLE       =   0x7FF0000000000000,               //01111111111100000000000000000000 00000000000000000000000000000000
    I86_PTE_NX              =   0x8000000000000000,               //10000000000000000000000000000000 00000000000000000000000000000000
};

/*********************************************************************************************************

//! a page map level 4 table format
typedef uint64_t pml4_entry;

extern void     plm4_entry_add_attrib (plm4_entry* e, uint64_t attrib);
extern void     plm4_entry_del_attrib (plm4_entry* e, uint64_t attrib);
extern void     plm4_entry_set_frame (plm4_entry*, physical_addr);
extern bool     plm4_entry_is_present (plm4_entry e);
extern bool     plm4_entry_is_user (plm4_entry);
extern bool     plm4_entry_is_4mb (plm4_entry);
extern bool     plm4_entry_is_writable (plm4_entry e);
extern physical_addr    plm4_entry_pfn (plm4_entry e);
extern void     plm4_entry_enable_global (plm4_entry e);


//! a page directery pointer entry
typedef uint64_t pdpe_entry;

extern void     pdpe_entry_add_attrib (pdpe_entry* e, uint64_t attrib);
extern void     pdpe_entry_del_attrib (pdpe_entry* e, uint64_t attrib);
extern void     pdpe_entry_set_frame (pdpe_entry*, physical_addr);
extern bool     pdpe_entry_is_present (pdpe_entry e);
extern bool     pdpe_entry_is_user (pdpe_entry);
extern bool     pdpe_entry_is_4mb (pdpe_entry);
extern bool     pdpe_entry_is_writable (pdpe_entry e);
extern physical_addr    pdpe_entry_pfn (pdpe_entry e);
extern void     pdpe_entry_enable_global (pdpe_entry e);

//! a page directery entry
typedef uint64_t pde_entry;

extern void     pde_entry_add_attrib (pde_entry* e, uint64_t attrib);
extern void     pde_entry_del_attrib (pde_entry* e, uint64_t attrib);
extern void     pde_entry_set_frame (pde_entry*, physical_addr);
extern bool     pde_entry_is_present (pde_entry e);
extern bool     pde_entry_is_user (pde_entry);
extern bool     pde_entry_is_4mb (pde_entry);
extern bool     pde_entry_is_writable (pde_entry e);
extern physical_addr    pde_entry_pfn (pde_entry e);
extern void     pde_entry_enable_global (pde_entry e);

//! a page directery entry
typedef uint64_t pte_entry;


extern void     pte_entry_add_attrib (pte_entry* e, uint64_t attrib);
extern void     pte_entry_del_attrib (pte_entry* e, uint64_t attrib);
extern void     pte_entry_set_frame (pte_entry*, physical_addr);
extern bool     pte_entry_is_present (pte_entry e);
extern bool     pte_entry_is_user (pte_entry);
extern bool     pte_entry_is_4mb (pte_entry);
extern bool     pte_entry_is_writable (pte_entry e);
extern physical_addr    pte_entry_pfn (pte_entry e);
extern void     pte_entry_enable_global (pte_entry e);

*******************************************************************************************************************/
