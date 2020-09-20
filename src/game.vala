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

using Games;

public const int ANIMATION_DELAY = 100;
public const int DEAD_DELAY = 30;
public const int CHANGE_DELAY = 20;
public const int WAITING_DELAY = 1;

public Game game = null;

public class Game {

    /*
     * Size of the game playing area
     */
    public const int GAME_WIDTH = 45;
    public const int GAME_HEIGHT = 30;

    public enum State {
        PLAYING = 1,
        WAITING,
        COMPLETE,
        DEAD,
        ROBOT,
        TYPE2,
        WTYPE2,
    }

    public enum KeyboardControl {
        NW = 0,
        N,
        NE,
        W,
        STAY,
        E,
        SW,
        S,
        SE,
        TELE,
        RTEL,
        WAIT,
    }

    Rand rand;
    public State state = State.PLAYING;
    public Arena arena;
    public GameConfig config { get; set; }
    public int width {
        get { return arena.width; }
    }
    public int height {
        get { return arena.height; }
    }
    public Arena.Coords player { get; private set; }
    public Arena.Coords? splat { get; private set; }

    int endlev_counter = 0;
    int current_level = 0;
    int score = 0;
    int kills = 0;
    int score_step = 0;
    int safe_teleports = 0;
    uint game_timer_id = -1;

    struct ArenaChange {
        Arena arena;
        Arena.Coords player;
        Arena.Coords? push;
    }

    public Game () {
        arena = new Arena (GAME_WIDTH, GAME_HEIGHT);
        rand = new Rand ();
    }

    public State get_state () {
        return state;
    }

    /**
     * Displays the high-score table
     **/
    public void show_scores () {
        highscores.run_dialog ();
    }

    /**
     * Enters a score in the high-score table
     **/
    void log_score (int sc) {
        string key;
        if (properties_super_safe_moves ()) {
            key = config.description + "-super-safe";
        } else if (properties_safe_moves ()) {
            key = config.description + "-safe";
        } else {
            key = config.description;
        }

        if (sc != 0) {
            string name = category_name_from_key (key);
            var category = new Scores.Category (key, name);
            highscores.add_score.begin (sc, category, null, (ctx, res) => {
                try {
                    highscores.add_score.end (res);
                } catch (Error error) {
                    warning ("Failed to add score: %s", error.message);
                }
            });
        }
    }

    /**
     * Ends the current game.
     **/
    void kill_player () {
        state = State.DEAD;
        play_sound (Sound.DIE);
        arena[player.x, player.y] = ObjectType.PLAYER;
        endlev_counter = 0;
        set_move_action_sensitivity (false);
        game_area.queue_draw ();
    }

    /**
     * add_kill
     * @type: robot type
     *
     * Description:
     * registers a robot kill and updates the score
     **/
    void add_kill (ObjectType type) {
        int si;
        if ((state == State.WAITING) || (state == State.WTYPE2)) {
            if (type == ObjectType.ROBOT1) {
                si = config.score_type1_waiting;
                kills += 1;
            } else {
                si = config.score_type2_waiting;
                kills += 2;
            }
        } else {
            if (type == ObjectType.ROBOT1) {
                si = config.score_type1;
            } else {
                si = config.score_type2;
            }
        }

        score += si;
        score_step += si;

        if (config.safe_score_boundary > 0) {
            while (score_step >= config.safe_score_boundary) {
                safe_teleports += 1;
                score_step -= config.safe_score_boundary;
            }
        }

        if (config.num_robots_per_safe > 0) {
            while (kills >= config.num_robots_per_safe) {
                safe_teleports += 1;
                kills -= config.num_robots_per_safe;
            }
        }

        if (safe_teleports > config.max_safe_teleports) {
            safe_teleports = config.max_safe_teleports;
        }

        update_game_status (score, current_level + 1, safe_teleports);
    }

    private int max_robots () {
        return arena.width * arena.height / 2;
    }

