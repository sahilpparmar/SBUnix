#include <stdio.h>
#include <syscall.h>
#include <sys/proc_mngr.h>
#include <sys/virt_mm.h>
#include <io_common.h>
#include <string.h>
#include <sys/types.h>
#include <sys/dirent.h>
#include <sys/kmalloc.h>
#include <io_common.h>
#include <string.h>

extern task_struct* CURRENT_TASK;

// These will get invoked in kernel mode

volatile int flag, counter;
volatile char buf[1024];

DIR* sys_opendir(uint64_t* entry, uint64_t* directory)
{
    
    char* dir_path = (char *)entry;
    DIR* dir = (DIR *)directory;

    fnode_t *currnode = root_node;
    char *temp = NULL; 
    int i;
    char *path = (char *)kmalloc(sizeof(char) * strlen(dir_path));
    kstrcpy(path, dir_path); 

    temp = kstrtok(path, "/");  
    
    while(temp != NULL)
    {
        for(i = 2; i < currnode->end; ++i){
            if(strcmp(temp, currnode->f_child[i]->f_name) == 0) {
                currnode = (fnode_t *)currnode->f_child[i];
                break;       
            }        
        }
        
        temp = kstrtok(NULL, "/");          
    }
   

    if(currnode->f_type == DIRECTORY) {
    
        dir->curr = 2; 
        dir->filenode = currnode; 
        return dir;
    } else {
        return NULL; 
    }
}


struct dirent* sys_readdir(uint64_t* entry)
{
    
