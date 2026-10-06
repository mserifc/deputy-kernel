
#include "deputy.h"

#define BUFFERLIMIT 64

typedef struct {
    char name[64];
    int id;
    int x, y;
    int width, height;
    bool fullsc;
    int owner_pid;
} Window_t;

typedef struct {
    int id;
    int x, y;
    int width, height;
} MaxedStore_t;

typedef struct {
    uint32_t bg;//           "#1c1c1c",
    uint32_t titleActive;//  "#e85a2a",
    uint32_t titleInactive;//"#2e2e2e",
    uint32_t windowBg;//     "#252525",
    uint32_t text;//         "#e8e0d5"
} Theme_t;

// Amber
Theme_t THEME = {
    0xFF1c1c1c,
    0xFFe85a2a,
    0xFF2e2e2e,
    0xFF252525,
    0xFFe8e0d5
};

// Nightsky
/*Theme_t THEME = {
    0xFF040409,
    0xFF2f4eae,
    0xFF061558,
    0xFF050e39,
    0xFFe8e0d5
};*/

typedef struct {
    int titlebarHeight;
    int shadowOffset;
    int buttonWidth;
    int titlePadding;
    int fontSize;
    int resizeHandle;

    int taskbarHeight;
    int taskbarPadding;
    int taskbarIconSize;
    int taskbarIconSpacing;
    int taskbarBottomGap;
    int taskbarSlantWidth;
    int taskbarSlantHeight;

    int windowMinW;
    int windowMinH;
} Config_t;

Config_t CONFIG = {
    .titlebarHeight   = 32,
    .shadowOffset     = 2,
    .buttonWidth      = 48,
    .titlePadding     = 8,
    .fontSize         = 14,
    .resizeHandle     = 16,

    .taskbarHeight    = 64,
    .taskbarPadding   = 16,
    .taskbarIconSize  = 64,
    .taskbarIconSpacing = 72,
    .taskbarBottomGap = 4,
    .taskbarSlantWidth  = 20,
    .taskbarSlantHeight = 30,

    .windowMinW = 8  + (48*3),   // titlePadding + buttonWidth*3
    .windowMinH = 32 + 60,       // titlebarHeight + 60
};

#define REGTYPE_REGISTER 1
#define REGTYPE_UNREGISTER 2
#define WINDOW_NAME_LENLIMIT 32
typedef struct {
    int type;
    int pid;
    char name[WINDOW_NAME_LENLIMIT];
    int width, height;
} RegisterPacked_t;

typedef struct { int x, y; } Offset_t;

int d_width, d_height;

Window_t WindowPool[BUFFERLIMIT]; int WindowPool_Cnt = 0;
int LivingIDs[BUFFERLIMIT]; int LivingIDs_Cnt = 0;
Window_t HiddenPool[BUFFERLIMIT]; int HiddenPool_Cnt = 0;
MaxedStore_t MaximizedStores[BUFFERLIMIT]; int MaximizedStores_Cnt = 0;

Offset_t SelWinDragOffset = {0, 0};

int WindowCounter = 0;
int SelectedWindow = -1;
int ClosingWindow = -1;
int ResizingWindow = -1;
int HiddingWindow = -1;
int MaxingWindow = -1;
int OwnerPIDSelector = -1;

bool dirty_state = true;
int cursor_x, cursor_y;
#define CLICK_STS_NOACT 0
#define CLICK_STS_CLICKED 1
#define CLICK_STS_RELEASED 2
int click_sts = 0;
bool onclick = false;
char event_cache[1024][3];
RegisterPacked_t regqueue_cache[80];
#define REGISTEREDPIDS_LIMIT 256
typedef struct { int pid, id; } RegisteredPID_t;
RegisteredPID_t registeredpids_Pool[REGISTEREDPIDS_LIMIT];
int registeredpids_Cnt = 0;
char path_cache[1024];

int pushelement(void* elm, void* queue, int* cnt, size_t elmsize, int limit) {
    if (*cnt >= limit) return -1;
    void* target = (void*)((size_t)queue + (*cnt * elmsize));
    memcpy(target, elm, elmsize);
    return (*cnt)++;
}

