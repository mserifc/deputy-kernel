#include "deputy.h"

#define REGTYPE_REGISTER 1
#define WINDOW_NAME_LENLIMIT 32
typedef struct {
    int type;
    int pid;
    char name[WINDOW_NAME_LENLIMIT];
    int width, height;
} RegisterPacked_t;

int create_window() {
    RegisterPacked_t pack;
    pack.type = REGTYPE_REGISTER;
    pack.pid = 11;
    strcpy(pack.name, "Test Window");
    pack.width = 500;
    pack.height = 300;
    int fd = syscall(SYS_OPEN, "/system/runtime/wm/register.queue", O_WRONLY);
    if (fd == -1) { printf("wintest: error: open\n"); return 1; }
    int sts = syscall(SYS_WRITE, fd, &pack, sizeof(pack));
    if (sts != sizeof(pack)) { printf("wintest: error: write\n"); return 1; syscall(SYS_CLOSE, fd); }
    syscall(SYS_CLOSE, fd);
}

int main() {
    printf("wintest: Test process started\n");
    if (create_window()) { printf("wintest: error: cannot create window"); }
    while  (1) {
        yield();
    }
    return 0;
}