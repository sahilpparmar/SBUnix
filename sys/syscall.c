#include <defs.h>
#include <sys/common.h>
#include <syscall.h>
#include <sys/paging.h>
#include <sys/proc_mngr.h>
#include <sys/elf.h>
#include <sys/tarfs.h>
#include <sys/virt_mm.h>
#include <io_common.h>
#include <sys/kstring.h>
#include <screen.h>
#include <sys/types.h>
#include <sys/fs.h>
#include <sys/dirent.h>
#include <sys/kmalloc.h>
#include <io_common.h>

// These will get invoked in kernel mode

extern fnode_t* root_node;


vma_struct* vmalogic(uint64_t addr, uint64_t nbytes, uint64_t flags, uint64_t type, uint64_t file_d)
{
    vma_struct *node, *iter, *temp;
    bool myflag = 0;

    node = alloc_new_vma(addr, addr + nbytes, flags, type, file_d); 
    //kprintf("\n node start %p, end%p fd %d", node->vm_start, node->vm_end, node->vm_file_descp);

    
    CURRENT_TASK->mm->vma_count++;
    CURRENT_TASK->mm->total_vm += nbytes;

    iter = CURRENT_TASK->mm->vma_list;

    // check position where we can insert this new vma in vma list
    while (iter->vm_next != NULL) {

        temp = iter;            
        iter = iter->vm_next;

        if (temp->vm_end < addr && (iter->vm_start > addr + nbytes)) {
            myflag = 1;
            break;
        }
    }

    if (myflag == 1) {
        temp->vm_next = node;
        node->vm_next = iter;
    } else {
        iter->vm_next = node; 
    }

    return node;
}

DIR* sys_opendir(uint64_t* entry, uint64_t* directory)
{
    char* dir_path = (char *)entry;
    DIR* dir = (DIR *)directory;
    fnode_t *auxnode, *currnode = root_node;
    char *temp = NULL; 
    char *path = (char *)kmalloc(sizeof(char) * kstrlen(dir_path));
    int i;

    kstrcpy(path, dir_path); 
    temp = kstrtok(path, "/");  

    while (temp != NULL) {
        auxnode = currnode;

        if (kstrcmp(temp, ".") == 0) {
            currnode = (fnode_t *)currnode->f_child[0];

        } else if (kstrcmp(temp,"..") == 0) {
            currnode = (fnode_t *)currnode->f_child[1];

        } else {

            for (i = 2; i < currnode->end; ++i){
                if (kstrcmp(temp, currnode->f_child[i]->f_name) == 0) {
                    currnode = (fnode_t *)currnode->f_child[i];
                    break;       
                }        
            }
            if (i == auxnode->end) {
                dir->curr     = NULL;
                dir->filenode = NULL;
                return dir;
            }
        } 
        temp = kstrtok(NULL, "/");          
    }

    if (currnode->f_type == DIRECTORY) {
        dir->curr     = 2; 
        dir->filenode = currnode; 
    } else {
        dir->curr     = NULL;
        dir->filenode = NULL;
    }
    return dir;
}


int sys_mkdir( uint64_t dir)
{
    char *dirpath = (char *)dir; 
    
    int dirlength = kstrlen(dirpath);
    char *path;
    DIR* tempdir = kmalloc(sizeof(struct file_dir));
    fnode_t *temp_node;
    tempdir = sys_opendir((uint64_t *)dirpath,(uint64_t *) tempdir);
 
    if (tempdir->filenode != NULL) {
        //kprintf("\n directory already exists"); 
        return -1;
    }
       
    path = (char *)kmalloc(sizeof(char) * dirlength);
    kstrcpy(path, dirpath);

   
    int end = dirlength;


    if (path[dirlength - 1] == '/')
        end = dirlength - 2;
    else
        end = dirlength - 1;
    
    while (dirpath[end] != '/'){
        --end; 
         
    }
    
    path[end] = '\0'; 
    tempdir = sys_opendir((uint64_t *)path, (uint64_t*)tempdir); 
    
    if (tempdir != NULL) {
        temp_node = (fnode_t *)kmalloc(sizeof(fnode_t));                
         
        make_node(temp_node, tempdir->filenode, path + end + 1, 0, 2, DIRECTORY, 0);  
        tempdir->filenode->f_child[tempdir->filenode->end] = temp_node;
        tempdir->filenode->end += 1;
        return 0; 
    } else {
        //kprintf("\n %s Directory doesnot exists", path);
        return -1; 
    }


}


