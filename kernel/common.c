#include "common.h"

int CursorLocation = 0;
bool CursorVisibility = true;

char ScannedPrompt[PromptLength];
int PromptPointer = 0;

char* Tokens[MaxStrTokens];
int TokenCount = 0;

int Digits[MaxIntDigits];
int DigitCount = 0;


void delay(uint32_t ms) {
    volatile uint32_t count;
    uint32_t i;
    for (i = 0; i < ms; ++i) {
        for (count = 0; count < 100000; ++count) {
            asm volatile("nop");
        }
    }
}

int getcursorlocation() {
    return CursorLocation;
}

int setcursorlocation(int point) {
    if (point < 0 || point >= DisplayWidth * DisplayHeight) {
        return -1;
    }
    CursorLocation = point;
    return 0;
}

int setcursorvisibility(bool toggle) {
    if (toggle == true || toggle == false) {
        CursorVisibility = toggle;
        return 0;
    } else { return -1; }
}

int gettokencount() {
    return TokenCount;
}

void scrolldown(int times) {
    for (int i = 0; i < times; ++i) {
        for (int j = 0; j < (DisplayWidth * (DisplayHeight - 1)); ++j) {
            if (DisplayPutchar(DisplayGetchar(j + DisplayWidth), j) == -1) { /* Handle error */ }
        }
        for (int j = ((DisplayHeight - 1) * DisplayWidth); j < DisplayWidth * DisplayHeight; ++j) {
            if (DisplayPutchar('\0', j) == -1) { /* Handle error */ }
        }
    }
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

bool isdigit(char chr) {
    return chr >= '0' && chr <= '9';
}

char** split(char* str) {
    int i = 0;
    int start = 0;
    TokenCount = 0;
    while (str[i] != 0) {
        if (str[i] == ' ') {
            str[i] = '\0';
            if (TokenCount < MaxStrTokens) {
                Tokens[TokenCount++] = (char*)&str[start];
            }
            start = i + 1;
        }
        i++;
    }
    if (TokenCount < MaxStrTokens) {
        Tokens[TokenCount++] = (char*)&str[start];
    }
    Tokens[TokenCount] = 0;
    return Tokens;
}

int* splitdigits(int num) {
    int temp = num;
    int count = 0;
    if (temp == 0) {
        Digits[count++] = 0;
    } else {
        while (temp > 0) {
            temp /= 10;
            count++;
        }
    }
    temp = num;
    for (int i = count - 1; i >= 0; --i) {
        Digits[i] = temp % 10;
        temp /= 10;
    }
    DigitCount = count;
    return Digits;
}

void printf(char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == '\n') {
            CursorLocation = (CursorLocation / DisplayWidth + 1) * DisplayWidth;
        } else {
            if (DisplayPutchar(str[i], CursorLocation) == -1) { /* Handle error */ }
            CursorLocation++;
        }
        if (CursorLocation >= DisplayWidth * DisplayHeight) {
            scrolldown(1);
            CursorLocation = (DisplayHeight - 1) * DisplayWidth;
        }
    }
}

void putchar(char chr) {
    if (chr == '\n') {
        CursorLocation = (CursorLocation / DisplayWidth + 1) * DisplayWidth;
    } else {
        if (DisplayPutchar(chr, CursorLocation) == -1) { /* Handle error */ }
        CursorLocation++;
    }
    if (CursorLocation >= DisplayWidth * DisplayHeight) {
        scrolldown(1);
        CursorLocation = (DisplayHeight - 1) * DisplayWidth;
    }
}

char* scanf(char* promptheader) {
    for (int i = 0; i < PromptLength; ++i) {
        ScannedPrompt[i] = '\0';
    }
    PromptPointer = 0;
    printf(promptheader);
    if (CursorVisibility) { DisplayPutcursor(CursorLocation); }
    while (1) {
        char input = KeyboardScankey();
        if (input == -1) { /* Handle error */ }
        if (input != '\0') {
            if (input == 0x0E) {
                if (PromptPointer > 0 && strlen(ScannedPrompt) > 0) {
                    PromptPointer--;
                    ScannedPrompt[PromptPointer] = '\0';
                    CursorLocation--;
                    if (DisplayPutchar('\0', CursorLocation) == -1) { /* Handle error */ }
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
        if (CursorVisibility) { DisplayPutcursor(CursorLocation); }
    }
}