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

use std::cell::{Cell, RefCell};
use std::rc::Rc;

use crate::slot::Slot;
use crate::{
    arena::{Arena, ObjectType, Position},
    game_config::GameConfig,
};

#[derive(Clone, Copy)]
pub enum PlayerCommand {
    NW,
    N,
    NE,
    W,
    Stay,
    E,
    SW,
    S,
    SE,
    SafeTeleport,
    RandomTeleport,
    Wait,
}

impl PlayerCommand {
    pub fn from_direction(dx: i32, dy: i32) -> Self {
        match (dx, dy) {
            (-1, -1) => PlayerCommand::NW,
            (0, -1) => PlayerCommand::N,
            (1, -1) => PlayerCommand::NE,
            (-1, 0) => PlayerCommand::W,
            (0, 0) => PlayerCommand::Stay,
            (1, 0) => PlayerCommand::E,
            (-1, 1) => PlayerCommand::SW,
            (0, 1) => PlayerCommand::S,
            (1, 1) => PlayerCommand::SE,
            _ => PlayerCommand::Stay,
        }
    }

    pub fn to_direction(self) -> Option<(i32, i32)> {
        match self {
            PlayerCommand::NW => Some((-1, -1)),
            PlayerCommand::N => Some((0, -1)),
            PlayerCommand::NE => Some((1, -1)),
            PlayerCommand::W => Some((-1, 0)),
            PlayerCommand::Stay => Some((0, 0)),
            PlayerCommand::E => Some((1, 0)),
            PlayerCommand::SW => Some((-1, 1)),
            PlayerCommand::S => Some((0, 1)),
            PlayerCommand::SE => Some((1, 1)),
            _ => None,
        }
    }
}

/*
 * Size of the game playing area
 */
pub const GAME_WIDTH: u32 = 45;
pub const GAME_HEIGHT: u32 = 30;

pub const DEAD_DELAY: u32 = 30;
pub const CHANGE_DELAY: u32 = 20;

pub struct Status {
    pub score: u32,
    pub current_level: u32,
    pub safe_teleports: u32,
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum State {
    Playing = 1,
    Waiting,
    Complete,
    Dead,
    Type2,
    WaitingType2,
}

#[derive(Clone, Copy)]
pub enum GameEvent {
    Teleported,
    Splat,
    LevelComplete,
    Death,
    Scored(u32),
    Victory,
    NoTeleportLocations,
    NoSafeTeleportLocations,
}

struct ArenaChange {
    arena: Arena,
    player: Position,
    push: Option<Position>,
}

#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Debug)]
pub enum MoveSafety {
    Unsafe,
    Safe,
    SuperSafe,
}

pub struct Game {
    state: Cell<State>,
    arena: RefCell<Arena>,
    pub config: RefCell<Rc<GameConfig>>,
    player: Cell<Position>,
    splat: Cell<Option<Position>>,
    rand: RefCell<Box<dyn rand::RngCore>>,
    endlev_counter: Cell<u32>,
    current_level: Cell<u32>,
    score: Cell<u32>,
    kills: Cell<u32>,
    score_step: Cell<u32>,
    safe_teleports: Cell<u32>,

    pub on_game_event: Slot<GameEvent>,
}

impl Game {
    pub fn new(config: &Rc<GameConfig>, rand: impl rand::RngCore + 'static) -> Self {
        Self {
            state: Cell::new(State::Playing),
            arena: RefCell::new(Arena::new(GAME_WIDTH, GAME_HEIGHT)),
            config: RefCell::new(config.clone()),
            player: Cell::new(Position { x: 0, y: 0 }),
            splat: Cell::new(None),
            rand: RefCell::new(Box::new(rand)),
            endlev_counter: Cell::new(0),
            current_level: Cell::new(0),
            score: Cell::new(0),
            kills: Cell::new(0),
            score_step: Cell::new(0),
            safe_teleports: Cell::new(0),
            on_game_event: Slot::new(),
        }
    }

