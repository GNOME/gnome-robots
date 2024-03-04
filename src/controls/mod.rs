pub mod controls;
pub mod editor;

use gtk::gdk;

pub fn key_from_u32(keyval: u32) -> gdk::Key {
    use gtk::glib::translate::FromGlib;
    unsafe { gdk::Key::from_glib(keyval) }
}

pub fn key_from_i32(keyval: i32) -> gdk::Key {
    key_from_u32(keyval as u32)
}

pub fn key_into_i32(keyval: gdk::Key) -> i32 {
    use gtk::glib::translate::IntoGlib;
    keyval.into_glib() as i32
}
