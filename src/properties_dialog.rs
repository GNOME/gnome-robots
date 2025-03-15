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

use crate::{
    controls::controls::create_controls_list,
    game_config::{GameConfig, GameConfigs},
    properties::Properties,
    theme::Theme,
    themes,
};
use adw::prelude::*;
use gettextrs::gettext;
use gtk::{gio, glib};
use std::rc::Rc;

pub fn show_preferences(
    parent_window: Option<&gtk::Window>,
    game_configs: &GameConfigs,
    themes: &gio::ListModel,
    settings: &gio::Settings,
) {
    let window = adw::PreferencesDialog::builder().build();

    /* The configuration page */
    let configuration_page = adw::PreferencesPage::builder()
        .title(gettext("Game"))
        .icon_name("gamepad2")
        .build();
    configuration_page.add(&create_game_group(game_configs, settings));
    configuration_page.add(&create_sound_group(settings));
    window.add(&configuration_page);

    /* The graphics page */
    let graphics_page = adw::PreferencesPage::builder()
        .title(gettext("Appearance"))
        .icon_name("brush-monitor")
        .build();
    let appearance_group = adw::PreferencesGroup::builder().build();
    appearance_group.add(&create_theme_picker(themes, settings));
    appearance_group.add(&create_background_picker(settings));
    graphics_page.add(&appearance_group);
    window.add(&graphics_page);

    /* The keyboard page */
    let keyboard_page = adw::PreferencesPage::builder()
        .title(gettext("Keyboard"))
        .icon_name("keyboard-shortcuts")
        .build();
    keyboard_page.add(&create_controls_list(settings));
    window.add(&keyboard_page);

    window.present(parent_window);
}

fn create_game_group(
    game_configs: &GameConfigs,
    properties: &gio::Settings,
) -> adw::PreferencesGroup {
    let group = adw::PreferencesGroup::new();

    let typemenu = create_game_config_picker(game_configs, &properties.selected_config());
    typemenu.connect_selected_item_notify(glib::clone!(
        #[weak]
        properties,
        move |cr| {
            if let Some(selection) = cr
                .selected_item()
                .and_downcast::<glib::BoxedAnyObject>()
                .map(|b| b.borrow::<Rc<GameConfig>>().clone())
            {
                properties.set_selected_config(&selection.name);
            }
        }
    ));
    group.add(&typemenu);

    let safe_chkbox = adw::SwitchRow::builder()
        .title(gettext("_Use safe moves"))
        .use_underline(true)
        .subtitle(gettext(
            "Prevent accidental moves that result in getting killed.",
        ))
        .active(properties.safe_moves())
        .build();
    group.add(&safe_chkbox);

    let super_safe_chkbox = adw::SwitchRow::builder()
        .title(gettext("U_se super safe moves"))
        .use_underline(true)
        .subtitle(gettext("Prevents all moves that result in getting killed."))
        .active(properties.super_safe_moves())
        .sensitive(properties.safe_moves())
        .build();
    group.add(&super_safe_chkbox);

    super_safe_chkbox.connect_active_notify(glib::clone!(
        #[weak]
        properties,
        move |cb| {
            properties.set_super_safe_moves(cb.is_active());
        }
    ));
    safe_chkbox.connect_active_notify(glib::clone!(
        #[weak]
        properties,
        move |cb| {
            properties.set_safe_moves(cb.is_active());
            super_safe_chkbox.set_sensitive(properties.safe_moves());
        }
    ));

    group
}

fn create_sound_group(settings: &gio::Settings) -> adw::PreferencesGroup {
    let group = adw::PreferencesGroup::new();

    let sound_chkbox = adw::SwitchRow::builder()
        .title(gettext("_Enable sounds"))
        .use_underline(true)
        .subtitle(gettext(
            "Play sounds for events like winning a level and dying.",
        ))
        .active(settings.sound())
        .build();
    sound_chkbox.connect_active_notify(glib::clone!(
        #[weak]
        settings,
        move |cb| {
            settings.set_sound(cb.is_active());
        }
    ));
    group.add(&sound_chkbox);

    group
}

fn create_game_config_picker(game_configs: &GameConfigs, current_config: &str) -> adw::ComboRow {
    let model = gio::ListStore::new::<glib::BoxedAnyObject>();
    let mut active_index = 0;
    for (i, config) in game_configs.game_configs.iter().enumerate() {
        model.append(&glib::BoxedAnyObject::new(config.clone()));
        if config.name == current_config {
            active_index = i as u32;
        }
    }

    adw::ComboRow::builder()
        .title(gettext("Game Type"))
        .model(&model)
        .expression(gtk::ClosureExpression::new::<String>(
            &[] as &[gtk::Expression],
            glib::closure!(|item: &glib::Object| {
                item.downcast_ref::<glib::BoxedAnyObject>()
                    .map(|b| b.borrow::<Rc<GameConfig>>().display_name())
                    .unwrap_or_default()
            }),
        ))
        .selected(active_index)
        .build()
}

fn create_theme_picker(themes: &gio::ListModel, settings: &gio::Settings) -> adw::ComboRow {
    let index = themes::find_by_name(themes, &settings.theme())
        .map(|r| r.0)
        .unwrap_or_default();

    let theme_picker = adw::ComboRow::builder()
        .title(gettext("_Image theme:"))
        .use_underline(true)
        .model(themes)
        .selected(index)
        .expression(gtk::PropertyExpression::new(
            Theme::static_type(),
            gtk::Expression::NONE,
            "name",
        ))
        .build();

    theme_picker.connect_selected_item_notify(glib::clone!(
        #[weak]
        settings,
        move |tp| {
            if let Some(theme) = tp.selected_item().and_downcast::<Theme>() {
                settings.set_theme(&theme.name());
            }
        }
    ));

    theme_picker
}

fn create_background_picker(settings: &gio::Settings) -> adw::ActionRow {
    let color_dialog = gtk::ColorDialog::new();

    let button = gtk::ColorDialogButton::builder()
        .dialog(&color_dialog)
        .rgba(&settings.bgcolour())
        .build();
    button.connect_rgba_notify(glib::clone!(
        #[weak]
        settings,
        move |b| {
            settings.set_bgcolour(b.rgba());
        }
    ));

    let color_row = adw::ActionRow::builder()
        .title(gettext("_Background color:"))
        .use_underline(true)
        .build();
    color_row.add_suffix(&button);

    color_row
}
