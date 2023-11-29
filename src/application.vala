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
using Adw;

class RobotsApplication : Adw.Application {

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
        Gtk.Window.set_default_icon_name ("org.gnome.Robots");

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
            var dialog = new Adw.MessageDialog (get_active_window (),
                                                _("_New Game"),
                                                _("Are you sure you want to discard the current game?"));

            dialog.add_response ("cancel", _("Keep _Playing"));
            dialog.add_response ("new", _("_New Game"));

            dialog.default_response = "new";
            dialog.close_response = "cancel";

            dialog.response.connect ((ret) => {
                dialog.destroy ();
                if (ret == "new") {
                    window.start_new_game ();
                }
            });
            dialog.present ();
        } else {
            activate ();
        }
    }

    private void preferences_cb () {
        show_preferences (get_active_window (),
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
        var launcher = new UriLauncher ("help:gnome-robots");
        launcher.launch.begin (get_active_window (), null);
    }

    private void about_cb () {
        var window = new AboutWindow ();
        window.set_transient_for (get_active_window ());

        window.application_icon = "org.gnome.Robots";
        window.application_name = _("Robots");
        window.version = VERSION;
        window.copyright = "Copyright © 1998–2008 Mark Rae\n"
                           + "Copyright © 2014–2016 Michael Catanzaro\n"
                           + "Copyright © 2020-2022 Andrey Kutejko";
        window.license_type = License.GPL_3_0;
        window.comments = _("Based on classic BSD Robots");
        window.developers = {
            "Mark Rae <m.rae@inpharmatica.co.uk>",
            "Andrey Kutejko <andy128k@gmail.com>"
        };
        window.artists = {
            "Kirstie Opstad <K.Opstad@ed.ac.uk>",
            "Rasoul M.P. Aghdam (player death sound)"
        };
        window.documenters = {
            "Aruna Sankaranarayanan"
        };
        window.translator_credits = _("translator-credits");
        window.website = "https://wiki.gnome.org/Apps/Robots";

        window.present ();
    }
}

