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
*/

#define APPFINDER_ALL 0
#define APPFINDER_HISTORY 1

enum
{
  APP_ICON = 0,
  APP_TEXT,
  APP_COLS
};

enum
{
  CAT_TEXT = 0,
  CAT_COLS
};

#if 0
/* Some standard paths for .desktop files */
const char *entriespaths [] = {
	"/usr/share/applications/",
	"/usr/share/applications/kde/",
	"/usr/local/share/applications/",
	"/usr/local/share/applications/kde/",
	"/opt/kde/share/applications/kde/",
	"/usr/X11R6/share/",
	"/opt/gnome/share/applications/",
	"/opt/gnome2/share/applications/",
	NULL
};


/* Some standard paths for icons */
const char *iconspaths [] = {
	"/usr/share/pixmaps/",
	"/usr/share/icons/default.kde/32x32/apps/",
	"/usr/share/icons/default.kde/32x32/devices/",
	"/usr/share/icons/default.kde/32x32/actions/",
	"/usr/share/icons/default.kde/32x32/mimetypes/",
	"/usr/share/icons/default.kde/32x32/filesystems/",
	"/opt/kde/share/icons/default.kde/32x32/apps/",
	"/opt/kde/share/icons/default.kde/32x32/devices/",
	"/opt/kde/share/icons/default.kde/32x32/actions/",
	"/opt/kde/share/icons/default.kde/32x32/mimetypes/",
	"/opt/kde/share/icons/default.kde/32x32/filesystems/",
	NULL
};
#endif

/* What to search for in .desktop files */
const char *keys [] = {
	"Name",
	"Comment",
	"Icon",
	"Categories",
	"OnlyShowIn",
	"Exec",
	"Terminal",
	NULL
};

const char *categories [] = {
	"All",
	"Recently Used",
	"Core",
	"Development",
	"Office",
	"Graphics",
	"Network",
	"AudioVideo",
	"Game",
	"Education",
	"System",
	"Filemanager",
	"Utility",
	NULL
};

const char *i18ncategories [] = {
	_("All"),
	_("Recently Used"),
	_("Core"),
	_("Development"),
	_("Office"),
	_("Graphics"),
	_("Network"),
	_("AudioVideo"),
	_("Game"),
	_("Education"),
	_("System"),
	_("Filemanager"),
	_("Utility"),
	NULL
};

/* Places where i can drop things */
GtkTargetEntry gte[] = {{"DESKTOP_PATH_ENTRY", 0, 0},
	{"text/plain", 0, 1},
	{"application/x-desktop", 0, 2},
	{"STRING", 0, 3},
	{"UTF8_STRING", 0, 4}
};
