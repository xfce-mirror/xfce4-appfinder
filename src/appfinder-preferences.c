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
#include <xfconf/xfconf.h>

#include <src/appfinder-preferences.h>
#include <src/appfinder-preferences-ui.h>
#include <src/appfinder-model.h>
#include <src/appfinder-private.h>



static void xfce_appfinder_preferences_response      (GtkWidget                *window,
                                                      gint                      response_id,
                                                      XfceAppfinderPreferences *preferences);
static void xfce_appfinder_preferences_clear_history (XfceAppfinderPreferences *preferences);



struct _XfceAppfinderPreferencesClass
{
  GtkBuilderClass __parent__;
};

struct _XfceAppfinderPreferences
{
  GtkBuilder __parent__;

  GObject  *dialog;
};



G_DEFINE_TYPE (XfceAppfinderPreferences, xfce_appfinder_preferences, GTK_TYPE_BUILDER)



static void
xfce_appfinder_preferences_class_init (XfceAppfinderPreferencesClass *klass)
{
}



static void
xfce_appfinder_preferences_init (XfceAppfinderPreferences *preferences)
{
  XfconfChannel *channel;
  GObject       *object;

  /* load the builder data into the object */
  gtk_builder_add_from_string (GTK_BUILDER (preferences), appfinder_preferences_ui,
                               appfinder_preferences_ui_length, NULL);

  preferences->dialog = gtk_builder_get_object (GTK_BUILDER (preferences), "dialog");
  appfinder_return_if_fail (XFCE_IS_TITLED_DIALOG (preferences->dialog));
  g_signal_connect (G_OBJECT (preferences->dialog), "response",
      G_CALLBACK (xfce_appfinder_preferences_response), preferences);

  channel = xfconf_channel_get ("xfce4-appfinder");

  object = gtk_builder_get_object (GTK_BUILDER (preferences), "remember-category");
  xfconf_g_property_bind (channel, "/RememberCategory", G_TYPE_BOOLEAN,
                          G_OBJECT (object), "active");

  object = gtk_builder_get_object (GTK_BUILDER (preferences), "always-center");
  xfconf_g_property_bind (channel, "/AlwaysCenter", G_TYPE_BOOLEAN,
                          G_OBJECT (object), "active");

  object = gtk_builder_get_object (GTK_BUILDER (preferences), "button-clear");
  g_signal_connect_swapped (G_OBJECT (object), "clicked",
      G_CALLBACK (xfce_appfinder_preferences_clear_history), preferences);
}



static void
xfce_appfinder_preferences_response (GtkWidget                *window,
                                     gint                      response_id,
                                     XfceAppfinderPreferences *preferences)
{
  appfinder_return_if_fail (GTK_IS_DIALOG (window));
  appfinder_return_if_fail (XFCE_IS_APPFINDER_PREFERENCES (preferences));

  gtk_widget_destroy (window);
  g_object_unref (G_OBJECT (preferences));
}



static void
xfce_appfinder_preferences_clear_history (XfceAppfinderPreferences *preferences)
{
  XfceAppfinderModel *model;
  
  appfinder_return_if_fail (XFCE_IS_APPFINDER_PREFERENCES (preferences));
  
  if (xfce_dialog_confirm (GTK_WINDOW (preferences->dialog), GTK_STOCK_CLEAR,
                           _("Clear Command History"),
                           _("This will permanently clear the custom command history."),
                           _("Are you sure you want to clear the command history?")))
    {
      model = xfce_appfinder_model_get ();
      xfce_appfinder_model_commands_clear (model);
      g_object_unref (G_OBJECT (model));
    }
}



void
xfce_appfinder_preferences_show (GdkScreen *screen)
{
  static XfceAppfinderPreferences *preferences = NULL;

  appfinder_return_if_fail (GDK_IS_SCREEN (screen));

  if (preferences == NULL)
    {
      preferences = g_object_new (XFCE_TYPE_APPFINDER_PREFERENCES, NULL);
      g_object_add_weak_pointer (G_OBJECT (preferences), (gpointer *) &preferences);
      gtk_widget_show (GTK_WIDGET (preferences->dialog));
    }
  else
    {
      gtk_window_present (GTK_WINDOW (preferences->dialog));
    }

  gtk_window_set_screen (GTK_WINDOW (preferences->dialog), screen);
}
