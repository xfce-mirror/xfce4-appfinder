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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <garcon/garcon.h>

#include <src/appfinder-window.h>



static gboolean  opt_finder = FALSE;
static gboolean  opt_version = FALSE;
static gchar    *opt_filename = NULL;



static GOptionEntry option_entries[] =
{
  { "finder", '\0', 0, G_OPTION_ARG_NONE, &opt_finder, N_("Start in expanded mode"), NULL },
  { "version", 'V', 0, G_OPTION_ARG_NONE, &opt_version, N_("Print version information and exit"), NULL },
  { G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME, &opt_filename, NULL, NULL },
  { NULL }
};



gint
main (gint argc, gchar **argv)
{
  GError      *error = NULL;
  GtkWidget   *window;
  const gchar *desktop;

  /* set translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

#ifdef G_ENABLE_DEBUG
  /* do NOT remove this line for now, If something doesn't work,
   * fix your code instead! */
  g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);
#endif

  if (!g_thread_supported ())
    g_thread_init (NULL);

  if (!gtk_init_with_args (&argc, &argv, _("[MENUFILE]"), option_entries, GETTEXT_PACKAGE, &error))
    {
      g_printerr ("%s: %s.\n", PACKAGE_NAME, error->message);
      g_printerr (_("Type \"%s --help\" for usage."), PACKAGE_NAME);
      g_printerr ("\n");
      g_error_free (error);

      return EXIT_FAILURE;
    }

  if (opt_version)
    {
      g_print ("%s %s (Xfce %s)\n\n", PACKAGE_NAME, PACKAGE_VERSION, xfce_version_string ());
      g_print ("%s\n", "Copyright (c) 2004-2010");
      g_print ("\t%s\n\n", _("The Xfce development team. All rights reserved."));
      g_print (_("Please report bugs to <%s>."), PACKAGE_BUGREPORT);
      g_print ("\n");

      return EXIT_SUCCESS;
    }

  /* if the value is unset, fallback to XFCE, if the
   * value is empty, allow all applications in the menu */
  desktop = g_getenv ("XDG_CURRENT_DESKTOP");
  if (G_LIKELY (desktop == NULL))
    desktop = "XFCE";
  else if (*desktop == '\0')
    desktop = NULL;
  garcon_set_environment (desktop);

  window = xfce_appfinder_window_new ();
  xfce_appfinder_window_set_expanded (XFCE_APPFINDER_WINDOW (window), opt_finder);
  gtk_widget_show (window);

  g_debug ("enter mainloop");

  gtk_main ();

  gtk_widget_hide (window);
  gtk_widget_destroy (window);

  return EXIT_SUCCESS;
}
