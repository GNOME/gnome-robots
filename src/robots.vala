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

RobotsWindow window = null;
int window_width = 0;
int window_height = 0;
bool window_is_maximized = false;
GameArea game_area = null;
Games.Scores.Context highscores;
GLib.Settings settings;
uint control_keys[12];

public class RobotsWindow : ApplicationWindow {

    private HeaderBar headerbar;
    private Label safe_teleports_label;
    // private GameArea game_area;
    private EventControllerKey key_controller;

    public RobotsWindow (Gtk.Application app, GameArea game_area) {
        Object (application: app);
        // this.game_area = game_area;

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

        configure_event.connect (window_configure_event_cb);
        window_state_event.connect (window_state_event_cb);
        set_default_size (settings.get_int ("window-width"), settings.get_int ("window-height"));
        if (settings.get_boolean ("window-is-maximized")) {
            maximize ();
        }

        GLib.ActionEntry[] win_entries = {
            { "random-teleport",  random_teleport_cb },
            { "safe-teleport",    safe_teleport_cb   },
            { "wait",             wait_cb            },
        };
        add_action_entries (win_entries, this);

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

        var action1 = (SimpleAction) window.lookup_action ("random-teleport");
        action1.set_enabled (is_playing);

        var action2 = (SimpleAction) window.lookup_action ("safe-teleport");
        action2.set_enabled (is_playing && game.status.safe_teleports > 0);

        var action3 = (SimpleAction) window.lookup_action ("wait");
        action3.set_enabled (is_playing);
    }

    private void random_teleport_cb () {
        if (game.player_command (PlayerCommand.RANDOM_TELEPORT)) {
            game_area.queue_draw ();
        }
    }

    private void safe_teleport_cb () {
        if (game.player_command (PlayerCommand.SAFE_TELEPORT)) {
            game_area.queue_draw ();
        }
    }

    private void wait_cb () {
        if (game.player_command (PlayerCommand.WAIT)) {
            game_area.queue_draw ();
        }
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

        for (var i = 0; i < control_keys.length; ++i) {
            if (pressed == ((char)control_keys[i]).toupper ()) {
                if (game.player_command ((PlayerCommand)i)) {
                    game_area.queue_draw ();
                }
                return true;
            }
        }

        return false;
    }

    private bool window_configure_event_cb () {
        if (!window_is_maximized)
            window.get_size (out window_width, out window_height);
        return false;
    }

    private bool window_state_event_cb (Gdk.EventWindowState event) {
        if ((event.changed_mask & Gdk.WindowState.MAXIMIZED) != 0)
            window_is_maximized = (event.new_window_state & Gdk.WindowState.MAXIMIZED) != 0;
        return false;
    }
}

public string? category_name_from_key (string key) {
    switch (key) {
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

Games.Scores.Category? create_category_from_key (string key) {
    string name = category_name_from_key (key);
    if (name == null)
        return null;
    return new Games.Scores.Category (key, name);
}

void keyboard_set (uint[] keys) {
    for (int i = 0; i < 12; ++i) {
        control_keys[i] = keys[i];
    }
}

class RobotsApplication : Gtk.Application {
    public RobotsApplication () {
        Object (
            application_id: "org.gnome.Robots",
            flags: ApplicationFlags.FLAGS_NONE);
    }

    protected override void startup () {
        base.startup ();

        Environment.set_application_name (_("Robots"));

        settings = new GLib.Settings ("org.gnome.Robots");

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
    }

    protected override void shutdown () {
        base.shutdown ();
        settings.set_int ("window-width", window_width);
        settings.set_int ("window-height", window_height);
        settings.set_boolean ("window-is-maximized", window_is_maximized);
    }

    protected override void activate () {
        load_properties ();

        if (window != null) {
            window.present_with_time (get_current_event_time ());
            return;
        }

        game_area = create_game_area ();
        window = new RobotsWindow (this, game_area);

        var importer = new Games.Scores.DirectoryImporter ();
        highscores = new Games.Scores.Context.with_importer_and_icon_name ("gnome-robots",
                                                                           /* Label on the scores dialog, next to map type dropdown */
                                                                           _("Game Type:"),
                                                                           window,
                                                                           create_category_from_key,
                                                                           Games.Scores.Style.POINTS_GREATER_IS_BETTER,
                                                                           importer,
                                                                           "org.gnome.Robots");

        window.show_all ();

        try {
            game_configs = new GameConfigs.load ();
        } catch (Error e) {
            /* Oops, no configs, we probably haven't been installed properly. */
            var errordialog = new MessageDialog.with_markup (window,
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

        game_area.background_color = properties.bgcolour;

        keyboard_set (properties.keys);

        game.config = game_configs.find_by_name (properties.selected_config);
        game.start_new_game ();
        game_area.queue_draw ();

        GLib.Settings.sync ();
    }

    private GameArea? create_game_area () {
        try {
            game = new Game ();
            Theme theme = get_theme_from_properties ();
            Bubble yahoo_bubble = new Bubble.from_data_file ("yahoo.png");
            Bubble aieee_bubble = new Bubble.from_data_file ("aieee.png");
            Bubble splat_bubble = new Bubble.from_data_file ("splat.png");
            return new GameArea (game,
                                 theme,
                                 aieee_bubble,
                                 yahoo_bubble,
                                 splat_bubble);
        } catch (Error e) {
            critical ("%s", e.message);
            // TODO message box
            quit ();
            return null; // this line should be unreachable
        }
    }

    private void new_game_cb () {
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
            game.start_new_game ();
            game_area.queue_draw ();
        }
    }

    private void preferences_cb () {
        show_properties_dialog ();
    }

    private void scores_cb () {
        game.show_scores ();
    }

    private void help_cb () {
        try {
            show_uri_on_window (window, "help:gnome-robots", get_current_event_time ());
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
                           "website",
                           "https://wiki.gnome.org/Apps/Robots");
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
