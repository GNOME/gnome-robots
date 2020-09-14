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

public class GameArea : DrawingArea {

    const int MINIMUM_TILE_WIDTH = 8;
    const int MINIMUM_TILE_HEIGHT = 8;

    private int tile_width = 0;
    private int tile_height = 0;

    private GestureMultiPress click_controller;
    private EventControllerMotion motion_controller;
    private Game game;
    private Theme _theme;
    private RGBA light_background;
    private RGBA dark_background;
    private Bubble aieee_bubble;
    private Bubble yahoo_bubble;
    private Bubble splat_bubble;

    private Animated player_animation;
    private Animated player_dead_animation;
    private Animated robot1_animation;
    private Animated robot2_animation;

    public Theme theme {
        get {
            return _theme;
        }
        set {
            _theme = value;
            queue_draw ();
        }
    }

    public RGBA background_color {
        get { return dark_background; }
        set {
            dark_background = value;
            light_background = calculate_light_color (value);
            queue_draw ();
        }
    }

    public string background_color_name {
        owned get {
            return rgba_to_string (dark_background);
        }
        set {
            RGBA color = RGBA ();
            if (color.parse (name)) {
                background_color = color;
            }
        }
    }

    public GameArea (Game game,
                     Theme theme,
                     Bubble aieee_bubble,
                     Bubble yahoo_bubble,
                     Bubble splat_bubble
    ) {
        this.game = game;
        this.theme = theme;
        this.background_color_name = "#7590AE";
        this.aieee_bubble = aieee_bubble;
        this.yahoo_bubble = yahoo_bubble;
        this.splat_bubble = splat_bubble;

        add_events (Gdk.EventMask.BUTTON_PRESS_MASK | Gdk.EventMask.BUTTON_RELEASE_MASK | Gdk.EventMask.POINTER_MOTION_MASK);
        configure_event.connect (event => resize_cb (event));
        draw.connect ((cr) => draw_cb (cr));

        click_controller = new GestureMultiPress (this);
        click_controller.pressed.connect ((n_pressed, x, y) => mouse_cb (n_pressed, x, y));

        motion_controller = new EventControllerMotion (this);
        motion_controller.motion.connect ((x, y) => move_cb (x, y));

        set_size_request (MINIMUM_TILE_WIDTH * game.arena.width,
                          MINIMUM_TILE_HEIGHT * game.arena.height);

        player_animation = Animated
            .sequence (Theme.Frames.PLAYER_START, 0)
            .limit (20)
            .then (
                Animated
                    .bounce (Theme.Frames.PLAYER_START, Theme.Frames.NUM_PLAYER_ANIMATIONS)
                    .repeat (2)
            )
            .forever ();

        player_dead_animation = Animated
            .bounce (Theme.Frames.PLAYER_DEAD, Theme.Frames.NUM_PLAYER_DEAD_ANIMATIONS)
            .forever ();

        robot1_animation = Animated
            .sequence (Theme.Frames.ROBOT1_START)
            .limit (Theme.Frames.NUM_ROBOT1_ANIMATIONS)
            .forever ();

        robot2_animation = Animated
            .sequence (Theme.Frames.ROBOT2_START)
            .limit (Theme.Frames.NUM_ROBOT2_ANIMATIONS)
            .forever ();
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

        if (game.splat != null) {
            splat_bubble.draw (cr,
                               game.splat.x * tile_width + 8,
                               game.splat.y * tile_height + 8);
        }

        switch (game.get_state ()) {
        case Game.State.DEAD:
            aieee_bubble.draw (cr,
                               game.player.x * tile_width + 8,
                               game.player.y * tile_height + 4);
            break;
        case Game.State.COMPLETE:
            yahoo_bubble.draw (cr,
                               game.player.x * tile_width + 8,
                               game.player.y * tile_height + 4);
            break;
        default:
            break;
        }

        return true;
    }

    private void draw_object (int x, int y, ObjectType type, Context cr) {
        if ((x + y) % 2 != 0) {
            cairo_set_source_rgba (cr, dark_background);
        } else {
            cairo_set_source_rgba (cr, light_background);
        }

        x *= tile_width;
        y *= tile_height;

        cr.rectangle (x, y, tile_width, tile_height);
        cr.fill ();

        int animation = 0;
        switch (type) {
        case ObjectType.PLAYER:
            if (game.get_state () != Game.State.DEAD) {
                animation = player_animation.frame;
            } else {
                animation = player_dead_animation.frame;
            }
            break;
        case ObjectType.ROBOT1:
            animation = robot1_animation.frame;
            break;
        case ObjectType.ROBOT2:
            animation = robot2_animation.frame;
            break;
        case ObjectType.HEAP:
            animation = 0;
            break;
        case ObjectType.NONE:
            break;
        }

        cr.save ();
        cr.translate (x, y);
        theme.draw_object (type, animation, cr, tile_width, tile_height);
        cr.restore ();
    }

    public void tick () {
        player_animation.tick ();
        player_dead_animation.tick ();
        robot1_animation.tick ();
        robot2_animation.tick ();
    }

    private void mouse_cb (int n_press, double x, double y) {
        if (game.get_state () != Game.State.PLAYING) {
            return;
        }

        int dx, dy;
        get_dir (x, y, out dx, out dy);

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
            get_dir (x, y, out dx, out dy);
            set_cursor_by_direction (window, dx, dy);
        }
    }

    private void get_dir (double ix, double iy, out int odx, out int ody) {
        const int[,] MOVE_TABLE = {
            {-1, 0}, {-1, -1}, {0, -1}, {1, -1},
            {1, 0}, {1, 1}, {0, 1}, {-1, 1}
        };
        int x = ((int) (ix / tile_width)).clamp (0, game.arena.width);
        int y = ((int) (iy / tile_height)).clamp (0, game.arena.height);

        /* If we click on our man then we assume we hold. */
        if ((x == game.player.x) && (y == game.player.y)) {
            odx = 0;
            ody = 0;
            return;
        }

        /* If the square clicked on is a valid move, go there. */
        int idx = x - game.player.x;
        int idy = y - game.player.y;
        if (idx.abs () < 2 && idy.abs () < 2) {
            odx = idx;
            ody = idy;
            return;
        }

        /* Otherwise go in the general direction of the mouse click. */
        double dx = ix - (game.player.x + 0.5) * tile_width;
        double dy = iy - (game.player.y + 0.5) * tile_height;

        double angle = Math.atan2 (dy, dx);

        /* Note the adjustment we have to make (+9, not +8) because atan2's idea
         * of octants and the ones we want are shifted by PI/8. */
        int octant = (((int) Math.floor (8.0 * angle / Math.PI) + 9) / 2) % 8;

        odx = MOVE_TABLE[octant, 0];
        ody = MOVE_TABLE[octant, 1];
    }
}

