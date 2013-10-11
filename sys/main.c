#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <sys/init_desc_table.h>
#include <sys/irq_common.h>
#include <sys/paging.h>
#include <sys/phys_mm.h>


void start(uint32_t* modulep, void* physbase, void* physfree)
{
    uint64_t smap_len = 0x0, smap_base = 0x0;

    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    while(modulep[0] != 0x9001) modulep += modulep[1]+2;
    for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            set_cursor_pos(15, 5);
            printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
            smap_len = smap->length;
            smap_base = smap->base;
        }
    }
   
    // kernel starts here
    set_cursor_pos(10, 5);
    printf("smaplen = %p smapbase = %p", smap_len, smap_base);
    pmmngr_init((smap_len - 0x300000)/8192, physfree, smap_base + 0x300000); 

    //set_cursor_pos(16, 5);
    //printf("%p %p", physbase, physfree);
    kernel_allocate();
//    __asm__(
//            "movq $0xFFFFF80400000, %rax;"
//            "movq %rax, %cr3;"
//           );
    while(1);
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;

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

    // Intialize
    init_gdt();
    init_idt();
    setup_tss();
    init_pic();
    init_timer(1);
    init_keyboard();

    init_screen(0xFFFFFFFF800B8000);
    clear_screen();
    set_color(RED, BLACK);
    set_cursor_pos(11, 25);
    printf("________________");
    set_cursor_pos(12, 25);
    printf("|    SBUnix    |");
    set_cursor_pos(13, 25);
    printf("----------------");

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

