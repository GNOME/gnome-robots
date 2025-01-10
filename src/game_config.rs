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
use gettextrs::gettext;
use std::error::Error;
use std::fs;
use std::path::Path;
use std::rc::Rc;

#[derive(PartialEq, Eq, serde::Deserialize)]
pub struct GameConfig {
    pub name: String,
    pub initial_type1: u32,
    pub initial_type2: u32,
    pub increment_type1: u32,
    pub increment_type2: u32,
    pub maximum_type1: u32,
    pub maximum_type2: u32,
    pub score_type1: u32,
    pub score_type2: u32,
    pub score_type1_waiting: u32,
    pub score_type2_waiting: u32,
    pub score_type1_splatted: u32,
    pub score_type2_splatted: u32,
    pub num_robots_per_safe: u32,
    pub safe_score_boundary: u32,
    pub initial_safe_teleports: u32,
    pub free_safe_teleports: u32,
    pub max_safe_teleports: u32,
    pub moveable_heaps: bool,
}

pub struct Reward {
    pub price: u32,
    pub safe_teleports_reward: u32,
}

impl GameConfig {
    pub fn display_name(&self) -> String {
        match self.name.as_str() {
            "classic_robots" => gettext("Classic robots"),
            "nightmare" => gettext("Nightmare"),
            "robots2" => gettext("Robots2"),
            "robots2_easy" => gettext("Robots2 easy"),
            "robots_with_safe_teleport" => gettext("Robots with safe teleport"),
            _ => self.name.replace("_", " "),
        }
    }

    pub fn type1_robots_on_level(&self, level: u32) -> u32 {
        (self.initial_type1 + self.increment_type1 * level).clamp(0, self.maximum_type1)
    }

    pub fn type2_robots_on_level(&self, level: u32) -> u32 {
        (self.initial_type2 + self.increment_type2 * level).clamp(0, self.maximum_type2)
    }

    pub fn score_reward(&self, score: u32) -> Reward {
        if self.safe_score_boundary > 0 {
            let safe_teleports_reward = score / self.safe_score_boundary;
            let price = safe_teleports_reward * self.safe_score_boundary;
            Reward {
                price,
                safe_teleports_reward,
            }
        } else {
            Reward {
                price: 0,
                safe_teleports_reward: 0,
            }
        }
    }

    pub fn kills_reward(&self, kills: u32) -> Reward {
        if self.num_robots_per_safe > 0 {
            let safe_teleports_reward = kills / self.num_robots_per_safe;
            let price = safe_teleports_reward * self.num_robots_per_safe;
            Reward {
                price,
                safe_teleports_reward,
            }
        } else {
            Reward {
                price: 0,
                safe_teleports_reward: 0,
            }
        }
    }

    pub fn from_file(filename: &Path) -> Result<Self, Box<dyn Error>> {
        let content = fs::read_to_string(filename)?;
        let config: Self = toml::from_str(&content)?;
        Ok(config)
    }
}

#[derive(Clone)]
pub struct GameConfigs {
    pub game_configs: Vec<Rc<GameConfig>>,
}

impl GameConfigs {
    pub fn load() -> Result<Self, Box<dyn Error>> {
        let directory = Path::new(DATA_DIRECTORY).join("games");
        let mut game_configs = Vec::new();
        for entry in fs::read_dir(&directory)? {
            let path = entry?.path();
            if path
                .extension()
                .is_some_and(|ex| ex == "cfg" || ex == "toml")
            {
                let gcfg = GameConfig::from_file(&path)?;
                game_configs.push(Rc::new(gcfg));
            }
        }

        if game_configs.is_empty() {
            return Err("No game config was found.".into());
        }
        Ok(Self { game_configs })
    }

    pub fn find_by_name(&self, name: &str) -> Option<&Rc<GameConfig>> {
        self.game_configs
            .iter()
            .find(|gc| gc.name == name || gc.name.replace('_', " ") == name)
    }

    pub fn best_match(&self, name: &str) -> &Rc<GameConfig> {
        self.find_by_name(name)
            .unwrap_or_else(|| &self.game_configs[0])
    }
}
