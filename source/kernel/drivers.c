#include "kernel.h"

#include "hw/devbus.h"

#include "drv/usb.h"

// * Types and structures

// Structure of device driver
typedef struct {
    uint16_t vendor;                        // Vendor
    uint16_t device;                        // Device (vendor-specific model)
    uint8_t class;                          // Class
    uint8_t subclass;                       // Subclass
    uint8_t interface;                      // Interface
    uint8_t revision;                       // Revision
    int (*init)(devbus_Device_t* dev);      // Initialize function
} drivers_List_t;

// * Variables and tables

// Driver list
static drivers_List_t drivers_List[] = {
    {   // USB Host Driver
        .vendor = -1,
        .device = -1,
        .class = 0x0C,
        .subclass = 0x03,
        .interface = -1,
        .revision = -1,
        .init = usb_init
    }
};

/**
 * @brief Function for load device driver
 * 
 * @param dev Device structure pointer
 */
void drivers_load(void* dev) {
    devbus_Device_t* info = (devbus_Device_t*)dev;
    for (size_t i = 0; i < sizeof(drivers_List) / sizeof(drivers_List_t); ++i) {
        if (drivers_List[i].vendor != (uint16_t)-1 && info->vendor != drivers_List[i].vendor) { continue; }
        if (drivers_List[i].device != (uint16_t)-1 && info->device != drivers_List[i].device) { continue; }
        if (drivers_List[i].class != (uint8_t)-1 && info->class != drivers_List[i].class) { continue; }
        if (drivers_List[i].subclass != (uint8_t)-1 && info->subclass != drivers_List[i].subclass) { continue; }
        if (drivers_List[i].interface != (uint8_t)-1 && info->interface != drivers_List[i].interface) { continue; }
        if (drivers_List[i].revision != (uint8_t)-1 && info->revision != drivers_List[i].revision) { continue; }
        if (drivers_List[i].init == NULL) { ERR("Driver %d initializer not found", i); continue; }
        if (drivers_List[i].init(info) != 0) {
            ERR("Device %d:%d:%d (driver %d) initialization failed",
                info->bus, info->slot, info->func, i); continue;
        } break;
    } return;
}