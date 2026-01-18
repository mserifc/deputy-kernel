#include "drivers/usb.h"

bool usb_Initialized = false;

BAR_t usb_BAR;
device_t usb_Device;
usb_FrameEntry_t* usb_FrameList;
bool usb_PortValid[2];

int usb_init(device_t* dev) {
    if (usb_Initialized || dev == NULL || dev->port == 0) { return -1; }
    usb_FrameList = (usb_FrameEntry_t*)(malloc((USB_FRENT_COUNT * USB_FRENT_SIZE) + (4 * 1024)) + (4 * 1024));
    if (usb_FrameList == NULL) { return -1; }
    ncopy(&usb_Device, dev, sizeof(device_t));
    port_outw(dev->port + 0xC0, 0x2000);
    uint32_t pcicmd; uint16_t usbcmd;
    pcicmd = device_read(dev->bus, dev->slot, dev->func, DEVICE_REG_COMMAND_STATUS) | DEVICE_CMDREG_BUSMASTER;
    device_write(dev->bus, dev->slot, dev->func, DEVICE_REG_COMMAND_STATUS, pcicmd);
    usbcmd = port_inw(dev->port + USB_CMD); port_outw(dev->port + USB_CMD, usbcmd | USB_CMD_HOSTRESET);
    usbcmd = port_inw(dev->port + USB_CMD); port_outw(dev->port + USB_CMD, usbcmd | USB_CMD_GRESET); delay(10);
    usbcmd = port_inw(dev->port + USB_CMD); port_outw(dev->port + USB_CMD, usbcmd & !USB_CMD_GRESET);
    usb_FrameList = (usb_FrameEntry_t*)((size_t)usb_FrameList & 0xFFFFF000);
    for (int i = 0; i < USB_FRENT_COUNT; ++i) { usb_FrameList[i] = USB_FRENT_EMPTY; }
    port_outl(dev->port + USB_FRBASE, (size_t)usb_FrameList);
    uint16_t portsc = port_inw(dev->port + USB_PORTSC1);
    usb_PortValid[0] = (portsc & USB_PORTSC_ISVALID && portsc != 0xFFFF) ? true : false;
    portsc = port_inw(dev->port + USB_PORTSC2);
    usb_PortValid[1] = (portsc & USB_PORTSC_ISVALID && portsc != 0xFFFF) ? true : false;
    usbcmd = port_inw(dev->port + USB_CMD); port_outw(dev->port + USB_CMD, usbcmd | USB_CMD_PACKSIZE | USB_CMD_RUN);
    usb_Initialized = true; return 0;
}

int usb_deviceEnable(uint16_t port) {
    if (!usb_Initialized || port == 0) { return -1; }
    if (!(port_inw(port) & USB_PORTSC_ISVALID)) { return -1; }
    port_outw(port, port_inw(port) | USB_PORTSC_RESET); delay(100);
    port_outw(port, port_inw(port) & (uint16_t)~USB_PORTSC_RESET); delay(50);
    port_outw(port, port_inw(port) | USB_PORTSC_ENABLE); delay(100);
    if (!(port_inw(port) & USB_PORTSC_ENABLE)) { return 1; } return 0;
}

int usb_deviceDisable(uint16_t port) {
    if (!usb_Initialized || port == 0) { return -1; }
    if (!(port_inw(port) & USB_PORTSC_ISVALID)) { return -1; }
    if (port_inw(port) & USB_PORTSC_ENABLE) { return -1; }
    port_outw(port, port_inw(port) & (uint16_t)~USB_PORTSC_ENABLE);
    if (port_inw(port) & USB_PORTSC_ENABLE) { return 1; } return 0;
}

// void* usb_linkDesc(int type, void* link) {
//     if (!usb_Initialized) { return NULL; }
//     void* linkaddr = NULL;
//     if (link == NULL) {
//         for (int i = 0; i < USB_FRENT_COUNT; ++i) {
//             if ()
//         }
//     }
//     if (type == USB_FRENT_TD) {
//         //
//     } else if (type == USB_FRENT_QH) {
//         //
//     } else { return NULL; }
// }

