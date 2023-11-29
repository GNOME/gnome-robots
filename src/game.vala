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

public enum PlayerCommand {
    NW = 0,
    N,
    NE,
    W,
    STAY,
    E,
    SW,
    S,
    SE,
    SAFE_TELEPORT,
    RANDOM_TELEPORT,
    WAIT;

    public static PlayerCommand from_direction (int dx, int dy) {
        foreach (var direction in directions) {
            if (direction.dx == dx && direction.dy == dy) {
                return direction.cmd;
            }
        }
        return PlayerCommand.STAY;
    }

    public bool to_direction (out int dx, out int dy) {
        foreach (var direction in directions) {
            if (direction.cmd == this) {
                dx = direction.dx;
                dy = direction.dy;
                return true;
            }
        }
        dx = 0;
        dy = 0;
        return false;
    }
}

struct Direction {
    PlayerCommand cmd;
    int dx;
    int dy;
}

const Direction[] directions = {
    { PlayerCommand.NW,    -1, -1 },
    { PlayerCommand.N,      0, -1 },
    { PlayerCommand.NE,     1, -1 },
    { PlayerCommand.W,     -1,  0 },
    { PlayerCommand.STAY,   0,  0 },
    { PlayerCommand.E,      1,  0 },
    { PlayerCommand.SW,    -1,  1 },
    { PlayerCommand.S,      0,  1 },
    { PlayerCommand.SE,     1,  1 }
};

public class Game {

    /*
     * Size of the game playing area
     */
    public const int GAME_WIDTH = 45;
    public const int GAME_HEIGHT = 30;

    public const int DEAD_DELAY = 30;
    public const int CHANGE_DELAY = 20;

    public struct Status {
        public int score;
        public int current_level;
        public int safe_teleports;
    }

    public enum State {
        PLAYING = 1,
        WAITING,
        COMPLETE,
        DEAD,
        TYPE2,
        WAITING_TYPE2,
    }

    public State state { get; private set; }
    public Arena arena { get; private set; }
    public GameConfig config { get; set; }
    public int width {
        get { return arena.width; }
    }
    public int height {
        get { return arena.height; }
    }
    public Arena.Coords player { get; private set; }
    public Arena.Coords? splat { get; private set; }

    private Rand rand;
    private int endlev_counter = 0;
    private int current_level = 0;
    private int score = 0;
    private int kills = 0;
    private int score_step = 0;
    private int safe_teleports = 0;

    public Status status {
        get {
            return Status () {
                score = score,
                current_level = current_level + 1,
                safe_teleports = safe_teleports
            };
        }
    }

    private struct ArenaChange {
        Arena arena;
        Arena.Coords player;
        Arena.Coords? push;
    }

    public Game () {
        arena = new Arena (GAME_WIDTH, GAME_HEIGHT);
        rand = new Rand ();
        state = State.PLAYING;
    }

    public enum Event {
        TELEPORTED,
        SPLAT,
        LEVEL_COMPLETE,
        DEATH,
        SCORED,
        VICTORY,
        NO_TELEPORT_LOCATIONS,
        NO_SAFE_TELEPORT_LOCATIONS,
    }

    public signal void game_event (Event event, int param = 0);

    /**
     * Ends the current game.
     **/
    private void kill_player () {
        state = State.DEAD;
        arena[player.x, player.y] = ObjectType.PLAYER;
        endlev_counter = 0;
        game_event (Event.DEATH);
    }

