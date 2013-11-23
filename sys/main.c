#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <sys/init_desc_table.h>
#include <sys/irq_common.h>
#include <sys/types.h>
#include <sys/paging.h>
#include <sys/phys_mm.h>
#include <sys/virt_mm.h>
#include <sys/kmalloc.h>
#include <sys/proc_mngr.h>

#define K_MEM_PAGES 518
#define INITIAL_STACK_SIZE 4096

char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;

void idle_process(void)
{
    kprintf("\nInside Idle Process %d", sys_getpid());
    while(1);
}

void start(uint32_t* modulep, void* physbase, void* physfree)
{
    uint64_t phys_size = 0x0, phys_base = 0x0;

    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    while(modulep[0] != 0x9001) modulep += modulep[1]+2;
    for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            set_cursor_pos(15, 5);
            kprintf("Available Physical Memory [%x-%x]", smap->base, smap->base + smap->length);
            phys_base = smap->base;
            phys_size = smap->length;
        }
    }
   
    // kernel starts here

    phys_init(phys_base, phys_size); 

    init_paging((uint64_t)&kernmem, (uint64_t)physbase, K_MEM_PAGES);

    // Reset the kernel stack
    __asm__ __volatile__("movq %0, %%rbp" : :"a"(&stack[0]));
    __asm__ __volatile__("movq %0, %%rsp" : :"a"(&stack[INITIAL_STACK_SIZE]));

    // Schedule an Idle Kernel Process 
    task_struct* idle_proc = alloc_new_task(FALSE);
    schedule_process(idle_proc, (uint64_t)idle_process, KERNEL_STACK_SIZE);
// Context Switching code between tarfs processes
#if 0
    create_elf_proc("bin/hello");
    create_elf_proc("bin/world");
    create_elf_proc("bin/ps");
#endif

    // Allow interrupts
    sti;

    kprintf("\nEnd of Kernel");
    while(1);
}


void boot(void)
{
    // note: function changes rsp, local stack variables can't be practically used
    register char *temp1, *temp2;
    __asm__(
            "movq %%rsp, %0;"
            "movq %1, %%rsp;"
            :"=g"(loader_stack)
            :"r"(&stack[INITIAL_STACK_SIZE])
           );

    // Disable interrupts
    cli;

    // Intialize
    init_gdt();
    init_tss();
    init_idt();
    init_pic();
    init_screen();
    init_timer(1);
    init_keyboard();

    set_cursor_pos(11, 25);
    kprintf("________________");
    set_cursor_pos(12, 25);
    kprintf("|    SBUnix    |");
    set_cursor_pos(13, 25);
    kprintf("----------------");

    start(
            (uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
            &physbase,
            (void*)(uint64_t)loader_stack[4]
         );
    for(
            temp1 = "!!!!! start() returned !!!!!", temp2 = (char*)0xb8000;
            *temp1;
            temp1 += 1, temp2 += 2
       ) *temp2 = *temp1;
    while(1);
}

