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

use crate::assets::Assets;
use crate::game::{Game, PlayerCommand, State};
use crate::game_area::GameArea;
use crate::game_config::GameConfigs;
use crate::properties::{Properties, CONTROL_KEYS};
use crate::scores::scores::{show_scores, Category};
use crate::window_size::remember_window_size;
use adw::{prelude::*, subclass::prelude::*};
use gettextrs::gettext;
use gtk::{gdk, gio, glib};
use rand::thread_rng;
use std::error::Error;
use std::rc::Rc;

mod imp {
    use super::*;
    use std::cell::OnceCell;

    pub struct RobotsWindow {
        pub window_title: adw::WindowTitle,
        pub safe_teleports_label: gtk::Label,
        pub game_area: OnceCell<GameArea>,
        pub settings: OnceCell<gio::Settings>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for RobotsWindow {
        const NAME: &'static str = "RobotsWindow";
        type Type = super::RobotsWindow;
        type ParentType = adw::ApplicationWindow;

        fn new() -> Self {
            Self {
                window_title: adw::WindowTitle::builder().title(gettext("Robots")).build(),
                safe_teleports_label: gtk::Label::builder().use_markup(true).build(),
                game_area: Default::default(),
                settings: Default::default(),
            }
        }

        fn class_init(klass: &mut Self::Class) {
            klass.install_action("win.random-teleport", None, |obj, _, _| {
                obj.imp().random_teleport_cb();
            });
            klass.install_action("win.safe-teleport", None, |obj, _, _| {
                obj.imp().safe_teleport_cb();
            });
            klass.install_action("win.wait", None, |obj, _, _| {
                obj.imp().wait_cb();
            });
        }
    }

    impl ObjectImpl for RobotsWindow {}
    impl WidgetImpl for RobotsWindow {}
    impl WindowImpl for RobotsWindow {}
    impl ApplicationWindowImpl for RobotsWindow {}
    impl AdwApplicationWindowImpl for RobotsWindow {}

    impl RobotsWindow {
        fn random_teleport_cb(&self) {
            self.game_area
                .get()
                .unwrap()
                .player_command(PlayerCommand::RandomTeleport);
        }

        fn safe_teleport_cb(&self) {
            self.game_area
                .get()
                .unwrap()
                .player_command(PlayerCommand::SafeTeleport);
        }

        fn wait_cb(&self) {
            self.game_area
                .get()
                .unwrap()
                .player_command(PlayerCommand::Wait);
        }
    }

