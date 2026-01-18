#include "drivers/keyboard.h"
#include "filesystem/ramfs.h"

// United States keyboard layout
const char keyboard_Layout_US[128][2] = {
//  { x  ,  X },       Shift off    | Shift On      | Code
    { 0  ,  0 },    // Unknown      | Unknown       | 0x00
    
    { 0  ,  0 },    // Escape       | None          | 0x01
    {'1' , '!'},    // Number 1     | Exclamation   | 0x02
    {'2' , '@'},    // Number 2     | At Sign       | 0x03
    {'3' , '#'},    // Number 3     | Hash Sign     | 0x04
    {'4' , '$'},    // Number 4     | Dollar Sign   | 0x05
    {'5' , '%'},    // Number 5     | Percent Sign  | 0x06
    {'6' , '^'},    // Number 6     | Caret         | 0x07
    {'7' , '&'},    // Number 7     | Ampersand     | 0x08
    {'8' , '*'},    // Number 8     | Asterisk      | 0x09
    {'9' , '('},    // Number 9     | L Parenthesis | 0x0A
    {'0' , ')'},    // Number 0     | R Parenthesis | 0x0B
    {'-' , '_'},    // Minus        | Underscore    | 0x0C
    {'=' , '+'},    // Equal        | Plus          | 0x0D
    {'\b','\b'},    // Backspace    | Backspace     | 0x0E

    {'\t','\t'},    // Tab          | Tab           | 0x0F
    {'q' , 'Q'},    // Lowercase q  | Uppercase Q   | 0x10
    {'w' , 'W'},    // Lowercase w  | Uppercase W   | 0x11
    {'e' , 'E'},    // Lowercase e  | Uppercase E   | 0x12
    {'r' , 'R'},    // Lowercase r  | Uppercase R   | 0x13
    {'t' , 'T'},    // Lowercase t  | Uppercase T   | 0x14
    {'y' , 'Y'},    // Lowercase y  | Uppercase Y   | 0x15
    {'u' , 'U'},    // Lowercase u  | Uppercase U   | 0x16
    {'i' , 'I'},    // Lowercase i  | Uppercase I   | 0x17
    {'o' , 'O'},    // Lowercase o  | Uppercase O   | 0x18
    {'p' , 'P'},    // Lowercase p  | Uppercase P   | 0x19
    {'[' , '{'},    // L Bracket    | L Brace       | 0x1A
    {']' , '}'},    // R Bracket    | R Brace       | 0x1B
    {'\n','\n'},    // Enter        | Enter         | 0x1C

    { 0  ,  0 },    // L Control    | None          | 0x1D
    {'a' , 'A'},    // Lowercase a  | Uppercase A   | 0x1E
    {'s' , 'S'},    // Lowercase s  | Uppercase S   | 0x1F
    {'d' , 'D'},    // Lowercase d  | Uppercase D   | 0x20
    {'f' , 'F'},    // Lowercase f  | Uppercase F   | 0x21
    {'g' , 'G'},    // Lowercase g  | Uppercase G   | 0x22
    {'h' , 'H'},    // Lowercase h  | Uppercase H   | 0x23
    {'j' , 'J'},    // Lowercase j  | Uppercase J   | 0x24
    {'k' , 'K'},    // Lowercase k  | Uppercase K   | 0x25
    {'l' , 'L'},    // Lowercase l  | Uppercase L   | 0x26
    {';' , ':'},    // Semicolon    | Colon         | 0x27
    {'\'', '"'},    // Quote        | Double Quote  | 0x28
    {'`' , '~'},    // Backtick     | Tilde         | 0x29
    { 0  ,  0 },    // L Shift      | L Shift       | 0x2A

    {'\\', '|'},    // Backslash    | Pipe          | 0x2B
    {'z' , 'Z'},    // Lowercase z  | Uppercase Z   | 0x2C
    {'x' , 'X'},    // Lowercase x  | Uppercase X   | 0x2D
    {'c' , 'C'},    // Lowercase c  | Uppercase C   | 0x2E
    {'v' , 'V'},    // Lowercase v  | Uppercase V   | 0x2F
    {'b' , 'B'},    // Lowercase b  | Uppercase B   | 0x30
    {'n' , 'N'},    // Lowercase n  | Uppercase N   | 0x31
    {'m' , 'M'},    // Lowercase m  | Uppercase M   | 0x32
    {',' , '<'},    // Comma        | Less Than     | 0x33
    {'.' , '>'},    // Dot          | Greater Than  | 0x34
    {'/' , '?'},    // Slash        | Question Mark | 0x35
    { 0  ,  0 },    // R Shift      | R Shift       | 0x36
    {'*' , '*'},    // Asterisk     | None          | 0x37
    { 0  ,  0 },    // L Alt        | None          | 0x38
    {' ' , ' '},    // Spacebar     | Spacebar      | 0x39
    { 0  ,  0 },    // Caps Lock    | Caps Lock     | 0x3A

    { 0  ,  0 },    // F1 Key       | None          | 0x3B
    { 0  ,  0 },    // F2 Key       | None          | 0x3C
    { 0  ,  0 },    // F3 Key       | None          | 0x3D
    { 0  ,  0 },    // F4 Key       | None          | 0x3E
    { 0  ,  0 },    // F5 Key       | None          | 0x3F
    { 0  ,  0 },    // F6 Key       | None          | 0x40
    { 0  ,  0 },    // F7 Key       | None          | 0x41
    { 0  ,  0 },    // F8 Key       | None          | 0x42
    { 0  ,  0 },    // F9 Key       | None          | 0x43
    { 0  ,  0 },    // F10 Key      | None          | 0x44

    { 0  ,  0 },    // Number Lock  | None          | 0x45
    { 0  ,  0 },    // Scroll Lock  | None          | 0x46
    {'7' , '7'},    // KP Number 7  | KP Number 7   | 0x47
    {'8' , '8'},    // KP Number 8  | KP Number 8   | 0x48
    {'9' , '9'},    // KP Number 9  | KP Number 9   | 0x49
    {'-' , '-'},    // KP Minus     | KP Minus      | 0x4A
    {'4' , '4'},    // KP Number 4  | KP Number 4   | 0x4B
    {'5' , '5'},    // KP Number 5  | KP Number 5   | 0x4C
    {'6' , '6'},    // KP Number 6  | KP Number 6   | 0x4D
    {'+' , '+'},    // KP Plus      | KP Plus       | 0x4E
    {'1' , '1'},    // KP Number 1  | KP Number 1   | 0x4F
    {'2' , '2'},    // KP Number 2  | KP Number 2   | 0x50
    {'3' , '3'},    // KP Number 3  | KP Number 3   | 0x51
    {'0' , '0'},    // KP Number 0  | KP Number 0   | 0x52
    {'.' , '.'},    // KP Dot       | KP Dot        | 0x53

    { 0  ,  0 },    // Unknown      | Unknown       | 0x54
    { 0  ,  0 },    // Unknown      | Unknown       | 0x55
    { 0  ,  0 },    // Unknown      | Unknown       | 0x56

    { 0  ,  0 },    // F11 Key      | Unknown       | 0x57
    { 0  ,  0 },    // F12 Key      | Unknown       | 0x58
};

