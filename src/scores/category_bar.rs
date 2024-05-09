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

use gettextrs::gettext;
use gtk::prelude::*;

pub fn category_bar(child: &impl IsA<gtk::Widget>) -> gtk::Widget {
    let catbar = gtk::Box::builder()
        .orientation(gtk::Orientation::Horizontal)
        .spacing(12)
        .margin_top(20)
        .margin_start(20)
        .margin_end(20)
        .halign(gtk::Align::Center)
        .build();

    let label = gtk::Label::builder()
        .label(gettext("Game Type:"))
        .halign(gtk::Align::Center)
        .build();
    catbar.append(&label);

    catbar.append(child);

    catbar.upcast()
}
