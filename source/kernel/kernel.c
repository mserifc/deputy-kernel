#include "kernel.h"

#include "multiboot.h"

#include "hw/port.h"
#include "hw/protect.h"
#include "hw/interrupts.h"
#include "hw/acpi.h"
#include "hw/devbus.h"
#include "hw/i8042.h"

#include "fs/tarfs.h"

#include "drv/keyboard.h"
#include "drv/mouse.h"

// * Variables and tables

// Physical size of the kernel in memory
size_t kernel_PhysicalSize;

// Operating system module size in memory
size_t kernel_OSModuleSize;

// Memory size of the machine
size_t kernel_MemorySize;

// CPU information table
kernel_CPUInfo_t kernel_CPUInfo;

void test(void) {
    printf("Merhaba, dunya!\n");
    // i386-elf-gcc -m32 -nostdlib -Ttext=0x4000000 -static -o test.elf test.c
    // INFO("Program starting...");
    // program_exec("/system/test.elf");
    exit();
}

void test1() {
    while (true) {
        printf("Example task 1 running...\n");
        sleep(1);
        yield();
        printf("Returned to example task 1\n");
    }
}

void test2() {
    while (true) {
        printf("Example task 2 working...\n");
        sleep(5);
        yield();
        printf("Returned to example task 2\n");
    }
}

void test3() {
    while (true) {
        printf("Example task 3 in focus...\n");
        sleep(10);
        yield();
        printf("Returned to example task 3\n");
    }
}

// * Functions

