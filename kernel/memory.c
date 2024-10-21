#include "kernel.h"

#define MemoryEntry 0x100000
#define MemorySize (1 * 1024 * 1024)
#define MemoryPageSize (16 * 1024)
#define MemoryPageCount (MemorySize / MemoryPageSize)

void* MemorySpace = (void*)MemoryEntry;

char MemoryUsageReport[100];

typedef struct {
    size_t number;
    bool allocted;
    void* entry;
} MemoryPage;

MemoryPage MemoryPageList[MemoryPageCount];

void memory_init() {
    for (size_t i = 0; i < MemoryPageCount; ++i) {
        MemoryPageList[i].number = i;
        MemoryPageList[i].allocted = false;
        MemoryPageList[i].entry = (void*)(MemoryEntry + (i * MemoryPageSize));
    }
    memset(MemorySpace, 0, MemorySize);
}

void* memory_allocate() {
    for (size_t i = 0; i < MemoryPageCount; ++i) {
        if (MemoryPageList[i].allocted == false) {
            MemoryPageList[i].allocted = true;
            return MemoryPageList[i].entry;
        }
    }
}

void memory_free(void* allocated) {
    if (allocated == null) { return; }
    size_t targetpagenumber = (((size_t)allocated) - MemoryEntry) / MemoryPageSize;
    MemoryPageList[targetpagenumber].allocted = false;
    memset(allocated, 0, (size_t)MemoryPageSize);
    allocated = null;
}

char* memory_report() {
    size_t usingblocks = 0;
    size_t usingsize = 0;
    size_t emptyblocks = 0;
    size_t emptysize = 0;
    for (size_t i = 0; i < MemoryPageCount; ++i) {
        if (MemoryPageList[i].allocted) {
            usingblocks++;
            usingsize+=MemoryPageSize;
        } else {
            emptyblocks++;
            emptysize+=MemoryPageSize;
        }
    }
    snprintf(MemoryUsageReport, 100,
        "%d blocks (%d byte) using, %d blocks (%d byte) free",
        usingblocks, usingsize, emptyblocks, emptysize);
    return MemoryUsageReport;
}

/*

#define MemoryPageSize (4 * 1024)
#define MemorySize (1 * 1024 * 1024)
#define MemoryPageCount (MemorySize / MemoryPageSize)

typedef struct {
    size_t number;
    bool allocated;
    char data[MemoryPageSize];
    MemoryPage* next;
} MemoryPage;

MemoryPage MemoryPageTable[MemoryPageCount];

void memory_init() {
    for (size_t i = 0; i < MemoryPageCount; ++i) {
        MemoryPageTable[i].number = i;
        MemoryPageTable[i].allocated = false;
        memset(MemoryPageTable[i].data, 0, MemoryPageSize);
    }
}

size_t memory_allocate(size_t size) {
    if (size = 0) { return null; }
    size_t RequestedPageCount;
    if (MemoryPageSize >= size) {
        RequestedPageCount = 1;
    } else {
        RequestedPageCount = size / MemoryPageSize;
        if (size % MemoryPageSize != 0) {
            RequestedPageCount++;
        }
    }
    size_t head;
    size_t counter = 0;
    MemoryPage* current = &MemoryPageTable[0];
    for (size_t i = 0; i < RequestedPageCount; ++i) {
        for (size_t j = 0; j < MemoryPageCount; ++j) {
            if (MemoryPageTable[j].allocated == false) {
                if (counter == 0) { head = j; }
                MemoryPageTable[j].allocated = true;
                memset(MemoryPageTable[j].data, 0, MemoryPageSize);
            }
        }
    }
    return MemoryPageTable[head].number;
    return null;
}

*/


/*

#include <stdio.h>

struct Node {
    int data;              // Veri alanı
    struct Node *next;     // Bir sonraki düğüme işaretçi
    struct Node *prev;     // Önceki düğüme işaretçi
};

int main() {
    // Statik olarak düğümleri tanımla
    struct Node node1, node2, node3;

    // Düğümlerin verilerini ata
    node1.data = 1;
    node2.data = 2;
    node3.data = 3;

    // Düğümleri birbirine bağla
    node1.next = &node2;  // node1'in bir sonraki düğümü node2
    node1.prev = NULL;     // node1, listenin başı, bu yüzden prev NULL

    node2.next = &node3;  // node2'nin bir sonraki düğümü node3
    node2.prev = &node1;  // node2'nin önceki düğümü node1

    node3.next = NULL;     // node3, listenin sonu, bu yüzden next NULL
    node3.prev = &node2;  // node3'nin önceki düğümü node2

    // Bağlantılı listeyi yazdır (ileri doğru)
    printf("İleri doğru: ");
    struct Node *current = &node1; // Başlangıçta ilk düğüm
    while (current != NULL) {
        printf("%d ", current->data); // Düğümün verisini yazdır
        current = current->next;       // Bir sonraki düğüme geç
    }
    printf("\n");

    // Bağlantılı listeyi yazdır (geri doğru)
    printf("Geri doğru: ");
    current = &node3; // Başlangıçta son düğüm
    while (current != NULL) {
        printf("%d ", current->data); // Düğümün verisini yazdır
        current = current->prev;       // Önceki düğüme geç
    }
    printf("\n");

    return 0;
}

*/