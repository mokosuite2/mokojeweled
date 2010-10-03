#include <glib.h>
#include <Elementary.h>

void test_mainwindow();

int main(int argc, char* argv[])
{
    g_debug("%s version %s", PACKAGE, VERSION);

    g_type_init();
    g_set_prgname(PACKAGE);

    elm_init(argc, argv);

    /* integrazione con GLib */
    if (!ecore_main_loop_glib_integrate())
        g_error("Ecore/GLib integration failed!");

    test_mainwindow();

    elm_run();
    elm_shutdown();

    return EXIT_SUCCESS;
}
