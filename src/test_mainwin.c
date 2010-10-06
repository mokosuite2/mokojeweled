#include <glib.h>
#include <Elementary.h>

static Evas_Object* win = NULL;

#define WIDTH       480
#define HEIGHT      600
#define GEM_SIZE    80

#define ROWS        (HEIGHT/GEM_SIZE)
#define COLS        (WIDTH/GEM_SIZE)

// parametri animazioni
#define FRAMETIME   0.02 //0.01
#define OFFSET      10  //20

// numero di gemme allineate minimo
#define MIN_ALIGNED     3

// tabella gemme
static Evas_Object* gems[COLS][ROWS] = {};

// gemme selezionate
static Evas_Object* selected1 = NULL;
static Evas_Object* selected2 = NULL;

// dati animazioni
static Ecore_Animator* swap_anim = NULL;
static int selected1_dest[2] = {-1, -1};
static int selected2_dest[2] = {-1, -1};
static int dx1 = 0, dx2 = 0;
static int dy1 = 0, dy2 = 0;

// flag movimento ripristinatorio
static gboolean backward = FALSE;
static guint running = 0;

static void swap(void);
static void autoremove_alignments(void);
static Evas_Object* make_gem(int col, int row, int x, int y, int index, Eina_Bool autocalculate);

static Evas_Object* gem_at(int x, int y)
{
    return gems[x][y];
}

/**
 * Determina se i punti passati sono adiacenti.
 */
static gboolean is_adjacent(int x1, int y1, int x2, int y2)
{
    //g_debug("P1=(%d,%d), P2=(%d,%d), dX=%d, dy=%d", x1, y1, x2, y2, x2-x1, y2-y1);
    return ((abs(x2 - x1) == 1 && y2 == y1) ^ (abs(y2 - y1) == 1 && x2 == x1));
}

/**
 * Deseleziona le gemme selezionate.
 */
static void unselect(gboolean delete)
{
    if (selected1) {
        edje_object_signal_emit(selected1, "unselect", "");
        if (delete) selected1 = NULL;
    }

    if (selected2) {
        edje_object_signal_emit(selected2, "unselect", "");
        if (delete) selected2 = NULL;
    }
}

static GList* _check_align(Evas_Object* g, int dx, int dy)
{
    GList* aligned = NULL;
    int* coords = (int *) evas_object_data_get(g, "coords");
    int index = GPOINTER_TO_INT(evas_object_data_get(g, "index"));

    // subito noi stessi! :)
    //aligned = g_list_append(aligned, g);

    if ((coords[0] + dx) < 0 || (coords[1] + dy) < 0 ||
        (coords[0] + dx) >= COLS || (coords[1] + dy) >= ROWS) {

        //g_debug("OUTER LIMITS - exiting");
        return aligned;
    }

    Evas_Object* other = gem_at(coords[0] + dx, coords[1] + dy);
    //g_debug("Align/other[%dx%d]=%p", coords[0] + dx, coords[1] + dy, other);

    if (other) {
        int tid = GPOINTER_TO_INT(evas_object_data_get(other, "index"));
        //g_debug("Align/other.index=%d, this.index=%d", tid, index);

        // aligned!!! prosegui :)
        if (tid == index) {
            aligned = g_list_concat(
                g_list_append(aligned, other),
                _check_align(other, dx, dy)
            );
        }
    }

    return aligned;
}

/**
 * Distrugge una gemma.
 */
static void destroy_gem(Evas_Object* gem)
{
    int* coords = (int *) evas_object_data_get(gem, "coords");
    Evas_Object* layout = (Evas_Object*) evas_object_data_get(gem, "layout");
    evas_object_del(layout);

    // aggiorna la matrice delle gemme
    gems[coords[0]][coords[1]] = NULL;

    // libera altri dati dell'oggetto
    g_free(coords);
}

/**
 * Verifica gli allineamenti della gemma con le sue adiacenti
 * a gruppi di 3 o piu'.
 * @param gem la gemma in questione
 * @return la lista delle gemme allineate (da eliminare dal tavolo)
 */
