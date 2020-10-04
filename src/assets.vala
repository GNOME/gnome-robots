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

public interface Assets : Object {
    public abstract Themes themes { get; }
    public abstract Bubble aieee_bubble { get; }
    public abstract Bubble yahoo_bubble { get; }
    public abstract Bubble splat_bubble { get; }
    public abstract Array<Gdk.Cursor> cursors { get; }
}

public class DirectoryAssets : Object, Assets {
    private Themes _themes;
    public override Themes themes { get { return _themes; } }

    private Bubble _aieee_bubble;
    public override Bubble aieee_bubble { get { return _aieee_bubble; } }

    private Bubble _yahoo_bubble;
    public override Bubble yahoo_bubble { get { return _yahoo_bubble; } }

    private Bubble _splat_bubble;
    public override Bubble splat_bubble { get { return _splat_bubble; } }

    private Array<Gdk.Cursor> _cursors;
    public override Array<Gdk.Cursor> cursors { get { return _cursors; } }

    public DirectoryAssets.from_directory (string directory) throws Error {
        _themes = Themes.from_directory (
            Path.build_filename (directory, "themes"));

        _yahoo_bubble = new Bubble.from_file (
            Path.build_filename (directory, "pixmaps", "yahoo.png"));
        _aieee_bubble = new Bubble.from_file (
            Path.build_filename (directory, "pixmaps", "aieee.png"));
        _splat_bubble = new Bubble.from_file (
            Path.build_filename (directory, "pixmaps", "splat.png"));

        var display = Gdk.Display.get_default ();
        _cursors = make_cursors_for_display (display);
    }
}

