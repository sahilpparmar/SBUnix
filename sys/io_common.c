//
// io_common.c -- Implementation for I/O Port accesses
//

#include <defs.h>
#include <io_common.h>
#include <stdio.h>

// Write a byte out to the specified port
inline void outb(uint16_t port, uint8_t value)
{
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Reading from the I/O ports to get data from devices like keyboard
inline unsigned char inb(uint16_t port)
{
    unsigned char value;
    asm volatile("inb %1, %0" : "=a"(value) : "dN"(port));
    return value;
}

void dump_regs()
{
    volatile uint64_t rsp, rbp, rsi, rdi, rip, rflags, rax, rbx, rcx, rdx;
    volatile uint64_t register cr0, cr2, cr3, cr4;

    __asm__ __volatile__(
        "movq %%rsp, %[rsp]\n\t"
        "movq %%rbp, %[rbp]\n\t"
        "movq %%rsi, %[rsi]\n\t"
        "movq %%rdi, %[rdi]\n\t"
        "movq %%rax, %[rax]\n\t"
        "movq %%rbx, %[rbx]\n\t"
        "movq %%rcx, %[rcx]\n\t"
        "movq %%rdx, %[rdx]\n\t"
        "pushfq\n\t"
        "popq %[rflags]\n\t"
        :
        [rsp]"=m"(rsp),
        [rbp]"=m"(rbp),
        [rsi]"=m"(rsi),
        [rdi]"=m"(rdi),
        [rax]"=m"(rax),
        [rbx]"=m"(rbx),
        [rcx]"=m"(rcx),
        [rdx]"=m"(rdx),
        [rflags]"=m"(rflags));

    __asm__ __volatile__(
        "leaq (%%rip), %[rip]\n\t"
        "movq %%cr0, %[cr0]\n\t"
        "movq %%cr2, %[cr2]\n\t"
        "movq %%cr3, %[cr3]\n\t"
        "movq %%cr4, %[cr4]\n\t"
        :
        [rip]"=r"(rip),
        [cr0]"=r"(cr0), 
        [cr2]"=r"(cr2),
        [cr3]"=r"(cr3),
        [cr4]"=r"(cr4));

    printf("\nRegister Dump:"
           "\nrsp = %p, rbp = %p, rsi = %p,"
           "\nrdi = %p, rip = %p, rflags = %p,"
           "\nrax = %p, rbx = %p, rcx = %p, rdx = %p,"
           "\ncr0 = %p, cr2 = %p, cr3 = %p, cr4 = %p",
           rsp, rbp, rsi, rdi, rip, rflags, rax, rbx, rcx, rdx, cr0, cr2, cr3, cr4);

}

void panic(char* msg)
{
       	dump_regs();
	printf("\n[FAULT]: %s\n", msg);
	while(1);
}

