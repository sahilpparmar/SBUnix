#include <stdlib.h>

void _start(void) {
    int argc = 1;
    char* argv[10];
    int res;

    //init_malloc();
    res = main(argc, argv);
    exit(res);
}
