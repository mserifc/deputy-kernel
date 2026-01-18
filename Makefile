# C Compiler
CC = i386-elf-gcc
# Assembler
AS = i386-elf-as
# Linker
LD = i386-elf-ld

# C Compiler Flags
CC_FLAGS = \
	-m32 \
	-nostdlib \
	-fno-builtin \
	-fno-exceptions \
	-fno-leading-underscore \
	-I include
# Assembler Flags
AS_FLAGS = --32
# Linker Flags
LD_FLAGS = -melf_i386

# Source Directory
SRC_DIR = kernel
# Build Directory
BUILD_DIR = build

# Kernel
KERNEL = $(BUILD_DIR)/kernel.elf
# Objects
OBJECTS = \
	$(BUILD_DIR)/entry.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/memory.o \
	$(BUILD_DIR)/common.o \
	\
	$(BUILD_DIR)/platform/i386/port.o \
	$(BUILD_DIR)/platform/i386/gdt_flush.o \
	$(BUILD_DIR)/platform/i386/gdt_init.o \
	$(BUILD_DIR)/platform/i386/interrupts.o \
	$(BUILD_DIR)/platform/i386/interrupt_stubs.o \
	\
	$(BUILD_DIR)/drivers/display.o \
	$(BUILD_DIR)/drivers/keyboard.o \
	\
	$(BUILD_DIR)/filesystem/ramfs.o

# Emulator
EMULATOR = qemu-system-i386
# Emulator Flags
EMULATOR_FLAGS = -monitor stdio
# -d int -serial stdio

# Make Kernel
all: $(KERNEL)

# Make Build Directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C Code Files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	mkdir -p $(@D)
	$(CC) $(CC_FLAGS) -c $< -o $@

# Assemble Assembly Files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	mkdir -p $(@D)
	$(AS) $(AS_FLAGS) $< -o $@

# Link Objects
$(KERNEL): linker.ld $(OBJECTS)
	$(LD) $(LD_FLAGS) -T linker.ld -o $@ $(OBJECTS)

# Run with Emulator
run:
	$(EMULATOR) $(EMULATOR_FLAGS) -kernel $(KERNEL)

# Cleanup
clean:
	rm -f $(OBJECTS) $(KERNEL)

# Mark 'all', 'run', and 'clean' as non-file targets.
.PHONY: all run clean