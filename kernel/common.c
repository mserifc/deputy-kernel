#include "common.h"

// Cursor environment variables
int CursorLocation = 0;             // Cursor position
bool CursorVisibility = true;       // Cursor visibility

// Prompt environment variables
char ScannedPrompt[PROMPT_LENGTH];  // Scanned prompt
int PromptPointer = 0;              // Prompt pointer

// String manipulation environment variables
char* StrTokens[MAX_STR_TOKENS];    // String token vector
int StrTokenCount = 0;              // String token count
char StrResult[10];                 // String result

// Integer manipulation environment variables
int IntDigits[MAX_INT_DIGITS];      // Integer digit vector
int IntDigitCount = 0;              // Integer digit count

// Date result
date_t Date;

// Function for get current instruction address
size_t getInstruction() { return (size_t)__builtin_return_address(0); }

// Function for update the cursor
int updateCursor() {
    display_putcursor(CursorLocation);
    return CursorLocation;
}

// Function for set cursor position
void putcursor(int ptr) {
    CursorLocation = ptr;
    updateCursor();
}

// Function for get string tokens
int getStrTokenCount() { return StrTokenCount; }

// Function for get integer digits
int getIntDigitCount() { return IntDigitCount; }

// Function for convert binary-coded decimal to decimal
uint8_t bcd2dec(uint8_t bcd) { return (bcd & 0x0F) + ((bcd >> 4) * 10); }

// Function for read RTC (Real Time Clock)
void readRTC (
    uint8_t* sec,
    uint8_t* min,
    uint8_t* hour,
    uint8_t* day,
    uint8_t* mon,
    uint16_t* year
) {
    port_outb(RTC_INDEX_PORT, 0x00);
    *sec = port_inb(RTC_DATA_PORT);

    port_outb(RTC_INDEX_PORT, 0x02);
    *min = port_inb(RTC_DATA_PORT);

    port_outb(RTC_INDEX_PORT, 0x04);
    *hour = port_inb(RTC_DATA_PORT);

    port_outb(RTC_INDEX_PORT, 0x07);
    *day = port_inb(RTC_DATA_PORT);

    port_outb(RTC_INDEX_PORT, 0x08);
    *mon = port_inb(RTC_DATA_PORT);

    port_outb(RTC_INDEX_PORT, 0x09);
    *year = port_inb(RTC_DATA_PORT);

    *sec = bcd2dec(*sec);
    *min = bcd2dec(*min);
    *hour = bcd2dec(*hour);
    *day = bcd2dec(*day);
    *mon = bcd2dec(*mon);
    *year = bcd2dec(*year) + 2000;
}

// Function for get RTC date
date_t date() {
    readRTC(
        &Date.sec,
        &Date.min,
        &Date.hour,
        &Date.day,
        &Date.mon,
        &Date.year
    );
    return Date;
}

// Function for introduce a delay (based on CPU speed)
void delay(uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        asm volatile ("nop");
    }
}

// Sleep the system for a certain amount of time (based on RTC)
void sleep(uint32_t sec) {
    uint32_t targetTime =
        (date().day * 24 * 60 * 60) +
        (date().hour * 60 * 60) +
        (date().min * 60) +
        date().sec + sec;
    asm volatile ("cli");
    interrupts_PICIRQEnable(INTERRUPTS_IRQ_TIMER);
    while (true) {
        asm volatile ("sti");
        asm volatile ("hlt");
        if (
            (date().day * 24 * 60 * 60) +
            (date().hour * 60 * 60) +
            (date().min * 60) +
            date().sec >= targetTime
        ) {
            asm volatile ("sti");
            return;
        }
        asm volatile ("sti");
    }
}

// Function for scroll down the CLI Display
int scrolldown(int times) {
    if (times < 0) { return -1; }
    for (int i = 0; i < times; ++i) {
        for (int j = 0; j < (DISPLAY_CLIWIDTH * (DISPLAY_CLIHEIGHT - 1)); ++j) {
            display_putchar(display_getchar(j + DISPLAY_CLIWIDTH), j);
        }
        for (int j = ((DISPLAY_CLIHEIGHT - 1) * DISPLAY_CLIWIDTH); j < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++j) {
            display_putchar('\0', j);
        }
    }
    return 0;
}

// Function for check if a character is a digit (returns true if the character is a number)
bool isdigit(char chr) {
    return chr >= '0' && chr <= '9';
}

