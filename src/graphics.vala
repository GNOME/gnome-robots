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

/*
 * Scenario pixmaps
 */
public const int SCENARIO_PIXMAP_WIDTH = 14;
public const int SCENARIO_PLAYER_START = 0;
public const int SCENARIO_ROBOT1_START = 5;
public const int SCENARIO_ROBOT2_START = 9;
public const int SCENARIO_HEAP_POS     = 13;

public const int NUM_ROBOT_ANIMATIONS  = 4;
public const int NUM_PLAYER_ANIMATIONS = 4;
public const int PLAYER_WAVE_WAIT      = 20;
public const int PLAYER_NUM_WAVES      = 2;

public int tile_width = 0;
public int tile_height = 0;

GamesPreimage theme_preimage = null;
Pixbuf theme_pixbuf = null;
bool rerender_needed = true;

RGBA light_background;
RGBA dark_background;

Pixbuf aieee_pixbuf = null;
Pixbuf yahoo_pixbuf = null;
Pixbuf splat_pixbuf = null;

int robot_animation = 0;
int player_animation = 0;
int player_num_waves = 0;
int player_wave_wait = 0;
int player_wave_dir = 1;

int bubble_xpos = 0;
int bubble_ypos = 0;
int bubble_xo = 0;
int bubble_yo = 0;
BubbleType bubble_type = BubbleType.NONE;

void render_graphics () {
    theme_pixbuf = theme_preimage.render (14 * tile_width, tile_height);
    rerender_needed = false;
}

/**
 * Loads all of the 'speech bubble' graphics
 **/
void load_bubble_graphics () throws Error {
    yahoo_pixbuf = new Pixbuf.from_file (
        GLib.Path.build_filename (DATA_DIRECTORY, "pixmaps", "yahoo.png"));
    aieee_pixbuf = new Pixbuf.from_file (
        GLib.Path.build_filename (DATA_DIRECTORY, "pixmaps", "aieee.png"));
    splat_pixbuf = new Pixbuf.from_file (
        GLib.Path.build_filename (DATA_DIRECTORY, "pixmaps", "splat.png"));
}


/**
 * load_game_graphics
 *
 * Description:
 * Loads all of the game graphics
 *
 * Returns:
 * TRUE on success FALSE otherwise
 **/
public void load_game_graphics () throws Error {
    if (theme_preimage != null) {
        free_game_graphics ();
    }

    var themedir = GLib.Path.build_filename (DATA_DIRECTORY, "themes");
    var filename = games_find_similar_file (properties_theme_name (), themedir);

    try {
        theme_preimage = new GamesPreimage.from_file (filename);
    } catch (Error e) {
        filename = games_find_similar_file ("robots", themedir);
        theme_preimage = new GamesPreimage.from_file (filename);
    }

    load_bubble_graphics ();

    rerender_needed = true;
}

/**
 * Frees all of the resources used by the game graphics
 **/
public void free_game_graphics () {
    theme_preimage = null;
    theme_pixbuf = null;
    aieee_pixbuf = null;
    yahoo_pixbuf = null;
    splat_pixbuf = null;
}

public void set_background_color (RGBA color) {
    if (game_area == null)
        return;

    /* While the two colours are labelled "light" and "dark" which one is
     * which actually depends on how light or dark the base colour is. */

    double brightness = color.red + color.green + color.blue;
    if (brightness > (1.0 / 1.1)) {
        /* Darken light colours. */
        light_background.red = 0.9 * color.red;
        light_background.green = 0.9 * color.green;
        light_background.blue = 0.9 * color.blue;
    } else if (brightness > 0.04) {
        /* Lighten darker colours. */
        light_background.red = 1.1 * color.red;
        light_background.green = 1.1 * color.green;
        light_background.blue = 1.1 * color.blue;
    } else {
        /* Very dark colours, add rather than multiply. */
        light_background.red += 0.04;
        light_background.green += 0.04;
        light_background.blue += 0.04;
    }
    light_background.alpha = 1.0;
    dark_background = color;

    clear_game_area ();
}

