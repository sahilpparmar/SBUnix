#include <defs.h>

#define INODES_PER_BLOCK 4
#define SUPER_BLOCK_SECTOR 1
#define AHCI_KERN_BASE 0xFFFFFFFF00000000
#define AHCI_PHYS_BASE 0x800000
#define AHCI_VIRT_BASE 0xFFFFFFFF00800000
#define PHYS_PAGE(a) (AHCI_PHYS_BASE + (a * 0x1000))
#define VIRT_PAGE(a) (AHCI_VIRT_BASE + (a * 0x1000))
#define SIZE_OF_SECTOR 512


typedef struct ext_inode {
    uint64_t i_size;
    uint64_t i_block_count;
    uint64_t i_isfree;
    uint64_t i_block[10];           // Number of Indirect Block
} ext_inode;

typedef struct ext_super_block {
    uint64_t s_ninodes;
    uint64_t s_freeblockscount;
    uint64_t s_blockbitmapstart;
    uint64_t s_inode_start;         // Location at inode start
    uint64_t s_freeinodescount;     // No. of free inodes
    uint64_t s_max_fsize;           // Max File Size
    uint64_t s_magic;
} super_block;

super_block *read_first_superblock(bool forceCreate);
bool read_inode(ext_inode* inode_entry, uint64_t inode_no);
void print_inodes();

