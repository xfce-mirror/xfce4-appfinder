/*
 * Copyright (C) 2011 Nick Schermer <nick@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <gdk/gdkkeysyms.h>
#include <xfconf/xfconf.h>

#include <src/appfinder-window.h>
#include <src/appfinder-model.h>
#include <src/appfinder-category-model.h>
#include <src/appfinder-private.h>



#define DEFAULT_WINDOW_WIDTH   400
#define DEFAULT_WINDOW_HEIGHT  400
#define DEFAULT_PANED_POSITION 180



static void       xfce_appfinder_window_finalize                 (GObject                     *object);
static void       xfce_appfinder_window_unmap                    (GtkWidget                   *widget);
static gboolean   xfce_appfinder_window_key_press_event          (GtkWidget                   *widget,
                                                                  GdkEventKey                 *event);
static void       xfce_appfinder_window_set_padding              (GtkWidget                   *entry,
                                                                  GtkWidget                   *align);
static void       xfce_appfinder_window_entry_changed            (XfceAppfinderWindow         *window);
static void       xfce_appfinder_window_entry_activate           (GtkEditable                 *entry,
                                                                  XfceAppfinderWindow         *window);
static gboolean   xfce_appfinder_window_entry_key_press_event    (GtkWidget                   *entry,
                                                                  GdkEventKey                 *event,
                                                                  XfceAppfinderWindow         *window);
static void       xfce_appfinder_window_entry_icon_released      (GtkEntry                    *entry,
                                                                  GtkEntryIconPosition         icon_pos,
                                                                  GdkEvent                    *event,
                                                                  XfceAppfinderWindow         *window);
static void       xfce_appfinder_window_drag_begin               (GtkWidget                   *widget,
                                                                  GdkDragContext              *drag_context,
                                                                  XfceAppfinderWindow         *window);
static void       xfce_appfinder_window_drag_data_get            (GtkWidget                   *widget,
                                                                  GdkDragContext              *drag_context,
                                                                  GtkSelectionData            *data,
                                                                  guint                        info,
                                                                  guint                        drag_time,
                                                                  XfceAppfinderWindow         *window);
static void       xfce_appfinder_window_category_changed         (GtkTreeSelection            *selection,
                                                                  XfceAppfinderWindow         *window);
static gboolean   xfce_appfinder_window_item_visible             (GtkTreeModel                *model,
                                                                  GtkTreeIter                 *iter,
                                                                  gpointer                     data);
static void       xfce_appfinder_window_item_changed             (XfceAppfinderWindow         *window);
static void       xfce_appfinder_window_row_activated            (XfceAppfinderWindow         *window);
static void       xfce_appfinder_window_icon_theme_changed       (XfceAppfinderWindow         *window);
static void       xfce_appfinder_window_execute                  (XfceAppfinderWindow         *window);



struct _XfceAppfinderWindowClass
{
  GtkWindowClass __parent__;
};

struct _XfceAppfinderWindow
{
  GtkWindow __parent__;

  XfceAppfinderModel         *model;

  XfceAppfinderCategoryModel *category_model;

  GtkEntryCompletion         *completion;

  GtkWidget                  *paned;
  GtkWidget                  *entry;
  GtkWidget                  *image;
  GtkWidget                  *treeview;

  GdkPixbuf                  *icon_find;

  GtkWidget                  *bbox;
  GtkWidget                  *button_launch;
  GtkWidget                  *bin_collapsed;
  GtkWidget                  *bin_expanded;

  gchar                      *filter_category;
  gchar                      *filter_text;

  gint                        last_window_height;
};

static const GtkTargetEntry target_list[] =
{
  { "text/uri-list", 0, 0 }
};



G_DEFINE_TYPE (XfceAppfinderWindow, xfce_appfinder_window, GTK_TYPE_WINDOW)



static void
xfce_appfinder_window_class_init (XfceAppfinderWindowClass *klass)
{
  GObjectClass   *gobject_class;
  GtkWidgetClass *gtkwidget_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_appfinder_window_finalize;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->unmap = xfce_appfinder_window_unmap;
  gtkwidget_class->key_press_event = xfce_appfinder_window_key_press_event;
}



static void
xfce_appfinder_window_init (XfceAppfinderWindow *window)
{
  GtkWidget          *vbox, *vbox2;
  GtkWidget          *entry;
  GtkWidget          *pane;
  GtkWidget          *scroll;
  GtkWidget          *sidepane;
  GtkWidget          *treeview;
  GtkWidget          *image;
  GtkWidget          *hbox;
  GtkWidget          *align;
  GtkTreeViewColumn  *column;
  GtkCellRenderer    *renderer;
  GtkTreeModel       *filter_model;
  GtkTreeSelection   *selection;
  GtkWidget          *bbox;
  GtkWidget          *button;
  GtkTreePath        *path;
  GtkEntryCompletion *completion;
  GtkIconTheme       *icon_theme;
  XfconfChannel      *channel;
  gint                integer;

  channel = xfconf_channel_get ("xfce4-appfinder");
  window->last_window_height = xfconf_channel_get_int (channel, "/LastWindowHeight", DEFAULT_WINDOW_HEIGHT);

  window->model = xfce_appfinder_model_get ();
  window->category_model = xfce_appfinder_category_model_new ();
  g_signal_connect_swapped (G_OBJECT (window->model), "categories-changed",
                            G_CALLBACK (xfce_appfinder_category_model_set_categories),
                            window->category_model);

  gtk_window_set_title (GTK_WINDOW (window), _("Application Finder"));
  integer = xfconf_channel_get_int (channel, "/LastWindowWidth", DEFAULT_WINDOW_WIDTH);
  gtk_window_set_default_size (GTK_WINDOW (window), integer, -1);
  gtk_window_set_icon_name (GTK_WINDOW (window), GTK_STOCK_EXECUTE);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  gtk_widget_show (vbox);

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
  gtk_widget_show (hbox);

  align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (hbox), align, FALSE, FALSE, 0);
  gtk_widget_show (align);

  window->icon_find = xfce_appfinder_model_load_pixbuf (GTK_STOCK_FIND, ICON_LARGE);
  window->image = image = gtk_image_new_from_pixbuf (window->icon_find);
  gtk_container_add (GTK_CONTAINER (align), image);
  gtk_widget_show (image);

  vbox2 = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);
  gtk_widget_show (vbox2);

  align = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_box_pack_start (GTK_BOX (vbox2), align, TRUE, TRUE, 0);
  gtk_widget_show (align);

  window->entry = entry = gtk_entry_new ();
  gtk_container_add (GTK_CONTAINER (align), entry);
  g_signal_connect (G_OBJECT (entry), "icon-release", G_CALLBACK (xfce_appfinder_window_entry_icon_released), window);
  g_signal_connect (G_OBJECT (entry), "realize", G_CALLBACK (xfce_appfinder_window_set_padding), align);
  g_signal_connect_swapped (G_OBJECT (entry), "changed", G_CALLBACK (xfce_appfinder_window_entry_changed), window);
  g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (xfce_appfinder_window_entry_activate), window);
  g_signal_connect (G_OBJECT (entry), "key-press-event", G_CALLBACK (xfce_appfinder_window_entry_key_press_event), window);
  gtk_entry_set_icon_tooltip_text (GTK_ENTRY (window->entry), GTK_ENTRY_ICON_SECONDARY, _("Toggle view mode"));
  gtk_widget_show (entry);

  window->completion = completion = gtk_entry_completion_new ();
  gtk_entry_completion_set_model (completion, GTK_TREE_MODEL (window->model));
  gtk_entry_completion_set_text_column (completion, XFCE_APPFINDER_MODEL_COLUMN_COMMAND);
  gtk_entry_completion_set_popup_completion (completion, TRUE);
  gtk_entry_completion_set_popup_single_match (completion, FALSE);
  gtk_entry_completion_set_inline_completion (completion, TRUE);

  window->bin_collapsed = gtk_alignment_new (0.0, 0.0, 1.0, 1.0);
  gtk_box_pack_start (GTK_BOX (vbox2), window->bin_collapsed, FALSE, TRUE, 0);
  gtk_widget_show (window->bin_collapsed);

  window->paned = pane = gtk_hpaned_new ();
  gtk_box_pack_start (GTK_BOX (vbox), pane, TRUE, TRUE, 0);
  integer = xfconf_channel_get_int (channel, "/LastPanedPosition", DEFAULT_PANED_POSITION);
  gtk_paned_set_position (GTK_PANED (pane), integer);
  g_object_set (G_OBJECT (pane), "position-set", TRUE, NULL);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_paned_add1 (GTK_PANED (pane), scroll);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll), GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_show (scroll);

  sidepane = gtk_tree_view_new_with_model (GTK_TREE_MODEL (window->category_model));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (sidepane), FALSE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (sidepane), FALSE);
  g_signal_connect_swapped (GTK_TREE_VIEW (sidepane), "start-interactive-search", G_CALLBACK (gtk_widget_grab_focus), entry);
  gtk_tree_view_set_row_separator_func (GTK_TREE_VIEW (sidepane),
      xfce_appfinder_category_model_row_separator_func, NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scroll), sidepane);
  gtk_widget_show (sidepane);

  path = gtk_tree_path_new_first ();
  gtk_tree_view_set_cursor (GTK_TREE_VIEW (sidepane), path, NULL, FALSE);
  gtk_tree_path_free (path);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (sidepane));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
  g_signal_connect (G_OBJECT (selection), "changed",
      G_CALLBACK (xfce_appfinder_window_category_changed), window);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_append_column (GTK_TREE_VIEW (sidepane), GTK_TREE_VIEW_COLUMN (column));

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (GTK_TREE_VIEW_COLUMN (column), renderer, FALSE);
  gtk_tree_view_column_set_attributes (GTK_TREE_VIEW_COLUMN (column), renderer,
                                       "pixbuf", XFCE_APPFINDER_CATEGORY_MODEL_COLUMN_ICON, NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
  gtk_tree_view_column_pack_start (GTK_TREE_VIEW_COLUMN (column), renderer, TRUE);
  gtk_tree_view_column_set_attributes (GTK_TREE_VIEW_COLUMN (column), renderer,
                                       "text", XFCE_APPFINDER_CATEGORY_MODEL_COLUMN_NAME, NULL);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_paned_add2 (GTK_PANED (pane), scroll);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll), GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_show (scroll);

  filter_model = gtk_tree_model_filter_new (GTK_TREE_MODEL (window->model), NULL);
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filter_model), xfce_appfinder_window_item_visible, window, NULL);

  window->treeview = treeview = gtk_tree_view_new_with_model (filter_model);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);
  gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (treeview), TRUE);
  gtk_tree_view_set_tooltip_column (GTK_TREE_VIEW (treeview), XFCE_APPFINDER_MODEL_COLUMN_TOOLTIP);
  g_signal_connect_swapped (GTK_TREE_VIEW (treeview), "row-activated", G_CALLBACK (xfce_appfinder_window_row_activated), window);
  g_signal_connect_swapped (GTK_TREE_VIEW (treeview), "start-interactive-search", G_CALLBACK (gtk_widget_grab_focus), entry);
  gtk_drag_source_set (treeview, GDK_BUTTON1_MASK, target_list, G_N_ELEMENTS (target_list), GDK_ACTION_COPY);
  g_signal_connect (G_OBJECT (treeview), "drag-begin", G_CALLBACK (xfce_appfinder_window_drag_begin), window);
  g_signal_connect (G_OBJECT (treeview), "drag-data-get", G_CALLBACK (xfce_appfinder_window_drag_data_get), window);
  gtk_container_add (GTK_CONTAINER (scroll), treeview);
  gtk_widget_show (treeview);
  g_object_unref (G_OBJECT (filter_model));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
  g_signal_connect_swapped (G_OBJECT (selection), "changed",
      G_CALLBACK (xfce_appfinder_window_item_changed), window);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_spacing (column, 2);
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (column));

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (GTK_TREE_VIEW_COLUMN (column), renderer, FALSE);
  gtk_tree_view_column_set_attributes (GTK_TREE_VIEW_COLUMN (column), renderer,
                                       "pixbuf", XFCE_APPFINDER_MODEL_COLUMN_ICON_SMALL, NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
  gtk_tree_view_column_pack_start (GTK_TREE_VIEW_COLUMN (column), renderer, TRUE);
  gtk_tree_view_column_set_attributes (GTK_TREE_VIEW_COLUMN (column), renderer,
                                       "markup", XFCE_APPFINDER_MODEL_COLUMN_ABSTRACT, NULL);

  window->bin_expanded = gtk_alignment_new (0.0, 0.0, 1.0, 1.0);
  gtk_box_pack_start (GTK_BOX (vbox), window->bin_expanded, FALSE, TRUE, 0);
  gtk_widget_show (window->bin_expanded);

  window->bbox = bbox = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (bbox), 6);
  gtk_widget_show (bbox);

  button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_container_add (GTK_CONTAINER (bbox), button);
  g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_widget_show (button);

  window->button_launch = button = gtk_button_new_with_mnemonic ("La_unch");
  gtk_container_add (GTK_CONTAINER (bbox), button);
  g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (xfce_appfinder_window_execute), window);
  gtk_widget_set_sensitive (button, FALSE);
  gtk_widget_show (button);

  image = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_BUTTON);
  gtk_button_set_image (GTK_BUTTON (button), image);

  icon_theme = gtk_icon_theme_get_for_screen (gtk_window_get_screen (GTK_WINDOW (window)));
  g_signal_connect_swapped (G_OBJECT (icon_theme), "changed", G_CALLBACK (xfce_appfinder_window_icon_theme_changed), window);

  APPFINDER_DEBUG ("constructed window");
}



static void
xfce_appfinder_window_finalize (GObject *object)
{
  XfceAppfinderWindow *window = XFCE_APPFINDER_WINDOW (object);

  g_object_unref (G_OBJECT (window->model));
  g_object_unref (G_OBJECT (window->category_model));
  g_object_unref (G_OBJECT (window->completion));
  g_object_unref (G_OBJECT (window->icon_find));

  g_free (window->filter_category);
  g_free (window->filter_text);

  (*G_OBJECT_CLASS (xfce_appfinder_window_parent_class)->finalize) (object);
}



static void
xfce_appfinder_window_unmap (GtkWidget *widget)
{
  XfceAppfinderWindow *window = XFCE_APPFINDER_WINDOW (widget);
  gint                 width, height;
  gint                 position;
  XfconfChannel       *channel;

  position = gtk_paned_get_position (GTK_PANED (window->paned));
  gtk_window_get_size (GTK_WINDOW (window), &width, &height);
  if (!gtk_widget_get_visible (window->paned))
    height = window->last_window_height;

  (*GTK_WIDGET_CLASS (xfce_appfinder_window_parent_class)->unmap) (widget);

  channel = xfconf_channel_get ("xfce4-appfinder");
  xfconf_channel_set_int (channel, "/LastWindowHeight", height);
  xfconf_channel_set_int (channel, "/LastWindowWidth", width);
  xfconf_channel_set_int (channel, "/LastPanedPosition", position);
}



static gboolean
xfce_appfinder_window_key_press_event (GtkWidget   *widget,
                                       GdkEventKey *event)
{
  if (event->keyval == GDK_Escape)
    {
      gtk_widget_destroy (widget);
      return TRUE;
    }

  return  (*GTK_WIDGET_CLASS (xfce_appfinder_window_parent_class)->key_press_event) (widget, event);
}



static void
xfce_appfinder_window_update_image (XfceAppfinderWindow *window,
                                    GdkPixbuf           *pixbuf)
{
  if (pixbuf == NULL)
    pixbuf = window->icon_find;

  /* gtk doesn't check this */
  if (gtk_image_get_pixbuf (GTK_IMAGE (window->image)) != pixbuf)
    gtk_image_set_from_pixbuf (GTK_IMAGE (window->image), pixbuf);
}