    pub fn button_box(safe_teleports_label: &gtk::Label) -> gtk::Box {
        let hbox = gtk::Box::builder()
            .orientation(gtk::Orientation::Horizontal)
            .spacing(10)
            .build();
        let size_group = gtk::SizeGroup::new(gtk::SizeGroupMode::Both);

        {
            let label = gtk::Label::builder()
                .label(gettext("Teleport _Randomly"))
                .use_underline(true)
                .margin_top(15)
                .margin_bottom(15)
                .build();
            let button = gtk::Button::builder()
                .action_name("win.random-teleport")
                .child(&label)
                .hexpand(true)
                .build();
            size_group.add_widget(&button);
            hbox.append(&button);
        }

        {
            let vbox = gtk::Box::builder()
                .orientation(gtk::Orientation::Vertical)
                .margin_top(15)
                .margin_bottom(15)
                .build();
            let label = gtk::Label::builder()
                .label(gettext("Teleport _Safely"))
                .use_underline(true)
                .build();
            vbox.append(&label);
            vbox.append(safe_teleports_label);
            let button = gtk::Button::builder()
                .action_name("win.safe-teleport")
                .child(&vbox)
                .hexpand(true)
                .build();
            size_group.add_widget(&button);
            hbox.append(&button);
        }

        {
            let label = gtk::Label::builder()
                .label(gettext("_Wait for Robots"))
                .use_underline(true)
                .margin_top(15)
                .margin_bottom(15)
                .build();
            let button = gtk::Button::builder()
                .action_name("win.wait")
                .child(&label)
                .hexpand(true)
                .build();
            size_group.add_widget(&button);
            hbox.append(&button);
        }

        hbox
    }
}

glib::wrapper! {
    pub struct RobotsWindow(ObjectSubclass<imp::RobotsWindow>)
        @extends adw::ApplicationWindow, gtk::ApplicationWindow, gtk::Window, gtk::Widget,
        @implements gio::ActionGroup, gio::ActionMap, gtk::Accessible, gtk::Buildable,
                    gtk::ConstraintTarget, gtk::Native, gtk::Root, gtk::ShortcutManager;
}

impl RobotsWindow {
    pub fn new(
        application: &impl IsA<gtk::Application>,
        settings: &gio::Settings,
        game_configs: GameConfigs,
        assets: &Rc<dyn Assets>,
    ) -> Result<Self, Box<dyn Error>> {
        let this: Self = glib::Object::builder()
            .property("application", application)
            .build();

        remember_window_size(this.upcast_ref(), settings);
        this.imp().settings.set(settings.clone()).ok().unwrap();

        let vbox = gtk::Box::builder()
            .orientation(gtk::Orientation::Vertical)
            .build();
        this.set_content(Some(&vbox));

        let headerbar = adw::HeaderBar::builder()
            .title_widget(&this.imp().window_title)
            .build();
        vbox.append(&headerbar);

        let appmenu = gio::Menu::new();
        appmenu.append_section(None, &{
            let section = gio::Menu::new();
            section.append_item(&gio::MenuItem::new(
                Some(&gettext("_New Game")),
                Some("app.new-game"),
            ));
            section.append_item(&gio::MenuItem::new(
                Some(&gettext("_Scores")),
                Some("app.scores"),
            ));
            section
        });
        appmenu.append_section(None, &{
            let section = gio::Menu::new();
            section.append_item(&gio::MenuItem::new(
                Some(&gettext("_Preferences")),
                Some("app.preferences"),
            ));
            section.append_item(&gio::MenuItem::new(
                Some(&gettext("_Help")),
                Some("app.help"),
            ));
            section.append_item(&gio::MenuItem::new(
                Some(&gettext("_About Robots")),
                Some("app.about"),
            ));
            section
        });

        let menu_button = gtk::MenuButton::builder()
            .icon_name("open-menu-symbolic")
            .menu_model(&appmenu)
            .build();
        headerbar.pack_end(&menu_button);

        let game = Game::new(
            game_configs.best_match(&settings.selected_config()),
            thread_rng(),
        );
        game.start_new_game();

        let game_area = GameArea::new(game_configs, assets, settings)?;
        game_area.connect_updated(glib::clone!(
            #[weak]
            this,
            move |ga| this.update_game_status(ga.game().as_ref().unwrap())
        ));

        let gridframe = gtk::AspectFrame::builder()
            .ratio((game.width() as f32) / (game.height() as f32))
            .hexpand(true)
            .vexpand(true)
            .child(&game_area)
            .build();

        game_area.set_game(game);
        this.imp().game_area.set(game_area).ok().unwrap();

        let toolbar_view = adw::ToolbarView::builder().content(&gridframe).build();

        let action_bar = gtk::ActionBar::builder().build();
        action_bar.set_center_widget(Some(&imp::button_box(&this.imp().safe_teleports_label)));
        toolbar_view.add_bottom_bar(&action_bar);

        vbox.append(&toolbar_view);

        let key_controller = gtk::EventControllerKey::new();
        key_controller.connect_key_pressed(glib::clone!(
            #[weak]
            this,
            #[upgrade_or]
            glib::Propagation::Proceed,
            move |_, keyval, _keycode, state| this.keyboard_cb(keyval, state)
        ));
        this.add_controller(key_controller);

        Ok(this)
    }

    fn update_game_status(&self, game: &Game) {
        let status = game.status();

        self.imp().window_title.set_subtitle(
            &gettext("Level: {level}\tScore: {score}")
                .replace("{level}", &status.current_level.to_string())
                .replace("{score}", &status.score.to_string()),
        );

        let remaining_teleports_text =
            gettext("(Remaining: %d)").replacen("%d", &status.safe_teleports.to_string(), 1);
        self.imp()
            .safe_teleports_label
            .set_markup(&format!("<small>{}</small>", remaining_teleports_text));

        let is_playing = game.state() != State::Complete && game.state() != State::Dead;
        self.enable_action("random-teleport", is_playing);
        self.enable_action("safe-teleport", is_playing && status.safe_teleports > 0);
        self.enable_action("wait", is_playing);
    }

    fn enable_action(&self, action_name: &str, enabled: bool) {
        if let Some(action) = self
            .lookup_action(action_name)
            .and_downcast::<gio::SimpleAction>()
        {
            action.set_enabled(enabled);
        }
    }

    fn keyboard_cb(&self, pressed: gdk::Key, state: gdk::ModifierType) -> glib::Propagation {
        if state.is_empty() {
            let settings = self.imp().settings.get().unwrap();
            let game_area = self.imp().game_area.get().unwrap();
            for ck in CONTROL_KEYS {
                if settings.get_control_key(ck) == pressed {
                    game_area.player_command(ck.command);
                    return glib::Propagation::Stop;
                }
            }
        }
        glib::Propagation::Proceed
    }

    pub fn start_new_game(&self) {
        self.imp().game_area.get().unwrap().start_new_game();
    }

    pub fn show_highscores(&self) {
        let settings = self.imp().settings.get().unwrap();

        let category = Category {
            key: settings.selected_config(),
            safety: settings.move_safety(),
        };

        show_scores(Some(&category), self);
    }
}
