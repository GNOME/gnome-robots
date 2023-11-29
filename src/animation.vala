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

abstract class Animated {
    protected abstract int do_tick ();
    public abstract void reset ();

    public int frame { get; private set; }

    public int tick () {
        frame = do_tick ();
        return frame;
    }

    public static Animated sequence (int start, int step = 1) {
        return new Sequence (start, step);
    }

    public Animated limit (int count) {
        return new Limit (this, count);
    }

    public static Animated bounce (int start, int limit) {
        return sequence (start).limit (limit)
            .then (sequence (start + limit - 1, -1).limit (limit));
    }

    public Animated then (Animated another) {
        return new Then (this, another);
    }

    public Animated repeat (int count) {
        return new Repeat (this, count);
    }

    public Animated forever () {
        return new Forever (this);
    }
}

class Sequence : Animated {
    private int start;
    private int step;
    private int current;

    public Sequence (int start, int step = 1) {
        this.start = start;
        this.step = step;
        this.current = start;
    }

    protected override int do_tick () {
        var result = current;
        current += step;
        return result;
    }

    public override void reset () {
        current = start;
    }
}

class Then : Animated {
    private Animated first;
    private Animated second;
    private bool is_second;

    public Then (Animated first, Animated second) {
        this.first = first;
        this.second = second;
        this.is_second = false;
    }

    protected override int do_tick () {
        if (is_second) {
            return second.tick ();
        } else {
            var result = first.tick ();
            if (result >= 0) {
                return result;
            } else {
                is_second = true;
                return second.tick ();
            }
        }
    }

    public override void reset () {
        this.first.reset ();
        this.second.reset ();
        this.is_second = false;
    }
}

class Repeat : Animated {
    private Animated inner;
    private int count;
    private int current;

    public Repeat (Animated inner, int count) {
        this.inner = inner;
        this.count = count;
        this.current = 0;
    }

    protected override int do_tick () {
        if (current >= count) {
            return -1;
        }
        var result = inner.tick ();
        if (result >= 0) {
            return result;
        } else {
            inner.reset ();
            ++current;
            return inner.tick ();
        }
    }

    public override void reset () {
        this.inner.reset ();
        this.current = 0;
    }
}

class Forever : Animated {
    private Animated inner;

    public Forever (Animated inner) {
        this.inner = inner;
    }

    protected override int do_tick () {
        var result = inner.tick ();
        if (result >= 0) {
            return result;
        } else {
            inner.reset ();
            return inner.tick ();
        }
    }

    public override void reset () {
        this.inner.reset ();
    }
}

class Limit : Animated {
    private Animated inner;
    private int _limit;
    private int count;

    public Limit (Animated inner, int limit) {
        this.inner = inner;
        this._limit = limit;
        this.count = 0;
    }

    protected override int do_tick () {
        if (count >= _limit) {
            return -1;
        }
        ++count;
        return inner.tick ();
    }

    public override void reset () {
        inner.reset ();
        count = 0;
    }
}

