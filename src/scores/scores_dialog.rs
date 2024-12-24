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
use super::no_scores::no_scores;
use super::scores::{Category, ScoreList, Scores};
use gettextrs::gettext;
use gtk::{gio, glib, prelude::*};

struct Page {
    category: Category,
    score_list: ScoreList,
}

pub fn scores_dialog(
    scores: Scores,
    current_cat: Option<&Category>,
    window: &gtk::Window,
) -> gtk::Window {
    let ScoresDialog {
        dlg,
        window_title,
        headerbar,
        vbox,
    } = new_scores_dialog(window);

    window_title.set_title(&gettext("High Scores"));
    headerbar.set_show_end_title_buttons(true);

    if scores.scores.is_empty() {
        vbox.append(&no_scores());
        return dlg.upcast();
    }

    vbox.set_spacing(20);

    let grid = create_scores_grid();

    let category_widget: gtk::Widget = if scores.scores.len() == 1 {
        let (category, score_list) = scores.scores.into_iter().next().unwrap();

        scores_grid_update(&grid, &score_list);

        gtk::Label::builder()
            .label(category.name().unwrap())
            .halign(gtk::Align::Center)
            .valign(gtk::Align::Center)
            .build()
            .upcast()
    } else {
        let model = gio::ListStore::new::<glib::BoxedAnyObject>();
        let mut to_select = None;
        for (index, (category, score_list)) in scores.scores.into_iter().enumerate() {
            if current_cat == Some(&category) {
                to_select = Some(index as u32);
            }
            let page = glib::BoxedAnyObject::new(Page {
                category,
                score_list,
            });
            model.append(&page);
        }

        let params: &[&gtk::Expression] = &[];

        let combo = gtk::DropDown::builder()
            .focus_on_click(false)
            .model(&model)
            .expression(gtk::ClosureExpression::with_callback::<String, _>(
                params,
                |values| {
                    values[0]
                        .get::<glib::BoxedAnyObject>()
                        .ok()
                        .and_then(|item| {
                            let page = item.borrow::<Page>();
                            page.category.name()
                        })
                        .unwrap_or_default()
                },
            ))
            .build();
        combo.connect_selected_item_notify(glib::clone!(
            #[weak]
            grid,
            move |cb| {
                if let Some(selected_category) =
                    cb.selected_item().and_downcast::<glib::BoxedAnyObject>()
                {
                    scores_grid_update(&grid, &selected_category.borrow::<Page>().score_list);
                }
            }
        ));
        combo.set_selected(to_select.unwrap_or_default());
        if let Some(selected_category) = model
            .item(to_select.unwrap_or_default())
            .and_downcast::<glib::BoxedAnyObject>()
        {
            scores_grid_update(&grid, &selected_category.borrow::<Page>().score_list);
        }

        combo.upcast()
    };

    vbox.append(&category_bar(&category_widget));

    vbox.append(
        &gtk::Separator::builder()
            .orientation(gtk::Orientation::Horizontal)
            .build(),
    );

    vbox.append(&grid);

    dlg.upcast()
}