static void
xfce_appfinder_window_set_padding (GtkWidget *entry,
                                   GtkWidget *align)
{
  gint padding;

  padding = (ICON_LARGE - entry->allocation.height) / 2;
  gtk_alignment_set_padding (GTK_ALIGNMENT (align), MAX (0, padding), 0, 0, 0);
}



/* TODO idle */
static void
xfce_appfinder_window_entry_changed (XfceAppfinderWindow *window)
{
  const gchar  *text;
  GdkPixbuf    *pixbuf;
  gchar        *normalized;
  GtkTreeModel *model;

  text = gtk_entry_get_text (GTK_ENTRY (window->entry));

  if (gtk_widget_get_visible (window->paned))
    {
      g_free (window->filter_text);

      if (IS_STRING (text))
        {
          normalized = g_utf8_normalize (text, -1, G_NORMALIZE_ALL);
          window->filter_text = g_utf8_casefold (normalized, -1);
          g_free (normalized);
        }
      else
        {
          window->filter_text = NULL;
        }

      model = gtk_tree_view_get_model (GTK_TREE_VIEW (window->treeview));
      gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (model));
    }
  else
    {
      gtk_widget_set_sensitive (window->button_launch, IS_STRING (text));

      pixbuf = xfce_appfinder_model_get_icon_for_command (window->model, text);
      xfce_appfinder_window_update_image (window, pixbuf);
      if (pixbuf != NULL)
        g_object_unref (G_OBJECT (pixbuf));
    }
}



