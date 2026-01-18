#include "kernel.h"

#include "hw/port.h"

// Random access buffer for subfunction results
char utils_RABuffer[64];

// * Subfunctions

/**
 * @brief Function for get CPU information by CPUID instruction
 * 
 * @param leaf Leaf (function) number (input)
 * @param subleaf subleaf (subfunction) number (input)
 * @param eax EAX register result (output)
 * @param ebx EBX register result (output)
 * @param ecx ECX register result (output)
 * @param edx EDX register result (output)
 */
void utils_cpuid(
    uint32_t leaf, uint32_t subleaf,
    uint32_t* eax, uint32_t* ebx,
    uint32_t* ecx, uint32_t* edx
) {
    asm volatile (
        "cpuid"
        : "=a" (*eax),
          "=b" (*ebx),
          "=c" (*ecx),
          "=d" (*edx)
        : "a" (leaf), "c" (subleaf)
    );
}

/**
 * @brief Function for read TSC (Time Stamp Counter)
 * 
 * @return 64-bit TSC value
 */
uint64_t utils_rdtsc() {
    if (!kernel_CPUInfo.has_tsc) { return 0; }
    uint32_t low, high;
    asm volatile ("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

/**
 * @brief Function for convert binary-coded decimal to decimal number
 */
uint8_t utils_bcd2dec(uint8_t bcd) { return (uint8_t)((bcd & 0x0F) + ((bcd >> 4) * 10)); }

/**
 * @brief Function for convert ASCII octal number into binary
 * 
 * @param str ASCII octal string
 * @param len Length of string
 */
int utils_oct2bin(const char* str, int len)
    { int n = 0; const char* c = str; while (len-- > 0) { n *= 8; n += *c - '0'; c++; } return n; }

/**
 * @brief Function for read RTC (Real-Time Clock)
 * 
 * @param sec Second address
 * @param min Minute address
 * @param hour Hour address
 * @param day Day address
 * @param mon Month address
 * @param year Year address
 */
void utils_readRTC(
    uint8_t* sec, uint8_t* min, uint8_t* hour,
    uint8_t* day, uint8_t* mon, uint16_t* year
) {
    port_outb(UTILS_RTC_INDEXPORT, 0x00); *sec = port_inb(UTILS_RTC_DATAPORT);
    port_outb(UTILS_RTC_INDEXPORT, 0x02); *min = port_inb(UTILS_RTC_DATAPORT);
    port_outb(UTILS_RTC_INDEXPORT, 0x04); *hour = port_inb(UTILS_RTC_DATAPORT);
    port_outb(UTILS_RTC_INDEXPORT, 0x07); *day = port_inb(UTILS_RTC_DATAPORT);
    port_outb(UTILS_RTC_INDEXPORT, 0x08); *mon = port_inb(UTILS_RTC_DATAPORT);
    port_outb(UTILS_RTC_INDEXPORT, 0x09); *year = port_inb(UTILS_RTC_DATAPORT);
    *sec = utils_bcd2dec(*sec); *min = utils_bcd2dec(*min); *hour = utils_bcd2dec(*hour);
    *day = utils_bcd2dec(*day); *mon = utils_bcd2dec(*mon); *year = (utils_bcd2dec((uint8_t)(*year))) + 2000;
}

/**
 * @brief Function for convert integer to ASCII string
 * 
 * @param num Specific integer for convert
 * 
 * @return String version of integer
 */
char* utils_itoa(int num) {
    fill(utils_RABuffer, 0, sizeof(utils_RABuffer));
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
    utils_RABuffer[digit_count] = '\0';
    for (i = digit_count - 1; i >= 0; i--) {
        if (is_negative && i == 0) {
            utils_RABuffer[i] = '-';
        } else {
            utils_RABuffer[i] = (char)(num % 10) + '0';
            num /= 10;
        }
    }
    return utils_RABuffer;
}

/**
 * @brief Function for convert hexdecimal integer to ASCII string
 * 
 * @param num Specific hexadecimal integer for convert
 * 
 * @return String version of hexadecimal integer
 */
char* utils_xtoa(uint32_t num) {
    fill(utils_RABuffer, 0, sizeof(utils_RABuffer));
    int i = 0;
    if (num == 0) {
        utils_RABuffer[i++] = '0';
        utils_RABuffer[i] = '\0';
        return utils_RABuffer;
    }
    while (num != 0) {
        int remainder = num % 16;
        if (remainder < 10) {
            utils_RABuffer[i++] = (char)remainder + '0';
        } else {
            utils_RABuffer[i++] = (char)(remainder - 10) + 'A';
        }
        num /= 16;
    }
    utils_RABuffer[i] = '\0';
    for (int j = 0; j < i / 2; j++) {
        char temp = utils_RABuffer[j];
        utils_RABuffer[j] = utils_RABuffer[i - j - 1];
        utils_RABuffer[i - j - 1] = temp;
    }
    return utils_RABuffer;
}

// * Functions

/**
 * @brief Function for convert a seed to a pseudo-random 32-bit integer using XORShift algorithm
 * 
 * @param seed Seed value
 * 
 * @return Pseudo-random value
 */
uint32_t xorshift32(uint32_t seed) {
    seed ^= seed << 21;
    seed ^= seed >> 31;
    seed ^= seed << 4;
    return (uint32_t)(seed * 2685821657736338717LL);
}

/**
 * @brief Function for encrypt or decrypt data with a key using XOR Cipher algorithm
 * 
 * @param input Input data to encrypt/decrypt
 * @param key Key value
 */
void xorcipher(char* input, const char* key) {
    size_t input_len = (size_t)length(input);
    size_t key_len = (size_t)length(key);
    for (size_t i = 0; i < input_len; ++i) {
        input[i] ^= key[i % key_len];
    }
}

/**
 * @brief Function for hash the given data using FNV-1a hash algorithm
 * 
 * @param data Address of data buffer to hash
 * @param len Length of data buffer
 * 
 * @return 32-bit hashed result
 */
uint32_t fnv1ahash(const uint8_t* data, size_t len) {
    uint32_t hash = 0x811C9DC5;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i]; hash *= 0x01000193;
    } return hash;
}

