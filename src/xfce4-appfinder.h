/*  xfce4-appfinder
 *
 *  Copyright (C) 2004-2005 Eduard Roccatello (eduard@xfce.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  This application is dedicated to DarkAngel (ILY!).
 */

#ifndef __XFCE4_APPFINDER_H__
#define __XFCE4_APPFINDER_H__

#include <gdk/gdk.h>
#include <gtk/gtkvbox.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define XFCE_APPFINDER(obj)           GTK_CHECK_CAST (obj, xfce_appfinder_get_type(), XfceAppfinder)
#define XFCE_APPFINDER_CLASS(klass)   GTK_CHECK_CLASS_CAST (klass, xfce_appfinder_get_type(), XfceAppfinderClass)
#define XFCE_IS_APPFINDER(obj)        GTK_CHECK_TYPE (obj, xfce_appfinder_get_type())
#define APPFINDER_ALL                  0

typedef struct _XfceAppfinder        XfceAppfinder;
typedef struct _XfceAppfinderClass   XfceAppfinderClass;

struct _XfceAppfinder
{
    gpointer  desktopData;
    GtkVBox      vbox;
    
    GtkWidget *hpaned;
    GtkWidget *rightvbox;
    
    GtkWidget *searchbox;
    GtkWidget *searchlabel;
    GtkWidget *searchentry;
    
    GtkWidget *categoriesTree;
    GtkWidget *appsTree;
    GtkWidget *appScroll;    
};

struct _XfceAppfinderClass
{
    GtkVBoxClass parent_class;
    
    void (* xfce_appfinder) (XfceAppfinder *appfinder);
};

enum
{
    CATEGORY_TREE_TEXT = 0,
    CATEGORY_TREE_COLS
};

enum
{
    APPLICATION_TREE_ICON = 0,
    APPLICATION_TREE_TEXT,
    APPLICATION_TREE_COLS
};

GtkType        xfce_appfinder_get_type          (void);
GtkWidget*     xfce_appfinder_new               (void);
void           xfce_appfinder_search            (XfceAppfinder  *appfinder, 
                                                 gchar          *pattern);
void           xfce_appfinder_view_categories   (XfceAppfinder  *appfinder,
                                                 gboolean        visible);
void           xfce_appfinder_clean             (XfceAppfinder  *appfinder);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __XFCE4_APPFINDER_H__ */
