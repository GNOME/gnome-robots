/*
 * Gnome Robots II - Cursor definitions.
 *
 * Copyright 2004 by Callum McKenzie.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * For more details see the file COPYING.
 */

#include <config.h>
#include <gtk/gtk.h>

#include "cursors.h"
#include "game.h"
#include "gbdefs.h"

#include "cursors.h"

typedef struct {
  gchar *small_name;
  int hotspot_x;
  int hotspot_y;
  GdkCursor *cursor;
} cursor_props;

cursor_props cursor_list[] = { /* cursors are 17x17 */
  { "up-left",     1,  1, NULL },
  { "up",          8,  1, NULL },
  { "up-right",   15,  1, NULL },
  { "left",        1,  8, NULL },
  { "hold",        8,  8, NULL },
  { "right",      15,  8, NULL },
  { "down-left",   1, 15, NULL },
  { "down",        8, 15, NULL },
  { "down-right", 15, 15, NULL }
};

GdkCursor *default_cursor;

void
make_cursors (void)
{
  GdkTexture *texture;
  int i;
  gchar *resource;
  cursor_props *c;

  default_cursor = gdk_cursor_new_from_name ("default", /* fallback */ NULL);

  c = cursor_list;
  for (i = 0; i < G_N_ELEMENTS (cursor_list); ++i) {
    resource = g_strconcat ("/org/gnome/Robots/cursors/cursor-", c->small_name, ".png", NULL);
    texture = gdk_texture_new_from_resource (resource);

    c->cursor = gdk_cursor_new_from_texture (texture, c->hotspot_x, c->hotspot_y, /* fallback */ NULL);

    g_object_unref (texture);
    g_free (resource);

    c++;
  }
}

void
set_cursor_default (GdkSurface * surface)
{
  gdk_surface_set_cursor (surface, default_cursor);
}

void
set_cursor_by_direction (GdkSurface * surface, int dx, int dy)
{
  int index;

  if (game_state != STATE_PLAYING) {
    set_cursor_default (surface);
    return;
  }

  index = 3 * dy + dx + 4;

  gdk_surface_set_cursor (surface, cursor_list[index].cursor);
}
