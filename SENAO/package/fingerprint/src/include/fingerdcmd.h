#ifndef _FINGERD_CMD_H_
#define _FINGERD_CMD_H_

int fingersyncdCmdHandle(char* cmd, char* reply);
void sync_data_delete_cb(void *eloop_ctx, void *timeout_ctx);

#endif /* _FINGERD_CMD_H_ */
