/*
 * Mokojeweled
 * Main menu
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
#include "gui.h"
#include "board.h"
#include "menu.h"

// menu window
static MokoWin* win = NULL;

static void _close(void *data, Evas_Object* obj, void* event)
{
    // quit now!
    elm_exit();
}

static void _new_game2(MokoInwin* popup, void* data, int index, bool final)
{
    EINA_LOG_DBG("NEW GAME index=%d, final=%d", index, final);

    board_new_game(index);
}

static void _new_game(void *data, Evas_Object* obj, void* event)
{
    MokoPopupMenu* ng = moko_popup_menu_new(win, NULL, MOKO_POPUP_BUTTONS, _new_game2, NULL);

    moko_popup_menu_add(ng, _("Normal game"), GAME_TYPE_NORMAL, FALSE);
    moko_popup_menu_add(ng, _("Timed game"), GAME_TYPE_TIMED, FALSE);

    mokoinwin_activate(MOKO_INWIN(ng));
}

static Evas_Object* add_menu_button(Evas_Object* box, const char* title, Evas_Smart_Cb callback, const void* data)
{
    Evas_Object* button = elm_button_add(win->win);
    elm_button_label_set(button, title);
    evas_object_smart_callback_add(button, "clicked", callback, data);

    evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(button);

    elm_box_pack_end(box, button);
    return button;
}

static void create_win(void)
{
    win = mokowin_new(PACKAGE, TRUE);
    win->delete_callback = _close;

    mokowin_create_vbox(win, FALSE);

    elm_win_title_set(win->win, PACKAGE_NAME);
    evas_object_resize(win->win, WIN_WIDTH, WIN_HEIGHT);

    Evas_Object* fr = elm_frame_add(win->win);
    elm_object_style_set(fr, "pad_large");
    evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(fr, 0.5, 0.0);
    elm_box_pack_start(win->vbox, fr);
    evas_object_show(fr);

    // title
    Evas_Object* title = elm_label_add(win->win);
    elm_label_label_set(title, PACKAGE_NAME);
    evas_object_size_hint_weight_set(title, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(title, 0.5, 0.0);
    elm_frame_content_set(fr, title);
    evas_object_show(title);

    // buttons box
    Evas_Object* btn_box = elm_box_add(win->win);
    elm_box_homogenous_set(btn_box, TRUE);
    evas_object_size_hint_weight_set(btn_box, EVAS_HINT_EXPAND, 0.8);
    evas_object_size_hint_align_set(btn_box, EVAS_HINT_FILL, -1.0);
    evas_object_show(btn_box);
    elm_box_pack_end(win->vbox, btn_box);

    // buttons

    // TODO continue game
    elm_object_disabled_set(
        add_menu_button(btn_box, _("Continue game"), NULL, NULL),
        TRUE);

    add_menu_button(btn_box, _("New game"), _new_game, NULL);

    // TODO options
    elm_object_disabled_set(
        add_menu_button(btn_box, _("Options"), NULL, NULL),
        TRUE);

    add_menu_button(btn_box, _("Exit"), _close, NULL);
}

void menu(void)
{
    if (!win)
        create_win();

    mokowin_activate(win);
}
