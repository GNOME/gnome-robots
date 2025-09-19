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

use crate::{
    animation::InfiniteAnimation,
    arena::ObjectType,
    assets::Assets,
    game::{Game, GameEvent, MoveSafety, PlayerCommand, State},
    game_config::{GameConfig, GameConfigs},
    graphics::calculate_contrast_color,
    properties::Properties,
    scores::scores::{add_score, Category},
    sound_player::{Sound, SoundPlayer},
    theme::Theme,
    themes,
    utils::settings_aware_widget,
};
use gettextrs::gettext;
use gtk::{gdk, gio, glib, graphene, prelude::*, subclass::prelude::*};
use std::{cell::Ref, error::Error, f64::consts::PI, rc::Rc, time::Duration};

const MINIMUM_TILE_WIDTH: u32 = 8;
const MINIMUM_TILE_HEIGHT: u32 = 8;
const ANIMATION_DELAY_MILLIS: u64 = 100;
const ANIMATION_DELAY: Duration = Duration::from_millis(ANIMATION_DELAY_MILLIS);

mod imp {
    use super::*;
    use crate::arena::Position;
    use crate::game::State;
    use crate::slot::ListenerId;
    use crate::theme::{
        player_animation, player_dead_animation, robot1_animation, robot2_animation,
    };
    use std::cell::{Cell, OnceCell, RefCell};
    use std::sync::OnceLock;
    use std::time::{SystemTime, UNIX_EPOCH};

    pub struct GameArea {
        pub game: RefCell<Option<Game>>,
        pub game_listener_id: Cell<Option<ListenerId>>,
        pub game_configs: OnceCell<GameConfigs>,
        pub assets: OnceCell<Rc<dyn Assets>>,
        pub theme: RefCell<Option<Theme>>,
        pub background_color: RefCell<gdk::RGBA>,
        pub alternate_background_color: RefCell<gdk::RGBA>,
        pub timer_id: Cell<Option<glib::SourceId>>,
        pub player_animation: Box<dyn InfiniteAnimation>,
        pub player_dead_animation: Box<dyn InfiniteAnimation>,
        pub robot1_animation: Box<dyn InfiniteAnimation>,
        pub robot2_animation: Box<dyn InfiniteAnimation>,
        pub sound_player: SoundPlayer,
        pub settings: OnceCell<gio::Settings>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for GameArea {
        const NAME: &'static str = "RobotsGameArea";
        type Type = super::GameArea;
        type ParentType = gtk::Widget;

        fn new() -> Self {
            Self {
                game: Default::default(),
                game_listener_id: Default::default(),
                game_configs: Default::default(),
                assets: Default::default(),
                theme: Default::default(),
                background_color: RefCell::new(gdk::RGBA::BLUE),
                alternate_background_color: RefCell::new(gdk::RGBA::GREEN),
                timer_id: Default::default(),

                player_animation: Box::new(player_animation()),
                player_dead_animation: Box::new(player_dead_animation()),
                robot1_animation: Box::new(robot1_animation()),
                robot2_animation: Box::new(robot2_animation()),

                sound_player: SoundPlayer::new(),
                settings: Default::default(),
            }
        }
    }

    impl ObjectImpl for GameArea {
        fn constructed(&self) {
            self.parent_constructed();

            let obj = self.obj();
            obj.set_layout_manager(Some(gtk::BinLayout::new()));

            let click_controller = gtk::GestureClick::new();
            click_controller.connect_pressed(glib::clone!(
                #[weak]
                obj,
                move |_, n_pressed, x, y| obj.mouse_cb(n_pressed, x, y)
            ));
            obj.add_controller(click_controller);

            let motion_controller = gtk::EventControllerMotion::new();
            motion_controller.connect_motion(glib::clone!(
                #[weak]
                obj,
                move |_, x, y| obj.move_cb(x, y)
            ));
            obj.add_controller(motion_controller);

            self.timer_id.set(Some(glib::timeout_add_local(
                ANIMATION_DELAY,
                glib::clone!(
                    #[strong]
                    obj,
                    move || {
                        obj.timer_cb();
                        glib::ControlFlow::Continue
                    }
                ),
            )));
        }

        fn dispose(&self) {
            if let Some(source_id) = self.timer_id.take() {
                source_id.remove();
            }

            while let Some(child) = self.obj().first_child() {
                child.unparent();
            }
        }