// int usb_unlinkDesc(void* desc, void* link) {
//     //
// }

// void* usb_createDesc(void* base, int type) {
//     if (!usb_Initialized || base == NULL) { return NULL; }
//     if (type == USB_FRENT_TD) {
//         usb_TransferDesc_t* td = (usb_TransferDesc_t*)base;
//         td->next = USB_FRENT_EMPTY;
//     } else if (type == USB_FRENT_QH) {
//         usb_QueueHead_t* qh = (usb_QueueHead_t*)base;
//         qh->horizontal = USB_FRENT_EMPTY;
//         qh->vertical = USB_FRENT_EMPTY;
//         return qh;
//     } else { return NULL; }
// }


// usb_QueueHead_t* queue;
// usb_TransferDesc_t* setup;

usb_TransferDesc_t* mouse;
bool first = true;
bool end = false;
void* buffer = NULL;

void usb_mouse() {
    // queue = (usb_QueueHead_t*)malloc(sizeof(usb_QueueHead_t) + 0x10) + 0x10;
    // queue = (usb_QueueHead_t*)((size_t)queue & 0xFFFFFFF0);
    // setup = (usb_TransferDesc_t*)malloc(sizeof(usb_TransferDesc_t) + 0x10) + 0x10;
    // setup = (usb_TransferDesc_t*)((size_t)setup & 0xFFFFFFF0);
    // usb_FrameList[0] = (size_t)&queue | USB_FRENT_QUEUE;
    // queue->horizontal = USB_FRENT_EMPTY;
    // queue->vertical = (size_t)&setup | USB_FRENT_DEPTH;
    // setup->next = USB_FRENT_EMPTY;
    // setup->status = 0;
    // setup->packhead = 0xE00000 | USB_TDPHEAD_DATATOGGLE | USB_TDHEAD_PACKTYPE_SETUP;
    // void* buffer = malloc(MEMORY_BLOCKSIZE);
    // fill(buffer, 0, MEMORY_BLOCKSIZE);
    // setup->buffer = (size_t)buffer;
    // delay(1024);
    // uint32_t* outbuff = (uint32_t*)buffer;
    // printf("%d\n", outbuff[0]);
    // printf("%d\n", outbuff[1]);
    // printf("%d\n", outbuff[2]);
    // printf("%d\n", outbuff[3]);
    if (first) {
        first = false;
    } else if (!end) {
        mouse = (usb_TransferDesc_t*)malloc(sizeof(usb_TransferDesc_t) + 0x10) + 0x10;
        if (mouse == NULL) { PANIC("Mouse err 1"); }
        mouse = (usb_TransferDesc_t*)((size_t)mouse & 0xFFFFFFF0);
        mouse->next = 0x1;
        mouse->status = (3 << 27) | (1 << 23);
        mouse->packhead = (7 << 21) | (0 << 19) | (1 << 15) | (0 << 8) | 0x69;
        void* buffer = malloc(MEMORY_BLOCKSIZE);
        if (buffer == NULL) { PANIC("Mouse err 2"); }
        mouse->buffer = (size_t)buffer;
        uint32_t* num = (uint32_t*)mouse->buffer;
        num[0] = 0xC0FFEE;
        printf("0x%x: 0x%x, 0x%x\n", (size_t)mouse->buffer, num[0], mouse->status);
        for (int i = 0; i < 1024; i += 8) { usb_FrameList[i] = (size_t)mouse; }
        end = true;
    } else {
        int curx = 0;
        int cury = 0;
        while(1) {
            if (!(mouse->status & USB_TDSTS_NONACK)) {
                uint8_t* inp = (uint8_t*)mouse->buffer;
                int x = (char)inp[1] / ((DISPLAY_CLIWIDTH + DISPLAY_CLIHEIGHT) / 8);
                int y = (char)inp[2] / ((DISPLAY_CLIWIDTH + DISPLAY_CLIHEIGHT) / 8);
                curx += x;
                cury += y;
                if (curx < 0) { curx = 0; }
                if (curx >= DISPLAY_CLIWIDTH - 1) { curx = DISPLAY_CLIWIDTH - 1; }
                if (cury < 0) { cury = 0; }
                if (cury >= DISPLAY_CLIHEIGHT - 1) { cury = DISPLAY_CLIHEIGHT - 1; }
                display_clear();
                display_putchar(219, (cury * DISPLAY_CLIWIDTH) + curx);
                printf("%d, %d\n", curx, cury);
                // uint32_t* num = (uint32_t*)mouse->buffer;
                // printf("0x%x: 0x%x, 0x%x\n", (size_t)mouse->buffer, num[0], mouse->status);
                mouse->status |= USB_TDSTS_ACTIVE;
            }
            yield();
        }
    }
}

