#pragma once

#include "types.h"
#include "multiboot.h"
#include "kernel.h"

// Enumeration for boot device numbers
enum detect_BOOTDEVICETABLE {
    BOOTDEVICE_INVALID  = 0,    // Invalid Device (Unknown Device)
    BOOTDEVICE_HARDDISK = 1,    // Harddisk Device
    BOOTDEVICE_FLOPPY   = 2,    // Floppy Device
    BOOTDEVICE_CDROM    = 3,    // CDROM Device
    BOOTDEVICE_USB      = 4,    // USB Device
    BOOTDEVICE_NET      = 5,    // Network Device
    BOOTDEVICE_SCSI     = 6     // Small Computer System Interface
};

typedef enum detect_BOOTDEVICETABLE detect_BootDeviceType;  // Define boot device type

// Function prototypes for get memory size and boot device
size_t detect_getMemorySize();      // Gives Memory Size
uint32_t detect_getBootDevice();    // Gives Boot Device
char* detect_getBootDeviceStr();    // Gives Boot Device (as string)

// Function prototype for initialize hardware detector
void detect_init(multiboot_info_t* info);   // Initialize Hardware Detector
