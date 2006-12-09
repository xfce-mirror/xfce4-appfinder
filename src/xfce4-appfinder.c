/*  xfce4-appfinder
 *
 *  Copyright (C) 2004-2006 Eduard Roccatello (eduard@xfce.org)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <string.h>
#include "xfce4-appfinder.h"

enum {
    APPLICATION_ACTIVATE_SIGNAL,
    APPLICATION_RIGHT_CLICK_SIGNAL,
    LAST_SIGNAL
};

typedef struct _XfceAppfinderListParam XfceAppfinderListParam;

struct _XfceAppfinderListParam
{
    GtkListStore         *store;
    GPatternSpec         *psearch;
    GPatternSpec         *pcat;
};

static void           xfce_appfinder_class_init      (XfceAppfinderClass     *klass);

static void           xfce_appfinder_init            (XfceAppfinder          *appfinder);

static GtkWidget*     create_categories_treeview     (void);

static GtkWidget*     create_applications_treeview   (XfceAppfinder          *appfinder);

static GtkListStore*  load_desktop_resources         (gint                    category, 
                                                      gchar                  *pattern,
                                                      XfceAppfinder          *appfinder);

                                               
static void           xfce_appfinder_list_add        (gchar                   *name,
                                                      XfceAppfinderCacheEntry *entry,
                                                      XfceAppfinderListParam  *param);
                                               
static void           build_desktop_paths            (void);

static gchar *        get_path_from_name             (gchar                  *name,
                                                      XfceAppfinder          *appfinder);

static void           callbackApplicationActivate    (GtkTreeView            *treeview,
                                                      GtkTreePath            *path,
                                                      GtkTreeViewColumn      *col,
                                                      gpointer                appfinder);
                                               
static gboolean       callbackCategoryTreeClick      (GtkTreeSelection       *selection,
                                                      GtkTreeModel           *model,
                                                      GtkTreePath            *path,
                                                      gboolean                path_currently_selected,
                                                      gpointer                userdata);

static gboolean       callbackApplicationRightClick  (GtkWidget              *treeview,
                                                      GdkEventButton         *event,
                                                      gpointer                appfinder);

static void           callbackDragFromAppsTree       (GtkWidget              *widget,
                                                      GdkDragContext         *dragContext,
                                                      GtkSelectionData       *data,
                                                      guint                   info,
                                                      guint                   time,
                                                      gpointer                appfinder);

static gint           sort_compare_func              (GtkTreeModel           *model,
                                                      GtkTreeIter            *a,
                                                      GtkTreeIter            *b,
                                                      gpointer                userdata);
                                               
static GHashTable *   createDesktopCache             ();
                                               
                                                                                              
static gint           xfce_appfinder_signals[LAST_SIGNAL] = { 0 };
static GPtrArray *    desktop_entries_paths;
static gint           showedcat = APPFINDER_ALL;

/* What to search for in .desktop files */
static const char *dotDesktopKeys [] =
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

static const char *dotDesktopCategories [] =
{
    N_("All"),
    N_("Core"),
    N_("Development"),
    N_("Office"),
    N_("Graphics"),
    N_("Network"),
    N_("AudioVideo"),
    N_("Game"),
    N_("Education"),
    N_("System"),
    N_("Filemanager"),
    N_("Utility"),
    NULL
};

/* Places where i can drop things */
static const GtkTargetEntry dotDesktopDropTarget[] =
{
    {"DESKTOP_PATH_ENTRY", 0, 0},
    {"text/plain", 0, 1},
    {"application/x-desktop", 0, 2},
    {"STRING", 0, 3},
    {"UTF8_STRING", 0, 4},
    {"text/uri-list", 0, 5}
};

GType
xfce_appfinder_get_type ()
{
    static GType appfinder_type = 0;
    
    if (!appfinder_type)
    {
        static const GTypeInfo appfinder_info =
        {
            sizeof(XfceAppfinderClass),
            NULL,
            NULL,
            (GClassInitFunc) xfce_appfinder_class_init,
            NULL,
            NULL,
            sizeof(XfceAppfinder),
            0,
            (GInstanceInitFunc) xfce_appfinder_init,
        };
        
        appfinder_type = g_type_register_static (GTK_TYPE_VBOX, "XfceAppfinder", &appfinder_info, 0);
    }
    
    return appfinder_type;
}

