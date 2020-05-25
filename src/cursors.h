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

#ifndef CURSORS_H
#define CURSORS_H

void make_cursors (void);
void set_cursor_default (GdkSurface * surface);
void set_cursor_by_direction (GdkSurface * surface, int dx, int dy);

#endif
