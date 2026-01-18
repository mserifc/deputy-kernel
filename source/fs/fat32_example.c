#include "fs/fat32.h"

#include "kernel.h"
#include "drv/ramdisk.h"

ramdisk_t fdisk;

/* ------------------- FAT32 Yapı Tanımları ------------------- */
#pragma pack(push, 1)
typedef struct {
    uint8_t  jmpBoot[3];
    uint8_t  OEMName[8];
    uint16_t bytesPerSector;
    uint8_t  sectorsPerCluster;
    uint16_t reservedSectorCount;
    uint8_t  numFATs;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t  media;
    uint16_t FATSize16;
    uint16_t sectorsPerTrack;
    uint16_t numHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;

    uint32_t FATSize32;
    uint16_t extFlags;
    uint16_t FSVersion;
    uint32_t rootCluster;
    uint16_t FSInfo;
    uint16_t backupBootSector;
    uint8_t  reserved[12];
    uint8_t  driveNumber;
    uint8_t  reserved1;
    uint8_t  bootSignature;
    uint32_t volumeID;
    uint8_t  volumeLabel[11];
    uint8_t  fileSystemType[8];
} __attribute__((packed)) FAT32_BootSector;

typedef struct {
    uint8_t  name[11];       // 8.3 dosya ismi
    uint8_t  attr;           // özellik bayrakları
    uint8_t  NTReserved;
    uint8_t  creationTimeTenths;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessDate;
    uint16_t firstClusterHigh;
    uint16_t writeTime;
    uint16_t writeDate;
    uint16_t firstClusterLow;
    uint32_t fileSize;
} __attribute__((packed)) FAT32_DirEntry;
#pragma pack(pop)

/* ------------------- Global Parametreler ------------------- */
static FAT32_BootSector bs;
static uint32_t FATStart, DataStart;
static uint32_t bytesPerCluster;

/* ------------------- FAT32 Yardımcı Fonksiyonlar ------------------- */
uint32_t cluster_to_lba(uint32_t cluster) {
    return DataStart + (cluster - 2) * bs.sectorsPerCluster;
}

uint32_t get_fat_entry(uint32_t cluster) {
    uint32_t fatOffset = cluster * 4;
    uint32_t sector = FATStart + (fatOffset / bs.bytesPerSector);
    uint8_t buf[512];
    ramdisk_read(&fdisk, sector, buf, 1);

    uint32_t* entry = (uint32_t*)(buf + (fatOffset % bs.bytesPerSector));
    return (*entry) & 0x0FFFFFFF; // 28 bit valid
}

/* ------------------- Mount İşlemi ------------------- */
int fat32_mount(uint32_t lba_start) {
    if (ramdisk_read(&fdisk, lba_start, &bs, 1) != 0) return -1;

    FATStart       = lba_start + bs.reservedSectorCount;
    DataStart      = FATStart + (bs.numFATs * bs.FATSize32);
    bytesPerCluster = bs.bytesPerSector * bs.sectorsPerCluster;

    return 0;
}

/* ------------------- Root Directory Listeleme ------------------- */
void fat32_list_root() {
    uint8_t buf[4096];
    uint32_t cluster = bs.rootCluster;

    do {
        for (uint8_t s = 0; s < bs.sectorsPerCluster; s++) {
            ramdisk_read(&fdisk, cluster_to_lba(cluster) + s, buf, 1);
            FAT32_DirEntry* entry = (FAT32_DirEntry*)buf;

            for (uint32_t i = 0; i < bs.bytesPerSector / sizeof(FAT32_DirEntry); i++) {
                if (entry[i].name[0] == 0x00) return; // boş entry
                if ((entry[i].attr & 0x0F) == 0x0F) continue; // uzun dosya adı
                if (entry[i].name[0] == 0xE5) continue; // silinmiş

                char name[12];
                ncopy(name, entry[i].name, 11);
                name[11] = '\0';
                // Burada 8.3 formatını gerçek isme çevirmelisin

                // Basit çıktı
                // (kendi printk ya da log fonksiyonunu koy)
                printf("File: %s Size: %d\n", name, entry[i].fileSize);
            }
        }
        cluster = get_fat_entry(cluster);
    } while (cluster < 0x0FFFFFF8);
}