static void
xfce_appfinder_class_init (XfceAppfinderClass *class)
{
    GtkObjectClass *object_class;
    
    object_class = (GtkObjectClass*) class;
    
    xfce_appfinder_signals[APPLICATION_ACTIVATE_SIGNAL] = g_signal_new("application-activate",
                                                                G_TYPE_FROM_CLASS(object_class),
                                                                G_SIGNAL_RUN_FIRST,
                                                                0,
                                                                NULL,
                                                                NULL,
                                                                g_cclosure_marshal_VOID__POINTER,
                                                                G_TYPE_NONE,
                                                                1,
                                                                G_TYPE_POINTER);
                                                                
    xfce_appfinder_signals[APPLICATION_RIGHT_CLICK_SIGNAL] = g_signal_new("application-right-click",
                                                                G_TYPE_FROM_CLASS(object_class),
                                                                G_SIGNAL_RUN_FIRST,
                                                                0,
                                                                NULL,
                                                                NULL,
                                                                g_cclosure_marshal_VOID__POINTER,
                                                                G_TYPE_NONE,
                                                                1,
                                                                G_TYPE_POINTER);
    class->xfce_appfinder = NULL;                        
    build_desktop_paths ();
}

static void
xfce_appfinder_init (XfceAppfinder *appfinder)
{
    appfinder->cache = createDesktopCache();
    
    appfinder->hpaned = GTK_WIDGET(gtk_hpaned_new ());
    gtk_paned_set_position (GTK_PANED(appfinder->hpaned), 100);
    gtk_container_add(GTK_CONTAINER(appfinder), appfinder->hpaned);
    
    appfinder->categoriesTree = create_categories_treeview();
    gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(GTK_TREE_VIEW(appfinder->categoriesTree)), callbackCategoryTreeClick, appfinder, NULL);
    gtk_paned_pack1(GTK_PANED(appfinder->hpaned), appfinder->categoriesTree, TRUE, TRUE);
    
    appfinder->appScroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(appfinder->appScroll), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(appfinder->appScroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_paned_pack2(GTK_PANED(appfinder->hpaned), appfinder->appScroll, TRUE, TRUE);    
    
    appfinder->appsTree = create_applications_treeview(appfinder);
    gtk_tree_view_enable_model_drag_source( GTK_TREE_VIEW(appfinder->appsTree),
                                            GDK_BUTTON1_MASK, dotDesktopDropTarget,
                                            6, GDK_ACTION_COPY);

    g_signal_connect(appfinder->appsTree, "row-activated", G_CALLBACK(callbackApplicationActivate), (gpointer) appfinder);
    g_signal_connect(appfinder->appsTree, "button-press-event", G_CALLBACK(callbackApplicationRightClick), (gpointer) appfinder);    
    g_signal_connect(appfinder->appsTree, "drag-data-get", G_CALLBACK(callbackDragFromAppsTree), (gpointer) appfinder);

    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(appfinder->appScroll), appfinder->appsTree);        
    gtk_widget_show_all(GTK_WIDGET(appfinder));
}

static void
callbackApplicationActivate (GtkTreeView        *treeview,
                             GtkTreePath        *path,
                             GtkTreeViewColumn  *col,
                             gpointer            appfinder)
{
    gchar *filePath = NULL;
    gchar *name = NULL;
    GtkTreeModel *model;
    GtkTreeIter   iter;
    
    model = gtk_tree_view_get_model(treeview);
    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        /* we fetch the name of the application to run */
        gtk_tree_model_get(model, &iter, APPLICATION_TREE_TEXT, &name, -1);
        if (name)
        {
            filePath = get_path_from_name (name, appfinder);
            g_free(name);
        }
    }
    if (filePath)
    {
        g_signal_emit (G_OBJECT (appfinder), xfce_appfinder_signals[APPLICATION_ACTIVATE_SIGNAL], 0, filePath);
        g_free(filePath);
    }
}

GtkWidget*
xfce_appfinder_new ()
{
    return GTK_WIDGET (g_object_new(xfce_appfinder_get_type(), NULL));
}

static GtkWidget*
create_categories_treeview (void)
{
    GtkTreeViewColumn *col;
    GtkCellRenderer   *renderer;
    GtkWidget         *view;
    GtkListStore      *store;
    GtkTreeIter        iter;
    gint               i = 0;
    
    store = gtk_list_store_new(CATEGORY_TREE_COLS, G_TYPE_STRING);
    
    while(dotDesktopCategories[i])
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, CATEGORY_TREE_TEXT, _(dotDesktopCategories[i++]), -1);
    }

    view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, _("Categories"));

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_set_attributes(col, renderer,
                                        "text", CATEGORY_TREE_TEXT,
                                        NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

    return view;
}

