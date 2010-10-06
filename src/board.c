/*
 * Mokojeweled
 * Game board
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
#include "menu.h"
#include "board.h"
#include "theme.h"

// board window
static MokoWin* win = NULL;

// gems matrix
static Evas_Object* gems[BOARD_WIDTH][BOARD_HEIGHT] = {};

// currently selected gems
static Evas_Object* selected1 = NULL;
static Evas_Object* selected2 = NULL;

// running animation counter
static uint running = 0;

// swap going back flag
static bool backward = FALSE;

// gem swapping stuff
static Ecore_Animator* swap_anim = NULL;
static int selected1_dest[2] = {-1, -1};
static int selected2_dest[2] = {-1, -1};
static int dx1 = 0, dx2 = 0;
static int dy1 = 0, dy2 = 0;

static void swap(void);
static void destroy_gem(Evas_Object* gem);
static void refill(void);
static void fall_gems(void);
static Evas_Object* make_gem(int col, int row, int x, int y, int index);
static void autoremove_alignments(void);

#define gem_at(x, y)    gems[x][y]

// can't define this as a macro - doesn't work properly
static int sign(int num)
{
    return (num != 0) ? num / abs(num) : 0;
}

/**
 * Returns true if the coordinates are directly adjacent.
 */
static bool is_adjacent(int x1, int y1, int x2, int y2)
{
    return ((abs(x2 - x1) == 1 && y2 == y1) ^ (abs(y2 - y1) == 1 && x2 == x1));
}

/**
 * Marks the selected gems as unselected.
 * @param delete if true, the cached variables are set to NULL
 */
static void unselect(bool delete)
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

/**
 * Iterates the second list to find duplicates in the first list.
 * @return a new list without duplicates
 */
static Eina_List* iterate_duplicate(Eina_List* res, Eina_List* list)
{
    Eina_List *iter;
    void* data;

    EINA_LIST_FOREACH(list, iter, data) {
        if (!eina_list_data_find(res, data))
            res = eina_list_append(res, data);
    }

    return res;
}

/**
 * Merges two list eliminating duplicates.
 * @return a merge list without duplicates
 */
static Eina_List* concat_duplicate(Eina_List* list1, Eina_List* list2)
{
    Eina_List* res = iterate_duplicate(NULL, list1);

    return iterate_duplicate(res, list2);
}

/**
 * Refills the board with missing gems, then triggers gems falling down.
 */
static void refill(void)
{
    int i, j, k;

    for (i = 0; i < BOARD_WIDTH; i++) {
        for (j = 0; j < BOARD_HEIGHT; j++) {
            if (!gems[i][j]) {

                for (k = j; k > 0; k--) {
                    gems[i][k] = gems[i][k - 1];
                    int* coords = (int *) evas_object_data_get(gems[i][k], "coords");
                    coords[1]++;
                }

                if (j && gems[i][1]) {
                    int old_y;
                    evas_object_geometry_get(gems[i][1], NULL, &old_y, NULL, NULL);

                    gems[i][0] = make_gem(i, 0, i * GEM_SIZE, old_y - GEM_SIZE, 0);
                }

                else {
                    gems[i][0] = make_gem(i, 0, i * GEM_SIZE, -GEM_SIZE, 0);
                }
            }
        }
    }

    fall_gems();
}

static bool _refill(void* data)
{
    refill();
    return FALSE;
}

static bool _remove_gems(void* list)
{
    Eina_List *iter;
    void* data;

    EINA_LIST_FOREACH((Eina_List*) list, iter, data) {
        EINA_LOG_DBG("Deleting gem %p", data);
        destroy_gem((Evas_Object*) data);
    }

    eina_list_free((Eina_List *) list);

    running--;

    // riempi i buchi! :)
    ecore_timer_add(0.1, _refill, NULL);

    return FALSE;
}