int popelement(int idx, void* queue, int* cnt, size_t elmsize) {
    if (idx < 0 || idx >= *cnt) return -1;
    for (int i = idx; i < *cnt - 1; i++) {
        void* dst = (void*)((size_t)queue + (i * elmsize));
        void* src = (void*)((size_t)queue + ((i+1) * elmsize));
        memcpy(dst, src, elmsize);
    }
    return (*cnt)--;
}

int create_window(char* _n, int _w, int _h) {
    if (OwnerPIDSelector == -1) { return -1; }
    dirty_state = true;
    Offset_t pos = {4, 4};
    switch (WindowCounter % 5) {
        case 0: {
            pos.x = (d_width/2) - (_w/2);
            pos.y = (d_height/2) - (_h/2);
            break;
        }
        case 2: {
            pos.x = d_width - _w - 4;
            break;
        }
        case 3: {
            pos.y = d_height - _h - 4;
            break;
        }
        case 4: {
            pos.x = d_width - _w - 4;
            pos.y = d_height - _h - 4;
            break;
        }
    }
    Window_t win;
    if (_n) strcpy(win.name, _n);
    else win.name[0] = '\0';
    win.id = WindowCounter++;
    win.x = pos.x; win.y = pos.y;
    win.width = _w; win.height = _h;
    win.fullsc = false;
    win.owner_pid = OwnerPIDSelector;
    pushelement(&win, WindowPool, &WindowPool_Cnt, sizeof(Window_t), BUFFERLIMIT);
    pushelement(&win.id, LivingIDs, &LivingIDs_Cnt, sizeof(int), BUFFERLIMIT);
    return WindowCounter-1;
}

int get_windowIndex(int _id) {
    for (int i = 0; i < WindowPool_Cnt; ++i) {
        if (WindowPool[i].id == _id) {
            return i;
        }
    } return -1;
}

int get_hiddenWindowIndex(int _id) {
    for (int i = 0; i< HiddenPool_Cnt; ++i) {
        if (HiddenPool[i].id == _id) {
            return i;
        }
    } return -1;
}

void focus_window(int _id) {
    dirty_state = true;
    int index = get_windowIndex(_id);
    if (index == -1) { printf("wm: Window %d not found", _id); return; }
    if (WindowPool_Cnt-1 == index) return;
    /*let stored = {
        id: WindowPool[index].id,
        name: WindowPool[index].name,
        x: WindowPool[index].x,
        y: WindowPool[index].y,
        width: WindowPool[index].width,
        height: WindowPool[index].height,
        fullsc: WindowPool[index].fullsc
    };*/
    Window_t stored;
    memcpy(&stored, &WindowPool[index], sizeof(Window_t));
    for (int i = index; i < WindowPool_Cnt-1; ++i) {
        /*WindowPool[i] = {
            id: WindowPool[i+1].id,
            name: WindowPool[i+1].name,
            x: WindowPool[i+1].x,
            y: WindowPool[i+1].y,
            width: WindowPool[i+1].width,
            height: WindowPool[i+1].height,
            fullsc: WindowPool[i+1].fullsc
        };*/
        memcpy(&WindowPool[i], &WindowPool[i+1], sizeof(Window_t));
    }
    /*WindowPool[WindowPool.length-1] = {
        id: stored.id,
        name: stored.name,
        x: stored.x,
        y: stored.y,
        width: stored.width,
        height: stored.height,
        fullsc: stored.fullsc
    };*/
    memcpy(&WindowPool[WindowPool_Cnt-1], &stored, sizeof(Window_t));
}

void close_window(int _id) {
    dirty_state = true;
    int index = get_windowIndex(_id);
    if (index == -1) { printf("wm: Window %d not found", _id); return; }
    for (int i = 0; i < LivingIDs_Cnt; ++i) {
        if (LivingIDs[i] == _id) {
            popelement(i, LivingIDs, &LivingIDs_Cnt, sizeof(int));
            break;
        }
    } popelement(index, WindowPool, &WindowPool_Cnt, sizeof(Window_t));
}

