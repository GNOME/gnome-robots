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
use gtk::{gdk, prelude::*};
use std::path::Path;

#[derive(Clone, Copy)]
pub enum Sound {
    Victory,
    Die,
    Teleport,
    Splat,
    Yahoo,
    Bad,
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
        let path = Path::new(SOUND_DIRECTORY);
        Self {
            victory: gtk::MediaFile::for_filename(path.join("victory.ogg")),
            die: gtk::MediaFile::for_filename(path.join("die.ogg")),
            teleport: gtk::MediaFile::for_filename(path.join("teleport.ogg")),
            splat: gtk::MediaFile::for_filename(path.join("splat.ogg")),
            yahoo: gtk::MediaFile::for_filename(path.join("yahoo.ogg")),
        }
    }

    pub fn beep(&self) {
        if let Some(display) = gdk::Display::default() {
            display.beep();
        }
    }

    pub fn play(&self, sound: Sound) {
        match sound {
            Sound::Victory => self.victory.play(),
            Sound::Die => self.die.play(),
            Sound::Teleport => self.teleport.play(),
            Sound::Splat => self.splat.play(),
            Sound::Yahoo => self.yahoo.play(),
            Sound::Bad => self.beep(),
        }
    }
}
