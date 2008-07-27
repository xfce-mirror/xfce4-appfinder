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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4menu/libxfce4menu.h>

#include "appfinder.h"

#ifndef _
#define _(x) x
#endif

#define MENUFILE        SYSCONFDIR "/xdg/menus/xfce-applications.menu"

/* applications menu */
static XfceMenu *root_menu = NULL;
static volatile int menu_is_ready = 0;

static gpointer create_menu (gpointer data);
static gboolean wait_for_menu (gpointer data);

/* command line options */
static gboolean opt_version = FALSE;
static char **opt_others = NULL;
static char *menufile = NULL;

static GOptionEntry option_entries[] = {
  {"version", 'v', 0, G_OPTION_ARG_NONE, &opt_version, N_("Show version information"), NULL},
  {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &opt_others, NULL, N_("[MENUFILE]")},
  {NULL}
};

/* main program */
gint
main (gint argc, gchar ** argv)
{
  GError *error = NULL;

  xfce_textdomain (GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");

  g_thread_init (NULL);

  g_set_application_name (PACKAGE_NAME);

  /* initialize gtk */
  if (!gtk_init_with_args (&argc, &argv, "", option_entries, GETTEXT_PACKAGE, &error))
    {
      g_print ("%s: %s\n", PACKAGE_NAME, error ? error->message : _("Failed to open display"));

      if (error != NULL)
        g_error_free (error);

      return EXIT_FAILURE;
    }

  /* handle the options */
  if (G_UNLIKELY (opt_version))
    {
      g_print ("%s %s\n\n", PACKAGE_NAME, PACKAGE_VERSION);
      g_print ("%s\n", _("Copyright (c) 2008"));
      g_print ("\t%s\n\n", _("The Xfce development team. All rights reserved."));
      g_print (_("Please report bugs to <%s>."), PACKAGE_BUGREPORT);
      g_print ("\n");

      return EXIT_SUCCESS;
    }
  if (G_UNLIKELY (opt_others))
    {
      menufile = opt_others[0];
    }

  /* find the applications */
  g_thread_create ((GThreadFunc) create_menu, NULL, FALSE, &error);

  /* build the GUI */
  appfinder_create_shell ();

  /* wait for menu to be parsed */
  g_idle_add ((GSourceFunc) wait_for_menu, NULL);

  gtk_main ();

  if (menu_is_ready)
    {
      xfce_menu_shutdown ();
    }

  return 0;
}



static gpointer
create_menu (gpointer data)
{
  GError *error = NULL;

  xfce_menu_init ("XFCE");

  if (menufile && g_file_test (menufile, G_FILE_TEST_EXISTS))
    {
      root_menu = xfce_menu_new (menufile, &error);
    }
  else if (g_file_test (MENUFILE, G_FILE_TEST_EXISTS))
    {
      root_menu = xfce_menu_new (MENUFILE, &error);
    }
  else
    {
      root_menu = xfce_menu_get_root (&error);
    }

  if (G_UNLIKELY (root_menu == NULL))
    {
      if (error)
        {
          g_critical (error->message);
          g_error_free (error);
        }
    }
  else
    {
      appfinder_set_menu (root_menu);
    }

  menu_is_ready = 1;
  return NULL;                  /* return value not used */
}



static gboolean
wait_for_menu (gpointer data)
{
  if (!menu_is_ready)
    return TRUE;

  appfinder_add_applications ();

  return FALSE;
}
