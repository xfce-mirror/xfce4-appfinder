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

#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include "af-constants.h"
#include "appfinder.h"
#include "inline-icon.h"

/**************
 * Functions
 **************/

void
cb_dragappstree (GtkWidget *widget, GdkDragContext *dc, GtkSelectionData *data,
				guint info, guint time, gpointer user_data) {
	GtkTreeModel *model;
    GtkTreeIter   iter;
	gchar *name = NULL;
	gchar *path = NULL;

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
		if (gtk_tree_model_get_iter(model, &iter,
					gtk_tree_row_reference_get_path(g_object_get_data(G_OBJECT(dc), "gtk-tree-view-source-row")))) {
			gtk_tree_model_get(model, &iter, APP_TEXT, &name, -1);
			if (name){
				if ((path = get_path_from_name(name))!=NULL) {
					gtk_selection_data_set (data, gdk_atom_intern ("text/plain", FALSE), 8, path, strlen(path));
					g_free(path);
				}
				g_free(name);
			}
		}
}

void
cb_searchentry (GtkEntry *entry,
			gpointer userdata)
{
	GtkTreeIter iter;
	t_appfinder *af = userdata;
	gchar *text = g_utf8_strdown(gtk_entry_get_text(entry), -1);
	showedcat = APPFINDER_ALL;
	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(af->appstree))));
	gtk_tree_view_set_model (GTK_TREE_VIEW(af->appstree),
							GTK_TREE_MODEL(fetch_desktop_resources(showedcat, text)));

	/* No application found. Tell the user about it */
	if (!gtk_tree_model_get_iter_first (gtk_tree_view_get_model (GTK_TREE_VIEW(af->appstree)), &iter))
	{
		gtk_list_store_append(GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(af->appstree))), &iter);
		gtk_list_store_set(GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(af->appstree))),
							&iter, APP_ICON, xfce_inline_icon_at_size (default_icon_data_48_48, 24, 24),
							APP_TEXT, "Sorry, no match for searched text.", -1);
		gtk_widget_set_sensitive(af->appstree, FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(af->appstree, TRUE);
	}

	gtk_tree_selection_unselect_all(
							gtk_tree_view_get_selection(GTK_TREE_VIEW(af->categoriestree)));
	if (text)
		g_free(text);
}


gchar *get_path_from_name(gchar *name) {
	GDir *dir;
	XfceDesktopEntry *dentry;
	gboolean found = FALSE;
	gchar *filename = NULL;
	gchar *dname = NULL;
	gint i = 0;

	if (!name)
		return NULL;

	while (entriespaths[i])
	{
		if ((dir = g_dir_open (entriespaths[i], 0, NULL))==NULL)
		{
			i++;
			continue;
		}

		while (!found && ((filename = (gchar *)g_dir_read_name(dir))!=NULL))
		{
			filename = g_strconcat(entriespaths[i], filename, NULL);
			if (g_file_test(filename, G_FILE_TEST_IS_DIR) ||
					!XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (filename, keys, 7))) {
				g_free(filename);
				continue;
			}

			xfce_desktop_entry_get_string (dentry, "Name", FALSE, &dname);

			if (dname) {
				if (strcmp(dname, name)==0)
					found = TRUE;
				g_free(dname);
			}

			if (!found)
				g_free(filename);
		}
		g_dir_close(dir);
		if (found)
			return filename;
		i++;
	}
	return NULL;
}

void
cb_appstree (GtkTreeView        *treeview,
             GtkTreePath        *path,
             GtkTreeViewColumn  *col,
             gpointer            userdata)
{
	gchar *name = NULL;
	GtkTreeModel *model;
	GtkTreeIter   iter;

    model = gtk_tree_view_get_model(treeview);
    if (gtk_tree_model_get_iter(model, &iter, path))
    {
		/* we fetch the name of the application to run */
		gtk_tree_model_get(model, &iter, APP_TEXT, &name, -1);
		if (name)
		{
			execute_from_name (name);
			g_free(name);
		}
    }
}

