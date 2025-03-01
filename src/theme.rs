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

use crate::animation::{self, Animation, InfiniteAnimation};
use crate::arena::ObjectType;
use crate::image::Image;
use gtk::{glib, glib::subclass::prelude::*, graphene, prelude::*};
use std::error::Error;
use std::ops::Range;
use std::path::{Path, PathBuf};

pub const PLAYER_ANIMATION_FRAMES: Range<usize> = 0..4;
pub const PLAYER_DEAD_ANIMATION_FRAMES: Range<usize> = 4..5;
pub const ROBOT1_ANIMATION_FRAMES: Range<usize> = 5..9;
pub const ROBOT2_ANIMATION_FRAMES: Range<usize> = 9..13;
pub const HEAP_ANIMATION_FRAMES: Range<usize> = 13..14;
pub const FRAMES_COUNT: usize = 14;

mod imp {
    use super::*;
    use std::cell::OnceCell;

    #[derive(glib::Properties, Default)]
    #[properties(wrapper_type = super::Theme)]
    pub struct Theme {
        pub image: OnceCell<Image>,
        #[property(get, construct_only)]
        pub path: OnceCell<PathBuf>,
        #[property(get, construct_only)]
        pub name: OnceCell<String>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for Theme {
        const NAME: &'static str = "RobotsTheme";
        type Type = super::Theme;
        type ParentType = glib::Object;
    }

    #[glib::derived_properties]
    impl ObjectImpl for Theme {}
}

glib::wrapper! {
    pub struct Theme(ObjectSubclass<imp::Theme>);
}

impl Theme {
    pub fn from_file(path: &Path) -> Result<Self, Box<dyn Error>> {
        let image = Image::from_file(path)?;
        let name = path
            .file_stem()
            .ok_or("Bad file name")?
            .to_string_lossy()
            .replace("_", " ");
        let this: Self = glib::Object::builder()
            .property("path", path)
            .property("name", name)
            .build();
        this.imp().image.set(image).ok().unwrap();
        Ok(this)
    }

    pub fn draw_object(
        &self,
        r#type: ObjectType,
        frame_no: usize,
        snapshot: &gtk::Snapshot,
        rect: &graphene::Rect,
    ) {
        let tile_no = match r#type {
            ObjectType::Player => frame_index(PLAYER_ANIMATION_FRAMES, frame_no),
            ObjectType::Robot1 => frame_index(ROBOT1_ANIMATION_FRAMES, frame_no),
            ObjectType::Robot2 => frame_index(ROBOT2_ANIMATION_FRAMES, frame_no),
            ObjectType::Heap => frame_index(HEAP_ANIMATION_FRAMES, frame_no),
            ObjectType::None => return,
        };

        snapshot.push_clip(rect);

        snapshot.save();
        let frame_offset = (tile_no as i32) * (rect.width() as i32);
        let texture_width = (rect.width() as i32) * (FRAMES_COUNT as i32);
        let texture_height = rect.height() as i32;
        let image = self.imp().image.get().unwrap();
        match image.scaled(texture_width, texture_height) {
            Ok(texture) => {
                snapshot.translate(&graphene::Point::new(
                    rect.x() - frame_offset as f32,
                    rect.y(),
                ));
                texture.snapshot(snapshot, texture_width as f64, texture_height as f64);
            }
            Err(error) => {
                eprintln!("Failed to get a scaled texture: {error}");
            }
        }
        snapshot.restore();

        snapshot.pop();
    }
}

fn frame_index(range: Range<usize>, frame_no: usize) -> usize {
    range.start + frame_no % range.len()
}

pub fn player_animation() -> impl InfiniteAnimation {
    animation::sequence(PLAYER_ANIMATION_FRAMES)
        .limit(1)
        .repeat(20)
        .then(animation::bounce(PLAYER_ANIMATION_FRAMES).repeat(2))
        .forever()
}

pub fn player_dead_animation() -> impl InfiniteAnimation {
    animation::bounce(PLAYER_DEAD_ANIMATION_FRAMES).forever()
}

pub fn robot1_animation() -> impl InfiniteAnimation {
    animation::sequence(ROBOT1_ANIMATION_FRAMES).forever()
}

pub fn robot2_animation() -> impl InfiniteAnimation {
    animation::sequence(ROBOT2_ANIMATION_FRAMES).forever()
}
