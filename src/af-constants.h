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

#ifndef __HAVE_AF_CONSTANTS_H
#define __HAVE_AF_CONSTANTS_H

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

/* What to search for in .desktop files */
static const char *keys [] =
{
    "Name",
    "Comment",
    "Icon",
    "Categories",
    "OnlyShowIn",
    "Exec",
    "Terminal",
    NULL
};

static const char *i18ncategories [] =
{
    N_("All"),
    N_("Recently Used"),
    N_("Core"),
    N_("Development"),
    N_("Office"),
    N_("Graphics"),
    N_("Network"),
    N_("AudioVideo"),
    N_("Game"),
    N_("Education"),
    N_("System"),
    N_("Filemanager"),
    N_("Utility"),
    NULL
};

#endif
