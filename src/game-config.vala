/*
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

public struct GameConfig {
    public string description;
    public int initial_type1;
    public int initial_type2;
    public int increment_type1;
    public int increment_type2;
    public int maximum_type1;
    public int maximum_type2;
    public int score_type1;
    public int score_type2;
    public int score_type1_waiting;
    public int score_type2_waiting;
    public int score_type1_splatted;
    public int score_type2_splatted;
    public int num_robots_per_safe;
    public int safe_score_boundary;
    public int initial_safe_teleports;
    public int free_safe_teleports;
    public int max_safe_teleports;
    public bool moveable_heaps;

    public string name () {
        return description.replace ("_", " ");
    }

    public static GameConfig from_file (string filename) throws Error {
        GameConfig gcfg = GameConfig ();

        gcfg.description = Path.get_basename (filename);
        if (gcfg.description.has_suffix (".cfg")) {
            gcfg.description = gcfg.description.substring (0, gcfg.description.length - 4);
        }

        var line_regex = new Regex ("^(\\w+)\\s*=\\s*(\\d+)");

        var file = File.new_for_path (filename);
        var dis = new DataInputStream (file.read ());

        string line;
        uint pflag = 0;
        while ((line = dis.read_line (null)) != null) {
            MatchInfo match_info;
            if (!line_regex.match (line, 0, out match_info)) {
                continue;
            }
            string key = match_info.fetch (1);
            int val = int.parse (match_info.fetch (2));

            if (key == "initial_type1") {
                gcfg.initial_type1 = val;
                pflag |= 0x00000001;
            }
            if (key == "initial_type2") {
                gcfg.initial_type2 = val;
                pflag |= 0x00000002;
            }
            if (key == "increment_type1") {
                gcfg.increment_type1 = val;
                pflag |= 0x00000004;
            }
            if (key == "increment_type2") {
                gcfg.increment_type2 = val;
                pflag |= 0x00000008;
            }
            if (key == "maximum_type1") {
                gcfg.maximum_type1 = val;
                pflag |= 0x00000010;
            }
            if (key == "maximum_type2") {
                gcfg.maximum_type2 = val;
                pflag |= 0x00000020;
            }
            if (key == "score_type1") {
                gcfg.score_type1 = val;
                pflag |= 0x00000040;
            }
            if (key == "score_type2") {
                gcfg.score_type2 = val;
                pflag |= 0x00000080;
            }
            if (key == "score_type1_waiting") {
                gcfg.score_type1_waiting = val;
                pflag |= 0x00000100;
            }
            if (key == "score_type2_waiting") {
                gcfg.score_type2_waiting = val;
                pflag |= 0x00000200;
            }
            if (key == "score_type1_splatted") {
                gcfg.score_type1_splatted = val;
                pflag |= 0x00000400;
            }
            if (key == "score_type2_splatted") {
                gcfg.score_type2_splatted = val;
                pflag |= 0x00000800;
            }
            if (key == "num_robots_per_safe") {
                gcfg.num_robots_per_safe = val;
                pflag |= 0x00001000;
            }
            if (key == "safe_score_boundary") {
                gcfg.safe_score_boundary = val;
                pflag |= 0x00002000;
            }
            if (key == "max_safe_teleports") {
                gcfg.max_safe_teleports = val;
                pflag |= 0x00004000;
            }
            if (key == "initial_safe_teleports") {
                gcfg.initial_safe_teleports = val;
                pflag |= 0x00008000;
            }
            if (key == "free_safe_teleports") {
                gcfg.free_safe_teleports = val;
                pflag |= 0x00010000;
            }
            if (key == "moveable_heaps") {
                gcfg.moveable_heaps = val != 0;
                pflag |= 0x00020000;
            }
        }

        // Check we have got all types
        if (pflag != 0x0003ffff) {
            throw new FileError.INVAL ("Bad game config file.");
        }

        return gcfg;
    }
}

public class GameConfigs {
    private Gee.ArrayList<GameConfig?> game_configs;
    private uint current_config;

    public GameConfigs.load () throws Error {
        var dname = Path.build_filename (DATA_DIRECTORY, "games");
        var dir = Dir.open (dname);
        game_configs = new Gee.ArrayList<GameConfig?> ();
        string? filename;
        while ((filename = dir.read_name ()) != null) {
            if (!filename.has_suffix (".cfg")) {
                continue;
            }
            var fullname = Path.build_filename (dname, filename);
            try {
                var gcfg = GameConfig.from_file (fullname);
                game_configs.add (gcfg);
            } catch (Error e) {
                warning ("%s", e.message);
            }
        }

        if (game_configs.size >= 0) {
            current_config = 0;
        } else {
            throw new FileError.NOENT ("No game config was found.");
        }
    }

    public uint count () {
        return game_configs.size;
    }

    public GameConfig? @get (uint n) {
        if (n < game_configs.size)
            return game_configs[(int)n];
        else
            return null;
    }

    public string? get_name (uint n) {
        if (n < game_configs.size)
            return game_configs[(int)n].description.replace ("_", " ");
        else
            return null;
    }

    public GameConfig get_current () {
        return game_configs[(int)current_config];
    }

    public uint get_current_index () {
        return current_config;
    }

    public void set_current_index (uint n) {
        if (n < game_configs.size)
            current_config = n;
    }
}