// Keyboard key states
bool keyboard_KeyState[256];

/**
 * @brief Function for convert keycode to character
 * 
 * @param key Keycode
 * 
 * @return Character
 */
char keyboard_tochar(key_t key) {
    if (((key & 0xFF00) >> 8) == KEYBOARD_KEY_SPECIAL) { return 0; }
    bool shift = false;
    if (
        keyboard_KeyState[KEYBOARD_KEY_CAPSLOCK] ||
        keyboard_KeyState[KEYBOARD_KEY_LSHIFT] ||
        keyboard_KeyState[KEYBOARD_KEY_RSHIFT]
    ) { shift = true; }
    uint8_t chr = (uint8_t)(key & 0xFF);
    return (char)keyboard_Layout_US[chr][shift];
}

/**
 * @brief Function for wait for a key input from keyboard
 * 
 * @return Entered keycode
 */
key_t keyboard_waitkey() {
    uint16_t result = 0x0000;
    int chardev = open("/dev/keyboard", O_RDONLY);
    if (chardev == -1) { return result; }
    waitforkey:
    uint8_t key; size_t n = 0; while (n != 1) { yield(); n = read(chardev, &key, 1); }
    if (key == KEYBOARD_KEY_SPECIAL) {
        size_t n = read(chardev, &key, 1); if (n != 1) { return result; }
        if (
            key >= KEYBOARD_KEY_SP_MM_PREVIOUS + KEYBOARD_KEY_RELEASE &&
            key <= KEYBOARD_KEY_SP_MM_MEDIASELECT + KEYBOARD_KEY_RELEASE
        ) { goto waitforkey; }
        result = ((uint16_t)KEYBOARD_KEY_SPECIAL << 8) | key;
        return result;
    }
    if (key == KEYBOARD_KEY_CAPSLOCK) {
        if (keyboard_KeyState[KEYBOARD_KEY_CAPSLOCK]) {
            keyboard_KeyState[KEYBOARD_KEY_CAPSLOCK] = false;
        } else {
            keyboard_KeyState[KEYBOARD_KEY_CAPSLOCK] = true;
        }
        goto waitforkey;
    }
    if (key == KEYBOARD_KEY_LSHIFT) {
        keyboard_KeyState[KEYBOARD_KEY_LSHIFT] = true;
        goto waitforkey;
    } else if (key == KEYBOARD_KEY_LSHIFT + KEYBOARD_KEY_RELEASE) {
        keyboard_KeyState[KEYBOARD_KEY_LSHIFT] = false;
        goto waitforkey;
    }
    if (key == KEYBOARD_KEY_RSHIFT) {
        keyboard_KeyState[KEYBOARD_KEY_RSHIFT] = true;
        goto waitforkey;
    } else if (key == KEYBOARD_KEY_RSHIFT + KEYBOARD_KEY_RELEASE) {
        keyboard_KeyState[KEYBOARD_KEY_RSHIFT] = false;
        goto waitforkey;
    }
    if (
        key >= KEYBOARD_KEY_ESCAPE + KEYBOARD_KEY_RELEASE &&
        key <= KEYBOARD_KEY_F12 + KEYBOARD_KEY_RELEASE
    ) { goto waitforkey; }
    result = (uint16_t)(key & 0x00FF);
    return result;
}