static GList* check_alignment(Evas_Object* gem)
{
    //int index = GPOINTER_TO_INT(evas_object_data_get(gem, "index"));
    //int* coords = (int *) evas_object_data_get(gem, "coords");

    //g_debug("Checking alignment for gem%02d, coords=%dx%d",
    //    index, coords[0], coords[1]);

    GList* left = _check_align(gem, -1, 0);
    //g_debug("Aligned on the left: %d", g_list_length(left));
    GList* right = _check_align(gem, 1, 0);
    //g_debug("Aligned on the right: %d", g_list_length(right));

    GList* top = _check_align(gem, 0, -1);
    //g_debug("Aligned on top: %d", g_list_length(top));

    GList* bottom = _check_align(gem, 0, 1);
    //g_debug("Aligned on bottom: %d", g_list_length(bottom));

    GList* aligned = NULL;
    if ((g_list_length(left) + g_list_length(right) + 1) >= MIN_ALIGNED) {
        aligned = g_list_concat(aligned, g_list_concat(left, right));
    }

    if ((g_list_length(top) + g_list_length(bottom) + 1) >= MIN_ALIGNED) {
        aligned = g_list_concat(aligned, g_list_concat(top, bottom));
    }

    return g_list_length(aligned) > 0 ?
        g_list_concat(g_list_append(NULL, gem), aligned) :
        NULL;
}

static GList* iterate_duplicate(GList* res, GList* list)
{
    GList* iter = list;
    while (iter) {
        if (!g_list_find(res, iter->data)) {
            res = g_list_append(res, iter->data);
        }

        iter = iter->next;
    }

    return res;
}

static GList* concat_duplicate(GList* list1, GList* list2)
{
    GList* res = iterate_duplicate(NULL, list1);

    return iterate_duplicate(res, list2);
}

static Eina_Bool _falldown(void* data)
{
    Evas_Object* gem = data;

    int x, y, dest_y;
    evas_object_geometry_get(gem, &x, &y, NULL, NULL);

    int* coords = (int *) evas_object_data_get(gem, "coords");
    int speed = GPOINTER_TO_INT(evas_object_data_get(gem, "speed"));

    if (!speed)
        speed = (OFFSET/5);
    else
        speed += (OFFSET/5);

    evas_object_data_set(gem, "speed", GINT_TO_POINTER(speed));

    y += speed;
    dest_y = coords[1] * GEM_SIZE;

    if (y <= dest_y) {
        evas_object_move(gem, x, y);
        return TRUE;
    }

    // per sicurezza :D
    evas_object_move(gem, x, dest_y);
    evas_object_data_set(gem, "speed", GINT_TO_POINTER(0));

    // ferma tutto!
    running--;

    if (!running) {
        autoremove_alignments();
        // EVVAI :D
        selected1 = selected2 = NULL;
    }

    return FALSE;
}

static void fall_gems(void)
{
    int i, j;
    for (i = 0; i < COLS; i++) {
        for (j = 0; j < ROWS; j++) {
            running++;
            ecore_animator_add(_falldown, gems[i][j]);
        }
    }
}

/**
 * Cerca i vuoti e comincia le animazioni per riempirli
 */
static void refill(void)
{
    int i, j, k;

    for (i = 0; i < COLS; i++) {
        for (j = 0; j < ROWS; j++) {
            if (!gems[i][j]) {

                for (k = j; k > 0; k--) {
                    gems[i][k] = gems[i][k - 1];
                    int* coords = (int *) evas_object_data_get(gems[i][k], "coords");
                    coords[1]++;
                }

                if (j && gems[i][1]) {
                    int old_y;
                    evas_object_geometry_get(gems[i][1], NULL, &old_y, NULL, NULL);

                    gems[i][0] = make_gem(i, 0, i * GEM_SIZE, old_y - GEM_SIZE, 0, FALSE);
                }

                else {
                    gems[i][0] = make_gem(i, 0, i * GEM_SIZE, -GEM_SIZE, 0, FALSE);
                }
            }
        }
    }

    fall_gems();
}

static gboolean _refill(gpointer data)
{
    refill();
    return FALSE;
}

static gboolean _remove_gems(void* data)
{
    GList* iter = data;
    while (iter) {
        g_debug("[%s] Deleting gem %p", __func__, iter->data);
        destroy_gem((Evas_Object*) iter->data);
        iter = iter->next;
    }
    g_list_free(data);

    running--;

    // riempi i buchi! :)
    //refill();
    g_timeout_add(100, _refill, NULL);

    return FALSE;
}

