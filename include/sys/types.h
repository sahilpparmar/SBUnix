#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#define cli __asm__ __volatile__("cli");
#define sti __asm__ __volatile__("sti");

#define PAGESIZE 0x1000
#define PAGE_2ALIGN 12     // 2 ^ PAGE_2ALIGN = PAGESIZE
#define PAGE_ALIGN(ADDR) ((ADDR) >> 12 << 12)

#define READ_CR2(lcr2) __asm__ __volatile__ ("movq %%cr2, %0;" : "=r"(lcr2));
#define LOAD_CR3(lcr3) __asm__ __volatile__ ("movq %0, %%cr3;" :: "r"(lcr3));
#define READ_CR3(lcr3) __asm__ __volatile__ ("movq %%cr3, %0;" : "=r"(lcr3));

#endif