struct dirent* sys_readdir(uint64_t* entry)
{
    DIR *dir = (DIR*)entry;
    if (dir->filenode->end < 3 || dir->curr == dir->filenode->end || dir->curr == 0) { 
        return NULL;
    } else {
        kstrcpy(dir->curr_dirent.name, dir->filenode->f_child[dir->curr]->f_name);
        dir->curr++;
        return &dir->curr_dirent;
    }
}

int sys_closedir(uint64_t* entry)
{
    DIR *dir = (DIR *)entry; 
    if(dir->filenode->f_type == DIRECTORY && dir->curr > 1) {

        dir->filenode = NULL; 
        dir->curr = 0;
        return 0;
    } else {
        return -1; 
    }
}

int sys_open(char* dir_path, uint64_t flags)
{
    fnode_t *temp_node;
    // allocate new filedescriptor
    FD* file_d = (FD *)kmalloc(sizeof(FD));
    fnode_t *aux_node = NULL, *currnode = root_node;
    
    vma_struct *iter;
    uint64_t addr;
    char *temp = NULL; 
    int i, inode_no, creat_flag = 0;
    char *path = (char *)kmalloc(sizeof(char) * kstrlen(dir_path));
    kstrcpy(path, dir_path); 

    temp = kstrtok(path, "/");  

    if (kstrcmp(temp, "rootfs") == 0) {

        while (temp != NULL) {
            aux_node = currnode;
            for (i = 2; i < currnode->end; ++i) {
                if (kstrcmp(temp, currnode->f_child[i]->f_name) == 0) {
                    currnode = (fnode_t *)currnode->f_child[i];
                    break;       
                }        
            }

            if (i == aux_node->end) {
                return -1;
            }

            temp = kstrtok(NULL, "/");          
        }

        if (currnode->f_type == DIRECTORY) {
            
            kprintf("\n Invalid open operation on directory");
            return -1;
        }

        file_d->filenode = currnode;
        file_d->curr     = currnode->start;
        file_d->f_perm   = flags;

        //traverse file descriptor array and insert this entry
        for (i = 3; i < MAXFD; ++i) {
            if (CURRENT_TASK->file_descp[i] == NULL) {
                CURRENT_TASK->file_descp[i] = (uint64_t *)file_d;
                return i;        
            }
        }

        return -1;

    } else if ( kstrcmp(temp, "Disk") == 0) {

        while (temp != NULL) {
            aux_node = currnode;
            for (i = 2; i < currnode->end; ++i) {
                if (kstrcmp(temp, currnode->f_child[i]->f_name) == 0) {
                    currnode = (fnode_t *)currnode->f_child[i];
                    break;       
                }        
            }

            if (i == aux_node->end && flags == O_CREAT) {
                creat_flag = 1; 
                break; 
            } else if (i == aux_node->end && flags != O_CREAT) {

                //kprintf("\nFile doesnot exist"); 
                return -1;
            }

            temp = kstrtok(NULL, "/");          
        }

        // find address for allocating new vma
        for (iter = CURRENT_TASK->mm->vma_list; iter->vm_next != NULL; iter = iter->vm_next);
        // page aligning the addr
        addr = (uint64_t)((((iter->vm_end - 1) >> 12) + 1) << 12);

        ext_inode* inode_entry = kmalloc(sizeof(ext_inode));                

        if (creat_flag == 1) { 
            //if file is opened for first time, creat flag will be set to 1         
            //go to disk, create new file of default minimum size
            //return inode number

            temp_node = (fnode_t *)kmalloc(sizeof(fnode_t));                
            
            inode_no = alloc_new_inode();

            if (inode_no == -1) {
                return -1;
            }
            kstrcpyn(inode_entry->i_name, (char*)dir_path, sizeof(inode_entry->i_name)-1);
            inode_entry->i_size = 0;
            inode_entry->i_block_count = 0;
            write_inode(inode_entry, inode_no);

            file_d->inode_struct = (uint64_t) inode_entry;

            //add as child of currnode 
            make_node(temp_node, currnode, temp, 0, 0, FILE, inode_no);  

            currnode->f_child[currnode->end] = temp_node;
            currnode->end += 1; 

            file_d->filenode = temp_node;
            file_d->curr     = addr;
            file_d->f_perm   = flags;

            //traverse file descriptor array and insert this entry
            for (i = 3; i < MAXFD; ++i) {
                if (CURRENT_TASK->file_descp[i] == NULL) {
                    CURRENT_TASK->file_descp[i] = (uint64_t *)file_d;
                    vmalogic(addr, 0, RW, FILETYPE, i);
                    return i;        
                }
            }
        } else if (aux_node != currnode) {
            //if the file is open for appending then in above loop
            //currnode will not be equal to aux node

            read_inode(inode_entry, currnode->f_inode_no);

            if (flags == O_TRUNC) {
                truncate_inode(inode_entry, currnode->f_inode_no);
            }

            file_d->inode_struct = (uint64_t) inode_entry; 
            
            file_d->filenode = currnode;
            file_d->f_perm   = flags;

            //traverse file descriptor array and insert this entry
            for (i = 3; i < MAXFD; ++i) {
                if (CURRENT_TASK->file_descp[i] == NULL) {
                    CURRENT_TASK->file_descp[i] = (uint64_t *)file_d;

                    vma_struct *new_vma = vmalogic(addr, inode_entry->i_size, RW, FILETYPE, i);
                    kmmap(new_vma->vm_start, new_vma->vm_end - new_vma->vm_start, RW_USER_FLAGS);
                    copy_blocks_to_vma(inode_entry, new_vma->vm_start);
                    //kprintf("\n[OPEN] %p start %s, addr %s, end %p, length %p", new_vma, new_vma->vm_start, addr, new_vma->vm_end, inode_entry->i_size);

                    if (flags == O_APPEND) {
                        file_d->curr     = addr + inode_entry->i_size;
                        kprintf("\nSEEK[%d]%p", i, file_d->curr);
                    } else {
                        file_d->curr     = addr;
                    }

                    return i;        
                }
            }
        }
        return -1;
    }
    return -1;
}

