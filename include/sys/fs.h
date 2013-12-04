#include <defs.h>

#define AHCI_KERN_BASE 0xFFFFFFFF00000000
#define AHCI_PHYS_BASE 0x800000
#define AHCI_VIRT_BASE 0xFFFFFFFF00800000
#define PHYS_PAGE(a) (AHCI_PHYS_BASE + (a * 0x1000))
#define VIRT_PAGE(a) (AHCI_VIRT_BASE + (a * 0x1000))

#define SIZE_OF_SECTOR          512
#define MAX_NUM_INODES          100
#define MAX_NUM_BLOCKS          70000
#define NUM_BMAP_SECTORS        18
#define INODES_PER_BLOCK        4
#define SUPER_BLOCK_SECTOR      1
#define INODE_START_SECTOR      2
#define BLOCK_BMAP_START_SECTOR 27
#define BLOCK_DATA_START_SECTOR 50
#define MAX_FILE_SIZE           327680  // 320 KB file
#define SUPER_BLOCK_MAGIC       0xFEDCBA1234567890

typedef struct ext_inode {
    uint64_t i_size;
    uint64_t i_block_count;
    uint64_t i_block[10];           // Number of Indirect Block
} ext_inode;

typedef struct ext_super_block {
    uint64_t s_inodestart;          // Location at inode start
    uint64_t s_ninodes;
    uint64_t s_freeinodescount;     // No. of free inodes
    uint64_t s_inode_bmap[2];
    uint64_t s_nblocks;
    uint64_t s_blockbmapstart;
    uint64_t s_freeblockscount;
    uint64_t s_blockdatastart;
    uint64_t s_max_fsize;           // Max File Size
    uint64_t s_magic;
} super_block;

void init_disk(bool forceCreate);
super_block *read_first_superblock(bool forceCreate);

void print_inodes();
int32_t alloc_new_inode();
void free_inode(int32_t inode_no);
bool read_inode(ext_inode* inode_entry, uint64_t inode_no);
bool write_inode(ext_inode* inode_entry, uint64_t inode_no);

int32_t alloc_new_block();
void free_block(int32_t block_no);
bool read_block(void* block_entry, uint64_t block_no);
bool write_block(void* block_entry, uint64_t block_no);

void read_sector(void* read_addr, uint64_t sector_no, uint64_t sec_off, uint64_t size);
void write_sector(void* write_addr, uint64_t sector_no, uint64_t sec_off, uint64_t size);

