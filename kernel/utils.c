#include "utils.h"

// RTC (Real-Time Clock) date result
date_t utils_Date;

// Splitted string tokens result
tokens_t utils_Tokens;

// String buffer for function results
char utils_StringBuffer[64];

// // Text memory for CLI (Command Line Interface) display
// char utils_TextMemory[DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT];

// Cursor location in CLI (Command Line Interface) display
int utils_CursorLocation = 0;//DISPLAY_CLIWIDTH * (DISPLAY_CLIHEIGHT - 1);

// Buffer for keep user prompt input
char utils_PromptBuffer[UTILS_PROMPTBUFFER_LENGTH];
int utils_PromptPointer = 0;    // Prompt pointer

// Private functions

uint64_t utils_rdtsc() {
    uint32_t low, high;
    asm volatile ("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

// Function for convert binary-coded decimal to decimal
uint8_t utils_bcd2dec(uint8_t bcd) { return (bcd & 0x0F) + ((bcd >> 4) * 10); }

// Function for read RTC (Real-Time Clock)
void utils_readRTC (
    uint8_t* sec,
    uint8_t* min,
    uint8_t* hour,
    uint8_t* day,
    uint8_t* mon,
    uint16_t* year
) {
    port_outb(UTILS_RTC_INDEXPORT, 0x00);
    *sec = port_inb(UTILS_RTC_DATAPORT);

    port_outb(UTILS_RTC_INDEXPORT, 0x02);
    *min = port_inb(UTILS_RTC_DATAPORT);

    port_outb(UTILS_RTC_INDEXPORT, 0x04);
    *hour = port_inb(UTILS_RTC_DATAPORT);

    port_outb(UTILS_RTC_INDEXPORT, 0x07);
    *day = port_inb(UTILS_RTC_DATAPORT);

    port_outb(UTILS_RTC_INDEXPORT, 0x08);
    *mon = port_inb(UTILS_RTC_DATAPORT);

    port_outb(UTILS_RTC_INDEXPORT, 0x09);
    *year = port_inb(UTILS_RTC_DATAPORT);

    *sec = utils_bcd2dec(*sec);
    *min = utils_bcd2dec(*min);
    *hour = utils_bcd2dec(*hour);
    *day = utils_bcd2dec(*day);
    *mon = utils_bcd2dec(*mon);
    *year = utils_bcd2dec(*year) + 2000;
}

// Function for scroll down the CLI Display
void utils_scrolldown(int times) {
    if (times < 0) { return; }
    for (int i = 0; i < times; ++i) {
        for (int j = 0; j < (DISPLAY_CLIWIDTH * (DISPLAY_CLIHEIGHT - 1)); ++j) {
            display_putchar(display_getchar(j + DISPLAY_CLIWIDTH), j);
        }
        for (int j = ((DISPLAY_CLIHEIGHT - 1) * DISPLAY_CLIWIDTH); j < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++j) {
            display_putchar('\0', j);
        }
    }
}

// Function for convert a seed to a pseudo-random 32-bit integer using XORShift algorithm
uint32_t xorshift32(uint32_t state) {
    state ^= state << 21;
    state ^= state >> 31;
    state ^= state << 4;
    return state * 2685821657736338717LL;
}

// Function for encrypt or decrypt data with a key
void xorcipher(char* input, char* key) {
    size_t input_len = length(input);
    size_t key_len = length(key);
    for (size_t i = 0; i < input_len; i++) {
        input[i] ^= key[i % key_len];
    }
}

// Function for hash the given data using FNV-1a hash algorithm
uint32_t fnv1ahash(uint8_t* data, size_t len) {
    uint32_t hash = 0x811C9DC5;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i]; hash *= 0x01000193;
    } return hash;
}

// Public functions

/**
 * @brief Function for get current RTC (Real-Time Clock) date
 * 
 * @return Date result
 */
date_t date() {
    utils_readRTC(
        &utils_Date.sec,
        &utils_Date.min,
        &utils_Date.hour,
        &utils_Date.day,
        &utils_Date.mon,
        &utils_Date.year
    );
    return utils_Date;
}

/**
 * @brief Function for introduce a delay (based on CPU speed)
 * 
 * @param ms Milliseconds
 */
void delay(uint32_t ms) {
    extern kernel_CPUInfo_t kernel_CPUInfo;
    uint64_t time = (kernel_CPUInfo.frequency * ms) / 1024;
    uint64_t end = time + utils_rdtsc();
    while (utils_rdtsc() < end) { asm volatile ("nop"); }
}

