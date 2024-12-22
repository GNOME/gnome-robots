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

use gtk::gio::{self, prelude::*};

const KEY_WINDOW_WIDTH: &str = "window-width";
const KEY_WINDOW_HEIGHT: &str = "window-height";
const KEY_WINDOW_IS_MAXIMIZED: &str = "window-is-maximized";

pub fn remember_window_size(window: &gtk::Window, settings: &gio::Settings) {
    settings
        .bind(KEY_WINDOW_WIDTH, window, "default-width")
        .build();
    settings
        .bind(KEY_WINDOW_HEIGHT, window, "default-height")
        .build();
    settings
        .bind(KEY_WINDOW_IS_MAXIMIZED, window, "maximized")
        .build();
}
