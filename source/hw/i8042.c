#include "hw/i8042.h"

#include "kernel.h"
#include "hw/acpi.h"
#include "hw/port.h"

#include "drv/keyboard.h"
#include "drv/mouse.h"

#define I8042_BOOTARCHFLAG 2

#define I8042_INDEXPORT 0x64
#define I8042_DATAPORT 0x60

#define i8042_STS_OUTPUTFULL (1 << 0)
#define i8042_STS_INPUTFULL (1 << 1)
#define i8042_STS_SYSTEMFLAG (1 << 2)
#define I8042_STS_2NDOUTFULL (1 << 5)
#define i8042_STS_TIMEOUTERR (1 << 6)
#define i8042_STS_PARITYERR (1 << 7)

#define I8042_CMD_DISABLE1STPORT 0xAD
#define I8042_CMD_DISABLE2NDPORT 0xA7
#define I8042_CMD_READCONFIGBYTE 0x20
#define I8042_CMD_WRITECONFIGBYTE 0x60
#define I8042_CMD_SELFTEST 0xAA
#define I8042_CMD_ENABLE2NDPORT 0xA8
#define I8042_CMD_TEST1STPORT 0xAB
#define I8042_CMD_TEST2NDPORT 0xA9
#define I8042_CMD_ENABLE1STPORT 0xAE
#define I8042_CMD_ENABLE2NDPORT 0xA8
#define I8042_CMD_SEND2NDPORT 0xD4
#define I8042_CMD_READOUTPORT 0xD0

#define I8042_OUT_SYSRST (1 << 0)
#define I8042_OUT_A20GATE (1 << 1)
#define I8042_OUT_2NDPORTCLOCK (1 << 2)
#define I8042_OUT_2NDPORTDATA (1 << 3)
#define I8042_OUT_1STOUTFULL (1 << 4)
#define I8042_OUT_2NDOUTFULL (1 << 5)
#define I8042_OUT_1STPORTCLOCK (1 << 6)
#define I8042_OUT_1STPORTDATA (1 << 7)

#define I8042_SELFTEST_SUCCESS 0x55

#define I8042_CFG_1STPORTINT (1 << 0)
#define I8042_CFG_2NDPORTINT (1 << 1)
#define I8042_CFG_SYSTEMFLAG (1 << 2)
#define I8042_CFG_1STPORTCLOCK (1 << 4)
#define I8042_CFG_2NDPORTCLOCK (1 << 5)
#define I8042_CFG_1STPORTTRANSLATION (1 << 6)

#define I8042_DEVCMD_RESET 0xFF
#define I8042_PTRDEVCMD_ENABLEDATAREP 0xF4
#define I8042_DEVRET_ACKNOW 0xFA
#define I8042_DEVRET_TESTPASS 0xAA
#define I8042_DEVRET_TESTFAIL 0xFC

bool i8042_InitLock = false;

bool i8042_1stPortSupport = false;
bool i8042_2ndPortSupport = false;

char* i8042_PortErrorLog[] = {
    "Test passed somehow",
    "Clock line stuck low",
    "Clock line stuck high",
    "Data line stuck low",
    "Data line stuck high"
};

#define I8042_KEY_EXTENDED 0xE0
#define I8042_KEY_RELEASE 0xF0