static Eina_List* _check_align(Evas_Object* g, int dx, int dy)
{
    Eina_List* aligned = NULL;
    int* coords = (int *) evas_object_data_get(g, "coords");
    int index = (int)(evas_object_data_get(g, "index"));

    // subito noi stessi! :)
    //aligned = g_list_append(aligned, g);

    if ((coords[0] + dx) < 0 || (coords[1] + dy) < 0 ||
        (coords[0] + dx) >= BOARD_WIDTH || (coords[1] + dy) >= BOARD_HEIGHT) {

        //g_debug("OUTER LIMITS - exiting");
        return aligned;
    }

    Evas_Object* other = gem_at(coords[0] + dx, coords[1] + dy);
    //g_debug("Align/other[%dx%d]=%p", coords[0] + dx, coords[1] + dy, other);

    if (other) {
        int tid = (int)(evas_object_data_get(other, "index"));
        //g_debug("Align/other.index=%d, this.index=%d", tid, index);

        // aligned!!! prosegui :)
        if (tid == index) {
            aligned = eina_list_merge(
                eina_list_append(aligned, other),
                _check_align(other, dx, dy)
            );
        }
    }

    return aligned;
}

/**
 * Check alignments around the given gem.
 * @param gem
 * @return a list of aligned gems (included the input one)
 */
static Eina_List* check_alignment(Evas_Object* gem)
{
    //int index = GPOINTER_TO_INT(evas_object_data_get(gem, "index"));
    //int* coords = (int *) evas_object_data_get(gem, "coords");

    //g_debug("Checking alignment for gem%02d, coords=%dx%d",
    //    index, coords[0], coords[1]);

    Eina_List* left = _check_align(gem, -1, 0);
    //g_debug("Aligned on the left: %d", g_list_length(left));
    Eina_List* right = _check_align(gem, 1, 0);
    //g_debug("Aligned on the right: %d", g_list_length(right));

    Eina_List* top = _check_align(gem, 0, -1);
    //g_debug("Aligned on top: %d", g_list_length(top));

    Eina_List* bottom = _check_align(gem, 0, 1);
    //g_debug("Aligned on bottom: %d", g_list_length(bottom));

    Eina_List* aligned = NULL;
    if ((eina_list_count(left) + eina_list_count(right) + 1) >= GEMS_MIN_ALIGNED) {
        aligned = eina_list_merge(aligned, eina_list_merge(left, right));
    }

    if ((eina_list_count(top) + eina_list_count(bottom) + 1) >= GEMS_MIN_ALIGNED) {
        aligned = eina_list_merge(aligned, eina_list_merge(top, bottom));
    }

    return eina_list_count(aligned) > 0 ?
        eina_list_append(aligned, gem) :
        NULL;
}

static bool _swap_step(void *data)
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

    // FIXME static coordinates match??
    if (x1 == selected1_dest[0] && y1 == selected1_dest[1] &&
        x2 == selected2_dest[0] && y2 == selected2_dest[1]) {

        int coords_tmp[2];
        int* coords1 = (int *) evas_object_data_get(selected1, "coords");
        int* coords2 = (int *) evas_object_data_get(selected2, "coords");

        // aggiorna le coordinate
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
        Eina_List* align1 = check_alignment(selected1);
        Eina_List* align2 = check_alignment(selected2);

        if (align1 || align2) {
            Eina_List* align = concat_duplicate(align1, align2);
            // running--; running++;
            ecore_timer_add(0.2, _remove_gems, align);
            return FALSE;
        }

        else {
            if (!backward) {
                ecore_animator_del(swap_anim);
                backward = TRUE;
                swap();
                running--;
                return FALSE;
            }
        }

        backward = FALSE;
        selected1 = selected2 = NULL;
        running--;
        return FALSE;
    }

    return TRUE;
}

/**
 * Swaps the selected gems, then check for alignments.
 */
