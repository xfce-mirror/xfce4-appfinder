/*  xfce4-appfinder
 *
 *  Copyright (C) 2004 Eduard Roccatello (eduard@xfce.org)
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
*/

#ifndef __HAVE_APPFINDER_H
#define __HAVE_APPFINDER_H

#include <af-constants.h>
#define CONFIGFILE "afhistory"

typedef struct _appfinder Appfinder;
struct _appfinder
{
    GtkWidget *mainwindow;
    GtkWidget *hpaned;
    GtkWidget *rightvbox;
    
    GtkWidget *searchbox;
    GtkWidget *searchlabel;
    GtkWidget *searchentry;
    
    GtkWidget *categoriestree;
    GtkWidget *appstree;
    GtkWidget *appscroll;
} _appfinder;

typedef struct _afdialog AfDialog;
struct _afdialog
{
    GtkWidget *dialog;
    GtkWidget *frame;
    GtkWidget *vbox;
    GtkWidget *vboxl;
    GtkWidget *header;
    GtkWidget *hbox;
    GtkWidget *img;
    GtkWidget *name;
    GtkWidget *comment;
    GtkWidget *cats;
    GtkWidget *exec;
    GtkWidget *bbox;
    GtkWidget *btnClose;
    GtkWidget *separator;
};

int showedcat;
int npaths;
const char *configfile;

/*********************
 * Functions Proto
 *********************/

gboolean xfce_appfinder_list_add (XfceDesktopEntry *dentry, GtkListStore *store, GPatternSpec  *psearch, GPatternSpec  *pcat);

GtkListStore *
create_categories_liststore (void);

GtkListStore *
create_search_liststore(gchar *textSearch);

GtkListStore *
create_apps_liststore(void);

GtkWidget *
create_apps_treeview(void);

GtkWidget *
create_categories_treeview(void);

Appfinder *
create_interface(void);

GtkListStore *fetch_desktop_resources (gint category, gchar *pattern);

gchar **parseHistory(void);

gchar *get_path_from_name(gchar *name);

void saveHistory(gchar *path);

void execute_from_name (gchar *name);

#endif
