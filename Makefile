CC = g++
AS = nasm
LD = /usr/local/opt/llvm/bin/ld.lld

CCFLAGS = -m32 --target=i386-unknown-linux-gnu -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions
ASFLAGS = -f elf32
LDFLAGS = -melf_i386

SRC_DIR = kernel
BUILD_DIR = build

KERNEL = $(SRC_DIR)/kernel.cpp
LOADER = $(SRC_DIR)/loader.s
OBJ = $(BUILD_DIR)/loader.o $(BUILD_DIR)/kernel.o

EMULATOR = qemu-system-i386
EMULATOR_FLAGS = -monitor stdio -kernel

all: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel.elf: $(OBJ)
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(OBJ)

run:
	$(EMULATOR) $(EMULATOR_FLAGS) $(BUILD_DIR)/kernel.elf

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/kernel.elf

.PHONY: all run clean