void execute_from_name (gchar *name)
{
	gchar *filepath = NULL;
	gchar *exec = NULL;
	gchar **execp = NULL;
	XfceDesktopEntry *dentry;

	if ((filepath = get_path_from_name(name)) &&
		XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (filepath, keys, 7)) &&
		xfce_desktop_entry_get_string (dentry, "Exec", FALSE, &exec))
	{
		saveHistory(filepath);
		if (exec) {
			if (g_strrstr(exec, "%")!= NULL)
			{
				execp = g_strsplit(exec, "%", 0);
				g_printf("Now starting \"%s\"...\n", execp[0]);
				exec_command(execp[0]);
				g_strfreev (execp);
			}
			else
			{
				g_printf("Now starting \"%s\"...\n", exec);
				exec_command(exec);
			}
			g_free(exec);
		}
	}
	else {
		xfce_info("Cannot execute %s\n", name);
	}
	if (filepath)
		g_free(filepath);
}

gboolean
cb_categoriestree (GtkTreeSelection *selection,
                       GtkTreeModel     *model,
                       GtkTreePath      *path,
                       gboolean          path_currently_selected,
                       gpointer          userdata)
{
	int next = showedcat;
	int i = 0;
	GtkTreeIter iter;
	gchar *name = NULL;
	t_appfinder *af = userdata;

    if (!path_currently_selected && gtk_tree_model_get_iter(model, &iter, path))
    {
		gtk_tree_model_get(model, &iter, CAT_TEXT, &name, -1);
		if (name)
		{
			while (categories[i])
			{
				if (strcmp(categories[i], name)==0)
				{
					next=i;
					break;
				}
				i++;
			}
			g_free(name);
		}
    }
	if (next == showedcat)
		return TRUE;
	showedcat = next;
	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(af->appstree))));
	gtk_tree_view_set_model (GTK_TREE_VIEW(af->appstree),
							GTK_TREE_MODEL(fetch_desktop_resources(showedcat, NULL)));

	/* Ok there are no items in the list. Write a message and disable the treeview */
	if (!gtk_tree_model_get_iter_first (gtk_tree_view_get_model (GTK_TREE_VIEW(af->appstree)), &iter))
	{
		gtk_list_store_append(GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(af->appstree))), &iter);
		gtk_list_store_set(GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(af->appstree))),
							&iter, APP_ICON, xfce_inline_icon_at_size (default_icon_data_48_48, 24, 24),
							APP_TEXT, "No items available", -1);
		gtk_widget_set_sensitive(af->appstree, FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(af->appstree, TRUE);
	}
    return TRUE; /* allow selection state to change */
  }

void cb_menurun (GtkMenuItem *menuitem, gpointer data)
{
	gchar *name = data;

	if (name)
	{
		execute_from_name(name);
		g_free (name);
	}
}