    pub fn width(&self) -> u32 {
        self.arena.borrow().width()
    }

    pub fn height(&self) -> u32 {
        self.arena.borrow().height()
    }

    pub fn get(&self, position: Position) -> ObjectType {
        self.arena.borrow().get(position)
    }

    pub fn state(&self) -> State {
        self.state.get()
    }

    pub fn status(&self) -> Status {
        Status {
            score: self.score.get(),
            current_level: self.current_level.get() + 1,
            safe_teleports: self.safe_teleports.get(),
        }
    }

    pub fn player(&self) -> Position {
        self.player.get()
    }

    pub fn splat(&self) -> Option<Position> {
        self.splat.get()
    }

    pub fn set_config(&self, config: &Rc<GameConfig>) {
        if *self.config.borrow() != *config {
            *self.config.borrow_mut() = config.clone();
            self.start_new_game();
        }
    }

    /// Ends the current game.
    fn kill_player(&self) {
        self.state.set(State::Dead);
        self.arena
            .borrow()
            .set(self.player.get(), ObjectType::Player);
        self.endlev_counter.set(0);
        self.on_game_event.emit(GameEvent::Death);
    }

    /**
     * add_kill
     * @type: robot type
     *
     * Description:
     * registers a robot kill and updates the score
     **/
    fn add_kill(&self, object_type: ObjectType) {
        let si = match (self.state.get(), object_type) {
            (State::Waiting | State::WaitingType2, ObjectType::Robot1) => {
                self.kills.set(self.kills.get() + 1);
                self.config.borrow().score_type1_waiting
            }
            (State::Waiting | State::WaitingType2, _) => {
                self.kills.set(self.kills.get() + 2);
                self.config.borrow().score_type2_waiting
            }
            (_, ObjectType::Robot1) => self.config.borrow().score_type1,
            _ => self.config.borrow().score_type2,
        };

        self.score.set(self.score.get() + si);
        self.score_step.set(self.score_step.get() + si);

        let score_reward = self.config.borrow().score_reward(self.score_step.get());
        self.score_step
            .set(self.score_step.get() - score_reward.price);
        self.add_safe_teleports(score_reward.safe_teleports_reward);

        let kill_reward = self.config.borrow().kills_reward(self.kills.get());
        self.kills.set(self.kills.get() - kill_reward.price);
        self.add_safe_teleports(kill_reward.safe_teleports_reward);
    }

    /// Creates a new level and populates it with robots
    fn generate_level(&self) {
        let arena = self.arena.borrow_mut();
        arena.clear();

        self.player.set(Position {
            x: arena.width() / 2,
            y: arena.height() / 2,
        });
        arena.set(self.player.get(), ObjectType::Player);

        let mut num_robots1 = self
            .config
            .borrow()
            .type1_robots_on_level(self.current_level.get());
        let mut num_robots2 = self
            .config
            .borrow()
            .type2_robots_on_level(self.current_level.get());

        let max_robots = arena.width() * arena.height() / 2;
        if num_robots1 + num_robots2 > max_robots {
            self.current_level.set(0);
            num_robots1 = self.config.borrow().initial_type1;
            num_robots2 = self.config.borrow().initial_type2;

            self.on_game_event.emit(GameEvent::Victory);
        }

        self.add_safe_teleports(self.config.borrow().free_safe_teleports);

        for _ in 0..num_robots1 {
            let p = arena
                .random_vacant_position(&mut *self.rand.borrow_mut())
                .expect("No vacant positions");
            arena.set(p, ObjectType::Robot1);
        }

        for _ in 0..num_robots2 {
            let p = arena
                .random_vacant_position(&mut *self.rand.borrow_mut())
                .expect("No vacant positions");
            arena.set(p, ObjectType::Robot2);
        }
    }

    fn add_safe_teleports(&self, teleports: u32) {
        self.safe_teleports.set(
            (self.safe_teleports.get() + teleports)
                .clamp(0, self.config.borrow().max_safe_teleports),
        );
    }

