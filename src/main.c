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
 *
 *  This program is dedicated to DarkAngel.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include <af-constants.h>
#include <appfinder.h>
#include <callbacks.h>
#include <inline-icon.h>

static gchar **entriespaths;

gchar *get_path_from_name(gchar *name) {
    GDir *dir;
    XfceDesktopEntry *dentry;
    gboolean found = FALSE;
    gchar *filename;
    gchar *dname;
    gchar *filepath;
    gint i = 0;

    g_return_val_if_fail(name!=NULL, NULL);

    while (entriespaths[i])
    {
        if ((dir = g_dir_open (entriespaths[i], 0, NULL)) == NULL)
        {
            i++;
            continue;
        }

        while (!found && ((filename = (gchar *)g_dir_read_name(dir)) != NULL))
        {
            filepath = g_build_filename(entriespaths[i], filename, NULL);
            if (g_file_test(filepath, G_FILE_TEST_IS_DIR) ||
                            !XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (filepath, keys, 7))) {
                    continue;
            }

            if (xfce_desktop_entry_get_string (dentry, "Name", FALSE, &dname) && dname) {
                    if (strcmp(dname, name)==0)
                            found = TRUE;
                    g_free(dname);
            }
        }
        g_dir_close(dir);
        if (found)
                return g_strdup(filepath);
        i++;
    }
    return NULL;
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
        if (exec)
        {
            if (g_strrstr(exec, "%")!= NULL)
            {
                execp = g_strsplit(exec, "%", 0);
                g_printf(g_strconcat(_("Now starting"), " \"", execp[0], "\"...\n", NULL));
                exec_command(execp[0]);
                g_strfreev (execp);
            }
            else
            {
                g_printf(g_strconcat(_("Now starting"), " \"", exec, "\"...\n", NULL));
                exec_command(exec);
            }
            g_free(exec);
        }
    }
    else
    {
        xfce_info(g_strconcat(_("Cannot execute"), " \"", name, "\"", NULL));
    }
    if (filepath)
        g_free(filepath);
}

/**********
 * create_interface
 **********/
Appfinder *create_interface(void)
{
    Appfinder *af;

    af = g_new(Appfinder, 1);
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
    gtk_label_set_markup_with_mnemonic(GTK_LABEL(af->searchlabel), _("<b>Search:</b>"));
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

GtkListStore *create_categories_liststore(void)
{
    int i = 0;
    GtkListStore  *store;
    GtkTreeIter    iter;
    
    store = gtk_list_store_new(CAT_COLS, G_TYPE_STRING);
    
    while(categories[i])
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, CAT_TEXT, _(i18ncategories[i]), -1);
        i++;
    }
    return store;
}

GtkWidget *create_categories_treeview(void)
{
    GtkTreeModel      *model;
    GtkTreeViewColumn *col;
    GtkCellRenderer   *renderer;
    GtkWidget         *view;

    model = GTK_TREE_MODEL(create_categories_liststore());

    view = gtk_tree_view_new_with_model(model);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, _("Categories"));

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_set_attributes(col, renderer,
                                        "text", CAT_TEXT,
                                        NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

    return view;
}

