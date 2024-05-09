/*
 * Copyright 2020-2024 Andrey Kutejko <andy128k@gmail.com>
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

use crate::utils::SettingsWarnExt;
use gtk::{gio, glib, prelude::*};

const KEY_WINDOW_WIDTH: &str = "window-width";
const KEY_WINDOW_HEIGHT: &str = "window-height";
const KEY_WINDOW_IS_MAXIMIZED: &str = "window-is-maximized";

pub fn remember_window_size(window: &gtk::Window, settings: &gio::Settings) {
    window.connect_default_width_notify(glib::clone!(@strong settings => move |w| {
        if !w.is_maximized() {
            settings.set_int_or_warn(KEY_WINDOW_WIDTH, w.default_width());
        }
    }));
    window.connect_default_height_notify(glib::clone!(@strong settings => move |w| {
        if !w.is_maximized() {
            settings.set_int_or_warn(KEY_WINDOW_HEIGHT, w.default_height());
        }
    }));
    window.connect_maximized_notify(glib::clone!(@strong settings => move |w| {
        settings.set_boolean_or_warn(KEY_WINDOW_IS_MAXIMIZED, w.is_maximized());
    }));
    window.set_default_size(
        settings.int(KEY_WINDOW_WIDTH),
        settings.int(KEY_WINDOW_HEIGHT),
    );
    if settings.boolean(KEY_WINDOW_IS_MAXIMIZED) {
        window.maximize();
    }
}
