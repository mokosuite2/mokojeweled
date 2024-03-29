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
static int gems_index[BOARD_WIDTH][BOARD_HEIGHT] = {};

// currently selected gems
static Evas_Object* selected1 = NULL;
static Evas_Object* selected2 = NULL;

// running animation counter
static uint running = 0;

// swap going back flag
static bool backward = FALSE;

// gem swapping stuff (FIXME this can be done in better ways)
static Ecore_Animator* swap_anim = NULL;
static int selected1_dest[2] = {-1, -1};
static int selected2_dest[2] = {-1, -1};
static int dx1 = 0, dx2 = 0;
static int dy1 = 0, dy2 = 0;

// running game data
static int current_level = 0;
static int game_type = -1;
static Ecore_Timer* timed_game_timer = NULL;

// scores
// TODO not used yet -- static int score_move = 0;      // move score buffer
static int score_level = 0;     // level score
static int score_total = 0;     // overall (game) score
static int score_bar = 0;       // progress bar score (mostly used for timed mode)

static void swap(void);
static void refill(void);
static void fall_gems(void);
static void autoremove_alignments(void);
static void destroy_board(void);
static void board_reset(void);

static void destroy_gem(Evas_Object* gem);
static Evas_Object* put_gem(int col, int row, int x, int y, int index);
static Evas_Object* make_gem(int col, int row, int x, int y, int index);

#define gem_at(x, y)        gems[x][y]
#define gem_index(x, y)     gems_index[x][y]


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

static int game_threshold(void)
{
    switch (game_type) {
        case GAME_TYPE_TIMED:
            return LEVEL_SCORE;

        case GAME_TYPE_NORMAL:
        default:
            return current_level * LEVEL_SCORE;
    }
}

/**
 * Updates the score progress bar.
 */
static void update_score_bar(void)
{
    // score_bar : (current_level * base_score) = x : 1.0
    // x = score_bar / (current_level * base_score)
    double val = (double) score_bar / game_threshold();

    // send message to progress bar
    Edje_Message_Float* px = m_new0(Edje_Message_Float, 1);
    px->val = val;
    if (px->val > 1.0) px->val = 1.0;
    else if (px->val < 0.0) px->val = 0.0;

    EINA_LOG_DBG("Updating score bar to %f (current_level = %d, score_level = %d)",
        px->val, current_level, score_level);

    edje_object_message_send(win->layout_edje, EDJE_MESSAGE_FLOAT, 0, px);
    free(px);
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

static bool _decrease_bar(void* data)
{
    score_bar -= GEM_POINTS;

    // FIXME ehm :)
    if (score_bar <= 0) {
        _close(NULL, NULL, NULL);
    }

    else {
        update_score_bar();
    }

    return TRUE;
}


/**
 * Check if level has been completed, if so step to the next one.
 */
static void check_nextlevel(void)
{
    // TODO check for timed game :)

    if (score_bar >= game_threshold()) {
        board_next_level();
    }
}

static bool _check_next_level(void* data)
{
    check_nextlevel();

    running--;
    return FALSE;
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
                    gems_index[i][k] = gems_index[i][k - 1];
                    int* coords = (int *) evas_object_data_get(gems[i][k], "coords");
                    coords[1]++;
                }

                if (j && gems[i][1]) {
                    int old_y;
                    evas_object_geometry_get(gems[i][1], NULL, &old_y, NULL, NULL);

                    put_gem(i, 0, i * GEM_SIZE, old_y - GEM_SIZE, -1);
                }

                else {
                    put_gem(i, 0, i * GEM_SIZE, -GEM_SIZE, -1);
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
        score_level += GEM_POINTS;
        score_total += GEM_POINTS;
        score_bar += GEM_POINTS;
    }

    eina_list_free((Eina_List *) list);

    EINA_LOG_DBG("Level score = %d, total score = %d", score_level, score_total);
    update_score_bar();

    running--;

    // riempi i buchi! :)
    ecore_timer_add(0.1, _refill, NULL);

    return FALSE;
}