    /**
     * add_kill
     * @type: robot type
     *
     * Description:
     * registers a robot kill and updates the score
     **/
    private void add_kill (ObjectType type) {
        int si;
        if ((state == State.WAITING) || (state == State.WAITING_TYPE2)) {
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
    }

    private int max_robots () {
        return arena.width * arena.height / 2;
    }

    /**
     * Creates a new level and populates it with robots
     **/
    private void generate_level () {
        arena.clear ();

        player = Arena.Coords () {
            x = arena.width / 2,
            y = arena.height / 2
        };
        arena[player.x, player.y] = ObjectType.PLAYER;

        var num_robots1 = int.min (
            config.initial_type1 + config.increment_type1 * current_level,
            config.maximum_type1
        );
        var num_robots2 = int.min (
            config.initial_type2 + config.increment_type2 * current_level,
            config.maximum_type2
        );

        if ((num_robots1 + num_robots2) > max_robots ()) {
            current_level = 0;
            num_robots1 = config.initial_type1;
            num_robots2 = config.initial_type2;

            game_event (Event.VICTORY);
        }

        safe_teleports += config.free_safe_teleports;

        if (safe_teleports > config.max_safe_teleports) {
            safe_teleports = config.max_safe_teleports;
        }

        for (int i = 0; i < num_robots1; ++i) {
            place_randomly (ObjectType.ROBOT1);
        }

        for (int i = 0; i < num_robots2; ++i) {
            place_randomly (ObjectType.ROBOT2);
        }
    }

    private void place_randomly (ObjectType obj) {
        int rand_x = rand.int_range (0, arena.width);
        int rand_y = rand.int_range (0, arena.height);
        foreach (var p in arena.iterate_from (rand_x, rand_y)) {
            if (arena[p.x, p.y] == ObjectType.NONE) {
                arena[p.x, p.y] = obj;
                return;
            }
        }
    }

    private void update_arena (ArenaChange change) {
        if (change.push != null) {
            switch (arena[change.push.x, change.push.y]) {
            case ObjectType.ROBOT1:
                splat = change.push;
                game_event (Event.SPLAT);
                score += config.score_type1_splatted;
                break;
            case ObjectType.ROBOT2:
                splat = change.push;
                game_event (Event.SPLAT);
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
                game_event (Event.LEVEL_COMPLETE);
                endlev_counter = 0;
            }
        }
    }

    public void tick () {
        if ((state == State.TYPE2) || (state == State.WAITING_TYPE2)) {
            var new_arena = move_type2_robots ();
            var change = ArenaChange () {
                arena = new_arena,
                player = this.player,
                push = null
            };
            update_arena (change);
            if (state == State.TYPE2) {
                state = State.PLAYING;
            } else if (state == State.WAITING_TYPE2) {
                state = State.WAITING;
            }
        } else if (state == State.WAITING) {
            splat = null;
            move_robots ();
        } else if (state == State.COMPLETE) {
            ++endlev_counter;
            if (endlev_counter >= CHANGE_DELAY) {
                ++current_level;
                generate_level ();
                state = State.PLAYING;
                splat = null;
            }
        } else if (state == State.DEAD) {
            ++endlev_counter;
            if (endlev_counter >= DEAD_DELAY) {
                if (score > 0) {
                    game_event (Event.SCORED, score);
                }
                start_new_game ();
            }
        }
    }

    /**
     * Initialises everything needed to start a new game
     **/
    public void start_new_game () {
        current_level = 0;
        score = 0;
        kills = 0;
        score_step = 0;

        safe_teleports = config.initial_safe_teleports;

        splat = null;
        generate_level ();

        state = State.PLAYING;
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
    private void move_robots () {
        var new_arena = move_all_robots ();

        var num_robots2 = new_arena.count (obj => obj == ObjectType.ROBOT2);
        if (num_robots2 > 0) {
            if (state == State.WAITING) {
                state = State.WAITING_TYPE2;
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

    private delegate void KillTracker (ObjectType victim);

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

    private ArenaChange? try_push_heap (int dx, int dy) {
        if (!config.moveable_heaps) {
            return null;
        }

        var coords = player.move (dx, dy);
        if (!arena.are_coords_valid (coords)
            || arena[coords.x, coords.y] != ObjectType.HEAP
        ) {
            return null;
        }

        var push_to = coords.move (dx, dy);
        if (!arena.are_coords_valid (push_to)
            || arena[push_to.x, push_to.y] == ObjectType.HEAP
        ) {
            return null;
        }

        var new_arena = arena.map ((obj) => {
            if (obj != ObjectType.PLAYER) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });
        new_arena[coords.x, coords.y] = ObjectType.PLAYER;
        new_arena[push_to.x, push_to.y] = ObjectType.HEAP;
        return ArenaChange () {
            arena = new_arena,
            player = coords,
            push = push_to
        };
    }

    /**
     * tries to move the player in a given direction
     **/
    private ArenaChange? try_player_move (int dx, int dy) {
        var coords = player.move (dx, dy);

        if (!arena.are_coords_valid (coords)) {
            return null;
        }

        if (arena[coords.x, coords.y] == ObjectType.HEAP) {
            return try_push_heap (dx, dy);
        } else {
            return move_player_to (coords);
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
    private bool safe_move_available () {
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

    private ArenaChange move_player_to (Arena.Coords coords) {
        var new_arena = arena.map ((obj) => {
            if (obj != ObjectType.PLAYER) {
                return obj;
            } else {
                return ObjectType.NONE;
            }
        });
        if (new_arena[coords.x, coords.y] == ObjectType.NONE) {
            new_arena[coords.x, coords.y] = ObjectType.PLAYER;
        }
        return ArenaChange () {
            arena = new_arena,
            player = coords,
            push = null
        };
    }

    public enum MoveSafety {
        UNSAFE,
        SAFE,
        SUPER_SAFE,
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
    private bool player_move (int dx, int dy, MoveSafety safety) {
        var change = try_player_move (dx, dy);

        if (change == null) {
            return false;
        }

        if (safety != MoveSafety.UNSAFE) {
            if (!check_safe (change)) {
                if (safety == MoveSafety.SUPER_SAFE || safe_move_available ()) {
                    return false;
                }
            }
        }

        splat = null;
        update_arena (change);

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
    private bool random_teleport () {
        int rand_x = rand.int_range (0, arena.width);
        int rand_y = rand.int_range (0, arena.height);
        foreach (var p in arena.iterate_from (rand_x, rand_y)) {
            if (arena[p.x, p.y] != ObjectType.NONE) {
                continue;
            }

            var change = move_player_to (p);

            update_arena (change);
            splat = null;
            game_event (Event.TELEPORTED);
            return true;
        }

        /* This should never happen. */
        game_event (Event.NO_TELEPORT_LOCATIONS);
        return false;
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
    private bool safe_teleport () {
        if (safe_teleports <= 0) {
            return false;
        }

        int rand_x = rand.int_range (0, arena.width);
        int rand_y = rand.int_range (0, arena.height);
        foreach (var p in arena.iterate_from (rand_x, rand_y)) {
            if (arena[p.x, p.y] != ObjectType.NONE) {
                continue;
            }

            var change = move_player_to (p);
            if (!check_safe (change)) {
                continue;
            }

            safe_teleports -= 1;

            update_arena (change);
            splat = null;
            game_event (Event.TELEPORTED);

            return true;
        }

        game_event (Event.NO_SAFE_TELEPORT_LOCATIONS);
        kill_player ();
        return false;
    }

    /**
     * handles player's commands
     **/
    public bool player_command (PlayerCommand cmd,
                                MoveSafety safety = MoveSafety.UNSAFE) {
        if (state != State.PLAYING) {
            return false;
        }

        switch (cmd) {
        case PlayerCommand.NW:
        case PlayerCommand.N:
        case PlayerCommand.NE:
        case PlayerCommand.W:
        case PlayerCommand.STAY:
        case PlayerCommand.E:
        case PlayerCommand.SW:
        case PlayerCommand.S:
        case PlayerCommand.SE:
            int dx, dy;
            assert (cmd.to_direction (out dx, out dy));
            if (player_move (dx, dy, safety)) {
                move_robots ();
                return true;
            } else {
                return false;
            }
        case PlayerCommand.SAFE_TELEPORT:
            if (safe_teleport ()) {
                move_robots ();
            }
            return true;
        case PlayerCommand.RANDOM_TELEPORT:
            if (random_teleport ()) {
                move_robots ();
            }
            return true;
        case PlayerCommand.WAIT:
            state = State.WAITING;
            return true;
        default:
            return false;
        }
    }
}

