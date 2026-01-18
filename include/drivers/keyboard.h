#pragma once

#include "types.h"
#include "hardware/port.h"
#include "kernel.h"

// * Types

typedef unsigned short key_t;       // Key type for represent key values

// * PS/2 keyboard keycode list

#define KEYBOARD_DATAPORT               0x60    // PS/2 keyboard data port
#define KEYBOARD_STATUSPORT             0x64    // PS/2 keyboard status port

#define KEYBOARD_KEY_ESCAPE             0x01    // Escape key in keyboard
#define KEYBOARD_KEY_1                  0x02    // Key 1 in keyboard
#define KEYBOARD_KEY_2                  0x03    // Key 2 in keyboard
#define KEYBOARD_KEY_3                  0x04    // Key 3 in keyboard
#define KEYBOARD_KEY_4                  0x05    // Key 4 in keyboard
#define KEYBOARD_KEY_5                  0x06    // Key 5 in keyboard
#define KEYBOARD_KEY_6                  0x07    // Key 6 in keyboard
#define KEYBOARD_KEY_7                  0x08    // Key 7 in keyboard
#define KEYBOARD_KEY_8                  0x09    // Key 8 in keyboard
#define KEYBOARD_KEY_9                  0x0A    // Key 9 in keyboard
#define KEYBOARD_KEY_0                  0x0B    // Key 0 in keyboard
#define KEYBOARD_KEY_MINUS              0x0C    // Minus key in keyboard
#define KEYBOARD_KEY_EQUAL              0x0D    // Equal key in keyboard
#define KEYBOARD_KEY_BACKSPACE          0x0E    // Backspace key in keyboard
#define KEYBOARD_KEY_TABULATION         0x0F    // Tab key in keyboard
#define KEYBOARD_KEY_Q                  0x10    // Key Q in keyboard
#define KEYBOARD_KEY_W                  0x11    // Key W in keyboard
#define KEYBOARD_KEY_E                  0x12    // Key E in keyboard
#define KEYBOARD_KEY_R                  0x13    // Key R in keyboard
#define KEYBOARD_KEY_T                  0x14    // Key T in keyboard
#define KEYBOARD_KEY_Y                  0x15    // Key Y in keyboard
#define KEYBOARD_KEY_U                  0x16    // Key U in keyboard
#define KEYBOARD_KEY_I                  0x17    // Key I in keyboard
#define KEYBOARD_KEY_O                  0x18    // Key O in keyboard
#define KEYBOARD_KEY_P                  0x19    // Key P in keyboard
#define KEYBOARD_KEY_LBRACKET           0x1A    // Left bracket key in keyboard
#define KEYBOARD_KEY_RBRACKET           0x1B    // Right bracket key in keyboard
#define KEYBOARD_KEY_ENTER              0x1C    // Enter key in keyboard
#define KEYBOARD_KEY_LCONTROL           0x1D    // Left control key in keyboard
#define KEYBOARD_KEY_A                  0x1E    // Key A in keyboard
#define KEYBOARD_KEY_S                  0x1F    // Key S in keyboard
#define KEYBOARD_KEY_D                  0x20    // Key D in keyboard
#define KEYBOARD_KEY_F                  0x21    // Key F in keyboard
#define KEYBOARD_KEY_G                  0x22    // Key G in keyboard
#define KEYBOARD_KEY_H                  0x23    // Key H in keyboard
#define KEYBOARD_KEY_J                  0x24    // Key J in keyboard
#define KEYBOARD_KEY_K                  0x25    // Key K in keyboard
#define KEYBOARD_KEY_L                  0x26    // Key L in keyboard
#define KEYBOARD_KEY_SEMICOLON          0x27    // Semicolon key in keyboard
#define KEYBOARD_KEY_QUOTE              0x28    // Quote key in keyboard
#define KEYBOARD_KEY_BACKTICK           0x29    // Backtick key in keyboard
#define KEYBOARD_KEY_LSHIFT             0x2A    // Left shift key in keyboard
#define KEYBOARD_KEY_BACKSLASH          0x2B    // Backslash key in keyboard
#define KEYBOARD_KEY_Z                  0x2C    // Key Z in keyboard
#define KEYBOARD_KEY_X                  0x2D    // Key X in keyboard
#define KEYBOARD_KEY_C                  0x2E    // Key C in keyboard
#define KEYBOARD_KEY_V                  0x2F    // Key V in keyboard
#define KEYBOARD_KEY_B                  0x30    // Key B in keyboard
#define KEYBOARD_KEY_N                  0x31    // Key N in keyboard
#define KEYBOARD_KEY_M                  0x32    // Key M in keyboard
#define KEYBOARD_KEY_COMMA              0x33    // Comma key in keyboard
#define KEYBOARD_KEY_DOT                0x34    // Dot key in keyboard
#define KEYBOARD_KEY_SLASH              0x35    // Slash key in keyboard
#define KEYBOARD_KEY_RSHIFT             0x36    // Right shift key in keyboard
#define KEYBOARD_KEY_NP_ASTERISK        0x37    // Numpad asterisk key in keyboard
#define KEYBOARD_KEY_ALT                0x38    // Alt key in keyboard
#define KEYBOARD_KEY_SPACE              0x39    // Spacebar in keyboard
#define KEYBOARD_KEY_CAPSLOCK           0x3A    // CapsLock key in keyboard
#define KEYBOARD_KEY_F1                 0x3B    // F1 key in keyboard
#define KEYBOARD_KEY_F2                 0x3C    // F2 key in keyboard
#define KEYBOARD_KEY_F3                 0x3D    // F3 key in keyboard
#define KEYBOARD_KEY_F4                 0x3E    // F4 key in keyboard
#define KEYBOARD_KEY_F5                 0x3F    // F5 key in keyboard
#define KEYBOARD_KEY_F6                 0x40    // F6 key in keyboard
#define KEYBOARD_KEY_F7                 0x41    // F7 key in keyboard
#define KEYBOARD_KEY_F8                 0x42    // F8 key in keyboard
#define KEYBOARD_KEY_F9                 0x43    // F9 key in keyboard
#define KEYBOARD_KEY_F10                0x44    // F10 key in keyboard
#define KEYBOARD_KEY_NUMBERLOCK         0x45    // NumberLock key in keyboard
#define KEYBOARD_KEY_SCROLLLOCK         0x46    // ScrollLock key in keyboard
#define KEYBOARD_KEY_NP_7               0x47    // Numpad 7 key in keyboard
#define KEYBOARD_KEY_NP_8               0x48    // Numpad 8 key in keyboard
#define KEYBOARD_KEY_NP_9               0x49    // Numpad 9 key in keyboard
#define KEYBOARD_KEY_NP_MINUS           0x4A    // Numpad minus key in keyboard
#define KEYBOARD_KEY_NP_4               0x4B    // Numpad 4 key in keyboard
#define KEYBOARD_KEY_NP_5               0x4C    // Numpad 5 key in keyboard
#define KEYBOARD_KEY_NP_6               0x4D    // Numpad 6 key in keyboard
#define KEYBOARD_KEY_NP_PLUS            0x4E    // Numpad plus key in keyboard
#define KEYBOARD_KEY_NP_1               0x4F    // Numpad 1 key in keyboard
#define KEYBOARD_KEY_NP_2               0x50    // Numpad 2 key in keyboard
#define KEYBOARD_KEY_NP_3               0x51    // Numpad 3 key in keyboard
#define KEYBOARD_KEY_NP_0               0x52    // Numpad 0 key in keyboard
#define KEYBOARD_KEY_NP_DOT             0x53    // Numpad dot key in keyboard
#define KEYBOARD_KEY_F11                0x57    // F11 key in keyboard
#define KEYBOARD_KEY_F12                0x58    // F12 key in keyboard

