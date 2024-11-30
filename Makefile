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
	$(BUILD_DIR)/port.o \
	$(BUILD_DIR)/gdt_flush.o \
	$(BUILD_DIR)/gdt_init.o \
	$(BUILD_DIR)/interrupts.o \
	$(BUILD_DIR)/interrupt_stubs.o \
	$(BUILD_DIR)/detect.o \
	$(BUILD_DIR)/memory.o \
	$(BUILD_DIR)/display.o \
	$(BUILD_DIR)/keyboard.o \
	$(BUILD_DIR)/common.o

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
	$(CC) $(CC_FLAGS) -c $< -o $@

# Assemble Assembly Files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	$(AS) $(AS_FLAGS) $< -o $@

# Link Objects
$(KERNEL): $(OBJECTS)
	$(LD) $(LD_FLAGS) -T linker.ld -o $@ $(OBJECTS)

# Run with Emulator
run:
	$(EMULATOR) $(EMULATOR_FLAGS) -kernel $(KERNEL)

# Cleanup
clean:
	rm -f $(BUILD_DIR)/*.o $(KERNEL)

# Mark 'all', 'run', and 'clean' as non-file targets.
.PHONY: all run clean
