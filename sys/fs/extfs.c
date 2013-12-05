#include <sys/fs.h>
#include <defs.h>
#include <sys/kmalloc.h>
#include <sys/ahci.h>
#include <stdio.h>

static super_block* s_star = NULL;
static ext_inode* inode_e  = NULL;
static uint64_t *sector_e  = NULL;

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
    int i;

    for (i = 0; i < s_star->s_ninodes/INODES_PER_BLOCK; i++) {
        write_sector((void*)sector_e, s_star->s_inodestart + i, 0, SIZE_OF_SECTOR);
    }
}

static void create_block_bmap(super_block *s_star)
{
    int i;

    for (i = 0; i < NUM_BMAP_SECTORS; i++) {
        write_sector((void*)sector_e, s_star->s_blockbmapstart + i, 0, SIZE_OF_SECTOR);
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
    s_star->s_nblocks          = MAX_NUM_BLOCKS;
    s_star->s_freeblockscount  = MAX_NUM_BLOCKS;
    s_star->s_blockbmapstart   = BLOCK_BMAP_START_SECTOR;
    s_star->s_blockdatastart   = BLOCK_DATA_START_SECTOR;
    s_star->s_max_fsize        = MAX_FILE_SIZE;
    s_star->s_magic            = SUPER_BLOCK_MAGIC;

    write_sector((void *)s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));
};

super_block *read_first_superblock(bool forceCreate) 
{
    s_star   = kmalloc(sizeof(super_block)); 
    inode_e  = kmalloc(sizeof(ext_inode));
    sector_e = kmalloc(sizeof(SIZE_OF_SECTOR));

    read_sector(s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));

    if (forceCreate || s_star->s_magic != SUPER_BLOCK_MAGIC) {
        kprintf("\nNew Superblock Created");
        create_first_superblock(s_star); 

        memset((void*)sector_e, 0, SIZE_OF_SECTOR);
        create_inode_structure(s_star);
        create_block_bmap(s_star);
        kprintf("\nNumber of Free Inodes: %d", s_star->s_freeinodescount);
        kprintf("\nNumber of Free Blocks: %d", s_star->s_freeblockscount);
    } else {
        kprintf("\nSuperblock Found");
        kprintf("\nNumber of Used Inodes: %d", s_star->s_ninodes - s_star->s_freeinodescount);
        kprintf("\nNumber of Free Inodes: %d", s_star->s_freeinodescount);
        kprintf("\nNumber of Used Blocks: %d", s_star->s_nblocks - s_star->s_freeblockscount);
        kprintf("\nNumber of Free Blocks: %d", s_star->s_freeblockscount);
    }

#if 0
    if (read_inode(inode_e, 0)) {
        kprintf("\nSTR: %s", inode_e->i_name);
        kprintf("\nSiz: %d", inode_e->i_size);
        kprintf("\nSTR: %d", inode_e->i_block_count);
        kprintf("\nSTR: %d", inode_e->i_block[0]);

        if (read_block(sector_e, 0, 0, 512)) {
            char *str = (char*) sector_e;
            kprintf("\nSTR: %s", str);
        }
    }

    kprintf("%p\t", s_star->s_ninodes);
    kprintf("%p\t", s_star->s_freeblockscount);
    kprintf("%p\t", s_star->s_blockbmapstart);
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

    // Update inode bitmap in super block
    write_sector((void *)s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));
}

static void inode_bmap_unset(uint64_t inode_no)
{
    s_star->s_inode_bmap[inode_no / 64] &= ~ (1UL << (inode_no % 64));
    s_star->s_freeinodescount++;

    // Update inode bitmap in super block
    write_sector((void *)s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));
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

    return inode_no;
};

void free_inode(int32_t inode_no)
{
    if (inode_no < 0 || inode_no >= s_star->s_ninodes || !inode_bmap_isset(inode_no)) {
        //kprintf("\nAlready Free Inode %d", inode_no);
        return;
    }

    read_inode(inode_e, inode_no);
    //kprintf("\nFreeing Inode %d", inode_no);

    // Update inode
    inode_e->i_size        = 0;
    inode_e->i_block_count = 0;
    write_inode(inode_e, inode_no);

    inode_bmap_unset(inode_no);

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

    //kprintf("\nI[%d]%s, %p, %p", inode_no, inode_entry->i_name, inode_entry->i_size, inode_entry->i_block_count);

    write_sector(inode_entry, sec_no, sec_off, sizeof(ext_inode));

    return TRUE;
};

//////////////////////////////////////////////////////////////////

static bool block_bmap_isset(uint64_t *sector_t, uint64_t block_no)
{
    return sector_t[block_no / 64] & (1UL << (block_no % 64));
}

static void block_bmap_set(uint64_t *sector_t, uint64_t block_no)
{
    sector_t[block_no / 64] |= (1UL << (block_no % 64));
    s_star->s_freeblockscount--;

    // Update inode bitmap in super block
    write_sector((void *)s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));
}

static void block_bmap_unset(uint64_t *sector_t, uint64_t block_no)
{
    sector_t[block_no / 64] &= ~ (1UL << (block_no % 64));
    s_star->s_freeblockscount++;

    // Update inode bitmap in super block
    write_sector((void *)s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));
}

