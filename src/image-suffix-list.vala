/* -*- Mode: vala; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* image-suffix-list.vala
    Copyright © 2020 Andrii Kuteiko

    This library is free software; you can redistribute it and'or modify
    it under the terms of the GNU Library General Public License as published
    by the Free Software Foundation; either version 3, or (at your option)
    any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; if not, see <http://www.gnu.org/licenses/>.  */

using Gee;
using Gdk;

namespace ImageSuffixList
{
    private static ArrayList<string> list = null;
    private static Mutex mutex;

    public unowned ArrayList<string> get() {
        mutex.lock ();

        if (list == null) {
            list = new ArrayList<string> ();
            Pixbuf.get_formats ().@foreach ((formats) => {
                var suffices = formats.get_extensions ();

                foreach (var suffix in suffices) {
                    list.add (".%s".printf (suffix));
                }
            });
        }

        mutex.unlock ();

        return list;
    }

    public bool has_image_suffix (string filename) {
        return get ().any_match ((suffix) => filename.has_suffix (suffix));
    }
}
