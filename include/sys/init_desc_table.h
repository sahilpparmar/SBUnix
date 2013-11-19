void init_gdt();
void init_idt();
void init_pic();
void init_tss();
void set_tss_rsp0(uint64_t rsp);

