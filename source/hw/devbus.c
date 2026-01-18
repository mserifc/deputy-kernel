#include "hw/devbus.h"

#include "kernel.h"
#include "hw/port.h"
#include "hw/acpi.h"

// * Types and structures

// Structure of PCIe device in memory
typedef struct {
    uint16_t vendor;            // Vendor ID
    uint16_t device;            // Device ID (Vendor-specific model ID)
    uint16_t command;           // Command register
    uint16_t status;            // Status register
    uint8_t revision;           // Revision
    uint8_t interface;          // Interface
    uint8_t subclass;           // Subclass
    uint8_t class;              // Class
    uint8_t cachesize;          // Cache line size
    uint8_t latencytimer;       // Latency timer
    uint8_t headertype;         // Header type
    uint8_t bist;               // BIST
    uint32_t BAR0;              // First base address register (index 0)
} devbus_PCIeDevice_t;

// * Variables

bool devbus_InitLock = false;       // Initialize lock for prevent re-initializing device bus

void* devbus_PCIeBase = NULL;       // PCIe base address (NULL at startup, after set NULL if not supported)

// * Functions

/**
 * @brief Function for read register(s) from specific device
 * 
 * @param bus Bus number
 * @param slot Slot/Device number
 * @param func Function number
 * @param reg Register index
 * 
 * @return Register value
 */
uint32_t devbus_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg) {
    if (!devbus_InitLock) { return -1; }    // Return if device bus not initialized
    if (devbus_PCIeBase != NULL) {  // If PCIe supported, use memory-mapped I/O
        uint32_t* regs = (uint32_t*)((size_t)devbus_PCIeBase + ((bus) << 20 | slot << 15 | func << 12)); return regs[reg];
    } else {                        // Else use PCI port I/O
        uint32_t addr = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | ((reg * 4) & 0xFC) | (uint32_t)0x80000000);
        port_outl(DEVBUS_PCI_INDEXPORT, addr); return port_inl(DEVBUS_PCI_DATAPORT);
    }
}

/**
 * @brief Function for write register(s) to specific device
 * 
 * @param bus Bus number
 * @param slot Slot/Device number
 * @param func Function number
 * @param reg Register index
 * @param value Register value to write
 */
void devbus_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg, uint32_t value) {
    if (!devbus_InitLock) { return; }   // Return if device bus not initialized
    if (devbus_PCIeBase != NULL) {  // If PCIe supported, use memory-mapped I/O
        uint32_t* regs = (uint32_t*)((size_t)devbus_PCIeBase + ((bus) << 20 | slot << 15 | func << 12)); regs[reg] = value;
    } else {                        // Else use PCI port I/O
        uint32_t addr = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | ((reg * 4) & 0xFC) | (uint32_t)0x80000000);
        port_outl(DEVBUS_PCI_INDEXPORT, addr); port_outl(DEVBUS_PCI_DATAPORT, value);
    }
}

/**
 * @brief Function for get base address register from specific device
 * 
 * @param bus Bus number
 * @param slot Slot/Device number
 * @param func Function number
 * @param num Base address register number
 */
