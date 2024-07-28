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

use crate::config::DATA_DIRECTORY;
use gtk::gdk;
use std::path::Path;

pub fn make_cursors() -> Vec<gdk::Cursor> {
    vec![
        make_cursor("cursor-up-left.png", 3, 3),
        make_cursor("cursor-up.png", 10, 3),
        make_cursor("cursor-up-right.png", 17, 3),
        make_cursor("cursor-left.png", 3, 10),
        make_cursor("cursor-hold.png", 10, 10),
        make_cursor("cursor-right.png", 17, 10),
        make_cursor("cursor-down-left.png", 3, 17),
        make_cursor("cursor-down.png", 10, 17),
        make_cursor("cursor-down-right.png", 17, 17),
    ]
}

fn make_cursor(filename: &str, hsx: i32, hsy: i32) -> gdk::Cursor {
    let path = Path::new(DATA_DIRECTORY).join("cursors").join(filename);
    let texture = gdk::Texture::from_filename(path).unwrap();
    gdk::Cursor::from_texture(&texture, hsx, hsy, None)
}
