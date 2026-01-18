# x86 Operating System Kernel

This project is the development of an x86-based operating system kernel. The goal is to build a fully functional kernel that can handle various system-level tasks, such as hardware interaction, memory management, and process handling. Currently, the project is in active development, with the following features already implemented or in progress:

## Features

### 1. Hardware Information Retrieval
- The kernel can query and retrieve essential hardware information, allowing the system to interact with the underlying hardware more efficiently.

### 2. Global Descriptor Table (GDT)
- The kernel includes a basic implementation of the Global Descriptor Table (GDT), essential for memory segmentation and defining various memory regions.

### 3. Interrupt and Exception Handling
- Interrupts and exceptions are handled by the kernel. While the basic structure is in place, further work is needed to fully handle all types of interrupts and exceptions.

### 4. Memory Management
- Basic memory management has been implemented. There is ongoing development to improve the management of physical and virtual memory.

### 5. Port I/O
- The kernel supports basic input/output operations through the x86 I/O ports, allowing for communication with peripheral devices.

### 6. Screen Output
- The kernel supports outputting text to the screen, allowing basic interaction and debugging through visual feedback.

### 7. Keyboard Input
- Keyboard input handling is in progress. The system can process basic keypresses, but further refinement is needed for full functionality.

### 8. Standard Library Functions
- A set of basic standard functions (e.g., memory allocation, string handling) is implemented, providing essential utilities for the kernel and applications.

## Upcoming Features

The following features are planned for future development:

### 1. Disk Management
- Implementing disk management capabilities to allow for data storage, reading, and writing to disk drives.

### 2. File System
- A simple file system will be developed to organize and manage files stored on disk.

### 3. System Calls
- Implementing system calls that allow user programs to interact with the kernel, requesting services such as file operations, process management, and hardware access.

### 4. Task/Process Management
- Developing task and process management to enable multitasking and process scheduling, allowing the kernel to run multiple programs concurrently.

## Getting Started

**Note:** Before building the kernel, you need to install *i386-elf-gcc* to your computer.

To build and run the kernel, follow these steps:

1. Clone the repository:
   ```bash
   git clone https://github.com/mserifc/kernel.git
   ```

2. Build kernel:
    ```bash
    make all
    ```

3. Run the kernel with QEMU:
    ```bash
    make run
    ```