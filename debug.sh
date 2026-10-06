qemu-system-i386 -d int,cpu_reset -no-reboot -machine q35 -cpu n270-v1 -m 512M -device qemu-xhci -rtc base=localtime -kernel build/kernel.elf -initrd build/system.tar 2>&1 | tail -50
