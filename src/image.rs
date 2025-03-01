/*
 * Copyright 2022-2024 Andrey Kutejko <andy128k@gmail.com>
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

use crate::svg_paintable::SvgPaintable;
use gtk::{cairo, gdk, gdk_pixbuf, glib, prelude::*};
use std::cell::RefCell;
use std::error::Error;
use std::path::Path;
use std::rc::Rc;

#[derive(Clone)]
pub struct Image(Rc<ImageInner>);

struct ImageInner {
    origin: ImageOrigin,
    scaled: RefCell<Option<gdk::Texture>>,
}

enum ImageOrigin {
    Svg(Rc<rsvg::SvgHandle>),
    Pixbuf(gdk_pixbuf::Pixbuf),
}

impl Image {
    fn new_svg(handle: rsvg::SvgHandle) -> Self {
        Self(Rc::new(ImageInner {
            origin: ImageOrigin::Svg(Rc::new(handle)),
            scaled: Default::default(),
        }))
    }

    fn new_pixbuf(pixbuf: gdk_pixbuf::Pixbuf) -> Self {
        Self(Rc::new(ImageInner {
            origin: ImageOrigin::Pixbuf(pixbuf),
            scaled: Default::default(),
        }))
    }

    pub fn from_file(filename: &Path) -> Result<Self, Box<dyn Error>> {
        rsvg::Loader::new()
            .read_path(filename)
            .map(|handle| Self::new_svg(handle))
            .or_else(|_svg_error| {
                let pixbuf = gdk_pixbuf::Pixbuf::from_file(filename)?;
                Ok(Self::new_pixbuf(pixbuf))
            })
    }

    pub fn to_paintable(&self) -> gdk::Paintable {
        match &self.0.origin {
            ImageOrigin::Svg(handle) => SvgPaintable::new(handle).upcast(),
            ImageOrigin::Pixbuf(pixbuf) => gdk::Texture::for_pixbuf(pixbuf).upcast(),
        }
    }

    pub fn scaled(&self, width: i32, height: i32) -> Result<gdk::Texture, Box<dyn Error>> {
        if let Some(texture) = self.0.scaled.borrow().as_ref() {
            if texture.width() == width && texture.height() == height {
                return Ok(texture.clone());
            }
        }

        let texture = match &self.0.origin {
            ImageOrigin::Svg(handle) => {
                let surface = cairo::ImageSurface::create(cairo::Format::ARgb32, width, height)?;

                let cr = cairo::Context::new(&surface)?;
                let rect = cairo::Rectangle::new(0.0, 0.0, width as f64, height as f64);
                let renderer = rsvg::CairoRenderer::new(handle);
                renderer.render_document(&cr, &rect)?;
                drop(cr);

                let stride = surface.stride() as usize;
                let data = surface.take_data()?;
                let bytes = glib::Bytes::from_owned(data.as_ref().to_vec());

                gdk::MemoryTexture::new(
                    width,
                    height,
                    gdk::MemoryFormat::B8g8r8a8Premultiplied,
                    &bytes,
                    stride,
                )
                .upcast()
            }
            ImageOrigin::Pixbuf(pixbuf) => gdk::Texture::for_pixbuf(
                &pixbuf
                    .scale_simple(width, height, gdk_pixbuf::InterpType::Bilinear)
                    .ok_or("Scale failed")?,
            ),
        };

        self.0.scaled.replace(Some(texture.clone()));

        Ok(texture)
    }
}
