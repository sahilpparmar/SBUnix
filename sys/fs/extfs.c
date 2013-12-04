#include <sys/fs.h>
#include <defs.h>
#include <sys/kmalloc.h>
#include <sys/ahci.h>
#include <stdio.h>

static super_block* s_star = NULL;
static ext_inode* inode_e  = NULL;

// Debug Routine for printing inodes and their current sizes
void print_inodes()
{
    kprintf("\tInode BitMap => %p %p", s_star->s_inode_bmap[1], s_star->s_inode_bmap[0]);
    for (int i = 0; i < s_star->s_ninodes; i++) {
        if (read_inode(inode_e, i))
            kprintf("\t[%d]=>%d", i, inode_e->i_size);
    }
}

static void create_inode_structure(super_block *s_star)
{
    uint64_t vaddr  = VIRT_PAGE(1);
    ext_inode *inode_ptr = (ext_inode*) vaddr;
    int i;

    for (i = 0; i < INODES_PER_BLOCK; i++) {
        inode_ptr->i_size        = 0;
        inode_ptr->i_block_count = 0;
        inode_ptr++;
    }

    for (i = 0; i < s_star->s_ninodes/INODES_PER_BLOCK; i++) {
        write_sector((void*)vaddr, s_star->s_inodestart + i, 0, SIZE_OF_SECTOR);
    }
}

static void create_first_superblock(super_block *s_star) 
{
    // Inode Info
    s_star->s_ninodes          = MAX_NUM_INODES;
    s_star->s_freeinodescount  = MAX_NUM_INODES;
    s_star->s_inodestart       = INODE_START_SECTOR;
    s_star->s_inode_bmap[0]    = 0UL;
    s_star->s_inode_bmap[1]    = 0UL;

    // Blocks Info
    s_star->s_blockbitmapstart = BLOCK_BMAP_START_SECTOR;
    s_star->s_blockdatastart   = BLOCK_DATA_START_SECTOR;
    s_star->s_freeblockscount  = MAX_NUM_BLOCKS;
    s_star->s_max_fsize        = MAX_FILE_SIZE;
    s_star->s_magic            = SUPER_BLOCK_MAGIC;

    write_sector((void *)s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));
};

super_block *read_first_superblock(bool forceCreate) 
{
    s_star  = kmalloc(sizeof(super_block)); 
    inode_e = kmalloc(sizeof(ext_inode));

    read_sector(s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));

    if (forceCreate || s_star->s_magic != SUPER_BLOCK_MAGIC) {
        kprintf("\nNew Superblock Created");
        create_first_superblock(s_star); 
        create_inode_structure(s_star);
    } else {
        kprintf("\nSuperblock Found");
        kprintf("\nNumber of Used Inodes: %d", s_star->s_ninodes - s_star->s_freeinodescount);
    }
    kprintf("\nNumber of Free Inodes: %d", s_star->s_freeinodescount);

#if 0
    kprintf("%p\t", s_star->s_ninodes);
    kprintf("%p\t", s_star->s_freeblockscount);
    kprintf("%p\t", s_star->s_blockbitmapstart);
    kprintf("%p\t", s_star->s_inodestart);
    kprintf("%p\t", s_star->s_freeinodescount);
    kprintf("%p\t", s_star->s_max_fsize);
    kprintf("%p\t", s_star->s_magic);
#endif
    return s_star;
};

static bool inode_bmap_isset(uint64_t inode_no)
{
    return s_star->s_inode_bmap[inode_no / 64] & (1UL << (inode_no % 64));
}

static void inode_bmap_set(uint64_t inode_no)
{
    s_star->s_inode_bmap[inode_no / 64] |= (1UL << (inode_no % 64));
    s_star->s_freeinodescount--;
}

static void inode_bmap_unset(uint64_t inode_no)
{
    s_star->s_inode_bmap[inode_no / 64] &= ~ (1UL << (inode_no % 64));
    s_star->s_freeinodescount++;
}

static int inode_first_free()
{
    uint64_t i, j;

    if (s_star->s_freeinodescount == 0) {
        return -1;
    }

    for (i = 0; i < 2; i++) {
        if (s_star->s_inode_bmap[i] != 0xFFFFFFFFFFFFFFFFUL) {

            for (j = 0; j < 64; j++) {
                uint64_t bit = 1UL << j;

                if (!(s_star->s_inode_bmap[i] & bit)) {
                    return i*64 + j;
                }
            }
        }
    }
    return -1;
}

int32_t alloc_new_inode()
{
    int32_t inode_no;
    
    inode_no = inode_first_free();

    if (inode_no == -1) 
        return -1;

    inode_bmap_set(inode_no);

    // Update inode bitmap in super block
    write_sector((void *)s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));

    return inode_no;
};

void free_inode(int32_t inode_no)
{
    if (inode_no < 0 || inode_no >= s_star->s_ninodes || !inode_bmap_isset(inode_no)) {
        kprintf("\nAlready Free Inode %d", inode_no);
        return;
    }

    read_inode(inode_e, inode_no);
    //kprintf("\nFreeing Inode %d", inode_no);

    // Update inode
    inode_e->i_size        = 0;
    inode_e->i_block_count = 0;
    write_inode(inode_e, inode_no);

    inode_bmap_unset(inode_no);

    // Update inode bitmap in super block
    write_sector((void *)s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));
};

bool read_inode(ext_inode* inode_entry, uint64_t inode_no)
{
    uint64_t sec_no, sec_off;

    if (inode_no < 0 || inode_no >= s_star->s_ninodes || !inode_bmap_isset(inode_no)) {
        return FALSE;
    }
    sec_no = s_star->s_inodestart + inode_no/INODES_PER_BLOCK;
    sec_off = (inode_no % INODES_PER_BLOCK) * sizeof(ext_inode);

    read_sector(inode_entry, sec_no, sec_off, sizeof(ext_inode));

    return TRUE;
};

bool write_inode(ext_inode* inode_entry, uint64_t inode_no)
{
    uint64_t sec_no, sec_off;

    if (inode_no < 0 || inode_no >= s_star->s_ninodes || !inode_bmap_isset(inode_no)) {
        return FALSE;
    }
    
    sec_no  = s_star->s_inodestart + inode_no/INODES_PER_BLOCK;
    sec_off = (inode_no % INODES_PER_BLOCK) * sizeof(ext_inode);

    write_sector(inode_entry, sec_no, sec_off, sizeof(ext_inode));

    return TRUE;
};