static uint8_t i8042_KeyTable[256] = {
    [0x01] = KEY_F9,
    [0x03] = KEY_F5,
    [0x04] = KEY_F3,
    [0x05] = KEY_F1,
    [0x06] = KEY_F2,
    [0x07] = KEY_F12,
    [0x09] = KEY_F10,
    [0x0A] = KEY_F8,
    [0x0B] = KEY_F6,
    [0x0C] = KEY_F4,
    [0x0D] = KEY_TAB,
    [0x0E] = KEY_BACKTICK,
    [0x11] = KEY_ALT,
    [0x12] = KEY_SHIFT,
    [0x14] = KEY_CTRL,
    [0x15] = KEY_Q,
    [0x16] = KEY_1,
    [0x1A] = KEY_Z,
    [0x1B] = KEY_S,
    [0x1C] = KEY_A,
    [0x1D] = KEY_W,
    [0x1E] = KEY_2,
    [0x21] = KEY_C,
    [0x22] = KEY_X,
    [0x23] = KEY_D,
    [0x24] = KEY_E,
    [0x25] = KEY_4,
    [0x26] = KEY_3,
    [0x29] = KEY_SPACE,
    [0x2A] = KEY_V,
    [0x2B] = KEY_F,
    [0x2C] = KEY_T,
    [0x2D] = KEY_R,
    [0x2E] = KEY_5,
    [0x31] = KEY_N,
    [0x32] = KEY_B,
    [0x33] = KEY_H,
    [0x34] = KEY_G,
    [0x35] = KEY_Y,
    [0x36] = KEY_6,
    [0x3A] = KEY_M,
    [0x3B] = KEY_J,
    [0x3C] = KEY_U,
    [0x3D] = KEY_7,
    [0x3E] = KEY_8,
    [0x41] = KEY_COMMA,
    [0x42] = KEY_K,
    [0x43] = KEY_I,
    [0x44] = KEY_O,
    [0x45] = KEY_0,
    [0x46] = KEY_9,
    [0x49] = KEY_PERIOD,
    [0x4A] = KEY_SLASH,
    [0x4B] = KEY_L,
    [0x4C] = KEY_SEMICOLON,
    [0x4D] = KEY_P,
    [0x4E] = KEY_MINUS,
    [0x52] = KEY_QUOTE,
    [0x54] = KEY_OPENBRACKET,
    [0x55] = KEY_EQUAL,
    [0x58] = KEY_CAPSLOCK,
    [0x59] = KEY_SHIFT,
    [0x5A] = KEY_ENTER,
    [0x5B] = KEY_CLOSEBRACKET,
    [0x5D] = KEY_BACKSLASH,
    [0x66] = KEY_BACKSPACE,
    [0x69] = KEY_NP_1,
    [0x6B] = KEY_NP_4,
    [0x6C] = KEY_NP_7,
    [0x70] = KEY_NP_0,
    [0x71] = KEY_NP_PERIOD,
    [0x72] = KEY_NP_2,
    [0x73] = KEY_NP_5,
    [0x74] = KEY_NP_6,
    [0x75] = KEY_NP_8,
    [0x76] = KEY_ESC,
    [0x77] = KEY_NUMLOCK,
    [0x78] = KEY_F11,
    [0x79] = KEY_NP_PLUS,
    [0x7A] = KEY_NP_3,
    [0x7B] = KEY_NP_MINUS,
    [0x7C] = KEY_NP_ASTERISK,
    [0x7D] = KEY_NP_9,
    [0x7E] = KEY_SCROLLLOCK,
    [0x83] = KEY_F7
};

static uint8_t i8042_ExtKeyTable[256] = {
    [0x11] = KEY_ALTGR,
    [0x14] = KEY_CTRL,
    [0x1F] = KEY_META,
    [0x27] = KEY_META,
    [0x4A] = KEY_NP_SLASH,
    [0x5A] = KEY_NP_ENTER,
    [0x69] = KEY_END,
    [0x6B] = KEY_ARROWLEFT,
    [0x6C] = KEY_HOME,
    [0x70] = KEY_INSERT,
    [0x71] = KEY_DELETE,
    [0x72] = KEY_ARROWDOWN,
    [0x74] = KEY_ARROWRIGHT,
    [0x75] = KEY_ARROWUP,
    [0x7A] = KEY_PAGEDOWN,
    [0x7D] = KEY_PAGEUP
};

// int ptrx = 0;
// int ptry = 0;
void i8042_proc() {
    if (!i8042_InitLock) { return; }
    if (port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) {
        if (port_inb(I8042_INDEXPORT) & I8042_STS_2NDOUTFULL) {
            uint8_t s = port_inb(I8042_DATAPORT);
            uint8_t timeout = 5;
            while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout) { delay(1); }
            if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL)) { return; }
            char x = port_inb(I8042_DATAPORT);
            timeout = 5;
            while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout) { delay(1); }
            if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL)) { return; }
            char y = port_inb(I8042_DATAPORT);
            // INFO("mouse: 0x%x, %d, %d", s, x, y);
            // ptrx += (x/1);
            // ptry += ((-y)/1);
            // if (ptrx < 0) { ptrx = 0; }
            // if (ptrx >= 80) { ptrx = 79; }
            // if (ptry < 0) { ptry = 0; }
            // if (ptry >= 25) { ptry = 24; }
            // uint16_t* vga = (uint16_t*)0xB8000;
            // for (int i = 0; i < 80*25; ++i) { vga[i] = 0; }
            // vga[(ptry * 80) + ptrx] = 0x0F00 | ((s & 1) ? 'X' : '+');
            if (((s>>3)&1) && !((s>>6)&3)) { mouse_send(s, x, y); }
        } else {
            bool extflag = false, relflag = false;
            rescan: delay(1); uint8_t scan = port_inb(I8042_DATAPORT);
            if (scan == I8042_KEY_EXTENDED) { extflag = true; goto rescan; }
            if (scan == I8042_KEY_RELEASE) { relflag = true; goto rescan; }
            key_t key = extflag ? i8042_ExtKeyTable[scan] : i8042_KeyTable[scan];
            if (key == KEY_UNKNOWN) { return; } if (relflag) { key |= KEY_RELEASE; } key_send(key);
        }
    }
}