/**
 * @brief Function for get current RTC (Real-Time Clock) date
 * 
 * @param base Date structure base to write current date
 */
void date(date_t* base) {
    utils_readRTC(
        &base->sec, &base->min, &base->hour,
        &base->day, &base->mon, &base->year
    );
}

/**
 * @brief Function for introduce a delay (based on CPU speed)
 * 
 * @param ms Milliseconds
 */
void delay(uint32_t ms) {
    if (!kernel_CPUInfo.has_tsc) { return; }
    uint64_t time = (kernel_CPUInfo.frequency * ms) / 1024;
    uint64_t end = time + utils_rdtsc();
    while (utils_rdtsc() < end) { asm volatile ("pause"); if (multitask_InStream) { yield(); } }
}

/**
 * @brief Function for sleep the system for a certain amount of time (based on RTC)
 * 
 * @param sec Seconds
 */
void sleep(uint32_t sec) {
    date_t current, start; date(&start);
    uint32_t targetTime =
        (uint32_t)(start.day * 24 * 60 * 60) +
        (uint32_t)(start.hour * 60 * 60) +
        (uint32_t)(start.min * 60) +
        (uint32_t)start.sec + sec;
    while (true) {
        date(&current); if ( (uint32_t)
            ((current.day * 24 * 60 * 60) +
            (current.hour * 60 * 60) +
            (current.min * 60) +
            current.sec) >= targetTime
        ) { return; } asm volatile ("pause");
        if (multitask_InStream) { yield(); }
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
 * @brief Function for copy a string from source to destination
 * 
 * @param dest Target destination
 * @param src Source to copy
 * 
 * @return Target destination
 */
char* copy(char* dest, const char* src) {
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
void* ncopy(void* dest, const void* src, size_t len) {
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
int compare(const char* str1, const char* str2) {
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
int ncompare(const void* ptr1, const void* ptr2, size_t len) {
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
int length(const char* str) {
    int len = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        len++;
    }
    return len;
}

/**
 * @brief Function for split a string by a deliminer
 * 
 * @param tok Address of tokens to write
 * @param str Specific string
 * @param deli Deliminer
 */
int split(tokens_t* tok, const char* str, char deli) {
    for (int i = 0; i < UTILS_SPLIT_TOKCOUNT; ++i) {
        for (int j = 0; j < UTILS_SPLIT_TOKLEN; ++j) {
            (*tok)[i][j] = '\0';
        }
    } int count = 0; int c = 0;
    bool in_token = false;
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == deli) {
            if (in_token) {
                (*tok)[count][c] = '\0';
                count++;
                c = 0;
                in_token = false;
                if (count >= UTILS_SPLIT_TOKCOUNT) break;
            }
        } else {
            if (c < UTILS_SPLIT_TOKLEN - 1) {
                (*tok)[count][c++] = str[i];
                in_token = true;
            }
        }
    }
    if (in_token && count < UTILS_SPLIT_TOKCOUNT) {
        (*tok)[count][c] = '\0';
        count++;
    }
    return count;
}

/**
 * @brief Function for format and write output to a string
 * 
 * @param buffer Specific buffer address to write output
 * @param size Limit for output
 * @param fmt Formatted string
 * @param ... Arguments in formatted string
 * 
 * @return Returns size of writed output
 */
int snprintf(char* buffer, size_t size, const char* fmt, ...) {
    va_list args;
    int written = 0;
    va_start(args, fmt);
    for (int i = 0; fmt[i] != '\0' && (size_t)written < size - 1; ++i) {
        if (fmt[i] == '%') {
            i++;
            if (fmt[i] == '\0') break;
            switch (fmt[i]) {
                case 'd': {
                    int num = va_arg(args, int);
                    char* numStr = utils_itoa(num);
                    for (int j = 0; numStr[j] != '\0' && (size_t)written < size - 1; j++, written++) {
                        buffer[written] = numStr[j];
                    }
                    break;
                }
                case 'x': {
                    uint32_t hex = va_arg(args, uint32_t);
                    char* hexStr = utils_xtoa(hex);
                    for (int j = 0; hexStr[j] != '\0' && (size_t)written < size - 1; j++, written++) {
                        buffer[written] = hexStr[j];
                    }
                    break;
                }
                case 'c': {
                    int chr = va_arg(args, int);
                    if ((size_t)written < size - 1) {
                        buffer[written++] = (char)chr;
                    }
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    for (int j = 0; str[j] != '\0' && (size_t)written < size - 1; j++, written++) {
                        buffer[written] = str[j];
                    }
                    break;
                }
                default: {
                    buffer[written++] = '%';
                    if ((size_t)written < size - 1) {
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
 * @brief Function for convert ASCII string to integer
 * 
 * @param str Specific number string for convert
 * 
 * @return Integer version of string
 */
int atoi(const char* str) {
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
 * @brief Function for unitize byte size into human-readable format
 * 
 * @param size Size in bytes
 * 
 * @return Human-readable format
 */
char* unit(size_t size) {
    static const char* units[] = { "B", "KB", "MB", "GB", "TB", "PB" };
    fill(utils_RABuffer, 0, sizeof(utils_RABuffer));
    int index = 0; size_t fmtd = size; while (fmtd >= 1024 && index < 5) { fmtd /= 1024; index++; }
    snprintf(utils_RABuffer, sizeof(utils_RABuffer), "%d%s", fmtd, units[index]);
    return utils_RABuffer;
}

/**
 * @brief Function for print a single character to the standard output
 * 
 * @param chr Specific character to print
 */
void putchar(const char chr) {
    if (chr == '\0') { return; }
    else if (chr == '\n') {
        // port_outb(0x3F8, (uint8_t)'\r');
        // port_outb(0x3F8, (uint8_t)'\n');
        console_print("\n", 1);
    } else {
        // port_outb(0x3F8, (uint8_t)chr);
        console_print(&chr, 1);
    }
}

/**
 * @brief Function for print a string to the standard output
 * 
 * @param str Specific string to print
 */
void puts(const char* str) {
    for (int i = 0; str[i] != '\0'; ++i) { putchar(str[i]); }
}

/**
 * @brief Function for print a string to the standard output with limit
 * 
 * @param str Specific string to print
 * @param len Length of string
 */
void nputs(const char* str, int len) {
    for (int i = 0; i < len; ++i) { putchar(str[i]); }
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
    for (int i = 0; fmt[i] != '\0'; ++i) {
        if (fmt[i] == '%') {
            switch (fmt[i + 1]) {
                case 'd': {
                    int num = va_arg(args, int);
                    puts(utils_itoa(num));
                    break;
                }
                case 'x': {
                    uint32_t hex = va_arg(args, uint32_t);
                    puts(utils_xtoa(hex));
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