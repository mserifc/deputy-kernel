#define WIDTH 80
#define HEIGHT 25

#define USERCOMMANDSIZE 100

#define MAX_TOKENS 100

unsigned short* VideoMemory = (unsigned short*)0xB8000;

int cursor = 0;
int shift = 0;

unsigned char usercommand[USERCOMMANDSIZE];

int backspacecount = 0;

char* promptheader = "# ";

typedef enum {
	COLOR_BLACK = 0x00,
	COLOR_BLUE = 0x01,
	COLOR_GREEN = 0x02,
	COLOR_CYAN = 0x03,
	COLOR_RED = 0x04,
	COLOR_MAGENTA = 0x05,
	COLOR_BROWN = 0x06,
	COLOR_GRAY = 0x07,
	COLOR_DARK_GRAY = 0x08,
	COLOR_LIGHT_BLUE = 0x09,
	COLOR_LIGHT_GREEN = 0x0A,
	COLOR_LIGHT_CYAN = 0x0B,
	COLOR_LIGHT_RED = 0x0C,
	COLOR_LIGHT_MAGENTA = 0x0D,
	COLOR_YELLOW = 0x0E,
	COLOR_WHITE = 0x0F
} ColorTable;

unsigned char textcolor = COLOR_WHITE;
unsigned char bgcolor = COLOR_BLACK;

char* tokens[MAX_TOKENS];
int token_count = 0;

const unsigned char keymap[216] = {
	 0 ,  0 , '1', '2', '3', '4', '5', '6', // 0x00 - 0x07
	'7', '8', '9', '0', '-', '=',  0 ,  0 , // 0x08 - 0x0F
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', // 0x10 - 0x17
	'o', 'p', '[', ']', '\n', 0 , 'a', 's', // 0x18 - 0x1F
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', // 0x20 - 0x27
	'\'','`',  0 , '\\','z', 'x', 'c', 'v', // 0x28 - 0x2F
	'b', 'n', 'm', ',', '.', '/',  0 , '*', // 0x30 - 0x37
	 0 , ' ',  0 ,  0 ,  0 ,  0 ,  0 ,  0 , // 0x38 - 0x3F
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , '7', // 0x40 - 0x47
	'8', '9', '-', '4', '5', '6', '+', '1', // 0x48 - 0x4F
	'2', '3', '0', '.',  0 ,  0 ,  0 ,  0 , // 0x50 - 0x57
};

const unsigned char shiftmap[(sizeof(keymap) / sizeof(keymap[0]))] = {
	 0 ,  0 , '!', '@', '#', '$', '%', '^', // 0x00 - 0x07
	'&', '*', '(', ')', '_', '+',  0 ,  0 , // 0x08 - 0x0F
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', // 0x10 - 0x17
	'O', 'P', '{', '}', '\n', 0 , 'A', 'S', // 0x18 - 0x1F
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', // 0x20 - 0x27
	'"', '~',  0 , '|', 'Z', 'X', 'C', 'V', // 0x28 - 0x2F
	'B', 'N', 'M', '<', '>', '?',  0 , '*', // 0x30 - 0x37
	 0 , ' ',  0 ,  0 ,  0 ,  0 ,  0 ,  0 , // 0x38 - 0x3F
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , '7', // 0x40 - 0x47
	'8', '9', '-', '4', '5', '6', '+', '1', // 0x48 - 0x4F
	'2', '3', '0', '.',  0 ,  0 ,  0 ,  0 , // 0x50 - 0x57
};

void outb(unsigned short port, unsigned char value) {
	asm volatile (
		"outb %0, %1"
		:
		: "a"(value), "Nd"(port)
	);
}

unsigned char inb(unsigned short port) {
	unsigned char value;
	asm volatile (
		"inb %1, %0"
		: "=a"(value)
		: "Nd"(port)
	);
	return value;
}

void printf(char* str) {
	for (int i = 0; str[i] != '\0'; ++i) {
		if (str[i] == '\n') {
			cursor = (cursor / WIDTH + 1) * WIDTH;
			continue;
		}
		if (cursor >= WIDTH * HEIGHT) {
			for (int j = 0; j < (WIDTH * (HEIGHT - 1)); ++j) {
				VideoMemory[j] = VideoMemory[j + WIDTH];
			}
			cursor = (HEIGHT - 1) * WIDTH;
			for (int j = cursor; j < WIDTH * HEIGHT; ++j) {
				VideoMemory[j] = 0;
			}
		}
		VideoMemory[cursor] = (VideoMemory[cursor] & 0xF000) | (((short)textcolor & 0x0F) << 8) | str[i];
		++cursor;
	}
}

