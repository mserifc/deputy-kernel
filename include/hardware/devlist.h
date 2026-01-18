char* device_ClassList[] = {
    "0:Unclassified",
    "1:Mass Storage Controller",
    "2:Network Controller",
    "3:Display Controller",
    "4:Multimedia Controller",
    "5:Memory Controller",
    "6:Bridge",
    "7:Simple Communication Controller",
    "8:Base System Peripheral",
    "9:Input Device Controller",
    "10:Docking Station",
    "11:Processor",
    "12:Serial Bus Controller",
    "13:Wireless Controller",
    "14:Intelligent Controller",
    "15:Satellite Communication Controller",
    "16:Encryption Controller",
    "17:Signal Processing Controller",
    "18:Processing Accelerator",
    "19:Non-Essential Instrumentation",
    "64:Co-processor",
    "255:Unassigned Class"
};

char* device_SubclassList[] = {
    "0:0:Non-VGA-Compatible Unclassified Device",
    "0:1:VGA-Compatible Unclassified Device",
    "1:0:SCSI Bus Controller",
    "1:1:IDE Controller",
    "1:2:Floppy Disk Controller",
    "1:3:IPI Bus Controller",
    "1:4:RAID Controller",
    "1:5:ATA Controller",
    "1:6:Serial ATA Controller",
    "1:7:Serial Attached SCSI Controller",
    "1:8:Non-Volatile Memory Controller",
    "2:0:Ethernet Controller",
    "2:1:Token Ring Controller",
    "2:2:FDDI Controller",
    "2:3:ATM Controller",
    "2:4:ISDN Controller",
    "2:5:WorldFip Controller",
    "2:6:PICMG 2.14 Multi Computing Controller",
    "2:7:Infiniband Controller",
    "2:8:Fabric Controller",
    "3:0:VGA Compatible Controller",
    "3:1:XGA Controller",
    "3:2:3D Controller",
    "4:0:Multimedia Video Controller",
    "4:1:Multimedia Audio Controller",
    "4:2:Computer Telephony Device",
    "4:3:Audio Device",
    "5:0:RAM Controller",
    "5:1:Flash Controller",
    "6:0:Host Bridge",
    "6:1:ISA Bridge",
    "6:2:EISA Bridge",
    "6:3:MCA Bridge",
    "6:4:PCI-to-PCI Bridge",
    "6:5:PCMCIA Bridge",
    "6:6:NuBus Bridge",
    "6:7:CardBus Bridge",
    "6:8:RACEway Bridge",
    "6:9:PCI-to-PCI Bridge",
    "6:10:InfiniBand-to-PCI Host Bridge",
    "7:0:Serial Controller",
    "7:1:Parallel Controller",
    "7:2:Multiport Serial Controller",
    "7:3:Modem",
    "7:4:IEEE 488.1/2 (GPIB) Controller",
    "7:5:Smart Card Controller",
    "8:0:PIC",
    "8:1:DMA Controller",
    "8:2:Timer",
    "8:3:RTC Controller",
    "8:4:PCI Hot-Plug Controller",
    "8:5:SD Host controller",
    "8:6:IOMMU",
    "9:0:Keyboard Controller",
    "9:1:Digitizer Pen",
    "9:2:Mouse Controller",
    "9:3:Scanner Controller",
    "9:4:Gameport Controller",
    "10:0:Generic",
    "11:0:386",
    "11:1:486",
    "11:2:Pentium",
    "11:3:Pentium Pro",
    "11:16:Alpha",
    "11:32:PowerPC",
    "11:48:MIPS",
    "11:64:Co-Processor",
    "12:0:FireWire (IEEE 1394) Controller",
    "12:1:ACCESS Bus Controller",
    "12:2:SSA",
    "12:3:USB Controller",
    "12:4:Fibre Channel",
    "12:5:SMBus Controller",
    "12:6:InfiniBand Controller",
    "12:7:IPMI Interface",
    "12:8:SERCOS Interface (IEC 61491)",
    "12:9:CANbus Controller",
    "13:0:iRDA Compatible Controller",
    "13:1:Consumer IR Controller",
    "13:16:RF Controller",
    "13:17:Bluetooth Controller",
    "13:18:Broadband Controller",
    "13:32:Ethernet Controller (802.1a)",
    "13:33:Ethernet Controller (802.1b)",
    "14:0:I20",
    "15:1:Satellite TV Controller",
    "15:2:Satellite Audio Controller",
    "15:3:Satellite Voice Controller",
    "15:4:Satellite Data Controller",
    "16:0:Network and Computing Encrpytion/Decryption",
    "16:16:Entertainment Encryption/Decryption",
    "17:0:DPIO Modules",
    "17:1:Performance Counters",
    "17:16:Communication Synchronizer",
    "17:32:Signal Processing Management"
};