static int block_first_free()
{
    uint64_t i, j, k;

    if (s_star->s_freeblockscount == 0) {
        return -1;
    }

    for (i = 0; i < NUM_BMAP_SECTORS; i++) {
        read_sector((void*)sector_e, i + s_star->s_blockbmapstart, 0, SIZE_OF_SECTOR);

        for (j = 0; j < SIZE_OF_SECTOR/8; j++) {
            if (sector_e[j] != 0xFFFFFFFFFFFFFFFFUL) {

                for (k = 0; k < 64; k++) {
                    uint64_t bit = 1UL << k;

                    if (!(sector_e[j] & bit)) {
                        return i*4096 + j*64 + k;
                    }
                }
            }
        }
    }
    return -1;
}

int32_t alloc_new_block()
{
    int32_t block_no, sec_no;
    
    block_no = block_first_free();

    if (block_no == -1) 
        return -1;

    sec_no = s_star->s_blockbmapstart + block_no/4096;
    
    // Update block bitmap
    read_sector((void*)sector_e, sec_no, 0, SIZE_OF_SECTOR);
    block_bmap_set(sector_e, block_no);
    write_sector((void*)sector_e, sec_no, 0, SIZE_OF_SECTOR);

    return block_no;
}

void free_block(int32_t block_no)
{
    int32_t sec_no;

    if (block_no < 0 || block_no >= s_star->s_nblocks) {
        //kprintf("\nAlready Free Block %d", block_no);
        return;
    }

    sec_no = s_star->s_blockbmapstart + block_no/4096;
    
    // Check if block is actually used
    read_sector((void*)sector_e, sec_no, 0, SIZE_OF_SECTOR);
    if (!block_bmap_isset(sector_e, block_no))
        return;

    //kprintf("\nFreeing Block %d", block_no);

    // Update block bitmap
    block_bmap_unset(sector_e, block_no);
    write_sector((void*)sector_e, sec_no, 0, SIZE_OF_SECTOR);
}

bool read_block(void* block_entry, uint64_t block_no, uint64_t block_off, uint64_t size)
{
    if (block_no < 0 || block_no >= s_star->s_nblocks) {
        return FALSE;
    }

    read_sector(block_entry, s_star->s_blockdatastart + block_no, block_off, size);

    return TRUE;
}

bool write_block(void* block_entry, uint64_t block_no, uint64_t block_off, uint64_t size)
{
    if (block_no < 0 || block_no >= s_star->s_nblocks) {
        return FALSE;
    }
    //kprintf("\n[WRITE]%s", block_entry, block_no, size);
    write_sector(block_entry, s_star->s_blockdatastart + block_no, block_off, size);

    return TRUE;
}

void copy_blocks_to_vma(ext_inode* inode_entry, uint64_t vma_start)
{
    uint64_t size = inode_entry->i_size;
    int i;

    for (i = 0; i < inode_entry->i_block_count; i++) {
        if (size < SIZE_OF_SECTOR) {
            read_block((void*) vma_start, inode_entry->i_block[i], 0, size);
        } else {
            read_block((void*) vma_start, inode_entry->i_block[i], 0, SIZE_OF_SECTOR);
        }
        vma_start = vma_start + SIZE_OF_SECTOR;
        size = size - SIZE_OF_SECTOR;
    }
}

void copy_vma_to_blocks(ext_inode* inode_entry, int32_t inode_no, uint64_t vma_start, uint64_t new_size)
{
    uint64_t new_block_count = new_size/(SIZE_OF_SECTOR+1) + 1;
    uint64_t cur_block_count = inode_entry->i_block_count;
    int32_t i;

    inode_entry->i_size = new_size;
    inode_entry->i_block_count = new_block_count;

    if (cur_block_count == new_block_count) {
        // Directly Copy all the contains; no need to alloc/dealloc blocks
        for (i = 0; i < new_block_count; i++) {

            if (new_size < SIZE_OF_SECTOR) {
                write_block((void*) vma_start, inode_entry->i_block[i], 0, new_size);
            } else {
                write_block((void*) vma_start, inode_entry->i_block[i], 0, SIZE_OF_SECTOR);
            }
            vma_start = vma_start + SIZE_OF_SECTOR;
            new_size = new_size - SIZE_OF_SECTOR;
        }

    } else if (cur_block_count > new_block_count) {
        // Copy all the contains and dealloc extra blocks

        for (i = 0; i < new_block_count; i++) {

            if (new_size < SIZE_OF_SECTOR) {
                write_block((void*) vma_start, inode_entry->i_block[i], 0, new_size);
            } else {
                write_block((void*) vma_start, inode_entry->i_block[i], 0, SIZE_OF_SECTOR);
            }
            vma_start = vma_start + SIZE_OF_SECTOR;
            new_size = new_size - SIZE_OF_SECTOR;
        }

        // Free unused blocks
        for (i = new_block_count; i < cur_block_count; i++) {
            free_block(inode_entry->i_block[i]);
        }
    
    } else if (cur_block_count < new_block_count) {
        // Copy all the contains and alloc extra blocks

        for (i = 0; i < cur_block_count; i++) {
            write_block((void*) vma_start, inode_entry->i_block[i], 0, SIZE_OF_SECTOR);
            vma_start = vma_start + SIZE_OF_SECTOR;
            new_size = new_size - SIZE_OF_SECTOR;
        }

        // Alloc new blocks
        for (i = cur_block_count; i < new_block_count; i++) {
            inode_entry->i_block[i] = alloc_new_block();

            if (new_size < SIZE_OF_SECTOR) {
                write_block((void*) vma_start, inode_entry->i_block[i], 0, new_size);
            } else {
                write_block((void*) vma_start, inode_entry->i_block[i], 0, SIZE_OF_SECTOR);
            }
            vma_start = vma_start + SIZE_OF_SECTOR;
            new_size = new_size - SIZE_OF_SECTOR;
        }
    }
    write_inode(inode_entry, inode_no);
}

