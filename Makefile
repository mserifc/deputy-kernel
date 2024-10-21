CC = i386-elf-gcc
AS = i386-elf-as
LD = i386-elf-ld
 
CC_FLAGS = \
	-m32 \
	-nostdlib \
	-fno-builtin \
	-fno-exceptions \
	-fno-leading-underscore \
	-I include
AS_FLAGS = --32
LD_FLAGS = -melf_i386

SRC_DIR = kernel
BUILD_DIR = build

KERNEL = $(SRC_DIR)/kernel.c
ENTRY = $(SRC_DIR)/entry.s
OBJECTS = \
	$(BUILD_DIR)/entry.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/port.o \
	$(BUILD_DIR)/display.o \
	$(BUILD_DIR)/keyboard.o \
	$(BUILD_DIR)/memory.o \
	$(BUILD_DIR)/common.o

EMULATOR = qemu-system-i386
EMULATOR_FLAGS = -monitor stdio

all: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	$(AS) $(AS_FLAGS) $< -o $@

$(BUILD_DIR)/kernel.elf: $(OBJECTS)
	$(LD) $(LD_FLAGS) -T linker.ld -o $@ $(OBJECTS)

run:
	$(EMULATOR) $(EMULATOR_FLAGS) -kernel $(BUILD_DIR)/kernel.elf

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/kernel.elf

.PHONY: all run clean