# Deputy the Operating System Kernel

A small, experimental operating system kernel developed as a personal learning project.

This repository contains the **current state** of the kernel.  
The codebase is functional but not fully cleaned up, and some subsystems are still incomplete.

The project shows approximately 32 builds due to internal forking, even though only 17 snapshots were published publicly.

> Warning: This project is not production-ready and is intended for educational and experimental purposes.

## Overview

### Basic information

- Architecture: 32-bit x86 (i386)
- Boot method: GRUB
- Kernel type: Monolithic

### Features

- Multiboot support
- Early boot setup (stack, GDT, IDT)
- Exception handling
- Hardware identification
- Memory detection and region tracking
- Dynamic memory management
- Cooperative multitasking
- Context switch mechanism
- Software interrupt handling (system call)
- Basic system calls
- Own kernel file system
- USTAR extractor/mounter
- FAT32 reader (experimental)
- Userland image loading
- PS/2 keyboard and mouse driver
- i8042 driver
- 24-bit color high resolution display driver (via GRUB, but not used currently)
- USB XHCI driver (hanged, incomplete)
- ACPI support
- PCI/PCIe bus support
- PIE ELF program loader
- Basic standard I/O support
- Basic runtime protection (program inspection)
- VGA/Serial kernel logging
- Basic test routines (memory management, file system, multitask, PCI/PCIe bus scan, etc.)
- Cross-compiler (clang) support and custom makefiles
- Code comments and historical notes

### Previously added but cancelled

- PIC and IRQ handling (deemed unnecessary for now)
- ATA driver (deemed unnecessary for concept and old)
- USB UHCI driver (old and not worth the effort)
- 320x240x256 and 640x480x16 display drivers (own but potentially dangerous for real hardware, replaced with GRUB video mode)
- Access control and accounts (hanged to be moved to userland)
- Built-in shell and text editor (hanged to be moved to userland)

### Not implemented

- SATA/NVMe driver (deemed unnecessary, replaced with USB flash storage)
- USB flash storage driver (planned, but XHCI driver hanged and project ended)
- Memory isolation (planned, but project ended)

## Build & Run

### Requirements
- i386-elf-gcc
- qemu-system-x86

### Building

By default, the project is built using i386-elf-gcc.

If i386-elf-gcc is not available, Clang can be used instead.
In that case, rename `Makefile_clang` to `Makefile` in the project root.

To build the kernel:
```sh
make all
```

### Running

To run the kernel in QEMU:

```sh
make run
```

## Acknowledgements & References

This project was developed primarily through experimentation, research, and direct implementation.

At very early stages, some publicly available resources were briefly reviewed to gain general orientation on operating system development. These reviews remained mostly at a conceptual level, and external source code was not directly studied or reused.

The current codebase is an independent implementation, designed and developed over time.

Key reference resources:
- OSDev Wiki — general reference for x86 architecture and kernel design concepts
- *Write Your Own Operating System* (YouTube channel) — early guidance on bootstrapping and foundational OS concepts

## Disclaimer

This project is not intended for production use. The code is provided as-is, primarily for learning, experimentation, and reference.
- 
