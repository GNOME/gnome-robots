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

public Themes themes = null;

public Themes get_themes () {
    if (themes == null) {
        themes = Themes.from_data_directory ();
    }
    return themes;
}

public class Themes : Gtk.ListStore {

    public enum Column {
        DISPLAY = 0,
        NAME,
        PATH,
    }

    public Themes () {
        set_column_types ({ Type.STRING, Type.STRING, Type.STRING });
    }

    public void add (string path) {
        var filename = Path.get_basename (path);
        var display = remove_suffix (filename).replace ("_", " ");

        Gtk.TreeIter iter;
        append (out iter);

        set_value (iter, Column.DISPLAY, display);
        set_value (iter, Column.NAME, filename);
        set_value (iter, Column.PATH, path);
    }

    public static Themes from_directory (string directory) {
        var themes = new Themes ();
        try {
            var dir = Dir.open (directory);
            string? filename;
            while ((filename = dir.read_name ()) != null) {
                if (ImageSuffixList.has_image_suffix (filename)) {
                    var fullname = Path.build_filename (directory, filename);
                    if (FileUtils.test (fullname, FileTest.IS_REGULAR)) {
                        themes.add (fullname);
                    }
                }
            }
        } catch (FileError e) {
            warning ("Themes.from_data_dir: %s.", e.message);
        }
        return themes;
    }

    public static Themes from_data_directory () {
        var directory = GLib.Path.build_filename (DATA_DIRECTORY, "themes");
        return from_directory (directory);
    }

    public Gtk.TreeIter? find_iter_by_name (string name) {
        Gtk.TreeIter iter;
        if (get_iter_first (out iter)) {
            do {
                Value val;
                get_value (iter, Column.NAME, out val);
                if (val == name) {
                    return iter;
                }
            } while (iter_next (ref iter));
        }
        return null;
    }

    public string? find_path_by_name (string name) {
        var iter = find_iter_by_name (name);
        if (iter != null) {
            Value path;
            get_value (iter, Column.PATH, out path);
            return path.get_string ();
        } else {
            return null;
        }
    }

    public void get_values (Gtk.TreeIter iter, out string name, out string path) {
        Value name_value;
        Value path_value;
        get_value (iter, Column.NAME, out name_value);
        get_value (iter, Column.PATH, out path_value);
        name = name_value.get_string ();
        path = path_value.get_string ();
    }

    public Gtk.TreeIter find_best_match_iter (string name) throws Error {
        const string DEFAULT_THEME = "robots.svg";

        var iter = find_iter_by_name (name);
        if (iter != null) {
            return iter;
        }

        iter = find_iter_by_name (DEFAULT_THEME);
        if (iter != null) {
            return iter;
        }

        if (get_iter_first (out iter)) {
            return iter;
        } else {
            throw new FileError.NOENT ("No theme was found.");
        }
    }

    public Theme find_best_match (string name) throws Error {
        var iter = find_best_match_iter (name);

        string theme_name;
        string theme_path;
        get_values (iter, out theme_name, out theme_path);

        return new Theme.from_file (theme_path, theme_name);
    }
}

string remove_suffix (string filename) {
    var s = filename.last_index_of_char ('.');
    if (s >= 0) {
        return filename.substring (0, s);
    } else {
        return filename;
    }
}

