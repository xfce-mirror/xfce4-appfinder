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

GtkTargetEntry gte[] = {{"DESKTOP_PATH_ENTRY", 0, 0},
	{"text/plain", 0, 1},
	{"application/x-desktop", 0, 2},
	{"STRING", 0, 3},
	{"UTF8_STRING", 0, 4}
};
