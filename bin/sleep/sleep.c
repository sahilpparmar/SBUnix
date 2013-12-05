#include <stdlib.h>

int main(int argc, char* argv[])
{
    int sec;
    if (argc == 2 && (sec = atoi(argv[1]) > 0)) {
        sleep(atoi(argv[1]) * 1000);
    }
    return 0;
}