// Function for split a string
char** split(char* str, char deli) {
    int i = 0;
    int start = 0;
    StrTokenCount = 0;
    while (str[i] != 0) {
        if (str[i] == deli) {
            str[i] = '\0';
            if (StrTokenCount < MAX_STR_TOKENS) {
                StrTokens[StrTokenCount++] = (char*)&str[start];
            }
            start = i + 1;
        }
        i++;
    }
    if (StrTokenCount < MAX_STR_TOKENS) {
        StrTokens[StrTokenCount++] = (char*)&str[start];
    }
    StrTokens[StrTokenCount] = 0;
    return StrTokens;
}

// Function for split a integer
int* splitdigits(int num) {
    int temp = num;
    int count = 0;
    if (temp == 0) {
        IntDigits[count++] = 0;
    } else {
        while (temp > 0) {
            temp /= 10;
            count++;
        }
    }
    temp = num;
    for (int i = count - 1; i >= 0; --i) {
        IntDigits[i] = temp % 10;
        temp /= 10;
    }
    IntDigitCount = count;
    return IntDigits;
}

// Function for fill a block of memory with specific value
void* memset(void* ptr, char value, size_t size) {
    uint8_t* point = ptr;
    for (size_t i = 0; i < size; i++) {
        point[i] = (uint8_t)value;
    }
    return ptr;
}

// Function for copy a block of memory from source to destination
void* memcpy(void* dest, const void* src, size_t size) {
    uint8_t* destination = dest;
    const uint8_t* source = src;
    for (size_t i = 0; i < size; i++) {
        destination[i] = source[i];
    }
    return dest;
}

// Function for get the length of specific string
int strlen(char* str) {
    int len = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        len++;
    }
    return len;
}

// Function for copy a string to another one
char* strcpy(char* dest, const char* src) {
	char* original_dest = dest;
	while (*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	}
	*dest = '\0';
	return original_dest;
}

// function for copy a string to another one (with limit)
char* strncpy(char* dest, const char* src, size_t num) {
    char* original_dest = dest;
    while (num > 0 && *src != '\0') {
        *dest = *src;
        dest++;
        src++;
        num--;
    }
    while (num > 0) {
        *dest = '\0';
        dest++;
        num--;
    }
    return original_dest;
}

// Function for compare two strings
int strcmp(const char* str1, const char* str2) {
	while (*str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	}
	return *(unsigned char*)str1 - *(unsigned char*)str2;
}

