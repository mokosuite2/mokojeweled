#include <glib.h>
#include <Elementary.h>

static Evas_Object* win = NULL;

static void _gem_clicked(void *data, Evas_Object* obj, const char* emission, const char* source)
{
    g_debug("Gem clicked! emission=%s, source=%s", emission, source);
}

void test_mainwindow()
{
    win = elm_win_add(NULL, "mokojeweled", ELM_WIN_BASIC);
    elm_win_autodel_set(win, TRUE);

    Evas_Object* bg = elm_bg_add(win);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_win_resize_object_add(win, bg);
    evas_object_show(bg);

    Evas_Object* layout = elm_layout_add(win);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_layout_file_set(layout, "test.edj", "main");
    elm_win_resize_object_add(win, layout);
    evas_object_show(layout);

    Evas_Object* edj = elm_layout_edje_get(layout);
    edje_object_signal_callback_add(edj, "clicked", "table:*", _gem_clicked, NULL);

    evas_object_resize(win, 400, 400);
    evas_object_show(win);
}