public void set_background_color_from_name (string name) {
    RGBA color = RGBA ();
    if (!color.parse (name)) {
        color.parse ("#7590AE");
    }
    set_background_color (color);
}

/**
 * draw_tile_pixmap
 * @tileno: Graphics tile number
 * @pno: Number of graphics set
 * @x: x position in grid squares
 * @y: y position in grid squares
 * @cr: context to draw on
 *
 * Description:
 * Draws tile pixmap @tileno from graphics set @pno at (@x, @y) in
 * a widget @area
 **/
void draw_tile_pixmap (int tileno, int x, int y, Context cr) {
    if ((x + y) % 2 != 0) {
        cairo_set_source_rgba (cr, dark_background);
    } else {
        cairo_set_source_rgba (cr, light_background);
    }

    x *= tile_width;
    y *= tile_height;

    cr.rectangle (x, y, tile_width, tile_height);
    cr.fill ();

    if (rerender_needed)
        render_graphics ();

    if ((tileno < 0) || (tileno >= SCENARIO_PIXMAP_WIDTH)) {
        /* nothing */
    } else {
        cairo_set_source_pixbuf (cr, theme_pixbuf, x - tileno * tile_width, y);
        cr.rectangle (x, y, tile_width, tile_height);
        cr.fill ();
    }
}


/**
 * draw_object
 * @x: x position
 * @y: y position
 * @type: object type
 * @cr: context to draw on
 *
 * Description:
 * Draws graphics for an object at specified location
 **/
public void draw_object (int x, int y, ObjectType type, Context cr) {
    if (game_area == null)
        return;

    switch (type) {
    case ObjectType.PLAYER:
        draw_tile_pixmap (SCENARIO_PLAYER_START + player_animation, x, y, cr);
        break;
    case ObjectType.ROBOT1:
        draw_tile_pixmap (SCENARIO_ROBOT1_START + robot_animation, x, y, cr);
        break;
    case ObjectType.ROBOT2:
        draw_tile_pixmap (SCENARIO_ROBOT2_START + robot_animation, x, y, cr);
        break;
    case ObjectType.HEAP:
        draw_tile_pixmap (SCENARIO_HEAP_POS, x, y, cr);
        break;
    case ObjectType.NONE:
        draw_tile_pixmap (-1, x, y, cr);
        break;
    }
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
 * reset_player_animation
 *
 * Description:
 * resets player animation to standing position
 **/
public void reset_player_animation () {
    player_wave_wait = 0;
    player_num_waves = 0;
    player_wave_dir = 1;
    player_animation = 0;
}


/**
 * player_animation_dead
 *
 * Description:
 * sets player animation to be dead
 **/
public void player_animation_dead () {
    player_wave_wait = 0;
    player_num_waves = 0;
    player_wave_dir = 1;
    player_animation = NUM_PLAYER_ANIMATIONS;
}


/**
 * animate_game_graphics
 *
 * Description:
 * updates animation for object graphics
 **/
public void animate_game_graphics () {
  ++robot_animation;
  if (robot_animation >= NUM_ROBOT_ANIMATIONS) {
    robot_animation = 0;
  }

  if (player_animation == NUM_PLAYER_ANIMATIONS) {
    /* do nothing */
  } else if (player_wave_wait < PLAYER_WAVE_WAIT) {
    ++player_wave_wait;
    player_animation = 0;
  } else {
    player_animation += player_wave_dir;
    if (player_animation >= NUM_PLAYER_ANIMATIONS) {
      player_wave_dir = -1;
      player_animation -= 2;
    } else if (player_animation < 0) {
      player_wave_dir = 1;
      player_animation = 1;
      ++player_num_waves;
      if (player_num_waves >= PLAYER_NUM_WAVES) {
        reset_player_animation ();
      }
    }
  }
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

