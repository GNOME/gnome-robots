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

using Gtk;
using Gee;

public class GamesFileList {

    [Flags]
    public enum Flags {
        REMOVE_EXTENSION = 1 << 0,
        REPLACE_UNDERSCORES = 1 << 1,
    }

    private TreeSet<string> files;

    /**
     * @glob: A pattern to match files against. See g_pattern_spec_new () for
     * details.
     * @varargs: A NULL terminated list of strings containing directory names to
     * be searched for files.
     *
     * This function takes a glob and a NULL terminated list of directories
     * and constructs a list of all files in the directories that match the glob.
     * Only regular files are returned.
     *
     * Return value: A pointer to a new FileList containing files
     * matching the glob in the path.
     **/
    public GamesFileList (string glob, ...) {
        var paths = va_list();
        files = new_internal (glob, paths);
    }

    /**
     * @path1: A NULL-terminated list of strings containing directories to be
     * searched.
     *
     * A convenience function which constructs a list of filenames which
     * are images that can be loaded via gdk-pixbuf. Whether a file is an
     * image or not is determined by its extension. The list of possible
     * extensions is determined by querying the gdk-pixbuf library the
     * first time this function is called.
     *
     * Return value: A new FileList containing the list of image files.
     **/
    public GamesFileList.images (string path1, ...) {
        files = new TreeSet<string> ();

        find_images (path1, files);

        var paths = va_list();
        for (string? pathentry = paths.arg<string?> (); pathentry != null ; pathentry = paths.arg<string?> ()) {
            find_images (pathentry, files);
        }
    }

    /* Transform the list of files to be only the basenames. */
    public void transform_basename () {
        var new_files = new TreeSet<string> ();
        foreach (var file in files) {
            new_files.add (Path.get_basename (file));
        }
        files = new_files;
    }

    /**
     * Get the number of elements in the file list.
     **/
    public uint length () {
        return files.size;
    }

    /**
     * @function: (scope call): The function to call on each item. It gets called with two
     * arguments: the file name and the pointer supplied to this function in
     * the userdata argument.
     * @userdata: (closure): An arbitrary pointer that gets passed as the second argument
     * to each call of function.
     *
     * Find a file name by iterating through a list until the given function
     * returns 0.
     *
     * Return value: A newly allocated string containing a copy of the file name,
     * or NULL if no file name was found.
     **/
    public string? find (CompareFunc<string> function, string val) {
        return files.first_match ((file) => function (file, val) == 0);
    }

    /**
     * @n: The 0-based index into the list.
     *
     * Obtain the (n+1)th file name from the list.
     **/
    //public string get_nth (int n) {
    //    return files[n];
    //}

    public TreeModel create_model (Flags flags) {
        var model = new Gtk.ListStore (2, Type.STRING, Type.STRING);
        foreach (var item in files) {
            var visible = item;

            /* These are a bit hackish, but we don't yet have a good regexp
             * library in glib. There are probably some ways these could
             * seriously mangle unicode strings. */
            if (Flags.REMOVE_EXTENSION in flags) {
                var s = visible.last_index_of_char ('.');
                if (s >= 0)
                    visible = visible.substring (0, s);
            }
            if (Flags.REPLACE_UNDERSCORES in flags) {
                visible = visible.replace ("_", " ");
            }

            TreeIter iter;
            model.append (out iter);

            model.set_value (iter, 0, visible);
            model.set_value (iter, 1, item);
        }
        return model;
    }

    /**
     * @selection: The name to select as the default. NULL means no default.
     * @flags: A set of flags to specify how the names are displayed.
     *
     * Create a combo box with the given list of strings as the entries. If
     * selection is non-NULL the matching file name is selected by default.
     * Otherwise nothing is selected. The flags affect how the names are
     * displayed. The valid flags are GAMES_FILE_LIST_REMOVE_EXTENSION, which
     * removes extensions, and GAMES_FILE_LIST_REPLACE_UNDERSCORES with replaces
     * underscores with spaces.
     *
     * Return value: A widget with the list of names.
     **/
    public ComboBox create_widget (string? selection, Flags flags) {
        var model = create_model (flags);
        var widget = new ComboBox.with_model (model);
        var renderer = new CellRendererText ();
        widget.pack_start (renderer, true);
        widget.add_attribute (renderer, "text", 0);

        bool found = false;
        if (selection != null) {
            TreeIter iter;
            if (model.get_iter_first (out iter)) {
                do {
                    Value file;
                    model.get_value (iter, 1, out file);
                    if (file == selection) {
                        widget.set_active_iter (iter);
                        found = true;
                        break;
                    }
                } while (model.iter_next (ref iter));
            }
        }
        if (!found) {
            widget.set_active (0);
        }

        return widget;
    }

    private static TreeSet<string> new_internal (string glob, va_list paths) {
        var result = new TreeSet<string> ();
        var filespec = new PatternSpec (glob);

        for (string? pathelement = paths.arg<string?> (); pathelement != null ; pathelement = paths.arg<string?> ()) {
            try {
                var dir = Dir.open (pathelement);
                string? filename;
                while ((filename = dir.read_name ()) != null) {
                    if (filespec.match_string (filename)) {
                        var fullname = Path.build_filename (pathelement, filename);
                        if (FileUtils.test (fullname, FileTest.IS_REGULAR)) {
                            result.add (fullname);
                        }
                    }
                }
            } catch (FileError e) {
                // ignored
            }
        }

        return result;
    }

    private static void find_images (string directory, TreeSet<string> result) {
        try {
            var dir = Dir.open (directory);
            string? filename;
            while ((filename = dir.read_name ()) != null) {
                if (ImageSuffixList.has_image_suffix (filename)) {
                    var fullname = Path.build_filename (directory, filename);
                    if (FileUtils.test (fullname, FileTest.IS_REGULAR)) {
                        result.add (fullname);
                    }
                }
            }
        } catch (FileError e) {
            // ignored
        }
    }
}

