#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include "af-constants.h"
#include "inline-icon.h"

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

GtkListStore *
create_categories_liststore (void);

GtkListStore *
create_search_liststore(gchar *textSearch);

GtkListStore *
create_apps_liststore(void);

GtkWidget *
create_apps_treeview(gchar *textSearch);

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
	t_appfinder *af = userdata;
	gchar *text = g_utf8_strdown(gtk_entry_get_text(entry), -1);
	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(af->appstree))));
	gtk_tree_view_set_model (GTK_TREE_VIEW(af->appstree), GTK_TREE_MODEL(fetch_desktop_resources(showedcat, text)));
	gtk_tree_selection_unselect_all
			(gtk_tree_view_get_selection(GTK_TREE_VIEW(af->categoriestree)));
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
			filename = g_strdup_printf ("%s%s", entriespaths[i], filename);
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
    GtkTreeModel *model;
    GtkTreeIter   iter;

    model = gtk_tree_view_get_model(treeview);
    if (gtk_tree_model_get_iter(model, &iter, path))
    {
		gchar *name, *dname, *filename, *exec = NULL, **execp;
		gboolean found = FALSE;
		XfceDesktopEntry *dentry;
		GDir *dir;
		gint i = 0;

		/* we fetch the name of the application to run */
		gtk_tree_model_get(model, &iter, APP_TEXT, &name, -1);

		while (entriespaths[i]!=NULL)
		{
			if ((dir = g_dir_open (entriespaths[i], 0, NULL))==NULL)
			{
				i++;
				continue;
			}

			while (!found && ((filename = (gchar *)g_dir_read_name(dir))!=NULL))
			{
				filename = g_strdup_printf ("%s%s", entriespaths[i], filename);
				if (g_file_test(filename, G_FILE_TEST_IS_DIR) ||
						!XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (filename, keys, 7)))
				{
					g_free(filename);
					continue;
				}

				xfce_desktop_entry_get_string (dentry, "Name", FALSE, &dname);

				if (dname)
				{
					if (strcmp(dname, name)==0)
					{
						xfce_desktop_entry_get_string (dentry, "Exec", FALSE, &exec);
						saveHistory(filename);
						found = TRUE;
					}
					g_free(dname);
				}

				g_free(filename);
			}
			g_dir_close(dir);
			if (found)
				break;
			i++;
		}

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
		else {
			xfce_info("Cannot execute %s\n", name);
		}
		if (name)
			g_free(name);
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
	gtk_tree_view_set_model (GTK_TREE_VIEW(af->appstree), GTK_TREE_MODEL(fetch_desktop_resources(showedcat, NULL)));
    return TRUE; /* allow selection state to change */
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
	gtk_window_set_icon(GTK_WINDOW(af->mainwindow),inline_icon_at_size (default_icon_data_48_48, 48, 48));

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
	af->appstree = create_apps_treeview(NULL);
	g_signal_connect(af->appstree, "row-activated", (GCallback) cb_appstree, NULL);
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
	/* We have the icon loaded so we can scale it to 24x24 */
	else
		icon = gdk_pixbuf_scale_simple(icon, 24, 24, GDK_INTERP_BILINEAR);

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
	gchar *filename;
	gchar *comment;
	gchar *name;
	gchar *img;
	gchar *dcategories;
	gchar **cats = NULL;
	gint x = 0;
	gint i = 0;

	if (pattern != NULL)
	{
		gchar *tmp = g_strdelimit (pattern, " ", '*');
		tmp = g_strdup_printf ("*%s*", tmp);
		ptrn = g_pattern_spec_new (tmp);
		if (tmp)
			g_free(tmp);
	}

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
		while (entriespaths[i]!=NULL) {
			if ((dir = g_dir_open (entriespaths[i], 0, NULL))!=NULL)
			{
				while ((filename = (gchar *)g_dir_read_name(dir))!=NULL)
				{
					filename = g_strdup_printf ("%s%s", entriespaths[i], filename);
					if (g_file_test(filename, G_FILE_TEST_IS_DIR) ||
							!XFCE_IS_DESKTOP_ENTRY(dentry = xfce_desktop_entry_new (filename, keys, 7)))
					{
						g_free(filename);
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

									if (filename)
										g_free(filename);

									continue;
								}
							}
					}

					if (!xfce_desktop_entry_get_string (dentry, "Name", FALSE, &name))
					{
						if (dentry)
							g_object_unref (dentry);

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
							if (dentry)
								g_object_unref (dentry);
							continue;
						}
						if (comment)
							g_free(comment);
					}

					if (xfce_desktop_entry_get_string (dentry, "Icon", FALSE, &img) && img)
					{
						icon = load_icon_entry(img);
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

					if (filename)
						g_free(filename);
				}
				g_dir_close(dir);
			}
			i++;
		}
	}
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


/**********
 *   create_categories_treeview
 **********/
GtkWidget *
create_apps_treeview(gchar *textSearch)
{
	GtkTreeModel      *model;
	GtkTreeViewColumn *col;
	GtkCellRenderer   *renderer;
	GtkWidget         *view;

	if (textSearch)
		model = GTK_TREE_MODEL(create_search_liststore(textSearch));
	else
		model = GTK_TREE_MODEL(create_apps_liststore());

	view = gtk_tree_view_new_with_model(model);

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
	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(view),                                        											GDK_BUTTON1_MASK, gte, 5, GDK_ACTION_COPY);
	g_signal_connect(view, "drag-data-get", (GCallback) cb_dragappstree, NULL);
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
	configfile = g_strdup_printf ("%s/afhistory", xfce_get_userdir ());
	appfinder = create_interface();
	gtk_main();
	return 0;
}