/*
	Icon - Name
	Comment:		Comment
	Categories:		Cats
	Command:		Exec
*/
void cb_menuinfo (GtkMenuItem *menuitem, gpointer data)
{
	AfDialog *dlg;
	GdkPixbuf *icon;
	GdkPixbuf *icon2;
	gchar *name = data;
	gchar *path = NULL;
	gchar *iconpath = NULL;
	gchar *comment = NULL;
	gchar *cats = NULL;
	gchar *exec = NULL;
	gchar **catsarray = NULL;
	XfceDesktopEntry *dentry;

	if (name && (path = get_path_from_name(name)) &&
		XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (path, keys, 7)))
	{
		dlg = g_new (AfDialog, 1);
		dlg->dialog = gtk_dialog_new ();
		gtk_window_set_title (GTK_WINDOW (dlg->dialog), _("Appfinder InfoBox"));
    	gtk_dialog_set_has_separator (GTK_DIALOG (dlg->dialog), FALSE);
	    icon = xfce_inline_icon_at_size (default_icon_data_48_48, 32, 32);
    	gtk_window_set_icon (GTK_WINDOW (dlg->dialog), icon);

		dlg->vbox = GTK_DIALOG (dlg->dialog)->vbox;
    	dlg->header = xfce_create_header (icon, g_strconcat(_("Informations about \""), name, "\"", NULL));
    	gtk_widget_show (dlg->header);
    	gtk_box_pack_start (GTK_BOX (dlg->vbox), dlg->header, FALSE, TRUE, 0);
		g_object_unref(icon);

		dlg->hbox = gtk_hbox_new(FALSE, 0);
		gtk_widget_show(dlg->hbox);
		gtk_box_pack_start (GTK_BOX (dlg->vbox), dlg->hbox, TRUE, TRUE, 10);

		dlg->vboxl = gtk_vbox_new(FALSE, 0);
		gtk_widget_show(dlg->vboxl);
		gtk_box_pack_start (GTK_BOX (dlg->hbox), dlg->vboxl, TRUE, TRUE, 10);

		dlg->frame = gtk_aspect_frame_new ("Icon", 0.5, 0.5, 1.8, TRUE);
		gtk_widget_show(dlg->frame);
		gtk_box_pack_start (GTK_BOX (dlg->hbox), dlg->frame, FALSE, TRUE, 10);

		xfce_desktop_entry_get_string (dentry, "Icon", FALSE, &iconpath);
		if (iconpath)
		{
			icon = load_icon_entry (iconpath);
			if (icon)
				icon = gdk_pixbuf_scale_simple(icon, 48, 48, GDK_INTERP_BILINEAR);
			else
			{
				icon = xfce_inline_icon_at_size (default_icon_data_48_48, 48, 48);
				icon2 = gdk_pixbuf_copy (icon);
				gdk_pixbuf_saturate_and_pixelate(icon, icon2, 0.0, TRUE);
				g_object_unref(icon);
				icon = icon2;
			}
			g_free(iconpath);
		}
		else
			icon = xfce_inline_icon_at_size (default_icon_data_48_48, 48, 48);

		dlg->img = gtk_image_new_from_pixbuf (icon);
		gtk_widget_show(dlg->img);
		gtk_container_add (GTK_CONTAINER (dlg->frame), dlg->img);

		dlg->name = gtk_label_new(NULL);
		gtk_label_set_markup (GTK_LABEL(dlg->name), g_strconcat("<b>Name:</b> ", name, NULL));
		gtk_misc_set_alignment (GTK_MISC(dlg->name), 0, 0);
		gtk_widget_show(dlg->name);
		gtk_box_pack_start (GTK_BOX (dlg->vboxl), dlg->name, FALSE, FALSE, 0);

		xfce_desktop_entry_get_string (dentry, "Comment", FALSE, &comment);
		if (!comment)
			comment = "N/A";
		dlg->comment = gtk_label_new(NULL);
		gtk_label_set_line_wrap (GTK_LABEL(dlg->comment), TRUE);
		gtk_label_set_markup (GTK_LABEL(dlg->comment), g_strconcat("<b>Comment:</b> ", comment, NULL));
		g_free(comment);
		gtk_misc_set_alignment (GTK_MISC(dlg->comment), 0, 0);
		gtk_widget_show(dlg->comment);
		gtk_box_pack_start (GTK_BOX (dlg->vboxl), dlg->comment, FALSE, FALSE, 0);

		xfce_desktop_entry_get_string (dentry, "Categories", FALSE, &cats);
		if (!cats)
			cats = "N/A";
		else
		{
			catsarray = g_strsplit (cats, ";", 0);
			cats = g_strchomp (g_strjoinv (", ", catsarray));
			cats[strlen(cats)-1] = '\0';
			g_strfreev (catsarray);
		}

		dlg->cats = gtk_label_new(NULL);
		gtk_label_set_markup (GTK_LABEL(dlg->cats), g_strconcat("<b>Categories:</b> ", cats, NULL));
		g_free(cats);
		gtk_misc_set_alignment (GTK_MISC(dlg->cats), 0, 0);
		gtk_widget_show(dlg->cats);
		gtk_box_pack_start (GTK_BOX (dlg->vboxl), dlg->cats, FALSE, FALSE, 0);

		xfce_desktop_entry_get_string (dentry, "Exec", FALSE, &exec);
		if (!exec)
			exec = "N/A";
		dlg->exec = gtk_label_new(NULL);
		gtk_label_set_markup (GTK_LABEL(dlg->exec), g_strconcat("<b>Command:</b> ", exec, NULL));
		gtk_misc_set_alignment (GTK_MISC(dlg->exec), 0, 0);
		g_free(exec);
		gtk_widget_show(dlg->exec);
		gtk_box_pack_start (GTK_BOX (dlg->vboxl), dlg->exec, FALSE, FALSE, 0);

		dlg->separator = gtk_hseparator_new();
		gtk_widget_show(dlg->separator);
		gtk_box_pack_start (GTK_BOX (dlg->vbox), dlg->separator, FALSE, TRUE, 0);

		dlg->btnClose = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
		gtk_dialog_add_action_widget (GTK_DIALOG (dlg->dialog), dlg->btnClose, GTK_RESPONSE_CLOSE);
		GTK_WIDGET_SET_FLAGS (dlg->btnClose, GTK_CAN_DEFAULT);
		gtk_widget_show(dlg->btnClose);

		g_signal_connect_swapped (GTK_OBJECT (dlg->dialog), "response",
								G_CALLBACK (gtk_widget_destroy), GTK_OBJECT (dlg->dialog));
								
		gtk_widget_grab_focus (dlg->btnClose);
		gtk_widget_show(dlg->dialog);
		g_free(name);
	}
}

