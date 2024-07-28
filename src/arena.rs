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

use rand::Rng;
use std::cell::Cell;

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum ObjectType {
    PLAYER = 0,
    HEAP = 1,
    ROBOT1 = 2,
    ROBOT2 = 3,
    NONE = 99,
}

#[derive(Clone, Copy)]
pub struct Position {
    pub x: u32,
    pub y: u32,
}

impl Position {
    pub fn move_by(self, dx: i32, dy: i32) -> Self {
        Self {
            x: self.x.saturating_add_signed(dx),
            y: self.y.saturating_add_signed(dy),
        }
    }
}

pub struct Arena {
    width: u32,
    height: u32,
    objects: Vec<Cell<ObjectType>>,
}

impl Arena {
    pub fn new(width: u32, height: u32) -> Self {
        Self {
            width,
            height,
            objects: vec![Cell::new(ObjectType::NONE); (width * height) as usize],
        }
    }

    pub fn width(&self) -> u32 {
        self.width
    }

    pub fn height(&self) -> u32 {
        self.height
    }

    pub fn clear(&self) {
        for obj in &self.objects {
            obj.set(ObjectType::NONE);
        }
    }

    pub fn are_coords_valid(&self, position: Position) -> bool {
        position.x < self.width && position.y < self.height
    }

    fn index_of(&self, position: Position) -> Option<usize> {
        if self.are_coords_valid(position) {
            Some((position.x + position.y * self.width) as usize)
        } else {
            None
        }
    }

    pub fn get(&self, position: Position) -> ObjectType {
        if let Some(index) = self.index_of(position) {
            self.objects[index].get()
        } else {
            ObjectType::NONE
        }
    }

    pub fn set(&self, position: Position, obj: ObjectType) {
        if let Some(index) = self.index_of(position) {
            self.objects[index].set(obj);
        }
    }

    pub fn put(&self, position: Position, obj: ObjectType) -> bool {
        if let Some(index) = self.index_of(position) {
            if self.objects[index].get() == ObjectType::NONE {
                self.objects[index].set(obj);
                return true;
            }
        }
        false
    }

    pub fn map(&self, mapper: impl Fn(ObjectType) -> ObjectType) -> Self {
        let new_arena = Self {
            width: self.width,
            height: self.height,
            objects: self.objects.clone(),
        };
        for obj in &new_arena.objects {
            obj.set((mapper)(obj.get()));
        }
        new_arena
    }

    pub fn count(&self, predicate: impl Fn(ObjectType) -> bool) -> usize {
        let mut result = 0;
        for obj in &self.objects {
            if predicate(obj.get()) {
                result += 1;
            }
        }
        result
    }

    pub fn random_vacant_position(&self, rand: &mut dyn rand::RngCore) -> Option<Position> {
        let size = self.width * self.height;
        let start = rand.gen_range(0..size);
        for i in 0..size {
            let index = (start + i) % size;
            if self.objects[index as usize].get() == ObjectType::NONE {
                return Some(Position {
                    x: index % self.width,
                    y: index / self.width,
                });
            }
        }
        None
    }

    pub fn iterate_from(&self, x: u32, y: u32) -> ArenaIter<'_> {
        ArenaIter {
            arena: self,
            initial_position: self.index_of(Position { x, y }).unwrap(),
            i: 0,
            size: self.objects.len(),
        }
    }
}

pub struct ArenaIter<'arena> {
    arena: &'arena Arena,
    initial_position: usize,
    i: usize,
    size: usize,
}

impl<'arena> Iterator for ArenaIter<'arena> {
    type Item = Position;

    fn next(&mut self) -> Option<Self::Item> {
        if self.i >= self.size {
            return None;
        }
        let position = ((self.initial_position + self.i) % self.size) as u32;
        let result = Position {
            x: position % self.arena.width,
            y: position / self.arena.width,
        };
        self.i += 1;
        Some(result)
    }
}
