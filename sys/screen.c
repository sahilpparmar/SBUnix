//
// screen.c -- get/set routines for access to video memory
//

#include <defs.h>
#include <stdio.h>
#include <screen.h>

#define MAX_ROW 25
#define MAX_COL 80
#define SIZEOF_LINE 160     // MAX_COL * 2 bytes
#define SIZEOF_BUFFER 4000  // MAX_ROW * MAX_COL * 2 bytes

uint64_t START_VADDR; 
uint64_t END_VADDR;         // START_VADDR + SIZEOF_BUFFER 

static uint64_t video_addr;
static uint8_t color_attr;

void init_screen(uint64_t vaddr)
{
    START_VADDR = vaddr; 
    END_VADDR   = vaddr + SIZEOF_BUFFER;
}

static uint64_t get_video_addr()
{
    return video_addr;
}

static void set_video_addr(uint64_t addr)
{
    video_addr = addr;
}

void get_cursor_pos(int32_t *row, int32_t *col)
{
    uint64_t curpos = (get_video_addr() - START_VADDR) / 2;
    *row = curpos / MAX_COL;
    *col = curpos % MAX_COL;
}

void set_cursor_pos(int32_t row, int32_t col)
{
    set_video_addr(START_VADDR + ((row * MAX_COL) + col) * 2);
}

uint8_t get_color()
{
    return color_attr;
}

void set_color(uint8_t fg, uint8_t bg)
{
    color_attr = ((bg << 4) & 0xF0) | (fg & 0x0F);        
}

void scroll(int32_t lines)
{
    uint64_t source, dest;
    int32_t rows, cols, size;

    if (lines >= MAX_COL) {
        clear_screen();
    } else {
        source = START_VADDR + (lines * SIZEOF_LINE);
        dest = START_VADDR;
        size = (MAX_ROW - lines) * SIZEOF_LINE;
        memcpy((void *)dest, (void *)source, size);

        dest = START_VADDR + size;
        size = lines * SIZEOF_LINE;
        memset((void *)dest, 0x0, size);  
    }

    get_cursor_pos(&rows, &cols);
    set_cursor_pos(rows - lines, cols); 
}

void putchar(char mychar)
{
    char *temp;
    uint64_t addr = get_video_addr();

    if(addr >= END_VADDR) {
        scroll(1); 
        addr = get_video_addr();
    }
    
    temp = (char *)addr;
    *temp++ = mychar;
    *temp++ = get_color();
    set_video_addr((uint64_t)temp);
}

void clear_screen()
{
    int32_t i;

    set_cursor_pos(0, 0);

    for(i = 0; i < MAX_ROW * MAX_COL; ++i) {
        putchar(' '); 
    }

    set_cursor_pos(0, 0);
}
