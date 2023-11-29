/*
 * Copyright 2020 Andrey Kutejko <andy128k@gmail.com>
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

using Gdk;

public RGBA string_to_rgba (string color) {
    RGBA rgba = RGBA ();
    rgba.parse (color);
    return rgba;
}

public string rgba_to_string (RGBA color) {
    return "#%04x%04x%04x".printf (
        (int) (color.red * 65535 + 0.5),
        (int) (color.green * 65535 + 0.5),
        (int) (color.blue * 65535 + 0.5));
}

public RGBA calculate_light_color (RGBA color) {
    /* While the two colours are labelled "light" and "dark" which one is
     * which actually depends on how light or dark the base colour is. */

    RGBA light = RGBA ();
    double brightness = color.red + color.green + color.blue;
    if (brightness > (1.0 / 1.1)) {
        /* Darken light colours. */
        light.red = 0.9f * color.red;
        light.green = 0.9f * color.green;
        light.blue = 0.9f * color.blue;
    } else if (brightness > 0.04) {
        /* Lighten darker colours. */
        light.red = 1.1f * color.red;
        light.green = 1.1f * color.green;
        light.blue = 1.1f * color.blue;
    } else {
        /* Very dark colours, add rather than multiply. */
        light.red = 0.04f + color.red;
        light.green = 0.04f + color.green;
        light.blue = 0.04f + color.blue;
    }
    light.alpha = 1.0f;
    return light;
}
