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

use crate::config::SOUND_DIRECTORY;
use gtk::{gdk, glib, prelude::*};
use std::path::Path;

#[derive(Clone, Copy)]
pub enum Sound {
    VICTORY,
    DIE,
    TELEPORT,
    SPLAT,
    YAHOO,
    BAD,
}

pub struct SoundPlayer {
    victory: gtk::MediaFile,
    die: gtk::MediaFile,
    teleport: gtk::MediaFile,
    splat: gtk::MediaFile,
    yahoo: gtk::MediaFile,
}

impl SoundPlayer {
    pub fn new() -> Self {
        Self {
            victory: new_media_file("victory.ogg"),
            die: new_media_file("die.ogg"),
            teleport: new_media_file("teleport.ogg"),
            splat: new_media_file("splat.ogg"),
            yahoo: new_media_file("yahoo.ogg"),
        }
    }

    pub fn beep(&self) {
        if let Some(display) = gdk::Display::default() {
            display.beep();
        }
    }

    pub fn play(&self, sound: Sound) {
        match sound {
            Sound::VICTORY => self.victory.play(),
            Sound::DIE => self.die.play(),
            Sound::TELEPORT => self.teleport.play(),
            Sound::SPLAT => self.splat.play(),
            Sound::YAHOO => self.yahoo.play(),
            Sound::BAD => self.beep(),
        }
    }
}

fn new_media_file(name: &str) -> gtk::MediaFile {
    let path = Path::new(SOUND_DIRECTORY).join(name);
    gtk::MediaFile::for_filename(&path)
}
