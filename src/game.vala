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

    int num_robots1 = 0;
    int num_robots2 = 0;
    int endlev_counter = 0;
    int current_level = 0;
    int score = 0;
    int kills = 0;
    int score_step = 0;
    int safe_teleports = 0;
    int player_xpos = 0;
    int player_ypos = 0;
    int push_xpos = -1;
    int push_ypos = -1;
    uint game_timer_id = -1;
    Arena temp_arena = null;

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
        arena[player_xpos, player_ypos] = ObjectType.PLAYER;
        endlev_counter = 0;
        add_aieee_bubble (player_xpos, player_ypos);
        set_move_action_sensitivity (false);
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

    /**
     * clears the arena
     **/
    void clear_arena () {
        arena.clear ();
        num_robots1 = 0;
        num_robots2 = 0;
    }

    /**
     * Set up the temporary arena for processing speculative moves.
     *
     **/
    void load_temp_arena () {
        temp_arena = arena.map ((obj) => {
            if (obj != ObjectType.PLAYER) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });
    }

    /**
     * check_location
     * @x: x position
     * @y: y position
     *
     * Description:
     * checks for an object at a given location
     *
     * Returns:
     * type of object if present or ObjectType.NONE
     **/
    public ObjectType check_location (int x, int y) {
        return arena.@get (x, y);
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
        clear_arena ();

        player_xpos = arena.width / 2;
        player_ypos = arena.height / 2;
        arena.@set(player_xpos, player_ypos, ObjectType.PLAYER);

        num_robots1 = config.initial_type1 + config.increment_type1 * current_level;

        if (num_robots1 > config.maximum_type1) {
            num_robots1 = config.maximum_type1;
        }
        if (num_robots1 > max_robots ()) {
            current_level = 0;
            num_robots1 = config.initial_type1;
            message_box (_("Congratulations, You Have Defeated the Robots!! \nBut Can You do it Again?"));
            play_sound (Sound.VICTORY);
        }

        num_robots2 = config.initial_type2 + config.increment_type2 * current_level;

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
            while (true) {
                int xp = rand.int_range (0, arena.width);
                int yp = rand.int_range (0, arena.height);

                if (check_location (xp, yp) == ObjectType.NONE) {
                    arena[xp, yp] = ObjectType.ROBOT1;
                    break;
                }
            }
        }

        for (int i = 0; i < num_robots2; ++i) {
            while (true) {
                int xp = rand.int_range (0, arena.width);
                int yp = rand.int_range (0, arena.height);

                if (check_location (xp, yp) == ObjectType.NONE) {
                    arena[xp, yp] = ObjectType.ROBOT2;
                    break;
                }
            }
        }

    }

    /**
     * update_arena
     *
     * Description:
     * Copies the temporary arena into the main game arena
     **/
    void update_arena () {
        num_robots1 = 0;
        num_robots2 = 0;

        for (int i = 0; i < GAME_WIDTH; ++i) {
            for (int j = 0; j < GAME_HEIGHT; ++j) {
                if (temp_arena[i, j] == ObjectType.HEAP &&
                    push_xpos == i && push_ypos == j
                ) {
                    if (arena[i, j] == ObjectType.ROBOT1) {
                        add_splat_bubble (i, j);
                        play_sound (Sound.SPLAT);
                        push_xpos = push_ypos = -1;
                        score += config.score_type1_splatted;
                    }
                    if (arena[i, j] == ObjectType.ROBOT2) {
                        add_splat_bubble (i, j);
                        play_sound (Sound.SPLAT);
                        push_xpos = push_ypos = -1;
                        score += config.score_type2_splatted;
                    }
                }

                arena[i, j] = temp_arena[i, j];
                if (arena[i, j] == ObjectType.ROBOT1) {
                    num_robots1 += 1;
                } else if (arena[i, j] == ObjectType.ROBOT2) {
                    num_robots2 += 1;
                }
            }
        }

        if (arena[player_xpos, player_ypos] != ObjectType.PLAYER) {
            kill_player ();
        } else {
            /* This is in the else statement to catch the case where the last
             * two robots collide on top of the human. Without the "else" this
             * leads to the player being ressurected and winning. */
            if ((num_robots1 + num_robots2) <= 0) {
                state = State.COMPLETE;
                play_sound (Sound.YAHOO);
                endlev_counter = 0;
                add_yahoo_bubble (player_xpos, player_ypos);
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

        clear_game_area ();

        if ((state == State.TYPE2) || (state == State.WTYPE2)) {
            move_type2_robots ();
            update_arena ();
            if (state == State.TYPE2) {
                state = State.PLAYING;
            } else if (state == State.WTYPE2) {
                state = State.WAITING;
            }
        } else if (state == State.WAITING) {
            remove_splat_bubble ();
            move_robots ();
        } else if (state == State.COMPLETE) {
            ++endlev_counter;
            if (endlev_counter >= CHANGE_DELAY) {
                ++current_level;
                remove_bubble ();
                clear_game_area ();
                generate_level ();
                state = State.PLAYING;
                set_move_action_sensitivity (true);
                update_game_status (score, current_level + 1, safe_teleports);
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

        remove_bubble ();
        generate_level ();
        clear_game_area ();

        state = State.PLAYING;

        update_game_status (score, current_level + 1, safe_teleports);
        set_move_action_sensitivity (true);
    }

    /**
     * move_all_robots
     *
     * Description:
     * Moves all of the robots and checks for collisions
     **/
    void move_all_robots () {
        temp_arena = arena.map ((obj) => {
            if (obj == ObjectType.PLAYER || obj == ObjectType.HEAP) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });

        int i, j;
        int nx, ny;
      for (i = 0; i < GAME_WIDTH; ++i) {
        for (j = 0; j < GAME_HEIGHT; ++j) {
          if ((arena.@get (i, j) == ObjectType.ROBOT1) || (arena.@get (i, j) == ObjectType.ROBOT2)) {
            nx = i;
            ny = j;
            if (player_xpos < nx)
              nx -= 1;
            if (player_xpos > nx)
              nx += 1;
            if (player_ypos < ny)
              ny -= 1;
            if (player_ypos > ny)
              ny += 1;

            if (temp_arena.@get (nx, ny) == ObjectType.HEAP) {
              add_kill (arena.@get (i, j));
            } else if ((temp_arena.@get (nx, ny) == ObjectType.ROBOT1) ||
                       (temp_arena.@get (nx, ny) == ObjectType.ROBOT2)) {
              add_kill (arena.@get (i, j));
              add_kill (temp_arena.@get (nx, ny));
              temp_arena.@set (nx, ny, ObjectType.HEAP);
            } else {
              temp_arena.@set (nx, ny, arena.@get (i, j));
            }
          }
        }
      }

    }

    /**
     * move_type2_robots
     *
     * Description:
     * Makes the extra move for all of the type2 robots
     **/
    void move_type2_robots () {
        int i, j;
        int nx, ny;

        temp_arena = arena.map ((obj) => {
            if (obj == ObjectType.PLAYER || obj == ObjectType.ROBOT1 || obj == ObjectType.HEAP) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });

      for (i = 0; i < arena.width; ++i) {
        for (j = 0; j < arena.height; ++j) {
          if (arena.@get (i, j) == ObjectType.ROBOT2) {
                nx = i;
                ny = j;
                if (player_xpos < nx)
                  nx -= 1;
                if (player_xpos > nx)
                  nx += 1;
                if (player_ypos < ny)
                  ny -= 1;
                if (player_ypos > ny)
                    ny += 1;

            if (temp_arena.@get (nx, ny) == ObjectType.HEAP) {
              add_kill (arena.@get (i, j));
            } else if ((temp_arena.@get (nx, ny) == ObjectType.ROBOT1) ||
                       (temp_arena.@get (nx, ny) == ObjectType.ROBOT2)) {
              add_kill (arena.@get (i, j));
              add_kill (temp_arena.@get (nx, ny));
              temp_arena.@set (nx, ny, ObjectType.HEAP);
            } else {
              temp_arena.@set (nx, ny, arena.@get (i, j));
            }
          }
        }
      }
    }

    /**
     * Starts the process of moving robots
     **/
    public void move_robots () {
        move_all_robots ();

        if (num_robots2 > 0) {
            if (state == State.WAITING) {
                state = State.WTYPE2;
            } else if (state == State.PLAYING) {
                state = State.TYPE2;
            }
        }

        update_arena ();
    }

    private static Arena chase (Arena arena, Gee.Predicate<ObjectType> is_chaser, int x, int y) {
        var new_arena = arena.map ((obj) => {
            if (obj == ObjectType.PLAYER || obj == ObjectType.HEAP) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });

        for (int i = 0; i < arena.width; ++i) {
            for (int j = 0; j < arena.height; ++j) {
                var who = arena.@get (i, j);
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

                    var destination = new_arena.@get (nx, ny);
                    if (destination == ObjectType.ROBOT1 ||
                        destination == ObjectType.ROBOT2 ||
                        destination == ObjectType.HEAP
                    ) {
                        new_arena.@set (nx, ny, ObjectType.HEAP);
                    } else {
                        new_arena.@set (nx, ny, who);
                    }
                }
            }
        }

        return new_arena;
    }

    /**
     * check_safe
     * @x: x position
     * @y: y position
     *
     * Description:
     * checks whether a given location is safe
     *
     * Returns:
     * TRUE if location is safe, FALSE otherwise
     **/
    bool check_safe (int x, int y) {
        if (temp_arena.@get (x, y) != ObjectType.NONE) {
            return false;
        }

        var temp2_arena = temp_arena.map ((obj) => {
            if (obj == ObjectType.PLAYER || obj == ObjectType.HEAP) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });

        for (int i = 0; i < GAME_WIDTH; ++i) {
            for (int j = 0; j < GAME_HEIGHT; ++j) {
                if (temp_arena[i, j] == ObjectType.ROBOT1 ||
                    temp_arena[i, j] == ObjectType.ROBOT2) {

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

                    if (temp2_arena[nx, ny] == ObjectType.ROBOT1 ||
                        temp2_arena[nx, ny] == ObjectType.ROBOT2 ||
                        temp2_arena[nx, ny] == ObjectType.HEAP) {
                        temp2_arena[nx, ny] = ObjectType.HEAP;
                    } else {
                        temp2_arena[nx, ny] = temp_arena[i, j];
                    }
                }
            }
        }

        if (temp2_arena[x, y] != ObjectType.NONE) {
            return false;
        }

        var temp3_arena = chase (temp2_arena, (obj) => obj == ObjectType.ROBOT2, x, y);

        if (temp3_arena.@get (x, y) != ObjectType.NONE) {
            return false;
        }

        return true;
    }

    /**
     * push_heap
     * @x: x position
     * @y: y position
     * @dx: x direction
     * @dy: y direction
     *
     * Description:
     * pushes a heap in a given direction
     *
     * Returns:
     * TRUE if heap can be pushed, FALSE otherwise
     **/
    bool push_heap (int x, int y, int dx, int dy) {
        int nx = x + dx;
        int ny = y + dy;

        if (temp_arena.@get (x, y) != ObjectType.HEAP)
            return false;

        if (nx < 0 || nx >= arena.width || ny < 0 || ny >= arena.height) {
            return false;
        }

        if (temp_arena.@get (nx, ny) == ObjectType.HEAP)
            return false;

        push_xpos = nx;
        push_ypos = ny;

        temp_arena.@set (nx, ny, ObjectType.HEAP);
        temp_arena.@set (x, y, ObjectType.NONE);

        return true;
    }

    /**
     * try_player_move
     * @dx: x direction
     * @dy: y direction
     *
     * Description:
     * tries to move the player in a given direction
     *
     * Returns:
     * TRUE if the player can move, FALSE otherwise
     **/
    bool try_player_move (int dx, int dy) {
        int nx = player_xpos + dx;
        int ny = player_ypos + dy;

        if ((nx < 0) || (nx >= arena.width) || (ny < 0) || (ny >= arena.height)) {
            return false;
        }

        load_temp_arena ();

        if (temp_arena.@get (nx, ny) == ObjectType.HEAP) {
            if (config.moveable_heaps) {
                if (!push_heap (nx, ny, dx, dy)) {
                    push_xpos = push_ypos = -1;
                    return false;
                }
            } else {
                return false;
            }
        }

        return true;
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
        if (try_player_move (-1, -1)) {
            if (check_safe (player_xpos - 1, player_ypos - 1)) {
                return true;
            }
        }
        if (try_player_move (0, -1)) {
            if (check_safe (player_xpos, player_ypos - 1)) {
                return true;
            }
        }
        if (try_player_move (1, -1)) {
            if (check_safe (player_xpos + 1, player_ypos - 1)) {
                return true;
            }
        }
        if (try_player_move (-1, 0)) {
            if (check_safe (player_xpos - 1, player_ypos)) {
                return true;
            }
        }
        if (try_player_move (0, 0)) {
            if (check_safe (player_xpos, player_ypos)) {
                return true;
            }
        }
        if (try_player_move (1, 0)) {
            if (check_safe (player_xpos + 1, player_ypos)) {
                return true;
            }
        }
        if (try_player_move (-1, 1)) {
            if (check_safe (player_xpos - 1, player_ypos + 1)) {
                return true;
            }
        }
        if (try_player_move (0, 1)) {
            if (check_safe (player_xpos, player_ypos + 1)) {
                return true;
            }
        }
        if (try_player_move (1, 1)) {
            if (check_safe (player_xpos + 1, player_ypos + 1)) {
                return true;
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
        load_temp_arena ();

        for (int x = 0; x < GAME_WIDTH; x++) {
            for (int y = 0; y < GAME_HEIGHT; y++) {
                if (check_safe (x, y))
                    return true;
            }
        }

        return false;
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

        int nx = player_xpos + dx;
        int ny = player_ypos + dy;

        if (properties_safe_moves ()) {
            if (!try_player_move (dx, dy)) {
                play_sound (Sound.BAD);
                return false;
            } else {
                if (!check_safe (nx, ny)) {
                    if (properties_super_safe_moves () || safe_move_available ()) {
                        play_sound (Sound.BAD);
                        return false;
                    }
                }
            }
        } else {
            if (!try_player_move (dx, dy)) {
                play_sound (Sound.BAD);
                return false;
            }
        }

        player_xpos = nx;
        player_ypos = ny;

        if (temp_arena.@get (player_xpos, player_ypos) == ObjectType.NONE) {
            temp_arena.@set (player_xpos, player_ypos, ObjectType.PLAYER);
        }

        remove_splat_bubble ();

        update_arena ();

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
        temp_arena = arena.map ((obj) => {
            if (obj != ObjectType.PLAYER) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });

        int xp;
        int yp;
        if (random_position ((x, y) => temp_arena.@get(x, y) == ObjectType.NONE, out xp, out yp)) {
            player_xpos = xp;
            player_ypos = yp;
            temp_arena.@set (player_xpos, player_ypos, ObjectType.PLAYER);

            update_arena ();
            remove_splat_bubble ();
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

        temp_arena = arena.map ((obj) => {
            if (obj != ObjectType.PLAYER) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });

        int xp;
        int yp;
        if (random_position ((x, y) => temp_arena.@get(x, y) == ObjectType.NONE && check_safe (x, y), out xp, out yp)) {
            player_xpos = xp;
            player_ypos = yp;
            temp_arena.@set (player_xpos, player_ypos, ObjectType.PLAYER);

            safe_teleports -= 1;
            update_game_status (score, current_level, safe_teleports);

            update_arena ();
            remove_splat_bubble ();
            play_sound (Sound.TELEPORT);

            return true;
        } else {
            /* This should never happen. */
            message_box (_("There are no teleport locations left!!"));
            return false;
        }
    }

    delegate bool PositionPredicate(int x, int y);

    private bool random_position (PositionPredicate predicate, out int xp, out int yp) {
        int ixp = rand.int_range (0, arena.width);
        int iyp = rand.int_range (0, arena.height);

        xp = ixp;
        yp = iyp;
        while (true) {
            if (predicate(xp, yp)) {
                return true;
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
                return false;
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

    public void get_dir (int ix, int iy, out int odx, out int ody) {
        const int[,] MOVE_TABLE = {
            {-1, 0}, {-1, -1}, {0, -1}, {1, -1},
            {1, 0}, {1, 1}, {0, 1}, {-1, 1}
        };
        int x = (ix / tile_width).clamp (0, arena.width);
        int y = (iy / tile_height).clamp (0, arena.height);

        /* If we click on our man then we assume we hold. */
        if ((x == player_xpos) && (y == player_ypos)) {
            odx = 0;
            ody = 0;
            return;
        }

        /* If the square clicked on is a valid move, go there. */
        int idx = x - player_xpos;
        int idy = y - player_ypos;
        if (idx.abs () < 2 && idy.abs () < 2) {
            odx = idx;
            ody = idy;
            return;
        }

        /* Otherwise go in the general direction of the mouse click. */
        double dx = ix - (player_xpos + 0.5) * tile_width;
        double dy = iy - (player_ypos + 0.5) * tile_height;

        double angle = Math.atan2 (dy, dx);

        /* Note the adjustment we have to make (+9, not +8) because atan2's idea
         * of octants and the ones we want are shifted by PI/8. */
        int octant = (((int) Math.floor (8.0 * angle / Math.PI) + 9) / 2) % 8;

        odx = MOVE_TABLE[octant, 0];
        ody = MOVE_TABLE[octant, 1];
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
