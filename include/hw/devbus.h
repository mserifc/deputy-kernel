#pragma once

#include "types.h"

// * Constants

// Classic PCI ports

#define DEVBUS_PCI_INDEXPORT    0xCF8                   // PCI index port
#define DEVBUS_PCI_DATAPORT     0xCFC                   // PCI data port

// Device registers

#define DEVBUS_REG_VENDOR_DEVICE            1           // Vendor and device (vendor-specific model) register
#define DEVBUS_REG_COMMAND_STATUS           2           // Command and status register
#define DEVBUS_REG_REV_PIF_SUBC_CLASS       3           // Class, subclass, interface and revision register
#define DEVBUS_REG_CACHE_LTIMER_HTYPE_BIST  4           // Cache line size, latency timer, header type and BIST register

// Command register bits

#define DEVBUS_CMDREG_IOSPACE           (1 << 0)        // I/O Space bit in command register
#define DEVBUS_CMDREG_MEMSPACE          (1 << 1)        // Memory Space bit in command register
#define DEVBUS_CMDREG_BUSMASTER         (1 << 2)        // Bus Master bit in command register
#define DEVBUS_CMDREG_SPECCYCLES        (1 << 3)        // Special Cycles bit in command register
#define DEVBUS_CMDREG_MEMWR_INVDATE     (1 << 4)        // Memory Write and Invalidate Enable bit in command register
#define DEVBUS_CMDREG_PALETTESNOOP      (1 << 5)        // VGA Palette Snoop bit in command register
#define DEVBUS_CMDREG_PARITYERRRESP     (1 << 6)        // Parity Error Response bit in command register
#define DEVBUS_CMDREG_SERRENABLE        (1 << 8)        // SERR# Enable bit in command register
#define DEVBUS_CMDREG_FASTBTBENABLE     (1 << 9)        // Fast Back-to-Back Enable bit in command register
#define DEVBUS_CMDREG_INTDISABLE        (1 << 10)       // Interrupt Disable bit in command register

// Status register bits

#define DEVBUS_STSREG_INTSTATUS         (1 << 3)        // Interrupt Status bit in status register
#define DEVBUS_STSREG_CAPABILITIES      (1 << 4)        // Capabilities List bit in status register
#define DEVBUS_STSREG_66MHZCAPABLE      (1 << 5)        // 66 MHz Capable bit in status register
#define DEVBUS_STSREG_FASTBTBCAPABLE    (1 << 7)        // Fast Back-to-Back Capable bit in status register
#define DEVBUS_STSREG_MDATAPARITYERR    (1 << 8)        // Master Data Parity Error bit in status register
#define DEVBUS_STSREG_DEVSELTIMING      (3 << 9)        // DEVSEL Timing bits in status register
#define DEVBUS_STSREG_SIGTARGETABORT    (1 << 11)       // Signaled Target Abort bit in status register
#define DEVBUS_STSREG_RCVTARGETABORT    (1 << 12)       // Received Target Abort bit in status register
#define DEVBUS_STSREG_RCVMASTERABORT    (1 << 13)       // Received Master Abort bit in status register
#define DEVBUS_STSREG_SIGSYSTEMERR      (1 << 14)       // Signaled System Error bit in status register
#define DEVBUS_STSREG_DETECTPARITYERR   (1 << 15)       // Detected Parity Error bit in status register

// Device location limits

#define DEVBUS_MAX_BUSES 256                            // Maximum bus count in PCI/PCIe
#define DEVBUS_MAX_SLOTS 32                             // Maximum slot/device count in a bus
#define DEVBUS_MAX_FUNCS 8                              // Maximum function count in a slot/device

// Base address I/O types

#define DEVBUS_BARTYPE_MM 0                             // Memory-mapped I/O base address type
#define DEVBUS_BARTYPE_IO 1                             // Port I/O base address type

// * Types and structures

// Structure of base address register
typedef struct {
    size_t addr;        // Base address
    uint8_t type;       // Type (MMIO or PortIO)
    bool prefetch;      // Prefetchable
} devbus_BAR_t;

// Structure of device
typedef struct {
    uint8_t bus; uint8_t slot; uint8_t func;    // Device location
    uint16_t vendor; uint16_t device;           // Vendor and device (vendor-specific model) IDs
    uint8_t revision; uint8_t interface;        // Interface and revision values
    uint8_t subclass; uint8_t class;            // Class and subclass values
} devbus_Device_t;

// * Functions

uint32_t devbus_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg);                     // Read register
void devbus_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg, uint32_t value);        // Write register
void devbus_getBAR(devbus_BAR_t* bar, uint8_t bus, uint8_t slot, uint8_t func, uint16_t num);   // Get base address
void devbus_get(devbus_Device_t* dev, uint8_t bus, uint8_t slot, uint8_t func);                 // Get all information
void devbus_init(void);                                                                         // Initialize device bus