/**
 * Check if a gem is part of an alignment. 
 * Thanks to gweled
 */
static int is_part_of_alignment(int x, int y)
{
    int i, result;
    int n0, n1, n2;
    
    result = 0;
    for (i = x - 2; i <= x; i++) {
        if (i >= 0 && i + 2 < BOARD_WIDTH) {
            n0 = 1 << gems_index[i][y];
            n1 = 1 << gems_index[i + 1][y];
            n2 = 1 << gems_index[i + 2][y];
            if (n0 & n1 & n2) {
                result |= 1;    // is part of an horizontal alignment
                break;
            }
        }
    }

    for (i = y - 2; i <= y; i++) {
        if (i >= 0 && i + 2 < BOARD_HEIGHT) {
            n0 = 1 << gems_index[x][i];
            n1 = 1 << gems_index[x][i + 1];
            n2 = 1 << gems_index[x][i + 2];
            if (n0 & n1 & n2) {
                result |= 2;    // is part of a vertical alignment
                break;
            }
        }
    }
    
    return result;
}

/**
 * WARNING: this function doesn't update the "coords" property of gems
 */
static void swap_gems (int x1, int y1, int x2, int y2)
{
    int tmp_index;
    Evas_Object* tmp_gem = NULL;

    tmp_gem = gem_at(x1, y1);
    gem_at(x1, y1) = gem_at(x2, y2);
    gem_at(x2, y2) = tmp_gem;

    tmp_index = gem_index(x1, y1);
    gem_index(x1, y1) = gem_index(x2, y2);
    gem_index(x2, y2) = tmp_index;
} 

/**
 * Checks if there are moves left.
 * Thanks to gweled
 */
static bool check_moves_left(int *pi, int *pj)
{
    int i, j;
    
    for (j = BOARD_HEIGHT - 1; j >= 0; j--)
        for (i = BOARD_WIDTH - 1; i >= 0; i--) {
            if (i > 0) {
                swap_gems (i - 1, j, i, j);
                if (is_part_of_alignment (i, j)) {
                    swap_gems (i - 1, j, i, j);
                    goto move_found;
                }
                swap_gems (i - 1, j, i, j);
            }
            if (i < (BOARD_WIDTH - 1)) {
                swap_gems (i + 1, j, i, j);
                if (is_part_of_alignment (i, j)) {
                    swap_gems (i + 1, j, i, j);
                    goto move_found;
                }
                swap_gems (i + 1, j, i, j);
            }
            if (j > 0) {
                swap_gems (i, j - 1, i, j);
                if (is_part_of_alignment (i, j)) {
                    swap_gems (i, j - 1, i, j);
                    goto move_found;
                }
                swap_gems (i, j - 1, i, j);
            }
            if (j < (BOARD_HEIGHT - 1)) {
                swap_gems (i, j + 1, i, j);
                if (is_part_of_alignment (i, j)) {
                    swap_gems (i, j + 1, i, j);
                    goto move_found;
                }
                swap_gems (i, j + 1, i, j);
            }
        }   
    return FALSE;
      
move_found:
    EINA_LOG_INFO("Move hint on %dx%d", i, j);
    if (pi && pj) {
        *pi = i;
        *pj = j;
    }
    return TRUE;
}

/**
 * Check alignments around the given gem.
 * @param gem
 * @return a list of aligned gems (included the input one)
 */
