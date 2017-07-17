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

#ifdef DEBUG
void    appfinder_refcount_debug_add (GObject     *object,
                                      const gchar *description);
#else
#define appfinder_refcount_debug_add(object, description) G_STMT_START{ (void)0; }G_STMT_END
#endif

#ifdef DEBUG
#define appfinder_assert(expr)                 g_assert (expr)
#define appfinder_assert_not_reached()         g_assert_not_reached ()
#define appfinder_return_if_fail(expr)         g_return_if_fail (expr)
#define appfinder_return_val_if_fail(expr,val) g_return_val_if_fail (expr, val)
#else
#define appfinder_assert(expr)                 G_STMT_START{ (void)0; }G_STMT_END
#define appfinder_assert_not_reached()         G_STMT_START{ (void)0; }G_STMT_END
#define appfinder_return_if_fail(expr)         G_STMT_START{ (void)0; }G_STMT_END
#define appfinder_return_val_if_fail(expr,val) G_STMT_START{ (void)0; }G_STMT_END
#endif

#define XFCE_APPFINDER_STOCK_BOOKMARK_NEW "bookmark-new"
#define XFCE_APPFINDER_STOCK_CLEAR "edit-clear"
#define XFCE_APPFINDER_STOCK_CLOSE "window-close"
#define XFCE_APPFINDER_STOCK_DELETE "edit-delete"
#define XFCE_APPFINDER_STOCK_DIALOG_ERROR "dialog-error"
#define XFCE_APPFINDER_STOCK_EDIT "gtk-edit"
#define XFCE_APPFINDER_STOCK_EXECUTE "system-run"
#define XFCE_APPFINDER_STOCK_FIND "edit-find"
#define XFCE_APPFINDER_STOCK_GO_DOWN "go-down"
#define XFCE_APPFINDER_STOCK_GO_UP "go-up"
#define XFCE_APPFINDER_STOCK_PREFERENCES "preferences-system"
#define XFCE_APPFINDER_STOCK_REVERT_TO_SAVED "document-revert"
#define XFCE_APPFINDER_STOCK_REMOVE "list-remove"



void appfinder_window_new (const gchar *startup_id,
                           gboolean     expanded);

#endif /* !__XFCE_APPFINDER_PRIVATE_H__ */
