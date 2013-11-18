int printf(const char *format, ...) {
    // Store the pointer in the rdx register
    __asm__ __volatile__("movq %[s], %%rdx;" : : [s]"m"(format));
    // Store the system call index in the rax register
    __asm__ __volatile__("movq $0, %rax;");
    __asm__ __volatile__("int $0x80;");
    return 0;
}

