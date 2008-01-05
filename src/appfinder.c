/* $Id$
 *
 * Copyright (c) 2008 Jasper Huijsmans <jasper@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <libxfce4menu/libxfce4menu.h>

#include "appfinder.h"

#ifndef _
#define _(x) x
#endif

#define SPACING         8
#define ICON_SIZE       32
#define VIEW_WIDTH      400
#define VIEW_HEIGHT     300

enum
{
  ITEM_COL,
  PIXBUF_COL,
  TEXT_COL,
  COMMENT_COL,
  NUM_COLUMS
};

typedef struct
{
  XfceMenuItem *item;
  GdkPixbuf *pixbuf;
  char *text;
  char *comment;
} AppData;



static void category_toggled (GtkToggleButton * tb, gpointer data);
static gboolean item_visible_func (GtkTreeModel * model, GtkTreeIter * iter, gpointer user_data);
static void view_data_get (GtkWidget * widget, GdkDragContext * drag_context,
			   GtkSelectionData * data, guint info, guint time, gpointer user_data);
static gboolean entry_focused (GtkWidget * entry, GdkEventFocus * ev, gpointer data);
static gboolean entry_key_pressed (GtkWidget * entry, GdkEventKey * ev, gpointer data);
static void execute_item (GtkWidget * button, gpointer data);
static void cursor_changed (GtkTreeView * view, gpointer data);
static gboolean key_pressed (GtkWidget * widget, GdkEventKey * ev, gpointer data);
static gboolean view_dblclick (GtkWidget * view, GdkEventButton * evt, gpointer data);
static void appfinder_add_items_for_category (gpointer key, gpointer value, gpointer data);
static void appfinder_add_category_widget (const char *category);
static void appfinder_add_application_data (AppData * data);



static GSList *menu_concat_items (XfceMenu * menu, GSList * list);
static void menu_add_applications (XfceMenu * menu, GSList * list);
static GdkPixbuf *menu_create_item_icon (XfceMenuItem * item);

static GtkWidget *application_window;
static GtkListStore *application_store;
static GtkWidget *application_view;
static GtkWidget *category_box;
static GtkWidget *radiobutton_all;
static GtkWidget *filter_entry;
static GtkWidget *exec_button;

static char *application_title = NULL;
static GHashTable *categories = NULL;
static GList *category_list = NULL;
static XfceMenuItem *current = NULL;

static const GtkTargetEntry dnd_target_list[] = {
  {"text/uri-list", 0, 0}
};



/*
 * public interface
 */

/**
 * appfinder_create_shell:
 *
 * Create appfinder window and all static widgets. The application view
 * and the category list will be filled in by appfinder_add_applications().
 **/
