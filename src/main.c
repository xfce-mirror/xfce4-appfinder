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
 *  This program is dedicated to DarkAngel.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include "main.h"
#include "appfinder.h"
#include "xfce4-appfinder.h"
#include "inline-icon.h"

void    callbackExecuteApplication      (GtkWidget         *widget,
                                         gchar             *path,
                                         gpointer           data);
                                         
void    callbackRightClickMenu          (GtkWidget         *widget,
                                         gchar             *path,
                                         gpointer           data);

void    callbackRunMenuActivate         (GtkMenuItem       *menuitem,
                                         gpointer           path);

void    callbackInformationMenuActivate (GtkMenuItem       *menuitem,
                                         gpointer           path);                                                                                  

void    callbackSearchApplication       (GtkEntry          *entry,
                                         gpointer           appfinder);

void    callbackCategoriesCheck         (GtkToggleButton   *togglebutton,
                                         gpointer           appfinder);

/* What to search for in .desktop files */
static const char *keys [] =
{
    "Name",
    "Comment",
    "Icon",
    "Categories",
    "OnlyShowIn",
    "Exec",
    "Terminal",
    NULL
};

gchar **parseHistory(void)
{
    gchar *contents;
    
    if (g_file_get_contents (configfile, &contents, NULL, NULL) && (contents != NULL))
    {
        gchar **history = g_strsplit (contents, "\n", -1);
        g_free (contents);
        return history;
    }
    return NULL;
}

void saveHistory(gchar *path)
{
    gchar **history = parseHistory();
    gint i = 0;
    FILE *f;
    
    /* We must check if it is already in the history before inserting it ;-) */
    if (history)
    {
        while(history[i] != NULL)
        {
            if (strcmp(path, history[i]) == 0)
            {
                return;
            }
            i++;
        }
    }
    
    /* history has i-1 elements at this point. If i-1 > 9 we need to drop some history elements */
    f = fopen(configfile, "w");
    fprintf(f, "%s\n", path);
    i = 0;
    if (history)
    {
        while (history[i] != NULL && i-1<10)
        {
            fprintf(f, "%s\n", history[i]);
            i++;
        }
        g_strfreev(history);
    }
    fclose(f);
}

void
callbackExecuteApplication (GtkWidget *widget, gchar *path, gpointer data)
{
    gchar               *exec       = NULL;
    gchar               *p          = NULL;
    XfceDesktopEntry    *dentry     = NULL;

    if (XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (path, keys, 7)) &&
        xfce_desktop_entry_get_string (dentry, "Exec", TRUE, &exec))
    {
//         saveHistory(filepath);
        if (exec)
        {
            /* Nullterminate the exec string at the first wildcard */
            if((p = strchr(exec, '%')))
                *p = 0;
            
                    
            /* filter out quotes around the command (yeah, people do that!) */
            if (exec[0] == '"') {
                gint i;
                
                for (i = 1; exec[i-1] != '\0'; ++i) {
                    if (exec[i] != '"')
                        exec[i-1] = exec[i];
                    else {
                        exec[i-1] = '\0';
                        break;
                    }
                }
            }
            
            g_printf(g_strconcat(_("Now starting"), " \"", exec, "\"...\n", NULL));
            g_spawn_command_line_async (exec, NULL);
            
            g_free(exec);
        }
    }
    else
    {
        g_warning("Something bad happened on callbackExecuteApplication with path: %s\n", path);
        xfce_info(_("Cannot execute the selected application"));
    }
}

void
callbackRunMenuActivate (GtkMenuItem *menuitem, gpointer path)
{
    callbackExecuteApplication (NULL, path, NULL);
}

