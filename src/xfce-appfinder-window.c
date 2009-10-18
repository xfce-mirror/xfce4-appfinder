/* vi:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2008 Jasper Huijsmans <jasper@xfce.org>.
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <garcon/garcon.h>
#include <gio/gio.h>
#include <xfconf/xfconf.h>

#include "xfce-appfinder-window.h"
#include "frap-icon-entry.h"



#define DEFAULT_WINDOW_WIDTH        640
#define DEFAULT_WINDOW_HEIGHT       480
#define DEFAULT_CLOSE_AFTER_EXECUTE FALSE
#define DEFAULT_CATEGORY            NULL

#define ICON_SIZE 32

#define ICON_COLUMN 0
#define TEXT_COLUMN 1
#define CATEGORY_COLUMN 2
#define ITEM_COLUMN 3
#define TOOLTIP_COLUMN 4



enum
{
  PROP_0,
  PROP_MENU_FILENAME,
  PROP_CATEGORY,
};



static void       xfce_appfinder_window_constructed        (GObject                  *object);
static void       xfce_appfinder_window_dispose            (GObject                  *object);
static void       xfce_appfinder_window_finalize           (GObject                  *object);
static void       xfce_appfinder_window_get_property       (GObject                  *object,
                                                            guint                     prop_id,
                                                            GValue                   *value,
                                                            GParamSpec               *pspec);
static void       xfce_appfinder_window_set_property       (GObject                  *object,
                                                            guint                     prop_id,
                                                            const GValue             *value,
                                                            GParamSpec               *pspec);
static void       _xfce_appfinder_window_closed            (XfceAppfinderWindow      *window);
static gpointer   _xfce_appfinder_window_reload_menu       (XfceAppfinderWindow      *window);
static void       _xfce_appfinder_window_entry_changed     (GtkEditable              *editable,
                                                            XfceAppfinderWindow      *window);
static void       _xfce_appfinder_window_entry_activated   (GtkEntry                 *entry,
                                                            XfceAppfinderWindow      *window);
static void       _xfce_appfinder_window_entry_focused     (GtkWidget                *entry,
                                                            GdkEventFocus            *event,
                                                            XfceAppfinderWindow      *window);
static gboolean   _xfce_appfinder_window_entry_key_pressed (GtkWidget                *widget,
                                                            GdkEventKey              *event,
                                                            XfceAppfinderWindow      *window);
static gboolean   _xfce_appfinder_window_radio_key_pressed (GtkWidget                *widget,
                                                            GdkEventKey              *event,
                                                            XfceAppfinderWindow      *window);
static gboolean   _xfce_appfinder_window_view_key_pressed  (GtkWidget                *widget,
                                                            GdkEventKey              *event,
                                                            XfceAppfinderWindow      *window);
static void       _xfce_appfinder_window_category_changed  (XfceAppfinderWindow      *window,
                                                            GtkToggleButton          *button);
static void       _xfce_appfinder_window_cursor_changed    (GtkTreeView              *tree_view,
                                                            XfceAppfinderWindow      *window);
static void       _xfce_appfinder_window_drag_data_get     (GtkWidget                *widget,
                                                            GdkDragContext           *drag_context,
                                                            GtkSelectionData         *data,
                                                            guint                     info,
                                                            guint                     drag_time,
                                                            XfceAppfinderWindow      *window);
static void       _xfce_appfinder_window_execute           (XfceAppfinderWindow      *window);
static void       _xfce_appfinder_window_load_menu_item    (XfceAppfinderWindow      *window,
                                                            GarconMenuItem             *item,
                                                            const gchar              *category,
                                                            gint                     *counter);
static GdkPixbuf *_xfce_appfinder_window_create_item_icon  (GarconMenuItem             *item);
static gboolean   _xfce_appfinder_window_visible_func      (GtkTreeModel             *filter,
                                                            GtkTreeIter              *iter,
                                                            gpointer                  user_data);
static void       _xfce_appfinder_window_set_category      (XfceAppfinderWindow      *window,
                                                            const gchar              *category);



struct _XfceAppfinderWindowClass
{
  XfceTitledDialogClass __parent__;
};

struct _XfceAppfinderWindow
{
  XfceTitledDialog __parent__;

  XfconfChannel *channel;

  GtkWidget     *search_entry;
  GtkWidget     *categories_alignment;
  GtkWidget     *categories_box;
  GtkWidget     *execute_button;

  GSList        *categories_group;
  gchar         *current_category;

  GtkListStore  *list_store;
  GtkTreeModel  *filter;
  GtkWidget     *tree_view;

  GarconMenu    *menu;
  gchar         *menu_filename;

  GThread       *reload_thread;
};



static const GtkTargetEntry dnd_target_list[] = {
  { "text/uri-list", 0, 0 }
};



G_DEFINE_TYPE (XfceAppfinderWindow, xfce_appfinder_window, XFCE_TYPE_TITLED_DIALOG)



static void
xfce_appfinder_window_class_init (XfceAppfinderWindowClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->constructed = xfce_appfinder_window_constructed;
  gobject_class->dispose = xfce_appfinder_window_dispose;
  gobject_class->finalize = xfce_appfinder_window_finalize;
  gobject_class->get_property = xfce_appfinder_window_get_property;
  gobject_class->set_property = xfce_appfinder_window_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_MENU_FILENAME,
                                   g_param_spec_string ("menu-filename",
                                                        "menu-filename",
                                                        "menu-filename",
                                                        NULL,
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_CATEGORY,
                                   g_param_spec_string ("category",
                                                        "category",
                                                        "category",
                                                        DEFAULT_CATEGORY,
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_READWRITE));
}



static void
xfce_appfinder_window_init (XfceAppfinderWindow *window)
{
  GtkTreeViewColumn *column;
  GtkCellRenderer   *renderer;
  GtkWidget         *vbox2;
  GtkWidget         *vbox3;
  GtkWidget         *hbox1;
  GtkWidget         *hbox2;
  GtkWidget         *check_button;
  GtkWidget         *execute_image;
  GtkWidget         *button;
  GtkWidget         *label;
  GtkWidget         *alignment;
  GtkWidget         *scrollwin;
  gchar             *text;
  gint               width;
  gint               height;

  window->menu = NULL;
  window->menu_filename = NULL;
  window->reload_thread = NULL;
  window->categories_group = NULL;
  window->current_category = NULL;

  window->channel = xfconf_channel_get ("xfce4-appfinder");

  g_signal_connect (window, "delete-event", G_CALLBACK (_xfce_appfinder_window_closed), NULL);

  gtk_window_set_title (GTK_WINDOW (window), _("Application Finder"));
  gtk_window_set_icon_name (GTK_WINDOW (window), "xfce4-appfinder");
  xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (window), _("Find and launch applications installed on your system"));
  gtk_dialog_set_has_separator (GTK_DIALOG (window), FALSE);

  width = xfconf_channel_get_int (window->channel, "/window-width", DEFAULT_WINDOW_WIDTH);
  height = xfconf_channel_get_int (window->channel, "/window-height", DEFAULT_WINDOW_HEIGHT);

  gtk_window_set_default_size (GTK_WINDOW (window), width, height);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

  vbox2 = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), vbox2, TRUE, TRUE, 0);
  gtk_widget_show (vbox2);

  hbox1 = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (vbox2), hbox1);
  gtk_widget_show (hbox1);

  vbox3 = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox3, FALSE, TRUE, 0);
  gtk_widget_show (vbox3);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<span weight=\"bold\" size=\"large\">%s</span>", _("Search"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &label->style->fg[GTK_STATE_INSENSITIVE]);
  gtk_box_pack_start (GTK_BOX (vbox3), label, FALSE, TRUE, 0);
  gtk_widget_show (label);
  g_free (text);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 1.0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 6, 12, 12);
  gtk_box_pack_start (GTK_BOX (vbox3), alignment, FALSE, TRUE, 0);
  gtk_widget_show (alignment);

#if GTK_CHECK_VERSION (2, 16, 0)
  window->search_entry = gtk_entry_new ();
  gtk_entry_set_icon_from_stock (GTK_ENTRY (window->search_entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
#else
  window->search_entry = frap_icon_entry_new ();
  frap_icon_entry_set_stock_id (FRAP_ICON_ENTRY (window->search_entry), GTK_STOCK_FIND);
#endif
  g_signal_connect (window->search_entry, "changed", G_CALLBACK (_xfce_appfinder_window_entry_changed), window);
  g_signal_connect (window->search_entry, "activate", G_CALLBACK (_xfce_appfinder_window_entry_activated), window);
  g_signal_connect (window->search_entry, "focus-in-event", G_CALLBACK (_xfce_appfinder_window_entry_focused), window);
  g_signal_connect (window->search_entry, "key-press-event", G_CALLBACK (_xfce_appfinder_window_entry_key_pressed), window);
  gtk_container_add (GTK_CONTAINER (alignment), window->search_entry);
  gtk_widget_show (window->search_entry);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<span weight=\"bold\" size=\"large\">%s</span>", _("Categories"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &label->style->fg[GTK_STATE_INSENSITIVE]);
  gtk_box_pack_start (GTK_BOX (vbox3), label, FALSE, TRUE, 0);
  gtk_widget_show (label);
  g_free (text);

  window->categories_alignment = gtk_alignment_new (0.0, 0.0, 1.0, 1.0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (window->categories_alignment), 0, 6, 12, 12);
  gtk_box_pack_start (GTK_BOX (vbox3), window->categories_alignment, FALSE, TRUE, 0);
  gtk_widget_show (window->categories_alignment);

  scrollwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (hbox1), scrollwin);
  gtk_widget_show (scrollwin);

  window->list_store =
    gtk_list_store_new (5, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, GARCON_TYPE_MENU_ITEM, G_TYPE_STRING);

  window->filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (window->list_store), NULL);
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (window->filter), _xfce_appfinder_window_visible_func, window, NULL);

  window->tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (window->filter));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (window->tree_view), FALSE);
  gtk_tree_view_set_tooltip_column (GTK_TREE_VIEW (window->tree_view), TOOLTIP_COLUMN);
  g_signal_connect (window->tree_view, "cursor-changed", G_CALLBACK (_xfce_appfinder_window_cursor_changed), window);
  g_signal_connect_swapped (window->tree_view, "row-activated", G_CALLBACK (_xfce_appfinder_window_execute), window);
  g_signal_connect (window->tree_view, "drag-data-get", G_CALLBACK (_xfce_appfinder_window_drag_data_get), window);
  g_signal_connect (window->tree_view, "key-press-event", G_CALLBACK (_xfce_appfinder_window_view_key_pressed), window);
  gtk_container_add (GTK_CONTAINER (scrollwin), window->tree_view);
  gtk_widget_show (window->tree_view);

  gtk_drag_source_set (window->tree_view, GDK_BUTTON1_MASK, dnd_target_list, G_N_ELEMENTS (dnd_target_list), GDK_ACTION_COPY);

  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "pixbuf", ICON_COLUMN, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (window->tree_view), GTK_TREE_VIEW_COLUMN (column));

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
  column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "markup", TEXT_COLUMN, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (window->tree_view), GTK_TREE_VIEW_COLUMN (column));

  hbox2 = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), hbox2, FALSE, TRUE, 0);
  gtk_widget_show (hbox2);

  check_button = gtk_check_button_new_with_mnemonic (_("C_lose after launch"));
  gtk_box_pack_start (GTK_BOX (hbox2), check_button, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (check_button), 6);
  gtk_widget_show (check_button);

  xfconf_g_property_bind (window->channel, "/close-after-execute", G_TYPE_BOOLEAN, G_OBJECT (check_button), "active");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), xfconf_channel_get_bool (window->channel, "/close-after-execute", DEFAULT_CLOSE_AFTER_EXECUTE));

  window->execute_button = gtk_button_new_with_mnemonic (_("Launch"));
  execute_image = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_BUTTON);
  gtk_button_set_image (GTK_BUTTON (window->execute_button), execute_image);
  gtk_dialog_add_action_widget (GTK_DIALOG (window), window->execute_button, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (window->execute_button, GTK_CAN_DEFAULT);
  gtk_button_set_focus_on_click (GTK_BUTTON (window->execute_button), FALSE);
  gtk_widget_set_sensitive (window->execute_button, FALSE);
  g_signal_connect_swapped (window->execute_button, "clicked", G_CALLBACK (_xfce_appfinder_window_execute), window);
  gtk_widget_show (window->execute_button);

  button = gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (_xfce_appfinder_window_closed), window);

  g_object_ref (G_OBJECT (GTK_DIALOG (window)->action_area));
  gtk_container_remove (GTK_CONTAINER (GTK_DIALOG (window)->vbox),
                        GTK_DIALOG (window)->action_area);
  gtk_box_pack_start (GTK_BOX (hbox2), GTK_DIALOG (window)->action_area, TRUE, TRUE, 0);
  g_object_unref (G_OBJECT (GTK_DIALOG (window)->action_area));
}



static void
xfce_appfinder_window_constructed (GObject *object)
{
  XfceAppfinderWindow *window = XFCE_APPFINDER_WINDOW (object);

  xfconf_g_property_bind (window->channel, "/category", G_TYPE_STRING, G_OBJECT (window), "category");
}



static void
xfce_appfinder_window_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_appfinder_window_parent_class)->dispose) (object);
}



static void
xfce_appfinder_window_finalize (GObject *object)
{
  XfceAppfinderWindow *window = XFCE_APPFINDER_WINDOW (object);

  g_free (window->current_category);
  g_free (window->menu_filename);

  if (G_LIKELY (GARCON_IS_MENU (window->menu)))
    g_object_unref (window->menu);

  g_slist_free (window->categories_group);

  (*G_OBJECT_CLASS (xfce_appfinder_window_parent_class)->finalize) (object);
}



static void
xfce_appfinder_window_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  XfceAppfinderWindow *window = XFCE_APPFINDER_WINDOW (object);

  switch (prop_id)
    {
    case PROP_CATEGORY:
      g_value_set_string (value, window->current_category);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_appfinder_window_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  XfceAppfinderWindow *window = XFCE_APPFINDER_WINDOW (object);

  switch (prop_id)
    {
    case PROP_MENU_FILENAME:
      g_free (window->menu_filename);
      window->menu_filename = g_strdup (g_value_get_string (value));
      break;

    case PROP_CATEGORY:
      _xfce_appfinder_window_set_category (window, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



GtkWidget *
xfce_appfinder_window_new (const gchar *filename)
{
  return g_object_new (XFCE_TYPE_APPFINDER_WINDOW, "menu-filename", filename, NULL);
}



void
xfce_appfinder_window_reload (XfceAppfinderWindow *window)
{
  _xfce_appfinder_window_reload_menu (window);
}



static void
_xfce_appfinder_window_entry_changed (GtkEditable         *editable,
                                      XfceAppfinderWindow *window)
{
  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));
  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (window->filter));
}



static void
_xfce_appfinder_window_entry_activated (GtkEntry            *entry,
                                        XfceAppfinderWindow *window)
{
  GtkTreePath *path;
  GtkTreePath *start;
  GtkTreePath *end;

  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));

  if (G_LIKELY (gtk_tree_view_get_visible_range (GTK_TREE_VIEW (window->tree_view), &start, &end)))
    {
      path = gtk_tree_path_new_first ();
      gtk_tree_view_set_cursor (GTK_TREE_VIEW (window->tree_view), path, NULL, FALSE);
      gtk_tree_path_free (path);

      gtk_tree_path_free (start);
      gtk_tree_path_free (end);

      gtk_widget_grab_focus (window->tree_view);
    }
}



static void
_xfce_appfinder_window_entry_focused (GtkWidget           *entry,
                                      GdkEventFocus       *event,
                                      XfceAppfinderWindow *window)
{
  GtkTreeSelection *selection;

  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (window->tree_view));

  if (G_LIKELY (selection != NULL))
    gtk_tree_selection_unselect_all (selection);

  gtk_widget_set_sensitive (window->execute_button, FALSE);
}



static gboolean
_xfce_appfinder_window_entry_key_pressed (GtkWidget           *widget,
                                          GdkEventKey         *event,
                                          XfceAppfinderWindow *window)
{
  GtkWidget *child;
  GList     *children;
  gboolean   handled = FALSE;

  g_return_val_if_fail (XFCE_IS_APPFINDER_WINDOW (window), FALSE);

  if (event->keyval == GDK_Up || event->keyval == GDK_Down)
    {
      children = gtk_container_get_children (GTK_CONTAINER (window->categories_box));

      if (G_LIKELY (children != NULL))
        {
          child = event->keyval == GDK_Down ? g_list_first (children)->data : g_list_last (children)->data;

          if (G_LIKELY (GTK_IS_TOGGLE_BUTTON (child)))
            {
              gtk_widget_grab_focus (child);
              gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (child), TRUE);

              handled = TRUE;
            }
        }
    }

  return handled;
}



static gboolean
_xfce_appfinder_window_radio_key_pressed (GtkWidget           *widget,
                                          GdkEventKey         *event,
                                          XfceAppfinderWindow *window)
{
  GList   *children;
  gboolean entry_grab = FALSE;
  gboolean handled = FALSE;

  g_return_val_if_fail (XFCE_IS_APPFINDER_WINDOW (window), FALSE);

  children = gtk_container_get_children (GTK_CONTAINER (window->categories_box));

  switch (event->keyval)
    {
    case GDK_Up:
    case GDK_Down:

      if (G_LIKELY (children != NULL))
        {
          if (event->keyval == GDK_Up)
            entry_grab = (widget == g_list_first (children)->data);
          else
            entry_grab = (widget == g_list_last (children)->data);

          if (G_UNLIKELY (entry_grab))
            {
              gtk_widget_grab_focus (window->search_entry);
              handled = TRUE;
            }
        }

      break;
    case GDK_Right:
      _xfce_appfinder_window_entry_activated (GTK_ENTRY (window->search_entry), window);
      handled = TRUE;
    default:
      break;
    }

  return handled;
}



static gboolean
_xfce_appfinder_window_view_key_pressed (GtkWidget           *widget,
                                         GdkEventKey         *event,
                                         XfceAppfinderWindow *window)
{
  g_return_val_if_fail (XFCE_IS_APPFINDER_WINDOW (window), FALSE);

  if (event->keyval == GDK_Left)
    {
      gtk_widget_grab_focus (window->search_entry);
      return TRUE;
    }
  else
    return FALSE;
}



static void
_xfce_appfinder_window_category_changed (XfceAppfinderWindow *window,
                                         GtkToggleButton     *button)
{
  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));

  if (gtk_toggle_button_get_active (button))
    _xfce_appfinder_window_set_category (window, gtk_button_get_label (GTK_BUTTON (button)));
}



static void
_xfce_appfinder_window_cursor_changed (GtkTreeView         *tree_view,
                                       XfceAppfinderWindow *window)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  GarconMenuItem     *item = NULL;

  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));

  selection = gtk_tree_view_get_selection (tree_view);

  if (G_LIKELY (selection != NULL && gtk_tree_selection_get_selected (selection, &model, &iter)))
    gtk_tree_model_get (model, &iter, ITEM_COLUMN, &item, -1);

  if (G_LIKELY (item != NULL))
    {
      gtk_widget_set_sensitive (window->execute_button, TRUE);
      gtk_widget_grab_default (window->execute_button);
    }
  else
    {
      gtk_widget_set_sensitive (window->execute_button, FALSE);
      gtk_widget_grab_focus (window->search_entry);
    }
}



static void
_xfce_appfinder_window_drag_data_get (GtkWidget           *widget,
                                      GdkDragContext      *drag_context,
                                      GtkSelectionData    *data,
                                      guint                info,
                                      guint                drag_time,
                                      XfceAppfinderWindow *window)
{
  GarconMenuItem   *item;
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  gchar            *uri;

  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (window->tree_view));

  if (G_UNLIKELY (selection == NULL || !gtk_tree_selection_get_selected (selection, &model, &iter)))
    return;

  gtk_tree_model_get (model, &iter, ITEM_COLUMN, &item, -1);

  if (G_UNLIKELY (item == NULL))
    return;

  uri = garcon_menu_item_get_uri (item);
  if (G_LIKELY (uri != NULL))
    {
      gtk_selection_data_set (data, data->target, 8, (guchar *) uri, strlen (uri));
      g_free (uri);
    }
}



static void
_xfce_appfinder_window_execute (XfceAppfinderWindow *window)
{
  GarconMenuItem   *item;
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  GdkScreen        *screen;
  GError           *error = NULL;
  gchar            *command, *uri;

  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (window->tree_view));

  if (G_UNLIKELY (selection == NULL || !gtk_tree_selection_get_selected (selection, &model, &iter)))
    return;

  gtk_tree_model_get (model, &iter, ITEM_COLUMN, &item, -1);

  if (G_UNLIKELY (item == NULL))
    return;

  uri = garcon_menu_item_get_uri (item);
  command = g_strconcat ("exo-open ", uri, NULL);
  g_free (uri);

  screen = xfce_gdk_screen_get_active (NULL);

  if (G_UNLIKELY (!xfce_spawn_command_line_on_screen (screen, command, FALSE, TRUE, &error)))
    {
      xfce_dialog_show_error (GTK_WINDOW (window), error,
           _("Could not execute application %s."), garcon_menu_element_get_name (GARCON_MENU_ELEMENT (item)));

      if (error != NULL)
        g_error_free (error);
    }

  g_free (command);

  if (G_LIKELY (xfconf_channel_get_bool (window->channel, "/close-after-execute", FALSE)))
    _xfce_appfinder_window_closed (window);
}



static void
_xfce_appfinder_window_closed (XfceAppfinderWindow *window)
{
  gint width;
  gint height;

  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));

  gtk_window_get_size (GTK_WINDOW (window), &width, &height);

  xfconf_channel_set_int (window->channel, "/window-width", width);
  xfconf_channel_set_int (window->channel, "/window-height", height);

  gtk_main_quit ();
}



static void
_xfce_appfinder_window_load_menu (XfceAppfinderWindow *window,
                                  GarconMenu          *menu,
                                  const gchar         *category,
                                  gint                *counter,
                                  gboolean             is_root,
                                  gboolean             is_category)
{
  GarconMenuDirectory *directory;
  GtkWidget           *button;
  GList               *items;
  GList               *menus;
  GList               *iter;
  const gchar         *name;
  gint                 current_counter = 0;

  g_return_if_fail (GARCON_IS_MENU (menu));
  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));

  directory = garcon_menu_get_directory (menu);

  if (G_LIKELY (directory != NULL))
    if (G_UNLIKELY (!garcon_menu_directory_get_show_in_environment (directory)
                    || garcon_menu_directory_get_hidden (directory)
                    || garcon_menu_directory_get_no_display (directory)))
      {
        return;
      }

  /* Determine menu name */
  name = garcon_menu_element_get_name (GARCON_MENU_ELEMENT (menu));

  /* Load menu items */
  items = garcon_menu_get_items (menu);
  for (iter = items; iter != NULL; iter = g_list_next (iter))
    _xfce_appfinder_window_load_menu_item (window, GARCON_MENU_ITEM (iter->data), is_category ? name : category, &current_counter);
  g_list_free (items);

  /* Load sub-menus */
  menus = garcon_menu_get_menus (menu);
  for (iter = menus; iter != NULL; iter = g_list_next (iter))
    _xfce_appfinder_window_load_menu (window, GARCON_MENU (iter->data), is_category ? name : category, &current_counter, FALSE, is_root ? TRUE : FALSE);
  g_list_free (menus);

  /* Create category widget */
  if (G_LIKELY (current_counter > 0 && is_category))
    {
      DBG ("name = %s", name);

      if (G_LIKELY (name != NULL))
        {
          button = gtk_radio_button_new_with_label (window->categories_group, name);
          gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
          g_signal_connect (button, "key-press-event", G_CALLBACK (_xfce_appfinder_window_radio_key_pressed), window);
          gtk_container_add (GTK_CONTAINER (window->categories_box), button);
          gtk_widget_show (button);

          if (G_UNLIKELY (window->current_category != NULL && g_utf8_collate (window->current_category, name) == 0))
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

          window->categories_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

          g_signal_connect_swapped (button, "toggled", G_CALLBACK (_xfce_appfinder_window_category_changed), window);
        }
    }

  *counter += 1;
}



