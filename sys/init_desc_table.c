//
// init_desc_table.c - Initializes GDT, IDT and PIC.
//

#include <defs.h>
#include <io_common.h>
#include <sys/init_desc_table.h>

/**********************************GDT****************************************/

#define MAX_GDT_ENTRIES 32

extern void load_gdt(uint64_t, uint64_t, uint64_t);

struct gdt_entry_struct
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access_bits;      // Contains TYPE, DT, DPL and P bits
    uint8_t  gran_bits;        // Contains limit_high, AVL, L, D and G bits
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr_struct
{
    uint16_t limit;            // Size of GDT table 
    uint64_t base;             // Base address of GDT table 
} __attribute__((packed));
     
typedef struct gdt_entry_struct gdt_entry_t;
typedef struct gdt_ptr_struct gdt_ptr_t;

gdt_entry_t gdt_entries[MAX_GDT_ENTRIES];
gdt_ptr_t   gdt_ptr;

static void gdt_set_gate(int32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt_entries[index].base_low  = (base & 0xFFFF);
    gdt_entries[index].base_mid  = (base >> 16) & 0xFF;
    gdt_entries[index].base_high = (base >> 24) & 0xFF;

    gdt_entries[index].limit_low = (limit & 0xFFFF);
    gdt_entries[index].access_bits = access;

    gdt_entries[index].gran_bits = (limit >> 16) & 0x0F;
    gdt_entries[index].gran_bits |= gran & 0xF0;
}

// Initialize GDT
void init_gdt()
{
    // Initialize GDTR
    gdt_ptr.limit = sizeof(gdt_entry_t) * MAX_GDT_ENTRIES;
    gdt_ptr.base  = (uint64_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);            // null descriptor 
    gdt_set_gate(1, 0x0, 0x0, 0x9A, 0x20);  // code segment for Ring 0 
    gdt_set_gate(2, 0x0, 0x0, 0x92, 0x20);  // data segment for Ring 0
    gdt_set_gate(3, 0x0, 0x0, 0xFA, 0x20);  // code segment for Ring 3 
    gdt_set_gate(4, 0x0, 0x0, 0xF2, 0x20);  // data segment for Ring 3 

    // Load GDTR
    load_gdt((uint64_t)&gdt_ptr, 0x08, 0x010);
}

/**********************************IDT****************************************/

#define MAX_IDT_ENTRIES 256

extern void load_idtr(uint64_t);

// ISR and IRQ asm routines
extern void isr0();
extern void isr10();
extern void isr13();
extern void isr14();
extern void syscall_handler();

extern void irq0();
extern void irq1();

struct idt_entry_struct
{
    uint16_t target_offset_low;
    uint16_t target_selector;
    uint8_t  ist_reserved_bits;
    uint8_t  access_bits;
    uint16_t target_offset_mid;
    uint32_t target_offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr_struct
{
    uint16_t limit;             // Size of IDT table 
    uint64_t base;              // Base address of IDT table 
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;
typedef struct idt_ptr_struct idt_ptr_t;

idt_entry_t idt_entries[MAX_IDT_ENTRIES];
idt_ptr_t   idt_ptr;

static void idt_set_gate(int32_t index, uint64_t target_offset, uint16_t target_selector, uint8_t access_flags)
{
    idt_entries[index].target_offset_low  = (target_offset & 0xFFFFUL);
    idt_entries[index].target_offset_mid  = (target_offset >> 16) & 0xFFFFUL;
    idt_entries[index].target_offset_high = (target_offset >> 32) & 0xFFFFFFFFUL;

    idt_entries[index].target_selector = target_selector;
    idt_entries[index].access_bits = access_flags;
    
    idt_entries[index].ist_reserved_bits = 0x0;
    idt_entries[index].reserved = 0x0;
}

// Initialize IDT
void init_idt()
{
    // Initialize IDTR
    idt_ptr.limit = sizeof(idt_entry_t) * MAX_IDT_ENTRIES;
    idt_ptr.base  = (uint64_t)&idt_entries;

    // ISRs
    idt_set_gate(0,   (uint64_t)isr0,  0x08, 0x8E);
    idt_set_gate(10,  (uint64_t)isr10, 0x08, 0x8E);
    idt_set_gate(13,  (uint64_t)isr13, 0x08, 0x8E);
    idt_set_gate(14,  (uint64_t)isr14, 0x08, 0x8E);
    idt_set_gate(128, (uint64_t)syscall_handler, 0x08, 0xEE);

    // IRQs
    idt_set_gate(32, (uint64_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint64_t)irq1, 0x08, 0x8E);

    // Load IDTR
    load_idtr((uint64_t)&idt_ptr);
}

/**********************************TSS****************************************/

extern void load_tss();

struct tss_t {
	uint32_t reserved;
	uint64_t rsp0;
	uint32_t unused[11];
}__attribute__((packed)) tss;

struct sys_segment_descriptor {
	uint64_t sd_lolimit:16;/* segment extent (lsb) */
	uint64_t sd_lobase:24; /* segment base address (lsb) */
	uint64_t sd_type:5;    /* segment type */
	uint64_t sd_dpl:2;     /* segment descriptor priority level */
	uint64_t sd_p:1;       /* segment descriptor present */
	uint64_t sd_hilimit:4; /* segment extent (msb) */
	uint64_t sd_xx1:3;     /* avl, long and def32 (not used) */
	uint64_t sd_gran:1;    /* limit granularity (byte/page) */
	uint64_t sd_hibase:40; /* segment base address (msb) */
	uint64_t sd_xx2:8;     /* reserved */
	uint64_t sd_zero:5;    /* must be zero */
	uint64_t sd_xx3:19;    /* reserved */
}__attribute__((packed));

void init_tss() {
    struct sys_segment_descriptor* sd = (struct sys_segment_descriptor*)&gdt_entries[5]; // 6th&7th entry in GDT
    sd->sd_lolimit = sizeof(struct tss_t)-1;
    sd->sd_lobase = ((uint64_t)&tss) & 0xFFFFFFUL;
    sd->sd_type = 9; // 386 TSS
    sd->sd_dpl = 0;
    sd->sd_p = 1;
    sd->sd_hilimit = 0;
    sd->sd_gran = 0;
    sd->sd_hibase = ((uint64_t)&tss) >> 24;

    __asm__ __volatile__("movq %%rsp, %[tss_rsp0];" : [tss_rsp0] "=m" (tss.rsp0));

    load_tss();
}

void set_tss_rsp0(uint64_t rsp)
{
    tss.rsp0 = rsp; 
}

/**********************************PIC****************************************/

void init_pic()
{
    // Master => Control Port: 0x20 and Data Port: 0x21
    // Slave  => Control Port: 0xA0 and Data Port: 0xA1
    
    // ICW1: Initialize Master and Slave to 0x11    
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    
    // ICW2: Remap the IRQ table: Master from 32 to 39 and Slave 40 to 47 
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    
    // ICW3: Tell Master and Slave PICs their cascade identity
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4: 8086/88 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Masking of Ports
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}


