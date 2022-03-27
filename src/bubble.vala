/*
 * Copyright 2020 Andrey Kutejko <andy128k@gmail.com>
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

using Gdk;
using Graphene;

public class Bubble {
    private Paintable paintable;

    public Bubble.from_file (string filename) throws Error {
        paintable = image_from_file (filename);
    }

    public void draw (Gtk.Snapshot snapshot, float x, float y) {
        var width = paintable.get_intrinsic_width ();
        var height = paintable.get_intrinsic_height ();

        var bubble_width = width / 2;
        var bubble_height = height / 2;

        var clip_rect = Rect () {
            origin = Point () {
                x = x < bubble_width ? x : x - bubble_width,
                y = y < bubble_height ? y : y - bubble_height,
            },
            size = Size () {
                width = bubble_width,
                height = bubble_height,
            },
        };

        snapshot.push_clip (clip_rect);
        snapshot.save ();
        snapshot.translate (Point () {
            x = x - bubble_width,
            y = y - bubble_height,
        });
        paintable.snapshot (snapshot, width, height);
        snapshot.restore ();
        snapshot.pop ();
    }
}

