#include "hw/acpi.h"

#include "kernel.h"
#include "hw/port.h"

// * Types and structures

// ACPI Root System Description Pointer (RSDP) structure
typedef struct {
    char Signature[8];          // Signature ("RSD PTR ")
    uint8_t Checksum;           // Checksum for verify the RSDP structure
    char OEMID[6];              // OEM ID
    uint8_t Revision;           // Revision of the RSDP structure (usually 0 or 2)
    uint32_t RsdtAddress;       // Address of the Root System Description Table (RSDT)
} PACKED acpi_RSDP_t;

// ACPI Extended System Description Pointer (XSDP) structure
typedef struct {
    uint32_t Length;            // Length of the XSDP structure
    uint64_t XsdtAddress;       // Address of the Extended System Description Table (XSDT)
    uint8_t ExtendedChecksum;   // Checksum for verify the XSDP structure
    uint8_t reserved[3];        // Reserved for future use
} PACKED acpi_XSDP_t;

// * Variables

bool acpi_InitLock = false;     // Initialize lock for prevent re-initializing ACPI

acpiTable_t acpiTable;          // Public ACPI table for other kernel components

// * Functions

/**
 * @brief Function for initialize the ACPI table
 */
void acpi_init() {
    if (acpi_InitLock) { return; } acpi_InitLock = true;    // Prevent re-initializing and lock the initializer
    // Reset the table
    ncopy(acpiTable.OEMID, "UNCFG", 6); acpiTable.Revision = -1;
    acpiTable.support = false; acpiTable.fadt = NULL; acpiTable.mcfg = NULL;
    // Find RSDP/XSDP table
    for (size_t i = 0x80000; i < 0xFFFFF; ++i) {        // Start at EBDA to end of the lower memory
        if (ncompare((void*)i, "RSD PTR ", 8) == 0) {   // Find "RSD PTR " signature
            acpi_RSDP_t* rsdp = (acpi_RSDP_t*)i; {      // If found, check summary of the structure
                uint8_t* rsdpbyte = (uint8_t*)i; uint32_t sum = 0;
                for (size_t i = 0; i < sizeof(acpi_RSDP_t); ++i) { sum += rsdpbyte[i]; }
                if ((sum & 0xFF) != 0x00) { PANIC("RSDP table damaged"); }          // If wrong, panic then
            } acpiTable.Revision = rsdp->Revision;                                  // Get revision number
            acpi_XSDP_t* xsdp = (acpi_XSDP_t*)(i + sizeof(acpi_RSDP_t));
            if (acpiTable.Revision) {                                       // Check summary of the XSDP if supported
                uint8_t* xsdpbyte = (uint8_t*)xsdp; uint32_t sum = 0;
                for (size_t i = 0; i < sizeof(acpi_XSDP_t); ++i) { sum += xsdpbyte[i]; }
                if ((sum & 0xFF) != 0x00) { PANIC("XSDP table damaged"); }          // If wrong, panic then
            }
            ncopy(acpiTable.OEMID, rsdp->OEMID, 6); acpiTable.support = true; // Get OEMID and mark table as ACPI supported
            acpi_SDTHeader_t* sdthead =     // Get SDT headers address
                (acpi_SDTHeader_t*)(size_t)(acpiTable.Revision ? xsdp->XsdtAddress : rsdp->RsdtAddress);
            // Check summary of SDT header
            uint8_t sum = 0; for (size_t i = 0; i < sdthead->Length; ++i) { sum += ((char *) sdthead)[i]; }
            if (sum != 0x00) { PANIC("SDT header damaged"); }       // If wrong, panic then
            int sdtcount = (sdthead->Length - sizeof(acpi_SDTHeader_t)) / 4;                // Calculate SDT count
            uint32_t* othersdt = (uint32_t*)((size_t)sdthead + sizeof(acpi_SDTHeader_t));   // Define SDT list
            // Find FADT table
            for (int i = 0; i < sdtcount; ++i) {
                // Get SDT header
                acpi_SDTHeader_t* t = (acpi_SDTHeader_t*)othersdt[i];
                // Check "FACP" signature and if correct, set as FADT on ACPI table
                if (ncompare(t->Signature, "FACP", 4) == 0) { acpiTable.fadt = (acpi_FADT_t*)othersdt[i]; }
            } if (acpiTable.fadt == NULL) { PANIC("No FADT found"); }       // If not found, panic then
            if ((port_inw(acpiTable.fadt->PM1aControlBlock) & 1) == 0) {                // Check ACPI active bit
                port_outb(acpiTable.fadt->SMI_CommandPort,acpiTable.fadt->AcpiEnable);  // If disable, active then
                if (kernel_CPUInfo.has_tsc & 2) {   // Use TSC delay if stable
                    uint32_t timeout = 5000 / 10;       // Set timeout
                    // Wait for ACPI to become active
                    while ((port_inw(acpiTable.fadt->PM1aControlBlock) & 1) == 0 && timeout > 0) { delay(10); --timeout; }
                    // If ACPI not activated until timeout, log as warning and continue without ACPI
                    if (timeout == 0) { ERR("ACPI activation timed out");
                        acpiTable.support = false; acpiTable.fadt = NULL; acpiTable.mcfg = NULL; break; }
                } else {                        // Else use RTC sleep
                    // ! WARN("Activating ACPI in 3 seconds..."); sleep(4);
                    if ((port_inw(acpiTable.fadt->PM1aControlBlock) & 1) == 0)
                        { ERR("ACPI activation failed"); }
                }
            }
            // Find MCFG table
            for (int i = 0; i < sdtcount; ++i) {
                // Get SDT header
                acpi_SDTHeader_t* t = (acpi_SDTHeader_t*)othersdt[i];
                // Check "MCFG" signature and if correct, set as MCFG on ACPI table
                if (ncompare(t->Signature, "MCFG", 4) == 0) { acpiTable.mcfg = (acpi_MCFG_t*)othersdt[i]; }
            } break;    // Break the loop
        }
    } if (!acpiTable.support) { WARN("ACPI not supported"); }
}