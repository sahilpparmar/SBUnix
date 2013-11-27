#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <sys/types.h>
#include <sys/virt_mm.h>
#include <io_common.h>

static uint64_t _mmngr_memory_size;
static uint64_t _mmngr_used_blocks;
static uint64_t _mmngr_max_blocks;
static uint64_t* _mmngr_memory_map;
static uint64_t _mmngr_base_addr;

static void mmap_set(int bit)
{
    _mmngr_memory_map[bit / 64] |= (1UL << (bit % 64));
}

static void mmap_unset(int bit)
{
    _mmngr_memory_map[bit / 64] &= ~ (1UL << (bit % 64));
}

static uint64_t phys_get_block_count() 
{
    return _mmngr_max_blocks;
}

uint64_t phys_get_free_block_count()
{
    return _mmngr_max_blocks - _mmngr_used_blocks;
}

static int mmap_first_free() 
{
    uint64_t i, j;

    for (i = 0; i < phys_get_block_count()/64; i++) {
        if (_mmngr_memory_map[i] != 0xFFFFFFFFFFFFFFFFUL) {

            for (j = 0; j < 64; j++) {              // test each bit in the qword
                uint64_t bit = 1UL << j;

                if (!(_mmngr_memory_map[i] & bit)) {
                    return i*64 + j;
                }
            }
        }
    }
    return -1;
}

void phys_init(uint64_t physBase, uint64_t physfree, uint64_t physSize) {

    uint64_t bitmap_t;
    
    // Start Physical Pages from 4MB
    _mmngr_base_addr   = physBase + 0x300000UL;
    _mmngr_memory_size = physSize - 0x300000UL;                   
    _mmngr_max_blocks  = _mmngr_memory_size >> PAGE_2ALIGN;     
    _mmngr_used_blocks = 0;

    // Set all physical memory to 0
    memset8((uint64_t *)_mmngr_base_addr, 0x0, _mmngr_memory_size/8);

    kprintf("\nPhysical Blocks Base:%p, Size:%p, Max:%p", _mmngr_base_addr, _mmngr_memory_size, _mmngr_max_blocks);

    // Set Bitmap to all 0
    bitmap_t = _mmngr_max_blocks/64 + 1;
    _mmngr_memory_map = (uint64_t *) (KERNEL_START_VADDR + physfree);
    memset8((uint64_t *)_mmngr_memory_map, 0x0, bitmap_t);
}

uint64_t phys_alloc_block() {

    uint64_t paddr = NULL;
    int frame = -1;

    if (phys_get_free_block_count() <= 0)
        return 0;   //out of memory

    frame = mmap_first_free();
    if (frame == -1)
        return 0;   //out of memory

    mmap_set(frame);

    paddr = _mmngr_base_addr + (frame << PAGE_2ALIGN);
    _mmngr_used_blocks++;
    
    //kprintf("\tNewPaddr: %p", paddr);
    return paddr;
}

void phys_free_block(uint64_t addr) {

    int frame = (addr - _mmngr_base_addr) >> PAGE_2ALIGN;

    if (addr < _mmngr_base_addr || addr > (_mmngr_base_addr + _mmngr_memory_size)) {
        kprintf("\tFreePaddr: %p", addr);
        panic("Trying to Free out of range Physical Block");
    }

    zero_out_phys_block(addr);

    mmap_unset(frame);

    _mmngr_used_blocks--;
}

