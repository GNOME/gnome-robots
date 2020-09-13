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

using Gtk;
using Gdk;
using Cairo;

/*
 * Size of the game playing area
 */
public const int GAME_WIDTH = 45;
public const int GAME_HEIGHT = 30;

public int tile_width = 0;
public int tile_height = 0;

Bubble aieee_bubble = null;
Bubble yahoo_bubble = null;
Bubble splat_bubble = null;

public void load_game_graphics () throws Error {
    yahoo_bubble = new Bubble.from_data_file ("yahoo.png");
    aieee_bubble = new Bubble.from_data_file ("aieee.png");
    splat_bubble = new Bubble.from_data_file ("splat.png");
}

public RGBA calculate_light_color (RGBA color) {
    /* While the two colours are labelled "light" and "dark" which one is
     * which actually depends on how light or dark the base colour is. */

    RGBA light = RGBA ();
    double brightness = color.red + color.green + color.blue;
    if (brightness > (1.0 / 1.1)) {
        /* Darken light colours. */
        light.red = 0.9 * color.red;
        light.green = 0.9 * color.green;
        light.blue = 0.9 * color.blue;
    } else if (brightness > 0.04) {
        /* Lighten darker colours. */
        light.red = 1.1 * color.red;
        light.green = 1.1 * color.green;
        light.blue = 1.1 * color.blue;
    } else {
        /* Very dark colours, add rather than multiply. */
        light.red = 0.04 + color.red;
        light.green = 0.04 + color.green;
        light.blue = 0.04 + color.blue;
    }
    light.alpha = 1.0;
    return light;
}

/**
 * clears the whole of the game area
 **/
public void clear_game_area () {
    if (game_area == null)
        return;

    game_area.queue_draw ();
}

