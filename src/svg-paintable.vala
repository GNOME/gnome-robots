/*
 * Copyright 2022 Andrey Kutejko <andy128k@gmail.com>
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
using Rsvg;

class SvgPaintable : Object, Gdk.Paintable {

    private Handle rsvg_handle;

    public SvgPaintable (Handle rsvg_handle) {
        this.rsvg_handle = rsvg_handle;
    }

    public void snapshot (Gdk.Snapshot snapshot,
                          double width,
                          double height
    ) {
        var rect = Rect () {
            origin = Point.zero (),
            size = Size () { width = (float) width, height = (float) height },
        };

        var gtk_snapshot = (Gtk.Snapshot) snapshot;
        var cr = gtk_snapshot.append_cairo (rect);

        try {
            var rsvg_rect = Rsvg.Rectangle () {
                x = 0,
                y = 0,
                width = width,
                height = height,
            };
            rsvg_handle.render_document (cr, rsvg_rect);
        } catch (GLib.Error error) {
            critical ("%s", error.message);
        }
    }

    public override int get_intrinsic_width () {
        double width;
        double height;

        if (!rsvg_handle.get_intrinsic_size_in_pixels (out width, out height)) {
            return 0;
        }
        return (int) Math.ceil (width);
    }

    public override int get_intrinsic_height () {
        double width;
        double height;

        if (!rsvg_handle.get_intrinsic_size_in_pixels (out width, out height)) {
            return 0;
        }
        return (int) Math.ceil (height);
    }
}

