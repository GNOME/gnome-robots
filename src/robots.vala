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
using Cairo;

public class RobotsWindow : ApplicationWindow {

    private HeaderBar headerbar;
    private Label safe_teleports_label;
    private GameArea game_area;
    private EventControllerKey key_controller;
    private Properties properties;
    private RobotsScoresContext highscores;

    public RobotsWindow (Gtk.Application app,
                         Properties properties,
                         GameConfigs game_configs,
                         Assets assets,
                         SoundPlayer sound_player
    ) throws Error {
        Object (application: app);
        remember_window_size (this, new WindowSizeSettings ("org.gnome.Robots"));
        this.properties = properties;

        headerbar = new HeaderBar ();
        headerbar.set_title (_("Robots"));
        headerbar.set_show_close_button (true);
        set_titlebar (headerbar);

        var appmenu = app.get_menu_by_id ("primary-menu");
        var menu_button = new MenuButton ();
        var icon = new Image.from_icon_name ("open-menu-symbolic", IconSize.BUTTON);
        menu_button.set_image (icon);
        menu_button.set_menu_model (appmenu);
        menu_button.show ();
        headerbar.pack_end (menu_button);

        GLib.ActionEntry[] win_entries = {
            { "random-teleport",  random_teleport_cb },
            { "safe-teleport",    safe_teleport_cb   },
            { "wait",             wait_cb            },
        };
        add_action_entries (win_entries, this);

        var game = new Game ();
        game_area = new GameArea (game,
                                  game_configs,
                                  assets,
                                  sound_player,
                                  properties);
        game_area.updated.connect (game => update_game_status (game));

        var gridframe = new Games.GridFrame (game.width, game.height);
        gridframe.add (game_area);

        var hbox = button_box ();

        var vbox = new Box (Orientation.VERTICAL, 0);
        vbox.pack_start (gridframe, true, true, 0);
        vbox.pack_start (hbox, false, false, 0);

        add (vbox);

        key_controller = new EventControllerKey (this);
        key_controller.key_pressed.connect (keyboard_cb);

        highscores = new RobotsScoresContext (this);
        game_area.add_score.connect ((game_type, score) => {
            highscores.add_game_score (game_type, score);
        });
    }

    private Box button_box () {
        var hbox = new Box (Orientation.HORIZONTAL, 0);
        var size_group = new SizeGroup (SizeGroupMode.BOTH);

        var style_context = hbox.get_style_context ();
        style_context.add_class ("linked");

        {
            var label = new Label.with_mnemonic (_("Teleport _Randomly"));
            label.margin_top = 15;
            label.margin_bottom = 15;
            var button = new Button ();
            button.add (label);
            button.set_action_name ("win.random-teleport");
            size_group.add_widget (button);
            hbox.pack_start (button, true, true, 0);
        }

        {
            safe_teleports_label = new Label (null);
            safe_teleports_label.set_justify (Justification.CENTER);
            safe_teleports_label.margin_top = 15;
            safe_teleports_label.margin_bottom = 15;
            var button = new Button ();
            button.add (safe_teleports_label);
            button.set_action_name ("win.safe-teleport");
            size_group.add_widget (button);
            hbox.pack_start (button, true, true, 0);
        }

        {
            var label = new Label.with_mnemonic (_("_Wait for Robots"));
            label.margin_top = 15;
            label.margin_bottom = 15;
            var button = new Button ();
            button.add (label);
            button.set_action_name ("win.wait");
            size_group.add_widget (button);
            hbox.pack_start (button, true, true, 0);
        }

        return hbox;
    }