static void swap(void)
{
    // deseleziona solo graficamente
    unselect(FALSE);

    // prepara dati animazione
    int* coords1 = (int *) evas_object_data_get(selected1, "coords");
    int* coords2 = (int *) evas_object_data_get(selected2, "coords");

    int coords1_real[2], coords2_real[2];

    selected1_dest[0] = coords2_real[0] = coords2[0] * GEM_SIZE;
    selected1_dest[1] = coords2_real[1] = coords2[1] * GEM_SIZE;
    selected2_dest[0] = coords1_real[0] = coords1[0] * GEM_SIZE;
    selected2_dest[1] = coords1_real[1] = coords1[1] * GEM_SIZE;

    dx1 = sign(selected1_dest[0] - coords1_real[0]) * GEM_OFFSET;
    dy1 = sign(selected1_dest[1] - coords1_real[1]) * GEM_OFFSET;

    dx2 = sign(selected2_dest[0] - coords2_real[0]) * GEM_OFFSET;
    dy2 = sign(selected2_dest[1] - coords2_real[1]) * GEM_OFFSET;

    // avvia animazione
    running++;
    swap_anim = ecore_animator_add(_swap_step, NULL);
}

/**
 * Destroys a gem.
 */
static void destroy_gem(Evas_Object* gem)
{
    int* coords = (int *) evas_object_data_get(gem, "coords");
    Evas_Object* layout = (Evas_Object*) evas_object_data_get(gem, "layout");
    evas_object_del(layout);

    // aggiorna la matrice delle gemme
    gems[coords[0]][coords[1]] = NULL;

    // libera altri dati dell'oggetto
    free(coords);
}

/**
 * Destroys all gems.
 */
static void destroy_board(void)
{
    int i, j;
    for (i = 0; i < BOARD_WIDTH; i++)
        for (j = 0; j < BOARD_HEIGHT; j++)
            if (gems[i][j]) {
                destroy_gem(gems[i][j]);
                gems[i][j] = NULL;
            }
}

static bool _falldown(void* data)
{
    Evas_Object* gem = data;

    int x, y, dest_y;
    evas_object_geometry_get(gem, &x, &y, NULL, NULL);

    int* coords = (int *) evas_object_data_get(gem, "coords");
    int speed = (int) (evas_object_data_get(gem, "speed"));

    if (!speed)
        speed = (GEM_OFFSET / 5);
    else
        speed += (GEM_OFFSET / 5);

    evas_object_data_set(gem, "speed", (void*)(speed));

    y += speed;
    dest_y = coords[1] * GEM_SIZE;

    if (y <= dest_y) {
        evas_object_move(gem, x, y);
        return TRUE;
    }

    // per sicurezza :D
    evas_object_move(gem, x, dest_y);
    evas_object_data_set(gem, "speed", (void*)(0));

    // ferma tutto!
    running--;

    if (!running) {
        autoremove_alignments();
        // here we go again!!
        selected1 = selected2 = NULL;
    }

    return FALSE;
}

/**
 * Starts gem falling down animation.
 */
static void fall_gems(void)
{
    int i, j;
    for (i = 0; i < BOARD_WIDTH; i++) {
        for (j = 0; j < BOARD_HEIGHT; j++) {
            running++;
            ecore_animator_add(_falldown, gems[i][j]);
        }
    }
}

/**
 * Checks alignments for each gem in the board.
 * @return a list of gems to be removed
 */
static Eina_List* get_alignments(void)
{
    Eina_List* align = NULL;
    int x, y;

    for (x = 0; x < BOARD_WIDTH; x++) {
        for(y = 0; y < BOARD_HEIGHT; y++) {
            Evas_Object* g = gem_at(x, y);
            if (g)
                align = concat_duplicate(align, check_alignment(g));
        }
    }

    return align;
}

/**
 * Checks for any alignments and removes the gem.
 */
static void autoremove_alignments(void)
{
    Eina_List* align = NULL;

    align = get_alignments();

    if (eina_list_count(align) > 0) {
        running++;
        ecore_timer_add(0.2, _remove_gems, align);
    }

    else {
        eina_list_free(align);
        selected1 = selected2 = NULL;
    }
}

/**
 * Check for any alignments and replace the aligned gems with new ones.
 */
static void remove_alignments(void)
{
    Eina_List* align = NULL, *iter;
    bool loop = FALSE;
    void* data;

    align = get_alignments();

    EINA_LIST_FOREACH(align, iter, data) {
        // we are inside, so reiter
        loop = TRUE;

        Evas_Object* gem = data;

        int* coords = evas_object_data_get(gem, "coords");
        int r = coords[0];
        int c = coords[1];

        int x, y;
        evas_object_geometry_get(gem, &x, &y, NULL, NULL);

        EINA_LOG_DBG("Deleting gem %p", gem);
        destroy_gem(gem);

        // rimpiazza subito!
        gems[r][c] = make_gem(r, c, x, y, 0);
    }

    eina_list_free(align);

    if (loop)
        remove_alignments();
}

