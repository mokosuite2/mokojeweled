/*
 * Mokojeweled
 * Game themes
 * Copyright (C) 2009-2010 Daniele Ricci <daniele.athome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include "globals.h"
#include "theme.h"

// FIXME everything is hardcoded now!!

const char* theme_get_path(void)
{
    return DATADIR "/" PACKAGE "/themes/gweled.edj";
}

int theme_gem_count(void)
{
    return 7;
}

// FIXME hardcoded :D
void theme_init(const char* name)
{
    elm_theme_extension_add(NULL, theme_get_path());
    elm_theme_overlay_add(NULL, "elm/bg/base/board");
}