static gint
sort_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer userdata)
{
    gint ret = 0;
    gchar *name1, *name2;

    gtk_tree_model_get(model, a, APPLICATION_TREE_TEXT, &name1, -1);
    gtk_tree_model_get(model, b, APPLICATION_TREE_TEXT, &name2, -1);

    if (name1 == NULL || name2 == NULL)
    {
        if (name1 == NULL && name2 == NULL)
        return 0;

        ret = (name1 == NULL) ? -1 : 1;
    }
    else
    {
        ret = g_utf8_collate(name1, name2);
    }

    g_free(name1);
    g_free(name2);

    return ret;
}

static GtkWidget*
create_applications_treeview (XfceAppfinder *appfinder)
{
    GtkTreeViewColumn *col;
    GtkTreeSortable   *sortable;
    GtkListStore      *liststore;
    GtkCellRenderer   *renderer;
    GtkWidget         *view;

    liststore = load_desktop_resources(0, NULL, appfinder);
    sortable = GTK_TREE_SORTABLE(liststore);

    gtk_tree_sortable_set_sort_func(sortable, APPLICATION_TREE_TEXT, sort_compare_func, GINT_TO_POINTER(APPLICATION_TREE_TEXT), NULL);

    gtk_tree_sortable_set_sort_column_id(sortable, APPLICATION_TREE_TEXT, GTK_SORT_ASCENDING);
    
    view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(liststore));
    
    col = gtk_tree_view_column_new();
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(col, renderer, FALSE);
    gtk_tree_view_column_set_attributes(col, renderer,
                                        "pixbuf", APPLICATION_TREE_ICON,
                                        NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_set_attributes(col, renderer,
                                        "text", APPLICATION_TREE_TEXT,
                                        NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

    return view;
}

static void
xfce_appfinder_list_add (gchar *name, XfceAppfinderCacheEntry *entry, XfceAppfinderListParam *param)
{
    GtkTreeIter     iter;
    GdkPixbuf      *icon = NULL;

    if (param->pcat && (g_pattern_match_string (param->pcat, g_utf8_casefold(entry->categories, -1)) == FALSE))
    {
            return;
    }
    
    if (param->psearch)
    {
        if (!((entry->comment && g_pattern_match_string (param->psearch, g_utf8_casefold(entry->comment, -1))) ||
               (entry->exec && g_pattern_match_string (param->psearch, g_utf8_casefold(entry->exec, -1))) ||
                g_pattern_match_string (param->psearch, g_utf8_casefold(name, -1))
              ))
        {
            return;
        }
    }
    
    if (entry->icon)
        icon = xfce_themed_icon_load(entry->icon, 24);
           
    gtk_list_store_append(param->store, &iter);
    gtk_list_store_set(param->store, &iter,
                       APPLICATION_TREE_ICON, icon,
                       APPLICATION_TREE_TEXT, name,
                       -1);
}

/**
 * This function handles all the searches into desktop files
 *
 * @param category - the category to search for (defined into the array in the header)
 * @param pattern - the pattern of the text to search for (set to NULL if any text is ok)
 * @returns GtkListStore * - a pointer to a new list store with the items
 */
static GtkListStore*
load_desktop_resources (gint category, gchar *pattern, XfceAppfinder *appfinder)
{
    XfceAppfinderListParam      *param     = NULL;
    GtkListStore                *store     = NULL;
    GPatternSpec                *psearch   = NULL;
    GPatternSpec                *pcat      = NULL;
    gchar                       *tmp       = NULL;

    store = gtk_list_store_new(APPLICATION_TREE_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);

    if (pattern != NULL)
    {
        tmp = g_strconcat("*", pattern, "*", NULL);
        psearch = g_pattern_spec_new (tmp);
        g_free(tmp);
    }

    if (category != APPFINDER_ALL)
    {
        tmp = g_strconcat("*", g_utf8_casefold(dotDesktopCategories[category], -1), "*", NULL);
        pcat = g_pattern_spec_new (tmp);
        g_free(tmp);
    }
    
    
    param = g_new(XfceAppfinderListParam, 1);
    param->store = store;
    param->psearch = psearch;
    param->pcat = pcat;
    
    g_hash_table_foreach (appfinder->cache, (GHFunc)xfce_appfinder_list_add, param);

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

static void
build_desktop_paths (void)
{
    const gchar *kdedir;
    gchar **applications;
    gint    napplications;
    gchar **apps;
    gint    napps;
    gchar **applnk;
    gint    napplnk;
    gint    n;
    
    applications = xfce_resource_lookup_all (XFCE_RESOURCE_DATA, "applications/");
    
    apps = xfce_resource_lookup_all (XFCE_RESOURCE_DATA, "apps/");
    
    applnk = xfce_resource_lookup_all (XFCE_RESOURCE_DATA, "applnk/");
    
    desktop_entries_paths = g_ptr_array_new ();
    
    g_ptr_array_add (desktop_entries_paths, (gpointer) xfce_get_homefile (".gnome", "share", "apps", NULL));
    g_ptr_array_add (desktop_entries_paths,  (gpointer) xfce_get_homefile (".kde", "share", "applnk", NULL));
    if ((kdedir = g_getenv("KDEDIR")) != NULL)
    {
        g_ptr_array_add (desktop_entries_paths, (gpointer) g_build_filename (kdedir, "share",  "applications",  "kde", NULL)); 
    }

    /* FreeBSD Gnome stuff */
    g_ptr_array_add (desktop_entries_paths, (gpointer) g_build_filename ("/usr", "X11R6", "share", "gnome", "applications", NULL));

    /* /usr/global stuff */
    g_ptr_array_add (desktop_entries_paths, (gpointer) g_build_filename ("/usr", "global", "share", "applications", NULL));
    
    for (n = 0; applications[n] != NULL; ++n)
    {
        g_ptr_array_add (desktop_entries_paths, (gpointer) applications[n]);
        g_ptr_array_add (desktop_entries_paths, (gpointer) g_build_filename (applications[n], "kde", NULL));
    }
    g_free (applications);
    
    for (n = 0; apps[n] != NULL; ++n)
    {
        g_ptr_array_add (desktop_entries_paths, (gpointer) apps[n]);
    }
    g_free (apps);
    
    for (n = 0; applnk[n] != NULL; ++n)
    {
        g_ptr_array_add (desktop_entries_paths, (gpointer) applnk[n]);
    }
    g_free (applnk);
}

static gchar *get_path_from_name(gchar *name, XfceAppfinder *appfinder)
{
    XfceAppfinderCacheEntry *entry;

    g_return_val_if_fail(name != NULL, NULL);
    g_return_val_if_fail(appfinder != NULL, NULL);
    entry = g_hash_table_lookup (appfinder->cache, name);
    
    if (entry)
    {
        return g_strdup(entry->path);
    }
    
    return NULL;
}

static gboolean
callbackCategoryTreeClick  (GtkTreeSelection *selection,
                            GtkTreeModel     *model,
                            GtkTreePath      *path,
                            gboolean          path_currently_selected,
                            gpointer          userdata)
{
    int next = showedcat;
    int i = 0;
    GtkTreeIter iter;
    gchar *name = NULL;
    XfceAppfinder *af = userdata;

    if (!path_currently_selected && gtk_tree_model_get_iter(model, &iter, path))
    {
        gtk_tree_model_get(model, &iter, CATEGORY_TREE_TEXT, &name, -1);
        if (name)
        {
            while (dotDesktopCategories[i])
            {
                if (strcmp(_(dotDesktopCategories[i]), name) == 0)
                {
                    next = i;
                    break;
                }
                i++;
            }
            g_free(name);
        }
    }
    
    if (next != showedcat)
    {
        GtkTreeSortable   *sortable;
        GtkListStore      *liststore;
        
        showedcat = next;
        liststore = GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(af->appsTree)));
        gtk_list_store_clear (liststore);
        g_object_unref (liststore);

        liststore = load_desktop_resources(showedcat, NULL, af);
        sortable = GTK_TREE_SORTABLE(liststore);
    
        gtk_tree_sortable_set_sort_func(sortable, APPLICATION_TREE_TEXT, sort_compare_func, GINT_TO_POINTER(APPLICATION_TREE_TEXT), NULL);
    
        gtk_tree_sortable_set_sort_column_id(sortable, APPLICATION_TREE_TEXT, GTK_SORT_ASCENDING);
        gtk_tree_view_set_model (GTK_TREE_VIEW(af->appsTree), GTK_TREE_MODEL(liststore));
    
        /* Ok there are no items in the list. Write a message and disable the treeview */
        if (!gtk_tree_model_get_iter_first (gtk_tree_view_get_model (GTK_TREE_VIEW(af->appsTree)), &iter))
        {
            gtk_list_store_append(GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(af->appsTree))), &iter);
            gtk_list_store_set(GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(af->appsTree))),
                                &iter, APPLICATION_TREE_ICON, xfce_themed_icon_load("xfce4-appfinder", 24),
                                APPLICATION_TREE_TEXT, _("No items available"), -1);
            gtk_widget_set_sensitive(af->appsTree, FALSE);
        }
        else
        {
            gtk_widget_set_sensitive(af->appsTree, TRUE);
        }
    }
    
    return TRUE; /* allow selection state to change */
}