    /**
     * generate_level
     *
     * Description:
     * Creates a new level and populates it with robots
     **/
    void generate_level () {
        arena.clear ();

        player = Arena.Coords () {
            x = arena.width / 2,
            y = arena.height / 2
        };
        arena[player.x, player.y] = ObjectType.PLAYER;

        var num_robots1 = config.initial_type1 + config.increment_type1 * current_level;

        if (num_robots1 > config.maximum_type1) {
            num_robots1 = config.maximum_type1;
        }
        if (num_robots1 > max_robots ()) {
            current_level = 0;
            num_robots1 = config.initial_type1;
            message_box (_("Congratulations, You Have Defeated the Robots!! \nBut Can You do it Again?"));
            play_sound (Sound.VICTORY);
        }

        var num_robots2 = config.initial_type2 + config.increment_type2 * current_level;

        if (num_robots2 > config.maximum_type2) {
            num_robots2 = config.maximum_type2;
        }

        if ((num_robots1 + num_robots2) > max_robots ()) {
            current_level = 0;
            num_robots1 = config.initial_type1;
            num_robots2 = config.initial_type2;
            message_box (_("Congratulations, You Have Defeated the Robots!! \nBut Can You do it Again?"));
            play_sound (Sound.VICTORY);
        }

        safe_teleports += config.free_safe_teleports;

        if (safe_teleports > config.max_safe_teleports) {
            safe_teleports = config.max_safe_teleports;
        }

        update_game_status (score, current_level, safe_teleports);

        for (int i = 0; i < num_robots1; ++i) {
            var p = random_position ((x, y) => arena[x, y] == ObjectType.NONE);
            assert (p != null);
            arena[p.x, p.y] = ObjectType.ROBOT1;
        }

        for (int i = 0; i < num_robots2; ++i) {
            var p = random_position ((x, y) => arena[x, y] == ObjectType.NONE);
            assert (p != null);
            arena[p.x, p.y] = ObjectType.ROBOT2;
        }
    }

    void update_arena (ArenaChange change) {
        if (change.push != null) {
            switch (arena[change.push.x, change.push.y]) {
            case ObjectType.ROBOT1:
                splat = change.push;
                play_sound (Sound.SPLAT);
                score += config.score_type1_splatted;
                break;
            case ObjectType.ROBOT2:
                splat = change.push;
                play_sound (Sound.SPLAT);
                score += config.score_type2_splatted;
                break;
            default:
                break;
            }
        }

        this.player = change.player;
        this.arena = change.arena;

        if (arena[player.x, player.y] != ObjectType.PLAYER) {
            kill_player ();
        } else {
            /* This is in the else statement to catch the case where the last
             * two robots collide on top of the human. Without the "else" this
             * leads to the player being ressurected and winning. */
            if (arena.count (obj => obj == ObjectType.ROBOT1 || obj == ObjectType.ROBOT2) <= 0) {
                state = State.COMPLETE;
                play_sound (Sound.YAHOO);
                endlev_counter = 0;
                set_move_action_sensitivity (false);
            }
        }

        update_game_status (score, current_level + 1, safe_teleports);
    }


    /**
     * timeout_cb
     * @data: callback data
     *
     * Description:
     * Game timer callback function
     **/
    bool timeout_cb () {
        game_area.tick ();

        game_area.queue_draw ();

        if ((state == State.TYPE2) || (state == State.WTYPE2)) {
            var new_arena = move_type2_robots ();
            var change = ArenaChange () {
                arena = new_arena,
                player = this.player,
                push = null
            };
            update_arena (change);
            if (state == State.TYPE2) {
                state = State.PLAYING;
            } else if (state == State.WTYPE2) {
                state = State.WAITING;
            }
        } else if (state == State.WAITING) {
            splat = null;
            game_area.queue_draw ();
            move_robots ();
        } else if (state == State.COMPLETE) {
            ++endlev_counter;
            if (endlev_counter >= CHANGE_DELAY) {
                ++current_level;
                generate_level ();
                state = State.PLAYING;
                set_move_action_sensitivity (true);
                update_game_status (score, current_level + 1, safe_teleports);
                splat = null;
                game_area.queue_draw ();
            }
        } else if (state == State.DEAD) {
            ++endlev_counter;
            if (endlev_counter >= DEAD_DELAY) {
                if (score > 0) {
                    log_score (score);
                }
                start_new_game ();
            }
        }

        return true;
    }

    /**
     * Destroys the game timer
     **/
    void destroy_game_timer () {
        if (game_timer_id != -1) {
            Source.remove (game_timer_id);
            game_timer_id = -1;
        }
    }


