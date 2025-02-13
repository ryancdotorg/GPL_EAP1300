/* Copyright (C) 2007-2016 B.A.T.M.A.N. contributors:
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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_CACHE_H
#define _NET_BATMAN_ADV_COMPAT_LINUX_CACHE_H

#include <linux/version.h>
#include_next <linux/cache.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)

/* hack for netlink.c which marked the family ops as ro */
#ifdef __ro_after_init
#undef __ro_after_init
#endif
#define __ro_after_init

#endif /* < KERNEL_VERSION(4, 6, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_CACHE_H */
