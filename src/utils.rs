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

use gtk::{gio, glib, prelude::*};
use std::cell::RefCell;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};

pub fn list_directory(directory: &Path) -> io::Result<Vec<PathBuf>> {
    let mut files = Vec::new();
    for entry in fs::read_dir(directory)? {
        files.push(entry?.path());
    }
    Ok(files)
}

pub fn now() -> u64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_or(0, |t| t.as_secs())
}

pub trait SettingsWarnExt {
    fn set_string_or_warn(&self, key: &str, value: &str);
    fn set_int_or_warn(&self, key: &str, value: i32);
    fn set_boolean_or_warn(&self, key: &str, value: bool);
}

fn warn_failed_set(key: &str, error: &dyn std::error::Error) {
    glib::g_warning!(None, "Failed to save setting key {}: {}", key, error);
}

impl SettingsWarnExt for gio::Settings {
    fn set_string_or_warn(&self, key: &str, value: &str) {
        if let Err(ref error) = self.set_string(key, value) {
            warn_failed_set(key, error);
        }
    }

    fn set_int_or_warn(&self, key: &str, value: i32) {
        if let Err(ref error) = self.set_int(key, value) {
            warn_failed_set(key, error);
        }
    }

    fn set_boolean_or_warn(&self, key: &str, value: bool) {
        if let Err(ref error) = self.set_boolean(key, value) {
            warn_failed_set(key, error);
        }
    }
}

pub fn settings_aware_widget<W, F>(
    widget: &W,
    settings: &gio::Settings,
    settings_key: Option<&str>,
    f: F,
) where
    W: IsA<gtk::Widget>,
    W: glib::clone::Downgrade,
    <W as glib::clone::Downgrade>::Weak: glib::clone::Upgrade<Strong = W>,
    F: Fn(&W, &gio::Settings) + 'static,
{
    let signal_handler_id = settings.connect_changed(
        settings_key,
        glib::clone!(
            #[weak]
            widget,
            move |s, _| (f)(&widget, s)
        ),
    );
    widget.connect_destroy({
        let signal_handler_id = RefCell::new(Some(signal_handler_id));
        let settings = settings.clone();
        move |_| {
            if let Some(signal_handler_id) = signal_handler_id.take() {
                settings.disconnect(signal_handler_id);
            }
        }
    });
}
