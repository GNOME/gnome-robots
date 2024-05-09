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

pub fn no_scores() -> gtk::Widget {
    let vbox = gtk::Box::builder()
        .orientation(gtk::Orientation::Vertical)
        .spacing(4)
        .hexpand(true)
        .vexpand(true)
        .valign(gtk::Align::Center)
        .width_request(450)
        .height_request(450)
        .build();
    vbox.add_css_class("dim-label");

    let image = gtk::Image::builder()
        .icon_name("org.gnome.Robots-symbolic")
        .pixel_size(64)
        .opacity(0.2)
        .build();
    vbox.append(&image);

    let title_label = gtk::Label::builder()
        .label(format!(
            "<b><span size=\"large\">{}</span></b>",
            gettext("No scores yet")
        ))
        .use_markup(true)
        .build();
    vbox.append(&title_label);

    let description_label = gtk::Label::builder()
        .label(gettext(
            "Play some games and your scores will show up here.",
        ))
        .build();
    vbox.append(&description_label);

    vbox.upcast()
}