    fn update_arena(&self, change: ArenaChange) {
        if let Some(push) = change.push {
            match self.arena.borrow().get(push) {
                ObjectType::Robot1 => {
                    self.splat.set(Some(push));
                    self.on_game_event.emit(GameEvent::Splat);
                    self.score
                        .set(self.score.get() + self.config.borrow().score_type1_splatted);
                }
                ObjectType::Robot2 => {
                    self.splat.set(Some(push));
                    self.on_game_event.emit(GameEvent::Splat);
                    self.score
                        .set(self.score.get() + self.config.borrow().score_type2_splatted);
                }
                _ => {}
            }
        }

        self.player.set(change.player);
        *self.arena.borrow_mut() = change.arena;

        if self.arena.borrow().get(change.player) != ObjectType::Player {
            self.kill_player();
        } else {
            /* This is in the else statement to catch the case where the last
             * two robots collide on top of the human. Without the "else" this
             * leads to the player being ressurected and winning. */
            if self
                .arena
                .borrow()
                .count(|obj| obj == ObjectType::Robot1 || obj == ObjectType::Robot2)
                == 0
            {
                self.state.set(State::Complete);
                self.on_game_event.emit(GameEvent::LevelComplete);
                self.endlev_counter.set(0);
            }
        }
    }

    pub fn tick(&self) {
        match self.state.get() {
            State::Type2 => {
                let new_arena = self.move_type2_robots();
                let change = ArenaChange {
                    arena: new_arena,
                    player: self.player.get(),
                    push: None,
                };
                self.update_arena(change);
                self.state.set(State::Playing);
            }
            State::WaitingType2 => {
                let new_arena = self.move_type2_robots();
                let change = ArenaChange {
                    arena: new_arena,
                    player: self.player.get(),
                    push: None,
                };
                self.update_arena(change);
                self.state.set(State::Waiting);
            }
            State::Waiting => {
                self.splat.set(None);
                self.move_robots();
            }
            State::Complete => {
                self.endlev_counter.set(self.endlev_counter.get() + 1);
                if self.endlev_counter.get() >= CHANGE_DELAY {
                    self.current_level.set(self.current_level.get() + 1);
                    self.generate_level();
                    self.state.set(State::Playing);
                    self.splat.set(None);
                }
            }
            State::Dead => {
                self.endlev_counter.set(self.endlev_counter.get() + 1);
                if self.endlev_counter.get() >= DEAD_DELAY {
                    if self.score.get() > 0 {
                        self.on_game_event.emit(GameEvent::Scored(self.score.get()));
                    }
                    self.start_new_game();
                }
            }
            State::Playing => {}
        }
    }

    /// Initialises everything needed to start a new game
    pub fn start_new_game(&self) {
        self.current_level.set(0);
        self.score.set(0);
        self.kills.set(0);
        self.score_step.set(0);

        self.safe_teleports
            .set(self.config.borrow().initial_safe_teleports);

        self.splat.set(None);
        self.generate_level();

        self.state.set(State::Playing);
    }

    /// Moves all of the robots and checks for collisions
    fn move_all_robots(&self) -> Arena {
        return Self::chase(
            &self.arena.borrow(),
            |obj| obj == ObjectType::Robot1 || obj == ObjectType::Robot2,
            self.player.get(),
            |victim| self.add_kill(victim),
        );
    }

    /// Makes the extra move for all of the type2 robots
    fn move_type2_robots(&self) -> Arena {
        return Self::chase(
            &self.arena.borrow(),
            |obj| obj == ObjectType::Robot2,
            self.player.get(),
            |victim| self.add_kill(victim),
        );
    }

