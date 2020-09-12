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
using Cairo;

public class Theme {

    public enum Frames {
        PLAYER_START = 0,
        PLAYER_DEAD = 4,
        ROBOT1_START = 5,
        ROBOT2_START = 9,
        HEAP_START = 13,
        COUNT = 14,

        NUM_PLAYER_ANIMATIONS = PLAYER_DEAD - PLAYER_START,
        NUM_PLAYER_DEAD_ANIMATIONS = ROBOT1_START - PLAYER_DEAD,
        NUM_PLAYER_TILES = ROBOT1_START - PLAYER_START,
        NUM_ROBOT1_ANIMATIONS = ROBOT2_START - ROBOT1_START,
        NUM_ROBOT2_ANIMATIONS = HEAP_START - ROBOT2_START,
        NUM_HEAP_ANIMATIONS = COUNT - HEAP_START,
    }

    private GamesPreimage preimage;
    private Pixbuf pixbuf;
    private int tile_width;
    private int tile_height;

    public Theme.from_file (string path) throws Error {
        preimage = new GamesPreimage.from_file (path);
        pixbuf = null;
    }

    public void draw_object (ObjectType type,
                             int frame_no,
                             Context cr,
                             int width,
                             int height
    ) {
        int tile_no = -1;
        switch (type) {
        case ObjectType.PLAYER:
            tile_no = Frames.PLAYER_START + frame_no % Frames.NUM_PLAYER_TILES;
            break;
        case ObjectType.ROBOT1:
            tile_no = Frames.ROBOT1_START + frame_no % Frames.NUM_ROBOT1_ANIMATIONS;
            break;
        case ObjectType.ROBOT2:
            tile_no = Frames.ROBOT2_START + frame_no % Frames.NUM_ROBOT2_ANIMATIONS;
            break;
        case ObjectType.HEAP:
            tile_no = Frames.HEAP_START + frame_no % Frames.NUM_HEAP_ANIMATIONS;
            break;
        case ObjectType.NONE:
            return;
        }

        if (pixbuf == null || tile_width != width || tile_height != height) {
            tile_width = width;
            tile_height = height;
            pixbuf = preimage.render (Frames.COUNT * tile_width, tile_height);
        }

        cairo_set_source_pixbuf (cr, pixbuf, - tile_no * tile_width, 0);
        cr.rectangle (0, 0, tile_width, tile_height);
        cr.fill ();
    }
}

