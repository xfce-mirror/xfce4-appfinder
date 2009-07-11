/* vi:set sw=2 sts=2 ts=2 et ai: */
/*-
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4menu/libxfce4menu.h>
#include <xfconf/xfconf.h>

#include "xfce-appfinder-window.h"



static gboolean     opt_version = FALSE;
static gchar      **opt_remaining = NULL;
static GOptionEntry opt_entries[] = {
  { "version", 'V', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &opt_version, N_("Version information"), NULL },
  { G_OPTION_REMAINING, 0, G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_FILENAME_ARRAY, &opt_remaining, NULL, N_("[MENUFILE]") },
};



int
main (int    argc,
      char **argv)
{
  GtkWidget *window;
  GError    *error = NULL;

  /* Set up translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* Initialize GTK+ and parse command line options */
  if (G_UNLIKELY (!gtk_init_with_args (&argc, &argv, NULL, opt_entries, PACKAGE, &error)))
    {
      if (G_LIKELY (error != NULL))
        {
          g_print ("%s: %s.\n", G_LOG_DOMAIN, error->message);
          g_print (_("Type '%s --help' for usage information."), G_LOG_DOMAIN);
          g_print ("\n");

          g_error_free (error);
        }
      else
        g_error (_("Unable to initialize GTK+."));

      return EXIT_FAILURE;
    }

  /* Print version info and quit whe the user entered --version or -V */
  if (G_UNLIKELY (opt_version))
    {
      g_print ("%s %s (Xfce %s)\n\n", G_LOG_DOMAIN, PACKAGE_VERSION, xfce_version_string ());
      g_print ("%s\n", "Copyright (c) 2008-2009");
      g_print ("\t%s\n\n", _("The Xfce development team. All rights reserved."));
      g_print (_("Please report bugs to <%s>."), PACKAGE_BUGREPORT);
      g_print ("\n");

      return EXIT_SUCCESS;
    }

  /* Initialize xfconf */
  if (G_UNLIKELY (!xfconf_init (&error)))
    {
      if (G_LIKELY (error != NULL))
        {
          g_error (_("Failed to connect to xfconf daemon. Reason: %s"), error->message);
          g_error_free (error);
        }
      else
        g_error (_("Failed to connect to xfconf daemon."));

      return EXIT_FAILURE;
    }

  /* Initialize menu library */
  xfce_menu_init ("XFCE");

  window = xfce_appfinder_window_new (opt_remaining != NULL ? opt_remaining[0] : NULL);
  xfce_appfinder_window_reload (XFCE_APPFINDER_WINDOW (window));
  gtk_widget_show (window);

  gtk_main ();

  /* Shutdown libraries */
  xfce_menu_shutdown ();
  xfconf_shutdown ();

  return EXIT_SUCCESS;
}
