//
// irq_handler.c -- High level interrupt service routines and interrupt request handlers
//                  for IRQ0 to IRQ15 
//

#include <defs.h>
#include <stdio.h>
#include <screen.h>
#include <io_common.h>
#include <sys/irq_common.h>
#include <syscall.h>

/****************************************************************
  IRQ1: Keyboard
****************************************************************/

/* Keyboard mapping in lower case */
unsigned char kbsmall[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', '\b', /* Backspace */
    '\t',           /* Tab */
    'q', 'w', 'e', 'r',   /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',   /* Enter key */
    0,          /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
    '\'', '`',   0,      /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',         /* 49 */
    'm', ',', '.', '/',   0,              /* Right shift */
    '*',
    0,    /* Alt */
    ' ',  /* Space bar */
    0,    /* Caps lock */
    0,    /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,    /* < ... F10 */
    0,    /* 69 - Num Lock*/
    0,    /* Scroll Lock */
    0,    /* Home key */
    0,    /* Up Arrow */
    0,    /* Page Up */
    '-',
    0,    /* Left Arrow */
    0,
    0,    /* Right Arrow */
    '+',
    0,    /* 79 - End key*/
    0,    /* Down Arrow */
    0,    /* Page Down */
    0,    /* Insert Key */
    0,    /* Delete Key */
    0,   0,   0,
    0,    /* F11 Key */
    0,    /* F12 Key */
    0,    /* All other keys are undefined */
};

/* Keyboard mapping on SHIFT press */
unsigned char kbcaps[128] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* Curly Brackets */
    '(', ')', '_', '+', '\b', /* Backspace */
    '\t',           /* Tab */
    'Q', 'W', 'E', 'R',   /* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',   /* Enter key */
    0,          /* 29   - Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
    '\"', '~',   0,      /* Left shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',         /* 49 */
    'M', '<', '>', '?',   0,              /* Right shift */
    '*',
    0,    /* Alt */
    ' ',  /* Space bar */
    0,    /* Caps lock */
    0,    /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,    /* < ... F10 */
    0,    /* 69 - Num Lock*/
    0,    /* Scroll Lock */
    0,    /* Home key */
    0,    /* Up Arrow */
    0,    /* Page Up */
    '-',
    0,    /* Left Arrow */
    0,
    0,    /* Right Arrow */
    '+',
    0,    /* 79 - End key*/
    0,    /* Down Arrow */
    0,    /* Page Down */
    0,    /* Insert Key */
    0,    /* Delete Key */
    0,   0,   0,
    0,    /* F11 Key */
    0,    /* F12 Key */
    0,    /* All other keys are undefined */
};      

typedef enum lastKeyPressed {
    NOKEY,
    SHIFT,
    ALT,
    CTRL
} lastKeyPressed_t;

static lastKeyPressed_t lastKeyPressed;
extern volatile int flag, counter;
extern volatile char buf[];


void init_keyboard()
{
    lastKeyPressed = NOKEY;
}

/* Handles the keyboard interrupt */
static void keyboard_handler(registers_t regs)
{
    uint8_t scancode, val;

    // Read from the keyboard's data buffer
    scancode = inb(0x60);
    if (scancode & 0x80) {
        // KeyPressUp Scancodes
        if (scancode == 170 || scancode == 184 || scancode == 157) {
            lastKeyPressed = NOKEY;
        }
    } else {

        /* Scancodes for CTRL , SHIFT , ALT press down are 29, 42, 56 
         * SHIFT : Prints characters in caps and secondary characters on keys
         * ALT   : Prints ~ tilt character followed by the next character pressed
         * CTRL  : Prints ^ character followed by the next character pressed
         */
        if (scancode == 42)
            lastKeyPressed = SHIFT;
        else if (scancode == 56)
            lastKeyPressed = ALT;
        else if (scancode == 29)
            lastKeyPressed = CTRL;
        else {
            switch (lastKeyPressed) {
                case NOKEY:
                    val = kbsmall[scancode];
                    break;
                case CTRL:
                    val = kbsmall[scancode];
                    break;
                case ALT:
                    val = kbsmall[scancode];
                    break;
                case SHIFT:
                    val = kbcaps[scancode];
                    break;
                default:
                    val = 0;
                    break;
            }
            if (flag == 1) {
                if (val == '\n') {
                    buf[counter++] = '\0';
                    flag = 0;
                } else if (val == '\b') {
                    if (get_video_addr() <= last_addr)
                        counter--;
                    else 
                        putchar(val);
                } else {
                    buf[counter++] = val;
                    
                    putchar(val);
                }
                
            }
            
        }
    }       
}

/****************************************************************
 IRQ Common Handler Routine
****************************************************************/
void irq_handler(registers_t regs)
{
    switch (regs.int_no) {
        case 32:
            // Called directly from irq0()
            break;
        case 33:
            keyboard_handler(regs);
            break;
        default:
            break;
    }
    // Send EOI signal to Master PIC
    outb(0x20, 0x20);
}

