#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <sys/init_desc_table.h>
#include <sys/irq_common.h>

void start(void* modulep, void* physbase, void* physfree)
{
    // kernel starts here
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;

void boot(void)
{
    // note: function changes rsp, local stack variables can't be practically used
    __asm__(
            "movq %%rsp, %0;"
            "movq %1, %%rsp;"
            :"=g"(loader_stack)
            :"r"(&stack[INITIAL_STACK_SIZE])
           );

    // Intialize
    init_gdt();
    init_idt();
    init_pic();
    init_timer(1);
    init_keyboard();

    start(
            (char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase,
            &physbase,
            (void*)(uint64_t)loader_stack[4]
         );

    clear_screen();
    set_color(RED, BLACK);
    set_cursor_pos(11, 25);
    printf("________________");
    set_cursor_pos(12, 25);
    printf("|    SBUnix    |");
    set_cursor_pos(13, 25);
    printf("----------------");

    while(1);
}

