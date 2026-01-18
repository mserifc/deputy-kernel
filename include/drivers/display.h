#pragma once

#include "types.h"
#include "platform/i386/port.h"

// Define screen width and height for the Command Line Interface (CLI)
#define DISPLAY_CLIWIDTH 80     // 80 columns
#define DISPLAY_CLIHEIGHT 25    // 25 rows

// Enumeration for display color
enum DISPLAY_COLORTABLE {
    DISPLAY_COLOR_BLACK         = 0x00,
    DISPLAY_COLOR_BLUE          = 0x01,
    DISPLAY_COLOR_GREEN         = 0x02,
    DISPLAY_COLOR_CYAN          = 0x03,
    DISPLAY_COLOR_RED           = 0x04,
    DISPLAY_COLOR_MAGENTA       = 0x05,
    DISPLAY_COLOR_BROWN         = 0x06,
    DISPLAY_COLOR_GRAY          = 0x07,
    DISPLAY_COLOR_DARK_GRAY     = 0x08,
    DISPLAY_COLOR_LIGHT_BLUE    = 0x09,
    DISPLAY_COLOR_LIGHT_GREEN   = 0x0A,
    DISPLAY_COLOR_LIGHT_CYAN    = 0x0B,
    DISPLAY_COLOR_LIGHT_RED     = 0x0C,
    DISPLAY_COLOR_LIGHT_MAGENTA = 0x0D,
    DISPLAY_COLOR_YELLOW        = 0x0E,
    DISPLAY_COLOR_WHITE         = 0x0F
};

// Define display color type
typedef enum DISPLAY_COLORTABLE display_color_t;

// Function prototypes
void display_clear();                                   // Clear the entire screen
void display_putchar(char chr, int ptr);                // Print a single character at a specified position (pointer) on the screen
char display_getchar(int ptr);                          // Get the character at a specific screen position (pointer)
void display_enablecursor(uint8_t start, uint8_t end);  // Enable the cursor, specifying the cursor size (not position)
void display_disablecursor();                           // Disable the cursor (hide it from the screen)
void display_putcursor(int ptr);                        // Set the cursor to a specific screen position (pointer)
size_t display_getcursor();                             // Get the current position of the cursor
void display_init();                                    // Initialize display driver

typedef int int32_t;

// 1. Dosya Başlığı (File Header)
typedef struct {
  uint16_t bfType;        // Dosya tipi (BM)
  uint32_t bfSize;        // Dosyanın toplam boyutu
  uint16_t bfReserved1;   // Ayrıcalıklı alan (genellikle 0)
  uint16_t bfReserved2;   // Ayrıcalıklı alan (genellikle 0)
  uint32_t bfOffBits;     // Resim verisinin dosyadaki başlangıç konumu
} BMPFileHeader;

// 2. Bitmap Bilgi Başlığı (Bitmap Information Header)
typedef struct {
  uint32_t biSize;            // Bitmap bilgi başlığının boyutu (genellikle 40 byte)
  int32_t  biWidth;           // Resmin genişliği (piksel cinsinden)
  int32_t  biHeight;          // Resmin yüksekliği (piksel cinsinden)
  uint16_t biPlanes;          // Düzlemler (genellikle 1)
  uint16_t biBitCount;        // Bit derinliği (genellikle 8, 24, 32)
  uint32_t biCompression;     // Sıkıştırma türü (0: yok, 1: RLE8, vb.)
  uint32_t biSizeImage;       // Resim verisinin boyutu
  int32_t  biXPelsPerMeter;   // Yatay piksel yoğunluğu (genellikle 0)
  int32_t  biYPelsPerMeter;   // Dikey piksel yoğunluğu (genellikle 0)
  uint32_t biClrUsed;         // Kullanılan renk sayısı
  uint32_t biClrImportant;    // Önemli renk sayısı
} BMPInfoHeader;

// 3. Renk Tablosu (Color Table) - 8-bit için
typedef struct {
  uint8_t blue;              // Mavi bileşeni
  uint8_t green;             // Yeşil bileşeni
  uint8_t red;               // Kırmızı bileşeni
  // uint8_t reserved;          // Rezerv alan (genellikle 0)
} BMPColor;

// 4. BMP Yapısı - Tam yapı (file + info + pixel verisi)
typedef struct {
  BMPFileHeader fileHeader;      // Dosya başlığı
  BMPInfoHeader infoHeader;      // Bitmap bilgi başlığı
  BMPColor* colorTable;          // Renk tablosu (8-bit için)
  // uint8_t* pixelData;            // Piksel verisi
} BMPImage;

void display_graphic_switch();
uint8_t* display_graphic_getFrameBufferSegment();
void display_graphic_putPixel(uint32_t x, uint32_t y, uint8_t c);
void display_graphic_fillRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t c);
void display_graphic_bmpViewer(void* image);