// a gem has been clicked!
static void _gem_clicked(void *data, Evas_Object* obj, const char* emission, const char* source)
{
    if (running > 0) {
        EINA_LOG_DBG("Animation is running!");
        return;
    }

    if (selected1 && selected2) {
        EINA_LOG_DBG("Gems are already selected!");
        return;
    }

    edje_object_signal_emit(obj, "select", "");

    // ottieni le coordinate della gemma
    int* coords = (int *) evas_object_data_get(obj, "coords");
    EINA_LOG_DBG("Gem %s clicked on %dx%d", source, coords[0], coords[1]);

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
        bool adjacent = is_adjacent(coords1[0], coords1[1],
            coords[0], coords[1]);
        EINA_LOG_DBG("Gems are adjacent: %s", adjacent ? "TRUE" : "FALSE");

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

/**
 * Creates a gem.
 * @param col column coordinate
 * @param row row coordinate
 * @param x x coordinate (pixels)
 * @param y y coordinate (pixels)
 * @param index gem index from theme, 0 for random
 */
static Evas_Object* make_gem(int col, int row, int x, int y, int index)
{
    if (index <= 0) {
        index = (rand() % theme_gem_count()) + 1;
    }

    Evas_Object* layout = elm_layout_add(win->win);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    char* name = strdup_sprintf("gem%02d", index);

    elm_layout_file_set(layout, theme_get_path(), name);
    free(name);

    evas_object_resize(layout, GEM_SIZE, GEM_SIZE);
    evas_object_move(layout, x, y);
    evas_object_show(layout);

    Evas_Object* edj = elm_layout_edje_get(layout);
    EINA_LOG_DBG("Loading gem%02d (%p) with coords %dx%d size %dx%d position %dx%d", index, edj, col, row, GEM_SIZE, GEM_SIZE, x, y);

    int* coords = m_new(int, 2);
    coords[0] = col;
    coords[1] = row;
    evas_object_data_set(edj, "coords", coords);
    evas_object_data_set(edj, "layout", layout);
    evas_object_data_set(edj, "index", (void *) (index));
    evas_object_data_set(edj, "speed", (void *) (0));

    edje_object_signal_callback_add(edj, "clicked", "*", _gem_clicked, NULL);
    // TODO edje_object_signal_callback_add(edj, "up", "*", _gem_clicked, NULL);
    // TODO edje_object_signal_callback_add(edj, "down", "*", _gem_clicked, NULL);

    return edj;
}

static void _close(void *data, Evas_Object* obj, void* event)
{
    if (!running) {
        // TODO pause menu
        destroy_board();
        mokowin_destroy(win);
        win = NULL;
        menu();
    }
}

static void create_win(void)
{
    win = mokowin_new(PACKAGE);
    win->delete_callback = _close;

    elm_win_title_set(win->win, PACKAGE_NAME);
    evas_object_resize(win->win, WIN_WIDTH, WIN_HEIGHT);
}

// here we go!
static bool _start(void* data)
{
    fall_gems();

    running--;
    return FALSE;
}

/**
 * Starts a new game.
 * Abort the previous one if any (without advice!!!).
 * @param type the type of game to begin
 */
void board_new_game(GameType type)
{
    if (!win)
        create_win();

    int i, j;

    // random seed
    srand(time(NULL));

    // destroy previous gems if present
    destroy_board();

    // populate the board
    for (i = 0; i < BOARD_WIDTH; i++)
        for (j = 0; j < BOARD_HEIGHT; j++)
            gems[i][j] = make_gem(i, j, i * GEM_SIZE, (j - BOARD_HEIGHT) * GEM_SIZE, 0);

    // remove alignments
    remove_alignments();

    // let's start!
    running++;
    ecore_timer_add(0.5, _start, NULL);

    mokowin_activate(win);
}
