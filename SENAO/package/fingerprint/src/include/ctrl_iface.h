#ifndef CTRL_IFACE_H
#define CTRL_IFACE_H

#include <fingerd.h>

int fingersyncd_fingerprint_init(struct fingersyncd_data *syscd);
void fingersyncd_fingerprint_deinit(struct fingersyncd_data *syscd);
int fingerprint_init(struct fingersyncd_data *syscd);
void fingerprint_deinit(struct fingersyncd_data *syscd);
int fingersyncd_ctrl_data_init(struct fingersyncd_data *syscd);
void fingersyncd_ctrl_data_deinit(struct fingersyncd_data *syscd);
int fingersyncd_cli_init(struct fingersyncd_data *syscd);
void fingersyncd_cli_deinit(struct fingersyncd_data *syscd);

#endif /* CTRL_IFACE_H */