gboolean
cb_appstreeclick (GtkWidget *widget, GdkEventButton *event, gpointer treeview)
{
	GtkWidget *menu, *menuitem, *icon;
	GtkTreeSelection *selection;
	GtkTreePath *treepath;
	GtkTreeModel *treemodel;
	GtkTreeIter iter;
	gchar *name;
	/* 3 is for right button */
	if (event->button == 3)
	{
	 	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		gtk_tree_selection_unselect_all(selection);
		if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW(treeview), event->x, event->y,
															&treepath, NULL, NULL, NULL))
		{
			gtk_tree_selection_select_path (selection, treepath);
			treemodel = gtk_tree_view_get_model(treeview);
			gtk_tree_model_get_iter (treemodel, &iter, treepath);
 			gtk_tree_model_get(treemodel, &iter, APP_TEXT, &name, -1);

			menu = gtk_menu_new();
			menuitem = gtk_image_menu_item_new_with_label ("Xfce4 Appfinder");
			gtk_widget_show (menuitem);
			gtk_widget_set_sensitive (menuitem, FALSE);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

			menuitem = gtk_separator_menu_item_new ();
			gtk_widget_show (menuitem);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

			menuitem = gtk_image_menu_item_new_with_label ("Run program");
			icon = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), icon);
			g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(cb_menurun), (gpointer)name);
			gtk_widget_show (icon);
			gtk_widget_show (menuitem);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

			menuitem = gtk_image_menu_item_new_with_label ("Informations...");
			icon = gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), icon);
			g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(cb_menuinfo), (gpointer)name);
			gtk_widget_show (icon);
			gtk_widget_show (menuitem);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

			menuitem = gtk_separator_menu_item_new ();
			gtk_widget_show (menuitem);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

			menuitem = gtk_image_menu_item_new_with_label ("Add to panel");
			icon = gtk_image_new_from_stock (GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), icon);
			gtk_widget_show (icon);
			gtk_widget_show (menuitem);
			gtk_widget_set_sensitive (menuitem, FALSE);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

			menuitem = gtk_image_menu_item_new_with_label ("Add to desktop menu");
			icon = gtk_image_new_from_stock (GTK_STOCK_INDEX, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), icon);
			gtk_widget_show (icon);
			gtk_widget_show (menuitem);
			gtk_widget_set_sensitive (menuitem, FALSE);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

			gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
			return TRUE;
		}
	}
	/* If hasn't been clicked with right button let's propagate the event */
	return FALSE;
}

/**********
 * create_interface
 **********/
