#include "fs/fat32.h"

// ! This file system reader development incomplete

#include "kernel.h"

#define FAT32_CLUSTYPE_FREE         0x00000000
#define FAT32_CLUSTYPE_RSVD         0x0FFFFFF0
#define FAT32_CLUSTYPE_BAD          0x0FFFFFF7
#define FAT32_CLUSTYPE_EOF_MIN      0x0FFFFFF8
#define FAT32_CLUSTYPE_EOF_MAX      0x0FFFFFFF

#define FAT32_FAT1_CLEANBIT         0x08000000
#define FAT32_FAT1_HWERRBIT         0x04000000

#define FAT32_DIRENT_DELETED        0xE5
#define FAT32_DIRENT_EMPTY          0x00
#define FAT32_DIRENT_HASKANJI       0x05

#define FAT32_ATTR_READ_ONLY        0x01
#define FAT32_ATTR_HIDDEN           0x02
#define FAT32_ATTR_SYSTEM           0x04
#define FAT32_ATTR_VOLUME_ID        0x08
#define FAT32_ATTR_DIRECTORY        0x10
#define FAT32_ATTR_ARCHIVE          0x20
#define FAT32_ATTR_LONGNAME         0x0F

#define FAT32_MAX_LFNLEN            256

ramdisk_t fat32_Disk;

typedef struct {
    uint8_t     jmpBoot[3];
    uint8_t     OEMName[8];
    uint16_t    bytesPerSector;
    uint8_t     sectorsPerCluster;
    uint16_t    reservedSectorCount;
    uint8_t     numFATs;
    uint16_t    rootEntryCount;
    uint16_t    totalSectors16;
    uint8_t     media;
    uint16_t    FATSize16;
    uint16_t    sectorsPerTrack;
    uint16_t    numHeads;
    uint32_t    hiddenSectors;
    uint32_t    totalSectors32;

    uint32_t    FATSize32;
    uint16_t    extFlags;
    uint16_t    FSVersion;
    uint32_t    rootCluster;
    uint16_t    FSInfo;
    uint16_t    backupBootSector;
    uint8_t     reserved[12];
    uint8_t     driveNumber;
    uint8_t     reserved1;
    uint8_t     bootSignature;
    uint32_t    volumeID;
    uint8_t     volumeLabel[11];
    uint8_t     fileSystemType[8];
} PACKED fat32_Bootsec_t;

typedef struct {
    uint8_t     name[11];
    uint8_t     attr;
    uint8_t     NTReserved;
    uint8_t     creationTimeTenths;
    uint16_t    creationTime;
    uint16_t    creationDate;
    uint16_t    lastAccessDate;
    uint16_t    firstClusterHigh;
    uint16_t    writeTime;
    uint16_t    writeDate;
    uint16_t    firstClusterLow;
    uint32_t    fileSize;
} PACKED fat32_Dirent_t;

typedef struct {
    uint8_t     order;
    uint16_t    name1[5];
    uint8_t     attr;
    uint8_t     type;
    uint8_t     checksum;
    uint16_t    name2[6];
    uint16_t    firstClusterLow;
    uint16_t    name3[2];
} PACKED fat32_LongDirent_t;

fat32_Bootsec_t fat32_Bootsec;
uint32_t        fat32_LBAStart;
uint32_t        fat32_DataSecLBA;
uint32_t        fat32_DataSecCnt;
uint32_t        fat32_DataClucCnt;

uint8_t* fat32_SecBuf;
char* fat32_LFNBuf;
char* fat32_FNameBuf;

static uint32_t fat32_getClusLBA(uint32_t clus)
    { return fat32_DataSecLBA + ((clus - 2) * fat32_Bootsec.sectorsPerCluster); }

static uint32_t fat32_getFATEntAddr(uint32_t clus)
    { return fat32_LBAStart + (clus * 4); }

static uint32_t fat32_getNextClusNum(uint32_t clus, uint8_t* sec) {
    uint32_t ofs = fat32_getFATEntAddr(clus) % fat32_Bootsec.bytesPerSector;
    uint32_t val = *((uint32_t*)&sec[ofs]); return val & 0x0FFFFFFF;
}

static uint8_t fat32_checksum (uint8_t *str) {
    uint16_t len; uint8_t sum = 0; for (len=11; len!=0; len--) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *str++;
    } return (sum);
}

