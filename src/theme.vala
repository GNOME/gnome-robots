/*
 * Copyright 2020-2023 Andrey Kutejko <andy128k@gmail.com>
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
using Graphene;
using Gdk;
using Rsvg;

public class Theme: Object {

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

    private Paintable paintable;
    public string path { get; private set; }
    public string name { get; private set; }
    public string display_name {
        owned get { return remove_suffix (this.name).replace ("_", " "); }
    }

    public Theme.from_file (string path, string name) throws GLib.Error {
        paintable = image_from_file (path);
        this.path = path;
        this.name = name;
    }

    public void draw_object (ObjectType type,
                             int frame_no,
                             Gtk.Snapshot snapshot,
                             Rect rect
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

        snapshot.push_clip (rect);

        snapshot.save ();
        snapshot.translate (Point () {
            x = rect.origin.x - tile_no * rect.size.width,
            y = rect.origin.y,
        });
        paintable.snapshot (snapshot, rect.size.width * Frames.COUNT, rect.size.height);
        snapshot.restore ();

        snapshot.pop ();
    }
}

string remove_suffix (string filename) {
    var s = filename.last_index_of_char ('.');
    if (s >= 0) {
        return filename.substring (0, s);
    } else {
        return filename;
    }
}

