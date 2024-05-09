/*
 * Copyright 2024 Andrey Kutejko <andy128k@gmail.com>
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

pub mod controls;
pub mod editor;

use gtk::gdk;

pub fn key_from_u32(keyval: u32) -> gdk::Key {
    use gtk::glib::translate::FromGlib;
    unsafe { gdk::Key::from_glib(keyval) }
}

pub fn key_from_i32(keyval: i32) -> gdk::Key {
    key_from_u32(keyval as u32)
}

pub fn key_into_i32(keyval: gdk::Key) -> i32 {
    use gtk::glib::translate::IntoGlib;
    keyval.into_glib() as i32
}
