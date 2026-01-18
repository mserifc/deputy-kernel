# C Compiler
CC = i386-elf-gcc
# Assembler
AS = i386-elf-as
# Linker
LD = i386-elf-ld
# Tools C Compiler
TOOLS_CC = g++

# C Compiler Flags
CC_FLAGS = \
	-m32 \
	-nostdlib \
	-ffreestanding \
	-fno-builtin \
	-fno-exceptions \
	-fno-leading-underscore \
	-I include
# Assembler Flags
AS_FLAGS = --32
# Linker Flags
LD_FLAGS = -m elf_i386
# Tools C Compiler Flags
TOOLS_CC_FLAGS = -std=c++17

# Source Directory
SOURCE_DIR = kernel
# Build Directory
BUILD_DIR = build
# Tools Directory
TOOLS_DIR = tools

# Kernel
KERNEL = $(BUILD_DIR)/kernel.elf
# Objects
OBJECTS = \
	$(BUILD_DIR)/entry.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/common.o \
	$(BUILD_DIR)/memory.o \
	\
	$(BUILD_DIR)/platform/i386/port.o \
	$(BUILD_DIR)/platform/i386/gdt_flush.o \
	$(BUILD_DIR)/platform/i386/gdt_init.o \
	$(BUILD_DIR)/platform/i386/interrupts.o \
	$(BUILD_DIR)/platform/i386/interrupt_stubs.o \
	\
	$(BUILD_DIR)/drivers/display.o \
	$(BUILD_DIR)/drivers/keyboard.o \
	$(BUILD_DIR)/drivers/disk.o \
	\
	$(BUILD_DIR)/filesystem/ownfs.o
# Tools
TOOLS = $(TOOLS_DIR)/fsbridge

# Emulator
EMULATOR = qemu-system-i386
# Emulator Flags
EMULATOR_FLAGS = \
	-drive file=$(BUILD_DIR)/disk.img,format=raw,if=ide \
	-rtc base=localtime \
	-monitor stdio
# -d int -serial stdio

# Make Kernel
all: $(KERNEL)

# Make Build Directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C Code Files
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	mkdir -p $(@D)
	$(CC) $(CC_FLAGS) -c $< -o $@

# Assemble Assembly Files
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.s | $(BUILD_DIR)
	mkdir -p $(@D)
	$(AS) $(AS_FLAGS) $< -o $@

# Link Objects
$(KERNEL): linker.ld $(OBJECTS)
	$(LD) $(LD_FLAGS) -T linker.ld -o $@ $(OBJECTS)

# Make tools
tools: $(TOOLS)

# Compile tools source codes
$(TOOLS_DIR)/%: $(TOOLS_DIR)/%.cpp
	$(TOOLS_CC) $(TOOLS_CC_FLAGS) $< -o $@

disk: $(TOOLS_DIR)/fsbridge
	dd if=/dev/zero of=$(BUILD_DIR)/disk.img bs=512 count=65536
	$(TOOLS_DIR)/fsbridge -s $(BUILD_DIR)/disk.img disk

# Run with Emulator
run: $(KERNEL) $(BUILD_DIR)/disk.img
	$(EMULATOR) $(EMULATOR_FLAGS) -kernel $(KERNEL)

# Clean up
clean:
	rm -f $(OBJECTS) $(KERNEL)

# Mark 'all', 'run', and 'clean' as non-file targets.
.PHONY: all tools disk run clean