int fat32_loadTree(uint32_t clus, int lvl) {
    uint32_t timeout = fat32_DataClucCnt;
    uint32_t cluster = clus;
    while (cluster < FAT32_CLUSTYPE_EOF_MIN && timeout > 0) { --timeout;
        uint8_t secbuf[fat32_Bootsec.sectorsPerCluster * fat32_Bootsec.bytesPerSector];
        for (uint32_t sec = 0; sec < fat32_Bootsec.sectorsPerCluster; ++sec) {
            if (ramdisk_read(&fat32_Disk,
                fat32_getClusLBA(cluster) + sec,
                secbuf + (sec * fat32_Bootsec.bytesPerSector), 1)
            != 0) { ERR("Unable to read sector from disk"); return -1; }
            char lfnbuf[FAT32_MAX_LFNLEN]; int lfnpos = 0;
            for (int ofs = 0; ofs < fat32_Bootsec.sectorsPerCluster * fat32_Bootsec.bytesPerSector; ++ofs) {
                fat32_Dirent_t* entry = (fat32_Dirent_t*)&secbuf[ofs];
                if (entry->name[0] == FAT32_DIRENT_EMPTY) { break; }
                if (entry->name[0] == FAT32_DIRENT_DELETED) { continue; }
                if ((entry->attr & FAT32_ATTR_LONGNAME) == FAT32_ATTR_LONGNAME) {
                    fat32_LongDirent_t* lfn = (fat32_LongDirent_t*)entry;
                    int i; for (i = 0; i < 5; ++i) {
                        if (lfn->name1[i] != 0xFFFF && lfn->name1[i] != 0x0000)
                            { lfnbuf[lfnpos++] = (char)lfn->name1[i]; }
                    } for (i = 0; i < 6; ++i) {
                        if (lfn->name2[i] != 0xFFFF && lfn->name2[i] != 0x0000)
                            { lfnbuf[lfnpos++] = (char)lfn->name2[i]; }
                    } for (i = 0; i < 2; ++i) {
                        if (lfn->name3[i] != 0xFFFF && lfn->name3[i] != 0x0000)
                            { lfnbuf[lfnpos++] = (char)lfn->name3[i]; }
                    } continue;
                }
                char filename[FAT32_MAX_LFNLEN];
                if (lfnpos > 0) { lfnbuf[lfnpos] = 0; copy(filename, lfnbuf); lfnpos = 0; }
                else { int i, j = 0;
                    for (i = 0; i < 8 && entry->name[i] != ' '; ++i) filename[j++] = entry->name[i];
                    if (entry->name[8] != ' ') { filename[j++] = '.';
                        for (i = 8; i < 11 && entry->name[i] != ' '; ++i) filename[j++] = entry->name[i];
                    } filename[j] = 0;
                }
                INFO("%s", filename);
            }
        } cluster = fat32_getNextClusNum(cluster, secbuf);
    } if (timeout == 0) { ERR("Cluster chain too long or corrupted"); return -1; }
}

int fat32_mount(uint32_t lba) {
    if (ramdisk_read(&fat32_Disk, lba, &fat32_Bootsec, 1) != 0)
        { ERR("Unable to read disk"); return -1; }
    fat32_LBAStart      =   lba + fat32_Bootsec.reservedSectorCount;
    fat32_DataSecLBA    =   fat32_LBAStart + (fat32_Bootsec.numFATs * fat32_Bootsec.FATSize32);
    fat32_DataSecCnt    =   fat32_Bootsec.totalSectors32 - (
                            fat32_Bootsec.reservedSectorCount +
                            (fat32_Bootsec.numFATs * fat32_Bootsec.FATSize32));
    fat32_DataClucCnt   =   fat32_DataSecCnt / fat32_Bootsec.sectorsPerCluster;
    if (fat32_DataClucCnt < 65525) { ERR("Volume format not supported or corrupted."); return -1; }
    fat32_SecBuf = (uint8_t*)malloc(fat32_Bootsec.sectorsPerCluster * fat32_Bootsec.bytesPerSector);
    fat32_LFNBuf = (char*)malloc(FAT32_MAX_LFNLEN); fat32_FNameBuf = (char*)malloc(FAT32_MAX_LFNLEN);
    if (!fat32_SecBuf || !fat32_LFNBuf || !fat32_FNameBuf)
        { ERR("Out of memory"); return -1; free(fat32_SecBuf); free(fat32_LFNBuf); free(fat32_FNameBuf); }
    if (fat32_loadTree(fat32_Bootsec.rootCluster, 0) != 0) { ERR("File system loading failed"); return -1; }
    INFO("Volume mounted");
}