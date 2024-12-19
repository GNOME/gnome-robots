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

use super::category_bar::category_bar;
use super::dialog::{new_scores_dialog, ScoresDialog};
use super::grid::{create_scores_grid, scores_grid_update};
use super::scores::{Category, Score, ScoreList};
use super::ROWS_TO_DISPLAY;
use gettextrs::gettext;
use gtk::{glib, prelude::*};
use std::cell::RefCell;
use std::rc::Rc;

pub async fn new_score_dialog(
    new_high_score: &Score,
    current_cat: &Category,
    score_list: &ScoreList,
    window: &gtk::Window,
) -> String {
    let ScoresDialog {
        dlg,
        window_title,
        headerbar,
        vbox,
    } = new_scores_dialog(window);

    window_title.set_title(&gettext("Congratulations!"));
    headerbar.set_show_end_title_buttons(false);

    vbox.set_spacing(20);

    let category_label = gtk::Label::builder()
        .label(current_cat.name().unwrap())
        .halign(gtk::Align::Center)
        .valign(gtk::Align::Center)
        .build();

    vbox.append(&category_bar(&category_label));
    vbox.append(
        &gtk::Separator::builder()
            .orientation(gtk::Orientation::Horizontal)
            .build(),
    );

    let grid = create_scores_grid();
    vbox.append(&grid);

    let done = gtk::Button::builder()
        .label(gettext("_Done"))
        .use_underline(true)
        .build();
    done.add_css_class("suggested-action");
    done.connect_clicked(glib::clone!(
        #[weak]
        dlg,
        move |_| dlg.close()
    ));

    headerbar.set_show_start_title_buttons(false);
    headerbar.set_show_end_title_buttons(false);
    headerbar.pack_end(&done);

    scores_grid_update(&grid, score_list);

    let user = Rc::new(RefCell::new(new_high_score.user.clone()));

    let count = score_list.scores.len();
    for (row_count, score) in score_list.scores.iter().enumerate().take(ROWS_TO_DISPLAY) {
        if score == new_high_score {
            if count > 1 && row_count == 0 {
                window_title.set_subtitle(&gettext("Your score is the best!"));
            } else {
                window_title.set_subtitle(&gettext("Your score has made the top ten."));
            }

            let entry = gtk::Entry::builder().text(&score.user).build(); // 20x20
            entry.connect_text_notify({
                let user = user.clone();
                move |e| {
                    *user.borrow_mut() = e.text().into();
                }
            });
            grid.attach(&entry, 2, row_count as i32 + 1, 1, 1);
        }
    }

    let (sender, receiver) = async_channel::unbounded();
    dlg.connect_close_request(move |_| {
        sender.send_blocking(()).unwrap();
        glib::Propagation::Proceed
    });
    dlg.present();

    receiver.recv().await.unwrap();

    let user = user.borrow_mut().clone(); // Rc::unwrap_or_clone()
    user
}
