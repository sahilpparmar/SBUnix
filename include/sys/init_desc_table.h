void init_gdt();
void init_idt();
void init_pic();

struct tss_t {
	uint32_t reserved;
	uint64_t rsp0;
	uint32_t unused[11];
}__attribute__((packed)) tss;

void setup_tss();

