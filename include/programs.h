#pragma once

#include "kernel.h"
#include "utils.h"
#include "drivers/display.h"
#include "drivers/keyboard.h"
#include "filesystem/ramfs.h"

#define PROGRAMS_TEXTEDIT_MAXLINECOUNT  50      // Max line count in text editor
#define PROGRAMS_TEXTEDIT_MAXCHARCOUNT  80      // Max character count per line in text editor
// Max buffer size in text editor
#define PROGRAMS_TEXTEDIT_BUFFSIZE (PROGRAMS_TEXTEDIT_MAXLINECOUNT * PROGRAMS_TEXTEDIT_MAXCHARCOUNT)

char*   programs_getHomePath();
char*   programs_getWorkingPath();

int     programs_textEditor(char* path);                    // Built-in kernel text editor
int     programs_commandHandler(char* path, char* cmd);     // Built-in kernel command handler