// Function for compare two strings (with limit)
int strncmp(const char* str1, const char* str2, size_t num) {
    while (num > 0) {
        if (*str1 != *str2) { return 1; }
        str1++;
        str2++;
        num--;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

// Function for format and write output to a string
int snprintf(char* buffer, size_t size, char* format, ...) {
    va_list args;
    int written = 0;
    va_start(args, format);
    for (int i = 0; format[i] != '\0' && written < size - 1; ++i) {
        if (format[i] == '%') {
            i++;
            if (format[i] == '\0') break;
            switch (format[i]) {
                case 'd': {
                    int num = va_arg(args, int);
                    char* numStr = itoa(num);
                    for (int j = 0; numStr[j] != '\0' && written < size - 1; j++, written++) {
                        buffer[written] = numStr[j];
                    }
                    break;
                }
                case 'x': {
                    uint32_t hex = va_arg(args, uint32_t);
                    char* hexStr = xtoa(hex);
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
                        buffer[written++] = format[i];
                    }
                    break;
                }
            }
        } else {
            buffer[written++] = format[i];
        }
    }
    buffer[written] = '\0';
    va_end(args);
    return written;
}

// Function for convert ASCII character to integer
int atoi(char* str) {
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
    while (isdigit(str[ptr])) {
        result = result * 10 + (str[ptr] - '0');
        ptr++;
    }
    return sign * result;
}

// Function for convert integer to ASCII character
char* itoa(int num) {
    memset(StrResult, 0, strlen(StrResult));
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
    StrResult[digit_count] = '\0';
    for (i = digit_count - 1; i >= 0; i--) {
        if (is_negative && i == 0) {
            StrResult[i] = '-';
        } else {
            StrResult[i] = (num % 10) + '0';
            num /= 10;
        }
    }
    return StrResult;
}

// Function for convert hexdecimal integer to ASCII character
char* xtoa(uint32_t num) {
    memset(StrResult, 0, strlen(StrResult));
    int i = 0;
    if (num == 0) {
        StrResult[i++] = '0';
        StrResult[i] = '\0';
        return StrResult;
    }
    while (num != 0) {
        int remainder = num % 16;
        if (remainder < 10) {
            StrResult[i++] = remainder + '0';
        } else {
            StrResult[i++] = (remainder - 10) + 'A';
        }
        num /= 16;
    }
    StrResult[i] = '\0';
    for (int j = 0; j < i / 2; j++) {
        char temp = StrResult[j];
        StrResult[j] = StrResult[i - j - 1];
        StrResult[i - j - 1] = temp;
    }
    return StrResult;
}

// Function for print a single character to the CLI output
void putchar(char chr) {
    if (chr == '\n') {
        CursorLocation = (CursorLocation / DISPLAY_CLIWIDTH + 1) * DISPLAY_CLIWIDTH;
    } else if (chr == '\t') {
        for (int i = 0; i < TAB_LENGTH; ++i) {
            display_putchar(' ', CursorLocation);
            CursorLocation++;
        }
    } else {
        display_putchar(chr, CursorLocation);
        CursorLocation++;
    }
    if (CursorLocation >= DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT) {
        scrolldown(1);
        CursorLocation = (DISPLAY_CLIHEIGHT - 1) * DISPLAY_CLIWIDTH;
    }
    updateCursor();
}

// Function for print a string to the CLI output
void puts(char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == '\n') {
            CursorLocation = (CursorLocation / DISPLAY_CLIWIDTH + 1) * DISPLAY_CLIWIDTH;
        } else if (str[i] == '\t') {
            for (int j = 0; j < TAB_LENGTH; ++j) {
                display_putchar(' ', CursorLocation);
                CursorLocation++;
            }
        } else {
            display_putchar(str[i], CursorLocation);
            CursorLocation++;
        }
        if (CursorLocation >= DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT) {
            scrolldown(1);
            CursorLocation = (DISPLAY_CLIHEIGHT - 1) * DISPLAY_CLIWIDTH;
        }
    }
    updateCursor();
}

// Function for print formatted output to the CLI output
void printf(char* format, ...) {
    va_list args;
    va_start(args, format);
    for (int i = 0; format[i] != '\0'; ++i) {
        if (format[i] == '%') {
            switch (format[i + 1]) {
                case 'd': {
                    int num = va_arg(args, int);
                    puts(itoa(num));
                    break;
                }
                case 'x': {
                    uint32_t hex = va_arg(args, uint32_t);
                    puts(xtoa(hex));
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
            CursorLocation = (CursorLocation / DISPLAY_CLIWIDTH + 1) * DISPLAY_CLIWIDTH;
        } else {
            display_putchar(format[i], CursorLocation);
            CursorLocation++;
        }
        if (CursorLocation >= DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT) {
            scrolldown(1);
            CursorLocation = (DISPLAY_CLIHEIGHT - 1) * DISPLAY_CLIWIDTH;
        }
    }
    va_end(args);
    updateCursor();
}

// Function for read user input and return the entered string
char* scanf(char* header) {
    for (int i = 0; i < PROMPT_LENGTH; ++i) {
        ScannedPrompt[i] = '\0';
    }
    PromptPointer = 0;
    printf(header);
    updateCursor();
    while (1) {
        char input = keyboard_scankey();
        if (
            input != '\0' &&
            input != KEYBOARD_KEY_ESCAPE &&
            input != '\t' &&
            input != KEYBOARD_KEY_CAPSLOCK &&
            input != KEYBOARD_KEY_LEFT_SHIFT &&
            input != KEYBOARD_KEY_LEFT_CTRL &&
            input != KEYBOARD_KEY_LEFT_ALT
        ) {
            if (input == '\b') {
                if (PromptPointer > 0 && strlen(ScannedPrompt) > 0) {
                    PromptPointer--;
                    ScannedPrompt[PromptPointer] = '\0';
                    CursorLocation--;
                    display_putchar('\0', CursorLocation);
                    updateCursor();
                }
            } else if (input == '\n') {
                ScannedPrompt[PROMPT_LENGTH - 1] = '\0';
                updateCursor();
                return ScannedPrompt;
            } else if (PromptPointer < PROMPT_LENGTH - 1) {
                ScannedPrompt[PromptPointer] = input;
                PromptPointer++;
                putchar(input);
                updateCursor();
            }
        }
        updateCursor();
    }
    updateCursor();
}