    /// Starts the process of moving robots
    fn move_robots(&self) {
        let new_arena = self.move_all_robots();

        let num_robots2 = new_arena.count(|obj| obj == ObjectType::Robot2);
        if num_robots2 > 0 {
            if self.state.get() == State::Waiting {
                self.state.set(State::WaitingType2);
            } else if self.state.get() == State::Playing {
                self.state.set(State::Type2);
            }
        }

        let change = ArenaChange {
            arena: new_arena,
            player: self.player.get(),
            push: None,
        };

        self.update_arena(change);
    }

    // private delegate void KillTracker (ObjectType victim);

    fn chase(
        arena: &Arena,
        is_chaser: impl Fn(ObjectType) -> bool,
        player: Position,
        track_kill: impl Fn(ObjectType),
    ) -> Arena {
        let new_arena = arena.map(|obj| {
            if obj == ObjectType::Player || (is_chaser)(obj) {
                ObjectType::None
            } else {
                obj
            }
        });

        for x in 0..arena.width() {
            for y in 0..arena.height() {
                let position = Position { x, y };
                let who = arena.get(position);
                if (is_chaser)(who) {
                    let new_postions = position.move_by(
                        ((player.x as i32) - (position.x as i32)).signum(),
                        ((player.y as i32) - (position.y as i32)).signum(),
                    );

                    let destination = new_arena.get(new_postions);
                    if destination == ObjectType::Heap {
                        (track_kill)(who);
                    } else if destination == ObjectType::Robot1 || destination == ObjectType::Robot2
                    {
                        (track_kill)(who);
                        (track_kill)(destination);
                        new_arena.set(new_postions, ObjectType::Heap);
                    } else {
                        new_arena.set(new_postions, who);
                    }
                }
            }
        }

        if new_arena.get(player) == ObjectType::None {
            new_arena.set(player, ObjectType::Player);
        }

        new_arena
    }

    /// checks whether a given location is safe
    fn check_safe(&self, change: &ArenaChange) -> bool {
        let temp2_arena = Self::chase(
            &change.arena,
            |obj| obj == ObjectType::Robot1 || obj == ObjectType::Robot2,
            change.player,
            |_| {},
        );

        if temp2_arena.get(change.player) != ObjectType::Player {
            return false;
        }

        let temp3_arena = Self::chase(
            &temp2_arena,
            |obj| obj == ObjectType::Robot2,
            change.player,
            |_| {},
        );

        if temp3_arena.get(change.player) != ObjectType::Player {
            return false;
        }

        true
    }

    fn try_push_heap(&self, dx: i32, dy: i32) -> Option<ArenaChange> {
        if !self.config.borrow().moveable_heaps {
            return None;
        }

        let arena = self.arena.borrow();
        let coords = self.player.get().move_by(dx, dy);
        if !arena.are_coords_valid(coords) || arena.get(coords) != ObjectType::Heap {
            return None;
        }

        let push_to = coords.move_by(dx, dy);
        if !arena.are_coords_valid(push_to) || arena.get(push_to) == ObjectType::Heap {
            return None;
        }

        let new_arena = arena.map(|obj| {
            if obj != ObjectType::Player {
                obj
            } else {
                ObjectType::None
            }
        });
        new_arena.set(coords, ObjectType::Player);
        new_arena.set(push_to, ObjectType::Heap);
        Some(ArenaChange {
            arena: new_arena,
            player: coords,
            push: Some(push_to),
        })
    }

    /// Tries to move the player in a given direction
    fn try_player_move(&self, dx: i32, dy: i32) -> Option<ArenaChange> {
        let coords = self.player.get().move_by(dx, dy);

        let arena = self.arena.borrow();
        if !arena.are_coords_valid(coords) {
            return None;
        }

        if arena.get(coords) == ObjectType::Heap {
            self.try_push_heap(dx, dy)
        } else {
            Some(self.move_player_to(coords))
        }
    }

    /**
     * safe_move_available
     *
     * Description:
     * checks to see if a safe move was available to the player
     *
     * Returns:
     * TRUE if there is a possible safe move, FALSE otherwise
     **/
    fn safe_move_available(&self) -> bool {
        for dx in -1..=1 {
            for dy in -1..=1 {
                if let Some(change) = self.try_player_move(dx, dy) {
                    if self.check_safe(&change) {
                        return true;
                    }
                }
            }
        }
        false
    }