void sys_close(int fd)
{
    ext_inode *inode_t = (ext_inode*) ((FD*) CURRENT_TASK->file_descp[fd])->inode_struct;
    int32_t inode_no = ((FD*) CURRENT_TASK->file_descp[fd])->filenode->f_inode_no;

    if (inode_t != NULL) {
        vma_struct *vma_l = CURRENT_TASK->mm->vma_list;
        vma_struct *last_vma_l = NULL;

        for (;vma_l != NULL; vma_l = vma_l->vm_next) {
            if (vma_l->vm_file_descp == fd) {
                break;
            }
            last_vma_l = vma_l;
        }
        copy_vma_to_blocks(inode_t, inode_no, vma_l->vm_start, vma_l->vm_end - vma_l->vm_start);

        last_vma_l->vm_next = vma_l->vm_next; 
        add_to_vma_free_list(vma_l);
    }
    
    //TODO add this filedescriptor to free list    
    CURRENT_TASK->file_descp[fd] = NULL;
}

uint64_t sys_read(uint64_t fd_type, uint64_t addr, uint64_t length)
{
    uint64_t end = 0, currlength = 0;

    if (fd_type == stdin) {
        length = gets(addr);

    } else if(fd_type > 2) {

        if ((CURRENT_TASK->file_descp[fd_type]) == NULL) {
            length = -1;

        } else if(((FD *)CURRENT_TASK->file_descp[fd_type])->filenode->f_inode_no != 0) { 
            //This file descriptor is associated with file on disk 

            vma_struct *iter; 

            currlength = (uint64_t)((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr;
            
            // get start and end of vma 
            for (iter = CURRENT_TASK->mm->vma_list; iter != NULL; iter = iter->vm_next) {
                if(iter->vm_file_descp == fd_type){
                    end   = iter->vm_end;
                    break; 
                }
            }
            
        } else {

            currlength = (uint64_t)((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr;
            end        = ((FD *)(CURRENT_TASK->file_descp[fd_type]))->filenode->end;
        }
        
        if ((end - currlength) < length) {
            length = (end - currlength);
        }

        memcpy((void *)addr, (void *)currlength, length);

        ((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr += length;
    }

    return length;
}

int sys_write(uint64_t fd_type, uint64_t addr, int length)
{

    if (fd_type == stdout || fd_type == stderr) {
        length = 0; 
        length = puts((char*) addr);

    } else if (fd_type > 2) {
        vma_struct *iter; 

        if ((CURRENT_TASK->file_descp[fd_type]) == NULL) {
            length = -1;
        } else if(((FD *)CURRENT_TASK->file_descp[fd_type])->f_perm == O_RDONLY ){
            //kprintf("\n Not valid permissions"); 
            length = -1; 
        } else {
            uint64_t end = 0, currlength = 0;

            currlength = ((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr;
            //get end of vma 
            for (iter = CURRENT_TASK->mm->vma_list; iter != NULL; iter = iter->vm_next) {
                if(iter->vm_file_descp == fd_type){
                    end = iter->vm_end;
                    break; 
                }
            }
            
            // adjust the end of vma if required
            if (currlength + length > end) {
                kmmap(iter->vm_end, (currlength + length) - iter->vm_end, RW_USER_FLAGS);
                iter->vm_end = currlength + length;
            }
           
            //kprintf("\n4currlength %p, string %p %s, end %p %p, length %p", currlength, iter, iter->vm_start, iter->vm_end, end, length);
            memcpy((void *)currlength, (void *)addr, length);
            
            ((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr += length;
        } 
    }

    return length;
}

int sys_lseek(uint64_t fd_type, int offset, int whence) 
{
    
    
    vma_struct* iter;
    ext_inode *inode_t = (ext_inode*) ((FD*) CURRENT_TASK->file_descp[fd_type])->inode_struct;
    
    if (inode_t == NULL) {
        //file descriptor belongs to tarfs 
        // ((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr = 0;
         //kprintf("\n cannot do lseek");
         return -1;
    } else {


        for (iter = CURRENT_TASK->mm->vma_list; iter != NULL; iter = iter->vm_next) {
            if(iter->vm_file_descp == fd_type){
                //start = iter->vm_start;
                break; 
            }
        }

        //kprintf("\n seek offset: %p start %p", offset, iter->vm_start);

        if(whence == SEEK_SET) {
            if (offset < 0)
                offset = 0;

            ((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr = iter->vm_start + offset;

        } else if(whence == SEEK_CUR) {

            if(((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr + offset > iter->vm_end) {

                ((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr = iter->vm_end;
            } else {        

                ((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr += offset;
            }

        } else if(whence == SEEK_END) {

            if (offset > 0)
                offset = 0;

            ((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr = iter->vm_end + offset;


        }else{
            offset = -1;
        }
    } 
    //kprintf("\n curr inside seek %p",((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr);
    return offset;

}

pid_t sys_fork()
{
    // Take a pointer to this process' task struct for later reference.
    task_struct *parent_task = CURRENT_TASK; 

    // Create a new process.
    task_struct* child_task = copy_task_struct(parent_task); 

    // Add it to the end of the ready queue
    schedule_process(child_task, parent_task->kernel_stack[KERNEL_STACK_SIZE-6], parent_task->kernel_stack[KERNEL_STACK_SIZE-3]);

    // Set return (rax) for child process to be 0
    child_task->kernel_stack[KERNEL_STACK_SIZE-7] = 0UL;

    return child_task->pid;
}

uint64_t sys_execvpe(char *file, char *argv[], char *envp[])
{
    //TODO: Need to load envp[]
    task_struct *new_task = create_elf_proc(file, argv);

    if (new_task) {
        task_struct *cur_task = CURRENT_TASK;

        // Exec process has same pid, ppid and parent
        set_next_pid(new_task->pid);
        new_task->pid  = cur_task->pid;
        new_task->ppid = cur_task->ppid;
        new_task->parent = cur_task->parent;
        memcpy((void*)new_task->file_descp, (void*)cur_task->file_descp, MAXFD*8);

        // Replace current child with new exec process
        replace_child_task(cur_task, new_task);

        // Exit from the current process
        empty_task_struct(cur_task);
        cur_task->task_state = EXIT_STATE;

        // Enable interrupt for scheduling next process
        __asm__ __volatile__ ("int $32");

        panic("EXECVPE terminated incorrectly");
    }
    // execvpe failed; so return -1
    return -1;
}

uint64_t sys_wait(uint64_t status)
{
    volatile task_struct *cur_task = CURRENT_TASK;
    int *status_p = (int*) status;

    if (cur_task->no_children == 0) {
        if (status_p) *status_p = -1;
        return -1;
    }

    // Reset last child exit
    cur_task->wait_on_child_pid = 0;
    cur_task->task_state = WAIT_STATE;

    // Enable interrupt for scheduling next process
    __asm__ __volatile__ ("int $32");

    if (status_p) *status_p = 0;
    return (uint64_t)cur_task->wait_on_child_pid;
}

uint64_t sys_waitpid(uint64_t fpid, uint64_t fstatus, uint64_t foptions)
{
    pid_t pid = fpid;
    volatile task_struct *cur_task = CURRENT_TASK;
    int *status_p = (int*) fstatus;

    if (cur_task->no_children == 0) {
        if (status_p) *status_p = -1;
        return -1;
    }

    if (pid > 0) {
        // If pid > 0, wait for the child with 'pid' to exit
        cur_task->wait_on_child_pid = pid;
    } else {
        // If pid <= 0, wait for any one of the children to exit
        cur_task->wait_on_child_pid = 0;
    }
    cur_task->task_state = WAIT_STATE;

    // Enable interrupt for scheduling next process
    __asm__ __volatile__ ("int $32");

    if (status_p) *status_p = 0;
    return (uint64_t)cur_task->wait_on_child_pid;
}

void sys_exit()
{
    task_struct *cur_task = CURRENT_TASK;

    // Remove the task from parent's child list
    if (cur_task->parent) {
        remove_child_from_parent(cur_task);
    }

    // Assign all the child processes as ZOMBIE 
    if (cur_task->childhead) {
        remove_parent_from_child(cur_task);
    }

    // Empty current task
    empty_task_struct(cur_task);
    cur_task->task_state = EXIT_STATE;
    //kprintf("\n[E]%s", cur_task->comm);

    // Enable interrupt for scheduling next process
    __asm__ __volatile__ ("int $32");

    panic("EXIT terminated incorrectly");
}

int sys_sleep(int msec)
{
    task_struct *task = CURRENT_TASK;

    // Convert into centiseconds, as our timer runs per 1 centisec
    task->sleep_time = msec/10;
    task->task_state = SLEEP_STATE;
    __asm__ __volatile__("int $32;");

    return task->sleep_time;
}

uint64_t sys_brk(uint64_t no_of_pages)
{
    uint64_t new_vaddr = CURRENT_TASK->mm->end_brk;

    //kprintf("\n New Heap Page Alloc:%p", new_vaddr);
    increment_brk(CURRENT_TASK, PAGESIZE * no_of_pages);

    return new_vaddr;
}

uint64_t sys_mmap(uint64_t addr, uint64_t nbytes, uint64_t flags)
{
    vma_struct *iter;    

    if (addr == 0x0) {
        // if address is not specified by the user then 
        // allocate new address above the vm_end of last vma
        for (iter = CURRENT_TASK->mm->vma_list; iter->vm_next != NULL; iter = iter->vm_next);

        // page aligning the addr
        addr = (uint64_t)((((iter->vm_end - 1) >> 12) + 1) << 12);

    } else {
        // check if allocating new page does not conflicts with memory mapped by other VMAs
        if (verify_addr(CURRENT_TASK, addr, addr + nbytes) == 0) {
            return NULL;
        }
    } 

    //node = alloc_new_vma((uint64_t)addr, (uint64_t)((void *)addr + nbytes), RW, ANON);
    vmalogic(addr, nbytes, RW, ANON, 0);
    return addr;
}

int sys_munmap(uint64_t* addr, uint64_t length)
{
    vma_struct *iter, *temp = NULL;
    uint64_t end_addr, no_of_pages, i;
    bool myflag = 0, retflag = 0; 

    // check if address is 4k aligned
    if (((uint64_t )addr & 0xfff) != 0)
        return -1;

    end_addr = (uint64_t)((void *)addr + length);
    end_addr = ((((end_addr - 1) >> 12) + 1) << 12);

    iter = CURRENT_TASK->mm->vma_list;

    // iterate through vma list and free all anon-type vma
    // check if there are any other vmas mappped on this page
    // if not then free the page

    for (iter = CURRENT_TASK->mm->vma_list; iter->vm_next != NULL; ) {
        temp = iter;
        iter = iter->vm_next;

        if (iter->vm_start >= (uint64_t)addr || iter->vm_end > (uint64_t)addr)
            break;
    }    

    //kprintf("\n address of this vma %p address of temp %p end addr %p\n", iter->vm_start, temp->vm_start, end_addr);

    while (iter != NULL) { 

        if (iter->vm_start < end_addr && iter->vm_type == ANON) {
            // delete this vma 
            temp->vm_next = iter->vm_next; 
            add_to_vma_free_list(iter);
            iter          = iter->vm_next; 
            retflag       = 1;

        } else if(iter->vm_type != ANON) {
            myflag = 1; 
            temp   = iter;
            iter   = iter->vm_next;
        } else {
            break; 
        }

        if (iter ==  NULL || iter->vm_end > end_addr)       
            break;
    }

    if (myflag == 0) {
        no_of_pages = (end_addr - (uint64_t )addr) / PAGESIZE;

        for (i = 0; i < no_of_pages; ++i) {
            // free this page 
            free_virt_page(addr);
            addr = (void *)addr + PAGESIZE; 
        }
    }

    if (retflag == 1) {
        return 0;
    } else {
        return -1;
    }
}

pid_t sys_getpid()
{
    return CURRENT_TASK->pid;
}

pid_t sys_getppid()
{
    return CURRENT_TASK->ppid;
}

int sys_clear()
{
    clear_screen();
    return 1;
}

const char *t_state[NUM_TASK_STATES] = { "RUNNING" , "READY  " , "SLEEP  " , "WAIT   " , "IDLE   " , "EXIT   ", "ZOMBIE "};

void sys_listprocess()
{
    int i = 0;
    task_struct *cur = CURRENT_TASK;

    kprintf("\n ===== LIST OF CURRENT PROCESSES ====== "
            "\n  #  |  PID  |  PPID  |   State   |  Process Name "
            "\n ----| ----- | ------ | --------- | --------------- ");

    while (cur) {
        kprintf("\n  %d  |   %d   |   %d    |  %s  |  %s  ",
                ++i, cur->pid, cur->ppid, t_state[cur->task_state], cur->comm);
        cur = cur->next;
    }
}    

// Set up the system call table
void* syscall_tbl[NUM_SYSCALLS] = 
{
    sys_read,
    sys_write,
    sys_brk,
    sys_fork,
    sys_execvpe,
    sys_wait,
    sys_waitpid,
    sys_exit,
    sys_mmap,
    sys_munmap,
    sys_getpid,
    sys_getppid,
    sys_listprocess,
    sys_opendir,
    sys_readdir,
    sys_closedir, 
    sys_open,
    sys_close,
    sys_sleep,
    sys_clear,
    sys_lseek,
    sys_mkdir
};

// The handler for the int 0x80
void syscall_handler(void)
{
    uint64_t syscallNo;

    __asm__ __volatile__("movq %%rax, %0;" : "=r"(syscallNo));

    if (syscallNo >= 0 && syscallNo < NUM_SYSCALLS) {
        void *func_ptr;
        uint64_t ret;

        __asm__ __volatile__("pushq %rdx;");
        func_ptr = syscall_tbl[syscallNo];
        __asm__ __volatile__(
                "movq %%rax, %0;"
                "popq %%rdx;"
                "callq *%%rax;"
                : "=a" (ret) : "r" (func_ptr)
                );
    }

    __asm__ __volatile__("iretq;");
}