    /**
     * create_game_timer
     *
     * Description:
     * Creates the game timer
     **/
    void create_game_timer () {
        if (game_timer_id != -1) {
            destroy_game_timer ();
        }

        game_timer_id = Timeout.add (ANIMATION_DELAY, timeout_cb);
    }

    /**
     * Initialises everything when game first starts up
     **/
    public void init_game () {
        create_game_timer ();
        start_new_game ();
    }

    /**
     * start_new_game
     *
     * Description:
     * Initialises everything needed to start a new game
     **/
    public void start_new_game () {
        current_level = 0;
        score = 0;
        kills = 0;
        score_step = 0;

        if (state == State.PLAYING)
            log_score (score);

        safe_teleports = config.initial_safe_teleports;

        splat = null;
        generate_level ();
        game_area.queue_draw ();

        state = State.PLAYING;

        update_game_status (score, current_level + 1, safe_teleports);
        set_move_action_sensitivity (true);
    }

    /**
     * Moves all of the robots and checks for collisions
     **/
    private Arena move_all_robots () {
        return chase (arena,
                      obj => obj == ObjectType.ROBOT1
                          || obj == ObjectType.ROBOT2,
                      player.x,
                      player.y,
                      victim => add_kill (victim));
    }

    /**
     * Makes the extra move for all of the type2 robots
     **/
    private Arena move_type2_robots () {
        return chase (arena,
                      obj => obj == ObjectType.ROBOT2,
                      player.x,
                      player.y,
                      victim => add_kill (victim));
    }

    /**
     * Starts the process of moving robots
     **/
    public void move_robots () {
        var new_arena = move_all_robots ();

        var num_robots2 = arena.count (obj => obj == ObjectType.ROBOT2);
        if (num_robots2 > 0) {
            if (state == State.WAITING) {
                state = State.WTYPE2;
            } else if (state == State.PLAYING) {
                state = State.TYPE2;
            }
        }

        var change = ArenaChange () {
            arena = new_arena,
            player = this.player,
            push = null
        };

        update_arena (change);
    }

    delegate void KillTracker(ObjectType victim);

    private static Arena chase (Arena arena, Gee.Predicate<ObjectType> is_chaser, int x, int y, KillTracker? track_kill) {
        var new_arena = arena.map ((obj) => {
            if (obj == ObjectType.PLAYER || is_chaser (obj)) {
                return ObjectType.NONE;
            } else {
                return obj;
            }
        });

        for (int i = 0; i < arena.width; ++i) {
            for (int j = 0; j < arena.height; ++j) {
                var who = arena[i, j];
                if (is_chaser (who)) {
                    int nx = i;
                    int ny = j;
                    if (x < nx)
                        nx -= 1;
                    if (x > nx)
                        nx += 1;
                    if (y < ny)
                        ny -= 1;
                    if (y > ny)
                        ny += 1;

                    var destination = new_arena[nx, ny];
                    if (destination == ObjectType.HEAP) {
                        if (track_kill != null) {
                            track_kill (who);
                        }
                    } else if (destination == ObjectType.ROBOT1 ||
                               destination == ObjectType.ROBOT2
                    ) {
                        if (track_kill != null) {
                            track_kill (who);
                            track_kill (destination);
                        }
                        new_arena[nx, ny] = ObjectType.HEAP;
                    } else {
                        new_arena[nx, ny] = who;
                    }
                }
            }
        }

        if (new_arena[x, y] == ObjectType.NONE) {
            new_arena[x, y] = ObjectType.PLAYER;
        }

        return new_arena;
    }

    /**
     * checks whether a given location is safe
     **/
    private bool check_safe (ArenaChange change) {
        var temp2_arena = chase (change.arena,
                                 obj => obj == ObjectType.ROBOT1
                                     || obj == ObjectType.ROBOT2,
                                 change.player.x,
                                 change.player.y,
                                 null);

        if (temp2_arena[change.player.x, change.player.y] != ObjectType.PLAYER) {
            return false;
        }

        var temp3_arena = chase (temp2_arena,
                                 obj => obj == ObjectType.ROBOT2,
                                 change.player.x,
                                 change.player.y,
                                 null);

        if (temp3_arena[change.player.x, change.player.y] != ObjectType.PLAYER) {
            return false;
        }

        return true;
    }

