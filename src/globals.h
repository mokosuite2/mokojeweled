/*
 * Mokojeweled
 * Globals definitions
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

#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <Elementary.h>
#include <libintl.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// default log domain
#undef EINA_LOG_DOMAIN_DEFAULT
#define EINA_LOG_DOMAIN_DEFAULT _log_dom
extern int _log_dom;

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

typedef Eina_Bool bool;

#define TRUE    EINA_TRUE
#define FALSE   EINA_FALSE

#ifdef DEBUG
#define LOG_LEVEL   EINA_LOG_LEVEL_DBG
#else
#define LOG_LEVEL   EINA_LOG_LEVEL_INFO
#endif

/**
 * Animation frame time
 */
#define FRAMETIME   0.01

// default window size
#define WIN_WIDTH       480
#define WIN_HEIGHT      600

// FIXME replace with a working one :)
#define _   gettext

#define return_if_fail(x)   ({ \
    if (!(x)) \
        EINA_LOG_WARN("(%s) failed!", #x); \
})

#define return_val_if_fail(x, v)   ({ \
    if (!(x)) { \
        EINA_LOG_WARN("(%s) failed!", #x); \
        return v; \
    } \
})

/**
 * malloc() and memset() to zero
 * @param s size
 * @return allocated area
 */
#define malloc0(s)     ({ \
    void* m = malloc(s); \
    memset(m, 0, s); \
    (m); \
})

/**
 * malloc(sizeof(t) * n) and memset() to zero
 * @param t type
 * @param n count
 * @return allocated area
 */
#define m_new0(t, n)     ({ \
    void* m = malloc(sizeof(t) * n); \
    memset(m, 0, sizeof(t) * n); \
    ((t*) m); \
})

/**
 * Game types
 */
typedef enum {
    GAME_TYPE_NORMAL = 0,
    GAME_TYPE_TIMED,
} GameType;

#endif  /* __GLOBALS_H */