        fn signals() -> &'static [glib::subclass::Signal] {
            static SIGNALS: OnceLock<Vec<glib::subclass::Signal>> = OnceLock::new();
            SIGNALS.get_or_init(|| {
                vec![glib::subclass::Signal::builder("updated")
                    .return_type::<()>()
                    .build()]
            })
        }
    }

    impl WidgetImpl for GameArea {
        fn snapshot(&self, snapshot: &gtk::Snapshot) {
            let game_ref = self.game.borrow();
            let Some(game) = game_ref.as_ref() else {
                return;
            };

            let time = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .map_or(0, |t| t.as_millis() / ANIMATION_DELAY_MILLIS as u128)
                as usize;
            let tile_width = (self.obj().width() as f64) / (game.width() as f64);
            let tile_height = (self.obj().height() as f64) / (game.height() as f64);
            let assets = self.assets.get().unwrap();

            for y in 0..game.height() {
                for x in 0..game.width() {
                    let position = Position { x, y };

                    let tile_rect = graphene::Rect::new(
                        ((x as f64) * tile_width) as f32,
                        ((y as f64) * tile_height) as f32,
                        (tile_width) as f32,
                        (tile_height) as f32,
                    );
                    let color = if (x + y) % 2 != 0 {
                        self.background_color.borrow()
                    } else {
                        self.alternate_background_color.borrow()
                    };
                    snapshot.append_color(&color, &tile_rect);

                    let object_type = game.get(position);
                    let frame = match object_type {
                        ObjectType::Player if game.state() == State::Dead => {
                            self.player_dead_animation.frame(time)
                        }
                        ObjectType::Player => self.player_animation.frame(time),
                        ObjectType::Robot1 => self.robot1_animation.frame(time),
                        ObjectType::Robot2 => self.robot2_animation.frame(time),
                        ObjectType::Heap => 0,
                        ObjectType::None => 0,
                    };
                    self.theme.borrow().as_ref().unwrap().draw_object(
                        object_type,
                        frame,
                        snapshot,
                        &tile_rect,
                    );
                }
            }

            if let Some(splat) = game.splat() {
                assets.splat_bubble().draw(
                    snapshot,
                    (splat.x as f64) * tile_width + 8.0,
                    (splat.y as f64) * tile_height + 8.0,
                );
            }

            match game.state() {
                State::Dead => {
                    let player = game.player();
                    assets.aieee_bubble().draw(
                        snapshot,
                        (player.x as f64) * tile_width + 8.0,
                        (player.y as f64) * tile_height + 4.0,
                    );
                }
                State::Complete => {
                    let player = game.player();
                    assets.yahoo_bubble().draw(
                        snapshot,
                        (player.x as f64) * tile_width + 8.0,
                        (player.y as f64) * tile_height + 4.0,
                    );
                }
                _ => {}
            }
        }
    }
}

glib::wrapper! {
    pub struct GameArea(ObjectSubclass<imp::GameArea>)
        @extends gtk::Widget,
        @implements gtk::Accessible, gtk::Buildable, gtk::ConstraintTarget;
}

impl GameArea {
    pub fn new(
        game_configs: GameConfigs,
        assets: &Rc<dyn Assets>,
        settings: &gio::Settings,
    ) -> Result<Self, Box<dyn Error>> {
        let this: Self = glib::Object::builder().build();

        this.imp().game_configs.set(game_configs).ok().unwrap();
        this.imp().assets.set(assets.clone()).ok().unwrap();
        this.set_background_color(settings.bgcolour());
        *this.imp().theme.borrow_mut() = Some(themes::find_best_match(
            &assets.themes(),
            &settings.theme(),
        )?);
        this.imp().settings.set(settings.clone()).ok().unwrap();

        settings_aware_widget(&this, settings, None, |ga, s| ga.properties_changed_cb(s));

        this.start_new_game();

        Ok(this)
    }