static Eina_Bool _swap_step(void *data)
{
    int x1, y1, x2, y2;

    evas_object_geometry_get(selected1, &x1, &y1, NULL, NULL);
    x1 += dx1;
    y1 += dy1;
    evas_object_move(selected1, x1, y1);

    evas_object_geometry_get(selected2, &x2, &y2, NULL, NULL);
    x2 += dx2;
    y2 += dy2;
    evas_object_move(selected2, x2, y2);

    //g_debug("MOVING! d1=(%d, %d), d2=(%d, %d)", dx1, dy1, dx2, dy2);

    // FIXME per ora verifica statica
    if (x1 == selected1_dest[0] && y1 == selected1_dest[1] &&
        x2 == selected2_dest[0] && y2 == selected2_dest[1]) {

        int size, coords_tmp[2];
        int* coords1 = (int *) evas_object_data_get(selected1, "coords");
        int* coords2 = (int *) evas_object_data_get(selected2, "coords");

        // aggiorna le coordinate
        size = GEM_SIZE;
        memcpy(coords_tmp, coords1, sizeof(int) * 2);

        // imposta le nuove coordinate
        memcpy(coords1, coords2, sizeof(int) * 2);
        memcpy(coords2, coords_tmp, sizeof(int) * 2);

        // aggiorna la matrice
        gems[coords1[0]][coords1[1]] = selected1;
        gems[coords2[0]][coords2[1]] = selected2;

        // FIXME per ora verifica statica
        //evas_object_move(selected1, coords1[0] * size, coords1[1] * size);
        //evas_object_move(selected2, coords2[0] * size, coords2[1] * size);

        // verifica allineamento
        GList* align1 = check_alignment(selected1);
        GList* align2 = check_alignment(selected2);

        if (align1 || align2) {
            GList* align = concat_duplicate(align1, align2);
            running++;
            g_timeout_add(200, _remove_gems, align);
            return FALSE;
        }

        else {
            if (!backward) {
                ecore_animator_del(swap_anim);
                backward = TRUE;
                swap();
                return FALSE;
            }
        }

        backward = FALSE;
        selected1 = selected2 = NULL;
        return FALSE;
    }

    return TRUE;
}

static int sign(int num)
{
    return (num != 0) ? num / abs(num) : 0;
}

/**
 * Scambia le gemme selezionate di posto.
 */
static void swap(void)
{
    // deseleziona solo graficamente
    unselect(FALSE);

    // prepara dati animazione
    int size = GEM_SIZE;
    int* coords1 = (int *) evas_object_data_get(selected1, "coords");
    int* coords2 = (int *) evas_object_data_get(selected2, "coords");

    int coords1_real[2], coords2_real[2];

    selected1_dest[0] = coords2_real[0] = coords2[0] * size;
    selected1_dest[1] = coords2_real[1] = coords2[1] * size;
    selected2_dest[0] = coords1_real[0] = coords1[0] * size;
    selected2_dest[1] = coords1_real[1] = coords1[1] * size;

    dx1 = sign(selected1_dest[0] - coords1_real[0]) * OFFSET;
    dy1 = sign(selected1_dest[1] - coords1_real[1]) * OFFSET;

    dx2 = sign(selected2_dest[0] - coords2_real[0]) * OFFSET;
    dy2 = sign(selected2_dest[1] - coords2_real[1]) * OFFSET;

    // avvia animazione
    swap_anim = ecore_animator_add(_swap_step, NULL);
}

static void _gem_clicked(void *data, Evas_Object* obj, const char* emission, const char* source)
{
    if (running > 0) {
        g_debug("Animation is running!");
        return;
    }

    if (selected1 && selected2) {
        g_debug("Gems are already selected!");
        return;
    }

    edje_object_signal_emit(obj, "select", "");

    // ottieni le coordinate della gemma
    int* coords = (int *) evas_object_data_get(obj, "coords");
    g_debug("Gem %s clicked on %dx%d", source, coords[0], coords[1]);

    if (!selected1) {
        selected1 = obj;
    }

    else if (!selected2) {
        // stessa gemma, deseleziona
        if (selected1 == obj) {
            unselect(TRUE);
            return;
        }

        // verifica fattibilita' selezione
        int* coords1 = (int *) evas_object_data_get(selected1, "coords");
        gboolean adjacent = is_adjacent(coords1[0], coords1[1],
            coords[0], coords[1]);
        g_debug("Gems are adjacent: %s", adjacent ? "TRUE" : "FALSE");

        selected2 = obj;

        if (adjacent) {
            // processa selezione
            swap();
        }

        else {
            unselect(TRUE);
            _gem_clicked(NULL, obj, emission, source);
        }
    }
}

static void _close(void *data, Evas_Object* obj, void* event)
{
    elm_exit();
}

