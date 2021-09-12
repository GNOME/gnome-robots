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

public class GamesControlsList : Bin {

    private Gtk.ListStore store;
    private TreeView view;
    private Properties properties;

    private enum Column {
        LABEL = 0,
        INDEX,
        KEYCODE,
        KEYMODS,
        COUNT
    }

    public GamesControlsList(Properties properties) {
        store = new Gtk.ListStore (Column.COUNT,
                                   Type.STRING,
                                   Type.INT,
                                   Type.UINT,
                                   Type.UINT);

        view = new TreeView.with_model (store);
        view.set_headers_visible (false);
        view.set_enable_search (false);

        /* label column */
        var label_renderer = new CellRendererText ();
        var column = new TreeViewColumn ();
        column.title = "Control";
        column.pack_start (label_renderer, true);
        column.add_attribute (label_renderer, "text", Column.LABEL);

        view.append_column (column);

        /* key column */
        var key_renderer = new CellRendererAccel ();
        key_renderer.editable = true;
        key_renderer.accel_mode = CellRendererAccelMode.OTHER;
        key_renderer.accel_edited.connect ((_cell, path, keyval, _mods, _keycode) => {
            accel_edited_cb (path, keyval);
        });
        key_renderer.accel_cleared.connect ((_cell, path) => {
            accel_cleared_cb(path);
        });

        var column2 = new TreeViewColumn ();
        column2.title = "Key";
        column2.pack_start (key_renderer, true);
        column2.add_attribute (key_renderer, "accel-key", Column.KEYCODE);
        column2.add_attribute (key_renderer, "accel-mods", Column.KEYMODS);

        view.append_column (column2);

        var sw = new ScrolledWindow (null, null);
        sw.hscrollbar_policy = PolicyType.NEVER;
        sw.vscrollbar_policy = PolicyType.AUTOMATIC;
        sw.shadow_type = ShadowType.IN;
        sw.add (view);

        add (sw);

        this.properties = properties;
        properties.changed.connect (properties_changed_cb);

        var key_labels = new string[] {
            _("Key to move NW"),
            _("Key to move N"),
            _("Key to move NE"),
            _("Key to move W"),
            _("Key to hold"),
            _("Key to move E"),
            _("Key to move SW"),
            _("Key to move S"),
            _("Key to move SE")
        };
        for (var i = 0; i < key_labels.length; ++i) {
            add_control (i, key_labels[i]);
        }
    }

    ~GamesControlsList () {
        properties.changed.disconnect (properties_changed_cb);
    }

    private void add_control (int index, string label) {
        TreeIter iter;
        store.append (out iter);
        store.set_value (iter, Column.LABEL, label);
        store.set_value (iter, Column.INDEX, index);
        store.set_value (iter, Column.KEYCODE, properties.keys[index]);
        store.set_value (iter, Column.KEYMODS, 0);
    }

    private void accel_edited_cb (string path_string, uint keyval) {
        var path = new TreePath.from_string (path_string);
        if (path == null)
            return;

        TreeIter iter;
        if (!store.get_iter (out iter, path)) {
            return;
        }

        Value key_index_value;
        store.get_value (iter, Column.INDEX, out key_index_value);
        int key_index = key_index_value.get_int();

        /* FIXME: what to do with the modifiers? */
        properties.keys[key_index] = keyval;
        store.set_value (iter, Column.KEYCODE, keyval);
    }

    private void accel_cleared_cb (string path_string) {
        var path = new TreePath.from_string (path_string);
        if (path == null)
            return;

        TreeIter iter;
        if (!store.get_iter (out iter, path))
            return;

        Value key_index_value;
        store.get_value (iter, Column.INDEX, out key_index_value);
        int key_index = key_index_value.get_int();

        /* FIXME: what to do with the modifiers? */
        properties.keys.reset (key_index);
        var keyval = properties.keys[key_index];
        store.set_value (iter, Column.KEYCODE, keyval);
    }

    private void properties_changed_cb () {
        TreeIter iter;
        if (store.get_iter_first (out iter)) {
            do {
                Value key_index_value;
                store.get_value (iter, Column.INDEX, out key_index_value);
                int key_index = key_index_value.get_int();

                store.set_value (iter, Column.KEYCODE, properties.keys[key_index]);
                store.set_value (iter, Column.KEYMODS, 0); // FIXME?
            } while (store.iter_next(ref iter));
        }
    }
}

