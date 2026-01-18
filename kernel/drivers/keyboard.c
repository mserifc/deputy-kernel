#include "drivers/keyboard.h"

bool keyboard_state[0x59];

bool KeyboardShiftstate = false;

bool keyboard_ShiftState = false;

// US Keyboard Layout
const uint8_t keyboard_Layout[128][2] = {
 // { X  ,  X },       Shift off    | Shift On      | Code
    { 0  ,  0 },    // Unknown      | Unknown       | 0x00
    
    {0x01,  0 },    // Escape       | None          | 0x01
    {'1' , '!'},    // Number 1     | Exclamation   | 0x02
    {'2' , '@'},    // Number 2     | At Sign       | 0x03
    {'3' , '#'},    // Number 3     | Pound Sign    | 0x04
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

    {0x1D,  0 },    // L Control    | None          | 0x1D
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
    {'`' , '`'},    // Backtick     | Tilde         | 0x29
    {0x2A,0x2A},    // L Shift      | L Shift       | 0x2A

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
    {0x36,0x36},    // R Shift      | R Shift       | 0x36
    {'*' , '*'},    // Asterisk     | None          | 0x37
    {0x38,  0 },    // L Alt        | None          | 0x38
    {' ' , ' '},    // Spacebar     | Spacebar      | 0x39
    {0x3A,0x3A},    // Caps Lock    | Caps Lock     | 0x3A

    {0x3B,  0 },    // F1 Key       | None          | 0x3B
    {0x3C,  0 },    // F2 Key       | None          | 0x3C
    {0x3D,  0 },    // F3 Key       | None          | 0x3D
    {0x3E,  0 },    // F4 Key       | None          | 0x3E
    {0x3F,  0 },    // F5 Key       | None          | 0x3F
    {0x40,  0 },    // F6 Key       | None          | 0x40
    {0x41,  0 },    // F7 Key       | None          | 0x41
    {0x42,  0 },    // F8 Key       | None          | 0x42
    {0x43,  0 },    // F9 Key       | None          | 0x43
    {0x44,  0 },    // F10 Key      | None          | 0x44

    {0x45,  0 },    // Number Lock  | None          | 0x45
    {0x46,  0 },    // Scroll Lock  | None          | 0x46
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

    {0x57,  0 },    // F11 Key      | Unknown       | 0x57
    {0x58,  0 },    // F12 Key      | Unknown       | 0x58
};

const uint8_t KeyboardKeyscanTable[216] = {
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

const uint8_t KeyboardShiftscanTable[(
    sizeof(KeyboardKeyscanTable) /
    sizeof(KeyboardKeyscanTable[0]))] = {
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

char keyboard_translate(uint8_t key) {
    char chr;
    if (key < 0x80) {
        chr = (char)keyboard_Layout[key][false];
    } else {
        chr = (char)keyboard_Layout[key][true];
    }
    return chr;
};

char keyboard_scankey() {
    uint8_t current, new, input;
    current = port_inb(KEYBOARD_PORT);
    while (1) {
        new = port_inb(KEYBOARD_PORT);
        if (new == keyboard_KEY_CAPSLOCK) {
            if (keyboard_ShiftState) {
                keyboard_state[keyboard_KEY_CAPSLOCK] = false;
                keyboard_ShiftState = false; continue;
            } else {
                keyboard_state[keyboard_KEY_CAPSLOCK] = true;
                keyboard_ShiftState = true; continue;
            }
        }
        if (
            new == keyboard_KEY_LEFT_SHIFT ||
            new == keyboard_KEY_RIGHT_SHIFT
        ) {
            if (!keyboard_state[keyboard_KEY_CAPSLOCK]) {
                keyboard_ShiftState = true; continue;
            } else { continue; }
        } else if (
            new == keyboard_KEY_LEFT_SHIFT + 0x80 ||
            new == keyboard_KEY_RIGHT_SHIFT + 0x80
        ) {
            if (!keyboard_state[keyboard_KEY_CAPSLOCK]) {
                keyboard_ShiftState = false; continue;
            } else { continue; }
        }
        if (current != new) { input = new; break; }
    }
    if (input < 0x80) {
        return (char)keyboard_Layout[input][keyboard_ShiftState];
    } else {
        return 0;
    }
}

uint8_t keyboard_scankeycode() {
    uint8_t current, new;
    current = port_inb(KEYBOARD_PORT);
    while (1) {
        new = port_inb(KEYBOARD_PORT);
        if (current != new) {
            return new;
        }
    }
    return 0;
}

char keyboard_scankey_stub() {
    uint8_t input = keyboard_scankeycode();
    if (input == 0x2A || input == 0x36) {
        KeyboardShiftstate = true;
        return '\0';
    }
    if (input == 0xAA || input == 0xB6) {
        KeyboardShiftstate = false;
        return '\0';
    }
    if (
        input == 0x0E ||
        input == 0x1C
    ) {
        return (char)input;
    }
    if (input < (sizeof(KeyboardKeyscanTable) /
        sizeof(KeyboardKeyscanTable[0]))) {
        if (!KeyboardShiftstate) {
            return (char)KeyboardKeyscanTable[input];
        } else if (KeyboardShiftstate) {
            return (char)KeyboardShiftscanTable[input];
        }
    }
    return -1;
}

// void keyboard_handler() {
//     printf("Keyboard Interrupt! IRQ: %d.", interrupts_PICGetIRR());
//     interrupts_PICSendEOI(IRQ_KEYBOARD);
// }

// void keyboard_init() {
//     for (int i = 0; i < 0x59; ++i) { keyboard_state[i] = false; }
//     interrupts_IDTSetGate(PIC1 + IRQ_KEYBOARD, (uint32_t)keyboard_handler);
//     interrupts_PICIRQEnable(IRQ_KEYBOARD);
// }