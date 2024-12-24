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

use super::new_score_dialog::new_score_dialog;
use super::scores_dialog::scores_dialog;
use super::ROWS_TO_DISPLAY;
use crate::config::PACKAGE;
use crate::game::MoveSafety;
use crate::utils::{list_directory, now};
use gettextrs::gettext;
use gtk::{glib, prelude::*};
use std::cmp::Reverse;
use std::collections::BTreeMap;
use std::fs;
use std::io::{self, BufRead, Write};
use std::path::{Path, PathBuf};

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Debug)]
pub struct Category {
    pub key: String,
    pub safety: MoveSafety,
}

impl Category {
    fn from_game_type(game_type: &str) -> Self {
        if let Some(key) = game_type.strip_suffix("-super-safe") {
            Self {
                key: key.to_owned(),
                safety: MoveSafety::SuperSafe,
            }
        } else if let Some(key) = game_type.strip_suffix("-safe") {
            Self {
                key: key.to_owned(),
                safety: MoveSafety::Safe,
            }
        } else {
            Self {
                key: game_type.to_owned(),
                safety: MoveSafety::Unsafe,
            }
        }
    }

    fn game_type(&self) -> String {
        match self.safety {
            MoveSafety::Unsafe => self.key.replace(' ', "_").clone(),
            MoveSafety::Safe => format!("{}-safe", self.key.replace(' ', "_")),
            MoveSafety::SuperSafe => format!("{}-super-safe", self.key.replace(' ', "_")),
        }
    }

    pub fn name(&self) -> Option<String> {
        match (self.key.as_str(), self.safety) {
            ("classic_robots", MoveSafety::Unsafe) => Some(gettext("Classic robots")),
            ("classic_robots", MoveSafety::Safe) => Some(gettext("Classic robots with safe moves")),
            ("classic_robots", MoveSafety::SuperSafe) => {
                Some(gettext("Classic robots with super-safe moves"))
            }
            ("nightmare", MoveSafety::Unsafe) => Some(gettext("Nightmare")),
            ("nightmare", MoveSafety::Safe) => Some(gettext("Nightmare with safe moves")),
            ("nightmare", MoveSafety::SuperSafe) => {
                Some(gettext("Nightmare with super-safe moves"))
            }
            ("robots2", MoveSafety::Unsafe) => Some(gettext("Robots2")),
            ("robots2", MoveSafety::Safe) => Some(gettext("Robots2 with safe moves")),
            ("robots2", MoveSafety::SuperSafe) => Some(gettext("Robots2 with super-safe moves")),
            ("robots2_easy", MoveSafety::Unsafe) => Some(gettext("Robots2 easy")),
            ("robots2_easy", MoveSafety::Safe) => Some(gettext("Robots2 easy with safe moves")),
            ("robots2_easy", MoveSafety::SuperSafe) => {
                Some(gettext("Robots2 easy with super-safe moves"))
            }
            ("robots_with_safe_teleport", MoveSafety::Unsafe) => {
                Some(gettext("Robots with safe teleport"))
            }
            ("robots_with_safe_teleport", MoveSafety::Safe) => {
                Some(gettext("Robots with safe teleport with safe moves"))
            }
            ("robots_with_safe_teleport", MoveSafety::SuperSafe) => {
                Some(gettext("Robots with safe teleport with super-safe moves"))
            }
            _ => None,
        }
    }
}

#[derive(PartialEq, Eq, Clone, Debug)]
pub struct Score {
    pub score: i64,
    pub time: u64,
    pub user: String,
}

impl Score {
    fn from_line(line: &str, fallback_user: &str) -> Option<Self> {
        let mut tokens = line.splitn(3, ' ');

        let score = tokens.next()?.parse().ok()?;
        let time = tokens.next()?.parse().ok()?;
        let user = tokens.next().unwrap_or(fallback_user).to_owned();

        Some(Self { score, time, user })
    }

