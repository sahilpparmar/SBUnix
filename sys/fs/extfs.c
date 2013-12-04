#include <sys/fs.h>
#include <defs.h>
#include <sys/kmalloc.h>
#include <sys/ahci.h>
#include <stdio.h>

static super_block* s_star = NULL;

static void create_inode_structure(super_block *s_star)
{
    uint64_t vaddr  = VIRT_PAGE(1);
    ext_inode *inode_ptr = (ext_inode*) vaddr;
    int i;

    for (i = 0; i < INODES_PER_BLOCK; i++) {
        inode_ptr->i_size        = 0;
        inode_ptr->i_block_count = 0;
        inode_ptr->i_isfree      = 1;

        inode_ptr++;
    }

    for (i = 0; i < s_star->s_ninodes/INODES_PER_BLOCK; i++) {
        write_sector((void*)vaddr, s_star->s_inode_start + i, 0, SIZE_OF_SECTOR);
    }
}

// Debug Routine for printing inodes and their current sizes
void print_inodes() {
    ext_inode* inode_entry = kmalloc(sizeof(ext_inode));
    for (int i = 0; i < s_star->s_ninodes; i++) {
        if (read_inode(inode_entry, i))
            kprintf("\t[%d]=>%d", i, inode_entry->i_size);
    }
}

static void create_first_superblock(super_block *s_star) 
{
    s_star->s_ninodes           = 100;
    s_star->s_inode_start       = 2;
    s_star->s_freeinodescount   = 100;
    s_star->s_blockbitmapstart  = 10;       // Need to change
    s_star->s_freeblockscount   = 70000;
    s_star->s_max_fsize         = 327680;   // 320 KB
    s_star->s_magic             = 0xFEDCBA1234567890;

    write_sector((void *)s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));
};

super_block *read_first_superblock(bool forceCreate) 
{
    s_star = kmalloc(sizeof(super_block)); 

    read_sector(s_star, SUPER_BLOCK_SECTOR, 0, sizeof(super_block));

    if (forceCreate || s_star->s_magic != 0xFEDCBA1234567890) {
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
    kprintf("%p\t", s_star->s_inode_start);
    kprintf("%p\t", s_star->s_freeinodescount);
    kprintf("%p\t", s_star->s_max_fsize);
    kprintf("%p\t", s_star->s_magic);
#endif
    return s_star;
};

bool read_inode(ext_inode* inode_entry, uint64_t inode_no)
{
    uint64_t sec_no, sec_off;

    if (inode_no < 0 || inode_no >= s_star->s_ninodes) {
        return FALSE;
    }
    
    sec_no = s_star->s_inode_start + inode_no/INODES_PER_BLOCK;
    sec_off = (inode_no % INODES_PER_BLOCK) * sizeof(ext_inode);

    read_sector(inode_entry, sec_no, sec_off, sizeof(ext_inode));

#if 0
    kprintf("\t%p", inode_entry->i_size);
    kprintf("\t%p", inode_entry->i_block_count);
    kprintf("\t%p", inode_entry->i_isfree);
    kprintf("\t%p", inode_entry->i_block[0]);
#endif

    return TRUE;
};