static Eina_List* check_alignment(Evas_Object* gem)
{
    Eina_List* list = NULL;
    int i;
    int* coords = (int *) evas_object_data_get(gem, "coords");
    int x = coords[0], y = coords[1];

    int index = gem_index(x, y);

    int align = is_part_of_alignment(x, y);
    
    // horizontal alignment - check x axis
    if (align & 0x1) {
        for (i = x; i < BOARD_WIDTH; i++) {
            if (gem_index(i, y) == index)
                list = eina_list_append(list, gem_at(i, y));
            else
                break;
        }
        for (i = x - 1; i >= 0; i--) {
            if (gem_index(i, y) == index)
                list = eina_list_append(list, gem_at(i, y));
            else
                break;
        }
    }

    // vertical alignment - check y axis
    if (align & 0x2) {
        for (i = y; i < BOARD_HEIGHT; i++) {
            if (gem_index(x, i) == index)
                list = eina_list_append(list, gem_at(x, i));
            else
                break;
        }
        for (i = y - 1; i >= 0; i--) {
            if (gem_index(x, i) == index)
                list = eina_list_append(list, gem_at(x, i));
            else
                break;
        }
    }

    return eina_list_count(list) > 0 ?
        eina_list_append(list, gem) :
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

        // update index table
        int index1 = gem_index(coords1[0], coords1[1]);
        int index2 = gem_index(coords2[0], coords2[1]);
        gems_index[coords1[0]][coords1[1]] = index2;
        gems_index[coords2[0]][coords2[1]] = index1;

        // aggiorna le coordinate
        memcpy(coords_tmp, coords1, sizeof(int) * 2);

        // imposta le nuove coordinate
        memcpy(coords1, coords2, sizeof(int) * 2);
        memcpy(coords2, coords_tmp, sizeof(int) * 2);

        // update gem table
        gems[coords1[0]][coords1[1]] = selected1;
        gems[coords2[0]][coords2[1]] = selected2;

        // FIXME for now static verification is enough -- no repositioning
        //evas_object_move(selected1, coords1[0] * size, coords1[1] * size);
        //evas_object_move(selected2, coords2[0] * size, coords2[1] * size);

        // verifica allineamento
        Eina_List* align1 = check_alignment(selected1);
        Eina_List* align2 = check_alignment(selected2);

        if (align1 || align2) {
            Eina_List* align = concat_duplicate(align1, align2);

            // freeze timer
            if (timed_game_timer)
                ecore_timer_freeze(timed_game_timer);

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
    gems_index[coords[0]][coords[1]] = -1;

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
            }

    if (timed_game_timer) {
        ecore_timer_del(timed_game_timer);
        timed_game_timer = NULL;
    }
}

