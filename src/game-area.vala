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
using Cairo;

public class GameArea : DrawingArea {

    const int MINIMUM_TILE_WIDTH = 8;
    const int MINIMUM_TILE_HEIGHT = 8;

    private GestureMultiPress click_controller;
    private EventControllerMotion motion_controller;
    private Game game;

    public GameArea (Game game) {
        this.game = game;

        add_events (Gdk.EventMask.BUTTON_PRESS_MASK | Gdk.EventMask.BUTTON_RELEASE_MASK | Gdk.EventMask.POINTER_MOTION_MASK);
        configure_event.connect (event => resize_cb (event));
        draw.connect ((cr) => draw_cb (cr));

        click_controller = new GestureMultiPress (this);
        click_controller.pressed.connect ((n_pressed, x, y) => mouse_cb (n_pressed, x, y));

        motion_controller = new EventControllerMotion (this);
        motion_controller.motion.connect ((x, y) => move_cb (x, y));

        set_size_request (MINIMUM_TILE_WIDTH * game.arena.width,
                          MINIMUM_TILE_HEIGHT * game.arena.height);
    }

    private bool resize_cb (Gdk.EventConfigure e) {
        int trial_width = e.width / game.arena.width;
        int trial_height = e.height / game.arena.height;

        if (trial_width != tile_width || trial_height != tile_height) {
            tile_width = trial_width;
            tile_height = trial_height;
        }

        return false;
    }

    private bool draw_cb (Context cr) {
        for (int j = 0; j < game.arena.height; j++) {
            for (int i = 0; i < game.arena.width; i++) {
                draw_object (i, j, game.check_location (i, j), cr);
            }
        }

        draw_bubble (cr);

        return true;
    }

    private void mouse_cb (int n_press, double x, double y) {
        if (game.get_state () != Game.State.PLAYING) {
            return;
        }

        int dx, dy;
        game.get_dir ((int)x, (int)y, out dx, out dy);

        if (game.player_move (dx, dy)) {
            game.move_robots ();
        }
    }

    private void move_cb (double x, double y) {
        var window = get_window ();
        if (game.get_state () != Game.State.PLAYING) {
            set_cursor_default (window);
        } else {
            int dx, dy;
            game.get_dir ((int)x, (int)y, out dx, out dy);
            set_cursor_by_direction (window, dx, dy);
        }
    }
}

