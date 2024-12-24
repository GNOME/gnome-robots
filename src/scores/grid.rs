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

use super::{scores::ScoreList, ROWS_TO_DISPLAY};
use gettextrs::gettext;
use gtk::prelude::*;

pub fn create_scores_grid() -> gtk::Grid {
    let grid = gtk::Grid::builder()
        .row_homogeneous(true)
        .column_spacing(40)
        .margin_start(30)
        .margin_end(30)
        .margin_bottom(20)
        .halign(gtk::Align::Center)
        .baseline_row(0)
        .build();

    grid.attach(&title_label(&gettext("Rank")), 0, 0, 1, 1);
    grid.attach(&title_label(&gettext("Score")), 1, 0, 1, 1);
    grid.attach(&title_label(&gettext("Player")), 2, 0, 1, 1);

    for row in 1..=ROWS_TO_DISPLAY {
        grid.attach(
            &gtk::Label::builder()
                .halign(gtk::Align::Center)
                .valign(gtk::Align::Center)
                .build(),
            0,
            row as i32,
            1,
            1,
        );
        grid.attach(
            &gtk::Label::builder()
                .halign(gtk::Align::Center)
                .valign(gtk::Align::Center)
                .build(),
            1,
            row as i32,
            1,
            1,
        );
        grid.attach(
            &gtk::Label::builder()
                .justify(gtk::Justification::Center)
                .valign(gtk::Align::Center)
                .build(),
            2,
            row as i32,
            1,
            1,
        );
    }

    grid
}

pub fn scores_grid_update(grid: &gtk::Grid, score_list: &ScoreList) {
    for row in 1..=ROWS_TO_DISPLAY {
        let rank_label = grid
            .child_at(0, row as i32)
            .and_downcast::<gtk::Label>()
            .unwrap();
        let score_label = grid
            .child_at(1, row as i32)
            .and_downcast::<gtk::Label>()
            .unwrap();
        let user_label = grid
            .child_at(2, row as i32)
            .and_downcast::<gtk::Label>()
            .unwrap();

        match score_list.scores.get(row - 1) {
            Some(score) => {
                rank_label.set_text(&row.to_string());
                score_label.set_text(&score.score.to_string());
                user_label.set_text(&score.user);
            }
            None => {
                rank_label.set_text("");
                score_label.set_text("");
                user_label.set_text("");
            }
        }
    }
}

fn title_label(label: &str) -> gtk::Label {
    gtk::Label::builder()
        .label(format!("<b>{}</b>", label))
        .use_markup(true)
        .build()
}