void
appfinder_create_shell (void)
{
  GtkIconTheme *theme;
  GdkPixbuf *icon;
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *buttonbox;
  GtkWidget *left;
  GtkWidget *right;
  GtkWidget *label;
  GtkWidget *align;
  GtkWidget *scroll;
  GtkWidget *button;
  char text[256];

  /* main window (don't show yet) */
  application_window = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), _("Application Finder"));
  gtk_container_set_border_width (GTK_CONTAINER (window), SPACING);

  g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (G_OBJECT (window), "key-press-event", G_CALLBACK (key_pressed), NULL);

  theme = gtk_icon_theme_get_default ();
  icon =
    gtk_icon_theme_load_icon (theme, "xfce4-appfinder", ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN,
			      NULL);
  if (!icon)
    {
      icon =
	gtk_icon_theme_load_icon (theme, "applications-other", ICON_SIZE,
				  GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
    }
  if (icon)
    {
      gtk_window_set_icon (GTK_WINDOW (window), icon);
      g_object_unref (icon);
    }

  /* layout */
  vbox = gtk_vbox_new (FALSE, SPACING);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  hbox = gtk_hbox_new (FALSE, SPACING);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  buttonbox = gtk_hbox_new (FALSE, SPACING);
  gtk_widget_show (buttonbox);
  gtk_box_pack_start (GTK_BOX (vbox), buttonbox, FALSE, FALSE, 0);

  /* left pane */
  left = gtk_vbox_new (FALSE, SPACING);
  gtk_widget_show (left);
  gtk_box_pack_start (GTK_BOX (hbox), left, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  g_snprintf (text, 256, "<span weight=\"bold\" size=\"large\">%s</span>", _("Filter"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &label->style->fg[GTK_STATE_INSENSITIVE]);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (left), label, FALSE, FALSE, 0);

  align = gtk_alignment_new (0, 0, 0, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (align), 0, 0, SPACING, 0);
  gtk_widget_show (align);
  gtk_box_pack_start (GTK_BOX (left), align, FALSE, FALSE, 0);

  filter_entry = gtk_entry_new ();
  gtk_widget_show (filter_entry);
  gtk_container_add (GTK_CONTAINER (align), filter_entry);

  g_signal_connect (G_OBJECT (filter_entry), "focus-in-event", G_CALLBACK (entry_focused), NULL);
  g_signal_connect (G_OBJECT (filter_entry), "key-press-event", G_CALLBACK (entry_key_pressed),
		    NULL);

  label = gtk_label_new (NULL);
  g_snprintf (text, 256, "<span weight=\"bold\" size=\"large\">%s</span>", _("Categories"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &label->style->fg[GTK_STATE_INSENSITIVE]);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (left), label, FALSE, FALSE, 0);

  align = gtk_alignment_new (0, 0, 1, 1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (align), 0, 0, SPACING, 0);
  gtk_widget_show (align);
  gtk_box_pack_start (GTK_BOX (left), align, FALSE, FALSE, 0);

  category_box = gtk_vbox_new (TRUE, SPACING);
  gtk_widget_show (category_box);
  gtk_container_add (GTK_CONTAINER (align), category_box);

  radiobutton_all = gtk_radio_button_new_with_label (NULL, _("All"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_all), TRUE);
  gtk_widget_show (radiobutton_all);
  gtk_box_pack_start (GTK_BOX (category_box), radiobutton_all, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (radiobutton_all), "toggled", G_CALLBACK (category_toggled), NULL);

  /* right pane */
  right = gtk_vbox_new (FALSE, SPACING);
  gtk_widget_show (right);
  gtk_box_pack_start (GTK_BOX (hbox), right, TRUE, TRUE, 0);

#if 0
  label = gtk_label_new (NULL);
  g_snprintf (text, 256, "<span weight=\"bold\" size=\"large\">%s</span>", _("Applications"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &label->style->fg[GTK_STATE_INSENSITIVE]);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (right), label, FALSE, FALSE, 0);
#endif

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll), GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_NEVER,
				  GTK_POLICY_ALWAYS);
  gtk_widget_show (scroll);
  gtk_box_pack_start (GTK_BOX (right), scroll, TRUE, TRUE, 0);

  application_view = gtk_tree_view_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (application_view), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (application_view), TRUE);
  gtk_widget_set_size_request (application_view, VIEW_WIDTH, VIEW_HEIGHT);
  gtk_widget_show (application_view);
  gtk_container_add (GTK_CONTAINER (scroll), application_view);

  g_signal_connect (G_OBJECT (application_view), "cursor-changed", G_CALLBACK (cursor_changed),
		    NULL);
  g_signal_connect (G_OBJECT (application_view), "key-press-event", G_CALLBACK (key_pressed), NULL);
  g_signal_connect (G_OBJECT (application_view), "button-press-event", G_CALLBACK (view_dblclick),
		    NULL);

  gtk_drag_source_set (application_view, GDK_BUTTON1_MASK, dnd_target_list,
		       G_N_ELEMENTS (dnd_target_list), GDK_ACTION_COPY);
  g_signal_connect (G_OBJECT (application_view), "drag-data-get", G_CALLBACK (view_data_get), NULL);

  /* buttons */
  button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (buttonbox), button, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gtk_main_quit), NULL);

  exec_button = button = gtk_button_new_from_stock (GTK_STOCK_EXECUTE);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  gtk_widget_set_sensitive (button, FALSE);
  gtk_widget_show (button);
  gtk_box_pack_end (GTK_BOX (buttonbox), button, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (execute_item), NULL);

  /* done */
  gtk_widget_show (window);
}



/**
 * appfinder_set_menu:
 *
 * Find categories and create cache of application data. This function
 * is intended to be called from a background thread, before calling
 * appfinder_add_applications() in the main thread. All GUI related 
 * actions are deferred to appfinder_add_applications().
 **/
