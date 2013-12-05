#include <defs.h>
#include <stdio.h>
#include <sys/ahci.h>
#include <sys/fs.h>
#include <sys/virt_mm.h>
#include <sys/kmalloc.h>
#include <sys/kstring.h>
#define SECTOR_SIZE 512
#define BLOCK_ADDR_SIZE 8

/* Each File can be max of 320kb. Every file can consist of maximum of 10 blocks.
 *
 * Each block would be addressing 64 (8 byte address) addresses of 512 byte blocks. 
 *
 * Hence each block maps to 32kb i.e 64 * 512 (bytes) = 32kb. 
 */

//uint64_t get_sector1(uint64_t inode_num, uint64_t offset)
//{
//    uint64_t block_off, *sector_num = NULL, sector_offset;
//    char* block = kmalloc(512);
//    
//    ext_inode* inode_entry = kmalloc(sizeof(ext_inode));
//    
//    if(read_inode(inode_entry, inode_num) == 0) 
//        return -1;
//        
//    block_off  = offset/SECTOR_SIZE;
//    memset(block, 0, SECTOR_SIZE);
//    read_sector(block, inode_entry->i_block[block_off], 0, SECTOR_SIZE);
//    
//    sector_offset = (offset - (block_off * BLOCK_SIZE))/SECTOR_SIZE;
//    memcpy((void*)sector_num, block, 8);
//   
//    kprintf("Offset : %d", sector_offset);
//    //sector_num = (SECTOR_SIZE/BLOCK_ADDR_SIZE) * block_num + sector_offset;
//
//    return (uint64_t)sector_num;
//}
//
//
///*
// *  Read offset bytes from device from beginning.
// */
//void read_sector(uint64_t inode_num, uint64_t offset)
//{
//    uint64_t start_sector = 0, end_sector = 0;
//    uint64_t * vaddr = NULL;
//    int sec_iter = 0;
//
//    end_sector = get_sector(inode_num, offset);
//    
//    for ( sec_iter = start_sector; sec_iter <= end_sector; sec_iter++)
//    {
//        read_disk( vaddr, sec_iter);
//        vaddr = vaddr + SECTOR_SIZE;
//    }
//}
//
//
///* Each File can be max of 320kb. Every file can consist of maximum of 10 blocks.
// *
// * Each block would be addressing 64 (8 byte address) addresses of 512 byte blocks. 
// *
// * Hence each block maps to 32kb i.e 64 * 512 (bytes) = 32kb. 
// */
//
//uint64_t get_sector(uint64_t inode_num, uint64_t offset)
//{
//    uint64_t block_off;
//    
//    /* Create an inode for the given inode number. */
//    ext_inode* inode_entry = kmalloc(sizeof(ext_inode));
//    
//    if(read_inode(inode_entry, inode_num) == 0) 
//        return -1;
//    
//    /* Obtain block offset to be read from the inode */
//    block_off  = offset/SECTOR_SIZE;
//
//    return (uint64_t)(inode_entry->i_block[block_off]);
//}
//
void test_read()
{
    uint64_t ino = 0, ino1 = 0;//, ino2 = 0;
    ino = alloc_new_inode();
    ino1 = alloc_new_inode();
    //ino2 = alloc_new_inode();
    print_inodes();
    ext_inode* inode_entry = kmalloc(sizeof(ext_inode));
    ext_inode* inode_entry1 = kmalloc(sizeof(ext_inode));
    //ext_inode* inode_entry2 = kmalloc(sizeof(ext_inode));
    //kprintf("Step 1: inode num = %d", ino);
    inode_entry->i_block[0] = 100;
    inode_entry1->i_block[0] = 120;
    //inode_entry2->i_block[0] = 140;
    
    char *a = "/Disk/bin/hello.txt";
    char *b = "/Disk/bin/abc.txt";
    //char *c = "/Disk/lib/world.txt";
    
    memcpy(inode_entry->i_name, a, kstrlen(a));
    memcpy(inode_entry1->i_name, b,kstrlen(b));
    //memcpy(inode_entry2->i_name,c , kstrlen(c));
    
    if(write_inode(inode_entry, ino) == 0){
        kprintf("Step 3.99999999999:");
        return;
    }
    if(write_inode(inode_entry1, ino1) == 0){
        kprintf("Step 3.99999999999:");
        return;
    }
    
    //kprintf("\n Inode nos: %d, %d, %d\n", ino, ino1, ino2);
    //kprintf("\ninode %d, %d", ino, ino1); 
    /*
    uint64_t *temp = kmalloc(512);  
    uint64_t *temp1 = kmalloc(512);  
    memset8(temp, 1000, 64);
    temp[32] = 2000;
    kprintf("Step 2:");
    write_sector(temp, 100, 0, 512);
    memset(temp1, 5, 512);
    kprintf("Step 3:");

    write_sector(temp1, 2000, 0, 512);
    kprintf("Step 4:");
//while(1);
    kprintf("Sector Number : ", get_sector(ino, 16400));
    
    char *temp = kmalloc(512);  
    //char *temp1 = kmalloc(512);  
    //char *temp2 = kmalloc(512);  
    //memset(temp, 'a', 512);
    //memset(temp1, 'b', 512);
    //memset(temp2, 'c', 512);
    //write_block(temp, 10);
    //write_block(temp1, 20);
    //write_block(temp2, 30);
    read_block(temp,20);
    //read_block(temp1,20);
    //read_block(temp2,10);
    //kprintf("Init Value : %s", temp);
    kprintf("Value : %s", temp);
    //kprintf("Value : %s", temp1);
    //kprintf("Value : %s", temp2);*/
}