/**
 * @brief Function for sleep the system for a certain amount of time (based on RTC)
 * 
 * @param sec Seconds
 */
void sleep(uint32_t sec) {
    date_t current, start = date();
    uint32_t targetTime =
        (start.day * 24 * 60 * 60) +
        (start.hour * 60 * 60) +
        (start.min * 60) +
        start.sec + sec;
    while (true) {
        current = date(); if (
            (current.day * 24 * 60 * 60) +
            (current.hour * 60 * 60) +
            (current.min * 60) +
            current.sec >= targetTime
        ) { return; } yield();
    }
}

/**
 * @brief Function for fill a block of memory with specific value
 * 
 * @param ptr Pointer of start address
 * @param chr Specific character to fill
 * @param len Length to be filled
 * 
 * @return Pointer of start address
 */
void* fill(void* ptr, char chr, size_t len) {
    uint8_t* point = ptr;
    for (size_t i = 0; i < len; i++) {
        point[i] = (uint8_t)chr;
    }
    return ptr;
}

/**
 * @brief Function for copy a block of memory from source to destination
 * 
 * @param dest Target destination
 * @param src Source to copy
 * 
 * @return Target destination
 */
char* copy(char* dest, char* src) {
    char* original_dest = dest;
	while (*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	}
	*dest = '\0';
	return original_dest;
}

/**
 * @brief Function for copy a block of memory from source to destination with limit
 * 
 * @param dest Target destination
 * @param src Source to copy
 * @param len Length to be copyed
 * 
 * @return Target destination
 */
void* ncopy(void* dest, void* src, size_t len) {
    uint8_t* destination = dest;
    const uint8_t* source = src;
    for (size_t i = 0; i < len; i++) {
        destination[i] = source[i];
    }
    return dest;
}

/**
 * @brief Function for compare two strings
 * 
 * @param str1 String 1
 * @param str2 String 2
 * 
 * @return Returns 0 if there is no difference between strings
 */
int compare(char* str1, char* str2) {
    while (*str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	}
	return *(unsigned char*)str1 - *(unsigned char*)str2;
}

/**
 * @brief Function for compare two buffers with limit
 * 
 * @param ptr1 Pointer of first buffer
 * @param ptr2 Pointer of second buffer
 * 
 * @return Returns 0 if there is no difference between buffers
 */
int ncompare(void* ptr1, void* ptr2, size_t len) {
    unsigned char* p1 = (unsigned char*)ptr1;
    unsigned char* p2 = (unsigned char*)ptr2;
    for (size_t i = 0; i < len; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}

/**
 * @brief Function for get length of specific string
 * 
 * @param str Specific string
 * 
 * @return Length of specific string
 */
int length(char* str) {
    int len = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        len++;
    }
    return len;
}

/**
 * @brief Function for split a string by a deliminer
 * 
 * @param str Specific string
 * @param deli Deliminer
 * 
 * @return Tokens
 */
tokens_t* split(char* str, char deli) {
    for (int i = 0; i < UTILS_SPLIT_MAXTOKENCOUNT; ++i) {
        for (int j = 0; j < UTILS_SPLIT_MAXTOKENLENGTH; ++j) {
            utils_Tokens.v[i][j] = '\0';
        }
    } utils_Tokens.c = 0;
    int c = 0;
    bool in_token = false;
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == deli) {
            if (in_token) {
                utils_Tokens.v[utils_Tokens.c][c] = '\0';
                utils_Tokens.c++;
                c = 0;
                in_token = false;
                if (utils_Tokens.c >= UTILS_SPLIT_MAXTOKENCOUNT) break;
            }
        } else {
            if (c < UTILS_SPLIT_MAXTOKENLENGTH - 1) {
                utils_Tokens.v[utils_Tokens.c][c++] = str[i];
                in_token = true;
            }
        }
    }
    if (in_token && utils_Tokens.c < UTILS_SPLIT_MAXTOKENCOUNT) {
        utils_Tokens.v[utils_Tokens.c][c] = '\0';
        utils_Tokens.c++;
    }
    return &utils_Tokens;
}

/**
 * @brief Function for convert ASCII string to integer
 * 
 * @param str Specific number string for convert
 * 
 * @return Integer version of string
 */