void
appfinder_set_menu (XfceMenu * root_menu)
{
  XfceMenuDirectory *directory;
  GSList *items;
  GSList *menus;
  GSList *iter;

  directory = xfce_menu_get_directory (root_menu);
  if (directory != NULL)
    {
      application_title = g_strdup (xfce_menu_directory_get_name (directory));
    }
  else
    {
      application_title = g_strdup (xfce_menu_get_name (root_menu));
    }

  /* NOTE:
   * All toplevel menus are categories, the rest we ignore. The toplevel
   * items will get category "Applications".
   */
  categories = g_hash_table_new ((GHashFunc) g_str_hash, (GEqualFunc) g_str_equal);

  /* toplevel items */
  items = xfce_menu_get_items (root_menu);
  menu_add_applications (NULL, items);
  g_slist_free (items);

  /* category submenus */
  menus = xfce_menu_get_menus (root_menu);
  for (iter = menus; iter != NULL; iter = iter->next)
    {
      items = menu_concat_items ((XfceMenu *) iter->data, NULL);
      menu_add_applications ((XfceMenu *) iter->data, items);
      g_slist_free (items);
    }
  g_slist_free (menus);
}

/**
 * appfinder_add_applications:
 *
 * Create category widgets and fill the application view. This function
 * should be called from the main (GUI) thread only.
 **/
void
appfinder_add_applications (void)
{
  GtkTreeModel *filter;
  GtkCellRenderer *r;
  GtkTreeViewColumn *col;
  GList *l;

  if (application_title)
    {
      gtk_window_set_title (GTK_WINDOW (application_window), application_title);
    }

  application_store = gtk_list_store_new (NUM_COLUMS,	/* columns      */
					  G_TYPE_POINTER,	/* Item         */
					  GDK_TYPE_PIXBUF,	/* Pixbuf       */
					  G_TYPE_STRING,	/* Text         */
					  G_TYPE_STRING);	/* Comment      */

  r = gtk_cell_renderer_pixbuf_new ();
  col = gtk_tree_view_column_new_with_attributes ("Icon", r, "pixbuf", PIXBUF_COL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (application_view), col);

  r = gtk_cell_renderer_text_new ();
  col = gtk_tree_view_column_new_with_attributes ("Description", r, "markup", TEXT_COL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (application_view), col);

  filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (application_store), NULL);
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filter),
					  item_visible_func, filter_entry, NULL);
  g_signal_connect_swapped (G_OBJECT (filter_entry), "changed",
			    G_CALLBACK (gtk_tree_model_filter_refilter), filter);

  gtk_tree_view_set_model (GTK_TREE_VIEW (application_view), filter);
  g_object_unref (G_OBJECT (filter));

#if GTK_CHECK_VERSION (2, 12, 0)
  gtk_tree_view_set_tooltip_column (GTK_TREE_VIEW (application_view), COMMENT_COL);
#endif

  /* add all apllications and category widgets */
  if (G_LIKELY (categories != NULL))
    {
      g_hash_table_foreach (categories, (GHFunc) appfinder_add_items_for_category, NULL);
      category_list = g_list_sort (g_hash_table_get_keys (categories), (GCompareFunc) strcmp);
      for (l = category_list; l != NULL; l = l->next)
	{
	  if (G_UNLIKELY (strcmp ((const char *) l->data, _("Applications")) != 0))
	    appfinder_add_category_widget ((const char *) l->data);
	}
      gtk_widget_set_size_request (application_view, -1, VIEW_HEIGHT);
    }
}

/*
 * GUI functions
 */
static inline void
unselect_all (void)
{
  GtkTreeSelection *selection;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (application_view));
  if (selection)
    {
      gtk_tree_selection_unselect_all (selection);
    }
  current = NULL;
  gtk_widget_set_sensitive (exec_button, FALSE);
}



