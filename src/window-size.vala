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

public interface WindowSize : Object {
    public abstract int width { get; set; }
    public abstract int height { get; set; }
    public abstract bool is_maximized { get; set; }
}

public class WindowSizeSettings : Object, WindowSize {
    public override int width {
        get { return settings.get_int ("window-width"); }
        set { settings.set_int ("window-width", value); }
    }

    public override int height {
        get { return settings.get_int ("window-height"); }
        set { settings.set_int ("window-height", value); }
    }

    public override bool is_maximized {
        get { return settings.get_boolean ("window-is-maximized"); }
        set { settings.set_boolean ("window-is-maximized", value); }
    }

    private GLib.Settings settings;

    public WindowSizeSettings (string schema_id) {
        this.settings = new GLib.Settings (schema_id);
    }
}

private bool window_configure_event_cb (Gtk.Window window,
                                        WindowSize size
) {
    if (!size.is_maximized) {
        int width, height;
        window.get_size (out width, out height);
        size.width = width;
        size.height = height;
    }
    return false;
}

private bool window_state_event_cb (Gtk.Window window,
                                    WindowSize size,
                                    Gdk.EventWindowState event
) {
    if ((event.changed_mask & Gdk.WindowState.MAXIMIZED) != 0) {
        size.is_maximized = (event.new_window_state & Gdk.WindowState.MAXIMIZED) != 0;
    }
    return false;
}

public void remember_window_size (Gtk.Window window, WindowSize size) {
    window.configure_event.connect (() => window_configure_event_cb (window, size));
    window.window_state_event.connect (event => window_state_event_cb (window, size, event));
    window.set_default_size (size.width, size.height);
    if (size.is_maximized) {
        window.maximize ();
    }
}