static Evas_Object* make_gem(int col, int row, int x, int y, int index, Eina_Bool autocalculate)
{
    char buf[PATH_MAX];
    if (index <= 0)
        index = g_random_int_range(1, 8);

    Evas_Object* layout = elm_layout_add(win);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    char* name = g_strdup_printf("gem%02d", index);
    snprintf(buf, sizeof(buf), "%s/themes/default.edj", PKGDATADIR);
    elm_layout_file_set(layout, buf, name);
    g_free(name);

    int size = GEM_SIZE;
    x = (x < 0 && autocalculate) ? col*size : x;
    y = (y < 0 && autocalculate) ? row*size : y;

    evas_object_resize(layout, size, size);
    evas_object_move(layout, x, y);
    evas_object_show(layout);

    Evas_Object* edj = elm_layout_edje_get(layout);
    g_debug("Loading gem%02d (%p) with coords %dx%d size %dx%d position %dx%d", index, edj, col, row, size, size, x, y);

    int* coords = g_new(int, 2);
    coords[0] = col;
    coords[1] = row;
    evas_object_data_set(edj, "coords", coords);
    evas_object_data_set(edj, "layout", layout);
    evas_object_data_set(edj, "index", GINT_TO_POINTER(index));
    evas_object_data_set(edj, "speed", GINT_TO_POINTER(0));

    edje_object_signal_callback_add(edj, "clicked", "*", _gem_clicked, NULL);
    //edje_object_signal_callback_add(edj, "up", "*", _gem_clicked, NULL);
    //edje_object_signal_callback_add(edj, "down", "*", _gem_clicked, NULL);

    return edj;
}

static GList* get_alignments(void)
{
    GList* align = NULL;
    int x, y;

    for (x = 0; x < COLS; x++) {
        for(y = 0; y < ROWS; y++) {
            Evas_Object* g = gem_at(x, y);
            if (g)
                align = concat_duplicate(align, check_alignment(g));
        }
    }

    return align;
}

static void autoremove_alignments(void)
{
    GList* align = NULL;

    align = get_alignments();

    if (g_list_length(align) > 0) {
        running++;
        g_timeout_add(200, _remove_gems, align);
    }

    else {
        g_list_free(align);
        selected1 = selected2 = NULL;
    }
}

static void remove_alignments(void)
{
    GList* align = NULL, *iter;
    gboolean loop = FALSE;

    align = get_alignments();
    iter = align;

    while (iter) {
        // se siamo entrati qui vuol dire che dobbiamo reiterare :(
        loop = TRUE;
        Evas_Object* gem = iter->data;

        int* coords = evas_object_data_get(gem, "coords");
        int r = coords[0];
        int c = coords[1];

        int x, y;
        evas_object_geometry_get(gem, &x, &y, NULL, NULL);

        g_debug("[%s] Deleting gem %p", __func__, gem);
        destroy_gem(gem);

        // rimpiazza subito!
        gems[r][c] = make_gem(r, c, x, y, 0, FALSE);

        iter = iter->next;
    }

    g_list_free(align);

    if (loop)
        remove_alignments();
}

static void dump_table(int sig_num)
{
    g_return_if_fail(gems != NULL);

    int x, y, mismatch = 0, empty = 0;
    for (x = 0; x < COLS; x++) {
        for(y = 0; y < ROWS; y++) {
            Evas_Object* gem = gem_at(x, y);
            if (gem) {
                int* coords = (int*) evas_object_data_get(gem, "coords");
                printf("[%dx%d, %dx%d]", x, y, coords[0], coords[1]);
                if (coords[0] != x || coords[1] != y)
                    mismatch++;
            }
            else {
                printf("[%dx%d, NxN]", x, y);
                empty++;
            }
        }
        printf("\n");
    }
    if (mismatch > 0 || empty > 0)
        g_warning("mismatch=%d, empty=%d", mismatch, empty);
}

void test_mainwindow()
{
    win = elm_win_add(NULL, "mokojeweled", ELM_WIN_BASIC);
    elm_win_title_set(win, "Mokojeweled");
    evas_object_smart_callback_add(win, "delete,request", _close, NULL);

    ecore_animator_frametime_set(FRAMETIME);

    Evas_Object* bg = elm_bg_add(win);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_win_resize_object_add(win, bg);
    evas_object_show(bg);

    #if 0
    int x, y;
    for (x = 0; x < COLS; x++) {
        for(y = 0; y < ROWS; y++) {
            gems[x][y] = make_gem(x, y, -1, -1, 0);
        }
    }
    #endif

    int i, j;
    for (i = 0; i < COLS; i++)
        for (j = 0; j < ROWS; j++)
            gems[i][j] = make_gem(i, j, i * GEM_SIZE, (j - ROWS) * GEM_SIZE, 0, FALSE);

    remove_alignments();
    fall_gems();

    evas_object_resize(win, WIDTH, HEIGHT);
    evas_object_show(win);

    // segnaletica ;)
    struct sigaction usr1 = {};
    usr1.sa_handler = dump_table;
    usr1.sa_flags = 0;
    sigaction(SIGUSR1, &usr1, NULL);
}
