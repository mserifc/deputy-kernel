void _start() {
    const char* filename = "output.txt";  // Dosya adı
    const char* message = "Merhaba, Assembly!\n";  // Yazılacak mesaj
    unsigned short* vga = (unsigned short*)0xB8000;
    //vga[0] = (0x0F00 | 'H'); vga[1] = (0x0F00 | 'i'); vga[2] = (0x0F00 | ' '); //while(1);
    unsigned int* msg = (unsigned int*)((unsigned int)message);
    // msg[0] = 0xF4;
    // asm volatile ("jmp *%0" :: "r"(message));
typedef struct {
    int op;
    unsigned int c; // 0xAARRGGBB or BB GG RR AA
    int x0, y0,
        x1, y1,
        x2, y2;
} display_Pkg_t;
    display_Pkg_t pkgs[] = {
        // DOT — one dot
        {
            .op = 1,
            .x0 = 20, .y0 = 10,
            .c  = 0xFFFFFF00
        },
        // LINE — cross line
        {
            .op = 3,
            .x0 = 0,   .y0 = 0,
            .x1 = 100, .y1 = 85,
            .c  = 0xFF00FFFF
        },
        // RECT — rectangle
        {
            .op = 2,
            .x0 = 50,  .y0 = 50,
            .x1 = 150, .y1 = 150,
            .c  = 0xFF00FF00
        },
        // TRI — triangle
        {
            .op = 4,
            .x0 = 180, .y0 = 100,
            .x1 = 150, .y1 = 200,
            .x2 = 275, .y2 = 185,
            .c  = 0xFFFF0000
        },
    };
    int dev;// = open("/dev/display", O_WRONLY);
    const char* path = "/dev/display";
    asm volatile (
        "mov $5, %%eax\n"
        "mov %1, %%ebx\n"
        "mov $2, %%ecx\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=m"(dev)
        : "r"(path)
        : "memory", "%eax", "%ebx", "%ecx"
    );
    if (dev == -1) {  }
    else {
        /*int sts = write(dev, pkgs, sizeof(pkgs));
        if (sts == -1) {  }*/
        int sts;
        asm volatile (
            "mov $4, %%eax\n"
            "mov %1, %%ebx\n"
            "mov %2, %%ecx\n"
            "mov %3, %%edx\n"
            "int $0x80\n"
            "mov %%eax, %0\n"
            : "=m"(sts)
            : "m"(dev), "r"(pkgs), "r"((int)sizeof(pkgs))
            : "memory", "%eax", "%ebx", "%ecx", "%edx"
        );
        if (sts == -1) { /* err */ }
    }
    while(1) {
    int len = 19;
    asm volatile (
        "movl $4, %%eax\n\t"
        "movl $1, %%ebx\n\t"
        "movl %0, %%ecx\n\t"
        "movl %1, %%edx\n\t"
        "int $0x80\n\t"
        :
        : "m"(message), "m"(len)
        : "%eax", "%ebx", "%ecx", "%edx"
    );
    //len--;
    
    //while(1){
        //for (int i = 0; message[i] != '\0'; ++i) { vga[i] = (0x0F00 | message[i]); }//while(1);
        __asm__ ("int $0x9E");
        /*asm volatile (
        "movl $4, %%eax\n\t"
        "movl $1, %%ebx\n\t"
        "movl %0, %%ecx\n\t"
        "movl %1, %%edx\n\t"
        "int $0x80\n\t"
        :
        : "r"(message), "r"(len)
        : "%eax", "%ebx", "%ecx", "%edx"
    );*/
        //asm volatile ("mov $1, %eax\nint $0x80");
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
