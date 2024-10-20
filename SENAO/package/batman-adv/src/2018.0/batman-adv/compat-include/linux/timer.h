/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2018  B.A.T.M.A.N. contributors:
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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H
#define _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H

#include <linux/version.h>
#include_next <linux/timer.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)

#define __setup_timer(_timer, _fn, _data, _flags) \
	do { \
		init_timer(_timer); \
		(_timer)->function = (_fn); \
		(_timer)->data = (_data); \
	} while (0)

#endif /* < KERNEL_VERSION(3, 7, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)

#define TIMER_DATA_TYPE		unsigned long
#define TIMER_FUNC_TYPE		void (*)(TIMER_DATA_TYPE)

static inline void timer_setup(struct timer_list *timer,
			       void (*callback)(struct timer_list *),
			       unsigned int flags)
{
	__setup_timer(timer, (TIMER_FUNC_TYPE)callback,
		      (TIMER_DATA_TYPE)timer, flags);
}

#define from_timer(var, callback_timer, timer_fieldname) \
	container_of(callback_timer, typeof(*var), timer_fieldname)

#endif /* < KERNEL_VERSION(4, 14, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H */
