#ifndef _GUESTSYNC_DEAMON_H_
#define _GUESTSYNC_DEAMON_H_

#define GUESTSYNC_CTRL_INTERFACE        "/var/guestsyncd_if"
#define GUESTSYNC_CTRL_INTERFACE_IF     "if0"

/* #include <sap_ostypes.h> */
#include <common.h>

struct guestsyncd_data {
    int ctrl_sock;
};

struct guestsyncdCmdCB {
    const char *cmdName;
    int (*handler)(char *cmd);
    const char *description;
};

struct guestsyncdCmd {
    const char* cmdName;
    int (*handler)(char *cmd, char *reply);
    struct guestsyncdCmdCB *subCmd;
    const char* description;
};

typedef struct _guestsyncd_interfaces {
    int count;
    struct guestsyncd_data **syscd;
}guestsyncd_interfaces;

#endif /* _GUESTSYNC_DEAMON_H_ */