// void usb_test() {
//     usb_QueueHead_t* queue = (usb_QueueHead_t*)malloc(sizeof(usb_QueueHead_t) + 0x10) + 0x10;
//     if (queue == NULL) { PANIC("Test err 1"); }
//     queue = (usb_QueueHead_t*)((size_t)queue & 0xFFFFFFF0);
//     usb_TransferDesc_t* setup = (usb_TransferDesc_t*)malloc(sizeof(usb_TransferDesc_t) + 0x10) + 0x10;
//     if (setup == NULL) { PANIC("Test err 2"); }
//     setup = (usb_TransferDesc_t*)((size_t)setup & 0xFFFFFFF0);
//     usb_TransferDesc_t* data = (usb_TransferDesc_t*)malloc(sizeof(usb_TransferDesc_t) + 0x10) + 0x10;
//     if (data == NULL) { PANIC("Test err 3"); }
//     data = (usb_TransferDesc_t*)((size_t)data & 0xFFFFFFF0);
//     usb_TransferDesc_t* end = (usb_TransferDesc_t*)malloc(sizeof(usb_TransferDesc_t) + 0x10) + 0x10;
//     if (end == NULL) { PANIC("Test err 4"); }
//     end = (usb_TransferDesc_t*)((size_t)end & 0xFFFFFFF0);
//     queue->horizontal = 0x1;
//     queue->vertical = (size_t)setup;
//     setup->next = (size_t)data;
//     setup->status = (1 << 23);
//     setup->packhead = 0x2D | (0 << 8) | (0 << 15) | (0 << 19) | (7 << 21);
//     setup->buffer = (size_t)malloc(MEMORY_BLOCKSIZE);
//     usb_SetupPack_t* sbuff = (usb_SetupPack_t*)setup->buffer;
//     // sbuff[0] = 0b10000000 | (6 << 8) | (1 << 16) | (0 << 24);
//     // sbuff[1] = 0 | (18 << 16);
//     // sbuff[0] = 0b10000000;
//     // sbuff[1] = 6;
//     // sbuff[2] = 1;
//     // sbuff[3] = 0;
//     // sbuff[4] = 0;
//     // sbuff[5] = 0;
//     // sbuff[6] = 0;
//     // sbuff[7] = 18;
//     sbuff->bmRequestType = 0b10000000;
//     sbuff->bRequest = 6;
//     sbuff->wValue = (1 << 8) | 0;
//     sbuff->wIndex = 0;
//     data->next = (size_t)end;
//     data->status = (1 << 23);
//     data->packhead = 0x69 | (0 << 8) | (0 << 15) | (1 << 19) | (17 << 21);
//     data->buffer = (size_t)malloc(MEMORY_BLOCKSIZE);
//     uint32_t* dbuff = (uint32_t*)data->buffer;
//     dbuff[0] = 0xC0FFEE;
//     end->next = 0x1;
//     end->status = (1 << 23);
//     end->packhead = 0xE1 | (0 << 8) | (0 << 15) | (1 << 19) | (0 << 21);
//     end->buffer = 0;
//     usb_FrameList[0] = (size_t)queue;
//     delay(1024);
//     printf("0x%x, 0x%x, 0x%x, 0x%x\n", setup->status, data->status, end->status, dbuff[0]);
// }