    DIR *dir = (DIR*)entry;
    if(dir->filenode->end < 3 || dir->curr == dir->filenode->end ||dir->curr == 0) { 
        return NULL;
    } else{
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

int gets(uint64_t addr)
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


int sys_munmap(uint64_t* addr, uint64_t length)
{
    vma_struct *iter, *temp = NULL;
    uint64_t end_addr, no_of_pages, i;
    bool myflag = 0, retflag = 0; 
 
    //check if address is 4k aligned
    if (((uint64_t )addr & 0xfff) != 0)
        return -1;
    
    end_addr = (uint64_t)((void *)addr + length);
    end_addr = ((((end_addr - 1) >> 12) + 1) << 12);
    
    iter = CURRENT_TASK->mm->vma_list;
    
    //iterate through vma list and free all anon-type vma
    //check if there are any other vmas mappped on this page
    //if not then free the page

    for (iter = CURRENT_TASK->mm->vma_list; iter->vm_next != NULL; ) {
        temp = iter;
        iter = iter->vm_next;
    
        if (iter->vm_start >= (uint64_t)addr || iter->vm_end > (uint64_t)addr)
            break;
    }    
    
    //kprintf("\n address of this vma %p address of temp %p end addr %p\n", iter->vm_start, temp->vm_start, end_addr);

    while(iter != NULL) { 
        
        if (iter->vm_start < end_addr && iter->vm_type == ANON) {
            //delete this vma 
            //TODO: need to add this to freelist of vma 
            temp->vm_next = iter->vm_next; 
            iter          = iter->vm_next; 
            retflag       = 1;
        
        }else if(iter->vm_type != ANON) {
            myflag = 1; 
            temp   = iter;
            iter   = iter->vm_next;
        }else {
            break; 
        }
       
        if (iter ==  NULL || iter->vm_end > end_addr)       
            break;
    
    }    
   
    if (myflag == 0) {
        
         no_of_pages = (end_addr - (uint64_t )addr)/PAGESIZE;
         
         for (i = 0; i < no_of_pages; ++i) {
             //free this page 
             free_virt_page(addr);
             addr = (void *)addr + PAGESIZE; 
         }    
    }

    if(retflag == 1){
        return 0;
    } else{
        return -1;
    }
}

uint64_t sys_open(uint64_t* dir_path, uint64_t flags)
{
    char* file_path = (char *)dir_path;
    
    //allocate new filedescriptor
    FD* file_d = (FD *)kmalloc(sizeof(FD));
    fnode_t *currnode = root_node;

    char *temp = NULL; 
    int i;
    char *path = (char *)kmalloc(sizeof(char) * strlen(file_path));
    kstrcpy(path, file_path); 

    temp = kstrtok(path, "/");  
    
    
    while(temp != NULL)
    {
        for(i = 2; i < currnode->end; ++i){
            if(strcmp(temp, currnode->f_child[i]->f_name) == 0) {
                currnode = (fnode_t *)currnode->f_child[i];
                break;       
            }        
        }
        
        temp = kstrtok(NULL, "/");          
    }
   

    file_d->filenode = currnode;
    file_d->curr = 0;
    
    
    //traverse file descriptor array and insert this entry
    for(i = 3; i < MAXFD; ++i) {
        if(CURRENT_TASK->file_descp[i] == NULL) {
            CURRENT_TASK->file_descp[i] = (uint64_t *)file_d;
    
            //kprintf("\n name: %s",((FD *)(CURRENT_TASK->file_descp[i]))->filenode->f_name);
            return i;        
        }
    }
   
   return 0;
}

void sys_close(int fd)
{
    //TODO add this filedescriptor to free list    
    CURRENT_TASK->file_descp[fd] = NULL;
    
}


uint64_t* sys_mmap(uint64_t* addr, uint64_t nbytes, uint64_t flags)
{
    vma_struct *node, *iter, *temp;
    bool myflag = 0;

    if (addr == 0x0) {

        //if address is not specified by the user then 
        //allocate new address above the vm_end of last vma
        for (iter = CURRENT_TASK->mm->vma_list; iter->vm_next != NULL; iter = iter->vm_next);

        //page aligning the addr
        addr = (uint64_t *)((((iter->vm_end - 1) >> 12) + 1) << 12);
    
    } else {

        //check if allocating new page does not conflicts with memory mapped by other VMAs
        if (verify_addr(CURRENT_TASK,(uint64_t)addr, (uint64_t)((void *)addr + nbytes)) == 0) {
            kprintf("\n not valid addr"); 
            return NULL;   
        }
    } 

    node           = alloc_new_vma((uint64_t)addr, (uint64_t)((void *)addr + nbytes)); 
    node->vm_flags = RW;
    node->vm_type  = ANON;

    CURRENT_TASK->mm->vma_count++;
    CURRENT_TASK->mm->total_vm += nbytes;

    iter = CURRENT_TASK->mm->vma_list;

    /*check position where we can insert this new vma in vma list*/
    while (iter->vm_next != NULL) {

        temp = iter;            
        iter = iter->vm_next;

        if (temp->vm_end < (uint64_t)addr && iter->vm_start > (uint64_t )((void *)addr + nbytes)) {
            myflag = 1;
            break;
        }
    }

    if (myflag == 1){
        temp->vm_next = node;
        node->vm_next = iter;
    } else {
        iter->vm_next = node; 
    }

    kmmap((uint64_t)addr, nbytes); 

    return (void *)addr;
}

uint64_t sys_read(uint64_t fd_type, uint64_t addr, uint64_t length)
{
    if (fd_type == 0) {
        length = gets(addr);

    } else if(fd_type > 2) {
        
        if((CURRENT_TASK->file_descp[fd_type]) == NULL) {
            length = -1;
        } else {
            uint64_t start, end;
            int currlength = 0;
    
            currlength = (int)((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr;
            start      = ((FD *)(CURRENT_TASK->file_descp[fd_type]))->filenode->start;
            end        = ((FD *)(CURRENT_TASK->file_descp[fd_type]))->filenode->end;
           
            if((end - (uint64_t)((void *)start + currlength)) < length) {
                length = (end - (uint64_t)((void *)start + currlength));
            }
            
            memcpy((uint64_t *)addr, (uint64_t *)((void *)start + currlength), length);
        
            ((FD *)(CURRENT_TASK->file_descp[fd_type]))->curr += length;
        } 
    }
    
    return length;
}





int sys_write(int n, uint64_t addr, int len)
{
    int l = 0;
    if(n == 1 || n == 2)
        l = puts((char*) addr);
    return l;
}

uint64_t sys_brk(uint64_t no_of_pages)
{
    uint64_t new_vaddr;
    uint64_t cur_top_vaddr = get_top_virtaddr();

    set_top_virtaddr(CURRENT_TASK->mm->end_brk);
    new_vaddr = (uint64_t)virt_alloc_pages(no_of_pages);

    //kprintf("\n New Heap Page Alloc:%p", new_vaddr);
    increment_brk(CURRENT_TASK, PAGESIZE * no_of_pages);

    // Restore old top Vaddr
    set_top_virtaddr(cur_top_vaddr);
    return new_vaddr;
}

pid_t sys_getpid()
{
    return CURRENT_TASK->pid;
}

pid_t sys_getppid()
{
    return CURRENT_TASK->ppid;
}

void sys_listprocess()
{
    int i = 0;
    task_struct *cur = CURRENT_TASK;

    kprintf("\n ===== LIST OF CURRENT PROCESSES ====== \n  #  |  PID  |  PPID  |  Process Name \n ----| ----- | ------ | --------------- ");
    while(cur)
    {
        kprintf("\n  %d  |   %d   |    %d   |  %s  ", ++i, cur->pid, cur->ppid, cur->comm);
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
    sys_close
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

