/*
 * Copyright 2022-2026 Andrey Kutejko <andy128k@gmail.com>
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

use gtk::{gdk, gio, graphene, gsk, prelude::*};
use std::cell::RefCell;
use std::error::Error;
use std::path::Path;
use std::rc::Rc;

#[derive(Clone)]
pub struct Image(Rc<ImageInner>);

struct ImageInner {
    image: gly::Image,
    texture: gdk::Texture,
    scaled: RefCell<Option<gdk::Texture>>,
}

impl Image {
    pub fn from_file(filename: &Path) -> Result<Self, Box<dyn Error>> {
        let file = gio::File::for_path(filename);
        let loader = gly::Loader::new(&file);
        let image = loader.load()?;
        let frame = image.next_frame()?;
        let texture = gly_gtk::frame_get_texture(&frame);
        Ok(Self(Rc::new(ImageInner {
            image,
            texture,
            scaled: Default::default(),
        })))
    }

    fn is_svg(&self) -> bool {
        const SVG_CONTENT_TYPE: &str = "image/svg+xml";
        self.0.image.mime_type() == SVG_CONTENT_TYPE
    }

    pub fn to_paintable(&self) -> gdk::Paintable {
        self.0.texture.clone().upcast()
    }

    pub fn scaled(
        &self,
        width: i32,
        height: i32,
        renderer: &gsk::Renderer,
    ) -> Result<gdk::Texture, Box<dyn Error>> {
        if let Some(texture) = self.0.scaled.borrow().as_ref()
            && texture.width() == width
            && texture.height() == height
        {
            return Ok(texture.clone());
        }

        let texture = if self.is_svg() {
            let request = gly::FrameRequest::new();
            request.set_scale(width as u32, height as u32);
            let frame = self.0.image.specific_frame(&request)?;
            gly_gtk::frame_get_texture(&frame)
        } else {
            let snapshot = gtk::Snapshot::new();
            let bounds = graphene::Rect::new(0.0, 0.0, width as f32, height as f32);
            snapshot.append_texture(&self.0.texture, &bounds);
            let node = snapshot.to_node().ok_or("No node")?;
            renderer.render_texture(node, None)
        };

        self.0.scaled.replace(Some(texture.clone()));

        Ok(texture)
    }
}
