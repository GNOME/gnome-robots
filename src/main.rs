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

use application::RobotsApplication;
use config::{GETTEXT_PACKAGE, LOCALEDIR};
use gtk::prelude::*;
use std::error::Error;
use std::process::Termination;

mod animation;
mod application;
mod arena;
mod assets;
mod bubble;
mod config;
mod controls;
mod cursors;
mod game;
mod game_area;
mod game_config;
mod graphics;
mod image;
mod properties;
mod properties_dialog;
mod scores;
mod slot;
mod sound_player;
mod svg_paintable;
mod theme;
mod themes;
mod utils;
mod window;
mod window_size;

fn main() -> Result<impl Termination, Box<dyn Error>> {
    gettextrs::setlocale(gettextrs::LocaleCategory::LcAll, "");
    gettextrs::bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR)?;
    gettextrs::bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8")?;
    gettextrs::textdomain(GETTEXT_PACKAGE)?;

    let app = RobotsApplication::default();
    Ok(app.run())
}
