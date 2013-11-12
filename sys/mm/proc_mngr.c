#include <defs.h>
#include <sys/phys_mm.h>
#include <sys/mm_types.h>
#include <sys/sched.h>
#include <sys/types.h>

static pid_t no_of_procs;

static proc_struct *cur_proc;

pid_t create_process(uint64_t pml4_t)
{
    pid_t proc_ind = no_of_procs++;

    return proc_ind;
}

// Initialises the tasking system.
void initialise_tasking()
{
    no_of_procs = 0;
    cur_proc = NULL;
}

// Returns the pid of the current process.
pid_t getpid()
{
    return cur_proc->pid;
}

