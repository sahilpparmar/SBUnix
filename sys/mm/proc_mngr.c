#include <defs.h>
#include <sys/phys_mm.h>
#include <sys/mm_types.h>
#include <sys/sched.h>
#include <sys/types.h>

static pid_t no_of_procs;

static proc_struct proc[5];
static proc_struct *cur_proc;

pid_t create_process(uint64_t pml4_t)
{
    pid_t proc_ind = no_of_procs++;

    // TODO: Need dynamic allocation
    if (no_of_procs > 5) {
        return -1;
    }

    proc[proc_ind].prio  = 0;       // Needs to be changed
    proc[proc_ind].flags = 0;       // Needs to be changed
    proc[proc_ind].state = 0;       // Needs to be changed

    proc[proc_ind].pid = no_of_procs;
    proc[proc_ind].pml4_t = pml4_t;

    proc[proc_ind].parent = NULL;
    proc[proc_ind].children = NULL;

    if (cur_proc == NULL) {
        cur_proc = &proc[proc_ind];
        cur_proc->next = cur_proc;
    } else {
        proc[proc_ind].next = cur_proc->next;
        cur_proc->next = &proc[proc_ind];
        cur_proc = cur_proc->next;
    }

    return cur_proc->pid;
}

// Initialises the tasking system.
void initialise_tasking()
{
    no_of_procs = 0;
    cur_proc = NULL;
}

// Called by the timer hook, this changes the running process.
void task_switch()
{

}

// Causes the current process' stack to be forcibly moved to a new location.
void move_stack(void *new_stack_start, uint32_t size)
{

}

// Returns the pid of the current process.
pid_t getpid()
{
    return cur_proc->pid;
}

