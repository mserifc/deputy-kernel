# x86 Operating System Kernel

This project aims to develop an operating system kernel for the x86 architecture. While still in progress, each feature will be documented as it is completed.

## Introduction

The project has started by focusing on fundamental operating system components. The following features are currently under development:

- **Memory Management**: Developing a memory manager to handle dynamic memory allocation and deallocation.
- **Task Management**: Creating a task scheduler to support multitasking.
- **File System**: Designing and implementing a basic file system to handle file operations.

## Features

### 1. Memory Management
- **Description**: The memory manager is being designed to support both physical and virtual memory management. Basic memory allocation and deallocation functions are complete.
- **Status**: In development.

### 2. Task Management
- **Description**: A task scheduler is being developed to provide support for multitasking. A basic structure for task creation and management is being established.
- **Status**: Not started yet.

### 3. File System
- **Description**: A basic file system is being designed to support fundamental file operations. Infrastructure for opening, closing, reading, and writing files is being developed.
- **Status**: Not started yet.

## Installation

1. Install the necessary tools and libraries. (Just i386-elf-gcc)
2. Navigate to the project directory.
3. Run the following command:
    ```bash
    make clean
    make all
    make run
    ```
