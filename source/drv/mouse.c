#include "drv/mouse.h"

#include "kernel.h"
#include "drv/display.h"

int mouse_PositionX = -1;
int mouse_PositionY = -1;

bool mouse_IsUpdated = true;

void mouse_send(uint8_t stat, char xmov, char ymov) {
    mouse_IsUpdated = true;
    if (mouse_PositionX == -1 ||
        mouse_PositionY == -1) {
        mouse_PositionX = display_Width/2;
        mouse_PositionY = display_Height/2;
    }
    mouse_PositionX += xmov;
    mouse_PositionY -= ymov;
    if (mouse_PositionX >= display_Width) {
        mouse_PositionX = display_Width;
    } else if (mouse_PositionX < 0) {
        mouse_PositionX = 0;
    } if (mouse_PositionY >= display_Height) {
        mouse_PositionY = display_Height;
    } else if (mouse_PositionY < 0) {
        mouse_PositionY = 0;
    }
    iocall_ForceAccess = true;
    int dev = open("/dev/mouse", O_WRONLY);
    if (dev == -1) { ERR("Unable to open device file '/dev/mouse'"); goto end; }
    char data[3] = { (stat&7), xmov, -ymov }; int sts = write(dev, data, 3);
    if (sts == -1) { ERR("Unable to write device file '/dev/mouse'"); goto end; }
    end: close(dev); iocall_ForceAccess = false;
}