t_appfinder *create_interface(void)
{
	t_appfinder *af;

	af = g_new(t_appfinder, 1);
	showedcat = APPFINDER_ALL;

	af->mainwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(af->mainwindow, "delete_event", gtk_main_quit, NULL);
	gtk_window_set_title(GTK_WINDOW(af->mainwindow), "Xfce4 Appfinder");
	gtk_window_set_icon(GTK_WINDOW(af->mainwindow),xfce_inline_icon_at_size (default_icon_data_48_48, 48, 48));

	af->hpaned = GTK_WIDGET(gtk_hpaned_new ());
	gtk_container_add(GTK_CONTAINER(af->mainwindow), af->hpaned);
	af->categoriestree = create_categories_treeview();
	gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(GTK_TREE_VIEW(af->categoriestree)), cb_categoriestree, af, NULL);
	gtk_paned_pack1(GTK_PANED(af->hpaned), af->categoriestree, TRUE, TRUE);

	af->rightvbox = GTK_WIDGET(gtk_vbox_new (FALSE, 0));
	gtk_paned_pack2(GTK_PANED(af->hpaned), af->rightvbox, TRUE, TRUE);
	
	af->searchbox = GTK_WIDGET(gtk_hbox_new(FALSE, 6));
	gtk_container_set_border_width(GTK_CONTAINER(af->searchbox), 6);
	gtk_box_pack_start(GTK_BOX(af->rightvbox), af->searchbox, FALSE, TRUE, 0);

	af->searchlabel = GTK_WIDGET(gtk_label_new(NULL));
	gtk_misc_set_alignment(GTK_MISC(af->searchlabel), 0.0, 0.5);
	gtk_label_set_markup_with_mnemonic(GTK_LABEL(af->searchlabel), "<b>Search:</b>");
	gtk_box_pack_start(GTK_BOX(af->searchbox), af->searchlabel, FALSE, TRUE, 0);

	af->searchentry = GTK_WIDGET(gtk_entry_new());
	g_signal_connect(af->searchentry, "activate", (GCallback) cb_searchentry, af);
	gtk_box_pack_start(GTK_BOX(af->searchbox), af->searchentry, TRUE, TRUE, 0);

	af->appscroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(af->appscroll), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(af->appscroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(af->rightvbox), af->appscroll, TRUE, TRUE, 0);
	af->appstree = create_apps_treeview();
	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(af->appstree),
										GDK_BUTTON1_MASK, gte, 5, GDK_ACTION_COPY);
	g_signal_connect(af->appstree, "row-activated", (GCallback) cb_appstree, NULL);
	g_signal_connect(af->appstree, "drag-data-get", (GCallback) cb_dragappstree, NULL);
	g_signal_connect(af->appstree, "button-press-event", G_CALLBACK(cb_appstreeclick), af->appstree);


	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(af->appscroll), af->appstree);

	gtk_window_set_position(GTK_WINDOW(af->mainwindow), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_default_size(GTK_WINDOW(af->mainwindow), gdk_screen_width ()/2, gdk_screen_height()/2);

	gtk_widget_show_all(af->mainwindow);
	return af;
}

/**********
 *  create_categories_liststore
 **********/
GtkListStore *create_categories_liststore(void)
{
  int i = 0;
  GtkListStore  *store;
  GtkTreeIter    iter;

  store = gtk_list_store_new(CAT_COLS, G_TYPE_STRING);

  while(categories[i])
  {
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, CAT_TEXT, categories[i], -1);
	i++;
  }
  return store;
}


/**********
 *
 *   create_categories_treeview
 *
 **********/