static void
xfce_appfinder_window_entry_activate (GtkEditable         *entry,
                                      XfceAppfinderWindow *window)
{
  GtkTreePath *path;

  if (gtk_widget_get_visible (window->paned))
    {
      if (gtk_tree_view_get_visible_range (GTK_TREE_VIEW (window->treeview), &path, NULL))
        {
          gtk_tree_view_set_cursor (GTK_TREE_VIEW (window->treeview), path, NULL, FALSE);
          gtk_tree_path_free (path);
        }

      gtk_widget_grab_focus (window->treeview);
    }
  else if (gtk_widget_get_sensitive (window->button_launch))
    {
      gtk_button_clicked (GTK_BUTTON (window->button_launch));
    }
}



static gboolean
xfce_appfinder_window_entry_key_press_event (GtkWidget           *entry,
                                             GdkEventKey         *event,
                                             XfceAppfinderWindow *window)
{
  gboolean     expand;
  const gchar *text;

  if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down)
    {
      /* only switch modes when there is no text in the entry */
      text = gtk_entry_get_text (GTK_ENTRY (window->entry));
      if (IS_STRING (text))
        return FALSE;

      expand = (event->keyval == GDK_KEY_Down);
      if (gtk_widget_get_visible (window->paned) != expand)
        {
          xfce_appfinder_window_set_expanded (window, expand);
          return TRUE;
        }
    }

  return FALSE;
}



