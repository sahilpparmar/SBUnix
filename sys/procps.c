#include <syscall.h>
#include <defs.h>
#include <stdlib.h>
#include <sys/proc_mngr.h>
#include <stdio.h>
void procps()
{
    task_struct *cur = CURRENT_TASK;
    kprintf(" Test");
    while(cur)
    {
        kprintf("sOHIL Process PID : %d", cur->pid);
        cur = cur->next;
    }
}