void
xfce_appfinder_search (XfceAppfinder *appfinder, const gchar *pattern)
{
    GtkTreeSortable   *sortable;
    GtkListStore      *liststore;    
    GtkTreeIter iter;
    gchar *text = g_utf8_casefold(pattern, -1);
    showedcat = APPFINDER_ALL;
    
    liststore = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(appfinder->appsTree)));
    gtk_list_store_clear (liststore);
    g_object_unref (liststore);

    liststore = load_desktop_resources(APPFINDER_ALL, text, appfinder);
    sortable = GTK_TREE_SORTABLE(liststore);

    gtk_tree_sortable_set_sort_func(sortable, APPLICATION_TREE_TEXT, sort_compare_func, GINT_TO_POINTER(APPLICATION_TREE_TEXT), NULL);

    gtk_tree_sortable_set_sort_column_id(sortable, APPLICATION_TREE_TEXT, GTK_SORT_ASCENDING);
 
    gtk_tree_view_set_model (GTK_TREE_VIEW(appfinder->appsTree), GTK_TREE_MODEL(liststore));

    /* No application found. Tell the user about it */
    if (!gtk_tree_model_get_iter_first (gtk_tree_view_get_model (GTK_TREE_VIEW(appfinder->appsTree)), &iter))
    {
        gtk_list_store_append(GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(appfinder->appsTree))), &iter);
        gtk_list_store_set(GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(appfinder->appsTree))),
                            &iter, APPLICATION_TREE_ICON, xfce_themed_icon_load("xfce4-appfinder", 24),
                            APPLICATION_TREE_TEXT, _("Sorry, no match for searched text."), -1);
        gtk_widget_set_sensitive(appfinder->appsTree, FALSE);
    }
    else
    {
        gtk_widget_set_sensitive(appfinder->appsTree, TRUE);
    }

    gtk_tree_selection_unselect_all (gtk_tree_view_get_selection(GTK_TREE_VIEW(appfinder->categoriesTree)));
    
    if (text)
    {
        g_free(text);
    }
    
}

