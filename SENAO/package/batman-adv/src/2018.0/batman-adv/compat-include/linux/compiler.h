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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_

#include <linux/version.h>
#include_next <linux/compiler.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)

#define READ_ONCE(x) ACCESS_ONCE(x)

#define WRITE_ONCE(x, val) ({ \
	ACCESS_ONCE(x) = (val); \
})

#endif /* < KERNEL_VERSION(3, 19, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_ */
