#pragma once

#include "types.h"

#define DISPLAY_OP_DOT  1
#define DISPLAY_OP_RECT 2
#define DISPLAY_OP_LINE 3
#define DISPLAY_OP_TRI  4
#define DISPLAY_OP_TEXT 5
#define DISPLAY_OP_RAW  6
#define DISPLAY_OP_IMG  7

typedef struct {
    int width;
    int height;
    int pitch;
    int depth;
} display_Info_t;

typedef struct {
    int op;
    uint32_t c; // 0xAARRGGBB or BB GG RR AA
    int x0, y0,
        x1, y1,
        x2, y2;
} display_Pkg_t;

extern void* display_VRAM;
extern int display_Width;
extern int display_Height;
extern int display_Pitch;
extern int display_Depth;

void display_proc(void);