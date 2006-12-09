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
#define BORDER  8

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
            
            DBG ("Now starting \"%s\"",  exec);
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
callbackRunMenuActivate (GtkMenuItem *menuitem, gpointer menu)
{
    gchar *path = g_object_get_data (G_OBJECT (menu), "path");
    
    callbackExecuteApplication (NULL, path, NULL);

    g_free (path);
}

void
callbackInformationMenuActivate (GtkMenuItem *menuitem, gpointer menu)
{
    GtkWidget *dialog;
    GtkWidget *header;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *frame;
    GtkWidget *img;
    GtkWidget *table;
    GtkWidget *label;
    GdkPixbuf *icon;
    GdkPixbuf *icon2;
    gchar *iconpath = NULL;
    gchar *comment = NULL;
    gchar *cats = NULL;
    gchar *name = NULL;
    gchar *exec = NULL;
    gchar *path;
    gchar **catsarray = NULL;
    XfceDesktopEntry *dentry;

    path = g_object_get_data (G_OBJECT (menu), "path");
    
    DBG ("Information about: %s", (char *)path);
    
    if (path && XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (path, keys, 7)))
    {
        dialog = gtk_dialog_new_with_buttons (_("Xfce 4 Appfinder"), NULL, 
                                              GTK_DIALOG_NO_SEPARATOR,
                                              GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
                                              NULL);
		icon = xfce_themed_icon_load("xfce4-appfinder", 48);
        gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-appfinder");
		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

        vbox = GTK_DIALOG (dialog)->vbox;
        gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);

        xfce_desktop_entry_get_string (dentry, "Name", TRUE, &name);
        header = xfce_create_header (icon, name);
        gtk_container_set_border_width (GTK_CONTAINER (header), BORDER - 2);
        gtk_widget_show (header);
        gtk_box_pack_start (GTK_BOX (vbox), header, FALSE, TRUE, 0);
        g_object_unref(icon);

        hbox = gtk_hbox_new(FALSE, BORDER);
        gtk_container_set_border_width (GTK_CONTAINER (hbox), BORDER - 2);
        gtk_widget_show(hbox);
        gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

        frame = gtk_frame_new (NULL);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
        gtk_widget_show(frame);
        gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);

        xfce_desktop_entry_get_string (dentry, "Icon", TRUE, &iconpath);
        
        if (iconpath)
        {
            icon = xfce_themed_icon_load(iconpath, 48);
            if (!icon)
            {
                icon = xfce_themed_icon_load("xfce4-appfinder", 48);
                icon2 = gdk_pixbuf_copy (icon);
                gdk_pixbuf_saturate_and_pixelate(icon, icon2, 0.0, TRUE);
                g_object_unref(icon);
                icon = icon2;
            }
            g_free(iconpath);
        }
        else
        {
            xfce_themed_icon_load("xfce4-appfinder", 48);
        }

        img = gtk_image_new_from_pixbuf (icon);
        gtk_misc_set_padding (GTK_MISC (img), 4, 4);
        gtk_widget_show(img);
        gtk_container_add (GTK_CONTAINER (frame), img);

        /* table */
        table = gtk_table_new (4, 2, FALSE);
        gtk_table_set_row_spacings (GTK_TABLE (table), BORDER / 2);
        gtk_table_set_col_spacings (GTK_TABLE (table), BORDER);
        gtk_widget_show (table);
        gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);

        /* name */
        label = gtk_label_new(NULL);
        gtk_misc_set_alignment (GTK_MISC(label), 0, 0);
        gtk_widget_show(label);
        gtk_table_attach_defaults (GTK_TABLE (table), label,
                                   0, 1, 0, 1);
        
        gtk_label_set_markup (GTK_LABEL(label), _("<b>Name</b>"));

        label = gtk_label_new(NULL);
        gtk_misc_set_alignment (GTK_MISC(label), 0, 0);
        gtk_widget_show(label);
        gtk_table_attach_defaults (GTK_TABLE (table), label,
                                   1, 2, 0, 1);

        gtk_label_set_markup (GTK_LABEL(label), name);
        

        /* comment */
        xfce_desktop_entry_get_string (dentry, "Comment", TRUE, &comment);
        if (!comment)
        {
            comment = g_strdup (_("N/A"));
        }

        label = gtk_label_new(NULL);
        gtk_misc_set_alignment (GTK_MISC(label), 0, 0);
        gtk_widget_show(label);
        gtk_table_attach_defaults (GTK_TABLE (table), label,
                                   0, 1, 1, 2);
        
        gtk_label_set_markup (GTK_LABEL(label), _("<b>Comment</b>"));

        label = gtk_label_new(NULL);
        gtk_misc_set_alignment (GTK_MISC(label), 0, 0);
        gtk_widget_show(label);
        gtk_table_attach_defaults (GTK_TABLE (table), label,
                                   1, 2, 1, 2);

        gtk_label_set_markup (GTK_LABEL(label), comment);
        
        g_free (comment);

        /* categories */
        xfce_desktop_entry_get_string (dentry, "Categories", TRUE, &cats);
        if (!cats)
        {
            cats = g_strdup (_("N/A"));
        }
        else
        {
            catsarray = g_strsplit (cats, ";", 0);
            g_free (cats);
            cats = g_strchomp (g_strjoinv (", ", catsarray));
            cats[strlen(cats)-1] = '\0';
            g_strfreev (catsarray);
        }

        label = gtk_label_new(NULL);
        gtk_misc_set_alignment (GTK_MISC(label), 0, 0);
        gtk_widget_show(label);
        gtk_table_attach_defaults (GTK_TABLE (table), label,
                                   0, 1, 2, 3);
        
        gtk_label_set_markup (GTK_LABEL(label), _("<b>Categories</b>"));

        label = gtk_label_new(NULL);
        gtk_misc_set_alignment (GTK_MISC(label), 0, 0);
        gtk_widget_show(label);
        gtk_table_attach_defaults (GTK_TABLE (table), label,
                                   1, 2, 2, 3);

        gtk_label_set_markup (GTK_LABEL(label), cats);
        
        g_free (cats);

        /* exec */
        xfce_desktop_entry_get_string (dentry, "Exec", TRUE, &exec);
        if (!exec)
        {
            exec = g_strdup (_("N/A"));
        }

        label = gtk_label_new(NULL);
        gtk_misc_set_alignment (GTK_MISC(label), 0, 0);
        gtk_widget_show(label);
        gtk_table_attach_defaults (GTK_TABLE (table), label,
                                   0, 1, 3, 4);
        
        gtk_label_set_markup (GTK_LABEL(label), _("<b>Command</b>"));

        label = gtk_label_new(NULL);
        gtk_misc_set_alignment (GTK_MISC(label), 0, 0);
        gtk_widget_show(label);
        gtk_table_attach_defaults (GTK_TABLE (table), label,
                                   1, 2, 3, 4);

        gtk_label_set_markup (GTK_LABEL(label), exec);
        
        g_free (exec);

        g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
                                  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT (dialog));
                                
        gtk_widget_show(dialog);
        g_free(name);

        g_object_unref (dentry);
    }

    g_free (path);
}

