/*
 * Copyright 2023 Andrey Kutejko <andy128k@gmail.com>
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

using Test;

static void test_arena_create () {
    var arena = new Arena (12, 10);
    assert_cmpint (arena.width, CompareOperator.EQ, 12);
    assert_cmpint (arena.height, CompareOperator.EQ, 10);
}

static void test_game_kill_two () {
    var config = GameConfig ();
    config.moveable_heaps = true;

    var game = new Game ();
    game.config = config;
    game.start_new_game ();

    var x = game.player.x;
    var y = game.player.y;

    game.arena.clear ();
    game.arena[x, y] = ObjectType.PLAYER;
    game.arena[x + 1, y] = ObjectType.HEAP;
    game.arena[x + 2, y] = ObjectType.ROBOT1;
    game.arena[x + 3, y] = ObjectType.ROBOT1;

    var changed = game.player_command (PlayerCommand.E, Game.MoveSafety.UNSAFE);
    assert_true (changed);

    assert_cmpint (game.arena[x, y], CompareOperator.EQ, ObjectType.NONE);
    assert_cmpint (game.arena[x + 1, y], CompareOperator.EQ, ObjectType.PLAYER);
    assert_cmpint (game.arena[x + 2, y], CompareOperator.EQ, ObjectType.HEAP);
    assert_cmpint (game.arena[x + 3, y], CompareOperator.EQ, ObjectType.NONE);
}

public static int main (string[] args) {
    Test.init (ref args);

    Test.add_func ("/arena/create", test_arena_create);
    Test.add_func ("/game/kill_two", test_game_kill_two);

    return Test.run ();
}