    fn move_player_to(&self, coords: Position) -> ArenaChange {
        let new_arena = self.arena.borrow().map(|obj| {
            if obj != ObjectType::Player {
                obj
            } else {
                ObjectType::None
            }
        });
        new_arena.put(coords, ObjectType::Player);
        ArenaChange {
            arena: new_arena,
            player: coords,
            push: None,
        }
    }

    /**
     * player_move
     * @dx: x direction
     * @dy: y direction
     *
     * Description:
     * moves the player in a given direction
     *
     * Returns:
     * TRUE if the player can move, FALSE otherwise
     **/
    fn player_move(&self, dx: i32, dy: i32, safety: MoveSafety) -> bool {
        let change = self.try_player_move(dx, dy);

        if let Some(change) = change {
            if safety != MoveSafety::Unsafe {
                if !self.check_safe(&change) {
                    if safety == MoveSafety::SuperSafe || self.safe_move_available() {
                        return false;
                    }
                }
            }

            self.splat.set(None);
            self.update_arena(change);

            true
        } else {
            false
        }
    }

    /**
     * random_teleport
     *
     * Description:
     * randomly teleports the player
     *
     * Returns:
     * TRUE if the player can be teleported, FALSE otherwise
     **/
    fn random_teleport(&self) -> bool {
        let Some(p) = self
            .arena
            .borrow()
            .random_vacant_position(&mut *self.rand.borrow_mut())
        else {
            /* This should never happen. */
            self.on_game_event.emit(GameEvent::NoTeleportLocations);
            return false;
        };

        let change = self.move_player_to(p);

        self.update_arena(change);
        self.splat.set(None);
        self.on_game_event.emit(GameEvent::Teleported);
        true
    }

    /**
     * safe_teleport
     *
     * Description:
     * teleports the player to safe location
     *
     * Returns:
     * TRUE if player can be teleported, FALSE otherwise
     **/
    fn safe_teleport(&self) -> bool {
        if self.safe_teleports.get() == 0 {
            return false;
        }

        let change = loop {
            let Some(p) = self
                .arena
                .borrow()
                .random_vacant_position(&mut *self.rand.borrow_mut())
            else {
                self.on_game_event.emit(GameEvent::NoSafeTeleportLocations);
                self.kill_player();
                return false;
            };

            let change = self.move_player_to(p);
            if self.check_safe(&change) {
                break change;
            }
        };

        self.safe_teleports.set(self.safe_teleports.get() - 1);

        self.update_arena(change);
        self.splat.set(None);
        self.on_game_event.emit(GameEvent::Teleported);

        true
    }

    /// handles player's commands
    pub fn player_command(
        &self,
        cmd: PlayerCommand,
        safety: MoveSafety, /* = MoveSafety::UNSAFE */
    ) -> bool {
        if self.state.get() != State::Playing {
            return false;
        }

        match cmd {
            PlayerCommand::NW
            | PlayerCommand::N
            | PlayerCommand::NE
            | PlayerCommand::W
            | PlayerCommand::Stay
            | PlayerCommand::E
            | PlayerCommand::SW
            | PlayerCommand::S
            | PlayerCommand::SE => {
                let direction = cmd.to_direction().unwrap();
                if self.player_move(direction.0, direction.1, safety) {
                    self.move_robots();
                    true
                } else {
                    false
                }
            }
            PlayerCommand::SafeTeleport => {
                if self.safe_teleport() {
                    self.move_robots();
                }
                true
            }
            PlayerCommand::RandomTeleport => {
                if self.random_teleport() {
                    self.move_robots();
                }
                true
            }
            PlayerCommand::Wait => {
                self.state.set(State::Waiting);
                true
            }
        }
    }
}
