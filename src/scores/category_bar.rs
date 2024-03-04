use gtk::prelude::*;

fn g_(str: &str) -> String {
    str.to_owned()
}

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
        .label(g_("Game Type:"))
        .halign(gtk::Align::Center)
        .build();
    catbar.append(&label);

    catbar.append(child);

    catbar.upcast()
}
