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

use crate::config::VERSION;
use adw::{prelude::*, subclass::prelude::*};
use gettextrs::gettext;
use gtk::{gdk, gio, glib};
use std::path::Path;

mod imp {
    use super::*;
    use crate::{
        assets::{Assets, DirectoryAssets},
        config::DATA_DIRECTORY,
        game_config::GameConfigs,
        properties_dialog::show_preferences,
        window::RobotsWindow,
    };
    use std::{cell::OnceCell, rc::Rc};

    pub struct RobotsApplication {
        settings: gio::Settings,
        game_configs: OnceCell<GameConfigs>,
        assets: OnceCell<Rc<dyn Assets>>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for RobotsApplication {
        const NAME: &'static str = "RobotsApplication";
        type Type = super::RobotsApplication;
        type ParentType = adw::Application;

        fn new() -> Self {
            Self {
                settings: gio::Settings::new("org.gnome.Robots"),
                game_configs: Default::default(),
                assets: Default::default(),
            }
        }
    }

    impl ObjectImpl for RobotsApplication {}

    impl ApplicationImpl for RobotsApplication {
        fn startup(&self) {
            self.parent_startup();

            if let Some(display) = gdk::Display::default() {
                gtk::IconTheme::for_display(&display)
                    .add_search_path(Path::new(DATA_DIRECTORY).join("icons"));
            } else {
                eprintln!("No default display found");
            }

            glib::set_application_name(&gettext("Robots"));
            gtk::Window::set_default_icon_name("org.gnome.Robots");

            self.obj().add_action_entries([
                gio::ActionEntry::builder("new-game")
                    .activate(|this: &Self::Type, _, _| this.imp().new_game_cb())
                    .build(),
                gio::ActionEntry::builder("preferences")
                    .activate(|this: &Self::Type, _, _| this.imp().preferences_cb())
                    .build(),
                gio::ActionEntry::builder("scores")
                    .activate(|this: &Self::Type, _, _| this.imp().scores_cb())
                    .build(),
                gio::ActionEntry::builder("help")
                    .activate(|this: &Self::Type, _, _| this.imp().help_cb())
                    .build(),
                gio::ActionEntry::builder("about")
                    .activate(|this: &Self::Type, _, _| this.imp().about_cb())
                    .build(),
                gio::ActionEntry::builder("quit")
                    .activate(|this: &Self::Type, _, _| this.quit())
                    .build(),
            ]);

            self.obj()
                .set_accels_for_action("app.new-game", &["<Primary>n"]);
            self.obj().set_accels_for_action("app.help", &["F1"]);
            self.obj()
                .set_accels_for_action("app.quit", &["<Primary>q"]);

            match DirectoryAssets::from_directory(Path::new(DATA_DIRECTORY)) {
                Ok(assets) => {
                    self.assets.set(Rc::new(assets)).ok().unwrap();
                }
                Err(e) => {
                    eprintln!("CRITICAL: Failed to load game assets: {}", e);
                    self.obj().quit();
                }
            }

            match GameConfigs::load() {
                Ok(game_configs) => {
                    self.game_configs.set(game_configs).ok().unwrap();
                }
                Err(e) => {
                    eprintln!("CRITICAL {}\n{}",
                        gettext("The program Robots was unable to find any valid game configuration files. Please check that the program is installed correctly."),
                        e
                    );
                    self.obj().quit();
                }
            }
        }

        fn activate(&self) {
            self.parent_activate();

            if let Some(window) = self.obj().active_window().and_downcast::<RobotsWindow>() {
                window.present();
                return;
            }

            match RobotsWindow::new(
                &*self.obj(),
                &self.settings,
                self.game_configs.get().unwrap().clone(),
                self.assets.get().unwrap(),
            ) {
                Ok(window) => {
                    window.present();
                }
                Err(e) => {
                    eprintln!("CRITICAL: {}", e);
                    // TODO message box
                    self.obj().quit();
                }
            }
        }
    }

    impl GtkApplicationImpl for RobotsApplication {}
    impl AdwApplicationImpl for RobotsApplication {}

    impl RobotsApplication {
        fn new_game_cb(&self) {
            if let Some(window) = self.obj().active_window().and_downcast::<RobotsWindow>() {
                let dialog = adw::AlertDialog::new(
                    Some(&gettext("New Game")),
                    Some(&gettext(
                        "Are you sure you want to discard the current game?",
                    )),
                );

                dialog.add_response("cancel", &gettext("Keep _Playing"));
                dialog.add_response("new", &gettext("_New Game"));
                dialog.set_response_appearance("new", adw::ResponseAppearance::Destructive);

                dialog.set_default_response(Some("new"));
                dialog.set_close_response("cancel");

                dialog.choose(&window.clone(), gio::Cancellable::NONE, move |ret| {
                    if ret == "new" {
                        window.start_new_game();
                    }
                });
            } else {
                self.activate();
            }
        }

        fn preferences_cb(&self) {
            show_preferences(
                self.obj().active_window().as_ref(),
                self.game_configs.get().unwrap(),
                &self.assets.get().unwrap().themes(),
                &self.settings,
            );
        }

        fn scores_cb(&self) {
            if let Some(window) = self.obj().active_window().and_downcast::<RobotsWindow>() {
                window.show_highscores();
            }
        }

        fn help_cb(&self) {
            let launcher = gtk::UriLauncher::new("help:gnome-robots");
            launcher.launch(
                self.obj().active_window().as_ref(),
                gio::Cancellable::NONE,
                |_| {},
            );
        }

        fn about_cb(&self) {
            adw::AboutDialog::builder()
                .application_icon("org.gnome.Robots")
                .application_name(gettext("Robots"))
                .version(VERSION)
                .copyright("Copyright © 1998–2008 Mark Rae\nCopyright © 2014–2016 Michael Catanzaro\nCopyright © 2020-2025 Andrey Kutejko")
                .license_type(gtk::License::Gpl30)
                .comments(gettext("Based on classic BSD Robots"))
                .developers([
                    "Mark Rae <m.rae@inpharmatica.co.uk>",
                    "Andrey Kutejko <andy128k@gmail.com>"
                ])
                .artists([
                    "Kirstie Opstad <K.Opstad@ed.ac.uk>",
                    "Rasoul M.P. Aghdam (player death sound)"
                ])
                .documenters([
                    "Aruna Sankaranarayanan"
                ])
                .translator_credits(gettext("translator-credits"))
                .website("https://gitlab.gnome.org/GNOME/gnome-robots")
                .build()
            .present(self.obj().active_window().as_ref());
        }
    }
}

glib::wrapper! {
    pub struct RobotsApplication(ObjectSubclass<imp::RobotsApplication>)
        @extends adw::Application, gtk::Application, gio::Application,
        @implements gio::ActionGroup, gio::ActionMap;
}

impl Default for RobotsApplication {
    fn default() -> Self {
        glib::Object::builder()
            .property("application-id", "org.gnome.Robots")
            .property("flags", gio::ApplicationFlags::default())
            .build()
    }
}