/**
 * @brief Function for wait for a character input from keyboard
 * 
 * @return Entered character input
 */
char keyboard_waitchar() {
    uint16_t input = keyboard_waitkey();
    if (((input & 0xFF00) >> 8) == KEYBOARD_KEY_SPECIAL) { return 0; }
    bool shift = false;
    if (
        keyboard_KeyState[KEYBOARD_KEY_CAPSLOCK] ||
        keyboard_KeyState[KEYBOARD_KEY_LSHIFT] ||
        keyboard_KeyState[KEYBOARD_KEY_RSHIFT]
    ) { shift = true; }
    uint8_t key = (uint8_t)(input & 0xFF);
    return (char)keyboard_Layout_US[key][shift];
}

/**
 * @brief Main function of PS/2 keyboard controller process for handle key inputs
 */
void keyboard_process() {
    uint8_t* chardev = ramfs_readFile("/dev/keyboard");
    if (chardev == NULL) { PANIC("Unable to read device /dev/keyboard"); }
    uint32_t* bufsize = (uint32_t*)chardev;
    while (true) {
        if (port_inb(KEYBOARD_STATUSPORT) & 1) {
            if (bufsize[0] < MEMORY_BLOCKSIZE - sizeof(uint32_t)) {
                uint8_t data = port_inb(KEYBOARD_DATAPORT);
                chardev[sizeof(uint32_t) + bufsize[0]] = data;
                ++bufsize[0];
            }
        } else { yield(); }
    }
}