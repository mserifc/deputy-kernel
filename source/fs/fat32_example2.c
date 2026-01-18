#include "fs/fat32.h"

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


static uint32_t fat32_getClusLBA(uint32_t clus)
    { return fat32_DataSecLBA + ((clus - 2) * fat32_Bootsec.sectorsPerCluster); }

static uint32_t fat32_getFATEntAddr(uint32_t clus)
    { return fat32_LBAStart + (clus * 4); }

static uint32_t fat32_getNextClusNum(uint32_t clus, uint8_t* sec) {
    uint32_t off = fat32_getFATEntAddr(clus) % fat32_Bootsec.bytesPerSector;
    uint32_t val = *((uint32_t*)&sec[off]); return val & 0x0FFFFFFF;
}

static uint8_t fat32_checksum (uint8_t *str) {
    uint16_t len; uint8_t sum = 0; for (len=11; len!=0; len--) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *str++;
    } return (sum);
}


#define MAX_LFN_LEN 255

void parse_short_name(uint8_t name[11], char *out) {
    int i, j = 0;
    for (i = 0; i < 8 && name[i] != ' '; i++) out[j++] = name[i];
    if (name[8] != ' ') {
        out[j++] = '.';
        for (i = 8; i < 11 && name[i] != ' '; i++) out[j++] = name[i];
    }
    out[j] = 0;
}

static void lfn_entry_to_chars(fat32_LongDirent_t *lfn, char *buffer, int *pos) {
    int i;
    for (i = 0; i < 5; i++) if(lfn->name1[i] != 0xFFFF && lfn->name1[i] != 0x0000) buffer[(*pos)++] = (char)lfn->name1[i];
    for (i = 0; i < 6; i++) if(lfn->name2[i] != 0xFFFF && lfn->name2[i] != 0x0000) buffer[(*pos)++] = (char)lfn->name2[i];
    for (i = 0; i < 2; i++) if(lfn->name3[i] != 0xFFFF && lfn->name3[i] != 0x0000) buffer[(*pos)++] = (char)lfn->name3[i];
}

int fat32_read_file(fat32_Dirent_t *entry, uint8_t *buffer) {
    uint32_t cluster = ((uint32_t)entry->firstClusterHigh << 16) | entry->firstClusterLow;
    uint32_t remaining = entry->fileSize;
    uint8_t secBuff[fat32_Bootsec.sectorsPerCluster * fat32_Bootsec.bytesPerSector];

    while (cluster < FAT32_CLUSTYPE_EOF_MIN && remaining > 0) {
        for (uint32_t s = 0; s < fat32_Bootsec.sectorsPerCluster; s++) {
            if (ramdisk_read(&fat32_Disk, fat32_getClusLBA(cluster) + s,
                             secBuff + s * fat32_Bootsec.bytesPerSector, 1) != 0) {
                ERR("Unable to read cluster sector"); return -1;
            }
        }

        uint32_t copySize = remaining;
        if (copySize > sizeof(secBuff)) copySize = sizeof(secBuff);
        ncopy(buffer, secBuff, copySize);
        buffer += copySize;
        remaining -= copySize;

        cluster = fat32_getNextClusNum(cluster, secBuff);
    }
    return 0;
}


void fat32_list_dir_recursive(uint32_t startCluster, int level) {
    uint32_t cluster = startCluster;

    while (cluster < FAT32_CLUSTYPE_EOF_MIN) {
        uint8_t secBuff[fat32_Bootsec.sectorsPerCluster * fat32_Bootsec.bytesPerSector];

        for (uint32_t s = 0; s < fat32_Bootsec.sectorsPerCluster; s++) {
            if (ramdisk_read(&fat32_Disk, fat32_getClusLBA(cluster) + s, secBuff + s * fat32_Bootsec.bytesPerSector, 1) != 0) {
                ERR("Unable to read cluster sector"); return;
            }
        }

        char lfnBuffer[MAX_LFN_LEN + 1];
        int lfnPos = 0;

        for (int off = 0; off < fat32_Bootsec.sectorsPerCluster * fat32_Bootsec.bytesPerSector; off += sizeof(fat32_Dirent_t)) {
            fat32_Dirent_t *entry = (fat32_Dirent_t*)&secBuff[off];

            if (entry->name[0] == FAT32_DIRENT_EMPTY) break;
            if (entry->name[0] == FAT32_DIRENT_DELETED) continue;

            if ((entry->attr & FAT32_ATTR_LONGNAME) == FAT32_ATTR_LONGNAME) {
                fat32_LongDirent_t *lfn = (fat32_LongDirent_t*)entry;
                lfn_entry_to_chars(lfn, lfnBuffer, &lfnPos);
                continue;
            }

            char filename[256];
            if (lfnPos > 0) {
                lfnBuffer[lfnPos] = 0;
                copy(filename, lfnBuffer);
                lfnPos = 0;
            } else {
                parse_short_name(entry->name, filename);
            }

            for (int i = 0; i < level; i++) printf("  "); // indent
            if (entry->attr & FAT32_ATTR_DIRECTORY) {
                printf("[DIR]  %s\n", filename);
                if (compare(filename, ".") != 0 && compare(filename, "..") != 0) {
                    // recursive alt dizin
                    uint32_t childCluster = ((uint32_t)entry->firstClusterHigh << 16) | entry->firstClusterLow;
                    if (childCluster >= 2 && childCluster < FAT32_CLUSTYPE_EOF_MIN) {
                        fat32_list_dir_recursive(childCluster, level + 1);
                    }
                }
            } else {
                printf("[FILE] %s (%d bytes)\n", filename, entry->fileSize);
                uint8_t* buf = (uint8_t*)calloc(1, entry->fileSize);
                fat32_read_file(entry, buf);
                nputs((char*)buf, entry->fileSize);
            }
        }

        cluster = fat32_getNextClusNum(cluster, secBuff);
    }
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
    fat32_list_dir_recursive(fat32_Bootsec.rootCluster, 0);
}