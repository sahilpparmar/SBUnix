//
// isr_handler.c -- High level interrupt service routines and interrupt request handlers.
//

#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <io_common.h>
#include <sys/irq_common.h>
#include <sys/types.h>

/* Divide by Zero handler */
static void divide_by_zero_handler(registers_t regs)
{
    panic("Divide by Zero!");
}

/* Invalid TSS exception handler */
static void tss_fault_handler(registers_t regs)
{
    dump_regs();
    panic("Invalid TSS Exception!");
}

/* General Protection fault handler */
static void gpf_handler(registers_t regs)
{
    dump_regs();
    panic("General Protection Fault!");
}

/* Page Fault handler */
static void page_fault_handler(registers_t regs)
{
    uint64_t err_code = regs.err_no;
    uint64_t fault_addr;

    READ_CR2(fault_addr);
    kprintf("\nFault Addr:%p Error Code:%p", fault_addr, err_code);
    panic("Page Fault!");
#if 0
    vm_struct* vma_to_map = NULL;
    u8int okay_to_map = 0;
    /* If the kernel faults or there's a fault in accessing a Present page, stop. */
    kprintf("The faulting address = %p\n", faulting_address);
    kprintf("The error code = %x\n", error_code);

    /* Get the PTE entry for the faulting address */
    u64int* ptr = (u64int*)PT_ENTRY(faulting_address);
    pt_e* pte_entry = ptr + PT_OFFSET(faulting_address); 
    kprintf("*pte_entry = %p\n", *pte_entry);
    if(is_supervisor(*pte_entry) || faulting_address >= KERN_VIR_START) {
        /* Page fault in kernel. Panic */
        dump_regs();
        panic("Page fault in kernel!");
    } else if (is_present(*pte_entry) && is_readonly(*pte_entry) && is_cow(*pte_entry)) {
        /* 
         * Copy on write!
         * Steps - 
         * 1. Allocate a new physical page.
         * 2. Read the faulting page and copy its contents to the new page.
         * 3. Update the PTE entry to point to the new physical page.
         * 4. Unset cow and readonly.
         */
        kmemcpy(backup_page, (void*)((u64int)faulting_address & 0xfffffffffffff000), PAGE_SIZE);
        phys_vir_addr* page_ptr = get_free_phys_page();
        set_base_addr(pte_entry, page_ptr->phys_addr);
        kmemcpy((void*)((u64int)faulting_address & 0xfffffffffffff000), backup_page, PAGE_SIZE);
        unset_cow(pte_entry);
        unset_readonly(pte_entry);
    } else if (!is_present(*pte_entry)){
        /* Perhaps page faulted on a mallocked page */
        vm_struct* vm_struct_ptr = CURRENT_TASK->vm_head;
        while (vm_struct_ptr != NULL){
            if (faulting_address >= vm_struct_ptr->vm_start && 
                    faulting_address <= vm_struct_ptr->vm_end){
                vma_to_map = vm_struct_ptr;
                okay_to_map = 1;
                break;
            }
            vm_struct_ptr = vm_struct_ptr->vm_next;
        }
        if(okay_to_map){
            kmmap((void*)vma_to_map->vm_start, 
                    vma_to_map->vm_end-vma_to_map->vm_start,
                    0, 0, 0, 0);
        } else {
            dump_regs();
            panic("SEGFAULT - shouldn't kill the kernel, though!");
        }
    } else {
        dump_regs();
        panic("Unhandled page fault!");
    }
#endif
}

/* Common handler */
void isr_handler(registers_t regs)
{
    switch (regs.int_no) {
        case 0:
            divide_by_zero_handler(regs);
            break;
        case 10:
            tss_fault_handler(regs);
            break;
        case 13:
            gpf_handler(regs);
            break;
        case 14:
            page_fault_handler(regs);
            break;
        default:
            break;
    }
}

