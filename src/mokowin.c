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

#define MENU_MAX_PRESS_TIME     800

static void _remove_inwin_index(MokoWin* mw, int index)
{
    MokoInwin* inwin = MOKO_INWIN(eina_list_nth(mw->inners, index));
    mokoinwin_destroy(inwin);
    mw->inners = eina_list_remove(mw->inners, inwin);
}

static void _close(void *mokowin, Evas_Object *obj, void *event_info)
{
    MokoWin *mw = (MokoWin*)mokowin;

    int inwin_count = eina_list_count(mw->inners);
    if (inwin_count > 0) {
        // rimuovi la inwin e distruggila
        _remove_inwin_index(mw, inwin_count - 1);
        return;
    }

    if (mw->menu_hover != NULL) {
        EINA_LOG_DBG("Hover visible: %d", (int)evas_object_visible_get(mw->menu_hover));
        if (evas_object_visible_get(mw->menu_hover)) {
            mokowin_menu_hide(mw);
            return;
        }
    }

    if (mw->delete_callback != NULL)
        (mw->delete_callback)(mokowin, obj, event_info);
}

static void _focus_change(void *mokowin, Evas_Object *obj, void *event_info)
{
    MokoWin *mw = (MokoWin*)mokowin;

    // FIXME non funziona... il window manager da il focus alla finestra??!?!
    //if (mw->menu_hover != NULL && evas_object_visible_get(mw->menu_hover))
    //    mokowin_menu_hide(mw);

    if (mw->focus_callback != NULL)
        (mw->focus_callback)(mokowin, obj, event_info);

}

// menu hover dismesso, nascondi tutto
static void _menu_hover_dismissed(void *mokowin, Evas_Object *obj, void *event_info)
{
    mokowin_menu_hide(MOKO_WIN(mokowin));
}

static void _key_down(void *mokowin, Evas *e, Evas_Object *obj, void *event_info)
{
    // FIXME bisogna aggiustare sta cosa pero'...
    MokoWin *mw = (MokoWin*)mokowin;
    Evas_Event_Key_Down *event = (Evas_Event_Key_Down*)event_info;
    EINA_LOG_DBG("Keyboard(down): %s, %u, %d", event->keyname, event->timestamp, (int) (event->timestamp - mw->last_keystamp));
    mw->key_down = TRUE;
}

static void _key_up(void *mokowin, Evas *e, Evas_Object *obj, void *event_info)
{
    MokoWin *mw = (MokoWin*)mokowin;

    Evas_Event_Key_Up *event = (Evas_Event_Key_Up*)event_info;
    EINA_LOG_DBG("Keyboard(up): %s, %u, %d", event->keyname, event->timestamp, (int) (event->timestamp - mw->last_keystamp));

    if ((!strcmp(event->keyname, MENU_KEY_PRIMARY1) ||
         !strcmp(event->keyname, MENU_KEY_PRIMARY2) ||
         !strcmp(event->keyname, MENU_KEY_SECONDARY)) &&
        ((event->timestamp - mw->last_keystamp) > 500) && mw->key_down) {

        // aggiorna il timestamp
        mw->last_keystamp = event->timestamp;
        mw->key_down = FALSE;

        int inwin_count = eina_list_count(mw->inners);
        // se non ci sono inners, comportati normalmente
        if (inwin_count == 0 && mw->menu_enable) {

            // switcha la visibilita' del menu
            if (evas_object_visible_get(mw->menu_hover)) {
                mokowin_menu_hide(mw);
            } else {
                mokowin_menu_show(mw);
            }

        // altrimenti rimuovi l'ultima inner
        } else if (inwin_count > 0) {
            // rimuovi la inwin e distruggila
            _remove_inwin_index(mw, inwin_count - 1);
        }

    }

}

Evas_Object* mokowin_menu_hover_button(MokoWin* mw, Evas_Object* table, const char* label, int x, int y, int w, int h)
{
    Evas_Object *bt = elm_button_add(mw->win);
    elm_button_label_set(bt, label);

    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(bt);
    elm_table_pack(table, bt, x, y, w, h);

    return bt;
}

Evas_Object* mokowin_vbox_button(MokoWin* mw, const char* label, Evas_Object* after, Evas_Object* before)
{
    Evas_Object *bt = elm_button_add(mw->win);
    elm_button_label_set(bt, label);

    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(bt);

    if (after)
        elm_box_pack_after(mw->vbox, bt, after);

    else if (before)
        elm_box_pack_before(mw->vbox, bt, before);

    else
        elm_box_pack_end(mw->vbox, bt);

    return bt;
}

