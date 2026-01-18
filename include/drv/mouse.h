#pragma once

#include "types.h"

#define MOUSE_LEFTBTN (1 << 0)
#define MOUSE_RIGHTBTN (1 << 1)
#define MOUSE_MIDDLEBTN (1 << 2)

void mouse_send(uint8_t stat, char xmov, char ymov);