    pub fn game(&self) -> Ref<'_, Option<Game>> {
        self.imp().game.borrow()
    }

    pub fn set_game(&self, game: Game) {
        let mut this_game = self.imp().game.borrow_mut();

        if let Some(listener_id) = self.imp().game_listener_id.take() {
            if let Some(game) = this_game.as_ref() {
                game.on_game_event.disconnect(listener_id);
            }
        }

        self.set_size_request(
            (MINIMUM_TILE_WIDTH * game.width()) as i32,
            (MINIMUM_TILE_HEIGHT * game.height()) as i32,
        );

        let game_listener_id = game.on_game_event.connect(glib::clone!(
            #[weak(rename_to = this)]
            self,
            move |event| {
                let event = *event;
                glib::MainContext::default().spawn_local(async move {
                    this.on_game_event(event).await;
                });
            }
        ));
        *this_game = Some(game);
        self.imp().game_listener_id.set(Some(game_listener_id));
    }

    pub fn set_background_color(&self, color: gdk::RGBA) {
        *self.imp().background_color.borrow_mut() = color;
        *self.imp().alternate_background_color.borrow_mut() = calculate_contrast_color(&color);
        self.queue_draw();
    }

    fn timer_cb(&self) {
        if let Some(game) = self.imp().game.borrow().as_ref() {
            game.tick();
            self.emit_by_name::<()>("updated", &[]);
        }
        self.queue_draw();
    }

    fn mouse_cb(&self, _n_press: i32, x: f64, y: f64) {
        let game_ref = self.imp().game.borrow();
        let Some(game) = game_ref.as_ref() else {
            return;
        };
        if game.state() == State::Playing {
            let (dx, dy) = self.get_dir(game, x, y);
            let cmd = PlayerCommand::from_direction(dx, dy);
            self.player_command(cmd);
        }
    }

    fn move_cb(&self, x: f64, y: f64) {
        let game_ref = self.imp().game.borrow();
        let Some(game) = game_ref.as_ref() else {
            return;
        };
        if game.state() == State::Playing {
            let (dx, dy) = self.get_dir(game, x, y);
            let cursor_index = 3 * (dy + 1) + (dx + 1);
            let cursor = &self.imp().assets.get().unwrap().cursors()[cursor_index as usize];
            self.set_cursor(Some(cursor));
        } else {
            self.set_cursor(None);
        }
    }

    fn get_dir(&self, game: &Game, ix: f64, iy: f64) -> (i32, i32) {
        let tile_width = (self.width() as f64) / (game.width() as f64);
        let tile_height = (self.height() as f64) / (game.height() as f64);

        let x = ((ix / tile_width) as u32).clamp(0, game.width());
        let y = ((iy / tile_height) as u32).clamp(0, game.height());

        let player = game.player();

        // If we click on our man then we assume we hold.
        if x == player.x && y == player.y {
            return (0, 0);
        }

        // go in the general direction of the mouse click.
        let dx = ix / tile_width - (player.x as f64 + 0.5);
        let dy = iy / tile_height - (player.y as f64 + 0.5);

        round_direction(dx, dy)
    }

    fn properties_changed_cb(&self, settings: &gio::Settings) {
        if let Some(game) = self.imp().game.borrow().as_ref() {
            let selected_config = settings.selected_config();
            match self
                .imp()
                .game_configs
                .get()
                .unwrap()
                .find_by_name(&selected_config)
            {
                Some(config) => {
                    game.set_config(config);
                }
                None => {
                    eprintln!("Cannot change config to {}", selected_config);
                }
            }
        }

        let selected_theme = settings.theme();
        match themes::find_best_match(&self.imp().assets.get().unwrap().themes(), &selected_theme) {
            Ok(theme) => {
                *self.imp().theme.borrow_mut() = Some(theme);
            }
            Err(error) => {
                eprintln!("Cannot change theme to {}: {}", selected_theme, error);
            }
        }

        self.set_background_color(settings.bgcolour());

        self.queue_draw();
    }

    pub fn start_new_game(&self) {
        if let Some(game) = self.imp().game.borrow().as_ref() {
            game.start_new_game();
            self.queue_draw();
        }
    }

    pub fn player_command(&self, cmd: PlayerCommand) {
        let game_ref = self.imp().game.borrow();
        let Some(game) = game_ref.as_ref() else {
            return;
        };
        let safety = self.move_safety();
        if game.player_command(cmd, safety) {
            self.queue_draw();
        } else {
            self.play_sound(Sound::Bad);
        }
    }

    async fn on_game_event(&self, event: GameEvent) {
        match event {
            GameEvent::Teleported => self.play_sound(Sound::Teleport),
            GameEvent::Splat => self.play_sound(Sound::Splat),
            GameEvent::LevelComplete => self.play_sound(Sound::Yahoo),
            GameEvent::Death => self.play_sound(Sound::Die),
            GameEvent::Scored(score) if score > 0 => self.log_score(score).await,
            GameEvent::Victory => {
                self.message_box(&gettext(
                    "Congratulations, You Have Defeated the Robots!! \nBut Can You do it Again?",
                ));
                self.play_sound(Sound::Victory);
            }
            GameEvent::NoTeleportLocations => {
                self.message_box(&gettext("There are no teleport locations left!!"));
            }
            _ => {}
        }
    }

    fn move_safety(&self) -> MoveSafety {
        let properties = self.imp().settings.get().unwrap();
        properties.move_safety()
    }

    fn game_config(&self) -> Option<Rc<GameConfig>> {
        let game = self.imp().game.borrow();
        game.as_ref().map(|g| g.config.borrow().clone())
    }

    /// Enters a score in the high-score table
    async fn log_score(&self, score: u32) {
        let Some(game_config) = self.game_config() else {
            return;
        };
        let Some(window) = self.root().and_downcast::<gtk::Window>() else {
            return;
        };

        let category = Category {
            key: game_config.name.clone(),
            safety: self.move_safety(),
        };
        add_score(&category, score as i64, &window).await;
    }

    fn play_sound(&self, sound: Sound) {
        let properties = self.imp().settings.get().unwrap();
        if properties.sound() {
            self.imp().sound_player.play(sound);
        }
    }

    /// Displays a modal dialog box with a given message
    fn message_box(&self, msg: &str) {
        if let Some(window) = self.root().and_downcast::<gtk::Window>() {
            let dialog = gtk::AlertDialog::builder().message(msg).modal(true).build();
            dialog.show(Some(&window));
        } else {
            eprintln!("There is no top level window.");
            println!("{}", msg);
        }
    }

    pub fn connect_updated<F>(&self, f: F) -> glib::signal::SignalHandlerId
    where
        F: Fn(&Self) + 'static,
    {
        self.connect_closure(
            "updated",
            false,
            glib::closure_local!(move |this: &Self| (f)(this)),
        )
    }
}