void hide_window(int _id) {
    dirty_state = true;
    int index = get_windowIndex(_id);
    if (index == -1) { printf("wm: Window %d not found", _id); return; }
    /*HiddenPool.push({
        id: WindowPool[index].id,
        name: WindowPool[index].name,
        x: WindowPool[index].x,
        y: WindowPool[index].y,
        width: WindowPool[index].width,
        height: WindowPool[index].height,
        fullsc: WindowPool[index].fullsc
    });*/
    Window_t win;
    memcpy(&win, &WindowPool[index], sizeof(Window_t));
    //pushelement(&win, WindowPool, &WindowPool_Cnt, sizeof(Window_t), BUFFERLIMIT);
    pushelement(&win, HiddenPool, &HiddenPool_Cnt, sizeof(Window_t), BUFFERLIMIT);
    popelement(index, WindowPool, &WindowPool_Cnt, sizeof(Window_t));
}

void show_window(int _id) {
    dirty_state = true;
    int index = get_hiddenWindowIndex(_id);
    if (index == -1) { printf("wm: Window %d not found", _id); return; }
    /*WindowPool.push({
        id: HiddenPool[index].id,
        name: HiddenPool[index].name,
        x: HiddenPool[index].x,
        y: HiddenPool[index].y,
        width: HiddenPool[index].width,
        height: HiddenPool[index].height,
        fullsc: HiddenPool[index].fullsc
    });*/
    pushelement(&HiddenPool[index], WindowPool, &WindowPool_Cnt, sizeof(Window_t), BUFFERLIMIT);
    popelement(index, HiddenPool, &HiddenPool_Cnt, sizeof(Window_t));
}

void maxim_window(int _id) {
    dirty_state = true;
    int index = get_windowIndex(_id);
    if (index == -1) { printf("wm: Window %d not found", _id); return; }
    if (!WindowPool[index].fullsc) {
        /*MaximizedStores.push({
            id: _id,
            x: WindowPool[index].x,
            y: WindowPool[index].y,
            width: WindowPool[index].width,
            height: WindowPool[index].height
        });*/
        MaxedStore_t store;
        store.id = _id;
        store.x = WindowPool[index].x;
        store.y = WindowPool[index].y;
        store.width = WindowPool[index].width;
        store.height = WindowPool[index].height;
        int idx = pushelement(&store, MaximizedStores, &MaximizedStores_Cnt, sizeof(MaxedStore_t), BUFFERLIMIT);
        MaximizedStores[idx].id = _id;
        WindowPool[index].x = 0;
        WindowPool[index].y = 0;
        WindowPool[index].width = d_width;
        WindowPool[index].height = d_height;
        WindowPool[index].fullsc = true;
    } else {
        int found = -1;
        for (int i = 0; i < MaximizedStores_Cnt; ++i) {
            if (MaximizedStores[i].id == _id) {
                found = i; break;
            }
        } if (found != -1) {
            WindowPool[index].x = MaximizedStores[found].x;
            WindowPool[index].y = MaximizedStores[found].y;
            WindowPool[index].width = MaximizedStores[found].width;
            WindowPool[index].height = MaximizedStores[found].height;
            /*for (int i = found; i < MaximizedStores_Cnt-1; ++i) {
                MaximizedStores[i] = MaximizedStores[i+1];
            } MaximizedStores.pop();*/
            popelement(found, MaximizedStores, &MaximizedStores_Cnt, sizeof(MaxedStore_t));
        } WindowPool[index].fullsc = false;
    }
}

