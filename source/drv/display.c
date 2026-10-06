#include "drv/display.h"

#include "kernel.h"
#include "hw/simd.h"
#include "drv/mouse.h"

#include "misc/Roboto_32.h"
#include "misc/pamtools.h"

void* display_VRAM = NULL;
void* display_CachedVRAM = NULL;
int display_Width;
int display_Height;
int display_Pitch;
int display_Depth;

bool display_FrontPrintMode = false;

pamtools_RawImageInfo_t* display_CursorImage = NULL;

static int display_fd = -1;
display_Pkg_t display_Cache[MEMORY_BLKSIZE/sizeof(display_Pkg_t)];

static inline void putpx(int x, int y, uint32_t c) {
    if (!display_CachedVRAM ||
        !(c & 0xFF000000)) return;
    if (x >= display_Width ||
        y >= display_Height ||
        x < 0 || y < 0) return;
    uint32_t* addr = (uint32_t*)(
        (uint8_t*)(display_FrontPrintMode ?
        display_VRAM : display_CachedVRAM)
        + y * display_Pitch
        + x * (display_Depth / 8)
    );
    *addr = c;
}

int putchr(int x, int y, char chr, int size, uint32_t c) {
    const int source_height = 32;
    const int avg_width = 20;
    if (size <= 0) return 0;
    if (chr < 33 || chr > 126)
        return (avg_width * size + source_height / 2) / source_height;

    int idx = chr - 33;
    int source_width = char_width[idx];
    if (source_width <= 0) return 0;
    const uint8_t* bm = (const uint8_t*)char_addr[idx];
    if (!bm) return 0;

    int target_width = (source_width * size + source_height / 2) / source_height;
    if (target_width <= 0) return 0;

    int scale_y = (source_height * 256) / size;
    int scale_x = (source_width  * 256) / target_width;

    // Clip
    int draw_cols = target_width;
    int draw_rows = size;
    if (x + draw_cols > display_Width)  draw_cols = display_Width  - x;
    if (y + draw_rows > display_Height) draw_rows = display_Height - y;
    if (x < 0 || y < 0 || draw_cols <= 0 || draw_rows <= 0) return target_width;

    for (int t_row = 0; t_row < draw_rows; ++t_row) {
        int s_row  = (t_row * scale_y) >> 8;
        int s_page = s_row >> 3;
        int s_bit  = s_row &  7;
        const uint8_t* row_bm = bm + s_page * source_width;
        uint32_t* dst_row = (uint32_t*)(
            (uint8_t*)display_CachedVRAM
            + (y + t_row) * display_Pitch
            + x * (display_Depth / 8)
        );
        for (int t_col = 0; t_col < draw_cols; ++t_col) {
            int s_col = (t_col * scale_x) >> 8;
            if (s_col < source_width && (row_bm[s_col] & (1 << s_bit)))
                dst_row[t_col] = c;
        }
    }
    return target_width;
}

void putstr(int x, int y, const char* str, int len, int size, uint32_t c) {
    int cx = x;
    for (int i = 0; i < len; ++i) {
        if (str[i] == ' ') {
            cx += (10 * size + 16) / 32;
        } else {
            int w = putchr(cx, y, str[i], size, c);
            if (w > 0) cx += w + 1;
        }
    }
}