    private bool can_push_heap (Arena arena, int x, int y, int dx, int dy) {
        if (arena[x, y] != ObjectType.HEAP) {
            return false;
        }

        int nx = x + dx;
        int ny = y + dy;

        if (nx < 0 || nx >= arena.width || ny < 0 || ny >= arena.height) {
            return false;
        }

        return arena[nx, ny] != ObjectType.HEAP;
    }

    /**
     * tries to move the player in a given direction
     **/
    private ArenaChange? try_player_move (int dx, int dy) {
        int nx = player.x + dx;
        int ny = player.y + dy;

        if (nx < 0 || nx >= arena.width || ny < 0 || ny >= arena.height) {
            return null;
        }

        if (arena[nx, ny] == ObjectType.HEAP) {
            // try to push a heap
            if (!config.moveable_heaps) {
                return null;
            }
            if (!can_push_heap (arena, nx, ny, dx, dy)) {
                return null;
            }

            var push = Arena.Coords () { x = nx + dx, y = ny + dy };
            var new_arena = arena.map ((obj) => {
                if (obj != ObjectType.PLAYER) {
                    return obj;
                } else {
                    return ObjectType.NONE;
                }
            });
            new_arena[push.x, push.y] = ObjectType.HEAP;
            new_arena[nx, ny] = ObjectType.PLAYER;

            return ArenaChange () {
                arena = new_arena,
                player = Arena.Coords () { x = nx, y = ny },
                push = push
            };
        } else {
            var new_arena = arena.map ((obj) => {
                if (obj != ObjectType.PLAYER) {
                    return obj;
                } else {
                    return ObjectType.NONE;
                }
            });
            if (new_arena[nx, ny] == ObjectType.NONE) {
                new_arena[nx, ny] = ObjectType.PLAYER;
            }
            return ArenaChange () {
                arena = new_arena,
                player = Arena.Coords () { x = nx, y = ny },
                push = null
            };
        }
    }