void usb_test() {
    // usb_QueueHead_t* queue = (usb_QueueHead_t*)malloc(sizeof(usb_QueueHead_t) + 0x10) + 0x10;
    // if (queue == NULL) { PANIC("Test err 1"); }
    // queue = (usb_QueueHead_t*)((size_t)queue & 0xFFFFFFF0);
    usb_TransferDesc_t* setup = (usb_TransferDesc_t*)malloc(sizeof(usb_TransferDesc_t) + 0x10) + 0x10;
    if (setup == NULL) { PANIC("Test err 2"); }
    setup = (usb_TransferDesc_t*)((size_t)setup & 0xFFFFFFF0);
    usb_TransferDesc_t* data = (usb_TransferDesc_t*)malloc(sizeof(usb_TransferDesc_t) + 0x10) + 0x10;
    if (data == NULL) { PANIC("Test err 3"); }
    data = (usb_TransferDesc_t*)((size_t)data & 0xFFFFFFF0);
    usb_TransferDesc_t* status = (usb_TransferDesc_t*)malloc(sizeof(usb_TransferDesc_t) + 0x10) + 0x10;
    if (status == NULL) { PANIC("Test err 4"); }
    status = (usb_TransferDesc_t*)((size_t)status & 0xFFFFFFF0);
    // -----------------------------------------------------------------------------------------------
    // queue->horizontal = 0x1;
    // queue->vertical = (size_t)setup;
    // -----------------------------------------------------------------------------------------------
    setup->next = (size_t)data;
    setup->status = (1 << 23);
    setup->packhead = 0x2D | (0 << 8) | (0 << 15) | (0 << 19) | (7 << 21);
    setup->buffer = (size_t)malloc(MEMORY_BLOCKSIZE);
    usb_SetupPack_t* sbuff = (usb_SetupPack_t*)setup->buffer;
    sbuff->bmRequestType = 0b10000000;
    sbuff->bRequest = 6;
    sbuff->wValue = (2 << 8) | 0;
    sbuff->wIndex = 0;
    sbuff->wLength = 18;
    // -----------------------------------------------------------------------------------------------
    data->next = (size_t)status;
    data->status = (1 << 23);
    data->packhead = 0x69 | (0 << 8) | (0 << 15) | (1 << 19) | (17 << 21);
    data->buffer = (size_t)malloc(MEMORY_BLOCKSIZE);
    uint32_t* dbuff = (uint32_t*)data->buffer;
    dbuff[0] = 0xC0FFEE;
    usb_ConfigDesc_t* ddev = (usb_ConfigDesc_t*)data->buffer;
    usb_InterfaceDesc_t* dif = (usb_InterfaceDesc_t*)(data->buffer + sizeof(usb_ConfigDesc_t));
    // -----------------------------------------------------------------------------------------------
    status->next = 0x1;
    status->status = (1 << 23);
    status->packhead = 0xE1 | (0 << 8) | (0 << 15) | (1 << 19) | (0 << 21);
    status->buffer = 0;
    // -----------------------------------------------------------------------------------------------
    usb_FrameList[0] = (size_t)setup;
    delay(1024);
    printf("0x%x, 0x%x, 0x%x, 0x%x\n", setup->status, data->status, status->status, dbuff[0]);
    // printf("\tbLength: 0x%x,\n\
    //         bDescriptorType: 0x%x,\n\
    //         bcdUSB: 0x%x,\n\
    //         bDeviceClass: 0x%x,\n\
    //         bDeviceSubClass: 0x%x,\n\
    //         bDeviceProtocol: 0x%x,\n\
    //         bMaxPacketSize0: 0x%x,\n\
    //         idVendor: 0x%x,\n\
    //         idProduct: 0x%x,\n\
    //         bcdDevice: 0x%x,\n\
    //         iManufacturer: 0x%x,\n\
    //         iProduct: 0x%x,\n\
    //         iSerialNumber: 0x%x,\n\
    //         bNumConfigurations: 0x%x\n",
    //     ddev->bLength,
    //     ddev->bDescriptorType,
    //     ddev->bcdUSB,
    //     ddev->bDeviceClass,
    //     ddev->bDeviceSubClass,
    //     ddev->bDeviceProtocol,
    //     ddev->bMaxPacketSize0,
    //     ddev->idVendor,
    //     ddev->idProduct,
    //     ddev->bcdDevice,
    //     ddev->iManufacturer,
    //     ddev->iProduct,
    //     ddev->iSerialNumber,
    //     ddev->bNumConfigurations);
    // ------------------------------------
    printf(" \
        bLength: 0x%x\n\
        bDescriptorType: 0x%x\n\
        wTotalLength: 0x%x\n\
        bNumInterfaces: 0x%x\n\
        bConfigurationValue: 0x%x\n\
        iConfiguration: 0x%x\n\
        bmAttributes: 0x%x\n\
        bMaxPower: 0x%x\n",
        ddev->bLength,
        ddev->bDescriptorType,
        ddev->wTotalLength,
        ddev->bNumInterfaces,
        ddev->bConfigurationValue,
        ddev->iConfiguration,
        ddev->bmAttributes,
        ddev->bMaxPower);
    // ------------------------------------
    printf("\
        bLength: 0x%x,\n\
        bDescriptorType: 0x%x,\n\
        bInterfaceNumber: 0x%x,\n\
        bAlternateSetting: 0x%x,\n\
        bNumEndpoints: 0x%x,\n\
        bInterfaceClass: 0x%x,\n\
        bInterfaceSubClass: 0x%x,\n\
        bInterfaceProtocol: 0x%x,\n\
        iInterface: 0x%x,\n",
        dif->bLength,
        dif->bDescriptorType,
        dif->bInterfaceNumber,
        dif->bAlternateSetting,
        dif->bNumEndpoints,
        dif->bInterfaceClass,
        dif->bInterfaceSubClass,
        dif->bInterfaceProtocol,
        dif->iInterface);
    first = false;
}