void putchar(char character) {
	if (character == '\n') {
		cursor = (cursor / WIDTH + 1) * WIDTH;
		return;
	}
	if (cursor >= WIDTH * HEIGHT) {
		for (int j = 0; j < (WIDTH * (HEIGHT - 1)); ++j) {
			VideoMemory[j] = VideoMemory[j + WIDTH];
		}
		cursor = (HEIGHT - 1) * WIDTH;
		for (int j = cursor; j < WIDTH * HEIGHT; ++j) {
			VideoMemory[j] = 0;
		}
	}
	VideoMemory[cursor] = (VideoMemory[cursor] & 0xF000) | (((short)textcolor & 0x0F) << 8) | character;
	++cursor;
}

void pushchar(unsigned char *str, unsigned char character) {
	int i = 0;
	while (str[i] != '\0') { i++; }
	str[i] = character;
	str[i + 1] = '\0';
}

void clear() {
	for (int i = 0; i < (WIDTH * HEIGHT); ++i)
	{
		VideoMemory[i] = ((short)bgcolor << 12);
	}
	cursor = 0;
}

void set_cursor(int x, int y) {
	if (x < WIDTH && y < HEIGHT) {
		cursor = (y * WIDTH) + x;
	}
}

void delay(int milliseconds) {
	int count = milliseconds * 1000;
	while (count-- > 0) {
		asm volatile("nop");
	}
}

int length(unsigned char *str) {
	int i = 0;
	while (str[i] != '\0') { i++; }
	return i;
}

char** split(unsigned char* str) {
	int i = 0;
	int start = 0;
	token_count = 0;

	while (str[i] != '\0') {
		if (str[i] == ' ') {
			str[i] = '\0';
			if (token_count < MAX_TOKENS) {
				tokens[token_count++] = (char*)&str[start];
			}
			start = i + 1;
		}
		i++;
	}
	if (token_count < MAX_TOKENS) {
		tokens[token_count++] = (char*)&str[start];
	}
	tokens[token_count] = 0;

	return tokens;
}

int strcmp(const char* str1, const char* str2) {
	while (*str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	}
	return *(unsigned char*)str1 - *(unsigned char*)str2;
}

void commandhandler() {
	usercommand[USERCOMMANDSIZE - 1] = '\0';
	putchar('\n');
	// printf(reinterpret_cast<char*>(usercommand));
	char** cmd = split(usercommand);
	if (backspacecount != 0) {
		if (cmd[0] && strcmp(cmd[0], "echo") == 0) {
			for(int i = 1; i < token_count; ++i) {
				printf(cmd[i]);
				putchar(' ');
			}
		} else if (cmd[0] && strcmp(cmd[0], "cd") == 0) {
			printf("File system does not exist.");
		} else if (cmd[0] && strcmp(cmd[0], "clear") == 0) {
			clear();
		} else {
			printf(cmd[0]);
			printf(" not found.");
		}
	}
	for (int i = 0; i < USERCOMMANDSIZE; ++i) {
		usercommand[i] = '\0';
	}
	backspacecount = 0;
}

unsigned char waitKeycode() {
	unsigned char current_input, new_input;
	current_input = inb(0x60);
	while(1) {
		new_input = inb(0x60);
		if (current_input != new_input) {
			return new_input;
		}
		delay(10);
	}
}

unsigned char waitKey() {
	unsigned char input = waitKeycode();
	if (input == 0x2A || input == 0x36) {
		shift = 1;
		return 0;
	}
	if (input == 0xAA || input == 0xB6) {
		shift = 0;
		return 0;
	}
	if (input == 0x0E) {
		if (backspacecount > 0) {
			int i = 0;
			while (usercommand[i] != '\0') { i++; }
			if (i > 0) {
				usercommand[i - 1] = '\0';
				cursor--;
				VideoMemory[cursor] = ((short)bgcolor << 12) | 0x0F00;
				backspacecount--;
			}
		}
	} else if (input == 0x1C) {
		commandhandler();
		putchar('\n');
		printf(promptheader);
	} else if (input < (sizeof(keymap) / sizeof(keymap[0]))) {
		if (!shift) {
			return keymap[input];
		} else if (shift) {
			return shiftmap[input];
		}
	}
	return 0;
}

extern "C" void _kernel_main(void* multiboot_structure, unsigned int magicnumber) {
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
	clear();
	set_cursor(0, 0);
	printf("Serif's Kernel Build 17, booted successfully.\n");
	printf("Welcome!\n\n");
	printf(promptheader);
	while(1) {
		unsigned char pressedkey = waitKey();
		if (pressedkey != 0 && length(usercommand) < (USERCOMMANDSIZE - 1)) {
			pushchar(usercommand, pressedkey);
			putchar(pressedkey);
			backspacecount++;
		}
	}
	while(1);
}