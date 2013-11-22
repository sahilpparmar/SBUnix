#include <stdio.h>
#include <syscall.h>
#include <sys/proc_mngr.h>
#include <string.h>
extern task_struct* CURRENT_TASK;

// These will get invoked in kernel mode. */

// [START] Accessory functions put in for testing scanf


volatile int flag, counter;
volatile char buf[1024];
/*
int strlen(const char *str)
{
    int len=0;
    while (*str++ != '\0')
        len += 1;
    return len;
}



// Copies 1 byte at a time
void *memcpy(void *destination, void *source, uint64_t num) 
{
    uint8_t *dest = (uint8_t *)destination;
    uint8_t *src = (uint8_t *)source;

    while(num--) {
        *dest++ = *src++; 
    }

    return destination;
}
*/
// [END] Accessory functions put in for testing scanf

int sys_puts(char* s)
{
    puts(s);
    return 0;
}

int sys_gets(uint64_t addr)
{
    char *user_buf = (char*) addr;
    int count;
    flag = 1;
    kprintf("\nEnter char:");
    sti;
    while(flag == 1);
    memcpy((void *)user_buf, (void *)buf, counter);
    count = counter;
    counter = 0;
    return count;
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

