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

use adw::prelude::*;

pub struct ScoresDialog {
    pub dlg: adw::Window,
    pub window_title: adw::WindowTitle,
    pub headerbar: adw::HeaderBar,
    pub vbox: gtk::Box,
}

pub fn new_scores_dialog(window: &gtk::Window) -> ScoresDialog {
    let dlg = adw::Window::builder()
        .resizable(true)
        .modal(true)
        .transient_for(window)
        .build();

    let window_title = adw::WindowTitle::builder().build();

    let headerbar = adw::HeaderBar::builder()
        .title_widget(&window_title)
        .build();

    let content = adw::ToolbarView::builder().build();
    dlg.set_content(Some(&content));

    content.add_top_bar(&headerbar);

    let vbox = gtk::Box::builder()
        .orientation(gtk::Orientation::Vertical)
        .build();
    content.set_content(Some(&vbox));

    ScoresDialog {
        dlg,
        window_title,
        headerbar,
        vbox,
    }
}