static void
xfce_appfinder_window_drag_begin (GtkWidget           *widget,
                                  GdkDragContext      *drag_context,
                                  XfceAppfinderWindow *window)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  GdkPixbuf        *pixbuf;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (window->treeview));
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, XFCE_APPFINDER_MODEL_COLUMN_ICON_SMALL, &pixbuf, -1);
      if (G_LIKELY (pixbuf != NULL))
        {
          gtk_drag_set_icon_pixbuf (drag_context, pixbuf, 0, 0);
          g_object_unref (G_OBJECT (pixbuf));
        }
    }
  else
    {
      gtk_drag_set_icon_stock (drag_context, GTK_STOCK_DIALOG_ERROR, 0, 0);
    }
}



static void
xfce_appfinder_window_drag_data_get (GtkWidget           *widget,
                                     GdkDragContext      *drag_context,
                                     GtkSelectionData    *data,
                                     guint                info,
                                     guint                drag_time,
                                     XfceAppfinderWindow *window)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  gchar            *uris[2];

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (window->treeview));
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      uris[1] = NULL;
      gtk_tree_model_get (model, &iter, XFCE_APPFINDER_MODEL_COLUMN_URI, &uris[0], -1);
      gtk_selection_data_set_uris (data, uris);
      g_free (uris[0]);
    }
}