gboolean xfce_appfinder_list_add (XfceDesktopEntry *dentry, GtkListStore *store, GPatternSpec  *psearch, GPatternSpec  *pcat)
{
    GtkTreeIter iter;
    GdkPixbuf *icon = NULL;
    gchar *name = NULL;
    gchar *img = NULL;
    gchar *dcat = NULL;
    gchar *comment = NULL;

    if (!(xfce_desktop_entry_get_string (dentry, "Name", FALSE, &name) && name))
    {
            return FALSE;
    }

    if (pcat)
    {
        if (!(xfce_desktop_entry_get_string (dentry, "Categories", FALSE, &dcat) && dcat))
        {
            return FALSE;
        }
        
        if (g_pattern_match_string (pcat, g_utf8_casefold(dcat, -1)) == FALSE)
        {
            g_free(dcat);
            return FALSE;
        }
    }
    
    if (psearch)
    {
        xfce_desktop_entry_get_string (dentry, "Comment", FALSE, &comment);
        if (!(comment && g_pattern_match_string (psearch, g_utf8_casefold(comment, -1))) &&
                    !g_pattern_match_string (psearch, name))
        {
            return FALSE;
        }
    }

    if (xfce_desktop_entry_get_string (dentry, "Icon", FALSE, &img) && img)
    {
        icon = xfce_themed_icon_load(img, 24);
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

    g_free(name);
    if (icon)
    {
        g_object_unref (icon);
    }

    return TRUE;
}

/*
 * This function handles all the searches into desktop files
 *
 * @param category - the category to search for (defined into the array in the header)
 * @param pattern - the pattern of the text to search for (set to NULL if any text is ok)
 * @returns GtkListStore * - a pointer to a new list store with the items
 */
GtkListStore *fetch_desktop_resources (gint category, gchar *pattern) {
    XfceDesktopEntry *dentry;
    GtkListStore  *store;
    GPatternSpec  *psearch = NULL;
    GPatternSpec  *pcat = NULL;
    gchar *tmp;
    GDir *dir;
    gchar *filename;
    gchar *fullpath;
    gint n = npaths - 1;
    gint i = 0; /* A counter for general use */

    store = gtk_list_store_new(APP_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);
    if (category==APPFINDER_HISTORY)
    {
        /* We load data from appfinder' rc file */
        gchar **history = parseHistory();
        if (history!=NULL)
        {
            while (history[i]!=NULL)
            {
                if (g_file_test(history[i], G_FILE_TEST_EXISTS) &&
                                XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (history[i], keys, 7))) 
                {
                    xfce_appfinder_list_add (dentry, store, NULL, FALSE);
                    if (dentry)
                    {
                        g_object_unref (dentry);
                    }
                }
                i++;
            }
            
            if (history)
            {
                g_strfreev(history);
            }
        }
    }
    else
    {
        if (pattern != NULL)
        {
            tmp = g_strconcat("*", g_utf8_casefold(pattern, -1), "*", NULL);
            psearch = g_pattern_spec_new (tmp);
            g_free(tmp);
        }

        if (category!=APPFINDER_ALL)
        {
            tmp = g_strconcat("*", g_utf8_casefold(categories[category], -1), "*", NULL);
            pcat = g_pattern_spec_new (tmp);
            g_free(tmp);
        }

        while (entriespaths[i]!=NULL)
        {
            if ((dir = g_dir_open (entriespaths[i], 0, NULL))!=NULL)
            {
                while ((filename = (gchar *)g_dir_read_name(dir))!=NULL)
                {
                    fullpath = g_build_filename(entriespaths[i], filename, NULL);
                    if (g_str_has_suffix(filename, ".desktop"))
                    {
                        dentry = xfce_desktop_entry_new (fullpath, keys, 7);
                        if (!XFCE_IS_DESKTOP_ENTRY(dentry))
                        {
                            continue;
                        }
                        xfce_appfinder_list_add (dentry, store, psearch, pcat);
                        g_object_unref (dentry);
                        g_free(fullpath);
                    }
                    else if (g_file_test(fullpath, G_FILE_TEST_IS_DIR))
                    {
                        entriespaths[n] = fullpath;
                        entriespaths[n] = NULL;
                    }
                }
                g_dir_close(dir);
            }
            i++;
        }
    }

    if (psearch)
    {
        g_pattern_spec_free (psearch);
    }
    
    if (pcat)
    {
        g_pattern_spec_free (pcat);
    }

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

static void
build_paths (void)
{
    gchar *kdedir;
    gchar **applications;
    gint    napplications;
    gchar **apps;
    gint    napps;
    gchar **applnk;
    gint    napplnk;
    gint    i, n;
    
    applications = xfce_resource_lookup_all (XFCE_RESOURCE_DATA, "applications/");
    for (napplications = 0; applications[napplications] != NULL; ++napplications);
    
    apps = xfce_resource_lookup_all (XFCE_RESOURCE_DATA, "apps/");
    for (napps = 0; apps[napps] != NULL; ++napps);
    
    applnk = xfce_resource_lookup_all (XFCE_RESOURCE_DATA, "applnk/");
    for (napplnk = 0; applnk[napplnk] != NULL; ++napplnk);
    
    entriespaths = g_new0 (gchar *, 2 * napplications + napps + napplnk + 4);
    i = 0;
    
    entriespaths[i++] = xfce_get_homefile (".kde", "share", "apps", NULL);
    entriespaths[i++] = xfce_get_homefile (".kde", "share", "applnk", NULL);
    if (kdedir = (gchar *)getenv("KDEDIR"))
    {
        entriespaths[i++] = g_build_filename (kdedir, "share/applications/kde", NULL); 
    }
    
    for (n = 0; n < napplications; ++n)
    {
        entriespaths[i++] = applications[n];
        entriespaths[i++] = g_build_filename (applications[n], "kde", NULL);
    }
    g_free (applications);
    
    for (n = 0; n < napps; ++i, ++n)
    {
        entriespaths[i] = apps[n];
    }
    g_free (apps);
    
    for (n = 0; n < napplnk; ++i, ++n)
    {
        entriespaths[n] = applnk[n];
    }
    g_free (applnk);
    
    g_print ("\nPATHS:\n");
    for (n = 0; entriespaths[n] != NULL; ++n)
    {
        g_print ("  %s\n", entriespaths[n]);
    }
    npaths = n;
    g_print ("\n\n");
}


gint
main (gint argc, gchar **argv)
{
    Appfinder *appfinder;
    
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    gtk_init(&argc, &argv);
    build_paths ();
    configfile = xfce_resource_save_location (XFCE_RESOURCE_CONFIG,
                                    "xfce4" G_DIR_SEPARATOR_S CONFIGFILE, 
                                    TRUE);
    appfinder = create_interface();
    gtk_main();
    return 0;
}