    /**
     * safe_move_available
     *
     * Description:
     * checks to see if a safe move was available to the player
     *
     * Returns:
     * TRUE if there is a possible safe move, FALSE otherwise
     **/
    bool safe_move_available () {
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                var change = try_player_move (dx, dy);
                if (change != null) {
                    if (check_safe (change)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    /**
     * safe_teleport_available
     *
     * Description:
     * Check for a safe teleport.
     *
     * Returns:
     * TRUE is a safe teleport is possible, FALSE otherwise.
     *
     */
    bool safe_teleport_available () {
        for (int x = 0; x < arena.width; x++) {
            for (int y = 0; y < arena.height; y++) {
                if (arena[x, y] == ObjectType.NONE) {
                    var change = teleport_to (x, y);
                    if (check_safe (change)) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    private ArenaChange teleport_to (int x, int y) {
        var new_arena = arena.map ((obj) => {
            if (obj != ObjectType.PLAYER) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });
        new_arena[x, y] = ObjectType.PLAYER;
        return ArenaChange () {
            arena = new_arena,
            player = Arena.Coords () { x = x, y = y },
            push = null
        };
    }

    /**
     * player_move
     * @dx: x direction
     * @dy: y direction
     *
     * Description:
     * moves the player in a given direction
     *
     * Returns:
     * TRUE if the player can move, FALSE otherwise
     **/
    public bool player_move (int dx, int dy) {
        var change = try_player_move (dx, dy);

        if (change == null) {
            play_sound (Sound.BAD);
            return false;
        }

        if (properties_safe_moves ()) {
            if (!check_safe (change)) {
                if (properties_super_safe_moves () || safe_move_available ()) {
                    play_sound (Sound.BAD);
                    return false;
                }
            }
        }

        splat = null;
        update_arena (change);
        game_area.queue_draw ();

        return true;
    }

    /**
     * random_teleport
     *
     * Description:
     * randomly teleports the player
     *
     * Returns:
     * TRUE if the player can be teleported, FALSE otherwise
     **/
    bool random_teleport () {
        var temp_arena = arena.map ((obj) => {
            if (obj != ObjectType.PLAYER) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });

        var p = random_position ((x, y) => temp_arena[x, y] == ObjectType.NONE);
        if (p != null) {
            temp_arena[p.x, p.y] = ObjectType.PLAYER;

            var change = ArenaChange () {
                arena = temp_arena,
                player = p,
                push = null
            };

            update_arena (change);
            splat = null;
            game_area.queue_draw ();
            play_sound (Sound.TELEPORT);

            return true;
        } else {
            /* This should never happen. */
            message_box (_("There are no teleport locations left!!"));
            return false;
        }
    }


    /**
     * safe_teleport
     *
     * Description:
     * teleports the player to safe location
     *
     * Returns:
     * TRUE if player can be teleported, FALSE otherwise
     **/
    bool safe_teleport () {
        if (!safe_teleport_available ()) {
            message_box (_("There are no safe locations to teleport to!!"));
            kill_player ();
            return false;
        }

        if (safe_teleports <= 0)
            return false;

        var temp_arena = arena.map ((obj) => {
            if (obj != ObjectType.PLAYER) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });

        var p = random_position ((x, y) => {
            if (temp_arena[x, y] != ObjectType.NONE) {
                return false;
            }
            var try_change = ArenaChange () {
                arena = temp_arena,
                player = Arena.Coords() { x = x, y = y },
                push = null
            };
            return check_safe (try_change);
        });
        if (p != null) {
            temp_arena[p.x, p.y] = ObjectType.PLAYER;

            safe_teleports -= 1;
            update_game_status (score, current_level, safe_teleports);

            var change = ArenaChange () {
                arena = temp_arena,
                player = p,
                push = null
            };

            update_arena (change);
            splat = null;
            game_area.queue_draw ();
            play_sound (Sound.TELEPORT);

            return true;
        } else {
            /* This should never happen. */
            message_box (_("There are no teleport locations left!!"));
            return false;
        }
    }

    delegate bool PositionPredicate(int x, int y);

    private Arena.Coords? random_position (PositionPredicate predicate) {
        int ixp = rand.int_range (0, arena.width);
        int iyp = rand.int_range (0, arena.height);

        int xp = ixp;
        int yp = iyp;
        while (true) {
            if (predicate(xp, yp)) {
                return Arena.Coords() {
                    x = xp,
                    y = yp
                };
            }

            ++xp;
            if (xp >= arena.width) {
                xp = 0;
                ++yp;
                if (yp >= arena.height) {
                    yp = 0;
                }
            }

            if (xp == ixp && yp == iyp) {
                return null;
            }
        }
    }

    /**
     * handles keyboard commands
     **/
    public void keypress (KeyboardControl key) {
        if (state != State.PLAYING)
            return;

        switch (key) {
        case KeyboardControl.NW:
            if (player_move (-1, -1)) {
                move_robots ();
            }
            break;
        case KeyboardControl.N:
            if (player_move (0, -1)) {
                move_robots ();
            }
            break;
        case KeyboardControl.NE:
            if (player_move (1, -1)) {
                move_robots ();
            }
            break;
        case KeyboardControl.W:
            if (player_move (-1, 0)) {
                move_robots ();
            }
            break;
        case KeyboardControl.STAY:
            if (player_move (0, 0)) {
                move_robots ();
            }
            break;
        case KeyboardControl.E:
            if (player_move (1, 0)) {
                move_robots ();
            }
            break;
        case KeyboardControl.SW:
            if (player_move (-1, 1)) {
                move_robots ();
            }
            break;
        case KeyboardControl.S:
            if (player_move (0, 1)) {
                move_robots ();
            }
            break;
        case KeyboardControl.SE:
            if (player_move (1, 1)) {
                move_robots ();
            }
            break;
        case KeyboardControl.TELE:
            if (safe_teleport ()) {
                move_robots ();
            }
            break;
        case KeyboardControl.RTEL:
            if (random_teleport ()) {
                move_robots ();
            }
            break;
        case KeyboardControl.WAIT:
            state = State.WAITING;
            break;
        }
    }
}

/**
 * Displays a modal dialog box with a given message
 **/
void message_box (string msg) {
    var box = new Gtk.MessageDialog (window,
                                     Gtk.DialogFlags.MODAL,
                                     Gtk.MessageType.INFO,
                                     Gtk.ButtonsType.OK,
                                     "%s", msg);
    box.run ();
    box.destroy ();
}