    public void update_game_status (Game game) {
        headerbar.set_subtitle (
            _("Level: %d\tScore: %d").printf (game.status.current_level, game.status.score));

        /* Second line of safe teleports button label. %d is the number of teleports remaining. */
        var remaining_teleports_text = _("(Remaining: %d)").printf (game.status.safe_teleports);
        /* First line of safe teleports button label. */
        var button_text = "%s\n<small>%s</small>".printf (_("Teleport _Safely"), remaining_teleports_text);
        safe_teleports_label.set_markup_with_mnemonic (button_text);

        var is_playing = game.state != Game.State.COMPLETE && game.state != Game.State.DEAD;

        var action1 = (SimpleAction) lookup_action ("random-teleport");
        action1.set_enabled (is_playing);

        var action2 = (SimpleAction) lookup_action ("safe-teleport");
        action2.set_enabled (is_playing && game.status.safe_teleports > 0);

        var action3 = (SimpleAction) lookup_action ("wait");
        action3.set_enabled (is_playing);
    }

    private void random_teleport_cb () {
        game_area.player_command (PlayerCommand.RANDOM_TELEPORT);
    }

    private void safe_teleport_cb () {
        game_area.player_command (PlayerCommand.SAFE_TELEPORT);
    }

    private void wait_cb () {
        game_area.player_command (PlayerCommand.WAIT);
    }

    private bool keyboard_cb (uint keyval, uint keycode, Gdk.ModifierType state) {
        /* This is a bit of a kludge to let through accelerator keys, otherwise
         * if N is used as a key, then Ctrl-N is never picked up. The cleaner
         * option, making the signal a connect_after signal skims the arrow keys
         * before we can get to them which is a bigger problem. */
        if ((state & (Gdk.ModifierType.CONTROL_MASK | Gdk.ModifierType.MOD1_MASK)) != 0) {
            return false;
        }

        char pressed = ((char) keyval).toupper ();

        for (var i = 0; i < properties.keys.size; ++i) {
            if (pressed == ((char)properties.keys[i]).toupper ()) {
                game_area.player_command ((PlayerCommand)i);
                return true;
            }
        }

        return false;
    }

    public void start_new_game () {
        game_area.start_new_game ();
    }

    public void show_highscores () {
        highscores.run_dialog ();
    }
}

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

        make_cursors ();

        try {
            assets = new DirectoryAssets.from_directory (DATA_DIRECTORY);
            game_configs = new GameConfigs.load ();
            sound_player = new SoundPlayer ();
        } catch (Error e) {
            critical ("%s", e.message);

            var errordialog = new MessageDialog.with_markup (get_active_window (), // is it null?
                                                             DialogFlags.MODAL,
                                                             MessageType.ERROR,
                                                             ButtonsType.OK,
                                                             "<b>%s</b>\n\n%s",
                                                             _("No game data could be found."),
                                                             _("The program Robots was unable to find any valid game configuration files. Please check that the program is installed correctly."));
            errordialog.set_resizable (false);
            errordialog.run ();

            quit ();
        }

        var settings = new GLib.Settings ("org.gnome.Robots");
        properties = new Properties (settings);
    }

    protected override void activate () {
        var window = get_active_window () as RobotsWindow;
        if (window != null) {
            window.present_with_time (get_current_event_time ());
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

        window.show_all ();
    }

    private void new_game_cb () {
        var window = get_active_window () as RobotsWindow;
        if (window != null) {
            var dialog = new MessageDialog (get_active_window (),
                                            DialogFlags.MODAL,
                                            MessageType.QUESTION,
                                            ButtonsType.NONE,
                                            _("Are you sure you want to discard the current game?"));

            dialog.add_button (_("Keep _Playing"), ResponseType.REJECT);
            dialog.add_button (_("_New Game"), ResponseType.ACCEPT);

            var ret = dialog.run ();
            dialog.destroy ();

            if (ret == ResponseType.ACCEPT) {
                window.start_new_game ();
            }
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
        try {
            show_uri_on_window (get_active_window (),
                                "help:gnome-robots",
                                get_current_event_time ());
        } catch (Error error) {
            warning ("Failed to show help: %s", error.message);
        }
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

public static int main (string[] args) {
    Intl.setlocale ();
    Intl.bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    Intl.bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    Intl.textdomain (GETTEXT_PACKAGE);

    var app = new RobotsApplication ();
    return app.run (args);
}

