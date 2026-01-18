    // * Get information from bootloader
        if (boot_magic == MULTIBOOT_BOOTLOADER_MAGIC) {         // Check multiboot magic number
            if (boot_info->flags & MULTIBOOT_INFO_VBE_INFO) { console_Active = false; }
            // Get memory size from bootloader
            kernel_MemorySize = (boot_info->mem_lower * 1024) + (boot_info->mem_upper * 1024);
            // Scan memory map for kernel memory field
            if (boot_info->flags & MULTIBOOT_INFO_MEM_MAP) {    // Check memory map from bootloader
                // Check for memory fields
                for (size_t i = 0; i < boot_info->mmap_length; i += sizeof(multiboot_memory_map_t)) {
                    // Get memory field from bootloader
                    multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (boot_info->mmap_addr + i);
                    // Check is kernel field or not
                    if(mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {              // Check this field available
                        if (mmmt->addr == (size_t)&kernel_Base)                 // If kernel field is here
                            { kernel_FieldSize = (size_t)mmmt->len; }           // Use this field
                    }
                } if (!kernel_FieldSize) { ERR("Kernel field not found"); } // Print error if kernel field not found
            } else { ERR("Failed to obtain memory map from bootloader"); }  // Print error if memory map invalid
        } else { ERR("Invalid multiboot magic number"); }                   // Print error if magic number invalid


void memory_init() {
    if (memory_InitLock) { return; } memory_InitLock = true;
    size_t size = kernel_FieldSize - kernel_PhysicalSize;
    if (size > UINT_MAX) { PANIC("64-bit addressing not supported"); }
    size_t supblkc = size / (MEMORY_BLKSIZE + sizeof(memory_Block_t)); supblkc -= supblkc ? 1 : 0;
    if (supblkc <= 0) { PANIC("Not enough memory detected"); }
    memory_Space = (void*)((size_t)&kernel_Limit + (supblkc * sizeof(memory_Block_t)));
    memory_Space = (void*)(((size_t)memory_Space + MEMORY_BLKSIZE) & ((size_t)~(MEMORY_BLKSIZE - 1)));
    memory_Limit = (void*)((size_t)memory_Space + (supblkc * sizeof(MEMORY_BLKSIZE)));
    memory_BlockV = (memory_Block_t*)((size_t)&kernel_Limit); memory_BlockC = supblkc;
    for (size_t i = 0; i < memory_BlockC; ++i) {
        memory_BlockV[i].entry = (void*)((size_t)memory_Space + (i * MEMORY_BLKSIZE));
        memory_BlockV[i].count = 0; memory_BlockV[i].allocated = false;
    }
}