static bool _board_reset(void* data)
{
    board_reset();

    running--;
    return FALSE;
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
        // no more moves!!!
        if (!check_moves_left(NULL, NULL)) {
            EINA_LOG_INFO("No more moves!");

            if (game_type == GAME_TYPE_NORMAL) {
                // end game
                _close(NULL, NULL, NULL);
                return FALSE;
            }

            else if (game_type == GAME_TYPE_TIMED) {
                if (timed_game_timer)
                    ecore_timer_freeze(timed_game_timer);

                // recreate board
                running++;
                ecore_timer_add(0.5, _board_reset, NULL);
            }

            return FALSE;
        }

        if (timed_game_timer)
            ecore_timer_thaw(timed_game_timer);

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
    int i, j, y;
    for (i = 0; i < BOARD_WIDTH; i++) {
        for (j = 0; j < BOARD_HEIGHT; j++) {
            evas_object_geometry_get(gem_at(i, j), NULL, &y, NULL, NULL);

            if (y < (j * GEM_SIZE)) {
                running++;
                ecore_animator_add(_falldown, gem_at(i, j));
            }
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
        if (timed_game_timer)
            ecore_timer_freeze(timed_game_timer);

        ecore_timer_add(0.2, _remove_gems, align);
    }

    else {
        eina_list_free(align);
        selected1 = selected2 = NULL;

        running++;
        ecore_timer_add(0.5, _check_next_level, NULL);
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

        // replace immediately
        put_gem(r, c, x, y, -1);
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
 * Creates and store a gem in the gems tables.
 * Same parameters as make_gem().
 * @param index gem index from theme, -1 for random
 */
static Evas_Object* put_gem(int col, int row, int x, int y, int index)
{
    if (index < 0) {
        index = rand() % theme_gem_count();
    }

    Evas_Object* gem = make_gem(col, row, x, y, index);
    // add to tables
    gems[col][row] = gem;
    gems_index[col][row] = index;

    return gem;
}

/**
 * Creates a gem.
 * @param col column coordinate
 * @param row row coordinate
 * @param x x coordinate (pixels)
 * @param y y coordinate (pixels)
 * @param index gem index from theme
 */
static Evas_Object* make_gem(int col, int row, int x, int y, int index)
{
    Evas_Object* layout = elm_layout_add(win->win);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    char* name = strdup_sprintf("gem%02d", index + 1);

    elm_layout_file_set(layout, theme_get_path(), name);
    free(name);

    evas_object_resize(layout, GEM_SIZE, GEM_SIZE);
    evas_object_move(layout, x, y);
    evas_object_show(layout);

    Evas_Object* edj = elm_layout_edje_get(layout);
    EINA_LOG_DBG("Loading gem%02d (%p) with coords %dx%d size %dx%d position %dx%d", index + 1, edj, col, row, GEM_SIZE, GEM_SIZE, x, y);

    int* coords = m_new(int, 2);
    coords[0] = col;
    coords[1] = row;
    evas_object_data_set(edj, "coords", coords);
    evas_object_data_set(edj, "layout", layout);
    evas_object_data_set(edj, "speed", (void *) (0));

    edje_object_signal_callback_add(edj, "clicked", "*", _gem_clicked, NULL);
    // TODO edje_object_signal_callback_add(edj, "up", "*", _gem_clicked, NULL);
    // TODO edje_object_signal_callback_add(edj, "down", "*", _gem_clicked, NULL);

    return edj;
}

static void create_win(void)
{
    win = mokowin_new(PACKAGE, FALSE);
    win->delete_callback = _close;
    mokowin_create_layout(win, theme_get_path(), "board/bg");

    elm_win_title_set(win->win, PACKAGE_NAME);
    evas_object_resize(win->win, WIN_WIDTH, WIN_HEIGHT);

    elm_object_style_set(win->bg, "board");
}

// here we go!
static bool _start(void* data)
{
    if (game_type == GAME_TYPE_TIMED) {
        if (!timed_game_timer)
            timed_game_timer = ecore_timer_add((double) 4 / current_level, _decrease_bar, NULL);
        else
            ecore_timer_thaw(timed_game_timer);
    }

    fall_gems();

    running--;
    return FALSE;
}

/**
 * Resets the board.
 */
static void board_reset(void)
{
    int i, j;

    // random seed
    srand(time(NULL));

    // destroy previous gems if present
    destroy_board();

    // populate the board
    for (i = 0; i < BOARD_WIDTH; i++)
        for (j = 0; j < BOARD_HEIGHT; j++)
            put_gem(i, j, i * GEM_SIZE, (j - BOARD_HEIGHT) * GEM_SIZE, -1);

    // remove alignments
    remove_alignments();

    // let's start!
    running++;
    ecore_timer_add(0.5, _start, NULL);
}

/**
 * Steps to the next level.
 * Reset the level score and reset the board.
 */
void board_next_level(void)
{
    current_level++;
    EINA_LOG_INFO("Starting level %d (total score %d)",
        current_level, score_total);

    switch (game_type) {
        case GAME_TYPE_NORMAL:
            score_bar = 0;
            break;
        case GAME_TYPE_TIMED:
            score_bar = LEVEL_SCORE / 2;

            // freeze countdown timer if present
            if (timed_game_timer) {
                ecore_timer_del(timed_game_timer);
                timed_game_timer = NULL;
            }

            break;
        default:
            EINA_LOG_CRIT("Unknown game type %d", game_type);
            score_bar = 0;
    }

    score_level = 0;
    update_score_bar();
    board_reset();
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

    EINA_LOG_DBG("Starting new game, type %d", type);
    game_type = type;
    score_total = 0;
    current_level = 20;
    board_next_level();

    mokowin_activate(win);
}
