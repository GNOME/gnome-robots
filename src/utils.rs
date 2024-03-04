use gtk::{gio, glib, prelude::*};
use std::cell::RefCell;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};

pub fn list_directory(directory: &Path) -> io::Result<Vec<PathBuf>> {
    let mut files = Vec::new();
    for entry in fs::read_dir(&directory)? {
        files.push(entry?.path());
    }
    Ok(files)
}

pub fn now() -> u64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_or(0, |t| t.as_secs())
}

pub fn settings_aware_widget<W, F>(
    widget: &W,
    settings: &gio::Settings,
    settings_key: Option<&str>,
    f: F,
) where
    W: IsA<gtk::Widget>,
    W: glib::clone::Downgrade,
    <W as glib::clone::Downgrade>::Weak: glib::clone::Upgrade<Strong = W>,
    F: Fn(&W, &gio::Settings) + 'static,
{
    let signal_handler_id = settings.connect_changed(
        settings_key,
        glib::clone!(@weak widget => move |s, _| {
            (f)(&widget, s);
        }),
    );
    widget.connect_destroy({
        let signal_handler_id = RefCell::new(Some(signal_handler_id));
        let settings = settings.clone();
        move |_| {
            if let Some(signal_handler_id) = signal_handler_id.take() {
                settings.disconnect(signal_handler_id);
            }
        }
    });
}
