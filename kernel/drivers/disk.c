#include "drivers/disk.h"

bool disk_Initialized = false;  // Disk initialize status
bool disk_Supported = false;    // Disk support status

// Function for wait Busy status
void disk_waitbsy() {
    while (port_inb(DISK_IO_PORT + 7) & DISK_STATUS_BSY);
}

// Function for wait Data Request status
void disk_waitdrq() {
    while (!(port_inb(DISK_IO_PORT + 7) & DISK_STATUS_DRQ));
}

// Function for reset ATA Disk Controller
void disk_reset() {
    port_outb(0x3F6, 0x04);
    delay(100000);
    port_outb(0x3F6, 0x00);
}

// Function for check for ATA Disk Controller support
int disk_support() {
    if (disk_Initialized) { return disk_Supported; }

    port_outb(DISK_IO_PORT + 6, 0xA0);
    port_outb(DISK_IO_PORT + 7, 0xEC);

    uint8_t status = port_inb(DISK_IO_PORT + 7);
    if (status == 0) {
        disk_Initialized = true;
        disk_Supported = false;
        return 0;
    }

    disk_waitbsy();

    if (status & 0x01) {
        disk_Initialized = true;
        disk_Supported = false;
        return 0;
    }

    if (!(status & 0x08)) {
        disk_Initialized = true;
        disk_Supported = false;
        return 0;
    }

    if (
        port_inb(DISK_IO_PORT + 4) == 0x00 &&
        port_inb(DISK_IO_PORT + 5) == 0x00
    ) {
        disk_reset();
        disk_Initialized = true;
        disk_Supported = true;
        return 1;
    }

    disk_reset();
    disk_Initialized = true;
    disk_Supported = false;
    return 0;
}

// Function for read sector from disk and write to specific buffer
int disk_readSector(uint32_t lba, uint8_t* buffer) {
    if (!disk_support()) { return -1; }

    disk_waitbsy();

    port_outb(DISK_IO_PORT + 6, (lba >> 24) | 0xE0);
    port_outb(DISK_IO_PORT + 2, 1);
    port_outb(DISK_IO_PORT + 3, (uint8_t) lba);
    port_outb(DISK_IO_PORT + 4, (uint8_t)(lba >> 8));
    port_outb(DISK_IO_PORT + 5, (uint8_t)(lba >> 16));
    port_outb(DISK_IO_PORT + 7, DISK_CMD_READ_SECTORS);

    disk_waitbsy();

    if (port_inb(DISK_IO_PORT + 7) & DISK_STATUS_ERR) { return -1; }

    for (int i = 0; i < 256; ++i) {
        uint16_t data = port_inw(DISK_IO_PORT);
        buffer[i * 2] = (uint8_t)data;
        buffer[i * 2 + 1] = (uint8_t)(data >> 8);
    }

    return 0;
}

// Function for write sector to disk from specific buffer
int disk_writeSector(uint32_t lba, uint8_t* buffer) {
    if (!disk_support()) { return -1; }

    disk_waitbsy();

    port_outb(DISK_IO_PORT + 6, (lba >> 24) | 0xE0);
    port_outb(DISK_IO_PORT + 2, 1);
    port_outb(DISK_IO_PORT + 3, (uint8_t) lba);
    port_outb(DISK_IO_PORT + 4, (uint8_t)(lba >> 8));
    port_outb(DISK_IO_PORT + 5, (uint8_t)(lba >> 16));
    port_outb(DISK_IO_PORT + 7, DISK_CMD_WRITE_SECTORS);

    disk_waitbsy();

    if (port_inb(DISK_IO_PORT + 7) & DISK_STATUS_ERR) { return -1; }

    for (int i = 0; i < 256; ++i) {
        uint16_t data = (buffer[i * 2 + 1] << 8) | buffer[i * 2];
        port_outw(DISK_IO_PORT, data);
    }

    disk_waitbsy();
    return 0;
}