// Initialize function of kernel
void kernel_init(multiboot_info_t* boot_info, multiboot_uint32_t boot_magic) {

    // * Check multiboot magic number
        if (boot_magic != MULTIBOOT_BOOTLOADER_MAGIC) { PANIC("Invalid multiboot magic number"); }

    // * Check system memory map
        if (!(boot_info->flags & MULTIBOOT_INFO_MEM_MAP)) { PANIC("Invalid memory map given by bootloader"); }

    // * Detect hardware and identify
        // Calculate kernel size
            kernel_PhysicalSize = (size_t)&kernel_Limit - (size_t)&kernel_Base;
        // Get memory size from bootloader
            kernel_MemorySize = (boot_info->mem_lower * 1024) + (boot_info->mem_upper * 1024);
        // Get information about CPU
        {   uint32_t eax, ebx, ecx, edx;
            // Vendor string (EAX=0)
            utils_cpuid(0, 0, &eax, &ebx, &ecx, &edx);
            ncopy(kernel_CPUInfo.vendor + 0, &ebx, 4);
            ncopy(kernel_CPUInfo.vendor + 4, &edx, 4);
            ncopy(kernel_CPUInfo.vendor + 8, &ecx, 4);
            kernel_CPUInfo.vendor[12] = '\0';
            // Processor brand string (EAX=0x80000002,3,4)
            static char brand[49];
            for (size_t i = 0; i < 3; i++) {
                utils_cpuid(0x80000002 + i, 0, &eax, &ebx, &ecx, &edx);
                ncopy(brand + i * 16 + 0, &eax, 4);
                ncopy(brand + i * 16 + 4, &ebx, 4);
                ncopy(brand + i * 16 + 8, &ecx, 4);
                ncopy(brand + i * 16 + 12, &edx, 4);
            }
            ncopy(kernel_CPUInfo.brand, brand, 48);
            kernel_CPUInfo.brand[48] = '\0';
            // Basic features (EAX=1)
            utils_cpuid(1, 0, &eax, &ebx, &ecx, &edx);
            // Flags
            kernel_CPUInfo.has_tsc = (edx >> 8) & 1;    // TSC bit in EDX
            if (!kernel_CPUInfo.has_tsc) { WARN("TSC not supported"); }
            kernel_CPUInfo.has_sse = (edx >> 25) & 1;   // SSE bit in EDX
            kernel_CPUInfo.has_avx = (ecx >> 28) & 1;   // AVX bit in ECX
            kernel_CPUInfo.has_vtx = (ecx >> 5) & 1;    // VMX bit in ECX (Intel VT-x)
            kernel_CPUInfo.has_aes = (ecx >> 25) & 1;   // AES bit in ECX
            // Number of logical processors (threads) (EBX bits 23:16)
            kernel_CPUInfo.threads = (ebx >> 16) & 0xff;
            // Number of cores (EAX=4, ECX=0)
            utils_cpuid(4, 0, &eax, &ebx, &ecx, &edx);
            kernel_CPUInfo.cores = ((eax >> 26) & 0x3f) + 1;    // Number of cores - 1 + 1 = cores
            if (kernel_CPUInfo.cores == 0) { kernel_CPUInfo.cores = 1; }    // Fallback
            // If threads is zero, use core count
            if (kernel_CPUInfo.threads == 0) { kernel_CPUInfo.threads = kernel_CPUInfo.cores; }
            // CPU Architecture (EAX=0x80000001)
            utils_cpuid(0x80000001, 0, &eax, &ebx, &ecx, &edx);
            // ECX bit 29 - LM (Long Mode)
            kernel_CPUInfo.has_x64 = (edx & (1 << 29)) != 0;
            // TSC stability (EAX=0x80000007)
            utils_cpuid(0x80000007, 0, &eax, &ebx, &ecx, &edx);
            if (kernel_CPUInfo.has_tsc && ((edx >> 8) & 1)) {   // Calculate first frequency if TSC stable
                sleep(1); uint64_t tsc1 = utils_rdtsc();        // Get TSC value after 1 second later
                sleep(1); uint64_t tsc2 = utils_rdtsc();        // Get TSC value after 2 seconds later
                kernel_CPUInfo.frequency = tsc2 - tsc1;         // Calculate frequency
                kernel_CPUInfo.has_tsc |= 2;                    // Set stability bit
            } else if (kernel_CPUInfo.has_tsc) { WARN("TSC not stable"); }
        }
    
    // * Find the kernel's memory field
    size_t fieldSize = 0; {         // Variable for get available field size
        // Check for memory fields
        for (size_t i = 0; i < boot_info->mmap_length; i += sizeof(multiboot_memory_map_t)) {
            // Get memory field from bootloader
            multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (boot_info->mmap_addr + i);
            // Check is kernel field or not
            if(mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {              // Check this field available
                if (mmmt->addr == (size_t)&kernel_Base)                 // If kernel field is here
                    { fieldSize = (size_t)mmmt->len; }                  // Use this field
            }
        } if (!fieldSize) { PANIC("Kernel field not found"); }          // If not found, halt machine
    }

    // * Get operating system module
        kernel_OSModuleSize = 0; if (boot_info->mods_count >= 1) {  // Check how many modules loaded
            // If multiple modules found, log as warn and use the first module as operating system
            if (boot_info->mods_count > 1) { WARN("Multiple modules found, using the first module"); }
            // Get module address from bootloader
            multiboot_module_t* mod = (multiboot_module_t*)boot_info->mods_addr;
            // Calculate operating system module size
            kernel_OSModuleSize = (size_t)(mod->mod_end - mod->mod_start);
            // Check if the OS module can fit in the kernel field
            if ((fieldSize - kernel_PhysicalSize) < kernel_OSModuleSize)
                { PANIC("Operating system module too large"); }     // Generate panic if the module cannot fit
            INFO("Loading OS module: %s", (char*)mod->cmdline);         // Print loaded operating system module
            uint32_t hash1 = fnv1ahash((void*)mod->mod_start, kernel_OSModuleSize); // Hash the module
            ncopy(&kernel_Limit, (void*)mod->mod_start, kernel_OSModuleSize);   // Load the module next to the kernel
            uint32_t hash2 = fnv1ahash(&kernel_Limit, kernel_OSModuleSize);         // Hast the loaded module
            if (hash1 != hash2) { PANIC("Failed to load the operating system module"); } // Generate panic if different
            // Module loaded successfully if no difference
            else { INFO("OS module loaded successfully (%s)", unit(kernel_OSModuleSize)); }
        } else { WARN("No operating system module found"); }    // Generate panic if no module loaded

    // * Initialize kernel components
        protect_init();                                                         // Initialize Protected Mode
        interrupts_init();                                                      // Initialize Interrupt Manager
        memory_init(fieldSize - (kernel_PhysicalSize + kernel_OSModuleSize));   // Initialize Memory Manager
        corefs_init();                                                          // Initialize Core File System
        multitask_init();                                                       // Initialize Multitasking
        mountmgr_init();                                                        // Initialize Mount Manager
        syscall_init();                                                         // Initialize System Call Manager
        acpi_init();                                                            // Initialize ACPI
        devbus_init();                                                          // Initialize Device Bus (PCI/PCIe)
        i8042_init();                                                           // Initialize I8042 PS/2 Controller
    
    // * Print information about kernel and hardware
    if (false) {
        putchar('\n');
        printf("Kernel size: %s\n", unit(kernel_PhysicalSize));         // Print kernel size
        printf("Memory size: %s\n", unit(kernel_MemorySize));           // Print physical memory size
        printf("Field size: %s\n", unit(fieldSize));                    // Print kernel field size
        printf("Free memory: %s\n", unit(mavail()));                    // Print free memory size
        { date_t current; date(&current);
        printf("Time and date: %d:%d:%d %d/%d/%d\n",                    // Print date and time
            current.hour, current.min, current.sec,                     // Print time
            current.day, current.mon, current.year);                    // Print date
        }
        printf("CPU Information:\n");                                   // Print CPU information
            printf("\tVendor: %s\n", kernel_CPUInfo.vendor);            // Print vendor name
            printf("\tBrand: %s\n", kernel_CPUInfo.brand);              // Print brand name
            printf("\tCores\tThreads\tSSE\tAVX\tVT-x\tAES\tx64\n");     // Print features
            printf("\t%d\t%d\t%s\t%s\t%s\t%s\t%s\n",
                kernel_CPUInfo.cores,                                   // Core count
                kernel_CPUInfo.threads,                                 // Thread count
                kernel_CPUInfo.has_sse ? "Yes" : "No",                  // SSE support
                kernel_CPUInfo.has_avx ? "Yes" : "No",                  // AVX support
                kernel_CPUInfo.has_vtx ? "Yes" : "No",                  // VT-x support
                kernel_CPUInfo.has_aes ? "Yes" : "No",                  // AES support
                kernel_CPUInfo.has_x64 ? "Yes" : "No");                 // x64 support
    } putchar('\n');

    // Print kernel boot success message and build version
    printf("Deputy the kernel: build %d, booted successfully.\n", KERNEL_BUILD);

    extern void kernel_main(void); kernel_main();   // Switch to kernel main
}

// Main function of kernel
void kernel_main(void) {
    puts("Welcome!\n");     // Print welcome message for user
    putchar('\n');

    // * Memory manager test
    if (false) {
        puts("---- Memory Test Started ----\n");
        printf("Available memory: %dKB\n", mavail()/1024);
        char* my_data = (char*)malloc(9*1024);
        if (my_data == NULL) { PANIC("Memory test error: Allocated memory is null"); }
        printf("Allocated a memory block.\n");
        // copy(my_data, "Hello, World!");
        snprintf(my_data, MEMORY_BLKSIZE, "Hello, %s!", "World");
        printf("Writed data to allocated memory block.\n");
        printf("My data: %s\n", my_data);
        printf("Available memory: %dKB\n", mavail()/1024);
        free(my_data);
        printf("Allocated memory freed.\n");
        printf("Available memory: %dKB\n", mavail()/1024);
        puts("---- Memory Test Ended ----\n");
        putchar('\n');
    }

    // * File system test
    if (false) {
        puts("---- File system test started ----\n");
        int status;
        status = fs_createDir("/home/");
        printf("Create /home/: %d\n", status); // !
        status = fs_createDir("/home/user/");
        printf("Create /home/user/: %d\n", status); // !
        char* content = "Hello, world!";
        status = fs_writeFile("/home/user/test.txt", 15, content);
        printf("Write /home/user/test.txt: %d\n", status); // !
        char* data = fs_readFile("/home/user/test.txt");
        if (data) {
            printf("Read /home/user/test.txt: %s\n", data);
        } else {
            printf("Read /home/user/test.txt: FAILED\n");
        } // !
        int* entries = fs_readDir("/home/user/");
        if (entries) {
            printf("Entries under /home/user/:\n");
            for (int i = 0; i < FS_MAX_ENTCOUNT && entries[i] != 0; ++i) {
                fs_Entry_t* ent = fs_dirent(entries[i]);
                if (ent) {
                    printf(" - %s (%d)\n", ent->name, ent->type);
                }
            }
        } else {
            printf("ReadDir /home/user/: FAILED\n");
        } // !
        status = fs_remove("/home/user/test.txt");
        printf("Remove /home/user/test.txt: %d\n", status); // !
        data = fs_readFile("/home/user/test.txt");
        if (data) {
            printf("Unexpected Read: %s\n", data);
        } else {
            printf("Read deleted file: Correctly failed\n");
        } // !
        status = fs_remove("/home/");
        printf("Try remove /home/: %d\n", status); // !
        status = fs_remove("/home/user/");
        printf("Remove /home/user/: %d\n", status); // !
        status = fs_remove("/home/");
        printf("Remove /home/: %d\n", status); // !
        status = fs_remove("/");
        printf("Remove root (/): %d\n", status); // !
        puts("---- File system test ended ----\n");
        putchar('\n');
    }

    // * PCI/PCIe device list
    if (false) {
        printf("PCI/PCIe Device list:\n"); devbus_Device_t devinfo;
        printf("VENDOR\tDEVICE\tREV\tIF\tSUBC\tCLASS\n");
        for (int b = 0; b < DEVBUS_MAX_BUSES; ++b) {
            for (int s = 0; s < DEVBUS_MAX_SLOTS; ++s) {
                for (int f = 0; f < DEVBUS_MAX_FUNCS; ++f) {
                    devbus_get(&devinfo, b, s, f); if (devinfo.vendor != 0xFFFF) {
                        printf("0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\n",
                            devinfo.vendor, devinfo.device, devinfo.revision,
                            devinfo.interface, devinfo.subclass, devinfo.class);
                    }
                }
            }
        }
    }

    // * Mount operating system module to core file system
    if (kernel_OSModuleSize) {
        INFO("Mounting OS module at root directory...");
        if (tarfs_mount("/", &kernel_Limit, kernel_OSModuleSize) == -1)
            { PANIC("Operating system module mounting failed"); }
        else { INFO("OS module mounted successfully"); }
    }

    // extern unsigned char _binary_test_disk_img_start[]; extern unsigned char _binary_test_disk_img_end[];
    // INFO("Disk size: %s", unit((size_t)&_binary_test_disk_img_end - (size_t)&_binary_test_disk_img_start));
    // int n=ramdisk_create(&fat32_Disk, ((size_t)&_binary_test_disk_img_end-(size_t)&_binary_test_disk_img_start)/512,512);
    // if (n != 0) { ERR("Unable to create ramdisk"); }
    // free(fat32_Disk.storage); fat32_Disk.storage = _binary_test_disk_img_start;
    // for (int i = 0; i < 4; ++i) {
    //     DiskPartEntry_t* entry = (DiskPartEntry_t*)(_binary_test_disk_img_start + UTILS_DISKPARTTABLEOFF + i*16);
    //     INFO("Partition %d, type 0x%x, start %d, end %d", i, entry->systemID, entry->relativeSector, entry->totalSector);
    //     if (entry->systemID == 0x0C) {
    //         char* part = (char*)((size_t)_binary_test_disk_img_start + (entry->relativeSector * 512));
    //         if (fat32_mount(entry->relativeSector) != 0) { ERR("Mount failed"); }
    //     }
    // }

    // extern void usb_process(void);
    // spawn("usb_process", usb_process);
    // while (true) { yield(); }

    // tarfs_list(&kernel_Limit, kernel_OSModuleSize);

    int* entries = fs_readDir("/");
    if (entries) {
        printf("Entries under /:\n");
        for (int i = 0; i < FS_MAX_ENTCOUNT && entries[i] != FS_DIRENTEND; ++i) {
            fs_Entry_t* ent = fs_dirent(entries[i]);
            if (!ent) { printf("dirent %d: FAILED", i); }
            if (ent) {
                printf(" - %s\tsize: %s (%dB))\n", ent->name, unit(ent->size), ent->size);
                // if (ent->type != FS_TYPE_DIR)
                //     { printf("%s: %s (%dB): %s\n", ent->name, unit(ent->size), ent->size, fs_readFile(ent->name)); }
            }
        }
    } else {
        printf("ReadDir /: FAILED\n");
    }

    // * Initialize device files
    if (true) { fs_Entry_t* dev; static char zero[MEMORY_BLKSIZE];
        if (fs_createDir("/dev/") != FS_STS_SUCCESS) { PANIC("Unable to create directory '/dev/'"); }
        // /dev/keyboard
        if (fs_writeFile("/dev/keyboard", MEMORY_BLKSIZE, zero) != FS_STS_SUCCESS)
            { PANIC("Unable to create device file '/dev/keyboard'"); }
        dev = fs_stat("/dev/keyboard"); if (!dev)
            { PANIC("Unable to get device file '/dev/keyboard'"); }
        dev->ftype = FS_TYPE_CHARDEV; dev->devperm = O_RDONLY;
        // /dev/mouse
        if (fs_writeFile("/dev/mouse", MEMORY_BLKSIZE, zero) != FS_STS_SUCCESS)
            { PANIC("Unable to create device file '/dev/mouse'"); }
        dev = fs_stat("/dev/mouse"); if (!dev)
            { PANIC("Unable to get device file '/dev/mouse'"); }
        dev->ftype = FS_TYPE_CHARDEV; dev->devperm = O_RDONLY;
    }

    if (true) {
        extern void kernel_idle(void);
        if (spawn("kernel_idle", kernel_idle) == -1)
            { PANIC("Failed to start kernel idle task"); }
        exec("/system/test.elf");
        while (true) {
            i8042_proc();
            char data;
            int keyboard = open("/dev/keyboard", O_RDONLY);
            if (read(keyboard, &data, 1) == 1)
                { INFO("%s: 0x%x", (data & KEY_RELEASE) ? "Released" : "Pressed", (data & KEY_CODE)); }
            char data2[3];
            int mouse = open("/dev/mouse", O_RDONLY);
            if (read(mouse, data2, 3) == 3) {
                INFO("Mouse: Stat: 0x%x, Xmox: %d, Ymov: %d", data2[0], data2[1], data2[2]);
            }
            yield();
        }
    }

    PANIC("No processes to execute");   // Switch to idle if no tasks found
}

void kernel_idle(void) {
    while (true) {
        if (!(kernel_CPUInfo.has_tsc & 2)) {
            uint64_t tsc1 = utils_rdtsc();
            sleep(1); uint64_t tsc2 = utils_rdtsc();
            kernel_CPUInfo.frequency = tsc2 - tsc1;
        }
        yield();
    }
}