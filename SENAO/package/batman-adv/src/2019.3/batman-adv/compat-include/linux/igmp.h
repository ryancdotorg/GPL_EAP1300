/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_IGMP_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_IGMP_H_

#include <linux/version.h>
#include_next <linux/igmp.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)

int ip_mc_check_igmp(struct sk_buff *skb);

#elif LINUX_VERSION_CODE < KERNEL_VERSION(5, 1, 0)

static inline int batadv_ip_mc_check_igmp(struct sk_buff *skb)
{
	return ip_mc_check_igmp(skb, NULL);
}

#define ip_mc_check_igmp(skb) \
	batadv_ip_mc_check_igmp(skb)

#endif /* < KERNEL_VERSION(4, 2, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_IGMP_H_ */
