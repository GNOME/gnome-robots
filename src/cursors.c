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

#include "cursor-down.h"
#include "cursor-down-left.h"
#include "cursor-down-right.h"
#include "cursor-hold.h"
#include "cursor-left.h"
#include "cursor-right.h"
#include "cursors.h"
#include "cursor-up.h"
#include "cursor-up-left.h"
#include "cursor-up-right.h"

typedef struct {
  int is_xpm;
  const guint8 *data;
  gsize data_len;
  int hsx;
  int hsy;
  GdkCursor *cursor;
} cursor_props;

#define CURSOR_ENTRY(is_xpm, d,hx,hy) { is_xpm, cursor_##d, sizeof (cursor_##d), hx, hy, NULL }

cursor_props cursor_list[] = {
  CURSOR_ENTRY (1, up_left, 3, 3),
  CURSOR_ENTRY (0, up, 10, 3),
  CURSOR_ENTRY (1, up_right, 17, 3),
  CURSOR_ENTRY (0, left, 3, 10),
  CURSOR_ENTRY (0, hold, 10, 10),
  CURSOR_ENTRY (0, right, 17, 10),
  CURSOR_ENTRY (0, down_left, 3, 17),
  CURSOR_ENTRY (0, down, 10, 17),
  CURSOR_ENTRY (0, down_right, 17, 17)
};

GdkCursor *default_cursor;

void
make_cursors (void)
{
  GdkPixbuf *pixbuf;
  int i;
  cursor_props *c;

  default_cursor = gdk_cursor_new_for_display (gdk_display_get_default (),
                                               GDK_LEFT_PTR);

  c = cursor_list;
  for (i = 0; i < G_N_ELEMENTS (cursor_list); ++i) {
    if (c->is_xpm) {
      pixbuf = gdk_pixbuf_new_from_xpm_data ((const char**) c->data);
    } else {
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      pixbuf = gdk_pixbuf_new_from_inline (c->data_len, c->data, FALSE, NULL);
G_GNUC_END_IGNORE_DEPRECATIONS
    }
    c->cursor = gdk_cursor_new_from_pixbuf (gdk_display_get_default (),
                                            pixbuf,
                                            c->hsx, c->hsy);
    g_object_unref (pixbuf);

    c++;
  }
}

void
set_cursor_default (GdkWindow * window)
{
  gdk_window_set_cursor (window, default_cursor);
}

void
set_cursor_by_direction (GdkWindow * window, int dx, int dy)
{
  int index;

  if (game_state != STATE_PLAYING) {
    set_cursor_default (window);
    return;
  }

  index = 3 * dy + dx + 4;

  gdk_window_set_cursor (window, cursor_list[index].cursor);
}
