#pragma once

#include "types.h"

// * Types and structures

// ACPI System Description Table (SDT) Header
typedef struct {
    char Signature[4];              // Table signature
    uint32_t Length;                // Length of the table
    uint8_t Revision;               // Revision number
    uint8_t Checksum;               // Checksum of the table
    char OEMID[6];                  // OEM identifier
    char OEMTableID[8];             // OEM table identifier
    uint32_t OEMRevision;           // OEM revision number
    uint32_t CreatorID;             // Creator ID
    uint32_t CreatorRevision;       // Creator revision number
} acpi_SDTHeader_t;

// ACPI Generic Address Structure (used for registers)
typedef struct {
    uint8_t AddressSpace;           // Address space type
    uint8_t BitWidth;               // Bit width of the register
    uint8_t BitOffset;              // Bit offset for the register
    uint8_t AccessSize;             // Access size
    uint64_t Address;               // Address of the register
} acpi_GenericAddressStructure;

// ACPI Fixed ACPI Description Table (FADT)
typedef struct {
    acpi_SDTHeader_t h;                                     // Header for the FADT
    uint32_t FirmwareCtrl;                                  // Firmware control address
    uint32_t Dsdt;                                          // Address of the DSDT table

    uint8_t  Reserved;                                      // Reserved for ACPI 1.0 compatibility

    uint8_t  PreferredPowerManagementProfile;               // Power management profile
    uint16_t SCI_Interrupt;                                 // System control interrupt
    uint32_t SMI_CommandPort;                               // SMI command port address
    uint8_t  AcpiEnable;                                    // ACPI enable flag
    uint8_t  AcpiDisable;                                   // ACPI disable flag
    uint8_t  S4BIOS_REQ;                                    // S4 BIOS request
    uint8_t  PSTATE_Control;                                // P-state control

    // Power Management Event Blocks
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;

    uint32_t GPE0Block;                                     // General Purpose Event block 0
    uint32_t GPE1Block;                                     // General Purpose Event block 1
    uint8_t  PM1EventLength;                                // PM1 event length
    uint8_t  PM1ControlLength;                              // PM1 control length
    uint8_t  PM2ControlLength;                              // PM2 control length
    uint8_t  PMTimerLength;                                 // PM timer length
    uint8_t  GPE0Length;                                    // GPE0 length
    uint8_t  GPE1Length;                                    // GPE1 length
    uint8_t  GPE1Base;                                      // GPE1 base address

    uint8_t  CStateControl;                                 // Control for C-state
    uint16_t WorstC2Latency;                                // Worst C2 latency
    uint16_t WorstC3Latency;                                // Worst C3 latency
    uint16_t FlushSize;                                     // Flush size
    uint16_t FlushStride;                                   // Flush stride
    uint8_t  DutyOffset;                                    // Duty cycle offset
    uint8_t  DutyWidth;                                     // Duty cycle width
    uint8_t  DayAlarm;                                      // Day alarm
    uint8_t  MonthAlarm;                                    // Month alarm
    uint8_t  Century;                                       // Century for RTC alarms

    uint16_t BootArchitectureFlags;                         // Boot architecture flags (ACPI 2.0+)

    uint8_t  Reserved2;                                     // Reserved for future use
    uint32_t Flags;                                         // Flags for various settings

    // Reset register for system reset
    acpi_GenericAddressStructure ResetReg;

    uint8_t  ResetValue;                                    // Reset value
    uint8_t  Reserved3[3];                                  // Reserved for future use
  
    // 64-bit pointers - Available on ACPI 2.0+
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;

    // 64-bit pointers for event blocks
    acpi_GenericAddressStructure X_PM1aEventBlock;
    acpi_GenericAddressStructure X_PM1bEventBlock;
    acpi_GenericAddressStructure X_PM1aControlBlock;
    acpi_GenericAddressStructure X_PM1bControlBlock;
    acpi_GenericAddressStructure X_PM2ControlBlock;
    acpi_GenericAddressStructure X_PMTimerBlock;
    acpi_GenericAddressStructure X_GPE0Block;
    acpi_GenericAddressStructure X_GPE1Block;
} acpi_FADT_t;

// ACPI Memory-mapped Configuration Space (MCFG) Table
typedef struct {
    acpi_SDTHeader_t h;     // Header for the MCFG
    uint64_t Reserved;      // Reserved for future use
    uint64_t pciebase;      // PCIe configuration base address
} acpi_MCFG_t;

// ACPI table structure for perform inspections by other kernel components
typedef struct {
    char OEMID[6];          // OEM ID
    uint8_t Revision;       // Revision number
    bool support;           // ACPI support of the machine
    acpi_FADT_t* fadt;      // Pointer to the FADT table
    acpi_MCFG_t* mcfg;      // Pointer to the MCFG table
} acpiTable_t;

// * Imports

extern acpiTable_t acpiTable;   // Import ACPI table automatically

// * Functions

void acpi_init(void);           // Initialize ACPI table