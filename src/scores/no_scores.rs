use gtk::prelude::*;

fn g_(str: &str) -> String {
    str.to_owned()
}

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
            g_("No scores yet")
        ))
        .use_markup(true)
        .build();
    vbox.append(&title_label);

    let description_label = gtk::Label::builder()
        .label(g_("Play some games and your scores will show up here."))
        .build();
    vbox.append(&description_label);

    vbox.upcast()
}