void
xfce_appfinder_clean (XfceAppfinder  *appfinder)
{
    GtkListStore *liststore;
    
    showedcat = APPFINDER_ALL;
    
    gtk_widget_set_sensitive(appfinder->appsTree, TRUE);
    
    liststore = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(appfinder->appsTree)));
    gtk_list_store_clear (liststore);
    g_object_unref (liststore);
    
    gtk_tree_view_set_model (GTK_TREE_VIEW(appfinder->appsTree), GTK_TREE_MODEL(load_desktop_resources (APPFINDER_ALL, NULL, appfinder)));
}

void
xfce_appfinder_view_categories (XfceAppfinder *appfinder, gboolean visible)
{
    if (!visible)
    {
        gtk_widget_hide(appfinder->categoriesTree);
        xfce_appfinder_clean (appfinder);
    }
    else
        gtk_widget_show_all(appfinder->categoriesTree);
}

static gboolean
callbackApplicationRightClick (GtkWidget *treeview, GdkEventButton *event, gpointer appfinder)
{
    GtkTreeSelection    *selection;
    GtkTreePath         *treepath;
    GtkTreeModel        *treemodel;
    GtkTreeIter          iter;
    gchar               *name = NULL;
    gchar               *filePath = NULL;
    
    /* 3 is for right button */
    if (event->button == 3)
    {
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
        gtk_tree_selection_unselect_all(selection);
        if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW(treeview), event->x, event->y, &treepath, NULL, NULL, NULL))
        {
            gtk_tree_selection_select_path (selection, treepath);
            treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
            gtk_tree_model_get_iter (treemodel, &iter, treepath);
            gtk_tree_model_get(treemodel, &iter, APPLICATION_TREE_TEXT, &name, -1);

            if (name)
            {
                filePath = get_path_from_name (name, appfinder);
                g_free(name);
            }
            
            if (filePath)
            {
                g_signal_emit (G_OBJECT (appfinder), xfce_appfinder_signals[APPLICATION_RIGHT_CLICK_SIGNAL], 0, filePath);
                g_free(filePath);
            }
            
            return TRUE;
        }
    }
    /* If hasn't been clicked with right button let's propagate the event */
    return FALSE;
}

