#include "kernel.h"

size_t read(int fd, void* buf, size_t count) {
    //
}

size_t write(int fd, void* buf, size_t count) {
    if (fd == STDOUT) {
        char* str = (char*)buf;
        for (int i = 0; i < count; ++i) { putchar(str[i]); }
    } return -1;
}