#pragma once

#include "types.h"
#include "platform/i386/port.h"
#include "common.h"

#define DISK_IO_PORT 0x01F0             // ATA Disk Controller I/O Port

#define DISK_CMD_RESET 0x04             // Controller reset command
#define DISK_CMD_READ_SECTORS 0x20      // Read sector command
#define DISK_CMD_WRITE_SECTORS 0x30     // Write sector command

#define DISK_STATUS_ERR 0x01            // Error Status
#define DISK_STATUS_BSY 0x80            // Busy Status
#define DISK_STATUS_DRQ 0x08            // Data Request Status (unused)

#define DISK_SECTOR_SIZE 512            // Sector size

// Function prototypes

void disk_waitbsy();        // Wait Busy Status
void disk_waitdrq();        // Wait Data Request Status (unused)

void disk_reset();          // Reset ATA Disk Controller
int disk_support();         // Check for ATA Disk Controller support

int disk_readSector(uint32_t lba, uint8_t* buffer);         // Read sector from disk
int disk_writeSector(uint32_t lba, uint8_t* buffer);        // Write sector to disk