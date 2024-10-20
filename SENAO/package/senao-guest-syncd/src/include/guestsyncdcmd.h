#ifndef _GUESTSYNCD_CMD_H_
#define _GUESTSYNCD_CMD_H_

int guestsyncdCmdHandle(char* cmd, char* reply);
void sync_data_delete_cb(void *eloop_ctx, void *timeout_ctx);

#endif /* _GUESTSYNCD_CMD_H_ */
