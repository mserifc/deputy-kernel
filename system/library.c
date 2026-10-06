
#include "deputy.h"

NAKED int syscall(int num, ...) {
    asm volatile (
        "pushl %ebx\n"
        "pushl %esi\n"
        "pushl %edi\n"
        "movl 16(%esp), %eax\n"
        "movl 20(%esp), %ebx\n"
        "movl 24(%esp), %ecx\n"
        "movl 28(%esp), %edx\n"
        "movl 32(%esp), %esi\n"
        "movl 36(%esp), %edi\n"
        "int $0x80\n"
        "popl %edi\n"
        "popl %esi\n"
        "popl %ebx\n"
        "ret\n"
    );
}

NAKED void yield() { asm volatile ("int $0x9E\nret"); }

/*void exit() {  }

int exec(const char* path) {  }

size_t read(int fd, void* buf, size_t count) {  }

size_t write(int fd, void* buf, size_t count) {  }

int open(const char* path, int flags) {  }

int close(int fd) {  }

void* mapmem(size_t size, int fd) {  }

void unmapmem(void* ptr) {  }*/

extern int main(); void _start() { main(); syscall(SYS_EXIT); }


// Utilities

// ═══════════════════════════════════════════
//  string.h
// ═══════════════════════════════════════════

int strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return p - s;
}

char *strcpy(char *dst, const char *src) {
    int i = 0;
    while ((dst[i] = src[i])) i++;
    return dst;
}

char *strncpy(char *dst, const char *src, int n) {
    int i = 0;
    while (i < n && (dst[i] = src[i])) i++;
    while (i < n) dst[i++] = 0;
    return dst;
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, int n) {
    while (n-- && *a && *a == *b) { a++; b++; }
    return n < 0 ? 0 : (unsigned char)*a - (unsigned char)*b;
}

char *strcat(char *dst, const char *src) {
    char *p = dst + strlen(dst);
    while ((*p++ = *src++));
    return dst;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == c) return (char *)s;
        s++;
    }
    return 0;
}

char *strstr(const char *hay, const char *needle) {
    int nlen = strlen(needle);
    while (*hay) {
        if (strncmp(hay, needle, nlen) == 0) return (char *)hay;
        hay++;
    }
    return 0;
}

char *strtok_r(char *str, const char *delim, char **saved) {
    if (str) *saved = str;
    else if (!*saved) return 0;
    
    while (**saved && strchr(delim, **saved)) (*saved)++;
    if (!**saved) { *saved = 0; return 0; }
    
    char *start = *saved;
    
    while (**saved && !strchr(delim, **saved)) (*saved)++;
    if (**saved) { **saved = '\0'; (*saved)++; }
    else *saved = 0;
    
    return start;
}

// ═══════════════════════════════════════════
//  memory
// ═══════════════════════════════════════════

void *memset(void *dst, int c, int n) {
    unsigned char *p = dst;
    while (n--) *p++ = (unsigned char)c;
    return dst;
}

void *memcpy(void *dst, const void *src, int n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dst;
}

