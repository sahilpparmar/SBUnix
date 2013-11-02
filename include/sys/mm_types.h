#ifndef _LINUX_MM_TYPES_H
#define _LINUX_MM_TYPES_H

struct vm_area_struct;
struct mm_struct;

struct vm_area_struct {
    struct mm_struct *vm_mm;        /* The address space we belong to. */
    unsigned long vm_start;         /* Our start address within vm_mm. */
    unsigned long vm_end;           /* The first byte after our end address
                                       within vm_mm. */
    /* linked list of VM areas per task, sorted by address */
    struct vm_area_struct *vm_next;

    unsigned long vm_flags;         /* Flags, see mm.h. */
};

struct mm_struct {
    struct vm_area_struct * mmap;           /* list of VMAs */
    struct vm_area_struct * mmap_cache;     /* last find_vma result */
    unsigned long mmap_base;                /* base of mmap area */
    unsigned long task_size;                /* size of task vm space */
    unsigned long cached_hole_size;         /* if non-zero, the largest hole below free_area_cache */
    unsigned long free_area_cache;          /* first hole of size cached_hole_size or larger */
    uint64_t *pml4_t;
    int map_count;                          /* number of VMAs */

    unsigned long hiwater_vm;               /* High-water virtual memory usage */

    unsigned long total_vm;
    unsigned long stack_vm;
    unsigned long start_code, end_code, start_data, end_data;
    unsigned long start_brk, brk, start_stack;
    unsigned long arg_start, arg_end, env_start, env_end;

    unsigned long flags; /* Must use atomic bitops to access the bits */
};

#endif