static void
xfce_appfinder_window_entry_icon_released (GtkEntry             *entry,
                                           GtkEntryIconPosition  icon_pos,
                                           GdkEvent             *event,
                                           XfceAppfinderWindow  *window)
{
  if (icon_pos == GTK_ENTRY_ICON_SECONDARY)
    xfce_appfinder_window_set_expanded (window, !gtk_widget_get_visible (window->paned));
}



static void
xfce_appfinder_window_category_changed (GtkTreeSelection    *selection,
                                        XfceAppfinderWindow *window)
{
  GtkTreeIter   iter;
  GtkTreeModel *model;
  gchar        *category;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, XFCE_APPFINDER_CATEGORY_MODEL_COLUMN_NAME, &category, -1);

      g_free (window->filter_category);

      if (g_strcmp0 (category, _("All Applications")) == 0)
        window->filter_category = NULL;
      else if (g_strcmp0 (category, _("Commands History")) == 0)
        window->filter_category = g_strdup ("\0");
      else
        window->filter_category = g_strdup (category);

      g_free (category);

      model = gtk_tree_view_get_model (GTK_TREE_VIEW (window->treeview));
      gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (model));
    }
}



static gboolean
xfce_appfinder_window_item_visible (GtkTreeModel *model,
                                    GtkTreeIter  *iter,
                                    gpointer      data)
{
  XfceAppfinderWindow *window = XFCE_APPFINDER_WINDOW (data);

  return xfce_appfinder_model_get_visible (XFCE_APPFINDER_MODEL (model), iter,
                                           window->filter_category, window->filter_text);
}