static void
category_toggled (GtkToggleButton * tb, gpointer data)
{
  gpointer appdata;
  GList *l;

  if (gtk_toggle_button_get_active (tb))
    {
      gtk_list_store_clear (application_store);
      if (GTK_WIDGET (tb) == radiobutton_all)
	{
	  /* show all */
	  for (l = category_list; l != NULL; l = l->next)
	    {
	      appdata = g_hash_table_lookup (categories, l->data);
	      if (appdata)
		{
		  appfinder_add_items_for_category (l->data, appdata, NULL);
		}
	    }
	}
      else
	{
	  /* 'data' is category */
	  appdata = g_hash_table_lookup (categories, data);
	  if (appdata)
	    {
	      appfinder_add_items_for_category (data, appdata, NULL);
	    }
	}
      gtk_widget_set_sensitive (exec_button, FALSE);
      gtk_widget_grab_focus (filter_entry);
    }
}



static gboolean
item_visible_func (GtkTreeModel * model, GtkTreeIter * iter, gpointer user_data)
{
  XfceMenuItem *item;
  const gchar *text;
  const gchar *item_text;
  GtkWidget *entry = GTK_WIDGET (user_data);
  gboolean visible;
  gchar *text_casefolded;
  gchar *info_casefolded;
  gchar *normalized;

  text = gtk_entry_get_text (GTK_ENTRY (entry));
  if (G_UNLIKELY (*text == '\0'))
    return TRUE;

  gtk_tree_model_get (model, iter, TEXT_COL, &item_text, ITEM_COL, &item, -1);
  if (G_UNLIKELY (item == NULL))
    return TRUE;

  normalized = g_utf8_normalize (text, -1, G_NORMALIZE_ALL);
  text_casefolded = g_utf8_casefold (normalized, -1);
  g_free (normalized);

  /* text (name & category) */
  normalized = g_utf8_normalize (item_text, -1, G_NORMALIZE_ALL);
  info_casefolded = g_utf8_casefold (normalized, -1);
  g_free (normalized);

  visible = (strstr (info_casefolded, text_casefolded) != NULL);

  g_free (info_casefolded);

  /* command */
  if (!visible)
    {
      normalized = g_utf8_normalize (xfce_menu_item_get_command (item), -1, G_NORMALIZE_ALL);
      info_casefolded = g_utf8_casefold (normalized, -1);
      g_free (normalized);

      visible = (strstr (info_casefolded, text_casefolded) != NULL);

      g_free (info_casefolded);
    }
  /* comment */
  if (!visible && xfce_menu_item_get_comment (item))
    {
      normalized = g_utf8_normalize (xfce_menu_item_get_comment (item), -1, G_NORMALIZE_ALL);
      info_casefolded = g_utf8_casefold (normalized, -1);
      g_free (normalized);

      visible = (strstr (info_casefolded, text_casefolded) != NULL);

      g_free (info_casefolded);
    }

  g_free (text_casefolded);

  return visible;
}



static void
view_data_get (GtkWidget * widget, GdkDragContext * drag_context, GtkSelectionData * data,
	       guint info, guint time, gpointer user_data)
{
  GtkTreeSelection *sel;
  GtkTreeModel *model;
  GtkTreeIter iter;
  XfceMenuItem *item;
  gchar *uri;

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));

  if (sel != NULL)
    {
      if (!gtk_tree_selection_get_selected (sel, &model, &iter))
	return;

      gtk_tree_model_get (model, &iter, ITEM_COL, &item, -1);

      uri = g_filename_to_uri (xfce_menu_item_get_filename (item), NULL, NULL);
      if (uri)
	{
	  gtk_selection_data_set (data, data->target, 8, (guchar *) uri, strlen (uri));
	  g_free (uri);
	}
    }
}



static gboolean
entry_focused (GtkWidget * entry, GdkEventFocus * ev, gpointer data)
{
  unselect_all ();
  return FALSE;
}



static gboolean
entry_key_pressed (GtkWidget * entry, GdkEventKey * ev, gpointer data)
{
  GtkTreePath *path;
  gboolean handled = FALSE;
  switch (ev->keyval)
    {
    case GDK_Return:
      gtk_widget_grab_focus (application_view);
      path = gtk_tree_path_new_from_string ("0");
      gtk_tree_view_set_cursor (GTK_TREE_VIEW (application_view), path, NULL, FALSE);
      gtk_tree_path_free (path);
      handled = TRUE;
      break;
    default:
      break;
    }
  return handled;
}



