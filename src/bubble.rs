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

use crate::image::Image;
use gtk::{gdk, graphene, prelude::*};
use std::error::Error;
use std::path::Path;

#[derive(Clone)]
pub struct Bubble {
    paintable: gdk::Paintable,
}

impl Bubble {
    pub fn from_file(filename: &Path) -> Result<Self, Box<dyn Error>> {
        Ok(Self {
            paintable: Image::from_file(filename)?.to_paintable(),
        })
    }

    pub fn draw(&self, snapshot: &gtk::Snapshot, x: f64, y: f64) {
        let x = x as f32;
        let y = y as f32;
        let width = self.paintable.intrinsic_width() as f32;
        let height = self.paintable.intrinsic_height() as f32;

        let bubble_width = width / 2.0;
        let bubble_height = height / 2.0;

        let clip_rect = graphene::Rect::new(
            if x < bubble_width {
                x
            } else {
                x - bubble_width
            },
            if y < bubble_height {
                y
            } else {
                y - bubble_height
            },
            bubble_width,
            bubble_height,
        );

        snapshot.push_clip(&clip_rect);
        snapshot.save();
        snapshot.translate(&graphene::Point::new(x - bubble_width, y - bubble_height));
        self.paintable
            .snapshot(snapshot, width as f64, height as f64);
        snapshot.restore();
        snapshot.pop();
    }
}