Evas_Object* mokowin_vbox_button_with_callback(MokoWin* mw, const char* label,
    Evas_Object* after, Evas_Object* before, Evas_Smart_Cb callback, void* data)
{
    Evas_Object* bt = mokowin_vbox_button(mw, label, after, before);
    evas_object_smart_callback_add(bt, "clicked", callback, data);

    return bt;
}

void mokowin_delete_data_set(MokoWin *mw, void* data)
{
    evas_object_smart_callback_del (mw->win, "delete,request", _close);
    evas_object_smart_callback_add (mw->win, "delete,request", _close, data);
}

void mokowin_menu_hide(MokoWin *mw)
{
    if (mw->menu_hover != NULL) {

        if (mw->layout != NULL)
            edje_object_signal_emit(mw->layout_edje, "hide", "menu_hover");

        evas_object_hide(mw->menu_hover);
    }
}

void mokowin_menu_show(MokoWin *mw)
{
    if (mw->menu_hover != NULL) {

        if (mw->layout != NULL)
            edje_object_signal_emit(mw->layout_edje, "show", "menu_hover");

        evas_object_show(mw->menu_hover);
    }

}

void mokowin_menu_enable(MokoWin *mw)
{
    return_if_fail(mw->vbox == NULL || mw->layout == NULL);

    if (mw->menu_hover == NULL) {
        mw->menu_hover = elm_hover_add(mw->win);

        evas_object_smart_callback_add(mw->menu_hover, "clicked", _menu_hover_dismissed, mw);

        evas_object_size_hint_weight_set(mw->menu_hover, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(mw->menu_hover, EVAS_HINT_FILL, 0.0);

        elm_object_style_set(mw->menu_hover, "popout");
        elm_hover_parent_set(mw->menu_hover, mw->win);
        elm_hover_target_set(mw->menu_hover, mw->win);

        if (mw->vbox != NULL && mw->scroller == NULL)
            elm_box_pack_end(mw->vbox, mw->menu_hover);

        if (mw->vbox != NULL && mw->scroller != NULL)
            elm_box_pack_end(mw->scroller_vbox, mw->menu_hover);

        if (mw->layout != NULL)
            elm_layout_content_set(mw->layout, "menu_hover", mw->menu_hover);
    }
}

void mokowin_menu_set(MokoWin *mw, Evas_Object *box)
{
    // menu_hover disabilitato/tentiamo di impostare lo stesso menu
    return_if_fail(mw->menu_hover != NULL);
    if (mw->current_menu == box) return;

    mokowin_menu_hide(mw);
    if (mw->current_menu != NULL)
        evas_object_hide(mw->current_menu);

    if (box == NULL) {
        mw->menu_enable = FALSE;
        if (mw->current_menu)
            evas_object_del(mw->current_menu);

        mw->current_menu = NULL;

    } else {
        evas_object_show(box);

        mw->current_menu = box;
        elm_hover_content_set(mw->menu_hover, "top", box);

        mw->menu_enable = TRUE;
    }
}

void mokowin_inner_add(MokoWin *mw, MokoInwin *inwin)
{
    mw->inners = eina_list_append(mw->inners, inwin);
}

void mokowin_inner_remove(MokoWin *mw, MokoInwin *inwin)
{
    mokoinwin_destroy(inwin);
    mw->inners = eina_list_remove(mw->inners, inwin);
}

void mokowin_create_layout(MokoWin *mw, const char *file, const char *group)
{
    // controlla se non abbiamo gia' scelto il layout
    return_if_fail(mw->vbox == NULL);

    if (mw->layout == NULL) {
        mw->layout = elm_layout_add(mw->win);

        evas_object_size_hint_weight_set(mw->layout, 1.0, 1.0);
        elm_win_resize_object_add(mw->win, mw->layout);

        //evas_object_focus_set(mw->layout, TRUE);
        evas_object_show(mw->layout);
    }

    elm_layout_file_set(mw->layout, file, group);
    mw->layout_edje = (Evas_Object*) elm_layout_edje_get (mw->layout);
}

void mokowin_create_vbox(MokoWin *mw, bool scroller)
{
    // controlla se non abbiamo gia' scelto il layout
    return_if_fail(mw->layout == NULL);

    mw->vbox = elm_box_add(mw->win);

    if (scroller) {
        evas_object_size_hint_weight_set(mw->vbox, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(mw->vbox, EVAS_HINT_FILL, 0.0);

        mw->scroller = elm_scroller_add(mw->win);

        evas_object_size_hint_weight_set(mw->scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(mw->scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_scroller_bounce_set(mw->scroller, FALSE, FALSE);
        evas_object_show(mw->scroller);

        //elm_win_resize_object_add(mw->win, mw->scroller);
        elm_scroller_content_set(mw->scroller, mw->vbox);

        // vbox per contenere il menu_hover (TODO aggiungere solo se si abilita il menu_hover)
        mw->scroller_vbox = elm_box_add(mw->win);
        evas_object_size_hint_weight_set(mw->scroller_vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_resize_object_add(mw->win, mw->scroller_vbox);
        evas_object_show(mw->scroller_vbox);

        elm_box_pack_start(mw->scroller_vbox, mw->scroller);

    } else {

        evas_object_size_hint_weight_set(mw->vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_resize_object_add(mw->win, mw->vbox);
    }

    evas_object_show(mw->vbox);
}

/* versione di mokowin_scroll_freeze_set con struttura helper per callback */
void mokowin_scroll_freeze_set_callback(ScrollCallbackData *data)
{
    return_if_fail(data != NULL);
    return_if_fail(data->win != NULL);

    mokowin_scroll_freeze_set(data->win, data->freeze);
}

void mokowin_scroll_freeze_set(MokoWin* mw, bool state)
{
    return_if_fail (mw->scroller != NULL);

    if (state) {
        elm_object_scroll_freeze_push(mw->scroller);
    } else {
        elm_object_scroll_freeze_pop(mw->scroller);
    }
}

void mokowin_activate(MokoWin *mw)
{
    evas_object_show(mw->win);
    elm_win_activate(mw->win);
}

void mokowin_hide(MokoWin *mw)
{
    evas_object_hide(mw->win);
}

void mokowin_destroy(MokoWin *mw)
{
    // distruggi finestra
    evas_object_del(mw->win);

    // distruggi istanza
    free(mw);
}

MokoWin* mokowin_new(const char *name, bool create_bg)
{
    return mokowin_new_with_type(name, ELM_WIN_BASIC, create_bg);
}

MokoWin* mokowin_sized_new(const char *name, size_t size, bool create_bg)
{
    return mokowin_sized_new_with_type(name, size, ELM_WIN_BASIC, create_bg);
}

MokoWin* mokowin_new_with_type(const char *name, Elm_Win_Type type, bool create_bg)
{
    return mokowin_sized_new_with_type(name, sizeof(MokoWin), type, create_bg);
}


MokoWin* mokowin_sized_new_with_type(const char *name, size_t size, Elm_Win_Type type, bool create_bg)
{
    MokoWin *mw = malloc0(size);

    /* costruisci la finestra */
    mw->win = elm_win_add(NULL, name, type);

    if(mw->win == NULL) {
        EINA_LOG_WARN("Cannot instantiate window");
        free(mw);
        return NULL;
    }

    evas_object_smart_callback_add (mw->win, "delete,request", _close, mw);
    evas_object_smart_callback_add (mw->win, "focus,in", _focus_change, mw);
    evas_object_smart_callback_add (mw->win, "focus,out", _focus_change, mw);

    if (!evas_object_key_grab(mw->win, MENU_KEY_PRIMARY1, 0, 0, FALSE))
        EINA_LOG_WARN("Unable to grab primary1 key %s", MENU_KEY_PRIMARY1);

    if (!evas_object_key_grab(mw->win, MENU_KEY_PRIMARY2, 0, 0, FALSE))
        EINA_LOG_WARN("Unable to grab primary2 key %s", MENU_KEY_PRIMARY2);

    if (!evas_object_key_grab(mw->win, MENU_KEY_SECONDARY, 0, 0, FALSE))
        EINA_LOG_WARN("Unable to grab secondary key %s", MENU_KEY_SECONDARY);

    evas_object_event_callback_add(mw->win, EVAS_CALLBACK_KEY_DOWN, _key_down, mw);
    evas_object_event_callback_add(mw->win, EVAS_CALLBACK_KEY_UP, _key_up, mw);

    if (create_bg) {
        /* background */
        mw->bg = elm_bg_add(mw->win);

        evas_object_size_hint_weight_set(mw->bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(mw->bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_win_resize_object_add(mw->win, mw->bg);

        evas_object_show(mw->bg);
    }

    return mw;
}
