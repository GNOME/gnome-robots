/*
 * Copyright 2020-2024 Andrey Kutejko <andy128k@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General pub License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General pub License for more details.
 *
 * You should have received a copy of the GNU General pub License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * For more details see the file COPYING.
 */

use crate::{
    controls::{key_from_i32, key_into_i32},
    game::{MoveSafety, PlayerCommand},
    utils::SettingsWarnExt,
};
use gtk::{gdk, gio, gio::prelude::*};
use std::marker::PhantomData;

const BACKGROUND_COLOR: &str = "background-color";
const GAME_CONFIGURATION: &str = "configuration";
const ENABLE_SOUND: &str = "enable-sound";
const SAFE_MOVES: &str = "use-safe-moves";
const SUPER_SAFE_MOVES: &str = "use-super-safe-moves";
const THEME: &str = "theme";

#[derive(Clone, Copy)]
pub struct ControlKey {
    pub settings_key: &'static str,
    pub command: PlayerCommand,
    _seal: PhantomData<()>,
}

impl ControlKey {
    const fn new(settings_key: &'static str, command: PlayerCommand) -> Self {
        Self {
            settings_key,
            command,
            _seal: PhantomData,
        }
    }
}

pub const CONTROL_KEY_NW: ControlKey = ControlKey::new("key00", PlayerCommand::NW);
pub const CONTROL_KEY_N: ControlKey = ControlKey::new("key01", PlayerCommand::N);
pub const CONTROL_KEY_NE: ControlKey = ControlKey::new("key02", PlayerCommand::NE);
pub const CONTROL_KEY_W: ControlKey = ControlKey::new("key03", PlayerCommand::W);
pub const CONTROL_KEY_HOLD: ControlKey = ControlKey::new("key04", PlayerCommand::Stay);
pub const CONTROL_KEY_E: ControlKey = ControlKey::new("key05", PlayerCommand::E);
pub const CONTROL_KEY_SW: ControlKey = ControlKey::new("key06", PlayerCommand::SW);
pub const CONTROL_KEY_S: ControlKey = ControlKey::new("key07", PlayerCommand::S);
pub const CONTROL_KEY_SE: ControlKey = ControlKey::new("key08", PlayerCommand::SE);

pub const CONTROL_KEYS: &[&ControlKey] = &[
    &CONTROL_KEY_NW,
    &CONTROL_KEY_N,
    &CONTROL_KEY_NE,
    &CONTROL_KEY_W,
    &CONTROL_KEY_HOLD,
    &CONTROL_KEY_E,
    &CONTROL_KEY_SW,
    &CONTROL_KEY_S,
    &CONTROL_KEY_SE,
];

pub trait Properties {
    fn safe_moves(&self) -> bool;
    fn set_safe_moves(&self, value: bool);
    fn super_safe_moves(&self) -> bool;
    fn set_super_safe_moves(&self, value: bool);
    fn sound(&self) -> bool;
    fn set_sound(&self, value: bool);
    fn bgcolour(&self) -> gdk::RGBA;
    fn set_bgcolour(&self, value: gdk::RGBA);
    fn selected_config(&self) -> String;
    fn set_selected_config(&self, value: &str);
    fn theme(&self) -> String;
    fn set_theme(&self, value: &str);

    fn get_control_key(&self, k: &ControlKey) -> gdk::Key;
    fn set_control_key(&self, k: &ControlKey, key: gdk::Key);
    fn reset_control_key(&self, k: &ControlKey);
    fn reset_all_control_keys(&self) {
        self.reset_control_key(&CONTROL_KEY_NW);
        self.reset_control_key(&CONTROL_KEY_N);
        self.reset_control_key(&CONTROL_KEY_NE);
        self.reset_control_key(&CONTROL_KEY_W);
        self.reset_control_key(&CONTROL_KEY_HOLD);
        self.reset_control_key(&CONTROL_KEY_E);
        self.reset_control_key(&CONTROL_KEY_SW);
        self.reset_control_key(&CONTROL_KEY_S);
        self.reset_control_key(&CONTROL_KEY_SE);
    }

    fn move_safety(&self) -> MoveSafety {
        match (self.safe_moves(), self.super_safe_moves()) {
            (_, true) => MoveSafety::SuperSafe,
            (true, false) => MoveSafety::Safe,
            _ => MoveSafety::Unsafe,
        }
    }
}

impl Properties for gio::Settings {
    fn safe_moves(&self) -> bool {
        self.boolean(SAFE_MOVES)
    }

    fn set_safe_moves(&self, value: bool) {
        self.set_boolean_or_warn(SAFE_MOVES, value);
    }

    fn super_safe_moves(&self) -> bool {
        self.boolean(SUPER_SAFE_MOVES)
    }

    fn set_super_safe_moves(&self, value: bool) {
        self.set_boolean_or_warn(SUPER_SAFE_MOVES, value);
    }

    fn sound(&self) -> bool {
        self.boolean(ENABLE_SOUND)
    }

    fn set_sound(&self, value: bool) {
        self.set_boolean_or_warn(ENABLE_SOUND, value);
    }

    fn bgcolour(&self) -> gdk::RGBA {
        gdk::RGBA::parse(self.string(BACKGROUND_COLOR)).unwrap_or(gdk::RGBA::BLACK)
    }

    fn set_bgcolour(&self, value: gdk::RGBA) {
        self.set_string_or_warn(BACKGROUND_COLOR, &value.to_string());
    }

    fn selected_config(&self) -> String {
        self.string(GAME_CONFIGURATION).into()
    }

    fn set_selected_config(&self, value: &str) {
        self.set_string_or_warn(GAME_CONFIGURATION, value);
    }

    fn theme(&self) -> String {
        self.string(THEME).into()
    }

    fn set_theme(&self, value: &str) {
        self.set_string_or_warn(THEME, value);
    }

    fn get_control_key(&self, k: &ControlKey) -> gdk::Key {
        key_from_i32(self.int(k.settings_key))
    }

    fn set_control_key(&self, k: &ControlKey, key: gdk::Key) {
        self.set_int_or_warn(k.settings_key, key_into_i32(key));
    }

    fn reset_control_key(&self, k: &ControlKey) {
        self.reset(k.settings_key);
    }
}
