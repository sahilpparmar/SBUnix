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
    panic("Page Fault Exception!");
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

