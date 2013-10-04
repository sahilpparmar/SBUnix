//
// irq_common.h -- Interface and structures for high level interrupt service routines
//

#include <defs.h>

struct registers
{
    uint64_t rdi, rsi, rbp, rsp, rdx, rcx, rbx, rax; // Registers
    uint64_t int_no, err_no;    // Interrupt number and error code
} __attribute__((packed));

typedef struct registers registers_t;

void init_timer(uint32_t);
void init_keyboard();

