#include "misc/pamtools.h"

#include "kernel.h"

pamtools_RawImageInfo_t* pamtools_loadImage(const char* path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) { return NULL; }
    const char* raw = (const char*)mapmem(UINT_MAX, fd);
    close(fd);
    if (raw == NULL) { return NULL; }
    size_t pos = 0;
    char line[128];
    #define READLINE() do { \
        int i = 0; \
        while (raw[pos] != '\n' && i < 127) { line[i++] = raw[pos++]; } \
        line[i] = '\0'; pos++; \
    } while(0)
    READLINE(); // "P7"
    if (line[0] != 'P' || line[1] != '7') {
        unmapmem(raw);
        return NULL;
    }
    int width = -1, height = -1, depth = -1;
    while (1) {
        READLINE();
        if (ncompare(line, "ENDHDR", 6) == 0) { break; }
        else if (ncompare(line, "WIDTH", 5) == 0) { width = atoi(line + 6); }
        else if (ncompare(line, "HEIGHT", 6) == 0) { height = atoi(line + 7); }
        else if (ncompare(line, "DEPTH", 5) == 0) { depth = atoi(line + 6); }
    }
    if (width <= 0 || height <= 0 || depth <= 0) {
        unmapmem(raw);
        return NULL;
    }
    size_t outSize = sizeof(pamtools_RawImageInfo_t) + ((size_t)width * height * 4);
    pamtools_RawImageInfo_t* out = (pamtools_RawImageInfo_t*)mapmem(outSize, -1);
    if (out == NULL) {
        unmapmem(raw);
        return NULL;
    }
    out->width = width;
    out->height = height;
    uint32_t* dst = (uint32_t*)(out + 1);
    const uint8_t* src = (const uint8_t*)(raw + pos);
    for (int i = 0; i < width * height; ++i) {
        uint8_t r = src[i * depth + 0];
        uint8_t g = src[i * depth + 1];
        uint8_t b = src[i * depth + 2];
        uint8_t a = (depth == 4) ? src[i * depth + 3] : 0xFF;
        dst[i] = ((uint32_t)a << 24) | (r << 16) | (g << 8) | b;
    }
    unmapmem(raw); return out;
}

void pamtools_unloadImage(pamtools_RawImageInfo_t* img) {
    if (img == NULL) { return; }
    unmapmem(img);
}