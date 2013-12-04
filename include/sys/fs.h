#include <defs.h>

#define AHCI_KERN_BASE 0xFFFFFFFF00000000
#define AHCI_PHYS_BASE 0x800000
#define AHCI_VIRT_BASE 0xFFFFFFFF00800000
#define PHYS_PAGE(a) (AHCI_PHYS_BASE + (a * 0x1000))
#define VIRT_PAGE(a) (AHCI_VIRT_BASE + (a * 0x1000))

#define SIZE_OF_SECTOR          512
#define MAX_NUM_INODES          100
#define NUM_DIRECT_BLOCKS       14
#define MAX_FILE_SIZE           7168    // NUM_DIRECT_BLOCKS * SIZE_OF_SECTORS
#define MAX_NUM_BLOCKS          1600    // > MAX_FILE_SIZE * MAX_NUM_INODES
#define INODES_PER_BLOCK        1       // sizeof(INODE)/SIZE_OF_SECTORS
#define SUPER_BLOCK_SECTOR      1
#define INODE_START_SECTOR      2       // SUPER_BLOCK_SECTOR + 1
#define NUM_BMAP_SECTORS        1       // MAX_NUM_BLOCKS/4096 
#define BLOCK_BMAP_START_SECTOR 102     // INODE_START_SECTOR + (MAX_NUM_INODES/INODES_PER_BLOCK)
#define BLOCK_DATA_START_SECTOR 103     // BLOCK_BMAP_START_SECTOR + NUM_BMAP_SECTORS
#define SUPER_BLOCK_MAGIC       0xFEDCBA1234567890

typedef struct ext_inode {
    char     i_name[128];               // Name of file
    uint64_t i_size;                    // Size of file
    uint64_t i_block_count;             // Number of blocks used
    uint64_t i_block[NUM_DIRECT_BLOCKS];// Pointers to Direct Block
} ext_inode;

typedef struct ext_super_block {
    uint64_t s_inodestart;              // Starting Sector No. at inode start
    uint64_t s_ninodes;                 // MAX number of inodes
    uint64_t s_freeinodescount;         // No. of free inodes
    uint64_t s_inode_bmap[2];           // Inode bitmap
    uint64_t s_nblocks;                 // Max number of blocks
    uint64_t s_blockbmapstart;          // Starting Sector No. of block bitmap
    uint64_t s_freeblockscount;         // No. of free blocks
    uint64_t s_blockdatastart;          // Starting Sector No. of block data
    uint64_t s_max_fsize;               // Max File Size
    uint64_t s_magic;                   // Magic Bits
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
bool read_block(void* block_entry, uint64_t block_no, uint64_t block_off, uint64_t size);
bool write_block(void* block_entry, uint64_t block_no, uint64_t block_off, uint64_t size);

void read_sector(void* read_addr, uint64_t sector_no, uint64_t sec_off, uint64_t size);
void write_sector(void* write_addr, uint64_t sector_no, uint64_t sec_off, uint64_t size);

void copy_blocks_to_vma(ext_inode* inode_entry, uint64_t vma_start);
void copy_vma_to_blocks(ext_inode* inode_entry, int32_t inode_no, uint64_t vma_start, uint64_t new_size);