fn round_direction(dx: f64, dy: f64) -> (i32, i32) {
    if dx.abs() < 0.5 && dy.abs() < 0.5 {
        return (0, 0);
    }

    const MOVE_TABLE: [(i32, i32); 8] = [
        (-1, 0),
        (-1, -1),
        (0, -1),
        (1, -1),
        (1, 0),
        (1, 1),
        (0, 1),
        (-1, 1),
    ];

    let angle = f64::atan2(dy, dx);

    /* Note the adjustment we have to make (+9, not +8) because atan2's idea
     * of octants and the ones we want are shifted by PI/8. */
    let octant = (((8.0 * angle / PI).floor() as i32 + 9) / 2) % 8;

    MOVE_TABLE[octant as usize]
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_round_direction() {
        assert_eq!(round_direction(-2.0, -3.0), (-1, -1));
        assert_eq!(round_direction(-3.0, -3.0), (-1, -1));
        assert_eq!(round_direction(-3.0, -1.0), (-1, 0));
        assert_eq!(round_direction(-3.0, 0.0), (-1, 0));
        assert_eq!(round_direction(-3.0, 1.0), (-1, 0));
        assert_eq!(round_direction(-3.0, 3.0), (-1, 1));

        assert_eq!(round_direction(-1.0, -3.0), (0, -1));
        assert_eq!(round_direction(-1.0, -1.0), (-1, -1));
        assert_eq!(round_direction(-1.0, 0.0), (-1, 0));
        assert_eq!(round_direction(-1.0, 1.0), (-1, 1));
        assert_eq!(round_direction(-1.0, 3.0), (0, 1));

        assert_eq!(round_direction(0.0, -2.0), (0, -1));
        assert_eq!(round_direction(0.0, -1.0), (0, -1));
        assert_eq!(round_direction(0.0, 0.0), (0, 0));
        assert_eq!(round_direction(0.0, 1.0), (0, 1));
        assert_eq!(round_direction(0.0, 2.0), (0, 1));

        assert_eq!(round_direction(1.0, -2.0), (1, -1));
        assert_eq!(round_direction(1.0, -1.0), (1, -1));
        assert_eq!(round_direction(1.0, 0.0), (1, 0));
        assert_eq!(round_direction(1.0, 1.0), (1, 1));
        assert_eq!(round_direction(1.0, 2.0), (1, 1));
        assert_eq!(round_direction(1.0, 3.0), (0, 1));

        assert_eq!(round_direction(2.0, -2.0), (1, -1));
        assert_eq!(round_direction(2.0, -1.0), (1, -1));
        assert_eq!(round_direction(2.0, 0.0), (1, 0));
        assert_eq!(round_direction(2.0, 1.0), (1, 1));
        assert_eq!(round_direction(2.0, 2.0), (1, 1));
    }
}
