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

use crate::bubble::Bubble;
use crate::cursors::make_cursors;
use crate::themes::themes_from_directory;
use gtk::{gdk, gio, prelude::*};
use std::error::Error;
use std::path::Path;

pub trait Assets {
    fn themes(&self) -> gio::ListModel;
    fn aieee_bubble(&self) -> Bubble;
    fn yahoo_bubble(&self) -> Bubble;
    fn splat_bubble(&self) -> Bubble;
    fn cursors(&self) -> Vec<gdk::Cursor>;
}

pub struct DirectoryAssets {
    themes: gio::ListStore,
    aieee_bubble: Bubble,
    yahoo_bubble: Bubble,
    splat_bubble: Bubble,
    cursors: Vec<gdk::Cursor>,
}

impl DirectoryAssets {
    pub fn from_directory(directory: &Path) -> Result<Self, Box<dyn Error>> {
        let themes = themes_from_directory(&directory.join("themes"));

        let pixmaps = directory.join("pixmaps");
        let yahoo_bubble = Bubble::from_file(&pixmaps.join("yahoo.png"))?;
        let aieee_bubble = Bubble::from_file(&pixmaps.join("aieee.png"))?;
        let splat_bubble = Bubble::from_file(&pixmaps.join("splat.png"))?;

        let cursors = make_cursors();

        Ok(Self {
            themes,
            aieee_bubble,
            yahoo_bubble,
            splat_bubble,
            cursors,
        })
    }
}

impl Assets for DirectoryAssets {
    fn themes(&self) -> gio::ListModel {
        self.themes.clone().upcast()
    }

    fn aieee_bubble(&self) -> Bubble {
        self.aieee_bubble.clone()
    }

    fn yahoo_bubble(&self) -> Bubble {
        self.yahoo_bubble.clone()
    }

    fn splat_bubble(&self) -> Bubble {
        self.splat_bubble.clone()
    }

    fn cursors(&self) -> Vec<gdk::Cursor> {
        self.cursors.clone()
    }
}
