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

use gtk::gdk;

pub fn calculate_contrast_color(color: &gdk::RGBA) -> gdk::RGBA {
    // While the two colours are labelled "light" and "dark" which one is
    // which actually depends on how light or dark the base colour is.

    let brightness = color.red() + color.green() + color.blue();
    if brightness > (1.0 / 1.1) {
        // Darken light colours.
        gdk::RGBA::new(
            0.9 * color.red(),
            0.9 * color.green(),
            0.9 * color.blue(),
            1.0,
        )
    } else if brightness > 0.04 {
        // Lighten darker colours.
        gdk::RGBA::new(
            1.1 * color.red(),
            1.1 * color.green(),
            1.1 * color.blue(),
            1.0,
        )
    } else {
        // Very dark colours, add rather than multiply.
        gdk::RGBA::new(
            0.04 + color.red(),
            0.04 + color.green(),
            0.04 + color.blue(),
            1.0,
        )
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_light_color() {
        let default = gdk::RGBA::parse("#7590AE").unwrap();
        assert_eq!(
            calculate_contrast_color(&default).to_string(),
            "rgb(105,130,157)"
        );

        let contrast = gdk::RGBA::parse("rgb(105,130,157)").unwrap();
        assert_eq!(
            calculate_contrast_color(&contrast).to_string(),
            "rgb(95,117,141)"
        );
    }
}
