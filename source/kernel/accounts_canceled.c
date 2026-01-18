#include "kernel.h"

#define ACCOUNTS_ACCTLIMIT 16
#define ACCOUNTS_NAMELIMIT 32
#define ACCOUNTS_HOMELIMIT 64

#define ACCOUNTS_DEF_NULL ((size_t)-1)
#define ACCOUNTS_DEF_ROOT 0
#define ACCOUNTS_DEF_DAEMON 1
#define ACCOUNTS_DEF_USERS 2
#define ACCOUNTS_DEF_NOBODY (ACCOUNTS_ACCTLIMIT - 1)

typedef struct {
    char name[ACCOUNTS_NAMELIMIT];
    int group[ACCOUNTS_ACCTLIMIT];
    char home[ACCOUNTS_HOMELIMIT];
    uint32_t pass; bool active;
} accounts_User_t;

typedef char accounts_Group_t[ACCOUNTS_NAMELIMIT];

bool accounts_InitLock = false;

accounts_User_t* accounts_UserV;

accounts_Group_t* accounts_GroupV;

int accounts_init() {
    if (accounts_InitLock) { return -1; }
    accounts_UserV = (accounts_User_t*)malloc(ACCOUNTS_ACCTLIMIT * sizeof(accounts_User_t));
    if (accounts_UserV == NULL) { kernel_panic("Out of memory"); }
    accounts_GroupV = (accounts_Group_t*)malloc(ACCOUNTS_ACCTLIMIT * sizeof(accounts_Group_t));
    if (accounts_GroupV == NULL) { kernel_panic("Out of memory"); }
    for (int i = 0; i < ACCOUNTS_ACCTLIMIT; ++i) {
        fill(accounts_UserV[i].name, 0, sizeof(accounts_UserV->name));
        fill(accounts_UserV[i].group, -1, sizeof(accounts_UserV->group));
        fill(accounts_UserV[i].home, 0, sizeof(accounts_UserV->home));
        accounts_UserV[i].pass = 0; accounts_UserV[i].active = false;
        fill(accounts_GroupV[i], 0, sizeof(accounts_Group_t));
    }
    // root
    copy(accounts_UserV[ACCOUNTS_DEF_ROOT].name, "root");
    copy(accounts_GroupV[ACCOUNTS_DEF_ROOT], "root");
    accounts_UserV[ACCOUNTS_DEF_ROOT].group[0] = ACCOUNTS_DEF_ROOT;
    copy(accounts_UserV[ACCOUNTS_DEF_ROOT].home, "/root/");
    accounts_UserV[ACCOUNTS_DEF_ROOT].pass = fnv1ahash("", 0);
    accounts_UserV[ACCOUNTS_DEF_ROOT].active = true;
    // daemon
    copy(accounts_UserV[ACCOUNTS_DEF_DAEMON].name, "daemon");
    copy(accounts_GroupV[ACCOUNTS_DEF_DAEMON], "daemon");
    accounts_UserV[ACCOUNTS_DEF_DAEMON].group[0] = ACCOUNTS_DEF_DAEMON;
    copy(accounts_UserV[ACCOUNTS_DEF_DAEMON].home, "");
    accounts_UserV[ACCOUNTS_DEF_DAEMON].pass = fnv1ahash("", 0);
    accounts_UserV[ACCOUNTS_DEF_DAEMON].active = true;
    // users
    copy(accounts_GroupV[ACCOUNTS_DEF_USERS], "users");
    // nobody & nogroup
    copy(accounts_UserV[ACCOUNTS_DEF_NOBODY].name, "nobody");
    accounts_UserV[ACCOUNTS_DEF_NOBODY].group[0] = ACCOUNTS_DEF_NOBODY;
    copy(accounts_GroupV[ACCOUNTS_DEF_NOBODY], "nogroup");
    copy(accounts_UserV[ACCOUNTS_DEF_NOBODY].home, "");
    accounts_UserV[ACCOUNTS_DEF_NOBODY].pass = fnv1ahash("", 0);
    accounts_UserV[ACCOUNTS_DEF_NOBODY].active = true;
    accounts_InitLock = true;
}