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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include <appfinder.h>
#include <callbacks.h>
#include <inline-icon.h>

void
cb_dragappstree (GtkWidget *widget, GdkDragContext *dc, GtkSelectionData *data,
                guint info, guint time, gpointer user_data) {
    GtkTreeModel *model;
    GtkTreeIter   iter;
    gchar *name = NULL;
    gchar *path = NULL;
    
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
        if (gtk_tree_model_get_iter(model, &iter,
                    gtk_tree_row_reference_get_path(g_object_get_data(G_OBJECT(dc), "gtk-tree-view-source-row"))))
        {
            gtk_tree_model_get(model, &iter, APP_TEXT, &name, -1);
            if (name)
            {
                if ((path = get_path_from_name(name)) != NULL)
                {
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
    Appfinder *af = userdata;
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
                            APP_TEXT, _("Sorry, no match for searched text."), -1);
        gtk_widget_set_sensitive(af->appstree, FALSE);
    }
    else
    {
        gtk_widget_set_sensitive(af->appstree, TRUE);
    }

    gtk_tree_selection_unselect_all (gtk_tree_view_get_selection(GTK_TREE_VIEW(af->categoriestree)));
    
    if (text)
    {
        g_free(text);
    }
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
    Appfinder *af = userdata;

    if (!path_currently_selected && gtk_tree_model_get_iter(model, &iter, path))
    {
        gtk_tree_model_get(model, &iter, CAT_TEXT, &name, -1);
        if (name)
        {
            while (i18ncategories[i])
            {
                if (strcmp(_(i18ncategories[i]), name) == 0)
                {
                    next = i;
                    break;
                }
                i++;
            }
            g_free(name);
        }
    }
    
    if (next == showedcat)
    {
        return TRUE;
    }
    
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
                            APP_TEXT, _("No items available"), -1);
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
    Comment:        Comment
    Categories:     Cats
    Command:        Exec
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

        dlg->frame = gtk_aspect_frame_new (_("Icon"), 0.5, 0.5, 1.8, TRUE);
        gtk_widget_show(dlg->frame);
        gtk_box_pack_start (GTK_BOX (dlg->hbox), dlg->frame, FALSE, TRUE, 10);

        xfce_desktop_entry_get_string (dentry, "Icon", FALSE, &iconpath);
        
        if (iconpath)
        {
            icon = xfce_themed_icon_load(iconpath, 48);
            if (!icon)
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
        {
            icon = xfce_inline_icon_at_size (default_icon_data_48_48, 48, 48);
        }

        dlg->img = gtk_image_new_from_pixbuf (icon);
        gtk_widget_show(dlg->img);
        gtk_container_add (GTK_CONTAINER (dlg->frame), dlg->img);

        dlg->name = gtk_label_new(NULL);
        gtk_label_set_markup (GTK_LABEL(dlg->name), g_strconcat(_("<b>Name:</b> "), name, NULL));
        gtk_misc_set_alignment (GTK_MISC(dlg->name), 0, 0);
        gtk_widget_show(dlg->name);
        gtk_box_pack_start (GTK_BOX (dlg->vboxl), dlg->name, FALSE, FALSE, 0);

        xfce_desktop_entry_get_string (dentry, _("Comment"), FALSE, &comment);
        if (!comment)
        {
            comment = _("N/A");
        }
        
        dlg->comment = gtk_label_new(NULL);
        gtk_label_set_line_wrap (GTK_LABEL(dlg->comment), TRUE);
        gtk_label_set_markup (GTK_LABEL(dlg->comment), g_strconcat(_("<b>Comment:</b> "), comment, NULL));
        g_free(comment);
        gtk_misc_set_alignment (GTK_MISC(dlg->comment), 0, 0);
        gtk_widget_show(dlg->comment);
        gtk_box_pack_start (GTK_BOX (dlg->vboxl), dlg->comment, FALSE, FALSE, 0);

        xfce_desktop_entry_get_string (dentry, "Categories", FALSE, &cats);
        if (!cats)
        {
            cats = _("N/A");
        }
        else
        {
            catsarray = g_strsplit (cats, ";", 0);
            cats = g_strchomp (g_strjoinv (", ", catsarray));
            cats[strlen(cats)-1] = '\0';
            g_strfreev (catsarray);
        }

        dlg->cats = gtk_label_new(NULL);
        gtk_label_set_markup (GTK_LABEL(dlg->cats), g_strconcat(_("<b>Categories:</b> "), cats, NULL));
        g_free(cats);
        gtk_misc_set_alignment (GTK_MISC(dlg->cats), 0, 0);
        gtk_widget_show(dlg->cats);
        gtk_box_pack_start (GTK_BOX (dlg->vboxl), dlg->cats, FALSE, FALSE, 0);

        xfce_desktop_entry_get_string (dentry, "Exec", FALSE, &exec);
        if (!exec)
        {
            exec = _("N/A");
        }
        dlg->exec = gtk_label_new(NULL);
        gtk_label_set_markup (GTK_LABEL(dlg->exec), g_strconcat(_("<b>Command:</b> "), exec, NULL));
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

            menuitem = gtk_image_menu_item_new_with_label (_("Run program"));
            icon = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU);
            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), icon);
            g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(cb_menurun), (gpointer)name);
            gtk_widget_show (icon);
            gtk_widget_show (menuitem);
            gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

            menuitem = gtk_image_menu_item_new_with_label (_("Informations..."));
            icon = gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU);
            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), icon);
            g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(cb_menuinfo), (gpointer)name);
            gtk_widget_show (icon);
            gtk_widget_show (menuitem);
            gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

            gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
            return TRUE;
        }
    }
    /* If hasn't been clicked with right button let's propagate the event */
    return FALSE;
}
