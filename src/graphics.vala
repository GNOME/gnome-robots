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

public enum BubbleType {
    NONE = 0,
    YAHOO,
    AIEEE,
    SPLAT,
}

public const int BUBBLE_WIDTH = 86;
public const int BUBBLE_HEIGHT = 34;
public const int BUBBLE_XOFFSET = 8;
public const int BUBBLE_YOFFSET = 4;

public int tile_width = 0;
public int tile_height = 0;

Pixbuf aieee_pixbuf = null;
Pixbuf yahoo_pixbuf = null;
Pixbuf splat_pixbuf = null;

int bubble_xpos = 0;
int bubble_ypos = 0;
int bubble_xo = 0;
int bubble_yo = 0;
BubbleType bubble_type = BubbleType.NONE;

public void load_game_graphics () throws Error {
    yahoo_pixbuf = new Pixbuf.from_file (
        GLib.Path.build_filename (DATA_DIRECTORY, "pixmaps", "yahoo.png"));
    aieee_pixbuf = new Pixbuf.from_file (
        GLib.Path.build_filename (DATA_DIRECTORY, "pixmaps", "aieee.png"));
    splat_pixbuf = new Pixbuf.from_file (
        GLib.Path.build_filename (DATA_DIRECTORY, "pixmaps", "splat.png"));
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

/**
 * Draws a bubble if there is one
 **/
public void draw_bubble (Context cr) {
    if (bubble_type == BubbleType.NONE)
        return;

    Pixbuf pmap;
    if (bubble_type == BubbleType.YAHOO) {
        pmap = yahoo_pixbuf;
    } else if (bubble_type == BubbleType.AIEEE) {
        pmap = aieee_pixbuf;
    } else {
        pmap = splat_pixbuf;
    }

    cairo_set_source_pixbuf (cr, pmap, bubble_xpos - bubble_xo, bubble_ypos - bubble_yo);
    cr.rectangle (bubble_xpos, bubble_ypos, BUBBLE_WIDTH, BUBBLE_HEIGHT);
    cr.fill ();
}

/**
 * add_bubble
 * @x: x position
 * @y: y position
 *
 * Description:
 * adds a bubble at @x,@y
 **/
void add_bubble (BubbleType type, int x, int y) {
    bubble_type = type;
    bubble_xpos = x * tile_width - BUBBLE_WIDTH + BUBBLE_XOFFSET;
    bubble_ypos = y * tile_height - BUBBLE_HEIGHT + BUBBLE_YOFFSET;

    bubble_xo = 0;
    bubble_yo = 0;

    if (bubble_ypos < 0) {
        bubble_yo = BUBBLE_HEIGHT;
        bubble_ypos += BUBBLE_HEIGHT;
    }
    if (bubble_xpos < 0) {
        bubble_xo = BUBBLE_WIDTH;
        bubble_xpos += BUBBLE_WIDTH;
    }
    game_area.queue_draw ();
}

/**
 * remove_bubble
 *
 * Description:
 * removes all types of bubble
 **/
public void remove_bubble () {
    if (bubble_type == BubbleType.NONE)
        return;

    bubble_type = BubbleType.NONE;
    game_area.queue_draw ();
}

/**
 * removes a splat bubble if there is one
 **/
public void remove_splat_bubble () {
    if (bubble_type != BubbleType.SPLAT)
        return;

    bubble_type = BubbleType.NONE;
    game_area.queue_draw ();
}


/**
 * add_yahoo_bubble
 * @x: x position
 * @y: y position
 *
 * Description:
 * adds and "Yahoo" bubble at @x,@y
 **/
public void add_yahoo_bubble (int x, int y) {
    add_bubble (BubbleType.YAHOO, x, y);
}


/**
 * add_aieee_bubble
 * @x: x position
 * @y: y position
 *
 * Description:
 * adds and "Aieee" bubble at @x,@y
 **/
public void add_aieee_bubble (int x, int y) {
    add_bubble (BubbleType.AIEEE, x, y);
}

/**
 * add_splat_bubble
 * @x: x position
 * @y: y position
 *
 * Description:
 * adds a "Splat" speech bubble at @x,@y
 **/
public void add_splat_bubble (int x, int y) {
    add_bubble (BubbleType.SPLAT, x, y);
    bubble_ypos += BUBBLE_YOFFSET;
}

