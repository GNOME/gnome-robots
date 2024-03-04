use super::{scores::ScoreList, ROWS_TO_DISPLAY};
use gtk::prelude::*;

fn g_(str: &str) -> String {
    str.to_owned()
}

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

    grid.attach(&title_label(&g_("Rank")), 0, 0, 1, 1);
    grid.attach(&title_label(&g_("Score")), 1, 0, 1, 1);
    grid.attach(&title_label(&g_("Player")), 2, 0, 1, 1);

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

        match score_list.scores.get(row) {
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
    let label = gtk::Label::builder().label(label).use_markup(true).build();
    label.add_css_class("bold"); // TODO: add CSS class
    label
}
