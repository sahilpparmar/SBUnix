//
// io_common.c -- Implementation for I/O Port accesses
//

#include <defs.h>
#include <io_common.h>

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

