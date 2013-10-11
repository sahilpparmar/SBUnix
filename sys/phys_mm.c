#include <defs.h>
#include <sys/phys_mm.h>
#include <stdio.h>

#define PMMNGR_BLOCKS_PER_BYTE 8
#define PMMNGR_BLOCK_SIZE 4096
#define PMMNGR_BLOCK_ALIGN PMMNGR_BLOCK_SIZE

static uint64_t _mmngr_memory_size = 0;
static uint64_t _mmngr_used_blocks = 0;
static uint64_t _mmngr_max_blocks = 0;
static uint64_t* _mmngr_memory_map = 0;
static uint64_t _mmngr_base_addr;

void mmap_set (int bit)
{
    _mmngr_memory_map[bit / 64] |= (1 << (bit % 64));
}

void mmap_unset (int bit)
{
    _mmngr_memory_map[bit / 64] &= ~ (1 << (bit % 64));
}

int mmap_test (int bit)
{
    return _mmngr_memory_map[bit / 64] &  (1 << (bit % 64));
}

int mmap_first_free () 
{
    uint64_t i;
    int j;

    for (i=0; i< pmmngr_get_block_count() /64; i++) {
        if (_mmngr_memory_map[i] != 0xFFFFFFFFFFFFFFFF) {

            for (j=0; j<64; j++) {              //! test each bit in the dword

                int bit = 1 << j;
                if (! (_mmngr_memory_map[i] & bit) ) {
                    return i*8*8+j;
                }
            }
        }
    }
    return -1;
}

void pmmngr_init (uint64_t memSize, physical_addr* bitmap, uint64_t base) {

    _mmngr_memory_size = memSize;
    _mmngr_memory_map  = bitmap;
    _mmngr_max_blocks  = (pmmngr_get_memory_size()*1024) / PMMNGR_BLOCK_SIZE;
    _mmngr_used_blocks = 0;
    _mmngr_base_addr   = base;
    
    memset (_mmngr_memory_map, 0x00000000, pmmngr_get_block_count() / PMMNGR_BLOCKS_PER_BYTE );
}


physical_addr pmmngr_alloc_block () {

    if (pmmngr_get_free_block_count() <= 0)
        return 0;   //out of memory

    int frame = mmap_first_free ();
    
    if (frame == -1)
        return 0;   //out of memory

    mmap_set (frame);

    physical_addr addr;
    addr = _mmngr_base_addr + frame * PMMNGR_BLOCK_SIZE;
    _mmngr_used_blocks++;
    
    //printf("..%p..",addr);
    return addr;
}

void pmmngr_free_block (uint64_t p) {

    physical_addr addr;
    addr = (physical_addr)p;
    int frame = (addr - _mmngr_base_addr) / PMMNGR_BLOCK_SIZE;

    mmap_unset (frame);

    _mmngr_used_blocks--;
}


uint64_t  pmmngr_get_memory_size () {

    return _mmngr_memory_size;
}

uint64_t pmmngr_get_block_count () {

    return _mmngr_max_blocks;
}

uint64_t pmmngr_get_use_block_count () {

    return _mmngr_used_blocks;
}

uint64_t pmmngr_get_free_block_count () {

    return _mmngr_max_blocks - _mmngr_used_blocks;
}

uint64_t pmmngr_get_block_size () {

    return PMMNGR_BLOCK_SIZE;
}


