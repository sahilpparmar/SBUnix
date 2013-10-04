//
// isr_handler.c -- High level interrupt service routines and interrupt request handlers.
//

#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <sys/irq_common.h>

static void isr0_handler(registers_t regs)
{
    set_cursor_pos(2, 0);
    printf("Divide by Zero!\n");
}

void isr_handler(registers_t regs)
{
    switch (regs.int_no) {
        case 0:
            isr0_handler(regs);
            break;
        default:
            break;
    }
}

