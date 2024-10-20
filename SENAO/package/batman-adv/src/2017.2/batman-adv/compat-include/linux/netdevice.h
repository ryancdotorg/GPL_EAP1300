/* Copyright (C) 2007-2017  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_

#include <linux/version.h>
#include_next <linux/netdevice.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)

#include <linux/netdev_features.h>

#endif /* < KERNEL_VERSION(3, 3, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)

#define netdev_upper_dev_unlink(slave, master) netdev_set_master(slave, NULL)
#define netdev_master_upper_dev_get(dev) \
({\
	ASSERT_RTNL();\
	dev->master;\
})

#endif /* < KERNEL_VERSION(3, 9, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)

#define NETDEV_CHANGEUPPER	0x0015

#define netdev_notifier_info_to_dev(ptr) ptr

#endif /* < KERNEL_VERSION(3, 11, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)

/* alloc_netdev() was defined differently before 2.6.38 */
#undef alloc_netdev
#define alloc_netdev(sizeof_priv, name, name_assign_type, setup) \
	alloc_netdev_mqs(sizeof_priv, name, setup, 1, 1)

#endif /* < KERNEL_VERSION(3, 17, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)

#define dev_get_iflink(_net_dev) ((_net_dev)->iflink)

#endif /* < KERNEL_VERSION(3, 19, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)

#define netdev_master_upper_dev_link(dev, upper_dev, upper_priv, upper_info) \
	netdev_set_master(dev, upper_dev)

#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0)

#define netdev_master_upper_dev_link(dev, upper_dev, upper_priv, upper_info) \
	netdev_master_upper_dev_link(dev, upper_dev)

#endif /* < KERNEL_VERSION(4, 5, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0)

#define netif_trans_update batadv_netif_trans_update
static inline void batadv_netif_trans_update(struct net_device *dev)
{
	dev->trans_start = jiffies;
}

#endif /* < KERNEL_VERSION(4, 7, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 9)

/* work around missing attribute needs_free_netdev and priv_destructor in
 * net_device
 */
#define ether_setup(dev) \
	void batadv_softif_free2(struct net_device *dev) \
	{ \
		batadv_softif_free(dev); \
		free_netdev(dev); \
	} \
	void (*t1)(struct net_device *dev) __attribute__((unused)); \
	bool t2 __attribute__((unused)); \
	ether_setup(dev)
#define needs_free_netdev destructor = batadv_softif_free2; t2
#define priv_destructor destructor = batadv_softif_free2; t1

#endif /* < KERNEL_VERSION(4, 11, 9) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_ */
