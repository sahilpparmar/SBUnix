#ifndef _MM_TYPES_H
#define _MM_TYPES_H

typedef struct vm_area_struct vma_struct;
typedef struct mm_struct mm_struct;

struct vm_area_struct {
    mm_struct *vm_mm;               // The address space we belong to.
    uint64_t vm_start;              // Our start address within vm_mm
    uint64_t vm_end;                // The first byte after our end address within vm_mm
    vma_struct *vm_next;            // linked list of VM areas per task, sorted by address
    uint64_t vm_flags;              // Flags, see mm.h
};

struct mm_struct {
    vma_struct * mmap;              // list of VMAs
    vma_struct * mmap_cache;        // last find_vma result
    uint64_t mmap_base;             // base of mmap area
    uint64_t task_size;             // size of task vm space
    uint64_t cached_hole_size;      // if non-zero, the largest hole below free_area_cache
    uint64_t free_area_cache;       // first hole of size cached_hole_size or larger
    uint64_t *pml4_t;
    uint32_t map_count;             // number of VMAs

    uint64_t hiwater_vm;            // High-water virtual memory usage

    uint64_t total_vm;
    uint64_t stack_vm;
    uint64_t start_code, end_code, start_data, end_data;
    uint64_t start_brk, brk, start_stack;
    uint64_t arg_start, arg_end, env_start, env_end;

    uint64_t flags;                 // Must use atomic bitops to access the bits
};


#endif
