#pragma once

#include "types.h"
#include "hardware/port.h"

#define DEVICE_INDEXPORT 0xCF8
#define DEVICE_DATAPORT 0xCFC

#define DEVICE_REG_VENDOR_DEVICE 1
#define DEVICE_REG_COMMAND_STATUS 2
#define DEVICE_REG_REV_PIF_SUBC_CLASS 3
#define DEVICE_REG_CACHE_LTIMER_HTYPE_BIST 4

#define DEVICE_CMDREG_IOSPACE (1 << 0)
#define DEVICE_CMDREG_MEMSPACE (1 << 1)
#define DEVICE_CMDREG_BUSMASTER (1 << 2)
#define DEVICE_CMDREG_SPECCYCLES (1 << 3)
#define DEVICE_CMDREG_MEMWR_INVDATE (1 << 4)
#define DEVICE_CMDREG_PALETTESNOOP (1 << 5)
#define DEVICE_CMDREG_PARITYERRRESP (1 << 6)
#define DEVICE_CMDREG_SERRENABLE (1 << 8)
#define DEVICE_CMDREG_FASTBTBENABLE (1 << 9)
#define DEVICE_CMDREG_INTDISABLE (1 << 10)

#define DEVICE_STSREG_INTSTATUS (1 << 3)
#define DEVICE_STSREG_CAPABILITIES (1 << 4)
#define DEVICE_STSREG_66MHZCAPABLE (1 << 5)
#define DEVICE_STSREG_FASTBTBCAPABLE (1 << 7)
#define DEVICE_STSREG_MDATAPARITYERR (1 << 8)
#define DEVICE_STSREG_DEVSELTIMING (3 << 9)
#define DEVICE_STSREG_SIGTARGETABORT (1 << 11)
#define DEVICE_STSREG_RCVTARGETABORT (1 << 12)
#define DEVICE_STSREG_RCVMASTERABORT (1 << 13)
#define DEVICE_STSREG_SIGSYSTEMERR (1 << 14)
#define DEVICE_STSREG_DETECTPARITYERR (1 << 15)

#define DEVICE_BARTYPE_MM 0
#define DEVICE_BARTYPE_IO 1

typedef struct {
    bool prefetch;
    uint8_t* addr;
    uint8_t type;
} BAR_t;

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint32_t port;
    uint16_t vendor;
    uint16_t device;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t interface;
    uint8_t subclass;
    uint8_t class;
} device_t;

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pciCheckVendor(uint8_t bus, uint8_t slot);

uint32_t device_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg);
void device_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg, uint32_t value);
void device_getBAR(BAR_t* bar, uint8_t bus, uint8_t slot, uint8_t func, uint16_t num);
void device_get(device_t* dev, uint8_t bus, uint8_t slot, uint8_t func);