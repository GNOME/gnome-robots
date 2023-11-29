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

public enum ObjectType {
    PLAYER = 0,
    HEAP = 1,
    ROBOT1 = 2,
    ROBOT2 = 3,
    NONE = 99,
}

public delegate ObjectType ArenaMapper (ObjectType obj);

public class Arena {

    public struct Coords {
        public int x;
        public int y;

        public Coords move (int dx, int dy) {
            return Coords () {
                x = x + dx,
                y = y + dy,
            };
        }
    }

    private int _width;
    private int _height;
    private ObjectType[] arena;

    public int width {
        get { return this._width; }
    }

    public int height {
        get { return this._height; }
    }

    public Arena (int width, int height) {
        this._width = width;
        this._height = height;
        this.arena = new ObjectType[width * height];
        clear ();
    }

    public void clear () {
        for (int i = 0; i < _width; ++i) {
            for (int j = 0; j < _height; ++j) {
                this.arena[i + j * _width] = ObjectType.NONE;
            }
        }
    }

    public bool are_coords_valid (Coords coords) {
        return coords.x >= 0
            && coords.y >= 0
            && coords.x < _width
            && coords.y < _height;
    }

    public ObjectType @get (int x, int y) {
        if (x < 0 || y < 0 || x >= _width || y >= _height) {
            return ObjectType.NONE;
        }
        return arena[x + y * _width];
    }

    public void @set (int x, int y, ObjectType obj) {
        if (x < 0 || y < 0 || x >= _width || y >= _height) {
            return;
        }
        arena[x + y * _width] = obj;
    }

    public Arena map (ArenaMapper mapper) {
        var new_arena = new Arena (_width, _height);

        for (int i = 0; i < _width; ++i) {
            for (int j = 0; j < _height; ++j) {
                new_arena.arena[i + j * _width] = mapper (this.arena[i + j * _width]);
            }
        }

        return new_arena;
    }

    public int count (Gee.Predicate<ObjectType> predicate) {
        int result = 0;
        for (int i = 0; i < _width; ++i) {
            for (int j = 0; j < _height; ++j) {
                if (predicate (this.arena[i + j * _width])) {
                    result += 1;
                }
            }
        }
        return result;
    }

    public ArenaIterable iterate_from (int x, int y) {
        return new ArenaIterable (this, x, y);
    }
}

public class ArenaIterable {
    private Arena arena;
    private int initial_x;
    private int initial_y;

    public ArenaIterable (Arena arena, int initial_x, int initial_y) {
        this.arena = arena;
        this.initial_x = initial_x;
        this.initial_y = initial_y;
    }

    public Iterator iterator () {
        return new Iterator (this);
    }

    public class Iterator {
        private int initial_position;
        private int width;
        private int size;
        private int i;

        public Iterator (ArenaIterable iterable) {
            this.initial_position =
                iterable.initial_x + iterable.arena.width * iterable.initial_y;
            this.width = iterable.arena.width;
            this.size = iterable.arena.width * iterable.arena.height;
            this.i = 0;
        }

        public Arena.Coords? next_value () {
            if (i >= size) {
                return null;
            }
            var position = (initial_position + i) % size;
            var result = Arena.Coords () {
                x = position % width,
                y = position / width
            };
            i += 1;
            return result;
        }
    }
}