void display_proc() {
    if (!display_CachedVRAM) {
        display_CachedVRAM = calloc(display_Height * display_Pitch, 1);
        if (!display_CachedVRAM) { PANIC("Out of memory"); }
    }

    if (display_fd == -1) {
        iocall_ForceAccess = true;
        display_fd = open("/dev/display", O_RDONLY);
        if (display_fd == -1) { ERR("Unable to open device file '/dev/display'"); iocall_ForceAccess = false; return; }
    }

    iocall_ForceAccess = true;
    int sts = read(display_fd, display_Cache, sizeof(display_Cache));
    iocall_ForceAccess = false;
    if (sts <= 0) goto flush;

    for (size_t pidx = 0; pidx < (size_t)(sts / sizeof(display_Pkg_t)); ++pidx) {
        display_Pkg_t* p = &display_Cache[pidx];

        if (p->op == DISPLAY_OP_DOT) {
            if (p->x0 >= 0 && p->x0 < display_Width &&
                p->y0 >= 0 && p->y0 < display_Height)
                putpx(p->x0, p->y0, p->c);

        } else if (p->op == DISPLAY_OP_RECT) {
            if ((p->c & 0xFF000000) == 0) continue;
            int x0 = p->x0 < 0 ? 0 : p->x0;
            int y0 = p->y0 < 0 ? 0 : p->y0;
            int x1 = p->x1 > display_Width  ? display_Width  : p->x1;
            int y1 = p->y1 > display_Height ? display_Height : p->y1;
            if (x0 >= x1 || y0 >= y1) continue;
            int row_bytes = (x1 - x0) * (display_Depth / 8);
            for (int i = y0; i < y1; ++i) {
                uint32_t* row = (uint32_t*)((uint8_t*)display_CachedVRAM + i * display_Pitch + x0 * (display_Depth / 8));
                // fill row with simd_copy if color is zero, else manual
                /*if (simd_SSEActivated) {
                    simd_extfill(row, p->c, (x1 - x0) * (display_Depth / 8));
                } else {
                    for (int j = 0; j < (x1 - x0); ++j)
                        row[j] = p->c;
                }*/
                extfill(row, p->c, (x1 - x0) * (display_Depth / 8));
            }

        } else if (p->op == DISPLAY_OP_LINE) {
            int dx = p->x1 - p->x0;
            int dy = p->y1 - p->y0;
            int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
            if (steps == 0) {
                if (p->x0 >= 0 && p->x0 < display_Width &&
                    p->y0 >= 0 && p->y0 < display_Height)
                    putpx(p->x0, p->y0, p->c);
                continue;
            }
            for (int i = 0; i <= steps; ++i) {
                int px = p->x0 + dx * i / steps;
                int py = p->y0 + dy * i / steps;
                if (px >= 0 && px < display_Width &&
                    py >= 0 && py < display_Height)
                    putpx(px, py, p->c);
            }

        } else if (p->op == DISPLAY_OP_TRI) {
            /*if (p->y0 > p->y1) { swap(&p->x0,&p->x1); swap(&p->y0,&p->y1); }
            if (p->y1 > p->y2) { swap(&p->x1,&p->x2); swap(&p->y1,&p->y2); }
            if (p->y0 > p->y1) { swap(&p->x0,&p->x1); swap(&p->y0,&p->y1); }
            if (p->y2 == p->y0) continue;
            int scan_y0 = p->y0 < 0 ? 0 : p->y0;
            int scan_y1 = p->y2 > display_Height ? display_Height : p->y2;
            for (int y = scan_y0; y <= scan_y1; y++) {
                int xa = p->x0 + (p->x2 - p->x0) * (y - p->y0) / (p->y2 - p->y0);
                int xb;
                if (y <= p->y1 && p->y1 != p->y0)
                    xb = p->x0 + (p->x1 - p->x0) * (y - p->y0) / (p->y1 - p->y0);
                else if (y > p->y1 && p->y2 != p->y1)
                    xb = p->x1 + (p->x2 - p->x1) * (y - p->y1) / (p->y2 - p->y1);
                else
                    xb = p->x1;
                if (xa > xb) { int t = xa; xa = xb; xb = t; }
                xa = xa < 0 ? 0 : xa;
                xb = xb > display_Width ? display_Width : xb;
                for (int x = xa; x <= xb; x++)
                    putpx(x, y, p->c);
            }*/
            if (p->y0 > p->y1) { swap(&p->x0,&p->x1); swap(&p->y0,&p->y1); }
            if (p->y1 > p->y2) { swap(&p->x1,&p->x2); swap(&p->y1,&p->y2); }
            if (p->y0 > p->y1) { swap(&p->x0,&p->x1); swap(&p->y0,&p->y1); }
            if (p->y2 == p->y0) continue;
            if ((p->c & 0xFF000000) == 0) continue;
            int scan_y0 = p->y0 < 0 ? 0 : p->y0;
            int scan_y1 = p->y2 > display_Height ? display_Height : p->y2;
            for (int y = scan_y0; y <= scan_y1; y++) {
                int xa = p->x0 + (p->x2 - p->x0) * (y - p->y0) / (p->y2 - p->y0);
                int xb;
                if (y <= p->y1 && p->y1 != p->y0)
                    xb = p->x0 + (p->x1 - p->x0) * (y - p->y0) / (p->y1 - p->y0);
                else if (y > p->y1 && p->y2 != p->y1)
                    xb = p->x1 + (p->x2 - p->x1) * (y - p->y1) / (p->y2 - p->y1);
                else
                    xb = p->x1;
                if (xa > xb) { int t = xa; xa = xb; xb = t; }
                xa = xa < 0 ? 0 : xa;
                xb = xb > display_Width ? display_Width : xb;
                if (xa >= xb) continue;

                uint32_t* row = (uint32_t*)((uint8_t*)display_CachedVRAM
                    + y * display_Pitch + xa * (display_Depth / 8));
                extfill(row, p->c, (xb - xa) * (display_Depth / 8));
            }

        } else if (p->op == DISPLAY_OP_TEXT) {
            putstr(p->x0, p->y0,
                (const char*)((size_t)p->x1 + kernel_UserlandBase),
                p->x2, p->y2, p->c);

        } else if (p->op == DISPLAY_OP_RAW) {
            uint32_t* src = (uint32_t*)((size_t)p->c + kernel_UserlandBase);
            for (int col = 0; col < p->y1; ++col) {
                for (int row = 0; row < p->x1; ++row) {
                    putpx(p->x0+row, p->y0+col, src[col * p->x1 + row]);
                }
            }
            // ----
            /*int x0 = p->x0 < 0 ? 0 : p->x0;
            int y0 = p->y0 < 0 ? 0 : p->y0;
            int x1 = x0 + p->x1 > display_Width  ? display_Width  - x0 : p->x1;
            int y1 = y0 + p->y1 > display_Height ? display_Height - y0 : p->y1;
            if (x1 <= 0 || y1 <= 0) continue;
            int row_bytes = x1 * (display_Depth / 8);
            for (int row = 0; row < y1; row++) {
                uint8_t* dst_row = (uint8_t*)display_CachedVRAM
                    + (y0 + row) * display_Pitch
                    + x0 * (display_Depth / 8);
                    uint8_t* src_row = src
                    + (y0 + row) * p->x1 * (display_Depth / 8)
                    + x0 * (display_Depth / 8);
                ncopy(dst_row, src_row, row_bytes);
            }*/

        } else if (p->op == DISPLAY_OP_IMG) {
            /*int width  = p->x1;
            int height = p->y1;
            uint32_t* src = (uint32_t*)((size_t)p->c + kernel_UserlandBase);
            int bpp = display_Depth / 8;
            int x0 = p->x0 < 0 ? 0 : p->x0;
            int y0 = p->y0 < 0 ? 0 : p->y0;
            int x1 = (p->x0 + width)  > display_Width  ? display_Width  : (p->x0 + width);
            int y1 = (p->y0 + height) > display_Height ? display_Height : (p->y0 + height);
            if (x0 >= x1 || y0 >= y1) continue;
            int src_off_x = x0 - p->x0;
            int src_off_y = y0 - p->y0;
            int copy_w = x1 - x0;
            size_t copy_bytes = (size_t)copy_w * bpp;
            for (int row = y0; row < y1; ++row) {
                uint8_t* dstRow = (uint8_t*)display_CachedVRAM + row * display_Pitch + x0 * bpp;
                uint32_t* srcRow = src + (row - y0 + src_off_y) * width + src_off_x;
                ncopy(dstRow, (uint8_t*)srcRow, copy_bytes);
            }*/
            int width  = p->x1;
            int height = p->y1;
            if (width <= 0 || height <= 0) continue;
            uint32_t* src = (uint32_t*)((size_t)p->c + kernel_UserlandBase);
            int bpp = display_Depth / 8;
            int x0 = p->x0 < 0 ? 0 : p->x0;
            int y0 = p->y0 < 0 ? 0 : p->y0;
            int x1 = (p->x0 + width)  > display_Width  ? display_Width  : (p->x0 + width);
            int y1 = (p->y0 + height) > display_Height ? display_Height : (p->y0 + height);
            if (x0 >= x1 || y0 >= y1) continue;
            int src_off_x = x0 - p->x0;
            int src_off_y = y0 - p->y0;
            if (src_off_x >= width || src_off_y >= height) continue;
            int copy_w = x1 - x0;
            size_t copy_bytes = (size_t)copy_w * bpp;
            for (int row = y0; row < y1; ++row) {
                uint8_t* dstRow = (uint8_t*)display_CachedVRAM + (size_t)row * display_Pitch + (size_t)x0 * bpp;
                uint32_t* srcRow = src + (size_t)(row - y0 + src_off_y) * (size_t)width + src_off_x;
                ncopy(dstRow, (uint8_t*)srcRow, copy_bytes);
            }

        } else {
            continue;
        }
    }

    flush:
    if (!mouse_IsUpdated && sts <= 0) return;
    ncopy(display_VRAM, display_CachedVRAM,
        display_Height * display_Pitch);
    // ----
    display_FrontPrintMode = true;
    if (!display_CursorImage) {
        display_CursorImage = pamtools_loadImage("/system/assets/cursor.pam");
    } if (display_CursorImage ) {
        if (mouse_PositionX >= 0 &&
            mouse_PositionY >= 0 &&
            mouse_PositionX < display_Width &&
            mouse_PositionY < display_Height) {
            pamtools_RawImageInfo_t* info = display_CursorImage;
            uint32_t* data = (uint32_t*)(display_CursorImage + 1);
            for (int col = 0; col < info->height; ++col) {
                for (int row = 0; row < info->width; ++row) {
                    putpx(mouse_PositionX+row, mouse_PositionY+col, data[col * info->width + row]);
                }
            }
        }
    } else {
        const int pointer_size = 8;
        for (int i = mouse_PositionY-pointer_size; i < mouse_PositionY+pointer_size; ++i) {
            uint32_t* addr = (uint32_t*)(
                (uint8_t*)display_VRAM
                + (i+1) * display_Pitch
                + (mouse_PositionX+1) * (display_Depth / 8)
            ); *addr = 0xFF000000;
        }
        for (int j = mouse_PositionX-pointer_size; j < mouse_PositionX+pointer_size; ++j) {
            putpx(j+1, mouse_PositionY+1, 0xFF000000);
        }
        for (int i = mouse_PositionY-pointer_size; i < mouse_PositionY+pointer_size; ++i) {
            uint32_t* addr = (uint32_t*)(
                (uint8_t*)display_VRAM
                + i * display_Pitch
                + mouse_PositionX * (display_Depth / 8)
            ); *addr = 0xFFFFFFFF;
        }
        for (int j = mouse_PositionX-pointer_size; j < mouse_PositionX+pointer_size; ++j) {
            putpx(j, mouse_PositionY, 0xFFFFFFFF);
        }
    } display_FrontPrintMode = false;
    mouse_IsUpdated = false;
}