static void
callbackDragFromAppsTree (GtkWidget *widget, GdkDragContext *dragContext,
                          GtkSelectionData *data, guint info,
                          guint time, gpointer appfinder)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;
    gchar *name = NULL;
    gchar *path = NULL;
    
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
        if (gtk_tree_model_get_iter(model, &iter,
                    gtk_tree_row_reference_get_path(g_object_get_data(G_OBJECT(dragContext), "gtk-tree-view-source-row"))))
        {
            gtk_tree_model_get(model, &iter, APPLICATION_TREE_TEXT, &name, -1);
            if (name)
            {
                if ((path = get_path_from_name(name, XFCE_APPFINDER(appfinder))) != NULL)
                {
                    gtk_selection_data_set (data, gdk_atom_intern ("text/plain", FALSE), 8, (guchar *)path, strlen(path));
                    g_free(path);
                }
                g_free(name);
            }
        }
}



static GHashTable *
createDesktopCache()
{
    XfceAppfinderCacheEntry  *cent      = NULL;
    XfceDesktopEntry         *dentry;    
    GHashTable               *hash;
    gchar                    *filename  = NULL;
    gchar                    *fullpath  = NULL;
    GDir                     *dir;
    gint                      i         = 0; /* A counter for general use */
    
    hash = g_hash_table_new ((GHashFunc) g_str_hash, (GEqualFunc) g_str_equal);
    
    while (i<desktop_entries_paths->len)
    {
        if ((dir = g_dir_open ((gchar *) g_ptr_array_index(desktop_entries_paths,i), 0, NULL))!=NULL)
        {
            while ((filename = (gchar *)g_dir_read_name(dir))!=NULL)
            {
                fullpath = g_build_filename(g_ptr_array_index(desktop_entries_paths,i), filename, NULL);
                if (g_str_has_suffix(filename, ".desktop"))
                {
                    dentry = xfce_desktop_entry_new (fullpath, dotDesktopKeys, 7);
                    if (XFCE_IS_DESKTOP_ENTRY(dentry))
                    {
                            gchar *name = NULL;
                            gchar *categories = NULL;
                            gchar *comment = NULL;
                            gchar *exec = NULL;
                            gchar *icon = NULL;
                        
                            xfce_desktop_entry_get_string (dentry, "Name", TRUE, &name);
                            xfce_desktop_entry_get_string (dentry, "Categories", TRUE, &categories);
                            xfce_desktop_entry_get_string (dentry, "Comment", TRUE, &comment);
                            xfce_desktop_entry_get_string (dentry, "Exec", TRUE, &exec);
                            xfce_desktop_entry_get_string (dentry, "Icon", TRUE, &icon);
                            
                            if (name && categories && exec)
                            {   
                            
                                cent = g_new(XfceAppfinderCacheEntry, 1);
                                cent->name = g_strdup(name);
                                cent->categories = g_strdup(categories);
                                cent->path = g_strdup(fullpath);
                                cent->exec = g_strdup(exec);

				if (comment)
				{
                                    cent->comment = g_strdup(comment);
				}
				else
				{
				    cent->comment = NULL;
				}
				
				if (icon)
			        {
                                    cent->icon = g_strdup(icon);
				}
				else
				{
				   cent->icon = NULL;
				}
				
                                g_hash_table_insert(hash, cent->name, cent);
                                
                                    g_free(name);
                                    g_free(categories);
                                    g_free(exec);
                                if (comment)
                                    g_free(comment);
                                if (icon)
                                    g_free(icon);
                            }
                            
                            g_object_unref (dentry);                        
                    }                  
                    g_free(fullpath);
                }
                else if (g_file_test(fullpath, G_FILE_TEST_IS_DIR))
                {
                    g_ptr_array_add(desktop_entries_paths, (gpointer) fullpath);
                }
            }
            g_dir_close(dir);
        }
        i++;
    }
    
    return hash;
}

