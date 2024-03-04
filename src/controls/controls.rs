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

use super::{editor::ControlEditor, key_from_i32, key_into_i32};
use crate::{
    properties::{Properties, *},
    utils::settings_aware_widget,
};
use adw::prelude::*;
use gtk::{gio, glib};

fn g_(str: &str) -> String {
    str.to_owned()
}

pub fn create_controls_list(settings: &gio::Settings) -> adw::PreferencesGroup {
    let pg = adw::PreferencesGroup::builder()
        .title(&g_("Keyboard"))
        .build();

    for (key, label) in [
        (CONTROL_KEY_NW, g_("Key to move NW")),
        (CONTROL_KEY_N, g_("Key to move N")),
        (CONTROL_KEY_NE, g_("Key to move NE")),
        (CONTROL_KEY_W, g_("Key to move W")),
        (CONTROL_KEY_HOLD, g_("Key to hold")),
        (CONTROL_KEY_E, g_("Key to move E")),
        (CONTROL_KEY_SW, g_("Key to move SW")),
        (CONTROL_KEY_S, g_("Key to move S")),
        (CONTROL_KEY_SE, g_("Key to move SE")),
    ] {
        let editor = new_editor(&settings, key);

        let action_row = adw::ActionRow::builder()
            .title(&label)
            .activatable_widget(&editor)
            .build();
        action_row.add_suffix(&editor);
        pg.add(&action_row);
    }

    let dbut = gtk::Button::builder()
        .label(&g_("_Restore Defaults"))
        .use_underline(true)
        .build();
    dbut.connect_clicked(glib::clone!(@weak settings => move |_| {
        settings.reset_all_control_keys();
    }));
    pg.set_header_suffix(Some(&dbut));

    pg
}

fn new_editor(settings: &gio::Settings, control_key: ControlKey) -> ControlEditor {
    let editor = ControlEditor::default();

    editor.set_key(settings.get_control_key(&control_key));

    editor.connect_edited(glib::clone!(@strong settings => move |keyval| {
        settings.set_control_key(&control_key, keyval);
    }));
    editor.connect_cleared(glib::clone!(@strong settings => move || {
        settings.reset_control_key(&control_key);
        settings.get_control_key(&control_key)
    }));
    settings_aware_widget(
        &editor,
        settings,
        Some(control_key.settings_key),
        move |e, s| {
            e.set_key(s.get_control_key(&control_key));
        },
    );
    editor
}
