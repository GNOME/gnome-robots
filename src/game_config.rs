/*
 * Copyright 2020-2026 Andrey Kutejko <andy128k@gmail.com>
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

use gettextrs::gettext;
use std::num::NonZeroU32;
use std::rc::Rc;

#[derive(PartialEq, Eq)]
pub struct GameConfig {
    pub name: &'static str,
    pub display_name: String,
    pub robot_type1: RobotTypeConfig,
    pub robot_type2: RobotTypeConfig,
    pub safe_teleports: Option<SafeTeleportsConfig>,
    pub moveable_heaps: bool,
}

#[derive(PartialEq, Eq)]
pub struct RobotTypeConfig {
    pub initial: u32,
    pub increment: u32,
    pub maximum: u32,
    pub score: u32,
    pub score_waiting: u32,
    pub score_splatted: u32,
}

#[derive(PartialEq, Eq)]
pub struct SafeTeleportsConfig {
    pub initial: u32,
    pub free: u32,
    pub max: u32,
    pub kill_reward_price: Option<NonZeroU32>,
}

pub struct Reward {
    pub price: u32,
    pub safe_teleports_reward: u32,
}

impl RobotTypeConfig {
    pub fn robots_on_level(&self, level: u32) -> u32 {
        (self.initial + self.increment * level).clamp(0, self.maximum)
    }
}

impl SafeTeleportsConfig {
    pub fn free(&self, current: u32) -> u32 {
        u32::min(self.free, self.max.saturating_sub(current))
    }

    pub fn kills_reward(&self, current: u32, kills: u32) -> Option<Reward> {
        let kill_reward_price: u32 = self.kill_reward_price?.into();
        let safe_teleports_reward =
            u32::min(kills / kill_reward_price, self.max.saturating_sub(current));
        (safe_teleports_reward > 0).then_some({
            Reward {
                price: safe_teleports_reward * kill_reward_price,
                safe_teleports_reward,
            }
        })
    }
}

fn classic_robots() -> GameConfig {
    GameConfig {
        name: "classic_robots",
        display_name: gettext("Classic robots"),
        robot_type1: RobotTypeConfig {
            initial: 10,
            increment: 10,
            maximum: 9999,
            score: 10,
            score_waiting: 10,
            score_splatted: 10,
        },
        robot_type2: RobotTypeConfig {
            initial: 0,
            increment: 0,
            maximum: 0,
            score: 0,
            score_waiting: 0,
            score_splatted: 0,
        },
        safe_teleports: None,
        moveable_heaps: false,
    }
}

fn robots_with_safe_teleport() -> GameConfig {
    GameConfig {
        name: "robots_with_safe_teleport",
        display_name: gettext("Robots with safe teleport"),
        safe_teleports: Some(SafeTeleportsConfig {
            initial: 0,
            free: 0,
            max: 10,
            kill_reward_price: NonZeroU32::new(1),
        }),
        ..classic_robots()
    }
}

fn robots2() -> GameConfig {
    GameConfig {
        name: "robots2",
        display_name: gettext("Robots2"),
        robot_type1: RobotTypeConfig {
            initial: 8,
            increment: 8,
            maximum: 9999,
            score: 10,
            score_waiting: 10,
            score_splatted: 20,
        },
        robot_type2: RobotTypeConfig {
            initial: 2,
            increment: 2,
            maximum: 9999,
            score: 20,
            score_waiting: 20,
            score_splatted: 40,
        },
        safe_teleports: Some(SafeTeleportsConfig {
            initial: 1,
            free: 0,
            max: 10,
            kill_reward_price: NonZeroU32::new(1),
        }),
        moveable_heaps: true,
    }
}

fn robots2_easy() -> GameConfig {
    let robots2 = robots2();
    GameConfig {
        name: "robots2_easy",
        display_name: gettext("Robots2 easy"),
        safe_teleports: Some(SafeTeleportsConfig {
            initial: 2,
            free: 1,
            ..robots2.safe_teleports.unwrap()
        }),
        ..robots2
    }
}

fn nightmare() -> GameConfig {
    GameConfig {
        name: "nightmare",
        display_name: gettext("Nightmare"),
        robot_type1: RobotTypeConfig {
            initial: 2,
            increment: 2,
            maximum: 9999,
            score: 10,
            score_waiting: 10,
            score_splatted: 20,
        },
        robot_type2: RobotTypeConfig {
            initial: 8,
            increment: 8,
            maximum: 9999,
            score: 20,
            score_waiting: 20,
            score_splatted: 40,
        },
        safe_teleports: Some(SafeTeleportsConfig {
            initial: 1,
            free: 1,
            max: 10,
            kill_reward_price: NonZeroU32::new(2),
        }),
        moveable_heaps: true,
    }
}

#[derive(Clone)]
pub struct GameConfigs {
    pub game_configs: Vec<Rc<GameConfig>>,
}

impl GameConfigs {
    pub fn new() -> Self {
        Self {
            game_configs: vec![
                Rc::new(classic_robots()),
                Rc::new(robots_with_safe_teleport()),
                Rc::new(robots2()),
                Rc::new(robots2_easy()),
                Rc::new(nightmare()),
            ],
        }
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
