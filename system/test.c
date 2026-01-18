void _start() {
    const char* filename = "output.txt";  // Dosya adı
    const char* message = "Merhaba, Assembly!";  // Yazılacak mesaj
    unsigned short* vga = (unsigned short*)0xB8000;
    vga[0] = (0x0F00 | 'H'); vga[1] = (0x0F00 | 'i'); vga[2] = (0x0F00 | ' '); //while(1);
    unsigned int* msg = (unsigned int*)((unsigned int)message);
    // msg[0] = 0xF4;
    // asm volatile ("jmp *%0" :: "r"(message));
    while(1){
        for (int i = 0; message[i] != '\0'; ++i) { vga[i] = (0x0F00 | message[i]); }//while(1);
        __asm__ ("int $0x9E");
    }
}

// void yaz_dosyaya() {
//     // asm volatile ("hlt");
//     const char* filename = "output.txt";  // Dosya adı
//     const char* message = "Merhaba, Assembly!";  // Yazılacak mesaj
//     unsigned short* vga = (unsigned short*)0xB8000;
//     vga[0] = (0x0F00 | 'H'); vga[1] = (0x0F00 | 'i'); vga[2] = (0x0F00 | ' '); //while(1);
//     unsigned int* msg = (unsigned int*)((unsigned int)message);
//     // msg[0] = 0xF4;
//     // asm volatile ("jmp *%0" :: "r"(message));
//     while(1){
//         for (int i = 0; message[i] != '\0'; ++i) { vga[i] = (0x0F00 | message[i]); }//while(1);
//         __asm__ ("int $0x9E");
//     }
//     for (int i = 0; filename[i] != '\0'; ++i) { vga[i] = (0x0F00 | filename[i]); }
//     __asm__ ("mov $1, %eax; int $0x80");
//     int fd;

//     // Dosyayı aç (O_CREAT | O_WRONLY | O_TRUNC)
//     __asm__ (
//         "movl $5, %%eax;"              // syscall number for open (5)
//         "movl %[filename], %%ebx;"     // Dosya adı (filename)
//         "movl $577, %%ecx;"            // flags: O_WRONLY | O_CREAT | O_TRUNC
//         "movl $420, %%edx;"            // mode: 0644
//         "int $0x80;"                   // syscall çağrısı
//         "movl %%eax, %[fd];"           // Dosya tanıtıcısını fd'ye kaydet
//         : [fd] "=r" (fd)               // output
//         : [filename] "r" (filename)    // input
//         : "%eax", "%ebx", "%ecx", "%edx" // clobbered registers
//     );

//     // Mesajı dosyaya yaz (write)
//     __asm__ (
//         "movl $4, %%eax;"              // syscall number for write (4)
//         "movl %[fd], %%ebx;"           // Dosya tanıtıcısı
//         "movl %[message], %%ecx;"      // Yazılacak mesaj
//         "movl $19, %%edx;"             // Mesaj uzunluğu
//         "int $0x80;"                   // syscall çağrısı
//         :
//         : [fd] "r" (fd), [message] "r" (message)
//         : "%eax", "%ebx", "%ecx", "%edx"
//     );

//     // Dosyayı kapat (close)
//     __asm__ (
//         "movl $6, %%eax;"              // syscall number for close (6)
//         "movl %[fd], %%ebx;"           // Dosya tanıtıcısı
//         "int $0x80;"                   // syscall çağrısı
//         :
//         : [fd] "r" (fd)
//         : "%eax", "%ebx"
//     );
//     __asm__ ("mov $1, %eax; int $0x80");
// }

// void _start() {
//     yaz_dosyaya();
//     extern int main();
//     main(); __asm__ ("mov $1, %eax\t\n int $0x80");
// }

// int main() {
//     yaz_dosyaya();
//     return 0;
// }