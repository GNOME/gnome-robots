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