GtkWidget *create_categories_treeview(void)
{
	GtkTreeModel      *model;
	GtkTreeViewColumn *col;
	GtkCellRenderer   *renderer;
	GtkWidget         *view;

	model = GTK_TREE_MODEL(create_categories_liststore());

	view = gtk_tree_view_new_with_model(model);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Categories");

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_set_attributes(col, renderer,
	                                    "text", CAT_TEXT,
	                                    NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

	return view;
}


/*
 * Load an icon entry from a given path
 *
 * @param img - absolute or relative path to the icon
 * @returns GdkPixbuf * a pointer to the icon (must be freed) (NULL if not found)
 */
GdkPixbuf *
load_icon_entry (gchar *img)
{
	gint i = 0;
	gchar *imgPath = NULL;
	GdkPixbuf *icon = NULL;
	GError    *error = NULL;

	if (!g_file_test(img, G_FILE_TEST_EXISTS))
	{
		/* ok, it's not a path. has it a .png suffix? */
		/* if not let's add it and check for the icon */
		if (g_str_has_suffix(img, ".png") || g_str_has_suffix(img, ".xpm"))
		{
			while(iconspaths[i])
			{
				imgPath = g_strdup_printf ("%s%s", iconspaths[i], img);
				if (g_file_test(imgPath, G_FILE_TEST_EXISTS))
					break;
				g_free(imgPath);
				imgPath = NULL;
				i++;
			}
		}
		else
		{
			while(iconspaths[i])
			{
				imgPath = g_strdup_printf ("%s%s.png", iconspaths[i], img);
				if (g_file_test(imgPath, G_FILE_TEST_EXISTS))
					break;
				g_free(imgPath);
				imgPath = g_strdup_printf ("%s%s.xpm", iconspaths[i], img);
				if (g_file_test(imgPath, G_FILE_TEST_EXISTS))
					break;
				g_free(imgPath);
				imgPath = NULL;
				i++;
			}
		}
	}
	else
	{
		imgPath = g_strdup(img);
	}

	if (imgPath==NULL)
		return NULL;

	icon = gdk_pixbuf_new_from_file(imgPath, &error);
	g_free(imgPath);

	if (error)
	{
		g_warning("%s", error->message);
		g_error_free(error);
		error = NULL;
		icon = NULL;
	}

	return icon;
}

/*
 * This function handles all the searches into desktop files
 *
 * @param category - the category to search for (defined into the array in the header)
 * @param pattern - the pattern of the text to search for (set to NULL if any text is ok)
 * @returns GtkListStore * - a pointer to a new list store with the items
 */
GtkListStore *fetch_desktop_resources (gint category, gchar *pattern) {
	XfceDesktopEntry *dentry = NULL;
	GtkListStore  *store = NULL;
	GtkTreeIter    iter;
	GdkPixbuf     *icon = NULL;
	GPatternSpec  *ptrn = NULL;
	GDir *dir = NULL;
	gboolean found = FALSE;
	gchar *filename = NULL;
	gchar *comment = NULL;
	gchar *name = NULL;
	gchar *img = NULL;
	gchar *dcategories = NULL;
	gchar **cats = NULL;
	gint x = 0; /* A counter for category section */
	gint i = 0; /* A counter for general use */

	store = gtk_list_store_new(APP_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);
	if (category==APPFINDER_HISTORY) {
		/* We load data from appfinder' rc file */
		gchar **history = parseHistory();
		if (history!=NULL) {
			while (history[i]!=NULL) {
				if (g_file_test(history[i], G_FILE_TEST_EXISTS) &&
						XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (history[i], keys, 7)) &&
						xfce_desktop_entry_get_string (dentry, "Name", FALSE, &name))
				{
					if (xfce_desktop_entry_get_string (dentry, "Icon", FALSE, &img) && img)
					{
							icon = load_icon_entry(img);
							if (icon)
								icon = gdk_pixbuf_scale_simple(icon, 24, 24, GDK_INTERP_BILINEAR);
							g_free(img);
					}
					else
					{
						icon = NULL;
					}
		
					gtk_list_store_append(store, &iter);
					gtk_list_store_set(store, &iter,
									APP_ICON, icon,
									APP_TEXT, name,
									-1);

					if (name)
						g_free(name);
					if (icon)
						g_object_unref (icon);
					if (dentry)
						g_object_unref (dentry);
				}
				i++;
			}
			if (history)
				g_strfreev(history);
		}
	}
	else {
		if (pattern != NULL)
		{
			gchar *tmp = g_strdelimit (pattern, " ", '*');
			tmp = g_strdup_printf ("*%s*", tmp);
			ptrn = g_pattern_spec_new (tmp);
			if (tmp)
				g_free(tmp);
		}

		while (entriespaths[i]!=NULL) {
			if ((dir = g_dir_open (entriespaths[i], 0, NULL))!=NULL)
			{
				while ((filename = (gchar *)g_dir_read_name(dir))!=NULL)
				{
					filename = g_strconcat (entriespaths[i], filename, NULL);
					if (g_file_test(filename, G_FILE_TEST_IS_DIR) ||
							!XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (filename, keys, 7)) || !dentry)
					{
						if (filename) {
							g_free(filename);
							filename = NULL;
						}
						continue;
					}

					/* if selected categories is not All filter the results */
					if (category != APPFINDER_ALL &&
							xfce_desktop_entry_get_string (dentry, "Categories", FALSE, &dcategories))
					{
							found = FALSE;
							x = 0;

							if (dcategories)
							{
								cats = g_strsplit (dcategories, ";", 0);
								g_free(dcategories);
							}

							if (cats)
							{
								while(cats[x])
								{
									if (g_ascii_strcasecmp(cats[x], categories[category])==0)
									{
										found = TRUE;
										break;
									}
									x++;
								}

								g_strfreev (cats);

								if (!found)
								{
									if (dentry)
										g_object_unref (dentry);

									if (filename) {
										g_free(filename);
										filename = NULL;
									}

									continue;
								}
							}
					}

					if (!xfce_desktop_entry_get_string (dentry, "Name", FALSE, &name) || !name)
					{
						if (dentry) {
							g_object_unref (dentry);
							dentry = NULL;
						}

						if (name)
							g_free(name);

						if (filename)
							g_free(filename);

						continue;
					}

					if (pattern != NULL)
					{

						if (!xfce_desktop_entry_get_string (dentry, "Comment", FALSE, &comment))
							comment = NULL;

						if (!g_pattern_match_string (ptrn, g_utf8_strdown(name, -1)) ||
							(comment != NULL ?
								!g_pattern_match_string (ptrn, g_utf8_strdown(comment, -1)) :
								FALSE))
						{
							if (name)
								g_free(name);
							if (comment)
								g_free(comment);
							if (filename)
								g_free(filename);
							if (dentry) {
								g_object_unref (dentry);
								dentry = NULL;
							}
							continue;
						}
						if (comment)
							g_free(comment);
					}

					if (xfce_desktop_entry_get_string (dentry, "Icon", FALSE, &img) && img)
					{
						icon = load_icon_entry(img);
						if (icon)
							icon = gdk_pixbuf_scale_simple(icon, 24, 24, GDK_INTERP_BILINEAR);
						g_free(img);
					}
					else
					{
						icon = NULL;
					}

					gtk_list_store_append(store, &iter);
					gtk_list_store_set(store, &iter,
						APP_ICON, icon,
						APP_TEXT, name,
						-1);

					if (name) {
						g_free(name);
						name = NULL;
					}

					if (icon) {
						g_object_unref (icon);
						icon = NULL;
					}

					if (dentry) {
						g_object_unref (dentry);
						dentry = NULL;
					}

					if (filename) {
						g_free(filename);
						filename = NULL;
					}
				}
				g_dir_close(dir);
			}
			i++;
		}
	}
	/* There are no elements in the tree, leave a message */

	if (ptrn)
		g_pattern_spec_free (ptrn);
	return store;
}