int i8042_init() {
    if (i8042_InitLock) { return -1; } i8042_InitLock = true;
    // Check PS/2 Controller support
    if (acpiTable.support && acpiTable.Revision > 0 &&  // Check ACPI support (PS/2 also supported if ACPI not supported)
        !(acpiTable.fadt->BootArchitectureFlags & I8042_BOOTARCHFLAG)) {    // Check I8042 bit in boot arch flags
        WARN("PS/2 Controller not supported"); return -1;                   // Log if not exists
    } i8042_1stPortSupport = true;
    // Disable PS/2 ports for controller initialization
    port_outb(I8042_INDEXPORT, I8042_CMD_DISABLE1STPORT);
    port_outb(I8042_INDEXPORT, I8042_CMD_DISABLE2NDPORT);
    // Read data and discard to flush controller's output buffer (prevent data stucks)
    port_inb(I8042_DATAPORT);
    // Read the controller configuration byte
    port_outb(I8042_INDEXPORT, I8042_CMD_READCONFIGBYTE);
    uint32_t timeout = 100000;
    while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout);
    if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL))
        { ERR("PS/2 controller initialization timed out (stage 1)"); return -1; }
    uint8_t cfg = port_inb(I8042_DATAPORT);
    // Perform controller self test
    port_outb(I8042_INDEXPORT, I8042_CMD_SELFTEST);
    timeout = 100000;
    while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout);
    if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL))
        { ERR("PS/2 controller initialization timed out (stage 2)"); return -1; }
    if (port_inb(I8042_DATAPORT) != I8042_SELFTEST_SUCCESS)
        { ERR("PS/2 self test failed"); return -1; }
    // Make changes and write the controller configuration byte
    cfg &= (uint8_t)~I8042_CFG_1STPORTINT;
    cfg &= (uint8_t)~I8042_CFG_1STPORTTRANSLATION;
    cfg &= (uint8_t)~I8042_CFG_1STPORTCLOCK;
    port_outb(I8042_INDEXPORT, I8042_CMD_WRITECONFIGBYTE);
    port_outb(I8042_DATAPORT, cfg);
    // Determine if there are 2 channels
    port_outb(I8042_INDEXPORT, I8042_CMD_ENABLE2NDPORT);
    port_outb(I8042_INDEXPORT, I8042_CMD_READCONFIGBYTE);
    timeout = 100000;
    while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout);
    if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL))
        { ERR("PS/2 controller initialization timed out (stage 3)"); return -1; }
    if ((port_inb(I8042_DATAPORT) & I8042_CFG_2NDPORTCLOCK))
        { WARN("PS/2 pointing device not supported"); i8042_2ndPortSupport = false; }
    else { i8042_2ndPortSupport = true; }
    port_outb(I8042_INDEXPORT, I8042_CMD_DISABLE2NDPORT);
    cfg &= (uint8_t)~I8042_CFG_2NDPORTINT;
    cfg &= (uint8_t)~I8042_CFG_2NDPORTCLOCK;
    port_outb(I8042_INDEXPORT, I8042_CMD_WRITECONFIGBYTE);
    port_outb(I8042_DATAPORT, cfg);
    // Perform interface tests
    if (i8042_1stPortSupport) {
        port_outb(I8042_INDEXPORT, I8042_CMD_TEST1STPORT);
        timeout = 100000;
        while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout);
        if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL))
            { ERR("PS/2 controller initialization timed out (stage 4)"); return -1; }
        uint8_t result = port_inb(I8042_DATAPORT);
        if (result != 0) {
            if (result > 4) {
                WARN("PS/2 keyboard test failed: %s", i8042_PortErrorLog[result]);
            } else { WARN("PS/2 keyboard test failed: Unknown error"); }
            i8042_1stPortSupport = false;
        }
    }
    if (i8042_2ndPortSupport) {
        port_outb(I8042_INDEXPORT, I8042_CMD_TEST2NDPORT);
        timeout = 100000;
        while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout);
        if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL))
            { ERR("PS/2 controller initialization timed out (stage 5)"); return -1; }
        uint8_t result = port_inb(I8042_DATAPORT);
        if (result != 0) {
            if (result > 4) {
                WARN("PS/2 pointing device test failed: %s", i8042_PortErrorLog[result]);
            } else { WARN("PS/2 pointing device test failed: Unknown error"); }
            i8042_2ndPortSupport = false;
        }
    }
    // Enable and reset devices
    port_outb(I8042_INDEXPORT, I8042_CMD_ENABLE1STPORT);
    port_outb(I8042_INDEXPORT, I8042_CMD_ENABLE2NDPORT);
    if (i8042_1stPortSupport) {
        timeout = 100000;
        while ((port_inb(I8042_INDEXPORT) & i8042_STS_INPUTFULL) && --timeout);
        if (port_inb(I8042_INDEXPORT) & i8042_STS_INPUTFULL)
            { ERR("PS/2 controller initialization timed out (stage 6)"); return -1; }
        port_outb(I8042_DATAPORT, I8042_DEVCMD_RESET);
        timeout = 50000000;
        while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout);
        if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL))
            { WARN("PS/2 keyboard not connected"); i8042_1stPortSupport = false; }
        else {
            port_inb(I8042_DATAPORT);   // Acknowledge
            while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout);
            if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL))
                { WARN("PS/2 keyboard not connected"); i8042_1stPortSupport = false; }
            else {
                uint8_t result = port_inb(I8042_DATAPORT);
                if (result == I8042_DEVRET_TESTFAIL) {
                    WARN("PS/2 keyboard reset failed"); i8042_1stPortSupport = false;
                } else if (result != I8042_DEVRET_TESTPASS) {
                    WARN("PS/2 keyboard reset response invalid. Ignoring keyboard");
                    i8042_1stPortSupport = false;
                }
            }
        }
    }
    if (i8042_2ndPortSupport) {
        port_outb(I8042_INDEXPORT, I8042_CMD_SEND2NDPORT);
        timeout = 100000;
        while ((port_inb(I8042_INDEXPORT) & i8042_STS_INPUTFULL) && --timeout);
        if (port_inb(I8042_INDEXPORT) & i8042_STS_INPUTFULL)
            { ERR("PS/2 controller initialization timed out (stage 7)"); return -1; }
        port_outb(I8042_DATAPORT, I8042_DEVCMD_RESET);
        timeout = 50000000;
        while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout);
        if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL))
            { WARN("PS/2 pointing device not connected"); i8042_2ndPortSupport = false; }
        else {
            port_inb(I8042_DATAPORT);   // Acknowledge
            while (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL) && --timeout);
            if (!(port_inb(I8042_INDEXPORT) & i8042_STS_OUTPUTFULL))
                { WARN("PS/2 pointing device not connected"); i8042_2ndPortSupport = false; }
            else {
                uint8_t result = port_inb(I8042_DATAPORT);
                if (result == I8042_DEVRET_TESTFAIL) {
                    WARN("PS/2 pointing device reset failed"); i8042_2ndPortSupport = false;
                } else if (result != I8042_DEVRET_TESTPASS) {
                    WARN("PS/2 pointing device reset response invalid. Ignoring pointing device");
                    i8042_2ndPortSupport = false;
                }
            }
        }
        // Enable PS/2 pointing device
        port_outb(I8042_INDEXPORT, I8042_CMD_SEND2NDPORT);
        timeout = 100000;
        while ((port_inb(I8042_INDEXPORT) & i8042_STS_INPUTFULL) && --timeout);
        if (port_inb(I8042_INDEXPORT) & i8042_STS_INPUTFULL)
            { ERR("PS/2 controller initialization timed out (stage 8)"); return -1; }
        port_outb(I8042_DATAPORT, I8042_PTRDEVCMD_ENABLEDATAREP);
    }
}