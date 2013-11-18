#include <stdio.h>
#include <syscall.h>
#include <sys/proc_mngr.h>

extern task_struct* CURRENT_TASK;

// These will get invoked in kernel mode. */
int sys_printf(char* s)
{
    puts(s);
    return 0;
}

int sys_scanf(char* s)
{
    return 0;
}

int sys_malloc(uint32_t size)
{
    uint64_t ret_addr = NULL;
    return ret_addr;
}

// Set up the system call table
void* syscall_tbl[] = 
{
    sys_printf,
    sys_scanf,
    sys_malloc
};

int NR_syscalls = sizeof(syscall_tbl)/sizeof(void*);

// The handler for the int 0x80
void syscall_handler(void)
{
    uint64_t rax;
    void *location;
    uint64_t ret;

    __asm__ __volatile__("movq %%rax, %0" : "=r"(rax));

    if (rax >= NR_syscalls)
        return;

    location = syscall_tbl[rax];

    __asm__ __volatile__("callq *%0;" : "=a" (ret) : "r" (location));

    __asm__ __volatile__("iretq;");
}

