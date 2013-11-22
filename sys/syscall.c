#include <stdio.h>
#include <syscall.h>
#include <sys/proc_mngr.h>
#include <sys/virt_mm.h>
#include <io_common.h>
#include <string.h>

extern task_struct* CURRENT_TASK;

// These will get invoked in kernel mode

volatile int flag, counter;
volatile char buf[1024];

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
    sti;
    while(flag == 1);

    memcpy((void *)user_buf, (void *)buf, counter);
    count = counter;
    counter = 0;
    return count;
}

int sys_mmap(uint64_t size)
{
    uint64_t ret_addr = NULL;
    return ret_addr;
}

uint64_t sys_brk(uint64_t no_of_pages)
{
    char *cp;
    uint64_t addr;
    addr = get_top_virtaddr();
    
    set_top_virtaddr(get_brk_top(CURRENT_TASK));
    cp = (char *)virt_alloc_pages(no_of_pages);
     
    //kprintf("\n current heap:%p", cp);
    increment_brk(CURRENT_TASK, (uint64_t)((void *)cp + PAGESIZE * no_of_pages));

    set_top_virtaddr(addr);
    return (uint64_t)cp;
}

// Set up the system call table
void* syscall_tbl[NUM_SYSCALLS] = 
{
    sys_puts,
    sys_gets,
    sys_brk,
    sys_fork,
    sys_mmap
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

