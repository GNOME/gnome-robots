/*
 * Copyright 2020-2024 Andrey Kutejko <andy128k@gmail.com>
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
use gtk::{gdk, glib, prelude::*, subclass::prelude::*};

mod imp {
    use super::*;
    use std::{cell::Cell, sync::OnceLock};

    pub struct ControlEditor {
        pub label: gtk::Label,
        pub key: Cell<Option<gdk::Key>>,
        pub editing: Cell<bool>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for ControlEditor {
        const NAME: &'static str = "RobotsControlEditor";
        type Type = super::ControlEditor;
        type ParentType = gtk::Widget;

        fn new() -> Self {
            Self {
                label: gtk::Label::builder()
                    .halign(gtk::Align::Start)
                    .valign(gtk::Align::Center)
                    .build(),
                key: Cell::new(None),
                editing: Cell::new(false),
            }
        }
    }

    impl ObjectImpl for ControlEditor {
        fn constructed(&self) {
            self.parent_constructed();

            let obj = self.obj();
            obj.set_layout_manager(Some(gtk::BinLayout::new()));
            obj.set_focusable(true);

            let click_controller = gtk::GestureClick::new();
            click_controller.connect_pressed(glib::clone!(
                #[weak(rename_to = imp)]
                self,
                move |_, _, _, _| imp.set_editing(true)
            ));
            obj.add_controller(click_controller);

            let key_controller = gtk::EventControllerKey::new();
            key_controller.connect_key_pressed(glib::clone!(
                #[weak(rename_to = imp)]
                self,
                #[upgrade_or]
                glib::Propagation::Proceed,
                move |_, keyval, _keycode, mods| imp.key_controller_key_pressed(keyval, mods)
            ));
            obj.add_controller(key_controller);

            self.label.set_parent(&*obj);

            obj.connect_mnemonic_activate(glib::clone!(
                #[weak(rename_to = imp)]
                self,
                #[upgrade_or]
                glib::Propagation::Proceed,
                move |_, _| {
                    imp.set_editing(true);
                    glib::Propagation::Proceed
                }
            ));
            obj.connect_move_focus(glib::clone!(
                #[weak(rename_to = imp)]
                self,
                move |_, _| imp.set_editing(false)
            ));
        }

        fn dispose(&self) {
            while let Some(child) = self.obj().first_child() {
                child.unparent();
            }
        }

        fn signals() -> &'static [glib::subclass::Signal] {
            static SIGNALS: OnceLock<Vec<glib::subclass::Signal>> = OnceLock::new();
            SIGNALS.get_or_init(|| {
                vec![
                    glib::subclass::Signal::builder("edited")
                        .param_types([gdk::Key::static_type()])
                        .return_type::<()>()
                        .build(),
                    glib::subclass::Signal::builder("cleared")
                        .return_type::<gdk::Key>()
                        .build(),
                ]
            })
        }
    }

    impl WidgetImpl for ControlEditor {}

    impl ControlEditor {
        pub fn set_editing(&self, editing: bool) {
            self.editing.set(editing);
            if editing {
                self.obj().grab_focus();
            }
            self.update_label();
        }

        pub fn update_label(&self) {
            if self.editing.get() {
                self.label.set_label(&gettext("New accelerator…"));
            } else if let Some(key) = self.key.get() {
                self.label
                    .set_label(&gtk::accelerator_get_label_with_keycode(
                        Some(&self.obj().display()),
                        key,
                        0,
                        gdk::ModifierType::empty(),
                    ));
            } else {
                self.label.set_label(&gettext("Disabled"));
            }
        }

        fn key_controller_key_pressed(
            &self,
            keyval: gdk::Key,
            mods: gdk::ModifierType,
        ) -> glib::Propagation {
            if self.editing.get() {
                self.set_editing(false);
                if mods == gdk::ModifierType::empty() {
                    return match keyval {
                        gdk::Key::Tab => glib::Propagation::Proceed,
                        gdk::Key::BackSpace => {
                            self.key.set(Some(self.obj().emit_cleared()));
                            glib::Propagation::Stop
                        }
                        gdk::Key::Escape => glib::Propagation::Stop,
                        _ => {
                            self.key.set(Some(keyval));
                            self.obj().emit_edited(keyval);
                            glib::Propagation::Stop
                        }
                    };
                }
            } else {
                if mods == gdk::ModifierType::empty() {
                    match keyval {
                        gdk::Key::Return | gdk::Key::KP_Enter => {
                            self.set_editing(true);
                            return glib::Propagation::Stop;
                        }
                        _ => return glib::Propagation::Proceed,
                    }
                }
            }
            glib::Propagation::Proceed
        }
    }
}

glib::wrapper! {
    pub struct ControlEditor(ObjectSubclass<imp::ControlEditor>)
        @extends gtk::Widget,
        @implements gtk::Accessible, gtk::Buildable, gtk::ConstraintTarget;
}

impl Default for ControlEditor {
    fn default() -> Self {
        glib::Object::builder().build()
    }
}

impl ControlEditor {
    pub fn key(&self) -> Option<gdk::Key> {
        self.imp().key.get()
    }

    pub fn set_key(&self, key: gdk::Key) {
        self.imp().key.set(Some(key));
        self.imp().update_label();
    }

    fn emit_edited(&self, key: gdk::Key) {
        self.emit_by_name::<()>("edited", &[&key]);
    }

    pub fn connect_edited<F>(&self, f: F) -> glib::signal::SignalHandlerId
    where
        F: Fn(gdk::Key) + 'static,
    {
        self.connect_closure(
            "edited",
            false,
            glib::closure_local!(move |_self: &Self, key: gdk::Key| (f)(key)),
        )
    }

    fn emit_cleared(&self) -> gdk::Key {
        self.emit_by_name::<gdk::Key>("cleared", &[])
    }

    pub fn connect_cleared<F>(&self, f: F) -> glib::signal::SignalHandlerId
    where
        F: Fn() -> gdk::Key + 'static,
    {
        self.connect_closure(
            "cleared",
            false,
            glib::closure_local!(move |_self: &Self| (f)()),
        )
    }
}
