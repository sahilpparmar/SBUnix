#include <stdio.h>
#include <syscall.h>
#include <sys/proc_mngr.h>

extern task_struct* CURRENT_TASK;

// These will get invoked in kernel mode. */
int sys_puts(char* s)
{
    puts(s);
    return 0;
}

char* sys_gets()
{
    return 0;
}

int sys_mmap(uint32_t size)
{
    uint64_t ret_addr = NULL;
    return ret_addr;
}

int sys_fork()
{
    return 0;
}

// Set up the system call table
void* syscall_tbl[NUM_SYSCALLS] = 
{
    sys_puts,
    sys_gets,
    sys_mmap,
    sys_fork
};

// The handler for the int 0x80
void syscall_handler(void)
{
    uint64_t syscallNo;

    __asm__ __volatile__("movq %%rax, %0;" : "=r"(syscallNo));

    if (syscallNo >= 0 && syscallNo < NUM_SYSCALLS) {
        void *func_ptr = syscall_tbl[syscallNo];
        uint64_t ret;

        __asm__ __volatile__("callq *%0;" : "=a" (ret) : "r" (func_ptr));
    }

    __asm__ __volatile__("iretq;");
}

