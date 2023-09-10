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

public class Properties {
    const string KEY_BACKGROUND_COLOR = "background-color";
    const string KEY_CONFIGURATION = "configuration";
    const string KEY_ENABLE_SOUND = "enable-sound";
    const string KEY_SAFE_MOVES = "use-safe-moves";
    const string KEY_SHOW_TOOLBAR = "show-toolbar";
    const string KEY_SUPER_SAFE_MOVES = "use-super-safe-moves";
    const string KEY_THEME = "theme";

    public class Keys {
        private GLib.Settings settings;

        internal Keys (GLib.Settings settings) {
            this.settings = settings;
        }

        public int size {
            get {
                return 9;
            }
        }

        public uint get (uint index) {
            if (index < size) {
                return (uint) settings.get_int ("key%02u".printf (index));
            } else {
                return 0;
            }
        }

        public void set (uint index, uint keyval) {
            if (index < size) {
                settings.set_int ("key%02u".printf (index), (int) keyval);
            }
        }

        public uint get_default (uint index) {
            if (index < size) {
                return (uint) settings
                       .get_default_value ("key%02u".printf (index))
                       .get_int32 ();
            } else {
                return 0;
            }
        }

        public void reset (uint index) {
            if (index < size) {
                string key = "key%02u".printf (index);
                settings.reset (key);
            }
        }

        public void reset_all () {
            for (var index = 0; index < size; ++index) {
                reset (index);
            }
        }
    }

    public bool safe_moves {
        get {
            return settings.get_boolean (KEY_SAFE_MOVES);
        }
        set {
            settings.set_boolean (KEY_SAFE_MOVES, value);
        }
    }

    public bool super_safe_moves {
        get {
            return settings.get_boolean (KEY_SUPER_SAFE_MOVES);
        }
        set {
            settings.set_boolean (KEY_SUPER_SAFE_MOVES, value);
        }
    }

    public bool sound {
        get {
            return settings.get_boolean (KEY_ENABLE_SOUND);
        }
        set {
            settings.set_boolean (KEY_ENABLE_SOUND, value);
        }
    }

    public bool show_toolbar {
        get {
            return settings.get_boolean (KEY_SHOW_TOOLBAR);
        }
        set {
            settings.set_boolean (KEY_SHOW_TOOLBAR, value);
        }
    }

    public Gdk.RGBA bgcolour {
        get {
            return string_to_rgba (settings.get_string (KEY_BACKGROUND_COLOR));
        }
        set {
            settings.set_string (KEY_BACKGROUND_COLOR, rgba_to_string (value));
        }
    }

    public string selected_config {
        owned get {
            return settings.get_string (KEY_CONFIGURATION);
        }
        set {
            settings.set_string (KEY_CONFIGURATION, value);
        }
    }

    public Keys keys {
        get {
            return _keys;
        }
    }

    public string theme {
        owned get {
            return settings.get_string (KEY_THEME);
        }
        set {
            settings.set_string (KEY_THEME, value);
        }
    }

    private GLib.Settings settings;
    private Keys _keys;
    private ulong notify_handler_id;

    public signal void changed ();

    public Properties (GLib.Settings settings) {
        this.settings = settings;
        this._keys = new Keys (settings);
        notify_handler_id = settings.changed.connect (() => changed ());
    }

    ~Properties () {
        settings.disconnect (notify_handler_id);
    }
}

