#include <sys/fs.h>
#include <defs.h>
#include <sys/kmalloc.h>
#include <sys/ahci.h>
#include <stdio.h>

void create_first_superblock(super_block *s_star) 
{
    s_star->s_ninodes           = 100;
    s_star->s_inode_start       = 2;
    s_star->s_freeinodescount   = 1000;
    s_star->s_blockbitmapstart  = 10;       // Need to change
    s_star->s_freeblockscount   = 70000;
    s_star->s_max_fsize         = 327680;   // 320 KB
    s_star->s_magic             = 0xFEDCBA1234567890;

    write_disk((void *)s_star, 1, sizeof(super_block));
};

super_block *read_first_superblock() 
{
    super_block* s_star = kmalloc(sizeof(super_block)); 

    read_disk(s_star, 1);

    if (s_star->s_magic != 0xFEDCBA1234567890) {
        kprintf("\nNew Superblock Created");
        create_first_superblock(s_star); 
    }
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