void
callbackRightClickMenu (GtkWidget *widget, gchar *path, gpointer data)
{
    GtkWidget    *menu;
    GtkWidget    *menuitem;
    GtkWidget    *icon;

    DBG ("Open menu for: %s", path);

    menu = gtk_menu_new();

    menuitem = gtk_image_menu_item_new_with_label (_("Run program"));
    icon = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), icon);
    g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(callbackRunMenuActivate), menu);
    gtk_widget_show (icon);
    gtk_widget_show (menuitem);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

    menuitem = gtk_image_menu_item_new_with_label (_("More Information..."));
    icon = gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), icon);
    g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(callbackInformationMenuActivate), menu);
    gtk_widget_show (icon);
    gtk_widget_show (menuitem);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

    g_object_set_data (G_OBJECT (menu), "path", g_strdup (path));
    gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

void
callbackSearchApplication (GtkEntry *entry, gpointer appfinder)
{
    xfce_appfinder_search(XFCE_APPFINDER(appfinder), gtk_entry_get_text(entry));
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
    gtk_window_set_icon_name (GTK_WINDOW (afWnd), "xfce4-appfinder");
    //gtk_window_set_icon (GTK_WINDOW(afWnd), xfce_inline_icon_at_size (default_icon_data_48_48, 48, 48));
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

    categoriesCheck = gtk_check_button_new_with_label (_("Show Categories"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (categoriesCheck), TRUE);
    g_signal_connect (G_OBJECT(categoriesCheck), "toggled", G_CALLBACK (callbackCategoriesCheck), (gpointer) af);
    gtk_box_pack_start(GTK_BOX(searchBox), categoriesCheck, FALSE, TRUE, 0);        
            
    gtk_widget_show_all (afWnd);
    gtk_main();
    return 0;
    
}  
