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

public class GamesControlsList : ScrolledWindow {

    private Gtk.ListStore store;
    private TreeView view;

    private GLib.Settings settings;
    private ulong notify_handler_id;

    private enum Column {
        CONFKEY = 0,
        LABEL,
        KEYCODE,
        KEYMODS,
        DEFAULT_KEYCODE,
        DEFAULT_KEYMODS,
        COUNT
    }

    public GamesControlsList(GLib.Settings settings) {
        store = new Gtk.ListStore (Column.COUNT,
                                   Type.STRING,
                                   Type.STRING,
                                   Type.UINT,
                                   Type.UINT,
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
        key_renderer.accel_edited.connect ((_cell, path, keyval, _mask, _keycode) => {
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

        hscrollbar_policy = PolicyType.NEVER;
        vscrollbar_policy = PolicyType.AUTOMATIC;
        shadow_type = ShadowType.IN;
        add (view);

        this.settings = settings;
        notify_handler_id = settings.changed.connect ((_settings, key) => {
            settings_changed_cb(key);
        });
    }

    ~GamesControlsList () {
        settings.disconnect (notify_handler_id);
    }

    public void add_control (string conf_key,
                             string label,
                             uint default_keyval) {
        var keyval = settings.get_int (conf_key);

        TreeIter iter;
        store.append (out iter);
        store.set_value (iter, Column.CONFKEY, conf_key);
        store.set_value (iter, Column.LABEL, label);
        store.set_value (iter, Column.KEYCODE, keyval);
        store.set_value (iter, Column.KEYMODS, 0);
        store.set_value (iter, Column.DEFAULT_KEYCODE, default_keyval);
        store.set_value (iter, Column.DEFAULT_KEYMODS, 0);
    }

    private void accel_edited_cb (string path_string, uint keyval) {
        var path = new TreePath.from_string (path_string);
        if (path == null)
            return;

        TreeIter iter;
        if (!store.get_iter (out iter, path)) {
            return;
        }

        Value conf_key;
        store.get_value (iter, Column.CONFKEY, out conf_key);

        /* Note: the model is updated in the conf notification callback */
        /* FIXME: what to do with the modifiers? */
        settings.set_int (conf_key.get_string(), (int) keyval);
    }

    private void accel_cleared_cb (string path_string) {
        var path = new TreePath.from_string (path_string);
        if (path == null)
            return;

        TreeIter iter;
        if (!store.get_iter (out iter, path))
            return;

        Value conf_key;
        store.get_value (iter, Column.CONFKEY, out conf_key);

        Value default_keyval;
        store.get_value (iter, Column.DEFAULT_KEYCODE, out default_keyval);

        /* Note: the model is updated in the conf notification callback */
        /* FIXME: what to do with the modifiers? */
        settings.set_int (conf_key.get_string(), (int) default_keyval.get_uint());
    }

    private void settings_changed_cb (string key) {
        var keyval = settings.get_int (key);

        /* find our gconf key in the list store and update it */
        TreeIter iter;
        if (store.get_iter_first (out iter)) {
            do {
                Value conf_key;
                store.get_value (iter, Column.CONFKEY, out conf_key);

                if (key == conf_key.get_string()) {
                    store.set_value (iter, Column.KEYCODE, keyval);
                    store.set_value (iter, Column.KEYMODS, 0); // FIXME?
                    break;
                }
            } while (store.iter_next(ref iter));
        }
    }
}