#define KEYBOARD_KEY_RELEASE            0x80    // Key release

#define KEYBOARD_KEY_SPECIAL            0xE0    // Additional keys

#define KEYBOARD_KEY_SP_MM_PREVIOUS     0xE010  // Multimedia previous key
#define KEYBOARD_KEY_SP_MM_NEXT         0xE019  // Multimedia next key
#define KEYBOARD_KEY_SP_NP_ENTER        0xE01C  // Numpad enter key
#define KEYBOARD_KEY_SP_RCONTROL        0xE01D  // Right control key
#define KEYBOARD_KEY_SP_MM_MUTE         0xE020  // Multimedia mute key
#define KEYBOARD_KEY_SP_MM_CALCULATOR   0xE021  // Multimedia calculator key
#define KEYBOARD_KEY_SP_MM_PLAY         0xE022  // Multimedia play key
#define KEYBOARD_KEY_SP_MM_STOP         0xE024  // Multimedia stop key
#define KEYBOARD_KEY_SP_MM_VOLUMEDOWN   0xE02E  // Multimedia volume down key
#define KEYBOARD_KEY_SP_MM_VOLUMEUP     0xE030  // Multimedia volume up key
#define KEYBOARD_KEY_SP_MM_WWWHOME      0xE032  // Multimedia WWW home key
#define KEYBOARD_KEY_SP_NP_SLASH        0xE035  // Numpad slash key
#define KEYBOARD_KEY_SP_ALTGR           0xE038  // Altgr key
#define KEYBOARD_KEY_SP_HOME            0xE047  // Home key
#define KEYBOARD_KEY_SP_CURSORUP        0xE048  // Cursor up key
#define KEYBOARD_KEY_SP_PAGEUP          0xE049  // Page up key
#define KEYBOARD_KEY_SP_CURSORLEFT      0xE04B  // Cursor left key
#define KEYBOARD_KEY_SP_CURSORRIGHT     0xE04D  // Cursor right key
#define KEYBOARD_KEY_SP_END             0xE04F  // End key
#define KEYBOARD_KEY_SP_CURSORDOWN      0xE050  // Cursor down key
#define KEYBOARD_KEY_SP_PAGEDOWN        0xE051  // Page down key
#define KEYBOARD_KEY_SP_INSERT          0xE052  // Insert key
#define KEYBOARD_KEY_SP_DELETE          0xE053  // Delete key
#define KEYBOARD_KEY_SP_LEFTGUI         0xE05B  // Left GUI key
#define KEYBOARD_KEY_SP_RIGHTGUI        0xE05C  // Right GUI key
#define KEYBOARD_KEY_SP_APPS            0xE05D  // Apps key
#define KEYBOARD_KEY_SP_ACPI_POWER      0xE05E  // ACPI power key
#define KEYBOARD_KEY_SP_ACPI_SLEEP      0xE05F  // ACPI sleep key
#define KEYBOARD_KEY_SP_ACPI_WAKE       0xE063  // ACPI wake key
#define KEYBOARD_KEY_SP_MM_WWWSEARCH    0xE065  // Multimedia WWW search key
#define KEYBOARD_KEY_SP_MM_WWWFAVORITES 0xE066  // Multimedia WWW favorites
#define KEYBOARD_KEY_SP_MM_WWWREFRESH   0xE067  // Multimedia WWW refresh
#define KEYBOARD_KEY_SP_MM_WWWSTOP      0xE068  // Multimedia WWW stop
#define KEYBOARD_KEY_SP_MM_WWWFORWARD   0xE069  // Multimedia WWW forward
#define KEYBOARD_KEY_SP_MM_WWWBACK      0xE06A  // Multimedia WWW back
#define KEYBOARD_KEY_SP_MM_MYCOMPUTER   0xE06B  // Multimedia my computer key
#define KEYBOARD_KEY_SP_MM_EMAIL        0xE06C  // Multimedia email key
#define KEYBOARD_KEY_SP_MM_MEDIASELECT  0xE06D  // Multimedia media select key

// * Public variables

extern bool keyboard_KeyState[256];

// * Public functions

char    keyboard_tochar(key_t key);     // Convert keycode to character
key_t   keyboard_waitkey();             // Wait for a key input
char    keyboard_waitchar();            // Wait for a character input

void    keyboard_process();             // Main keyboard controller process for handle key inputs