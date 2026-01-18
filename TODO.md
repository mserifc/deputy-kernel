# I will fix the following problems when the kernel is finished:

- Many buffers are set statically. These will be set to dynamically

- Some string size constraints do not include the null terminator, but I found this unnecessary, so this will be fixed.

- Command handler and shell program will be moved to user space for security.

- More system calls will be added.