static gpointer
_xfce_appfinder_window_reload_menu (XfceAppfinderWindow *window)
{
  GtkWidget *button;
  GError    *error = NULL;
  gint       counter = 0;

  g_return_val_if_fail (XFCE_IS_APPFINDER_WINDOW (window), NULL);

  DBG ("window->menu_filename = %s", window->menu_filename);

  if (G_UNLIKELY (window->menu_filename != NULL))
    window->menu = garcon_menu_new_for_path (window->menu_filename);
  else
    window->menu = garcon_menu_new_applications ();

  if (G_UNLIKELY (window->menu == NULL))
    {
      if (G_UNLIKELY (window->menu_filename != NULL))
        xfce_dialog_show_error (GTK_WINDOW (window), NULL,
          _("Could not load menu from %s"), window->menu_filename);
      else
        xfce_dialog_show_error (GTK_WINDOW (window), NULL,
          _("Could not load system menu"));

      if (error != NULL)
        g_error_free (error);

      return NULL;
    }

  if (!garcon_menu_load (window->menu, NULL, &error))
    {
      g_message ("failed to load the menu: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  if (GTK_IS_WIDGET (window->categories_box))
    gtk_widget_destroy (window->categories_box);

  window->categories_box = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (window->categories_alignment), window->categories_box);
  gtk_widget_show (window->categories_box);

  button = gtk_radio_button_new_with_label (NULL, _("All"));
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  g_signal_connect_swapped (button, "toggled", G_CALLBACK (_xfce_appfinder_window_category_changed), window);
  g_signal_connect (button, "key-press-event", G_CALLBACK (_xfce_appfinder_window_radio_key_pressed), window);
  gtk_container_add (GTK_CONTAINER (window->categories_box), button);
  gtk_widget_show (button);

  if (window->current_category == NULL || g_utf8_collate (window->current_category, _("All")) == 0)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

  window->categories_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  _xfce_appfinder_window_load_menu (window, window->menu, NULL, &counter, TRUE, FALSE);

  return NULL;
}



static void
_xfce_appfinder_window_load_menu_item (XfceAppfinderWindow *window,
                                       GarconMenuItem        *item,
                                       const gchar         *category,
                                       gint                *counter)
{
  GtkTreeIter   iter;
  GdkPixbuf    *pixbuf;
  GString      *tooltip_str;
  GList        *categories;
  GList        *lp;
  const gchar  *name = NULL;
  const gchar  *comment;
  const gchar  *command;
  gchar       **categories_array;
  gchar        *categories_string;
  gchar        *text;
  gchar        *tooltip = NULL;
  guint         n;

  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));

  if (G_UNLIKELY (!garcon_menu_item_get_show_in_environment (item) || garcon_menu_item_get_no_display (item)))
    return;

  if (G_UNLIKELY (garcon_menu_item_only_show_in_environment (item) || garcon_menu_item_has_category (item, "X-XFCE")))
    {
      name = garcon_menu_item_get_generic_name (item);

      if (G_UNLIKELY (name == NULL))
        name = garcon_menu_element_get_name (GARCON_MENU_ELEMENT (item));
    }
  else
    name = garcon_menu_element_get_name (GARCON_MENU_ELEMENT (item));

  comment = garcon_menu_item_get_comment (item);
  pixbuf = _xfce_appfinder_window_create_item_icon (item);
  categories = garcon_menu_item_get_categories (item);
  command = garcon_menu_item_get_command (item);

  if (G_LIKELY (comment != NULL))
    text = g_strdup_printf ("<b>%s</b>\n%s", name, comment);
  else
    text = g_strdup_printf ("<b>%s</b>", name);

  tooltip_str = g_string_new (NULL);

  if (G_LIKELY (categories != NULL))
    {
      categories_array = g_new0 (gchar *, g_list_length (categories) + 1);

      for (lp = categories, n = 0; lp != NULL; lp = lp->next, ++n)
        categories_array[n] = lp->data;

      categories_string = g_strjoinv (", ", categories_array);
      g_string_append_printf (tooltip_str, _("<b>Categories:</b> %s"), categories_string);
      g_free (categories_string);

      g_free (categories_array);
    }

  if (command != NULL && *command != '\0')
    {
      if (categories != NULL)
        g_string_append_c (tooltip_str, '\n');

      g_string_append_printf (tooltip_str, _("<b>Command:</b> %s"), command);
    }

  tooltip = g_string_free (tooltip_str, FALSE);

  gtk_list_store_append (window->list_store, &iter);
  gtk_list_store_set (window->list_store, &iter,
                      ICON_COLUMN, pixbuf,
                      TEXT_COLUMN, text,
                      CATEGORY_COLUMN, category,
                      ITEM_COLUMN, item,
                      TOOLTIP_COLUMN, tooltip,-1);

  g_free (text);
  g_free (tooltip);

  if (GDK_IS_PIXBUF (pixbuf))
    g_object_unref (pixbuf);

  *counter += 1;
}



