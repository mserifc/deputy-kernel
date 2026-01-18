#include "common.h"
#include "sysapi/syscall.h"

int syscall_stdio_read(char* buffer, size_t count) {
    char* prompt = scanf("");
    memcpy(buffer, prompt, count - 1);
    buffer[count - 1] = '\0';
}

int syscall_stdio_write(char* buffer, size_t count) {
    int result = 0;
    for (int i = 0; i < count; ++i) {
        putchar(buffer[i]);
        result++;
    }
    return result;
}

void syscall_stdio_init() {
    syscall_addEntry(SYS_READ, (size_t)syscall_stdio_read);
    syscall_addEntry(SYS_WRITE, (size_t)syscall_stdio_write);
}