static void
execute_item (GtkWidget * button, gpointer data)
{
  char *exec;
  GError *error = NULL;

  if (current)
    {
      exec = g_strconcat ("exo-open ", xfce_menu_item_get_filename (current), NULL);
      if (!gdk_spawn_command_line_on_screen (gtk_widget_get_screen (button), exec, &error))
	{
	  if (error)
	    {
	      g_critical (error->message);
	      g_error_free (error);
	    }
	}
      g_free (exec);
    }
}



static void
cursor_changed (GtkTreeView * view, gpointer data)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;

  selection = gtk_tree_view_get_selection (view);
  if (selection && gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      /* get selected item */
      gtk_tree_model_get (model, &iter, ITEM_COL, &current, -1);
    }
  else
    {
      current = NULL;
    }
  if (current != NULL)
    {
      gtk_widget_set_sensitive (exec_button, TRUE);
      gtk_widget_grab_default (exec_button);
    }
  else
    {
      gtk_widget_set_sensitive (exec_button, FALSE);
      gtk_widget_grab_focus (filter_entry);
    }
}



static gboolean
key_pressed (GtkWidget * widget, GdkEventKey * ev, gpointer data)
{
  gboolean handled = FALSE;
  switch (ev->keyval)
    {
    case GDK_Return:
      if (current)
	{
	  execute_item (exec_button, NULL);
	  handled = TRUE;
	}
      break;
    case GDK_Escape:
      gtk_main_quit ();
      handled = TRUE;
      break;
    default:
      break;
    }
  return handled;
}



static gboolean
view_dblclick (GtkWidget * view, GdkEventButton * evt, gpointer data)
{
  if (current && evt->button == 1 && evt->type == GDK_2BUTTON_PRESS)
    {
      execute_item (exec_button, NULL);
    }

  return FALSE;
}



static void
appfinder_add_application_data (AppData * data)
{
  GtkTreeIter iter;

  if (G_UNLIKELY (data->pixbuf == NULL))
    {
      data->pixbuf = menu_create_item_icon (data->item);
    }

  gtk_list_store_append (application_store, &iter);
  gtk_list_store_set (application_store, &iter, ITEM_COL, data->item, PIXBUF_COL, data->pixbuf,
		      TEXT_COL, data->text, COMMENT_COL, data->comment, -1);
}



static void
appfinder_add_items_for_category (gpointer key, gpointer value, gpointer data)
{
  AppData *appdata;
  GArray *item_array;
  guint i;

  item_array = (GArray *) value;

  for (i = 0; i < item_array->len; i++)
    {
      appdata = &g_array_index (item_array, AppData, i);
      appfinder_add_application_data (appdata);
    }
}



static void
appfinder_add_category_widget (const char *category)
{
  GtkWidget *rb;
  GtkWidget *label;

  rb = gtk_radio_button_new_from_widget (GTK_RADIO_BUTTON (radiobutton_all));
  gtk_widget_show (rb);
  gtk_box_pack_start (GTK_BOX (category_box), rb, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (rb), "toggled", G_CALLBACK (category_toggled), g_strdup (category));

  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), category);
  gtk_widget_show (label);
  gtk_container_add (GTK_CONTAINER (rb), label);
}



/* 
 * menu handling
 */

static GSList *
menu_concat_items (XfceMenu * menu, GSList * list)
{
  GSList *items;
  GSList *menus;
  GSList *iter;

  items = xfce_menu_get_items (menu);
  menus = xfce_menu_get_menus (menu);

  for (iter = menus; iter != NULL; iter = iter->next)
    {
      items = menu_concat_items ((XfceMenu *) iter->data, items);
    }

  g_slist_free (menus);

  return g_slist_concat (list, items);
}