static GdkPixbuf *
_xfce_appfinder_window_create_item_icon (GarconMenuItem *item)
{
  GdkPixbuf    *icon = NULL;
  GtkIconTheme *icon_theme;
  const gchar  *icon_name;
  const gchar  *item_name;
  gchar        *base_name;
  gchar        *extension;
  gchar        *new_item_name;
  gchar         new_icon_name[1024];

  /* Get current icon theme */
  icon_theme = gtk_icon_theme_get_default ();

  item_name = garcon_menu_element_get_name (GARCON_MENU_ELEMENT (item));
  icon_name = garcon_menu_element_get_icon_name (GARCON_MENU_ELEMENT (item));

  if (icon_name == NULL)
    return NULL;

  /* Check if we have an absolute filename */
  if (g_path_is_absolute (icon_name) && g_file_test (icon_name, G_FILE_TEST_EXISTS))
    icon = gdk_pixbuf_new_from_file_at_scale (icon_name, ICON_SIZE, ICON_SIZE, TRUE, NULL);
  else
    {
      /* Try to load the icon name directly using the icon theme */
      icon = gtk_icon_theme_load_icon (icon_theme, icon_name, ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);

      /* If that didn't work, try to remove the filename extension if there is one */
      if (icon == NULL)
        {
          /* Get basename (just to be sure) */
          base_name = g_path_get_basename (icon_name);

          /* Determine position of the extension */
          extension = g_utf8_strrchr (base_name, -1, '.');

          /* Make sure we found an extension */
          if (extension != NULL)
            {
              /* Remove extension */
              g_utf8_strncpy (new_icon_name, base_name, g_utf8_strlen (base_name, -1) - g_utf8_strlen (extension, -1));

              /* Try to load the pixbuf using the new icon name */
              icon = gtk_icon_theme_load_icon (icon_theme, new_icon_name, ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
            }

          /* Free basename */
          g_free (base_name);

          /* As a last fallback, we try to load the icon by lowercase item name */
          if (icon == NULL && item_name != NULL)
            {
              new_item_name = g_utf8_strdown (item_name, -1);
              icon = gtk_icon_theme_load_icon (icon_theme, new_item_name, ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
              g_free (new_item_name);
            }
        }
    }

  /* Fallback to default application icon */
  if (icon == NULL)
    icon = gtk_icon_theme_load_icon (icon_theme, "applications-other", ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);

  /* Scale icon (if needed) */
  if (icon != NULL)
    {
      GdkPixbuf *old_icon = icon;
      icon = gdk_pixbuf_scale_simple (old_icon, ICON_SIZE, ICON_SIZE, GDK_INTERP_BILINEAR);
      g_object_unref (old_icon);
    }

  return icon;
}



static gboolean
_xfce_appfinder_window_visible_func (GtkTreeModel *filter,
                                     GtkTreeIter  *iter,
                                     gpointer      user_data)
{
  XfceAppfinderWindow *window = user_data;
  GarconMenuItem      *item;
  gchar               *category;
  gchar               *text;
  gchar               *search_text;
  gchar               *normalized;
  gchar               *command;
  gboolean             visible = FALSE;

  g_return_val_if_fail (XFCE_IS_APPFINDER_WINDOW (window), FALSE);

  gtk_tree_model_get (filter, iter,
                      CATEGORY_COLUMN, &category,
                      TEXT_COLUMN, &text,
                      ITEM_COLUMN, &item, -1);

  if (G_UNLIKELY (text == NULL))
    return FALSE;

  normalized = g_utf8_normalize (gtk_entry_get_text (GTK_ENTRY (window->search_entry)), -1, G_NORMALIZE_ALL);
  search_text = g_utf8_casefold (normalized, -1);
  g_free (normalized);

  normalized = g_utf8_normalize (text, -1, G_NORMALIZE_ALL);
  g_free (text);
  text = g_utf8_casefold (normalized, -1);
  g_free (normalized);

  if (garcon_menu_item_get_command (item) != NULL)
    {
      normalized = g_utf8_normalize (garcon_menu_item_get_command (item), -1, G_NORMALIZE_ALL);
      command = g_utf8_casefold (normalized, -1);
      g_free (normalized);
    }
  else
    {
      command = g_strdup ("");
    }

  if (g_strstr_len (text, -1, search_text) != NULL ||
      g_strstr_len (command, -1, search_text) != NULL)
    {
      if (window->current_category == NULL ||
          g_utf8_strlen (window->current_category, -1) == 0 ||
          g_utf8_collate (window->current_category, _("All")) == 0)
        {
          visible = TRUE;
        }
      else
        {
          if (category != NULL && g_utf8_collate (category, window->current_category) == 0)
            visible = TRUE;
        }
    }

  g_free (search_text);
  g_free (command);
  g_free (text);
  g_free (category);

  return visible;
}



static void
_xfce_appfinder_window_set_category (XfceAppfinderWindow *window,
                                     const gchar         *category)
{
  g_return_if_fail (XFCE_IS_APPFINDER_WINDOW (window));

  if (G_LIKELY (window->current_category != NULL))
    if (G_UNLIKELY (g_utf8_collate (window->current_category, category) == 0))
      return;

  g_free (window->current_category);
  window->current_category = g_strdup (category);

  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (window->filter));
  gtk_widget_set_sensitive (window->execute_button, FALSE);

  g_object_notify (G_OBJECT (window), "category");
}
