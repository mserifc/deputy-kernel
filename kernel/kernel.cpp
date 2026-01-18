#define WIDTH 80
#define HEIGHT 25

#define MAX_USER_COMMAND_SIZE 100

#define MAX_TOKENS 100

#define MAX_FILE_COUNT 64
#define MAX_FILENAME_LENGTH 32
#define MAX_FILE_SIZE 4064

unsigned short* VideoMemory = (unsigned short*)0xB8000;

int cursor = 0;
int shift = 0;

unsigned char usercommand[MAX_USER_COMMAND_SIZE];

int backspacecount = 0;

char* promptheader = "# ";

enum ColorTable {
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
};

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

struct File {
	char name[MAX_FILENAME_LENGTH];
	char data[MAX_FILE_SIZE];
};

struct Directory {
	struct File files[MAX_FILE_COUNT];
	int filecount = 0;
};

struct Directory root[MAX_FILE_COUNT];

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

void loadBackcolor() {
	for (int i = 0; i < (WIDTH * HEIGHT); ++i) {
		VideoMemory[i] = (((short)bgcolor & 0x0F) << 12) | (((short)textcolor & 0x0F) << 8) | (VideoMemory[i] & 0x00FF);
	}
}

void loadCursor() {
	VideoMemory[cursor] = (((short)textcolor & 0x0F) << 12) | (((short)bgcolor & 0x0F) << 8) | (VideoMemory[cursor] & 0x00FF);
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

int ulength(unsigned char *str) {
	int i = 0;
	while (str[i] != '\0') { i++; }
	return i;
}

int length(char* str) {
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

void intToString(int num, char* str) {
	int temp = num;
	int len = 0;
	while (temp > 0) {
		len++;
		temp /= 10;
	}
	str[len] = '\0';
	while (num > 0) {
		str[--len] = (num % 10) + '0';
		num /= 10;
	}
}

void splitNumber(int num, int* digits, int *len) {
	int temp = num;
	int digit = 0;
	if (num == 0) {
		digits[0] = 0;
		*len = 1;
		return;
	}
	while (temp != 0) {
		temp /= 10;
		digit++;
	}
	temp = num;
	for (int i = digit - 1; i >= 0; i--) {
		digits[i] = temp % 10;
		temp /= 10;
	}
	*len = digit;
}

char toChar(int num) {
	switch (num) {
		case 0:
			return '0';
		case 1:
			return '1';
		case 2:
			return '2';
		case 3:
			return '3';
		case 4:
			return '4';
		case 5:
			return '5';
		case 6:
			return '6';
		case 7:
			return '7';
		case 8:
			return '8';
		case 9:
			return '9';
		default:
			return -1;
	}
}

int toNumber(char character) {
	switch (character) {
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		default:
			return -1;
	}
}

int isNumber(char* str) {
	for (int i = 0; i < length(str); ++i) {
		if (toNumber(str[i]) == -1) {
			return -1;
		}
	}
	return 0;
}

int initDirectory(struct Directory* dir) {
	for (int i = 0; i < MAX_FILE_COUNT; ++i) {
		for (int j = 0; j < MAX_FILENAME_LENGTH; ++j) {
			dir->files[i].name[j] = '\0';
		}
		for (int j = 0; j < MAX_FILE_SIZE; ++j) {
			dir->files[i].data[j] = '\0';
		}
	}
	dir->filecount = 0;
	return 0;
}

int createFile(struct Directory* dir, char* filename, char* content) {
	if (dir->filecount >= (MAX_FILE_COUNT - 1)) {
		return -1;
	}
	strcpy(dir->files[dir->filecount].name, filename);
	dir->files[dir->filecount].name[MAX_FILENAME_LENGTH - 1] = '\0';
	strcpy(dir->files[dir->filecount].data, content);
	dir->files[dir->filecount].data[MAX_FILE_SIZE - 1] = '\0';
	dir->filecount++;
	return 0;
}

File* findFile(struct Directory* dir, char* filename) {
	for (int i = 0; i < dir->filecount; i++) {
		if (strcmp(dir->files[i].name, filename) == 0) {
			return &dir->files[i];
		}
	}
	return 0;
}

int findIndex(struct Directory* dir, char* filename) {
	for (int i = 0; i < dir->filecount; i++) {
		if (strcmp(dir->files[i].name, filename) == 0) {
			return i;
		}
	}
	return -1;
}

void writeFile(struct Directory* dir, char* filename, char* content) {
	if (findFile(root, filename) == 0) {
		createFile(root, filename, content);
	} else {
		strcpy(findFile(root, filename)->data, content);
	}
}

int removeFile(struct Directory* dir, char* filename) {
	int targetindex = findIndex(dir, filename);
	if (targetindex != -1) {
		for (int i = (targetindex + 1); i < dir->filecount; ++i) {
			for (int j = 0; j < MAX_FILENAME_LENGTH; ++j) {
				dir->files[i - 1].name[j] = dir->files[i].name[j];
			}
			for (int j = 0; j < MAX_FILE_SIZE; ++j) {
				dir->files[i - 1].data[j] = dir->files[i].data[j];
			}
		}
		for (int i = 0; i < MAX_FILENAME_LENGTH; ++i) {
			dir->files[dir->filecount + 1].name[i] = '\0';
		}
		for (int i = 0; i < MAX_FILE_SIZE; ++i) {
			dir->files[dir->filecount + 1].data[i] = '\0';
		}
		dir->filecount--;
		return 0;
	} else { return -1; }
}

void _errorCommandnotfound(char* commandname) {
	printf("Command ");
	printf(commandname);
	printf(" not found.");
}

void _errorNotentered(char* thing) {
	printf(thing);
	printf(" not entered.");
}

void _errorUnabletonothing(char* action) {
	printf("Unable to ");
	printf(action);
	printf(" nothing");
}

void _errorFilenotfound(char* filename) {
	printf("File ");
	printf(filename);
	printf(" not found.");
}

void _errorFilelimit(char* filename) {
	printf("Unable to create file ");
	printf(filename);
	printf(": File limit reached.");
}

void commandhandler() {
	usercommand[MAX_USER_COMMAND_SIZE - 1] = '\0';
	putchar('\n');
	// printf(reinterpret_cast<char*>(usercommand));
	char** cmd = split(usercommand);
	if (backspacecount != 0) {
		if (cmd[0] && strcmp(cmd[0], "help") == 0) {
			printf("Serif's kernel build 18.\n\n");
			printf("Available commands:\n\n");
			printf("echo [STRING] - Print text to the standard output.\n");
			printf("ls - List directory contents.\n");
			printf("read [FILE_NAME] - Print file content.\n");
			printf("create [FILE/FILES] - Create one or more files.\n");
			printf("write [FILE] [CONTENT] - Write a file or change content.\n");
			printf("rm [FILE] - Removes a file.\n");
			printf("clear - Clear screen.\n");
		} else if (
			cmd[0] && strcmp(cmd[0], "echo") == 0 ||
			cmd[0] && strcmp(cmd[0], "print") == 0
		) {
			for (int i = 1; i < token_count; ++i) {
				printf(cmd[i]);
				putchar(' ');
			}
		} else if (
			cmd[0] && strcmp(cmd[0], "cd") == 0 ||
			cmd[0] && strcmp(cmd[0], "mkdir") == 0 ||
			cmd[0] && strcmp(cmd[0], "rmdir") == 0
		) {
			printf("File system does not support directories.");
		} else if (
			cmd[0] && strcmp(cmd[0], "ls") == 0 ||
			cmd[0] && strcmp(cmd[0], "list") == 0 ||
			cmd[0] && strcmp(cmd[0], "dir") == 0
		) {
			for (int i = 0; i < root->filecount; i++) {
				printf(root->files[i].name);
				putchar('\n');
			}
		} else if (
			cmd[0] && strcmp(cmd[0], "read") == 0 ||
			cmd[0] && strcmp(cmd[0], "cat") == 0
		) {
			if (token_count > 1) {
				if (length(cmd[1]) > 0) {
					File* target = findFile(root, cmd[1]);
					if (target != 0) {
						printf(target->data);
					} else { _errorFilenotfound(cmd[1]); }
				} else { _errorUnabletonothing("read"); }
			} else { _errorUnabletonothing("read"); }
		} else if (
			cmd[0] && strcmp(cmd[0], "create") == 0 ||
			cmd[0] && strcmp(cmd[0], "touch") == 0
		) {
			int writedfilescounter = 0;
			for (int i = 1; i < token_count; ++i) {
				if (root->filecount < MAX_FILE_COUNT) {
					if (length(cmd[i]) > 0) {
						writeFile(root, cmd[i], "");
						writedfilescounter++;
					} else {
						printf("Cannot create null named files.\n");
					}
				} else { _errorFilelimit(cmd[i]); }
			}
			if (writedfilescounter <= 0) {
				_errorUnabletonothing("create");
			}
		} else if (cmd[0] && strcmp(cmd[0], "write") == 0) {
			if (root->filecount < MAX_FILE_COUNT) {
				if (token_count > 1) {
					if (length(cmd[1]) > 0) {
						if (token_count > 2) {
							if (length(cmd[2]) > 0) {
								char content[MAX_FILE_SIZE];
								int k = 0;
								for (int i = 0; i < MAX_FILE_SIZE; ++i) {
									content[i] = '\0';
								}
								writeFile(root, cmd[1], content);
								for (int i = 2; i < token_count; ++i) {
									for (int j = 0; j < length(cmd[i]); ++j) {
										content[k] = cmd[i][j];
										k++;
									}
									content[k] = ' ';
									k++;
								}
								writeFile(root, cmd[1], content);
							} else { writeFile(root, cmd[1], ""); }
						} else { writeFile(root, cmd[1], ""); }
					} else { _errorUnabletonothing("write"); }
				} else { _errorUnabletonothing("write"); }
			} else { _errorFilelimit(cmd[1]); }
		} else if (
			cmd[0] && strcmp(cmd[0], "rm") == 0 ||
			cmd[0] && strcmp(cmd[0], "del") == 0
		) {
			if (removeFile(root, cmd[1]) != 0) { _errorFilenotfound(cmd[1]); }
		} else if (
			cmd[0] && strcmp(cmd[0], "clear") == 0 ||
			cmd[0] && strcmp(cmd[0], "clean") == 0 ||
			cmd[0] && strcmp(cmd[0], "clr") == 0 ||
			cmd[0] && strcmp(cmd[0], "cln") == 0 ||
			cmd[0] && strcmp(cmd[0], "cls") == 0
		) {
			clear();
		} else {
			if (token_count > 0) {
				if (length(cmd[0]) > 0) {
					_errorCommandnotfound(cmd[0]);
				}
			}
		}
	}
	for (int i = 0; i < MAX_USER_COMMAND_SIZE; ++i) {
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
	initDirectory(root);
	createFile(root, "readme.txt", "Welcome to Serif's kernel (Build 18), thanks for trying!\n\n Whats new from build 18?\n\n- Shell prompt fixes.\n- Added shell prompt pointer.\n- Added process error handler.\n- New simple help page. (see with 'help' command)\n- Added new file system commands.\n- Added single level file system.\n");
	printf("Serif's Kernel Build 18, booted successfully.\n");
	printf("Welcome!\n\n");
	printf("Type 'help' for learn basic commands.\n\n");
	printf(promptheader);
	loadBackcolor();
	loadCursor();
	while(1) {
		unsigned char pressedkey = waitKey();
		if (pressedkey != 0 && ulength(usercommand) < (MAX_USER_COMMAND_SIZE - 1)) {
			pushchar(usercommand, pressedkey);
			putchar(pressedkey);
			backspacecount++;
		}
		loadBackcolor();
		loadCursor();
	}
	while(1);
}