static void
xfce_appfinder_window_item_changed (XfceAppfinderWindow *window)
{
  GtkTreeIter       iter;
  GtkTreeModel     *model;
  gboolean          can_launch;
  GtkTreeSelection *selection;
  GdkPixbuf        *pixbuf;

  if (gtk_widget_get_visible (window->paned))
    {
      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (window->treeview));
      can_launch = gtk_tree_selection_get_selected (selection, &model, &iter);
      gtk_widget_set_sensitive (window->button_launch, can_launch);

      if (can_launch)
        {
          gtk_tree_model_get (model, &iter, XFCE_APPFINDER_MODEL_COLUMN_ICON_LARGE, &pixbuf, -1);
          if (G_LIKELY (pixbuf != NULL))
            {
              xfce_appfinder_window_update_image (window, pixbuf);
              g_object_unref (G_OBJECT (pixbuf));
            }
        }
      else
        {
          xfce_appfinder_window_update_image (window, NULL);
        }
    }
}



static void
xfce_appfinder_window_row_activated (XfceAppfinderWindow *window)
{
  if (gtk_widget_get_sensitive (window->button_launch))
    gtk_button_clicked (GTK_BUTTON (window->button_launch));
}



static void
xfce_appfinder_window_icon_theme_changed (XfceAppfinderWindow *window)
{
  if (window->icon_find != NULL)
    g_object_unref (G_OBJECT (window->icon_find));
  window->icon_find = xfce_appfinder_model_load_pixbuf (GTK_STOCK_FIND, ICON_LARGE);

  /* drop cached pixbufs */
  xfce_appfinder_model_icon_theme_changed (window->model);
  xfce_appfinder_category_model_icon_theme_changed (window->category_model);

  /* update state */
  xfce_appfinder_window_entry_changed (window);
  xfce_appfinder_window_item_changed (window);
}



static gboolean
xfce_appfinder_window_execute_command (const gchar  *cmd,
                                       GdkScreen    *screen,
                                       GError      **error)
{
  gboolean          in_terminal;
  gchar            *cmdline, *exo_open;
  const gchar      *exo_open_prefix[] = { "file://", "http://", "https://" };
  guint             i;
  gboolean          result = FALSE;

  if (g_str_has_prefix (cmd, "#"))
    {
      /* open manual page in the terminal */
      cmdline = g_strconcat ("man ", cmd + 1, NULL);
      in_terminal = TRUE;
    }
  else if (g_str_has_prefix (cmd, "$"))
    {
      /* open in the terminal */
      cmdline = xfce_expand_variables (cmd + 1, NULL);
      in_terminal = TRUE;
    }
  else
    {
      cmdline = xfce_expand_variables (cmd, NULL);
      in_terminal = FALSE;
    }

  result = xfce_spawn_command_line_on_screen (screen, cmdline, in_terminal, FALSE, error);
  if (!result)
    {
      /* TODO instead check the exo exit code */
      /* check if this is something exo-open can handle */
      for (i = 0; !result && i < G_N_ELEMENTS (exo_open_prefix); i++)
        if (g_str_has_prefix (cmdline, exo_open_prefix[i]))
          result = TRUE;

      if (result)
        {
          /* try to spawn again */
          exo_open = g_strconcat ("exo-open ", cmdline, NULL);
          result = xfce_spawn_command_line_on_screen (screen, exo_open, FALSE, FALSE, error);
          g_free (exo_open);
        }
    }

  g_free (cmdline);

  return result;
}