void usb_process() {
    if (!usb_Initialized) { exit(); return; }
    while (true) {
        uint16_t portsc = port_inw(usb_Device.port + USB_STS);
        if (portsc & USB_STS_ERRINT) { printf("[usbd: Error interrupt detected]"); }
        if (portsc & USB_STS_SYSERR) { printf("[usbd: System error detected]"); }
        if (portsc & USB_STS_PROCERR) { printf("[usbd: Process error detected]"); }
        if (usb_PortValid[0]) {
            portsc = port_inw(usb_Device.port + USB_PORTSC1);
            if (portsc & USB_PORTSC_CONSTSC) {
                if (portsc & USB_PORTSC_CONSTS) {
                    switch (usb_deviceEnable(usb_Device.port + USB_PORTSC1)) {
                        case 0: { printf("[usbd: Port 1: Connected successfully]"); break; }
                        case -1: { printf("[usbd: Port 1: Unable to start device]"); break; }
                        case 1: { printf("[usbd: Port 1: Device cannot start]"); break; }
                        default: { printf("[usbd: Port 1: Unknown error]"); }
                    }
                } else { printf("[usbd: Port 1: Disconnected]"); }
                port_outw(usb_Device.port + USB_PORTSC1, port_inw(usb_Device.port + USB_PORTSC1) | USB_PORTSC_CONSTSC);
            }
        }
        if (usb_PortValid[1]) {
            portsc = port_inw(usb_Device.port + USB_PORTSC2);
            if (portsc & USB_PORTSC_CONSTSC) {
                if (portsc & USB_PORTSC_CONSTS) {
                    switch (usb_deviceEnable(usb_Device.port + USB_PORTSC2)) {
                        case 0: { printf("[usbd: Port 2: Connected successfully]"); break; }
                        case -1: { printf("[usbd: Port 2: Unable to start device]"); break; }
                        case 1: { printf("[usbd: Port 2: Device cannot start]"); break; }
                        default: { printf("[usbd: Port 2: Unknown error]"); }
                    }
                } else { printf("[usbd: Port 2: Disconnected]"); }
                port_outw(usb_Device.port + USB_PORTSC2, port_inw(usb_Device.port + USB_PORTSC2) | USB_PORTSC_CONSTSC);
            }
        } yield();
    }
}