char* device_InterfaceList[] = {
    "1:1:0:ISA Compatibility mode-only controller",
    "1:1:5:PCI native mode-only controller",
    "1:1:10:ISA Compatibility mode controller, supports both channels switched to PCI native mode",
    "1:1:15:PCI native mode controller, supports both channels switched to ISA compatibility mode",
    "1:1:128:ISA Compatibility mode-only controller, supports bus mastering",
    "1:1:133:PCI native mode-only controller, supports bus mastering",
    "1:1:138:ISA Compatibility mode controller, supports both channels switched to PCI native mode, supports bus mastering",
    "1:1:143:PCI native mode controller, supports both channels switched to ISA compatibility mode, supports bus mastering",
    //
    "1:5:32:Single DMA",
    "1:5:48:Chained DMA",
    //
    "1:6:0:Vendor Specific Interface",
    "1:6:1:AHCI 1.0",
    "1:6:2:Serial Storage Bus",
    //
    "1:7:0:SAS",
    "1:7:1:Serial Storage Bus",
    //
    "1:8:1:NVMHCI",
    "1:8:2:NVM Express",
    //
    "3:0:0:VGA Controller",
    "3:0:1:8514-Compatible Controller",
    //
    "6:4:0:Normal Decode",
    "6:4:1:Subtractive Decode",
    //
    "6:8:0:Transparent Mode",
    "6:8:1:Endpoint Mode",
    //
    "6:9:64:Semi-Transparent, Primary bus towards host CPU",
    "6:9:128:Semi-Transparent, Secondary bus towards host CPU",
    //
    "7:0:0:8250-Compatible (Generic XT)",
    "7:0:1:16450-Compatible",
    "7:0:2:16550-Compatible",
    "7:0:3:16650-Compatible",
    "7:0:4:16750-Compatible",
    "7:0:5:16850-Compatible",
    "7:0:6:16950-Compatible",
    //
    "7:1:0:Standard Parallel Port",
    "7:1:1:Bi-Directional Parallel Port",
    "7:1:2:ECP 1.X Compliant Parallel Port",
    "7:1:3:IEEE 1284 Controller",
    "7:1:254:IEEE 1284 Target Device",
    //
    "7:3:0:Generic Modem",
    "7:3:1:Hayes 16450-Compatible Interface",
    "7:3:2:Hayes 16550-Compatible Interface",
    "7:3:3:Hayes 16650-Compatible Interface",
    "7:3:4:Hayes 16750-Compatible Interface",
    //
    "8:0:0:Generic 8259-Compatible",
    "8:0:1:ISA-Compatible",
    "8:0:2:EISA-Compatible",
    "8:0:16:I/O APIC Interrupt Controller",
    "8:0:32:I/O(x) APIC Interrupt Controller",
    //
    "8:1:0:Generic 8237-Compatible",
    "8:1:1:ISA-Compatible",
    "8:1:2:EISA-Compatible",
    //
    "8:2:0:Generic 8254-Compatible",
    "8:2:1:ISA-Compatible",
    "8:2:2:EISA-Compatible",
    "8:2:3:HPET",
    //
    "8:3:0:Generic RTC",
    "8:3:1:ISA-Compatible",
    //
    "9:4:0:Generic",
    "9:4:16:Extended",
    //
    "12:0:0:Generic",
    "12:0:16:OHCI",
    //
    "12:3:0:UHCI Controller",
    "12:3:16:OHCI Controller",
    "12:3:32:EHCI (USB2) Controller",
    "12:3:48:XHCI (USB3) Controller",
    "12:3:128:Unspecified",
    "12:3:254:USB Device (Not a host controller)",
    //
    "12:7:0:SMIC",
    "12:7:1:Keyboard Controller Style",
    "12:7:2:Block Transfer"
};

typedef struct {
    char* class;
    char* subclass;
    char* interface;
} device_Type_t;

device_Type_t devicetypebuff;

#include "utils.h"

device_Type_t* device_type(device_Info_t* info) {
    if (info->vendor == 0xFFFF) { return NULL; }
    devicetypebuff.class = NULL;
    devicetypebuff.subclass = NULL;
    devicetypebuff.interface = NULL;
    for (int c = 0; c < 22; ++c) {
        if (convert_atoi(split(device_ClassList[c], ':')->v[0]) == info->class) {
            devicetypebuff.class = device_ClassList[c];
            for (int s = 0; s < 95; ++s) {
                if (convert_atoi(split(device_SubclassList[s], ':')->v[0]) == info->class &&
                    convert_atoi(split(device_SubclassList[s], ':')->v[1]) == info->subclass) {
                        devicetypebuff.subclass = device_SubclassList[s];
                        for (int i = 0; i < 69; ++i) {
                            if (convert_atoi(split(device_InterfaceList[i], ':')->v[0]) == info->class &&
                                convert_atoi(split(device_InterfaceList[i], ':')->v[1]) == info->subclass &&
                                convert_atoi(split(device_InterfaceList[i], ':')->v[2]) == info->interface) {
                                devicetypebuff.interface = device_InterfaceList[i];
                            }
                        }
                }
            }
        }
    } return &devicetypebuff;
}