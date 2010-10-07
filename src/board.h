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

#ifndef __BOARD_H
#define __BOARD_H

#include "globals.h"

#define GEM_SIZE            80  // gem size
#define GEM_OFFSET          10  // movement offset
#define GEM_POINTS          10  // points per gem

#define LEVEL_SCORE         800 // level score base

#define BOARD_HEIGHT        (WIN_HEIGHT / GEM_SIZE)
#define BOARD_WIDTH         (WIN_WIDTH / GEM_SIZE)

// number of minimal aligned gems
#define GEMS_MIN_ALIGNED    3

void board_next_level(void);

void board_new_game(GameType type);

#endif  /* __BOARD_H */
