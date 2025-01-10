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

use crate::theme::Theme;
use crate::utils::list_directory;
use gtk::{gio, gio::prelude::*};
use std::path::Path;

pub fn themes_from_directory(directory: &Path) -> gio::ListStore {
    let files = list_directory(directory).unwrap_or_else(|err| {
        eprintln!(
            "Failed to list theme directory {}: {}",
            directory.display(),
            err
        );
        Vec::new()
    });

    let themes = gio::ListStore::new::<Theme>();
    for path in files {
        match Theme::from_file(&path) {
            Ok(theme) => {
                themes.append(&theme);
            }
            Err(error) => {
                eprintln!("Failed to load theme '{}': {}", path.display(), error)
            }
        }
    }
    themes
}

pub fn find_by_name(themes: &gio::ListModel, name: &str) -> Option<(u32, Theme)> {
    let size = themes.n_items();
    for index in 0..size {
        let Some(theme) = themes.item(index).and_downcast::<Theme>() else {
            continue;
        };
        if theme.name() == name || theme.path().file_name().is_some_and(|n| n == name) {
            return Some((index, theme));
        }
    }
    None
}

const DEFAULT_THEME: &str = "robots.svg";

pub fn find_best_match(themes: &gio::ListModel, name: &str) -> Result<Theme, &'static str> {
    if let Some((_, theme)) = find_by_name(themes, name) {
        return Ok(theme);
    }
    if let Some((_, theme)) = find_by_name(themes, DEFAULT_THEME) {
        return Ok(theme);
    }
    Err("No theme was found.")
}
