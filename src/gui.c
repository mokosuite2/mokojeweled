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

/* -- MokoPopupAlert -- */

static void _popup_alert_delete(void *popup)
{
    MokoPopupAlert *p = (MokoPopupAlert*)popup;

    // prima chiama il callback...
    if (p->callback != NULL)
        (p->callback)(p, p->data);
}

static void _popup_alert_ok(void *popup, Evas_Object *obj, void *event_info)
{
    MokoPopupAlert *p = (MokoPopupAlert*)popup;
    // distruggi tutto!
    // distrutto dall'array -- mokoinwin_destroy((MokoInwin*)popup);
    mokowin_inner_remove(p->parent, MOKO_INWIN(p));
}

MokoPopupAlert* moko_popup_alert_new_with_callback(MokoWin *parent, const char *label, void* callback, void* data)
{
    MokoPopupAlert *p = moko_popup_alert_new(parent, label);
    p->callback = callback;
    p->data = data;

    return p;
}

MokoPopupAlert* moko_popup_alert_new(MokoWin *parent, const char *label)
{
    /* costruisci la finestra */
    MokoInwin *inwin = mokoinwin_new(parent);    

    return_val_if_fail(inwin != NULL || label != NULL, NULL);

    MokoPopupAlert *p = realloc(inwin, sizeof(MokoPopupAlert));
    p->win.delete_callback = _popup_alert_delete;
    p->parent = parent;
    p->callback = NULL;
    p->data = NULL;

    elm_object_style_set(MOKO_INWIN(p)->inwin, "minimal_vertical");

    // box principale
    Evas_Object *vbox = elm_box_add(MOKO_INWIN(p)->inwin);
    evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(vbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(vbox);

    // label
    p->label = elm_label_add(MOKO_INWIN(p)->inwin);
    elm_object_text_set(p->label, label);
    elm_label_line_wrap_set(p->label, TRUE);
    //elm_label_wrap_width_set(p->label, 400);

    evas_object_size_hint_weight_set(p->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p->label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(p->label);

    // padding frame
    Evas_Object *pad_frame = elm_frame_add(MOKO_INWIN(p)->inwin);
    evas_object_size_hint_weight_set(pad_frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(pad_frame, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_style_set(pad_frame, "pad_medium");
    elm_object_content_set(pad_frame, p->label);

    evas_object_show(pad_frame);
    elm_box_pack_start(vbox, pad_frame);

    // pulsante OK
    p->ok_button = elm_button_add(MOKO_INWIN(p)->inwin);
    elm_object_text_set(p->ok_button, _("OK"));
    evas_object_smart_callback_add(p->ok_button, "clicked", _popup_alert_ok, p);

    evas_object_size_hint_weight_set(p->ok_button, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(p->ok_button, EVAS_HINT_FILL, 0.0);
    evas_object_show(p->ok_button);

    elm_box_pack_end(vbox, p->ok_button);

    elm_object_content_set(MOKO_INWIN(p)->inwin, vbox);

    // supporto inners di mokowin
    mokowin_inner_add(p->parent, MOKO_INWIN(p));

    mokoinwin_activate(MOKO_INWIN(p));
    return p;
}

/* -- MokoPopupStatus -- */

void moko_popup_status_activate(MokoPopupStatus *popup, const char *status)
{
    if (status != NULL)
        elm_object_text_set(popup->status, status);

    mokoinwin_activate(MOKO_INWIN(popup));
}

MokoPopupStatus* moko_popup_status_new(MokoWin *parent, const char *status)
{
    /* costruisci la finestra */
    MokoInwin *inwin = mokoinwin_new(parent);    

    return_val_if_fail(inwin != NULL, NULL);

    MokoPopupStatus *p = realloc(inwin, sizeof(MokoPopupStatus));
    p->parent = parent;

    elm_object_style_set(MOKO_INWIN(p)->inwin, "minimal");

    // label
    p->status = elm_label_add(MOKO_INWIN(p)->inwin);

    evas_object_size_hint_weight_set(p->status, 1.0, 0.0);
    evas_object_size_hint_align_set(p->status, 0.5, -1.0);
    evas_object_show(p->status);

    elm_object_content_set(MOKO_INWIN(p)->inwin, p->status);

    // FIXME inners con azione
    //mokowin_inner_add(p->parent, MOKO_INWIN(p));

    if (status != NULL)
        moko_popup_status_activate(p, status);

    return p;
}

/* -- MokoPopupMenu -- */

static void _menu_button_click(void *data, Evas_Object *obj, void *event_info)
{
    MokoPopupMenu *p = (MokoPopupMenu*)data;
    int index = (int)evas_object_data_get(obj, "index");

    p->index = index;

    if (p->style == MOKO_POPUP_BUTTONS || p->style == MOKO_POPUP_CHECKS) {

        if (p->callback)
            (p->callback)(p, p->data, p->index, TRUE);

        mokowin_inner_remove(p->parent, MOKO_INWIN(p));

    } else if (p->style == MOKO_POPUP_CHECKS_OK) {

        if (p->callback)
            (p->callback)(p, p->data, p->index, FALSE);
    }
}

static void _menu_ok_click(void *data, Evas_Object *obj, void *event_info)
{
    MokoPopupMenu *p = (MokoPopupMenu*)data;

    if (p->style == MOKO_POPUP_CHECKS_OK) {

        if (p->callback)
            (p->callback)(p, p->data, p->index, TRUE);

        mokowin_inner_remove(p->parent, MOKO_INWIN(p));
    }
}

Evas_Object* moko_popup_menu_add(MokoPopupMenu* popup, const char *label, int index, bool selected)
{
    return_if_fail(popup != NULL && label != NULL);

    Evas_Object *b;

    if (popup->style == MOKO_POPUP_CHECKS_OK || popup->style == MOKO_POPUP_CHECKS) {

        b = elm_radio_add(MOKO_INWIN(popup)->inwin);
        elm_object_text_set(b, label);
        elm_radio_state_value_set(b, index);
        if (popup->last)
            elm_radio_group_add(b, popup->last);

        evas_object_data_set(b, "index", (void*) index);
        evas_object_smart_callback_add(b, "changed", _menu_button_click, popup);

        if (selected) {
            popup->orig_index = popup->index = index;
            elm_radio_value_set(b, index);
        }

    } else {

        b = elm_button_add(MOKO_INWIN(popup)->inwin);
        elm_object_text_set(b, label);

        evas_object_smart_callback_add(b, "clicked", _menu_button_click, popup);
    }

    evas_object_data_set(b, "index", (void*) index);
    evas_object_size_hint_weight_set(b, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(b, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(b);

    elm_box_pack_end(popup->vbox, b);

    popup->last = b;
    return b;
}

MokoPopupMenu* moko_popup_menu_new(MokoWin *parent, const char *message, MokoPopupMenuStyle style, void* callback, void* data)
{
    /* costruisci la finestra */
    MokoInwin *inwin = mokoinwin_new(parent);    

    return_val_if_fail(inwin != NULL, NULL);

    MokoPopupMenu *p = realloc(inwin, sizeof(MokoPopupMenu));
    p->parent = parent;
    p->style = style;
    p->callback = callback;
    p->data = data;
    p->last = NULL;

    elm_object_style_set(MOKO_INWIN(p)->inwin, "minimal_vertical");

    Evas_Object* gbox = elm_box_add(MOKO_INWIN(p)->inwin);
    evas_object_size_hint_weight_set(gbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(gbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(gbox);

    // label
    if (message != NULL) {
        p->label = elm_label_add(MOKO_INWIN(p)->inwin);
        elm_object_text_set(p->label, message);
        elm_label_line_wrap_set(p->label, TRUE);

        evas_object_size_hint_weight_set(p->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(p->label, 0.5, EVAS_HINT_FILL);
        evas_object_show(p->label);

        Evas_Object* frame = elm_frame_add(MOKO_INWIN(p)->inwin);
        evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(frame, 0.5, EVAS_HINT_FILL);
        elm_object_style_set(frame, "pad_medium");
        elm_object_content_set(frame, p->label);
        evas_object_show(frame);

        elm_box_pack_start(gbox, frame);
    }

    // TODO scroller funzionante :|
    Evas_Object* scroll = elm_scroller_add(MOKO_INWIN(p)->inwin);
    elm_scroller_content_min_limit(scroll, TRUE, TRUE);
    elm_scroller_policy_set(scroll, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
    elm_scroller_bounce_set(scroll, FALSE, FALSE);
    evas_object_size_hint_min_set(scroll, 0, 400);

    evas_object_size_hint_weight_set(scroll, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(scroll, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(scroll);

    p->vbox = elm_box_add(MOKO_INWIN(p)->inwin);
    evas_object_size_hint_weight_set(p->vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p->vbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(p->vbox);

    elm_object_content_set(scroll, p->vbox);
    elm_box_pack_end(gbox, scroll);

    if (style == MOKO_POPUP_CHECKS_OK) {
        // aggiungi pulsante ok alla fine
        Evas_Object* btn_ok = elm_button_add(MOKO_INWIN(p)->inwin);
        elm_object_text_set(btn_ok, _("OK"));
        evas_object_size_hint_weight_set(btn_ok, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(btn_ok, EVAS_HINT_FILL, EVAS_HINT_FILL);

        evas_object_smart_callback_add(btn_ok, "clicked", _menu_ok_click, p);

        evas_object_show(btn_ok);

        // TODO pulsante cancel?
        p->bottom = btn_ok;
        elm_box_pack_end(gbox, p->bottom);
    }

    // supporto inners di mokowin
    mokowin_inner_add(p->parent, MOKO_INWIN(p));

    elm_object_content_set(MOKO_INWIN(p)->inwin, gbox);

    return p;
}

/* -- MokoPopupSlider -- */

static void _pslider_button_ok(void* data, Evas_Object* obj, void* event_info)
{
    MokoPopupSlider *p = (MokoPopupSlider *)data;
    p->win.delete_callback = NULL;

    if (p->value == p->orig_value)
        p->changed = FALSE;

    if (p->callback)
        (p->callback)(data, p->data, p->value, TRUE);

    mokowin_inner_remove(p->parent, MOKO_INWIN(p));
}

static void _pslider_button_cancel(void* data, Evas_Object* obj, void* event_info)
{
    MokoPopupSlider *p = (MokoPopupSlider *)data;
    p->win.delete_callback = NULL;

    p->value = p->orig_value;

    if (p->callback)
        (p->callback)(data, p->data, p->value, TRUE);

    if (obj != NULL)
        mokowin_inner_remove(p->parent, MOKO_INWIN(p));
}

static void _pslider_close(void* inwin)
{
    _pslider_button_cancel(inwin, NULL, NULL);
}

static void _pslider_change(void* data, Evas_Object* obj, void* event_info)
{
    MokoPopupSlider *p = (MokoPopupSlider *)data;
    p->value = (int)elm_slider_value_get(p->slider);
    p->changed = TRUE;

    EINA_LOG_DBG("Slider changed: %d", p->value);

    if (p->callback)
        (p->callback)(data, p->data, p->value, FALSE);
}

MokoPopupSlider* moko_popup_slider_new(MokoWin *parent, const char *message, int value, void* callback, void* data)
{
    /* costruisci la finestra */
    MokoInwin *inwin = mokoinwin_new(parent);    

    return_val_if_fail(inwin != NULL, NULL);

    MokoPopupSlider *p = realloc(inwin, sizeof(MokoPopupSlider));
    p->parent = parent;
    p->callback = callback;
    p->data = data;
    p->value = p->orig_value = value;
    p->changed = FALSE;
    p->win.delete_callback = _pslider_close;

    elm_object_style_set(MOKO_INWIN(p)->inwin, "minimal_vertical");

    p->vbox = elm_box_add(MOKO_INWIN(p)->inwin);
    evas_object_size_hint_weight_set(p->vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p->vbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(p->vbox);

    Evas_Object* frame_vbox = elm_box_add(MOKO_INWIN(p)->inwin);
    evas_object_size_hint_weight_set(frame_vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(frame_vbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(frame_vbox);

    // frame per padding di label e slider
    Evas_Object* frame = elm_frame_add(MOKO_INWIN(p)->inwin);
    evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_style_set(frame, "pad_medium");
    elm_object_content_set(frame, frame_vbox);
    evas_object_show(frame);

    elm_box_pack_start(p->vbox, frame);

    // label
    p->label = elm_label_add(MOKO_INWIN(p)->inwin);
    elm_object_text_set(p->label, message);
    elm_label_line_wrap_set(p->label, TRUE);

    evas_object_size_hint_weight_set(p->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p->label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(p->label);

    elm_box_pack_start(frame_vbox, p->label);

    // slider :)
    p->slider = elm_slider_add(MOKO_INWIN(p)->inwin);
    evas_object_size_hint_weight_set(p->slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p->slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(p->slider);

    elm_slider_min_max_set(p->slider, 0.0, 100.0);
    elm_slider_value_set(p->slider, (double)p->value);
    evas_object_smart_callback_add(p->slider, "delay,changed", _pslider_change, p);

    elm_box_pack_end(frame_vbox, p->slider);

    // hbox per i due pulsanti OK/Annulla
    Evas_Object* hbox = elm_box_add(MOKO_INWIN(p)->inwin);
    elm_box_horizontal_set(hbox, TRUE);
    evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(hbox);

    // pulsante OK
    Evas_Object *b = elm_button_add(MOKO_INWIN(p)->inwin);
    elm_object_text_set(b, _("OK"));
    evas_object_size_hint_weight_set(b, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(b, EVAS_HINT_FILL, 0.0);

    evas_object_smart_callback_add(b, "clicked", _pslider_button_ok, p);

    evas_object_show(b);
    elm_box_pack_end(hbox, b);

    // pulsante Annulla
    b = elm_button_add(MOKO_INWIN(p)->inwin);
    elm_object_text_set(b, _("Cancel"));
    evas_object_size_hint_weight_set(b, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(b, EVAS_HINT_FILL, 0.0);

    evas_object_smart_callback_add(b, "clicked", _pslider_button_cancel, p);

    evas_object_show(b);
    elm_box_pack_end(hbox, b);

    // aggiungi hbox pulsanti alla vbox principale
    elm_box_pack_end(p->vbox, hbox);

    // supporto inners di mokowin
    mokowin_inner_add(p->parent, MOKO_INWIN(p));

    elm_object_content_set(MOKO_INWIN(p)->inwin, p->vbox);

    return p;
}
