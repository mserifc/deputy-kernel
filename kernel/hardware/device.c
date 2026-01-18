#include "hardware/device.h"

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
  
    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
  
    // Write out the address
    port_outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)((port_inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

uint16_t pciCheckVendor(uint8_t bus, uint8_t slot) {
    uint16_t vendor, device;
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */
    if ((vendor = pciConfigReadWord(bus, slot, 0, 0)) != 0xFFFF) {
       device = pciConfigReadWord(bus, slot, 0, 2);
    //    . . .
    } return (vendor);
}

uint32_t device_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg) {
    uint32_t addr = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | ((reg * 4) & 0xFC) | (uint32_t)0x80000000);
    port_outl(DEVICE_INDEXPORT, addr); return port_inl(DEVICE_DATAPORT);
}

void device_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg, uint32_t value) {
    uint32_t addr = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | ((reg * 4) & 0xFC) | (uint32_t)0x80000000);
    port_outl(DEVICE_INDEXPORT, addr); port_outl(DEVICE_DATAPORT, value);
}

void device_getBAR(BAR_t* bar, uint8_t bus, uint8_t slot, uint8_t func, uint16_t num) {
    if (bar == NULL) { return; }
    uint8_t htype = ((device_read(bus, slot, func, 3) >> 16) & 0xFF) & 0x7F;
    if (num >= 6 - (4 * htype)) { return; }
    bar->addr = 0; bar->prefetch = 0; bar->type = 0;
    uint32_t base = device_read(bus, slot, func, 0x4 + num);
    bar->type = (base & 0x1) ? DEVICE_BARTYPE_IO : DEVICE_BARTYPE_MM;
    if (bar->type == DEVICE_BARTYPE_MM) {
        switch ((base >> 1) & 3) {
            case 0: // 32-bit mode
            case 1: // 20-bit mode
            case 2: // 64-bit mode
                break;
        }
    } else {
        bar->addr = (uint8_t*)(base & ~3);
        bar->prefetch = false;
    }
}

void device_get(device_t* dev, uint8_t bus, uint8_t slot, uint8_t func) {
    if (dev == NULL) { return; }
    uint32_t reg0 = device_read(bus, slot, func, 0);
    uint32_t reg1 = device_read(bus, slot, func, 1);
    uint32_t reg2 = device_read(bus, slot, func, 2);
    dev->port = 0; BAR_t bar; for (int i = 0; i < 6; ++i) {
        device_getBAR(&bar, bus, slot, func, i);
        if (bar.addr && bar.type == DEVICE_BARTYPE_IO) { dev->port = (size_t)bar.addr; }
    }
    dev->bus = bus; dev->slot = slot; dev->func = func;
    dev->vendor = (uint16_t)((reg0 >> 0) & 0xFFFF);
    dev->device = (uint16_t)((reg0 >> 16) & 0xFFFF);
    dev->command = (uint16_t)((reg1 >> 0) & 0xFFFF);
    dev->status = (uint16_t)((reg1 >> 16) & 0xFFFF);
    dev->revision = (uint8_t)((reg2 >> 0) & 0xFF);
    dev->interface = (uint8_t)((reg2 >> 8) & 0xFF);
    dev->subclass = (uint8_t)((reg2 >> 16) & 0xFF);
    dev->class = (uint8_t)((reg2 >> 24) & 0xFF);
}