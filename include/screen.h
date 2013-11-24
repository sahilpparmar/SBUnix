#include <defs.h>

#define BLACK          0
#define BLUE           1
#define GREEN          2
#define CYAN           3
#define RED            4
#define MAGENTA        5
#define BROWN          6
#define LIGHT_GREY     7
#define DARK_GREY      8
#define LIGHT_BLUE     9 
#define LIGHT_GREEN   10
#define LIGHT_CYAN    11
#define LIGHT_RED     12
#define LIGHT_MAGENTA 13
#define LIGHT_BROWN   14
#define WHITE         15

uint64_t get_video_addr();
void set_video_addr(uint64_t addr);

void get_cursor_pos(int32_t *row, int32_t *col);
void set_cursor_pos(int32_t row, int32_t col);

uint8_t get_color();
void set_color(uint8_t fg, uint8_t bg);

void init_screen();
void clear_screen();
void scroll(int32_t lines);

void newline();
void newtab();
void backspace();