int convert_atoi(char* str) {
    int result = 0;
    int sign = 1;
    int ptr = 0;
    while (str[ptr] == ' ') {
        ptr++;
    }
    if (str[ptr] == '-') {
        sign = -1;
        ptr++;
    } else if (str[ptr] == '+') {
        ptr++;
    }
    while (str[ptr] >= '0' && str[ptr] <= '9') {
        result = result * 10 + (str[ptr] - '0');
        ptr++;
    }
    return sign * result;
}

/**
 * @brief Function for convert integer to ASCII string
 * 
 * @param num Specific integer for convert
 * 
 * @return String version of integer
 */
char* convert_itoa(int num) {
    fill(utils_StringBuffer, 0, length(utils_StringBuffer));
    int temp = num;
    int digit_count = 0;
    int i;
    int is_negative = 0;
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    do {
        digit_count++;
        temp /= 10;
    } while (temp != 0);
    if (is_negative) {
        digit_count++;
    }
    utils_StringBuffer[digit_count] = '\0';
    for (i = digit_count - 1; i >= 0; i--) {
        if (is_negative && i == 0) {
            utils_StringBuffer[i] = '-';
        } else {
            utils_StringBuffer[i] = (num % 10) + '0';
            num /= 10;
        }
    }
    return utils_StringBuffer;
}

/**
 * @brief Function for convert hexdecimal integer to ASCII string
 * 
 * @param num Specific hexadecimal integer for convert
 * 
 * @return String version of hexadecimal integer
 */
char* convert_xtoa(uint32_t num) {
    fill(utils_StringBuffer, 0, length(utils_StringBuffer));
    int i = 0;
    if (num == 0) {
        utils_StringBuffer[i++] = '0';
        utils_StringBuffer[i] = '\0';
        return utils_StringBuffer;
    }
    while (num != 0) {
        int remainder = num % 16;
        if (remainder < 10) {
            utils_StringBuffer[i++] = remainder + '0';
        } else {
            utils_StringBuffer[i++] = (remainder - 10) + 'A';
        }
        num /= 16;
    }
    utils_StringBuffer[i] = '\0';
    for (int j = 0; j < i / 2; j++) {
        char temp = utils_StringBuffer[j];
        utils_StringBuffer[j] = utils_StringBuffer[i - j - 1];
        utils_StringBuffer[i - j - 1] = temp;
    }
    return utils_StringBuffer;
}

/**
 * @brief Function for get cursor position
 * 
 * @return Cursor position
 */
int getcursor() { return utils_CursorLocation; }

/**
 * @brief Function for set cursor position
 * 
 * @param ptr Specific cursor position
 */
void putcursor(int ptr) {
    utils_CursorLocation = ptr;
    display_putcursor(utils_CursorLocation);
}

/**
 * @brief Function for print a single character to the CLI output
 * 
 * @param chr Specific character to print
 */
void putchar(char chr) {
    if (chr == '\0') { return; }
    if (chr == '\n') {
        utils_CursorLocation = (utils_CursorLocation / DISPLAY_CLIWIDTH + 1) * DISPLAY_CLIWIDTH;
    } else if (chr == '\t') {
        int tablen = UTILS_TABLENGTH - (getcursor() % UTILS_TABLENGTH);
        for (int i = 0; i < tablen; ++i) {
            display_putchar(' ', utils_CursorLocation);
            utils_CursorLocation++;
        }
    } else {
        display_putchar(chr, utils_CursorLocation);
        utils_CursorLocation++;
    }
    if (utils_CursorLocation >= DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT) {
        utils_scrolldown(1);
        utils_CursorLocation = (DISPLAY_CLIHEIGHT - 1) * DISPLAY_CLIWIDTH;
    }
    display_putcursor(utils_CursorLocation);
}

/**
 * @brief Function for print a string to the CLI output
 * 
 * @param str Specific string to print
 */
void puts(char* str) {
    for (int i = 0; str[i] != '\0'; ++i) { putchar(str[i]); }
}

/**
 * @brief Function for print formatted output to the CLI output
 * 
 * @param format Formatted string to print
 * @param ... Arguments in formatted string
 */
