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

#ifndef __XFCE_APPFINDER_MODEL_H__
#define __XFCE_APPFINDER_MODEL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _XfceAppfinderModelClass XfceAppfinderModelClass;
typedef struct _XfceAppfinderModel      XfceAppfinderModel;

#define XFCE_TYPE_APPFINDER_MODEL            (xfce_appfinder_model_get_type ())
#define XFCE_APPFINDER_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_APPFINDER_MODEL, XfceAppfinderModel))
#define XFCE_APPFINDER_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_APPFINDER_MODEL, XfceAppfinderModelClass))
#define XFCE_IS_APPFINDER_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_APPFINDER_MODEL))
#define XFCE_IS_APPFINDER_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_APPFINDER_MODEL))
#define XFCE_APPFINDER_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_APPFINDER_MODEL, XfceAppfinderModelClass))

#define ICON_SMALL   32
#define ICON_LARGE   48

#define ITER_GET_DATA(iter)          (((GSList *) (iter)->user_data)->data)
#define ITER_INIT(iter, iter_stamp, iter_data) \
G_STMT_START { \
  (iter).stamp = iter_stamp; \
  (iter).user_data = iter_data; \
} G_STMT_END
#define IS_STRING(str) ((str) != NULL && *(str) != '\0')

enum
{
  XFCE_APPFINDER_MODEL_COLUMN_ABSTRACT,
  XFCE_APPFINDER_MODEL_COLUMN_ICON_SMALL,
  XFCE_APPFINDER_MODEL_COLUMN_ICON_LARGE,
  XFCE_APPFINDER_MODEL_COLUMN_VISIBLE,
  XFCE_APPFINDER_MODEL_COLUMN_COMMAND,
  XFCE_APPFINDER_MODEL_COLUMN_URI,
  XFCE_APPFINDER_MODEL_COLUMN_TOOLTIP,
  XFCE_APPFINDER_MODEL_N_COLUMNS,
};



GType               xfce_appfinder_model_get_type             (void) G_GNUC_CONST;

XfceAppfinderModel *xfce_appfinder_model_new                  (void) G_GNUC_MALLOC;

void                xfce_appfinder_model_filter_category      (XfceAppfinderModel  *model,
                                                               const gchar         *category);

void                xfce_appfinder_model_filter_string        (XfceAppfinderModel  *model,
                                                               const gchar         *seach_string);

gboolean            xfce_appfinder_model_execute              (XfceAppfinderModel  *model,
                                                               GtkTreeIter         *iter,
                                                               GdkScreen           *screen,
                                                               gboolean            *is_regular_command,
                                                               GError             **error);

GdkPixbuf          *xfce_appfinder_model_load_pixbuf          (const gchar         *icon_name,
                                                               gint                 size) G_GNUC_MALLOC;

gboolean            xfce_appfinder_model_save_command         (XfceAppfinderModel  *model,
                                                               const gchar         *command,
                                                               GError             **error);

GdkPixbuf          *xfce_appfinder_model_get_icon_for_command (XfceAppfinderModel  *model,
                                                               const gchar         *command);

G_END_DECLS

#endif /* !__XFCE_APPFINDER_MODEL_H__ */