void
callbackInformationMenuActivate (GtkMenuItem *menuitem, gpointer path)
{
    AfDialog *dlg;
    GdkPixbuf *icon;
    GdkPixbuf *icon2;
    gchar *iconpath = NULL;
    gchar *comment = NULL;
    gchar *cats = NULL;
    gchar *name = NULL;
    gchar *exec = NULL;
    gchar **catsarray = NULL;
    XfceDesktopEntry *dentry;

    if (path && XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (path, keys, 7)))
    {
        dlg = g_new (AfDialog, 1);
        dlg->dialog = gtk_dialog_new ();
        gtk_window_set_modal (GTK_WINDOW (dlg->dialog), TRUE);
        gtk_window_set_title (GTK_WINDOW (dlg->dialog), _("Appfinder InfoBox"));
        gtk_dialog_set_has_separator (GTK_DIALOG (dlg->dialog), FALSE);
        icon = xfce_inline_icon_at_size (default_icon_data_48_48, 32, 32);
        gtk_window_set_icon (GTK_WINDOW (dlg->dialog), icon);

        dlg->vbox = GTK_DIALOG (dlg->dialog)->vbox;
        xfce_desktop_entry_get_string (dentry, "Name", TRUE, &name);
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

        xfce_desktop_entry_get_string (dentry, "Icon", TRUE, &iconpath);
        
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

        xfce_desktop_entry_get_string (dentry, _("Comment"), TRUE, &comment);
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

        xfce_desktop_entry_get_string (dentry, "Categories", TRUE, &cats);
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

        xfce_desktop_entry_get_string (dentry, "Exec", TRUE, &exec);
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

void
callbackRightClickMenu (GtkWidget *widget, gchar *path, gpointer data)
{
    GtkWidget    *menu;
    GtkWidget    *menuitem;
    GtkWidget    *icon;
               
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
    g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(callbackRunMenuActivate), (gpointer) g_strdup(path));
    gtk_widget_show (icon);
    gtk_widget_show (menuitem);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

    menuitem = gtk_image_menu_item_new_with_label (_("Informations..."));
    icon = gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), icon);
    g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(callbackInformationMenuActivate), (gpointer) g_strdup(path));
    gtk_widget_show (icon);
    gtk_widget_show (menuitem);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

    gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

void
callbackSearchApplication (GtkEntry *entry, gpointer appfinder)
{
    xfce_appfinder_search(XFCE_APPFINDER(appfinder), g_strdup(gtk_entry_get_text(entry)));
}

void
callbackCategoriesCheck (GtkToggleButton *togglebutton, gpointer appfinder)
{
    xfce_appfinder_view_categories(XFCE_APPFINDER(appfinder), gtk_toggle_button_get_active (togglebutton));
}

gint
main (gint argc, gchar **argv)
{
    GtkWidget *afWnd;
    GtkWidget *af;
    GtkWidget *vbox;
    GtkWidget *searchBox;
    GtkWidget *searchLabel;
    GtkWidget *searchEntry;
    GtkWidget *categoriesCheck;
    
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    
//     configfile = xfce_resource_save_location (XFCE_RESOURCE_CONFIG, "xfce4" G_DIR_SEPARATOR_S CONFIGFILE, TRUE);
        
    gtk_init(&argc, &argv);
    
    afWnd = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT(afWnd), "delete_event", G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_title (GTK_WINDOW(afWnd), "Xfce4 Appfinder");
    gtk_window_set_icon (GTK_WINDOW(afWnd), xfce_inline_icon_at_size (default_icon_data_48_48, 48, 48));
    gtk_window_set_position (GTK_WINDOW(afWnd), GTK_WIN_POS_CENTER_ALWAYS);
    gtk_window_set_default_size (GTK_WINDOW(afWnd), gdk_screen_width ()/2, gdk_screen_height()/2);

    vbox = GTK_WIDGET(gtk_vbox_new (FALSE, 0));
    gtk_container_add (GTK_CONTAINER(afWnd), vbox);
    
    searchBox = GTK_WIDGET(gtk_hbox_new(FALSE, 6));
    gtk_container_set_border_width(GTK_CONTAINER(searchBox), 6);
    gtk_box_pack_start(GTK_BOX(vbox), searchBox, FALSE, TRUE, 0);
   
    af    = GTK_WIDGET(xfce_appfinder_new());  
    g_signal_connect (G_OBJECT(af), "application-activate", G_CALLBACK(callbackExecuteApplication), NULL);
    g_signal_connect (G_OBJECT(af), "application-right-click", G_CALLBACK(callbackRightClickMenu), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), af, TRUE, TRUE, 0);

    searchLabel = GTK_WIDGET(gtk_label_new(NULL));
    gtk_misc_set_alignment(GTK_MISC(searchLabel), 0.0, 0.5);
    gtk_label_set_markup_with_mnemonic(GTK_LABEL(searchLabel), _("<b>Search:</b>"));
    gtk_box_pack_start(GTK_BOX(searchBox), searchLabel, FALSE, TRUE, 0);

    searchEntry = GTK_WIDGET(gtk_entry_new());
    g_signal_connect(searchEntry, "activate", G_CALLBACK(callbackSearchApplication), (gpointer) af);
    gtk_box_pack_start(GTK_BOX(searchBox), searchEntry, TRUE, TRUE, 0);        

    categoriesCheck = gtk_check_button_new_with_label ("Show categories");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (categoriesCheck), TRUE);
    g_signal_connect (G_OBJECT(categoriesCheck), "toggled", G_CALLBACK (callbackCategoriesCheck), (gpointer) af);
    gtk_box_pack_start(GTK_BOX(searchBox), categoriesCheck, FALSE, TRUE, 0);        
            
    gtk_widget_show_all (afWnd);
    gtk_main();
    return 0;
    
}  