RawImageInfo_t* wallpaper_img = NULL;
bool input_error = false;
void update_framebuffer() {
    //if (!dirty_state) { return; } else {}
    if (!wallpaper_img) {
        wallpaper_img = loadImage("/system/assets/wallpaper.pam");
    }
    if (wallpaper_img) {
        int x = (d_width  - wallpaper_img->width)  / 2;
        int y = (d_height - wallpaper_img->height) / 2;
        uint32_t* pixels = (uint32_t*)(wallpaper_img + 1);
        drawImage(pixels, x, y, wallpaper_img->width, wallpaper_img->height);
    } else {
        drawRect(0, 0, d_width, d_height, THEME.bg);printf("ERROR\n");
    }
    for (int i = 0; i < WindowPool_Cnt; ++i) {
        Window_t* w = &WindowPool[i];
        //drawRect(w->x+CONFIG.shadowOffset, w->y+CONFIG.shadowOffset, w->width, w->height, 0xFF000000);
        drawRect(w->x+CONFIG.shadowOffset, w->y+w->height, w->width, CONFIG.shadowOffset, 0xFF000000);
        drawRect(w->x+w->width, w->y+CONFIG.shadowOffset, CONFIG.shadowOffset, w->height, 0xFF000000);
        drawRect(w->x, w->y+CONFIG.titlebarHeight, w->width, w->height-CONFIG.titlebarHeight, THEME.windowBg);
        drawRect(w->x, w->y, w->width, CONFIG.titlebarHeight, (i == WindowPool_Cnt-1 ? THEME.titleActive : THEME.titleInactive));
        if (ClosingWindow == w->id) {
            drawRect(w->x + w->width - (CONFIG.buttonWidth*1), w->y, CONFIG.buttonWidth, CONFIG.titlebarHeight, 0xFFFF0000);
        } else if (MaxingWindow == w->id) {
            drawRect(w->x + w->width - (CONFIG.buttonWidth*2), w->y, CONFIG.buttonWidth, CONFIG.titlebarHeight, 0xFF00FF00);
        } else if (HiddingWindow == w->id) {
            drawRect(w->x + w->width - (CONFIG.buttonWidth*3), w->y, CONFIG.buttonWidth, CONFIG.titlebarHeight, 0xFFFFFF00);
        }
        drawText(w->x+CONFIG.titlePadding, w->y+((CONFIG.titlebarHeight/2) - (CONFIG.fontSize/2)), w->name, strlen(w->name), CONFIG.fontSize, THEME.text);
        drawText(w->x+w->width-(CONFIG.buttonWidth*1)+(CONFIG.buttonWidth/2)-((CONFIG.fontSize+(CONFIG.fontSize/2))/4), w->y+((CONFIG.titlebarHeight/2) - (CONFIG.fontSize-(CONFIG.fontSize/6))), "x", 1, CONFIG.fontSize+(CONFIG.fontSize/2), THEME.text);
        drawText(w->x+w->width-(CONFIG.buttonWidth*2)+(CONFIG.buttonWidth/2)-((CONFIG.fontSize+(CONFIG.fontSize/2))/4), w->y+((CONFIG.titlebarHeight/2) - (CONFIG.fontSize-(CONFIG.fontSize/6)-(CONFIG.fontSize/6))), "+", 1, CONFIG.fontSize+(CONFIG.fontSize/2), THEME.text);
        drawText(w->x+w->width-(CONFIG.buttonWidth*3)+(CONFIG.buttonWidth/2)-((CONFIG.fontSize+(CONFIG.fontSize/2))/4), w->y+((CONFIG.titlebarHeight/2) - (CONFIG.fontSize-(CONFIG.fontSize/6)-(CONFIG.fontSize/6))), "-", 1, CONFIG.fontSize+(CONFIG.fontSize/2), THEME.text);
        if (i == WindowPool_Cnt-1) {
            drawLine(w->x + w->width, w->y + w->height-(CONFIG.resizeHandle/2), w->x + w->width-(CONFIG.resizeHandle/2), w->y + w->height, 0xFF000000);
            drawLine(w->x + w->width, w->y + w->height-(CONFIG.resizeHandle/4), w->x + w->width-(CONFIG.resizeHandle/4), w->y + w->height, 0xFF000000);
        }
    }
    drawTri(CONFIG.taskbarSlantWidth, d_height-CONFIG.taskbarSlantHeight, 0, d_height, d_width-CONFIG.taskbarSlantWidth, d_height-CONFIG.taskbarSlantHeight, THEME.titleInactive);
    drawTri(d_width-CONFIG.taskbarSlantWidth, d_height-CONFIG.taskbarSlantHeight, 0, d_height, d_width, d_height, THEME.titleInactive);
    for (int i = 0; i < LivingIDs_Cnt; ++i) {
        drawRect(CONFIG.taskbarPadding+(i*CONFIG.taskbarIconSpacing), d_height-CONFIG.taskbarIconSize-CONFIG.taskbarBottomGap, CONFIG.taskbarIconSize, CONFIG.taskbarIconSize, THEME.titleActive);
    }
    if (input_error) {
        drawRect(0, d_height/2-64, d_width, 128, 0xFF000);
        char* msg = "Input error";
        drawText(40, d_height/2-32, msg, strlen(msg), 64, 0xFFFFFFFF);
    }
    /*drawLine(cursor_x-5, cursor_y-5, cursor_x+5, cursor_y+5, 0xFFFFFFFF);
    drawLine(cursor_x+5, cursor_y-5, cursor_x-5, cursor_y+5, 0xFFFFFFFF);*/
    /*if (!img) {
        img = loadImage("/system/assets/cursor2.pam");
    }
    if (img) {
        uint32_t* pixels = (uint32_t*)(img + 1);
        drawImage(pixels, 0, 0, img->width, img->height);
    } else {
        printf("ERROR\n");
    }*/
    drawFlush(); dirty_state = false;
}

