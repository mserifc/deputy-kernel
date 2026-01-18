#include "drv/mouse.h"

#include "kernel.h"

void mouse_send(uint8_t stat, char xmov, char ymov) {
    iocall_ForceAccess = true;
    int dev = open("/dev/mouse", O_WRONLY);
    if (dev == -1) { ERR("Unable to open device file '/dev/mouse'"); goto end; }
    char data[3] = { (stat&7), xmov, ymov }; int sts = write(dev, data, 3);
    if (sts == -1) { ERR("Unable to write device file '/dev/mouse'"); goto end; }
    end: close(dev); iocall_ForceAccess = false;
}