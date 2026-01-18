#include "common.h"

int CursorLocation = 0;
bool CursorVisibility = true;

char ScannedPrompt[PromptLength];
int PromptPointer = 0;

char* StrTokens[MaxStrTokens];
int StrTokenCount = 0;

int IntDigits[MaxIntDigits];
int IntDigitCount = 0;

char toASCIIresult[10];

int setCursorLocation(int point) {
    if (point < 0 || point >= DisplayWidth * DisplayHeight) {
        return -1;
    }
    CursorLocation = point;
    return 0;
}

int setCursorVisibility(bool toggle) {
    if (toggle == true || toggle == false) {
        CursorVisibility = toggle;
        return 0;
    } else { return -1; }
}

int getCursorLocation() { return CursorLocation; }

int getStrTokenCount() { return StrTokenCount; }

int getIntDigitCount() { return IntDigitCount; }

int scrolldown(int times) {
    if (times < 0) { return -1; }
    for (int i = 0; i < times; ++i) {
        for (int j = 0; j < (DisplayWidth * (DisplayHeight - 1)); ++j) {
            if (display_putchar(display_getchar(j + DisplayWidth), j) == -1) { /* Handle error */ }
        }
        for (int j = ((DisplayHeight - 1) * DisplayWidth); j < DisplayWidth * DisplayHeight; ++j) {
            if (display_putchar('\0', j) == -1) { /* Handle error */ }
        }
    }
    return 0;
}

bool isdigit(char chr) {
    return chr >= '0' && chr <= '9';
}

char** split(char* str) {
    int i = 0;
    int start = 0;
    StrTokenCount = 0;
    while (str[i] != 0) {
        if (str[i] == ' ') {
            str[i] = '\0';
            if (StrTokenCount < MaxStrTokens) {
                StrTokens[StrTokenCount++] = (char*)&str[start];
            }
            start = i + 1;
        }
        i++;
    }
    if (StrTokenCount < MaxStrTokens) {
        StrTokens[StrTokenCount++] = (char*)&str[start];
    }
    StrTokens[StrTokenCount] = 0;
    return StrTokens;
}

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

char* strreset(char* str) {
    for (int i = 0; i != '\0'; ++i) {
        str[i] = '\0';
    }
    return str;
}

int strlen(char* str) {
    int len = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        len++;
    }
    return len;
}

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

int strcmp(const char* str1, const char* str2) {
	while (*str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	}
	return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;

    while (*str == ' ') {
        str++;
    }
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    while (isdigit(*str)) {
        result = result * 10 + (*str - '0');
    }
    return sign * result;
}

char* itoa(int num) {
    strreset(toASCIIresult);
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
    toASCIIresult[digit_count] = '\0';
    for (i = digit_count - 1; i >= 0; i--) {
        if (is_negative && i == 0) {
            toASCIIresult[i] = '-';
        } else {
            toASCIIresult[i] = (num % 10) + '0';
            num /= 10;
        }
    }
    return toASCIIresult;
}

char* itos(int num) {
    strreset(toASCIIresult);
    splitdigits(num);
    for (int i = 0; i < IntDigitCount; ++i) {
        char result = 0;
        switch (IntDigits[i]) {
            case 0: { result = '0'; break; }
            case 1: { result = '1'; break; }
            case 2: { result = '2'; break; }
            case 3: { result = '3'; break; }
            case 4: { result = '4'; break; }
            case 5: { result = '5'; break; }
            case 6: { result = '6'; break; }
            case 7: { result = '7'; break; }
            case 8: { result = '8'; break; }
            case 9: { result = '9'; break; }
        }
        toASCIIresult[i] = result;
    }
    return toASCIIresult;
}

char* xtoa(uint32_t num) {
    strreset(toASCIIresult);
    int i = 0;
    if (num == 0) {
        toASCIIresult[i++] = '0';
        toASCIIresult[i] = '\0';
        return toASCIIresult;
    }
    while (num != 0) {
        int remainder = num % 16;
        if (remainder < 10) {
            toASCIIresult[i++] = remainder + '0';
        } else {
            toASCIIresult[i++] = (remainder - 10) + 'A';
        }
        num /= 16;
    }
    toASCIIresult[i] = '\0';
    for (int j = 0; j < i / 2; j++) {
        char temp = toASCIIresult[j];
        toASCIIresult[j] = toASCIIresult[i - j - 1];
        toASCIIresult[i - j - 1] = temp;
    }
    return toASCIIresult;
}

void putchar(char chr) {
    if (chr == '\n') {
        CursorLocation = (CursorLocation / DisplayWidth + 1) * DisplayWidth;
    } else {
        if (display_putchar(chr, CursorLocation) == -1) { /* Handle error */ }
        CursorLocation++;
    }
    if (CursorLocation >= DisplayWidth * DisplayHeight) {
        scrolldown(1);
        CursorLocation = (DisplayHeight - 1) * DisplayWidth;
    }
}

void puts(char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == '\n') {
            CursorLocation = (CursorLocation / DisplayWidth + 1) * DisplayWidth;
        } else {
            if (display_putchar(str[i], CursorLocation) == -1) { /* Handle error */ }
            CursorLocation++;
        }
        if (CursorLocation >= DisplayWidth * DisplayHeight) {
            scrolldown(1);
            CursorLocation = (DisplayHeight - 1) * DisplayWidth;
        }
    }
}

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
                    puts("0x");
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
            CursorLocation = (CursorLocation / DisplayWidth + 1) * DisplayWidth;
        } else {
            if (display_putchar(format[i], CursorLocation) == -1) { /* Handle error */ }
            CursorLocation++;
        }
        if (CursorLocation >= DisplayWidth * DisplayHeight) {
            scrolldown(1);
            CursorLocation = (DisplayHeight - 1) * DisplayWidth;
        }
    }
    va_end(args);
}

char* scanf(char* header) {
    for (int i = 0; i < PromptLength; ++i) {
        ScannedPrompt[i] = '\0';
    }
    PromptPointer = 0;
    printf(header);
    if (CursorVisibility) { display_loadcursor(CursorLocation); }
    while (1) {
        char input = keyboard_scankey();
        if (input == -1) { /* Handle error */ }
        if (input != '\0') {
            if (input == 0x0E) {
                if (PromptPointer > 0 && strlen(ScannedPrompt) > 0) {
                    PromptPointer--;
                    ScannedPrompt[PromptPointer] = '\0';
                    CursorLocation--;
                    if (display_putchar('\0', CursorLocation) == -1) { /* Handle error */ }
                }
            } else if (input == 0x1C) {
                ScannedPrompt[PromptLength - 1] = '\0';
                return ScannedPrompt;
            } else if (PromptPointer < PromptLength - 1) {
                ScannedPrompt[PromptPointer] = input;
                PromptPointer++;
                putchar(input);
            }
        }
        if (CursorVisibility) { display_loadcursor(CursorLocation); }
    }
}