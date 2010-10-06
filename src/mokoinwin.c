/*
 * Mokojeweled
 * GUI utilities
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

#include "gui.h"

void mokoinwin_activate(MokoInwin *mw)
{
    evas_object_show(mw->inwin);
    elm_win_inwin_activate(mw->inwin);
}

void mokoinwin_hide(MokoInwin *mw)
{
    evas_object_hide(mw->inwin);
}

void mokoinwin_destroy(MokoInwin *mw)
{
    if (mw != NULL) {

        // prima richiama il callback
        if (mw->delete_callback)
            (mw->delete_callback)(mw);

        // distruggi finestra
        evas_object_del(mw->inwin);
    
        // distruggi istanza
        free(mw);
    }
}

MokoInwin *mokoinwin_new(MokoWin *parent)
{
    MokoInwin *mw = m_new0(MokoInwin, 1);

    /* costruisci la finestra */
    mw->inwin = elm_win_inwin_add(parent->win);

    if(mw->inwin == NULL) {
        EINA_LOG_WARN("Cannot instantiate inner window");
        free(mw);
        return NULL;
    }

    // MALEDETTO!!!!!
    //elm_win_resize_object_add(parent->win, mw->inwin);

    return mw;
}
