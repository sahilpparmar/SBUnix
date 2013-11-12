#include <defs.h>

inline void outb(uint16_t, uint8_t);
inline uint8_t inb(uint16_t);
void panic(char* msg);
void dump_regs();