static void
xfce_appfinder_window_execute (XfceAppfinderWindow *window)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter, orig;
  GError           *error = NULL;
  gboolean          result = FALSE;
  GdkScreen        *screen;
  const gchar      *text;
  gchar            *cmd = NULL;
  gboolean          regular_command = FALSE;

  if (!gtk_widget_get_sensitive (window->button_launch))
    return;

  screen = gtk_window_get_screen (GTK_WINDOW (window));
  if (gtk_widget_get_visible (window->paned))
    {
      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (window->treeview));
      if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {
          gtk_tree_model_filter_convert_iter_to_child_iter (GTK_TREE_MODEL_FILTER (model), &orig, &iter);
          result = xfce_appfinder_model_execute (window->model, &orig, screen, &regular_command, &error);

         if (!result && regular_command)
            {
              gtk_tree_model_get (model, &iter, XFCE_APPFINDER_MODEL_COLUMN_COMMAND, &cmd, -1);
              result = xfce_appfinder_window_execute_command (cmd, screen, &error);
              g_free (cmd);
            }
        }
    }
  else
    {
      text = gtk_entry_get_text (GTK_ENTRY (window->entry));
      if (xfce_appfinder_window_execute_command (text, screen, &error))
        result = xfce_appfinder_model_save_command (window->model, text, &error);
    }

  gtk_entry_set_icon_from_stock (GTK_ENTRY (window->entry), GTK_ENTRY_ICON_PRIMARY,
                                 result ? NULL : GTK_STOCK_DIALOG_ERROR);
  gtk_entry_set_icon_tooltip_text (GTK_ENTRY (window->entry), GTK_ENTRY_ICON_PRIMARY,
                                   error != NULL ? error->message : NULL);

  if (error != NULL)
    {
      g_warning ("Failed to execute: %s", error->message);
      g_error_free (error);
    }

  if (result)
    gtk_widget_destroy (GTK_WIDGET (window));
}



void
xfce_appfinder_window_set_expanded (XfceAppfinderWindow *window,
                                    gboolean             expanded)
{
  GdkGeometry         hints;
  gint                width;
  GtkWidget          *parent;
  GtkEntryCompletion *completion;

  APPFINDER_DEBUG ("set expand = %s", expanded ? "true" : "false");

  /* force window geomentry */
  if (expanded)
    {
      gtk_window_set_geometry_hints (GTK_WINDOW (window), NULL, NULL, 0);
      gtk_window_get_size (GTK_WINDOW (window), &width, NULL);
      gtk_window_resize (GTK_WINDOW (window), width, window->last_window_height);
    }
  else
    {
      if (gtk_widget_get_visible (GTK_WIDGET (window)))
        gtk_window_get_size (GTK_WINDOW (window), NULL, &window->last_window_height);

      hints.max_height = -1;
      hints.max_width = G_MAXINT;
      gtk_window_set_geometry_hints (GTK_WINDOW (window), NULL, &hints, GDK_HINT_MAX_SIZE);
    }

  /* repack the button box */
  g_object_ref (G_OBJECT (window->bbox));
  parent = gtk_widget_get_parent (window->bbox);
  if (parent != NULL)
    gtk_container_remove (GTK_CONTAINER (parent), window->bbox);
  if (expanded)
    gtk_container_add (GTK_CONTAINER (window->bin_expanded), window->bbox);
  else
    gtk_container_add (GTK_CONTAINER (window->bin_collapsed), window->bbox);
  gtk_widget_set_visible (window->bin_expanded, expanded);
  gtk_widget_set_visible (window->bin_collapsed, !expanded);
  g_object_unref (G_OBJECT (window->bbox));

  /* show/hide pane with treeviews */
  gtk_widget_set_visible (window->paned, expanded);

  /* toggle icon */
  gtk_entry_set_icon_from_icon_name (GTK_ENTRY (window->entry), GTK_ENTRY_ICON_SECONDARY,
                                     expanded ? GTK_STOCK_GO_UP : GTK_STOCK_GO_DOWN);
  gtk_entry_set_icon_from_icon_name (GTK_ENTRY (window->entry), GTK_ENTRY_ICON_PRIMARY, NULL);

  /* update completion (remove completed text of restart completion) */
  completion = gtk_entry_get_completion (GTK_ENTRY (window->entry));
  if (completion != NULL)
    gtk_editable_delete_selection (GTK_EDITABLE (window->entry));
  gtk_entry_set_completion (GTK_ENTRY (window->entry), expanded ? NULL : window->completion);
  if (!expanded)
    gtk_entry_completion_insert_prefix (window->completion);

  /* update state */
  xfce_appfinder_window_entry_changed (window);
  xfce_appfinder_window_item_changed (window);
}
