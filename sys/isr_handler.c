//
// isr_handler.c -- High level interrupt service routines and interrupt request handlers.
//

#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <io_common.h>
#include <sys/irq_common.h>

/* Divide by Zero handler */
static void isr0_handler(registers_t regs)
{
    panic("Divide by Zero!");
}

/* Invalid TSS exception handler */
static void isr10_handler(registers_t regs)
{
    panic("Invalid TSS Exception!");
}

/* General Protection fault handler */
static void isr13_handler(registers_t regs)
{
    panic("General Protection Fault!");
}

/* Page Fault handler */
static void isr14_handler(registers_t regs)
{
    panic("Page Fault!");
//    volatile uint64_t fault_addr = 0x0;
//    volatile uint64_t err_code = 0x0;
//    vma_struct* vma_to_map = NULL;
//    bool okay_to_map = 0;
//
//    __asm__ __volatile__("movq %%cr2, %[cr2_r]":[cr2_r]"=r"(fault_addr));
//    __asm__ __volatile__("movq %%r10, %[err_code]":[err_code]"=r"(err_code));
//    kprintf("inside pf handler for %p with code %x", fault_addr, err_code);
//    // If the kernel faults or there's a fault in accessing a Present page, stop
//    if (fault_addr >= KERNEL_START_VADDR || (err_code & 0x01)) { 
//        // Page fault in Kernel 
//        dump_regs();
//        panic("Bad type of Page fault Happened!");
//    } else {
//        /* @TODO - Assume that the CURRENT_TASK has caused the page fault. 
//         *  Check if this is always true */
//        vm_struct* vm_struct_ptr = CURRENT_TASK->vm_head;
//        while (vm_struct_ptr != NULL) {
//            if (fault_addr >= vm_struct_ptr->vm_start && fault_addr <= vm_struct_ptr->vm_end) {
//                vma_to_map = vm_struct_ptr;
//                okay_to_map = 1;
//                break;
//            }
//            vm_struct_ptr = vm_struct_ptr->vm_next;
//        }
//    }
//    if (okay_to_map) {
//        mmap((void*)vma_to_map->vm_start, vma_to_map->vm_end-vma_to_map->vm_start);
//    }
}

/* Common handler */
void isr_handler(registers_t regs)
{
    switch (regs.int_no) {
        case 0:
            isr0_handler(regs);
            break;
        case 10:
            isr10_handler(regs);
            break;
        case 13:
            isr13_handler(regs);
            break;
        case 14:
            isr14_handler(regs);
            break;
        default:
            break;
    }
}

