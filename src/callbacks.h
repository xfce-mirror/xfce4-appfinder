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
