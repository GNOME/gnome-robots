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

public enum Sound {
    VICTORY = 0,
    DIE,
    TELEPORT,
    SPLAT,
    YAHOO,
    BAD,
}

public class SoundPlayer {

    private GSound.Context ctx;

    public SoundPlayer () {
        try {
            ctx = new GSound.Context ();
        } catch (Error error) {
            warning ("Failed to create gsound context: %s", error.message);
            ctx = null;
        }
    }

    public void beep () {
        var display = Gdk.Display.get_default ();
        if (display != null) {
            display.beep ();
        }
    }

    public void play_file (string name) {
        if (ctx == null) {
            return;
        }

        var filename = "%s.ogg".printf (name);
        var path = Path.build_filename (SOUND_DIRECTORY, filename);

        try {
            ctx.play_simple (null,
                             GSound.Attribute.MEDIA_NAME, name,
                             GSound.Attribute.MEDIA_FILENAME, path);
        } catch (Error error) {
            warning ("Failed to play sound \"%s\": %s", name, error.message);
        }
    }

    public void play (Sound sound) {
        switch (sound) {
        case Sound.VICTORY:
            play_file ("victory");
            break;
        case Sound.DIE:
            play_file ("die");
            break;
        case Sound.TELEPORT:
            play_file ("teleport");
            break;
        case Sound.SPLAT:
            play_file ("splat");
            break;
        case Sound.YAHOO:
            play_file ("yahoo");
            break;
        case Sound.BAD:
            beep ();
            break;
        }
    }
}

