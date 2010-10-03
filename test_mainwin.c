#include <glib.h>
#include <Elementary.h>

static Evas_Object* win = NULL;

void test_mainwindow()
{
    win = elm_win_add(NULL, "mokojeweled", ELM_WIN_BASIC);
    elm_win_autodel_set(win, TRUE);

    Evas_Object* bg = elm_bg_add(win);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_win_resize_object_add(win, bg);
    evas_object_show(bg);

    evas_object_resize(win, 400, 400);
    evas_object_show(win);
}
