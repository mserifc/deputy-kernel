#include "detect.h"

// Memory Lower (Below 1MB) and Upper (Above 1MB) Sizes
size_t detect_MemoryLowerSize;
size_t detect_MemoryUpperSize;

// Boot Device Number
uint32_t detect_BootDevice;

// Boot Info Structure
multiboot_info_t* boot_info;

// Boot Devices (as string)
char* detect_BootDeviceStr[] = {
    "Invalid",
    "Harddisk",
    "Floppy",
    "CDROM",
    "USB",
    "Network",
    "SCSI"
};

// Function for get memory size
size_t detect_getMemorySize() { return detect_MemoryLowerSize + detect_MemoryUpperSize; }

// Function for get boot device number
uint32_t detect_getBootDevice() { return detect_BootDevice; }

// Function for get boot device as string
char* detect_getBootDeviceStr() { return detect_BootDeviceStr[detect_getBootDevice()]; }

// Function for initialize hardware detector
void detect_init(multiboot_info_t* info) {
    boot_info = info;                                   // Get bootloader boot info
    detect_MemoryLowerSize = boot_info->mem_lower;      // Get lower (Below 1MB) memory size
    detect_MemoryUpperSize = boot_info->mem_upper;      // Get upper (Above 1MB) memory size
    if (boot_info->flags & (1 << 4)) {                  // Check boot device flag
        detect_BootDevice = boot_info->boot_device;     // Get boot device if boot device flag set
    } else {
        detect_BootDevice = 0;                          // Else set boot device invalid
    }
}
