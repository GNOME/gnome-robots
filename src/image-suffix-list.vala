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
