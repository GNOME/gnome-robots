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

use gtk::{cairo, gdk, gdk::subclass::prelude::*, glib, graphene};
use std::rc::Rc;

mod imp {
    use super::*;
    use gtk::prelude::SnapshotExt;
    use std::cell::OnceCell;

    #[derive(Default)]
    pub struct SvgPaintable {
        pub handle: OnceCell<Rc<rsvg::SvgHandle>>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for SvgPaintable {
        const NAME: &'static str = "SvgPaintable";
        type Type = super::SvgPaintable;
        type Interfaces = (gdk::Paintable,);
    }

    impl ObjectImpl for SvgPaintable {}

    impl PaintableImpl for SvgPaintable {
        fn intrinsic_width(&self) -> i32 {
            let Some(handle) = self.handle.get() else {
                eprintln!("SvgPaintable is not initialized");
                return 0;
            };
            let renderer = rsvg::CairoRenderer::new(handle);
            match renderer.intrinsic_size_in_pixels() {
                Some((width, _height)) => width as i32,
                _ => 0,
            }
        }

        fn intrinsic_height(&self) -> i32 {
            let Some(handle) = self.handle.get() else {
                eprintln!("SvgPaintable is not initialized");
                return 0;
            };
            let renderer = rsvg::CairoRenderer::new(handle);
            match renderer.intrinsic_size_in_pixels() {
                Some((_width, height)) => height as i32,
                _ => 0,
            }
        }

        fn snapshot(&self, snapshot: &gdk::Snapshot, width: f64, height: f64) {
            let Some(handle) = self.handle.get() else {
                eprintln!("SvgPaintable is not initialized");
                return;
            };

            let rect = graphene::Rect::new(0.0, 0.0, width as f32, height as f32);
            let cr = snapshot.append_cairo(&rect);

            let rect = cairo::Rectangle::new(0.0, 0.0, width, height);
            let renderer = rsvg::CairoRenderer::new(handle);
            if let Err(error) = renderer.render_document(&cr, &rect) {
                eprintln!("SVG rendering error: {}", error);
            }
        }
    }
}

glib::wrapper! {
    pub struct SvgPaintable(ObjectSubclass<imp::SvgPaintable>) @implements gdk::Paintable;
}

impl SvgPaintable {
    pub fn new(handle: &Rc<rsvg::SvgHandle>) -> Self {
        let this: Self = glib::Object::builder().build();
        this.imp().handle.set(handle.clone()).ok().unwrap();
        this
    }
}
