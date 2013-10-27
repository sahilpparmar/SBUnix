#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

#include <sys/mm_types.h>
#include <sys/types.h>

struct proc_struct;

struct proc_struct {
    unsigned int flags;    
    int prio;

    int state;
    pid_t pid;

    //struct mm_struct mm;
    uint64_t pml4_t;
    
    struct proc_struct *parent;
    struct proc_struct *children;
};

#endif
