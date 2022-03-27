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

using Gtk;
using Games;

public class RobotsScoresContext : Games.Scores.Context {

    public RobotsScoresContext (Window game_window) {
        base.with_importer_and_icon_name ("gnome-robots",
                                          /* Label on the scores dialog, next to map type dropdown */
                                          _("Game Type:"),
                                          game_window,
                                          create_category_from_game_type,
                                          Scores.Style.POINTS_GREATER_IS_BETTER,
                                          new Scores.DirectoryImporter (),
                                          "org.gnome.Robots");
    }

    public void add_game_score (string game_type, int score) {
        var category = create_category_from_game_type (game_type);
        if (category == null) {
            warning ("Failed to add score for unknown game '%s'.", game_type);
        }
        add_score.begin (score, category, null, (ctx, res) => {
            try {
                add_score.end (res);
            } catch (Error error) {
                warning ("Failed to add score: %s", error.message);
            }
        });
    }

    private static string? category_name_from_game_type (string game_type) {
        switch (game_type) {
        case "classic_robots":
            return N_("Classic robots");
        case "classic_robots-safe":
            return N_("Classic robots with safe moves");
        case "classic_robots-super-safe":
            return N_("Classic robots with super-safe moves");
        case "nightmare":
            return N_("Nightmare");
        case "nightmare-safe":
            return N_("Nightmare with safe moves");
        case "nightmare-super-safe":
            return N_("Nightmare with super-safe moves");
        case "robots2":
            return N_("Robots2");
        case "robots2-safe":
            return N_("Robots2 with safe moves");
        case "robots2-super-safe":
            return N_("Robots2 with super-safe moves");
        case "robots2_easy":
            return N_("Robots2 easy");
        case "robots2_easy-safe":
            return N_("Robots2 easy with safe moves");
        case "robots2_easy-super-safe":
            return N_("Robots2 easy with super-safe moves");
        case "robots_with_safe_teleport":
            return N_("Robots with safe teleport");
        case "robots_with_safe_teleport-safe":
            return N_("Robots with safe teleport with safe moves");
        case "robots_with_safe_teleport-super-safe":
            return N_("Robots with safe teleport with super-safe moves");
        default:
            return null;
        }
    }

    private static Scores.Category? create_category_from_game_type (string game_type) {
        string name = category_name_from_game_type (game_type);
        if (name == null)
            return null;
        return new Games.Scores.Category (game_type, name);
    }
}

