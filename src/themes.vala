/*
 * Copyright 2020-2023 Andrey Kutejko <andy128k@gmail.com>
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

namespace Themes {

    public ListStore from_directory (string directory) {
        var themes = new ListStore (typeof (Theme));
        try {
            var dir = Dir.open (directory);
            string? filename;
            while ((filename = dir.read_name ()) != null) {
                if (ImageSuffixList.has_image_suffix (filename)) {
                    var path = Path.build_filename (directory, filename);
                    if (FileUtils.test (path, FileTest.IS_REGULAR)) {
                        var theme = new Theme.from_file (path, filename);
                        themes.append (theme);
                    }
                }
            }
        } catch (Error e) {
            warning ("Themes.from_directory: %s.", e.message);
        }
        return themes;
    }

    public struct ThemeResult {
        public uint index;
        public Theme theme;
    }

    public ThemeResult? find_by_name (ListModel themes, string name) {
        var size = themes.get_n_items ();
        for (var index = 0; index < size; ++index) {
            var theme = (Theme) themes.get_item (index);
            if (theme.name == name) {
                return ThemeResult () {
                           index = index, theme = theme
                };
            }
        }
        return null;
    }

    const string DEFAULT_THEME = "robots.svg";

    public Theme find_best_match (ListModel themes, string name) throws Error {
        var named = find_by_name (themes, name);
        if (named != null) {
            return named.theme;
        }
        var deflt = find_by_name (themes, DEFAULT_THEME);
        if (deflt != null) {
            return deflt.theme;
        }
        throw new FileError.NOENT ("No theme was found.");
    }
}

