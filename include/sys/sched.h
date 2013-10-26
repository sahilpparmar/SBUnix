#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

#include <mm_types.h>
#include <types.h>

struct task_struct;

struct task_struct {
    unsigned int flags;    
    int prio;

    int exit_state;
    pid_t pid;

    struct mm_struct *mm;

    struct task_struct *parent;
    struct task_struct *children;

};

#endif
