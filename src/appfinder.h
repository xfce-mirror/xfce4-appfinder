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

typedef struct {
	GtkWidget *mainwindow;
	GtkWidget *hpaned;
	GtkWidget *rightvbox;

	GtkWidget *searchbox;
	GtkWidget *searchlabel;
	GtkWidget *searchentry;

	GtkWidget *categoriestree;
	GtkWidget *appstree;
	GtkWidget *appscroll;
} t_appfinder;

typedef struct _afdialog AfDialog;
struct _afdialog {
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
const char *configfile;

/*********************
 * Functions Proto
 *********************/
void
cb_appstree (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata);
void
cb_searchentry (GtkEntry *entry,
			gpointer userdata);
gboolean
cb_categoriestree (GtkTreeSelection *selection,
                       GtkTreeModel     *model,
                       GtkTreePath      *path,
                       gboolean          path_currently_selected,
                       gpointer          userdata);

void
cb_dragappstree (GtkWidget *widget, GdkDragContext *drag_context, GtkSelectionData *data,
				guint info, guint time, gpointer user_data);

gboolean
cb_appstreeclick (GtkWidget *widget, GdkEventButton *event, gpointer treeview);

void cb_menurun (GtkMenuItem *menuitem, gpointer data);

void cb_menuinfo (GtkMenuItem *menuitem, gpointer data);

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

t_appfinder *
create_interface(void);

GdkPixbuf *
load_icon_entry(gchar *img);

gchar *parseExec(gchar *exec);

GtkListStore *fetch_desktop_resources (gint category, gchar *pattern);

gchar **parseHistory(void);

gchar *get_path_from_name(gchar *name);

void saveHistory(gchar *path);

void execute_from_name (gchar *name);