int mouse_fd = -1;
int eventlistener() {
    int result = true;
    if (mouse_fd == -1) {
        mouse_fd = syscall(SYS_OPEN, "/dev/mouse", O_RDONLY);
        if (mouse_fd == -1) { puts("wm: Mouse open error\n"); result=-1; goto end; }
    }
    int sts = syscall(SYS_READ, mouse_fd, event_cache, sizeof(event_cache));
    if (sts == -1) { puts("wm: Mouse read error\n"); result=-1; goto end; }
    if (!sts) { result=false; goto end; }
    for (int i = 0; i < sts/3; ++i) {
        char btnsts = event_cache[i][0];
        cursor_x += event_cache[i][1];
        cursor_y += event_cache[i][2];
        //dirty_state = true;
        if (cursor_x >= d_width) { cursor_x = d_width-1; }
        else if (cursor_x < 0) { cursor_x = 0; }
        if (cursor_y >= d_height) { cursor_y = d_height-1; }
        else if (cursor_y < 0) { cursor_y = 0; }
        if (btnsts & MOUSE_LEFTBTN && !onclick) {
            onclick = true;
            click_sts = CLICK_STS_CLICKED;
        } else if (!(btnsts & MOUSE_LEFTBTN) && onclick) {
            onclick = false;
            click_sts = CLICK_STS_RELEASED;
        } else { result = false; }
        Offset_t pos = {cursor_x, cursor_y};
        if (click_sts == CLICK_STS_CLICKED) { click_sts = CLICK_STS_NOACT;
    if (d_height-CONFIG.taskbarIconSize-CONFIG.taskbarBottomGap <= cursor_y) { goto end; }
    for (int i = WindowPool_Cnt-1; i >= 0; --i) {
        Window_t* w = &WindowPool[i];
        if (
            w->x <= pos.x &&
            w->x + w->width > pos.x &&
            w->y <= pos.y &&
            w->y + w->height > pos.y
        ) {
            focus_window(w->id);
            if (
                w->x+w->width-CONFIG.resizeHandle <= pos.x &&
                w->x+w->width > pos.x &&
                w->y+w->height-CONFIG.resizeHandle <= pos.y &&
                w->y+w->height > pos.y &&
                !w->fullsc
            ) {
                ResizingWindow = w->id;
            } else if (
                w->x+w->width-(CONFIG.buttonWidth*1) <= pos.x &&
                w->x+w->width-(CONFIG.buttonWidth*0) > pos.x &&
                w->y <= pos.y &&
                w->y+CONFIG.titlebarHeight > pos.y
            ) {
                ClosingWindow = w->id;
            } else if (
                w->x+w->width-(CONFIG.buttonWidth*2) <= pos.x &&
                w->x+w->width-(CONFIG.buttonWidth*1) > pos.x &&
                w->y <= pos.y &&
                w->y+CONFIG.titlebarHeight > pos.y
            ) {
                MaxingWindow = w->id;
            } else if (
                w->x+w->width-(CONFIG.buttonWidth*3) <= pos.x &&
                w->x+w->width-(CONFIG.buttonWidth*2) > pos.x &&
                w->y <= pos.y &&
                w->y+CONFIG.titlebarHeight > pos.y
            ) {
                HiddingWindow = w->id;
            } else if (
                w->x <= pos.x &&
                w->x + w->width > pos.x &&
                w->y <= pos.y &&
                w->y + CONFIG.titlebarHeight > pos.y &&
                !w->fullsc
            ) {
                SelWinDragOffset.x = pos.x - w->x;
                SelWinDragOffset.y = pos.y - w->y;
                SelectedWindow = w->id;
            } else { SelectedWindow = -1; }
            goto end;
        }
    } SelectedWindow = -1;
        } else if (click_sts == CLICK_STS_RELEASED) { click_sts = CLICK_STS_NOACT;
    if (ClosingWindow != -1) {
        int idx = get_windowIndex(ClosingWindow);
        if (idx == -1) { printf("Window %d not found\n", ClosingWindow); }
        else {
            Window_t* w = &WindowPool[idx];
            if (
                w->x+w->width-(CONFIG.buttonWidth*1) <= pos.x &&
                w->x+w->width-(CONFIG.buttonWidth*0) > pos.x &&
                w->y <= pos.y &&
                w->y+CONFIG.titlebarHeight > pos.y
            ) {
                close_window(ClosingWindow);
            }
        }
        ClosingWindow = -1;
    } else if (MaxingWindow != -1) {
        int idx = get_windowIndex(MaxingWindow);
        if (idx == -1) { printf("Window %d not found\n", MaxingWindow); }
        else {
            Window_t* w = &WindowPool[idx];
            if (
                w->x+w->width-(CONFIG.buttonWidth*2) <= pos.x &&
                w->x+w->width-(CONFIG.buttonWidth*1) > pos.x &&
                w->y <= pos.y &&
                w->y+CONFIG.titlebarHeight > pos.y
            ) {
                maxim_window(MaxingWindow);
            }
        }
        MaxingWindow = -1;
    } else if (HiddingWindow != -1) {
        int idx = get_windowIndex(HiddingWindow);
        if (idx == -1) { printf("Window %d not found\n", HiddingWindow); }
        else {
            Window_t* w = &WindowPool[idx];
            if (
                w->x+w->width-(CONFIG.buttonWidth*3) <= pos.x &&
                w->x+w->width-(CONFIG.buttonWidth*2) > pos.x &&
                w->y <= pos.y &&
                w->y+CONFIG.titlebarHeight > pos.y
            ) {
                hide_window(HiddingWindow);
            }
        }
        HiddingWindow = -1;
    } else {
        for (int i = 0; i < LivingIDs_Cnt; ++i) {
            if (
                CONFIG.taskbarPadding+(i*CONFIG.taskbarIconSpacing) <= pos.x &&
                d_height-CONFIG.taskbarIconSize-CONFIG.taskbarBottomGap <= pos.y &&
                (CONFIG.taskbarPadding+(i*CONFIG.taskbarIconSpacing))+CONFIG.taskbarIconSize > pos.x &&
                (d_height-CONFIG.taskbarIconSize-CONFIG.taskbarBottomGap)+CONFIG.taskbarIconSize > pos.y
            ) {
                int idx = get_hiddenWindowIndex(LivingIDs[i]);
                if (idx != -1) {
                    show_window(LivingIDs[i]);
                } else if (get_windowIndex(LivingIDs[i]) == WindowPool_Cnt-1) {
                    hide_window(LivingIDs[i]);
                } else {
                    focus_window(LivingIDs[i]);
                }
            }
        }
    }
    SelectedWindow = -1;
    ResizingWindow = -1;
        } else {
    if (ResizingWindow != -1) {
        result = true;
        Window_t* w = &WindowPool[get_windowIndex(ResizingWindow)];
        w->width = max(CONFIG.windowMinW+((CONFIG.fontSize/2)*strlen(w->name)), pos.x - w->x);
        w->height = max(CONFIG.windowMinH, pos.y - w->y);
    } else if (SelectedWindow != -1) {
        result = true;
        if (d_height-CONFIG.taskbarIconSize-CONFIG.taskbarBottomGap <= pos.y) { goto end; }
        Window_t* w = &WindowPool[get_windowIndex(SelectedWindow)];
        w->x = pos.x - SelWinDragOffset.x;
        w->y = pos.y - SelWinDragOffset.y;
    }
        }
    } end:
        return result;
}

