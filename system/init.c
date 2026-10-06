
#include "deputy.h"

int main() {
    puts("\n----> Initializing System\n");
    puts("\t-> Starting window manager...");
    if (syscall(SYS_EXEC, "/system/wm") != -1) { puts(" OK\n"); }
    else { puts(" ERROR\n"); return 1; }
    putchar('\n'); while(1) { yield(); }
}