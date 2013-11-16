#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <sys/phys_mm.h>

#define PHYS_BLOCK_SIZE 4096 
#define PHYS_BLOCK_ALIGN PHYS_BLOCK_SIZE
#define PAGE_2ALIGN 12     // 2 ^ PAGE_2ALIGN = PHYS_BLOCK_ALIGN

static uint64_t _mmngr_memory_size;
static uint64_t _mmngr_used_blocks;
static uint64_t _mmngr_max_blocks;
static uint64_t* _mmngr_memory_map;
static uint64_t _mmngr_base_addr;

// Currently this works for 128(512*64*4k) MB RAM.
// Need to design a better way to dynamically allocate the bitmap!
static uint64_t bitmap_t[512];

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

// Uncomment below functions when we actually use them
#if 0
static int mmap_test(int bit)
{
    return _mmngr_memory_map[bit / 64] & (1UL << (bit % 64));
}

static uint64_t  phys_get_memory_size() {
    return _mmngr_memory_size;
}

static uint64_t phys_get_use_block_count() {

    return _mmngr_used_blocks;
}

static uint64_t phys_get_block_size() {

    return PHYS_BLOCK_SIZE;
}
#endif

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

void phys_init(uint64_t physBase, uint64_t physSize) {

    // Start Physical Pages from 4MB
    _mmngr_base_addr   = physBase + 0x300000UL;
    _mmngr_memory_size = physSize - 0x300000UL;                   
    _mmngr_max_blocks  = _mmngr_memory_size >> PAGE_2ALIGN;     
    _mmngr_used_blocks = 0;

    // Set all physical memory to 0
    memset8((void*)_mmngr_base_addr, 0x0, _mmngr_memory_size/8);

    kprintf("\nPhysical Blocks Base:%p, Size:%p, Max:%p", _mmngr_base_addr, _mmngr_memory_size, _mmngr_max_blocks);

    // Set Bitmap to all 0
    _mmngr_memory_map  = bitmap_t;
    memset8((void*)_mmngr_memory_map, 0x0, sizeof(bitmap_t)/8);
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
    
    // kprintf("\tNewPaddr: %p", paddr);
    return paddr;
}

void phys_free_block(uint64_t addr) {

    int frame = (addr - _mmngr_base_addr) >> PAGE_2ALIGN;

    mmap_unset(frame);

    _mmngr_used_blocks--;
}