    fn to_line(&self) -> String {
        format!("{} {} {}", self.score, self.time, self.user)
    }

    fn rename(self, user: String) -> Self {
        Self { user, ..self }
    }
}

fn save_score_to_file(scores_dir: &Path, category: &Category, score: &Score) -> io::Result<()> {
    fs::create_dir_all(scores_dir)?;

    let filename = scores_dir.join(category.game_type());
    let mut file = fs::OpenOptions::new()
        .create(true)
        .append(true)
        .open(&filename)?;
    writeln!(file, "{}", score.to_line())?;
    Ok(())
}

#[derive(Default)]
pub struct ScoreList {
    pub scores: Vec<Score>,
}

impl ScoreList {
    pub fn load(path: &Path, fallback_user: &str) -> io::Result<Self> {
        let mut scores = Vec::new();
        let file = fs::OpenOptions::new().read(true).open(path)?;
        for line in io::BufReader::new(file).lines() {
            let line = line?;
            match Score::from_line(&line, fallback_user) {
                Some(score) => scores.push(score),
                None => {
                    eprintln!(
                        "Failed to read malformed score {} in {}.",
                        line,
                        path.display()
                    )
                }
            }
        }
        scores.sort_by_key(|s| Reverse(s.score));
        Ok(Self { scores })
    }

    pub fn high_scores(&self, n: usize) -> Vec<&Score> {
        self.scores.iter().take(n).collect()
    }

    pub fn add(&mut self, score: Score) {
        self.scores.push(score);
        self.scores.sort_by_key(|s| Reverse(s.score));
    }
}

#[derive(Default)]
pub struct Scores {
    pub scores: BTreeMap<Category, ScoreList>,
}

impl Scores {
    fn load(dir: &Path) -> io::Result<Self> {
        let mut scores = BTreeMap::new();

        let user = glib::real_name();
        let user = user.to_string_lossy();

        let files = list_directory(dir)?;
        for file in files {
            let Some(file_name) = file.file_name().and_then(|s| s.to_str()) else {
                continue;
            };

            let category = Category::from_game_type(file_name);

            if category.name().is_none() {
                continue;
            }

            let scores_of_single_category = ScoreList::load(&file, &user)?;

            scores.insert(category, scores_of_single_category);
        }

        Ok(Self { scores })
    }
}

fn scores_dir() -> PathBuf {
    glib::user_data_dir().join(PACKAGE).join("scores")
}

pub fn show_scores(category: Option<&Category>, parent_window: &impl IsA<gtk::Window>) {
    let scores_dir = scores_dir();
    let scores = Scores::load(&scores_dir).unwrap_or_else(|e| {
        eprintln!("Failed to load scores: {}", e);
        Default::default()
    });
    let dialog = scores_dialog(scores, category, parent_window.upcast_ref());
    dialog.present();
}

pub async fn add_score(category: &Category, score: i64, parent_window: &gtk::Window) {
    if category.name().is_none() {
        eprintln!("Failed to add score for unknown game '{:?}'.", category);
    };

    let user = glib::real_name();
    let user = user.to_string_lossy();

    let score = Score {
        score,
        time: now(),
        user: user.to_string(),
    };

    let scores_dir = scores_dir();
    let mut score_list = ScoreList::load(&scores_dir.join(category.game_type()), &user)
        .map_err(|err| {
            eprintln!("Failed to read scores for {}: {}", category.key, err);
            err
        })
        .unwrap_or_default();

    let high_score_gained = {
        let best_scores = score_list.high_scores(ROWS_TO_DISPLAY);
        best_scores.len() < ROWS_TO_DISPLAY || score.score > best_scores.last().unwrap().score
    };

    if high_score_gained {
        score_list.add(score.clone());

        let new_name = new_score_dialog(&score, category, &score_list, parent_window).await;
        if let Err(error) = save_score_to_file(&scores_dir, category, &score.rename(new_name)) {
            eprint!("Failed to save scores for {}: {}", category.key, error);
        }
    }
}