/*
 * Create a new list of desktop resources
 *
 * @return GtkListStore * - a new list of items
 */
GtkListStore *create_apps_liststore(void)
{
	return fetch_desktop_resources(showedcat, NULL);
}

/*
 * Create a new list of desktop resources that matches the given text
 *
 * @param textSearch - a pointer to the text searched
 * @return GtkListStore * - a new list of items
 */
GtkListStore *create_search_liststore(gchar *textSearch)
{
	return fetch_desktop_resources(showedcat, textSearch);
}


/*
 * Create a new treeview for applications installed on the system
 */
GtkWidget *
create_apps_treeview()
{
	GtkTreeViewColumn *col;
	GtkCellRenderer   *renderer;
	GtkWidget         *view;

	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(create_apps_liststore()));

	col = gtk_tree_view_column_new();
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_set_attributes(col, renderer,
	                                    "pixbuf", APP_ICON,
	                                    NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_set_attributes(col, renderer,
	                                    "text", APP_TEXT,
	                                    NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

	return view;
}

gchar **parseHistory(void) {
	gchar *contents;
	if (g_file_get_contents (configfile, &contents, NULL, NULL) && (contents != NULL)) {
			gchar **history = g_strsplit (contents, "\n", -1);
			g_free (contents);
			return history;
	}
	return NULL;
}

void saveHistory(gchar *path) {
	gchar **history = parseHistory();
	gint i = 0;
	FILE *f;
	/* We must check if it is already in the history before inserting it ;-) */
	if (history) {
		while(history[i] != NULL) {
			if (strcmp(path, history[i])==0) {
				return;
			}
			i++;
		}
	}
	/* history has i-1 elements at this point. If i-1 > 9 we need to drop some history elements */
	f = fopen(configfile, "w");
	fprintf(f, "%s\n", path);
	i = 0;
	if (history) {
		while (history[i] != NULL && i-1<10) {
			fprintf(f, "%s\n", history[i]);
			i++;
		}
		g_strfreev(history);
	}
	fclose(f);
}
/**********
 *   main
 **********/
gint
main (gint argc, gchar **argv)
{
	t_appfinder *appfinder;
	gtk_init(&argc, &argv);
	configfile = g_strconcat (xfce_get_userdir(), "/afhistory", NULL);
	appfinder = create_interface();
	gtk_main();
	return 0;
}