void printf(char* format, ...) {
    va_list args;
    va_start(args, format);
    for (int i = 0; format[i] != '\0'; ++i) {
        if (format[i] == '%') {
            switch (format[i + 1]) {
                case 'd': {
                    int num = va_arg(args, int);
                    puts(convert_itoa(num));
                    break;
                }
                case 'x': {
                    uint32_t hex = va_arg(args, uint32_t);
                    puts(convert_xtoa(hex));
                    break;
                }
                case 'c': {
                    int chr = va_arg(args, int);
                    putchar(chr);
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    puts(str);
                    break;
                }
                default: {
                    if (format[i + 1] == '%') {
                        putchar('%');
                    } else {
                        putchar('%');
                        putchar(format[i + 1]);
                    }
                    break;
                }
            }
            i++;
        } else if (format[i] == '\n') {
            utils_CursorLocation = (utils_CursorLocation / DISPLAY_CLIWIDTH + 1) * DISPLAY_CLIWIDTH;
        } else if (format[i] == '\t') {
            int tablen = UTILS_TABLENGTH - (getcursor() % UTILS_TABLENGTH);
            for (int i = 0; i < tablen; ++i) { putchar(' '); }
        } else {
            display_putchar(format[i], utils_CursorLocation);
            utils_CursorLocation++;
        }
        if (utils_CursorLocation >= DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT) {
            utils_scrolldown(1);
            utils_CursorLocation = (DISPLAY_CLIHEIGHT - 1) * DISPLAY_CLIWIDTH;
        }
    }
    va_end(args);
    display_putcursor(utils_CursorLocation);
}

/**
 * @brief Function for format and write output to a string
 * 
 * @param buffer Specific buffer address to write output
 * @param size Limit for output
 * @param fmt Formatted string
 * @param ... Arguments in formatted string
 * 
 * @return Returns size of output (without limit)
 */
int snprintf(char* buffer, size_t size, char* fmt, ...) {
    va_list args;
    int written = 0;
    va_start(args, fmt);
    for (int i = 0; fmt[i] != '\0' && written < size - 1; ++i) {
        if (fmt[i] == '%') {
            i++;
            if (fmt[i] == '\0') break;
            switch (fmt[i]) {
                case 'd': {
                    int num = va_arg(args, int);
                    char* numStr = convert_itoa(num);
                    for (int j = 0; numStr[j] != '\0' && written < size - 1; j++, written++) {
                        buffer[written] = numStr[j];
                    }
                    break;
                }
                case 'x': {
                    uint32_t hex = va_arg(args, uint32_t);
                    char* hexStr = convert_xtoa(hex);
                    for (int j = 0; hexStr[j] != '\0' && written < size - 1; j++, written++) {
                        buffer[written] = hexStr[j];
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
                    char* str = va_arg(args, char*);
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
    return written;
}

/**
 * @brief Function for read user input and return the entered string
 * 
 * @param header Header for prompt
 * 
 * @return User input
 */
char* prompt(char* header) {
    for (int i = 0; i < UTILS_PROMPTBUFFER_LENGTH; ++i) {
        utils_PromptBuffer[i] = '\0';
    }
    char* passwordHeader = "Password: ";
    bool password = ncompare(header, passwordHeader, length(passwordHeader)) ? false : true;
    utils_PromptPointer = 0;
    printf(header);
    display_putcursor(utils_CursorLocation);
    char* input = (char*)malloc(UTILS_PROMPTBUFFER_LENGTH);
    if (input == NULL) { return utils_PromptBuffer; }
    while (1) {
        size_t written = read(STDIN, input, UTILS_PROMPTBUFFER_LENGTH);
        for (int i = 0; i < written; ++i) {
            if (input[i] != '\0' && input[i] != '\t') {
                if (input[i] == '\b') {
                    if (utils_PromptPointer > 0 && length(utils_PromptBuffer) > 0) {
                        utils_PromptPointer--;
                        utils_PromptBuffer[utils_PromptPointer] = '\0';
                        if (!password) {
                            utils_CursorLocation--;
                            display_putchar('\0', utils_CursorLocation);
                            display_putcursor(utils_CursorLocation);
                        }
                    }
                } else if (input[i] == '\n') {
                    utils_PromptBuffer[UTILS_PROMPTBUFFER_LENGTH - 1] = '\0';
                    display_putcursor(utils_CursorLocation);
                    free(input); return utils_PromptBuffer;
                } else if (utils_PromptPointer < UTILS_PROMPTBUFFER_LENGTH - 1) {
                    utils_PromptBuffer[utils_PromptPointer] = input[i];
                    utils_PromptPointer++;
                    if (!password) {
                        putchar(input[i]);
                        display_putcursor(utils_CursorLocation);
                    }
                }
            }
            display_putcursor(utils_CursorLocation);
        } yield();
    }
    display_putcursor(utils_CursorLocation);
}