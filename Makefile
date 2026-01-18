# Compiler
CC = i386-elf-gcc
# Assembler
AS = i386-elf-as
# Linker
LD = i386-elf-ld

# Compiler flags
CC_FLAGS = \
	-m32 \
	-nostdlib \
	-ffreestanding \
	-fno-builtin \
	-fno-exceptions \
	-fno-leading-underscore \
	-I include
# 	-mno-80387
# Assembler flags
AS_FLAGS = --32
# Linker flags
LD_FLAGS = -m elf_i386

# Build directory
BUILD_DIR = build
# Kernel source directory
KERNEL_DIR = kernel

# The resulting executable kernel
KERNEL = $(BUILD_DIR)/kernel.elf
# Linker script
LINKER = linker.ld
# Objects
OBJECTS = \
	$(BUILD_DIR)/entry.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/utils.o \
	$(BUILD_DIR)/memory.o \
	$(BUILD_DIR)/multitask.o \
	$(BUILD_DIR)/multitask_switch.o \
	$(BUILD_DIR)/syscall.o \
	$(BUILD_DIR)/iocall.o \
	\
	$(BUILD_DIR)/hardware/port.o \
	$(BUILD_DIR)/hardware/protect_flush.o \
	$(BUILD_DIR)/hardware/protect_init.o \
	$(BUILD_DIR)/hardware/interrupts.o \
	$(BUILD_DIR)/hardware/interrupt_stubs.o \
	$(BUILD_DIR)/hardware/device.o \
	\
	$(BUILD_DIR)/drivers/display.o \
	$(BUILD_DIR)/drivers/keyboard.o \
	$(BUILD_DIR)/drivers/usb.o \
	\
	$(BUILD_DIR)/filesystem/ramfs.o

# Emulator
EMULATOR = qemu-system-i386
# Emulator flags
EMULATOR_FLAGS = -machine q35 -device sdhci-pci -device sd-card -device qemu-xhci -device usb-mouse,id=mouse -monitor stdio
# -usb -device usb-mouse,id=usbmouse -monitor stdio

# Build the kernel
all: $(KERNEL)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	mkdir -p $(@D)
	$(CC) $(CC_FLAGS) -c $< -o $@

# Assemble assembly files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.s | $(BUILD_DIR)
	mkdir -p $(@D)
	$(AS) $(AS_FLAGS) $< -o $@

# Link objects and create the kernel
$(KERNEL): $(LINKER) $(OBJECTS)
	$(LD) $(LD_FLAGS) -T $(LINKER) -o $@ $(OBJECTS)

# Run the kernel with emulator
run: $(KERNEL)
	$(EMULATOR) $(EMULATOR_FLAGS) -kernel $(KERNEL)

# Clean up
clean:
	rm -f $(OBJECTS) $(KERNEL)

# Mark 'all', 'run', and 'clean' as non-file targets
.PHONY: all run clean