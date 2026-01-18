#pragma once

#include "types.h"
#include "utils.h"
#include "hardware/port.h"
#include "hardware/device.h"

#define USB_CMD 0x00
#define USB_STS 0x02
#define USB_INTR 0x04
#define USB_FRNUM 0x06
#define USB_FRBASE 0x08
#define USB_SOFMOD 0x0C
#define USB_PORTSC1 0x10
#define USB_PORTSC2 0x12

#define USB_CMD_RUN (1 << 0)
#define USB_CMD_HOSTRESET (1 << 1)
#define USB_CMD_GRESET (1 << 2)
#define USB_CMD_GSUSPEND (1 << 3)
#define USB_CMD_GRESUME (1 << 4)
#define USB_CMD_SWDEBUG (1 << 5)
#define USB_CMD_CONFIGFLAG (1 << 6)
#define USB_CMD_PACKSIZE (1 << 7)

#define USB_STS_INT (1 << 0)
#define USB_STS_ERRINT (1 << 1)
#define USB_STS_RESUME (1 << 2)
#define USB_STS_SYSERR (1 << 3)
#define USB_STS_PROCERR (1 << 4)
#define USB_STS_HALTED (1 << 5)

#define USB_INTR_TIMEOUT (1 << 0)
#define USB_INTR_RESUME (1 << 1)
#define USB_INTR_COMPLETE (1 << 2)
#define USB_INTR_SHORTPACK (1 << 3)

#define USB_PORTSC_CONSTS (1 << 0)
#define USB_PORTSC_CONSTSC (1 << 1)
#define USB_PORTSC_ENABLE (1 << 2)
#define USB_PORTSC_ENABLEC (1 << 3)
#define USB_PORTSC_LINESTS (3 << 4)
#define USB_PORTSC_RESUME (1 << 6)
#define USB_PORTSC_ISVALID (1 << 7)
#define USB_PORTSC_SPEED (1 << 8)
#define USB_PORTSC_RESET (1 << 9)
#define USB_PORTSC_SUSPEND (1 << 12)

#define USB_FRENT_EMPTY (1 << 0)
#define USB_FRENT_QUEUE (1 << 1)
#define USB_FRENT_DEPTH (1 << 2)
#define USB_FRENT_ADDR 0xFFFFFFF0
#define USB_FRENT_TD 0
#define USB_FRENT_QH 1

#define USB_TDSTS_ACTUALLEN (0x7FF << 0)
#define USB_TDSTS_STUFFERR (1 << 17)
#define USB_TDSTS_TIMEOUT (1 << 18)
#define USB_TDSTS_NONACK (1 << 19)
#define USB_TDSTS_BABBLE (1 << 20)
#define USB_TDSTS_BUFFERR (1 << 21)
#define USB_TDSTS_STALLED (1 << 22)
#define USB_TDSTS_ACTIVE (1 << 23)
#define USB_TDSTS_COMPINT (1 << 24)
#define USB_TDSTS_ISOCHRON (1 << 25)
#define USB_TDSTS_LOWSPEED (1 << 26)
#define USB_TDSTS_ERRCOUNT (3 << 27)
#define USB_TDSTS_SHORTPACK (1 << 29)

#define USB_TDPHEAD_PACKTYPE (0xFF << 0)
#define USB_TDPHEAD_DEVICE (0x7F << 8)
#define USB_TDPHEAD_ENDPOINT (0xF << 15)
#define USB_TDPHEAD_DATATOGGLE (1 << 19)
#define USB_TDPHEAD_MAXLEN (0x7FF << 21)

#define USB_TDHEAD_PACKTYPE_IN 0x69
#define USB_TDHEAD_PACKTYPE_OUT 0xE1
#define USB_TDHEAD_PACKTYPE_SETUP 0x2D

#define USB_FRENT_COUNT 1024
#define USB_FRENT_SIZE 4

typedef uint32_t usb_FrameEntry_t;

typedef struct {
    uint32_t horizontal;
    uint32_t vertical;
} usb_QueueHead_t;

typedef struct {
    uint32_t next;
    uint32_t status;
    uint32_t packhead;
    uint32_t buffer;
    uint8_t sysuse[16];
} usb_TransferDesc_t;

typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} usb_SetupPack_t;

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} PACKED usb_DeviceDesc_t;

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} PACKED usb_ConfigDesc_t;

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} PACKED usb_InterfaceDesc_t;

int usb_init(device_t* dev);
void usb_process();