static void
menu_add_applications (XfceMenu * menu, GSList * list)
{
  XfceMenuDirectory *directory;
  XfceMenuItem *item;
  AppData data;
  GArray *item_array;
  GSList *iter;
  gint size;
  guint i;
  const char *value = NULL;
  char *name;
  char *category;

  size = g_slist_length (list);

  if (size)
    {
      if (menu != NULL)
	{
	  directory = xfce_menu_get_directory (menu);
	  if (directory != NULL)
	    {
	      value = xfce_menu_directory_get_name (directory);
	    }
	  else
	    {
	      value = xfce_menu_get_name (menu);
	    }
	}
      category = value ? g_markup_escape_text (value, -1) : g_strdup (_("Applications"));
      item_array = g_array_sized_new (FALSE, FALSE, sizeof (AppData), size);
      for (i = 0, iter = list; i < size; i++, iter = iter->next)
	{
	  if (G_UNLIKELY (!XFCE_IS_MENU_ITEM (iter->data)))	/* separator */
	    {
	      continue;
	    }

	  item = XFCE_MENU_ITEM (iter->data);
	  data.item = item;

	  /* IMPORTANT: create pixbuf from the GUI thread later, not here. */
	  data.pixbuf = NULL;

	  value = xfce_menu_item_get_name (item);
	  name = value ? g_markup_escape_text (value, -1) : g_strdup ("?");
	  data.text = g_strdup_printf ("<b>%s</b>\n<i><small>%s</small></i>", name, category);
	  g_free (name);

	  value = xfce_menu_item_get_comment (item);
	  /* data.comment = value ? g_markup_escape_text (value, -1) : NULL; */
	  data.comment = value ? g_strdup (value) : NULL;

	  g_array_append_val (item_array, data);
	}
      if (item_array->len)
	{
	  g_hash_table_insert (categories, category, item_array);
	}
      else
	{
	  g_array_free (item_array, TRUE);
	  g_free (category);
	}
    }
}



static GdkPixbuf *
menu_create_item_icon (XfceMenuItem * item)
{
  GdkPixbuf *icon = NULL;
  GtkIconTheme *icon_theme;
  const gchar *icon_name;
  const gchar *item_name;
  gchar *basename;
  gchar *extension;
  gchar *new_item_name;
  gchar new_icon_name[1024];

  /* Get current icon theme */
  icon_theme = gtk_icon_theme_get_default ();

  item_name = xfce_menu_element_get_name (XFCE_MENU_ELEMENT (item));
  icon_name = xfce_menu_element_get_icon_name (XFCE_MENU_ELEMENT (item));

  if (icon_name == NULL)
    return NULL;

  /* Check if we have an absolute filename */
  if (g_path_is_absolute (icon_name) && g_file_test (icon_name, G_FILE_TEST_EXISTS))
    icon = gdk_pixbuf_new_from_file_at_scale (icon_name, ICON_SIZE, ICON_SIZE, TRUE, NULL);
  else
    {
      /* Try to load the icon name directly using the icon theme */
      icon =
	gtk_icon_theme_load_icon (icon_theme, icon_name, ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN,
				  NULL);

      /* If that didn't work, try to remove the filename extension if there is one */
      if (icon == NULL)
	{
	  /* Get basename (just to be sure) */
	  basename = g_path_get_basename (icon_name);

	  /* Determine position of the extension */
	  extension = g_utf8_strrchr (basename, -1, '.');

	  /* Make sure we found an extension */
	  if (extension != NULL)
	    {
	      /* Remove extension */
	      g_utf8_strncpy (new_icon_name, basename,
			      g_utf8_strlen (basename, -1) - g_utf8_strlen (extension, -1));

	      /* Try to load the pixbuf using the new icon name */
	      icon =
		gtk_icon_theme_load_icon (icon_theme, new_icon_name, ICON_SIZE,
					  GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	    }

	  /* Free basename */
	  g_free (basename);

	  /* As a last fallback, we try to load the icon by lowercase item name */
	  if (icon == NULL && item_name != NULL)
	    {
	      new_item_name = g_utf8_strdown (item_name, -1);
	      icon =
		gtk_icon_theme_load_icon (icon_theme, new_item_name, ICON_SIZE,
					  GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	      g_free (new_item_name);
	    }
	}
    }

  /* fallback */
  if (icon == NULL)
    {
      icon =
	gtk_icon_theme_load_icon (icon_theme, "applications-other", ICON_SIZE,
				  GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
    }

  /* Scale icon (if needed) */
  if (icon != NULL)
    {
      GdkPixbuf *old_icon = icon;
      icon = gdk_pixbuf_scale_simple (old_icon, ICON_SIZE, ICON_SIZE, GDK_INTERP_BILINEAR);
      g_object_unref (old_icon);
    }

  return icon;
}
