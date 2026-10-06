
#include "types.h"

typedef struct {
    int width;
    int height;
} pamtools_RawImageInfo_t;

pamtools_RawImageInfo_t* pamtools_loadImage(const char* path);
void pamtools_unloadImage(pamtools_RawImageInfo_t* img);