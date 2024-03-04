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
use regex::Regex;
use std::fs;
use std::io::{self, BufRead};
use std::path::Path;
use std::rc::Rc;
use std::sync::OnceLock;

#[derive(PartialEq, Eq)]
pub struct GameConfig {
    pub description: String,
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

fn line_regex() -> &'static Regex {
    static REGEX: OnceLock<Regex> = OnceLock::new();
    REGEX.get_or_init(|| Regex::new("^(\\w+)\\s*=\\s*(\\d+)").unwrap())
}

fn parse_line(line: &str) -> Option<(&str, u32)> {
    let captures = line_regex().captures(line)?;
    let key = captures.get(1)?.as_str();
    let val = captures.get(2)?.as_str().parse::<u32>().ok()?;
    Some((key, val))
}

pub struct Reward {
    pub price: u32,
    pub safe_teleports_reward: u32,
}

impl GameConfig {
    pub fn name(&self) -> String {
        self.description.replace("_", " ")
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

    pub fn from_file(filename: &Path) -> io::Result<Self> {
        let description = filename
            .file_stem()
            .unwrap_or_default()
            .to_string_lossy()
            .to_string();
        let mut initial_type1: Option<u32> = None;
        let mut initial_type2: Option<u32> = None;
        let mut increment_type1: Option<u32> = None;
        let mut increment_type2: Option<u32> = None;
        let mut maximum_type1: Option<u32> = None;
        let mut maximum_type2: Option<u32> = None;
        let mut score_type1: Option<u32> = None;
        let mut score_type2: Option<u32> = None;
        let mut score_type1_waiting: Option<u32> = None;
        let mut score_type2_waiting: Option<u32> = None;
        let mut score_type1_splatted: Option<u32> = None;
        let mut score_type2_splatted: Option<u32> = None;
        let mut num_robots_per_safe: Option<u32> = None;
        let mut safe_score_boundary: Option<u32> = None;
        let mut initial_safe_teleports: Option<u32> = None;
        let mut free_safe_teleports: Option<u32> = None;
        let mut max_safe_teleports: Option<u32> = None;
        let mut moveable_heaps: Option<bool> = None;

        let file = fs::OpenOptions::new().read(true).open(filename)?;
        for line in io::BufReader::new(file).lines() {
            let line = line?;
            let Some((key, val)) = parse_line(&line) else {
                continue;
            };
            match key {
                "initial_type1" => initial_type1 = Some(val),
                "initial_type2" => initial_type2 = Some(val),
                "increment_type1" => increment_type1 = Some(val),
                "increment_type2" => increment_type2 = Some(val),
                "maximum_type1" => maximum_type1 = Some(val),
                "maximum_type2" => maximum_type2 = Some(val),
                "score_type1" => score_type1 = Some(val),
                "score_type2" => score_type2 = Some(val),
                "score_type1_waiting" => score_type1_waiting = Some(val),
                "score_type2_waiting" => score_type2_waiting = Some(val),
                "score_type1_splatted" => score_type1_splatted = Some(val),
                "score_type2_splatted" => score_type2_splatted = Some(val),
                "num_robots_per_safe" => num_robots_per_safe = Some(val),
                "safe_score_boundary" => safe_score_boundary = Some(val),
                "max_safe_teleports" => max_safe_teleports = Some(val),
                "initial_safe_teleports" => initial_safe_teleports = Some(val),
                "free_safe_teleports" => free_safe_teleports = Some(val),
                "moveable_heaps" => moveable_heaps = Some(val != 0),
                _ => {}
            }
        }

        Ok(Self {
            description,
            initial_type1: initial_type1.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `initial_type1` is missing.")
            })?,
            initial_type2: initial_type2.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `initial_type2` is missing.")
            })?,
            increment_type1: increment_type1.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `increment_type1` is missing.")
            })?,
            increment_type2: increment_type2.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `increment_type2` is missing.")
            })?,
            maximum_type1: maximum_type1.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `maximum_type1` is missing.")
            })?,
            maximum_type2: maximum_type2.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `maximum_type2` is missing.")
            })?,
            score_type1: score_type1.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `score_type1` is missing.")
            })?,
            score_type2: score_type2.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `score_type2` is missing.")
            })?,
            score_type1_waiting: score_type1_waiting.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `score_type1_waiting` is missing.")
            })?,
            score_type2_waiting: score_type2_waiting.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `score_type2_waiting` is missing.")
            })?,
            score_type1_splatted: score_type1_splatted.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `score_type1_splatted` is missing.")
            })?,
            score_type2_splatted: score_type2_splatted.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `score_type2_splatted` is missing.")
            })?,
            num_robots_per_safe: num_robots_per_safe.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `num_robots_per_safe` is missing.")
            })?,
            safe_score_boundary: safe_score_boundary.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `safe_score_boundary` is missing.")
            })?,
            initial_safe_teleports: initial_safe_teleports.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `initial_safe_teleports` is missing.")
            })?,
            free_safe_teleports: free_safe_teleports.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `free_safe_teleports` is missing.")
            })?,
            max_safe_teleports: max_safe_teleports.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `max_safe_teleports` is missing.")
            })?,
            moveable_heaps: moveable_heaps.ok_or_else(|| {
                io::Error::other("Bad game config file. Value `moveable_heaps` is missing.")
            })?,
        })
    }
}

#[derive(Clone)]
pub struct GameConfigs {
    pub game_configs: Vec<Rc<GameConfig>>,
}

impl GameConfigs {
    pub fn load() -> io::Result<Self> {
        let directory = Path::new(DATA_DIRECTORY).join("games");
        let mut game_configs = Vec::new();
        for entry in fs::read_dir(&directory)? {
            let path = entry?.path();
            if path.extension().map_or(false, |ex| ex == "cfg") {
                let gcfg = GameConfig::from_file(&path)?;
                game_configs.push(Rc::new(gcfg));
            }
        }

        if game_configs.is_empty() {
            return Err(io::Error::other("No game config was found."));
        }
        Ok(Self { game_configs })
    }

    pub fn find_by_name(&self, name: &str) -> Option<&Rc<GameConfig>> {
        self.game_configs
            .iter()
            .find(|gc| gc.description == name || gc.name() == name)
    }

    pub fn best_match(&self, name: &str) -> &Rc<GameConfig> {
        self.find_by_name(name)
            .unwrap_or_else(|| &self.game_configs[0])
    }
}
