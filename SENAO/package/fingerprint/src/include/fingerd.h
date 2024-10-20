#ifndef _FINGERD_DEAMON_H_
#define _FINGERD_DEAMON_H_

#define FINGERD_CTRL_INTERFACE        "/tmp/fingerd_if"
#define FINGERD_CTRL_INTERFACE_IF     "if0"

/* #include <sap_ostypes.h> */
#include <common.h>

struct fingersyncd_data {
    int ctrl_sock;
};

struct fingersyncdCmdCB {
    const char *cmdName;
    int (*handler)(char *cmd);
    const char *description;
};

struct fingersyncdCmd {
    const char* cmdName;
    int (*handler)(char *cmd, char *reply);
    struct fingersyncdCmdCB *subCmd;
    const char* description;
};

typedef struct _fingersyncd_interfaces {
    int count;
    struct fingersyncd_data **syscd;
}fingersyncd_interfaces;

#endif /* _FINGERD_DEAMON_H_ */
