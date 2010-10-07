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

#ifndef __GUI_H
#define __GUI_H

#include "globals.h"

#define MENU_KEY_PRIMARY1   "XF86PowerOff"
#define MENU_KEY_PRIMARY2   "Keycode-219"
#define MENU_KEY_SECONDARY  "F5"

struct _MokoWin {
    Evas_Object *win;
    Evas_Object *bg;

    /* layout con vbox */
    Evas_Object *vbox;
    Evas_Object *scroller;
    Evas_Object *scroller_vbox;

    /* layout con Edje */
    Evas_Object *layout;
    Evas_Object *layout_edje;

    /* menu hover */
    Evas_Object *menu_hover;
    Evas_Object *current_menu;
    unsigned int last_keystamp;
    bool key_down;
    bool menu_enable;

    /* supporto per MokoInwin */
    Eina_List *inners;

    Evas_Smart_Cb delete_callback;
    Evas_Smart_Cb focus_callback;

    /* dati utente */
    void* data;
};

typedef struct _MokoWin MokoWin;


struct _MokoInwin {
    Evas_Object *inwin;

    void (*delete_callback)(void* inwin);
};

typedef struct _MokoInwin MokoInwin;

/* -- MokoWin -- */

#define MOKO_WIN(x)     ((MokoWin*)x)

void mokowin_menu_hide(MokoWin *mw);
void mokowin_menu_show(MokoWin *mw);

void mokowin_create_layout(MokoWin *mw, const char *file, const char *group);
void mokowin_create_vbox(MokoWin *mw, bool scroller);

void mokowin_menu_enable(MokoWin *mw);
void mokowin_menu_set(MokoWin *wm, Evas_Object *box);

void mokowin_inner_add(MokoWin *mw, MokoInwin *inwin);
void mokowin_inner_remove(MokoWin *mw, MokoInwin *inwin);

struct _ScrollCallbackData
{
    MokoWin* win;
    bool freeze;
};

typedef struct _ScrollCallbackData ScrollCallbackData;

void mokowin_delete_data_set(MokoWin *mw, void* data);

Evas_Object* mokowin_menu_hover_button(MokoWin* mw, Evas_Object* table, const char* label, int x, int y, int w, int h);
Evas_Object* mokowin_vbox_button(MokoWin* mw, const char* label, Evas_Object* after, Evas_Object* before);
Evas_Object* mokowin_vbox_button_with_callback(MokoWin* mw, const char* label,
    Evas_Object* after, Evas_Object* before, Evas_Smart_Cb callback, void* data);

void mokowin_scroll_freeze_set_callback(ScrollCallbackData *data);
void mokowin_scroll_freeze_set(MokoWin* mw, bool state);

void mokowin_activate(MokoWin *mw);
void mokowin_hide(MokoWin *mw);

void mokowin_destroy(MokoWin *mw);

MokoWin* mokowin_new(const char *name, bool create_bg);
MokoWin* mokowin_sized_new(const char *name, size_t size, bool create_bg);
MokoWin* mokowin_new_with_type(const char *name, Elm_Win_Type type, bool create_bg);
MokoWin* mokowin_sized_new_with_type(const char *name, size_t size, Elm_Win_Type type, bool create_bg);

/* -- MokoInwin -- */

#define MOKO_INWIN(x)     ((MokoInwin*)x)

void mokoinwin_activate(MokoInwin *mw);
void mokoinwin_hide(MokoInwin *mw);

void mokoinwin_destroy(MokoInwin *mw);
MokoInwin *mokoinwin_new(MokoWin *parent);

/* -- MokoPopupAlert -- */

struct _MokoPopupAlert {
    MokoInwin win;      // per il casting a MokoInwin
    MokoWin* parent;

    Evas_Object* ok_button;
    Evas_Object* label;

    void (*callback)(void* popup, void* data);
    void* data;
};

typedef struct _MokoPopupAlert MokoPopupAlert;

MokoPopupAlert* moko_popup_alert_new_with_callback(MokoWin *parent, const char *label, void* callback, void* data);
MokoPopupAlert* moko_popup_alert_new(MokoWin *parent, const char *label);


/* -- MokoPopupStatus -- */

struct _MokoPopupStatus {
    MokoInwin win;      // per il casting a MokoInwin
    MokoWin* parent;
    Evas_Object* status;
};

typedef struct _MokoPopupStatus MokoPopupStatus;

void moko_popup_status_activate(MokoPopupStatus *popup, const char *status);

MokoPopupStatus* moko_popup_status_new(MokoWin *parent, const char *status);

/* -- MokoPopupMenu -- */

typedef enum {

    MOKO_POPUP_BUTTONS = 0,
    MOKO_POPUP_CHECKS,
    MOKO_POPUP_CHECKS_OK

} MokoPopupMenuStyle;

struct _MokoPopupMenu {
    MokoInwin win;      // per il casting a MokoInwin
    MokoWin* parent;

    MokoPopupMenuStyle style;

    Evas_Object* label;
    Evas_Object* vbox;

    /* ultimo pulsante/check aggiunto */
    Evas_Object* last;

    /* usato per i pulsanti di fondo */
    Evas_Object* bottom;

    /* indice dell'elemento selezionato */
    int index;

    /* indice dell'elemento selezionato originale */
    int orig_index;

    void (*callback)(void* popup, void* data, int index, bool final);
    void* data;
};

typedef struct _MokoPopupMenu MokoPopupMenu;

Evas_Object* moko_popup_menu_add(MokoPopupMenu* popup, const char *label, int index, bool selected);

MokoPopupMenu* moko_popup_menu_new(MokoWin *parent, const char *message, MokoPopupMenuStyle style, void* callback, void* data);

/* -- MokoPopupSlider -- */

struct _MokoPopupSlider {
    MokoInwin win;      // per il casting a MokoInwin
    MokoWin* parent;

    Evas_Object* label;
    Evas_Object* vbox;
    Evas_Object* slider;

    /* flag slider modificato */
    bool changed;

    /* valore scelto dallo slider */
    int value;

    /* valore iniziale dello slider */
    int orig_value;

    void (*callback)(void* popup, void* data, int value, bool final);
    void* data;
};

typedef struct _MokoPopupSlider MokoPopupSlider;

MokoPopupSlider* moko_popup_slider_new(MokoWin *parent, const char *message, int value, void* callback, void* data);


/* -- MokoList -- TODO */

struct _MokoList {
};

typedef struct _MokoList MokoList;

MokoList* moko_list_new(Evas_Object* parent);

#endif  /* __GUI_H */