int register_process(int pid, char* name, int w, int h) {
    for (int i = 0; i < registeredpids_Cnt; ++i) {
        if (registeredpids_Pool[i].pid == pid) {
            return 0;
        }
    }
    OwnerPIDSelector = pid;
    int id = create_window(name, w, h);
    OwnerPIDSelector = -1;
    if (id == -1) { return -1; }
    memset(path_cache, '\0', 1024);
    snprintf(path_cache, 1024, "/system/runtime/wm/pid_%d/", pid);
    int sts = syscall(SYS_MKDIR, path_cache);
    if (sts == -1) { close_window(id); return -1; }
    snprintf(path_cache, 1024, "/system/runtime/wm/pid_%d/cmd.queue", pid);
    sts = syscall(SYS_MKNOD, path_cache, S_IFIFO);
    if (sts == -1) {
        memset(path_cache, '\0', 1024);
        snprintf(path_cache, 1024, "/system/runtime/wm/pid_%d/", pid);
        syscall(SYS_RMDIR, path_cache);
        close_window(id);
        return -1;
    }
    snprintf(path_cache, 1024, "/system/runtime/wm/pid_%d/event.queue", pid);
    sts = syscall(SYS_MKNOD, path_cache, S_IFIFO);
    if (sts == -1) {
        memset(path_cache, '\0', 1024);
        snprintf(path_cache, 1024, "/system/runtime/wm/pid_%d/cmd.queue", pid);
        syscall(SYS_REMOVE, path_cache);
        snprintf(path_cache, 1024, "/system/runtime/wm/pid_%d/", pid);
        syscall(SYS_RMDIR, path_cache);
        close_window(id);
        return -1;
    }
    RegisteredPID_t regpid = {
        .pid = pid,
        .id = id
    };
    pushelement(&regpid, registeredpids_Pool, &registeredpids_Cnt, sizeof(RegisteredPID_t), REGISTEREDPIDS_LIMIT);
    printf("Register success! PID:%d\n", pid);
    return 0;
}
int unregister_process(int pid) {
    int idx = -1; for (int i = 0; i < registeredpids_Cnt; ++i) {
        if (registeredpids_Pool[i].pid == pid) {
            idx = i; break;
        }
    } if (idx == -1) { return 0; }
    memset(path_cache, '\0', 1024);
    snprintf(path_cache, 1024, "/system/runtime/wm/pid_%d/cmd.queue", pid);
    syscall(SYS_REMOVE, path_cache);
    snprintf(path_cache, 1024, "/system/runtime/wm/pid_%d/event.queue", pid);
    syscall(SYS_REMOVE, path_cache);
    memset(path_cache, '\0', 1024);
    snprintf(path_cache, 1024, "/system/runtime/wm/pid_%d/", pid);
    syscall(SYS_RMDIR, path_cache);
    focus_window(registeredpids_Pool[idx].id);
    close_window(registeredpids_Pool[idx].id);
    popelement(idx, registeredpids_Pool, &registeredpids_Cnt, sizeof(RegisteredPID_t));
    return 0;
}

