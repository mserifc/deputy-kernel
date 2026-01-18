# Compiler
CC = i386-elf-gcc
# Assembler
AS = i386-elf-as
# Linker
LD = i386-elf-ld

# Compiler flags
CC_FLAGS = \
	-m32 \
	-Wextra \
	-nostdlib \
	-ffreestanding \
	-fno-builtin \
	-fno-exceptions \
	-fno-leading-underscore \
	-I include
# -O2 -g
# Assembler flags
AS_FLAGS = --32
# Linker flags
LD_FLAGS = -m elf_i386
# For binary format (also change the kernel file extension to bin): --oformat binary

# Build directory
BUILD_DIR = build
# Source directory
SOURCE_DIR = source

# The resulting executable kernel
KERNEL = $(BUILD_DIR)/kernel.elf
# Operating system module
MODULE = $(BUILD_DIR)/system.tar
# Linker script
LINKER = linker.ld
# Objects
OBJECTS = \
	$(BUILD_DIR)/kernel/entry.o \
	$(BUILD_DIR)/kernel/kernel.o \
	$(BUILD_DIR)/kernel/console.o \
	$(BUILD_DIR)/kernel/utils.o \
	$(BUILD_DIR)/kernel/memory.o \
	$(BUILD_DIR)/kernel/corefs.o \
	$(BUILD_DIR)/kernel/multitask_swi.o \
	$(BUILD_DIR)/kernel/multitask.o \
	$(BUILD_DIR)/kernel/drivers.o \
	$(BUILD_DIR)/kernel/mountmgr.o \
	$(BUILD_DIR)/kernel/syscall.o \
	$(BUILD_DIR)/kernel/iocall.o \
	\
	$(BUILD_DIR)/hw/port.o \
	$(BUILD_DIR)/hw/protect_flush.o \
	$(BUILD_DIR)/hw/protect_init.o \
	$(BUILD_DIR)/hw/interrupt_stubs.o \
	$(BUILD_DIR)/hw/interrupts.o \
	$(BUILD_DIR)/hw/acpi.o \
	$(BUILD_DIR)/hw/devbus.o \
	$(BUILD_DIR)/hw/i8042.o \
	\
	$(BUILD_DIR)/fs/tarfs.o \
	\
	$(BUILD_DIR)/drv/usb.o \
	$(BUILD_DIR)/drv/keyboard.o \
	$(BUILD_DIR)/drv/mouse.o

# Emulator
EMULATOR = qemu-system-x86_64
# Emulator flags
EMULATOR_FLAGS = -machine q35 -cpu n270-v1 -m 512M -device qemu-xhci -rtc base=localtime -monitor stdio
# For inject machine check exception (also need mce feature in cpu): mce 0 0 0xbc00000100000000 0x0 0x12345678 0x0

# Build the kernel
all: $(KERNEL) $(MODULE)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C files
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	mkdir -p $(@D)
	$(CC) $(CC_FLAGS) -c $< -o $@

# Assemble assembly files
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.s | $(BUILD_DIR)
	mkdir -p $(@D)
	$(AS) $(AS_FLAGS) $< -o $@

# Link objects and create the kernel
$(KERNEL): $(LINKER) $(OBJECTS)
	$(LD) $(LD_FLAGS) -T $(LINKER) -o $@ $(OBJECTS)

# Build operating system module
$(MODULE): system
	tar --format=ustar -cf $@ $<

# i386-elf-objcopy -I binary -O elf32-i386 test/disk.img build/disk_img.o

# Run the kernel with OS module via emulator
run: $(KERNEL) $(MODULE)
	$(EMULATOR) $(EMULATOR_FLAGS) -kernel $(KERNEL) -initrd $(MODULE)

# Clean up
clean:
	rm -f $(OBJECTS) $(KERNEL) $(MODULE)

# Mark 'all', 'run', and 'clean' as non-file targets
.PHONY: all run clean