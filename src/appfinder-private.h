/*
 * Copyright (C) 2011 Nick Schermer <nick@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __XFCE_APPFINDER_PRIVATE_H__
#define __XFCE_APPFINDER_PRIVATE_H__

#define ICON_SMALL   32
#define ICON_LARGE   48

#define ITER_GET_DATA(iter)          (((GSList *) (iter)->user_data)->data)
#define ITER_INIT(iter, iter_stamp, iter_data) \
G_STMT_START { \
  (iter).stamp = iter_stamp; \
  (iter).user_data = iter_data; \
} G_STMT_END
#define IS_STRING(str) ((str) != NULL && *(str) != '\0')

#ifdef DEBUG
#define APPFINDER_DEBUG(...) g_print ("xfce4-appfinder-dbg: "); g_print (__VA_ARGS__); g_print ("\n")
#else
#define APPFINDER_DEBUG(...) G_STMT_START{ (void)0; }G_STMT_END
#endif

#endif /* !__XFCE_APPFINDER_PRIVATE_H__ */