int regqueue_fd = -1;
int main() {
    {
        display_Info_t dinfo;
        int fd = syscall(SYS_OPEN, "/dev/display.info", O_RDONLY);
        if (fd == -1) { puts("wm: Display info open error\n"); return 1; }
        int sts = syscall(SYS_READ, fd, &dinfo, sizeof(dinfo));
        if (sts == -1) { puts("wm: Display info read error\n"); return 1; }
        printf("Width\tHeight\tPitch\tDepth\n");
        printf("%d\t%d\t%d\t%d-bits\n", dinfo.width, dinfo.height, dinfo.pitch, dinfo.depth);
        d_width = dinfo.width;
        d_height = dinfo.height;
        syscall(SYS_CLOSE, fd);
    }
    {
        cursor_x = d_width/2;
        cursor_y = d_height/2;
    }
    
    int sts = syscall(SYS_MKDIR, "/system/runtime/wm/");
    if (sts == -1) { puts("wm: Unable to create directory\n"); return 1; }
    sts = syscall(SYS_MKNOD, "/system/runtime/wm/register.queue", S_IFIFO);
    if (sts == -1) { puts("wm: Unable to create register queue\n"); return 1; }
    
    
    
    // ! Test sequences start
    
    //char* msg = "[\\Hello, world!]";
    //drawText(100, 100, msg, strlen(msg), 16, 0xFFFFFFFF);
    /*create_window("Example Program", 640, 360);
    create_window("Example Application", 800, 500);
    create_window("Example Game", 960, 540);
    create_window("Example Window", 720, 450);*/
    /*create_window("Example Window", 320, 240);
    create_window("Example Program", 320, 240);
    create_window("Example Application", 320, 240);*/
    RegisterPacked_t testpack;
    testpack.type = REGTYPE_REGISTER;
    testpack.pid = 10;
    strcpy(testpack.name, "Launcher");
    testpack.width = 320;
    testpack.height = 240;
    int testfd = syscall(SYS_OPEN, "/system/runtime/wm/register.queue", O_WRONLY);
    int teststs = syscall(SYS_WRITE, testfd, &testpack, sizeof(testpack));
    printf("wmtest: %d\n", teststs);
    syscall(SYS_CLOSE, testfd);
    
    syscall(SYS_EXEC, "/system/wintest");
    
    // ! Test sequences end
    
    
    
    update_framebuffer();
    while (1) {
        if (regqueue_fd == -1) {
            regqueue_fd = syscall(SYS_OPEN, "/system/runtime/wm/register.queue", O_RDONLY);
            if (regqueue_fd == -1) { puts("wm: Register queue not found\n"); return 1; }
        }
        int sts = syscall(SYS_READ, regqueue_fd, regqueue_cache, sizeof(regqueue_cache));
        if (sts == -1) { puts("wm: Unable to listen register queue\n"); }
        else if (sts) {
            for (int i = 0; i < sts/sizeof(RegisterPacked_t); ++i) {
                int type = regqueue_cache[i].type;
                int pid = regqueue_cache[i].pid;
                char* name = regqueue_cache[i].name;
                name[WINDOW_NAME_LENLIMIT-1] = '\0';
                int w = regqueue_cache[i].width;
                int h = regqueue_cache[i].height;
                if (type == REGTYPE_REGISTER) {
                    register_process(pid, name, w, h);
                } else if (type == REGTYPE_UNREGISTER) {
                    //unregister_process(pid);
                }
            }
        }
        // ----
        input_error = false;
        int refresh = eventlistener();
        /*if (refresh == -1) {
            dirty_state = true;
            input_error = true;
            update_framebuffer();
        } else if (refresh) {
            dirty_state = true;
        }*/ //else { yield(); }
        if (refresh || dirty_state) {
            update_framebuffer();
        }
        //update_framebuffer();
        yield();
    }
}
