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

using Gtk;

public class GamesControlsList : Adw.PreferencesGroup {

    private Properties properties;
    private ControlEditor[] editors;

    public GamesControlsList (Properties properties) {
        this.properties = properties;
        properties.changed.connect (properties_changed_cb);

        title = _("Keyboard");

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
        editors = new ControlEditor[key_labels.length];
        for (var i = 0; i < key_labels.length; ++i) {
            editors[i] = new_editor (i);

            var action_row = new Adw.ActionRow ();
            action_row.title = key_labels[i];
            action_row.add_suffix (editors[i]);
            action_row.activatable_widget = editors[i];
            add (action_row);
        }

        var dbut = new Button.with_mnemonic (_("_Restore Defaults"));
        dbut.clicked.connect (reset_keys);
        header_suffix = dbut;
    }

    ~GamesControlsList () {
        properties.changed.disconnect (properties_changed_cb);
    }

    private ControlEditor new_editor (int index) {
        var editor = new ControlEditor ();
        editor.keycode = properties.keys[index];
        editor.edited.connect ((keyval) => {
            properties.keys[index] = keyval;
            editor.keycode = keyval;
        });
        editor.cleared.connect (() => {
            properties.keys.reset (index);
            editor.keycode = properties.keys[index];
        });
        return editor;
    }

    private void properties_changed_cb () {
        for (var i = 0; i < editors.length; ++i) {
            editors[i].keycode = properties.keys[i];
        }
    }

    private void reset_keys () {
        properties.keys.reset_all ();
    }
}

class ControlEditor : Widget {
    private Label label;

    private uint keycode_ = 0;
    public uint keycode {
        get {
            return keycode_;
        }
        set {
            keycode_ = value;
            update_label ();
        }
    }

    private bool editing_ = false;
    public bool editing {
        get {
            return editing_;
        }
        set {
            editing_ = value;
            if (editing_) {
                grab_focus ();
            }
            update_label ();
        }
    }

    public ControlEditor () {
        set_layout_manager (new BinLayout ());
        focusable = true;

        var click_controller = new GestureClick ();
        click_controller.pressed.connect ((_n, _x, _y) => {
            editing = true;
        });
        add_controller (click_controller);

        var key_controller = new EventControllerKey ();
        key_controller.key_pressed.connect ((keyval, _keycode, mods) => key_controller_key_pressed (keyval, mods));
        add_controller (key_controller);

        label = new Label ("");
        label.halign = Align.START;
        label.valign = Align.CENTER;
        label.set_parent (this);

        notify["mnemonic-activate"].connect (() => {
            editing = true;
        });

        move_focus.connect_after ((direction) => {
            editing = false;
        });
    }

    private void update_label () {
        if (editing) {
            label.label = _("New accelerator…");
        } else if (keycode_ > 0) {
            label.label = accelerator_get_label_with_keycode (get_display (), keycode_, keycode_, 0);
        } else {
            label.label = _("Disabled");
        }
    }

    public signal void edited (uint keyval);
    public signal void cleared ();

    private bool key_controller_key_pressed (uint keyval, Gdk.ModifierType mods) {
        if (editing) {
            editing = false;
            if (mods == 0) {
                switch (keyval) {
                case Gdk.Key.Tab:
                    return false;
                case Gdk.Key.BackSpace:
                    cleared ();
                    break;
                case Gdk.Key.Escape:
                    break;
                default:
                    edited (keyval);
                    break;
                }
                return true;
            }
        } else {
            if (mods == 0) {
                switch (keyval) {
                case Gdk.Key.Return:
                case Gdk.Key.KP_Enter:
                    editing = true;
                    return true;
                default:
                    return false;
                }
            }
        }
        return false;
    }
}

