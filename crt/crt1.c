#include <stdlib.h>

void _start(uint64_t stacktop) {
    uint64_t *argc;
    char **argv;
    int res;

    // Find pointers to argc and argv on stack
    argc = (uint64_t*)((void*)&stacktop+0x10UL);
    if (*argc < 1 || *argc > 10) {
        argc = (uint64_t*)((void*)&stacktop+0x20UL);
        argv = (char**)((void*)&stacktop+0x28UL);
    } else {
        argv = (char**)((void*)&stacktop+0x18UL);
    }

    res = main(*argc, argv); 

    exit(res);
}

