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
}

