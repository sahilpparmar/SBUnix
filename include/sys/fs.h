#include <defs.h>
#define EXT_NAME_LEN 255
#define EXT_ROOT_INO 1

#define EXT_SUPER_MAGIC 0x137D

#define EXT_INODES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct ext_inode)))

typedef struct ext_inode {
    uint64_t i_size;
    uint64_t i_block_count;
    uint64_t i_isfree;
    uint16_t i_nlinks;
    uint64_t i_block[10]; // Number of Indirect Block
} ext_inode;

struct ext_free_inode {
    uint64_t count;
    uint64_t free[14];
    uint64_t next;
};

struct ext_free_block {
    uint64_t count;
    uint64_t free[254];
    uint64_t next;
};

typedef struct ext_super_block {
    uint64_t s_ninodes;
    uint64_t s_freeblockscount;
    uint64_t s_blockbitmapstart;
    uint64_t s_inode_start;         // Location at inode start
    uint64_t s_freeinodescount;     // No. of free inodes
    uint64_t s_max_fsize;           // Max File Size
    uint64_t s_magic;
} super_block;

struct ext_dir_entry {
    uint64_t inode;
    uint16_t rec_len;
    uint16_t name_len;
    char name[EXT_NAME_LEN];
};

void create_first_superblock();
super_block *read_first_superblock();