void devbus_getBAR(devbus_BAR_t* bar, uint8_t bus, uint8_t slot, uint8_t func, uint16_t num) {
    // No not run this function if device bus not initialized, BAR structure NULL or BAR number is invalid
    if (!devbus_InitLock || bar == NULL || num >= 6) { return; }
    // Get header type register from device
    uint8_t htype = ((devbus_read(bus, slot, func, 3) >> 16) & 0xFF) & 0x7F;
    // Return if requested base address register not available on device
    if (num >= 6 - (4 * htype)) { return; }
    // Reset all values
    bar->addr = 0; bar->prefetch = 0; bar->type = 0;
    // Get base address register
    uint32_t base = devbus_read(bus, slot, func, 0x4 + num);
    // Get base address I/O type
    bar->type = (base & 0x1) ? DEVBUS_BARTYPE_IO : DEVBUS_BARTYPE_MM;
    if (bar->type == DEVBUS_BARTYPE_MM) {       // If base address is memory-mapped I/O
        switch ((base >> 1) & 3) {
            // 32-bit mode
            case 0: { bar->addr = (size_t)devbus_read(bus, slot, func, 4 + num); break; }   // Write address to structure
            // 20-bit mode
            case 1: { bar->addr = (size_t)devbus_read(bus, slot, func, 4 + num); break; }   // Write address to structure
            // 64-bit mode
            case 2: {
                // Return if register number is 5 (because 64-bit address covers 2 base address registers)
                if (num >= 5) { return; } bar->addr = (size_t) (                // Combine two BARs into 64-bit address
                    ((uint64_t)devbus_read(bus, slot, func, 4+num+1) << 32) |   // Take the first BAR and shift 32 times
                    devbus_read(bus, slot, func, 4+num)                         // Take the second BAR and combine
                ); break;
            }
        } bar->addr &= (size_t)~0xF;
    } else {                        // If base address is port I/O
        bar->addr = (base & ~3);    // Write address to structure without first 2 bits
        bar->prefetch = false;      // Mark prefetchable as false
    }
}

/**
 * @brief Function for get bulk information about specific device
 * 
 * @param dev Device structure address to write
 * @param bus Bus number
 * @param slot Slot/Device number
 * @param func Function number
 */
void devbus_get(devbus_Device_t* dev, uint8_t bus, uint8_t slot, uint8_t func) {
    // No not run this function if device bus not initialized or BAR structure NULL
    if (!devbus_InitLock || dev == NULL) { return; }
    uint32_t reg0 = devbus_read(bus, slot, func, 0);        // Get first register
    uint32_t reg2 = devbus_read(bus, slot, func, 2);        // Get third register
    dev->bus = bus; dev->slot = slot; dev->func = func;     // Set device location to structure
    dev->vendor = (uint16_t)((reg0 >> 0) & 0xFFFF);         // Set vendor ID
    dev->device = (uint16_t)((reg0 >> 16) & 0xFFFF);        // Set device ID (vendor-specific model)
    dev->revision = (uint8_t)((reg2 >> 0) & 0xFF);          // Set revision
    dev->interface = (uint8_t)((reg2 >> 8) & 0xFF);         // Set interface
    dev->subclass = (uint8_t)((reg2 >> 16) & 0xFF);         // Set subclass
    dev->class = (uint8_t)((reg2 >> 24) & 0xFF);            // Set class
}

/**
 * @brief Function for initialize device bus system
 */
void devbus_init() {
    if (devbus_InitLock) { return; } devbus_InitLock = true;    // Prevent re-initializing and lock the initializer
    if (acpiTable.mcfg != NULL) {                                           // Use express if MCFG table not NULL
        devbus_PCIeBase = (void*)(size_t)acpiTable.mcfg->pciebase;              // Get PCIe base
        devbus_PCIeDevice_t* root = (devbus_PCIeDevice_t*)devbus_PCIeBase;      // Get root bridge
        // Do panic if root bridge not found or misconfigured
        if (root->vendor == (uint16_t)-1) { PANIC("PCIe root bridge not found or misconfigured"); }
    } else if (devbus_read(0, 0, 0, 0) != (uint32_t)-1) { devbus_PCIeBase = NULL; } // Use legacy if host bridge available
    else { WARN("No PCI or PCI Express buses found"); devbus_InitLock = false; }    // Else no PCI or PCIe buses found
    devbus_Device_t dev;                                                // Scan devices and load drivers
    for (int b = 0; b < DEVBUS_MAX_BUSES; ++b) {                            // Scan buses
        for (int s = 0; s < DEVBUS_MAX_SLOTS; ++s) {                        // Scan slots/devices
            for (int f = 0; f < DEVBUS_MAX_FUNCS; ++f) {                    // Scan functions
                devbus_get(&dev, b, s, f); if (dev.vendor != 0xFFFF) {      // Get and check device information
                    drivers_load(&dev);                                     // Load driver for this device if available
                }
            }
        }
    }
}