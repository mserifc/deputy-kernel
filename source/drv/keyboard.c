#include "drv/keyboard.h"

#include "kernel.h"

bool key_poll(char* data) {
    return false;
}

void key_send(key_t key) {
    iocall_ForceAccess = true;
    int dev = open("/dev/keyboard", O_WRONLY);
    if (dev == -1) { ERR("Unable to open device file '/dev/keyboard'"); goto end; }
    key_t data = key; int sts = write(dev, &data, 1);
    if (sts == -1) { ERR("Unable to write device file '/dev/keyboard'"); goto end; }
    end: close(dev); iocall_ForceAccess = false;
}