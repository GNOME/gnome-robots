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
using Gdk;

class RobotsApplication : Gtk.Application {

    private Properties properties;

    private GameConfigs game_configs;
    private Assets assets;
    private SoundPlayer sound_player;

    public RobotsApplication () {
        Object (
            application_id: "org.gnome.Robots",
            flags: ApplicationFlags.FLAGS_NONE);
    }

    protected override void startup () {
        base.startup ();

        Environment.set_application_name (_("Robots"));
        Window.set_default_icon_name ("org.gnome.Robots");

        GLib.ActionEntry[] app_entries = {
            { "new-game",         new_game_cb    },
            { "preferences",      preferences_cb },
            { "scores",           scores_cb      },
            { "help",             help_cb        },
            { "about",            about_cb       },
            { "quit",             quit           },
        };
        add_action_entries (app_entries, this);

        set_accels_for_action ("app.new-game", { "<Primary>n" });
        set_accels_for_action ("app.help", { "F1" });
        set_accels_for_action ("app.quit", { "<Primary>q" });

        try {
            assets = new DirectoryAssets.from_directory (DATA_DIRECTORY);
            game_configs = new GameConfigs.load ();
            sound_player = new SoundPlayer ();
        } catch (Error e) {
            critical ("%s\n%s",
                _("The program Robots was unable to find any valid game configuration files. Please check that the program is installed correctly."),
                e.message
            );
            quit ();
        }

        var settings = new GLib.Settings ("org.gnome.Robots");
        properties = new Properties (settings);
    }

    protected override void activate () {
        var window = get_active_window () as RobotsWindow;
        if (window != null) {
            window.present_with_time (CURRENT_TIME);
            return;
        }

        try {
            window = new RobotsWindow (this,
                                       properties,
                                       game_configs,
                                       assets,
                                       sound_player);
        } catch (Error e) {
            critical ("%s", e.message);
            // TODO message box
            quit ();
        }
        window.show ();
    }

    private void new_game_cb () {
        var window = get_active_window () as RobotsWindow;
        if (window != null) {
            var dialog = new AlertDialog (_("Are you sure you want to discard the current game?"));
            dialog.buttons = new string[] {
                _("Keep _Playing"),
                _("_New Game")
            };
            dialog.modal = true;
            dialog.default_button = 0;
            dialog.cancel_button = 0;
            dialog.choose.begin (window, null, (obj, res) => {
                try {
                    var choice = dialog.choose.end (res);
                    if (choice == 1) {
                        window.start_new_game ();
                    }
                } catch (Error e) {
                    warning ("%s", e.message);
                }
            });
        } else {
            activate ();
        }
    }

    private void preferences_cb () {
        PropertiesDialog.show_dialog (get_active_window (),
                                      game_configs,
                                      assets.themes,
                                      properties);
    }

    private void scores_cb () {
        var window = get_active_window () as RobotsWindow;
        if (window != null) {
            window.show_highscores ();
        }
    }

    private void help_cb () {
        show_uri (get_active_window (), "help:gnome-robots", CURRENT_TIME);
    }

    private void about_cb () {
        string[] authors = {
            "Mark Rae <m.rae@inpharmatica.co.uk>"
        };

        string[] artists = {
            "Kirstie Opstad <K.Opstad@ed.ac.uk>",
            "Rasoul M.P. Aghdam (player death sound)"
        };

        string[] documenters = {
            "Aruna Sankaranarayanan"
        };

        show_about_dialog (get_active_window (),
                           "name", _("Robots"),
                           "version", VERSION,
                           "copyright", "Copyright © 1998–2008 Mark Rae\n"
                                + "Copyright © 2014–2016 Michael Catanzaro\n"
                                + "Copyright © 2020 Andrey Kutejko",
                           "license-type", License.GPL_3_0,
                           "comments", _("Based on classic BSD Robots"),
                           "authors", authors,
                           "artists", artists,
                           "documenters", documenters,
                           "translator-credits", _("translator-credits"),
                           "logo-icon-name", "org.gnome.Robots",
                           "website", "https://wiki.gnome.org/Apps/Robots");
    }
}