void *memmove(void *dst, const void *src, int n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

int memcmp(const void *a, const void *b, int n) {
    const unsigned char *p = a, *q = b;
    while (n--) {
        if (*p != *q) return *p - *q;
        p++; q++;
    }
    return 0;
}

// ═══════════════════════════════════════════
//  math
// ═══════════════════════════════════════════

int abs(int n) {
    return n < 0 ? -n : n;
}

int pow(int base, int exp) {
    int result = 1;
    while (exp--) result *= base;
    return result;
}

int sqrt(int n) {
    if (n < 0) return -1;
    int x = n, y = (x + 1) / 2;
    while (y < x) { x = y; y = (x + n / x) / 2; }
    return x;
}

int min(int a, int b) { return a < b ? a : b; }
int max(int a, int b) { return a > b ? a : b; }

int clamp(int val, int lo, int hi) {
    return val < lo ? lo : val > hi ? hi : val;
}

// ═══════════════════════════════════════════
//  stdlib
// ═══════════════════════════════════════════

int atoi(const char *s) {
    int result = 0, sign = 1;
    while (*s == ' ') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    while (*s >= '0' && *s <= '9')
        result = result * 10 + (*s++ - '0');
    return result * sign;
}

// itoa: buf need to be at least 12 bytes
char *itoa(int n, char *buf) {
    char tmp[12];
    int i = 0, j = 0;
    int neg = 0;

    if (n < 0) { neg = 1; n = -n; }
    if (n == 0) { tmp[i++] = '0'; }
    while (n > 0) { tmp[i++] = '0' + (n % 10); n /= 10; }
    if (neg) tmp[i++] = '-';

    while (i--) buf[j++] = tmp[i];
    buf[j] = 0;
    return buf;
}

// xtoa: buf need to be at least 9 bytes
char *xtoa(unsigned int n, char *buf) {
    char tmp[8];
    int i = 0, j = 0;
    const char *hex = "0123456789abcdef";

    if (n == 0) { tmp[i++] = '0'; }
    while (n > 0) {
        tmp[i++] = hex[n & 0xF];
        n >>= 4;
    }

    // "0x" prefix istersen:
    // buf[j++] = '0';
    // buf[j++] = 'x';

    while (i--) buf[j++] = tmp[i];
    buf[j] = 0;
    return buf;
}

int isdigit(int c)  { return c >= '0' && c <= '9'; }
int isalpha(int c)  { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
int isalnum(int c)  { return isdigit(c) || isalpha(c); }
int isupper(int c)  { return c >= 'A' && c <= 'Z'; }
int islower(int c)  { return c >= 'a' && c <= 'z'; }
int isspace(int c)  { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
int toupper(int c)  { return islower(c) ? c - 32 : c; }
int tolower(int c)  { return isupper(c) ? c + 32 : c; }

/**
 * @brief Function for print a single character to the standard output
 * 
 * @param chr Specific character to print
 */
void putchar(const char chr) {
    syscall(SYS_WRITE, STDOUT, &chr, 1);
}

/**
 * @brief Function for print a string to the standard output
 * 
 * @param str Specific string to print
 */
void puts(const char* str) {
    syscall(SYS_WRITE, STDOUT, str, strlen(str));
}

/**
 * @brief Function for print formatted output to the standard output
 * 
 * @param format Formatted string to print
 * @param ... Arguments in formatted string
 */
void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char tmp[12];
    for (int i = 0; fmt[i] != '\0'; ++i) {
        if (fmt[i] == '%') {
            switch (fmt[i + 1]) {
                case 'd': {
                    int num = va_arg(args, int);
                    puts(itoa(num, tmp));
                    break;
                }
                case 'x': {
                    uint32_t hex = va_arg(args, uint32_t);
                    puts(xtoa(hex, tmp));
                    break;
                }
                case 'c': {
                    int chr = va_arg(args, int);
                    putchar((char)chr);
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    puts(str);
                    break;
                }
                default: {
                    if (fmt[i + 1] == '%') {
                        putchar('%');
                    } else {
                        putchar('%');
                        putchar(fmt[i + 1]);
                    }
                    break;
                }
            }
            i++;
        } else { putchar(fmt[i]); }
    } va_end(args);
}

int snprintf(char *buffer, size_t size, const char *fmt, ...) {
    if (size == 0) { return 0; }

    va_list args;
    size_t written = 0;
    va_start(args, fmt);

    for (int i = 0; fmt[i] != '\0' && written < size - 1; ++i) {
        if (fmt[i] == '%') {
            i++;
            if (fmt[i] == '\0') break;

            switch (fmt[i]) {
                case 'd': {
                    int num = va_arg(args, int);
                    char numBuf[12];
                    itoa(num, numBuf);
                    for (int j = 0; numBuf[j] != '\0' && written < size - 1; j++, written++) {
                        buffer[written] = numBuf[j];
                    }
                    break;
                }
                case 'x': {
                    unsigned int hex = va_arg(args, unsigned int);
                    char hexBuf[9];
                    xtoa(hex, hexBuf);
                    for (int j = 0; hexBuf[j] != '\0' && written < size - 1; j++, written++) {
                        buffer[written] = hexBuf[j];
                    }
                    break;
                }
                case 'c': {
                    int chr = va_arg(args, int);
                    if (written < size - 1) {
                        buffer[written++] = (char)chr;
                    }
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char*);
                    for (int j = 0; str[j] != '\0' && written < size - 1; j++, written++) {
                        buffer[written] = str[j];
                    }
                    break;
                }
                default: {
                    buffer[written++] = '%';
                    if (written < size - 1) {
                        buffer[written++] = fmt[i];
                    }
                    break;
                }
            }
        } else {
            buffer[written++] = fmt[i];
        }
    }

    buffer[written] = '\0';
    va_end(args);
    return (int)written;
}

int display_fd = -1;
static int get_display_fd() {
    if (display_fd == -1) {
        display_fd = syscall(SYS_OPEN, "/dev/display", O_WRONLY);
    } return display_fd;
}

static display_Pkg_t draw_batch[64];
static int draw_batch_cnt = 0;
void drawFlush() {
    if (!draw_batch_cnt) return;
    syscall(SYS_WRITE, get_display_fd(), draw_batch, draw_batch_cnt * sizeof(display_Pkg_t));
    draw_batch_cnt = 0;
}

int drawRect(int x, int y, int w, int h, uint32_t c) {
    //if (draw_batch_cnt==sizeof(draw_batch)/sizeof(display_Pkg_t)) { return -1; }
    draw_batch[draw_batch_cnt++] = (display_Pkg_t){
        .op = DISPLAY_OP_RECT,
        .c = c,
        .x0 = x,
        .y0 = y,
        .x1 = x+w,
        .y1 = y+h
    };
    return 0;
}

int drawLine(int x0, int y0, int x1, int y1, uint32_t c) {
    draw_batch[draw_batch_cnt++] = (display_Pkg_t){
        .op = DISPLAY_OP_LINE,
        .c = c,
        .x0 = x0,
        .y0 = y0,
        .x1 = x1,
        .y1 = y1
    };
    return 0;
}

int drawTri(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t c) {
    draw_batch[draw_batch_cnt++] = (display_Pkg_t){
        .op = DISPLAY_OP_TRI,
        .c = c,
        .x0 = x0,
        .y0 = y0,
        .x1 = x1,
        .y1 = y1,
        .x2 = x2,
        .y2 = y2
    };
    return 0;
}

int drawText(int x0, int y0, const char* str, int len, int size, uint32_t c) {
    draw_batch[draw_batch_cnt++] = (display_Pkg_t){
        .op = DISPLAY_OP_TEXT,
        .c = c,
        .x0 = x0,
        .y0 = y0,
        .x1 = (int)((size_t)str),
        .x2 = len,
        .y2 = size
    };
    return 0;
}

int drawRaw(uint32_t* buffer, int x, int y, int w, int h) {
    draw_batch[draw_batch_cnt++] = (display_Pkg_t){
        .op = DISPLAY_OP_RAW,
        .c = (int)((size_t)buffer),
        .x0 = x,
        .y0 = y,
        .x1 = w,
        .y1 = h
    };
    return 0;
}

int drawImage(uint32_t* buffer, int x, int y, int w, int h) {
    draw_batch[draw_batch_cnt++] = (display_Pkg_t){
        .op = DISPLAY_OP_IMG,
        .c = (int)((size_t)buffer),
        .x0 = x,
        .y0 = y,
        .x1 = w,
        .y1 = h
    };
    return 0;
}

RawImageInfo_t* loadImage(const char* path) {
    // 1. PAM dosyasını mmap ile aç (kaynak veri)
    int fd = syscall(SYS_OPEN, path, O_RDONLY);
    if (fd == -1) { return NULL; }
    const char* raw = (const char*)syscall(SYS_MAPMEM, UINT_MAX, fd);
    syscall(SYS_CLOSE, fd);
    if (raw == NULL) { return NULL; }

    // 2. PAM header'ını parse et
    size_t pos = 0;
    char line[128];
    #define READLINE() do { \
        int i = 0; \
        while (raw[pos] != '\n' && i < 127) { line[i++] = raw[pos++]; } \
        line[i] = '\0'; pos++; \
    } while(0)

    READLINE(); // "P7"
    if (line[0] != 'P' || line[1] != '7') {
        syscall(SYS_UNMAPMEM, raw);
        return NULL;
    }

    int width = -1, height = -1, depth = -1;
    while (1) {
        READLINE();
        if (strncmp(line, "ENDHDR", 6) == 0) { break; }
        else if (strncmp(line, "WIDTH", 5) == 0) { width = atoi(line + 6); }
        else if (strncmp(line, "HEIGHT", 6) == 0) { height = atoi(line + 7); }
        else if (strncmp(line, "DEPTH", 5) == 0) { depth = atoi(line + 6); }
    }
    if (width <= 0 || height <= 0 || depth <= 0) {
        syscall(SYS_UNMAPMEM, raw);
        return NULL;
    }

    // 3. Çıktı buffer'ını mmap ile ayır (anonim bellek)
    size_t outSize = sizeof(RawImageInfo_t) + ((size_t)width * height * 4);
    RawImageInfo_t* out = (RawImageInfo_t*)syscall(SYS_MAPMEM, outSize, -1);
    if (out == NULL) {
        syscall(SYS_UNMAPMEM, raw);
        return NULL;
    }

    out->width = width;
    out->height = height;

    // 4. PAM piksellerini ARGB'ye çevirip çıktı buffer'ına yaz
    uint32_t* dst = (uint32_t*)(out + 1);
    const uint8_t* src = (const uint8_t*)(raw + pos);
    for (int i = 0; i < width * height; ++i) {
        uint8_t r = src[i * depth + 0];
        uint8_t g = src[i * depth + 1];
        uint8_t b = src[i * depth + 2];
        uint8_t a = (depth == 4) ? src[i * depth + 3] : 0xFF;
        dst[i] = ((uint32_t)a << 24) | (r << 16) | (g << 8) | b;
    }

    // 5. Kaynak PAM mapping'ine artık ihtiyaç yok
    syscall(SYS_UNMAPMEM, raw);

    return out;
}

void unloadImage(RawImageInfo_t* img) {
    if (img == NULL) { return; }
    syscall(SYS_UNMAPMEM, img);
}