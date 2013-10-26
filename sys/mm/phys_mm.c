#include <defs.h>
#include <sys/phys_mm.h>
#include <stdio.h>
#include <screen.h>

#define PMMNGR_BLOCKS_PER_BYTE 8
#define PMMNGR_BLOCK_SIZE 4096
#define PMMNGR_BLOCK_ALIGN PMMNGR_BLOCK_SIZE

static uint64_t _mmngr_memory_size;
static uint64_t _mmngr_used_blocks;
static uint64_t _mmngr_max_blocks;
static uint64_t* _mmngr_memory_map;
static uint64_t _mmngr_base_addr;

// Currently this works for 128(8192*64*4k) MB RAM.
// Need to design a better way to dynamically allocate the bitmap!
static uint64_t bitmap_t[8192];

static void mmap_set(int bit)
{
    _mmngr_memory_map[bit / 64] |= (1 << (bit % 64));
}

static void mmap_unset(int bit)
{
    _mmngr_memory_map[bit / 64] &= ~ (1 << (bit % 64));
}

static uint64_t pmmngr_get_block_count () {

    return _mmngr_max_blocks;
}

static uint64_t pmmngr_get_free_block_count () {

    return _mmngr_max_blocks - _mmngr_used_blocks;
}


// Uncomment below functions when we actually use them
#if 0
static int mmap_test(int bit)
{
    return _mmngr_memory_map[bit / 64] &  (1 << (bit % 64));
}

static uint64_t  pmmngr_get_memory_size () {
    return _mmngr_memory_size;
}


static uint64_t pmmngr_get_use_block_count () {

    return _mmngr_used_blocks;
}

static uint64_t pmmngr_get_block_size () {

    return PMMNGR_BLOCK_SIZE;
}
#endif

static int mmap_first_free () 
{
    uint64_t i;
    int j;

    for (i=0; i< pmmngr_get_block_count() /64; i++) {
        if (_mmngr_memory_map[i] != 0xFFFFFFFFFFFFFFFF) {

            for (j=0; j<64; j++) {              //! test each bit in the dword

                int bit = 1 << j;
                if (! (_mmngr_memory_map[i] & bit) ) {
                    return i*64 + j;
                }
            }
        }
    }
    return -1;
}

void pmmngr_init (uint64_t physSize, uint64_t physBase) {

    _mmngr_memory_size = physSize;
    _mmngr_memory_map  = bitmap_t;
    _mmngr_max_blocks  = (physSize*1024) / PMMNGR_BLOCK_SIZE;
    _mmngr_used_blocks = 0;
    _mmngr_base_addr   = physBase;

    //set_cursor_pos(18, 5);
    //printf("Bimap addr = %p", bitmap_t);
    
    memset(_mmngr_memory_map, 0x0, pmmngr_get_block_count() / PMMNGR_BLOCKS_PER_BYTE );
}

uint64_t pmmngr_alloc_block () {

    uint64_t addr = 0;
    int frame = -1;

    if (pmmngr_get_free_block_count() <= 0)
        return 0;   //out of memory

    frame = mmap_first_free();
    if (frame == -1)
        return 0;   //out of memory

    mmap_set(frame);

    addr = _mmngr_base_addr + (frame * PMMNGR_BLOCK_SIZE);
    _mmngr_used_blocks++;
    
    return addr;
}

void pmmngr_free_block (uint64_t addr) {

    int frame = (addr - _mmngr_base_addr) / PMMNGR_BLOCK_SIZE;

    mmap_unset(frame);

    _mmngr_used_blocks--;
}
