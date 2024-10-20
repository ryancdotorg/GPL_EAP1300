#ifndef CTRL_IFACE_H
#define CTRL_IFACE_H

#include <guestsyncd.h>

int guestsyncd_ctrl_data_init(struct guestsyncd_data *syscd);
void guestsyncd_ctrl_data_deinit(struct guestsyncd_data *syscd);
int guestsyncd_cli_init(struct guestsyncd_data *syscd);
void guestsyncd_cli_deinit(struct guestsyncd_data *syscd);

#endif /* CTRL_IFACE_H */
