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

const string KEY_GEOMETRY_GROUP = "geometry";

ApplicationWindow window = null;
int window_width = 0;
int window_height = 0;
bool window_is_maximized = false;
GameArea game_area = null;
Games.Scores.Context highscores;
GLib.Settings settings;

const GLib.ActionEntry[] app_entries = {
    { "new-game",         new_game_cb    },
    { "preferences",      preferences_cb },
    { "scores",           scores_cb,     },
    { "help",             help_cb,       },
    { "about",            about_cb,      },
    { "quit",             quit_cb,       },
};

const GLib.ActionEntry[] win_entries = {
    { "random-teleport",  random_teleport_cb },
    { "safe-teleport",    safe_teleport_cb   },
    { "wait",             wait_cb            },
};

int safe_teleports = 0;
Label safe_teleports_label;
HeaderBar headerbar;

EventControllerKey key_controller;
uint control_keys[12];

public void set_move_action_sensitivity (bool state) {
    var action1 = (SimpleAction) window.lookup_action ("random-teleport");
    action1.set_enabled (state);

    var action2 = (SimpleAction) window.lookup_action ("safe-teleport");
    action2.set_enabled (state && safe_teleports > 0);

    var action3 = (SimpleAction) window.lookup_action ("wait");
    action3.set_enabled (state);
}

public void update_game_status (int score, int current_level, int safes) {
    /* Window subtitle. The first %d is the level, the second is the score. \t creates a tab. */
    var subtitle = _("Level: %d\tScore: %d").printf (current_level, score);
    headerbar.set_subtitle (subtitle);

    safe_teleports = safes;

    var action = (SimpleAction) window.lookup_action ("safe-teleport");
    action.set_enabled (safe_teleports > 0);

    /* Second line of safe teleports button label. %d is the number of teleports remaining. */
    var remaining_teleports_text = _("(Remaining: %d)").printf (safe_teleports);
    /* First line of safe teleports button label. */
    var button_text = "%s\n<small>%s</small>".printf (_("Teleport _Safely"), remaining_teleports_text);
    safe_teleports_label.set_markup_with_mnemonic (button_text);
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

void preferences_cb () {
    show_properties_dialog ();
}

void scores_cb () {
    game.show_scores ();
}

void help_cb () {
    try {
        show_uri_on_window (window, "help:gnome-robots", get_current_event_time ());
    } catch (Error error) {
        warning ("Failed to show help: %s", error.message);
    }
}

void about_cb () {
  string[] authors = { "Mark Rae <m.rae@inpharmatica.co.uk>" };

  string[] artists = { "Kirstie Opstad <K.Opstad@ed.ac.uk>", "Rasoul M.P. Aghdam (player death sound)" };

  string[] documenters = { "Aruna Sankaranarayanan" };

  show_about_dialog (window,
                     "name", _("Robots"),
                     "version", VERSION,
                     "copyright", "Copyright © 1998–2008 Mark Rae\nCopyright © 2014–2016 Michael Catanzaro",
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

void quit_cb () {
    quit_game ();
}

void new_game_cb () {
    var dialog = new MessageDialog (window,
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
    }
}

void random_teleport_cb () {
    game.keypress (Game.KeyboardControl.RTEL);
}

void safe_teleport_cb () {
    game.keypress (Game.KeyboardControl.TELE);
}

void wait_cb () {
    game.keypress (Game.KeyboardControl.WAIT);
}

bool window_configure_event_cb () {
    if (!window_is_maximized)
        window.get_size (out window_width, out window_height);
    return false;
}

bool window_state_event_cb (Widget widget, Gdk.EventWindowState event) {
    if ((event.changed_mask & Gdk.WindowState.MAXIMIZED) != 0)
        window_is_maximized = (event.new_window_state & Gdk.WindowState.MAXIMIZED) != 0;
    return false;
}

public void quit_game () {
    window.close ();
}

void startup (Gtk.Application app) {
    Environment.set_application_name (_("Robots"));

    settings = new GLib.Settings ("org.gnome.Robots");

    Window.set_default_icon_name ("org.gnome.Robots");

    app.add_action_entries (app_entries, app);

    app.set_accels_for_action ("app.new-game", { "<Primary>n" });
    app.set_accels_for_action ("app.help", { "F1" });
    app.set_accels_for_action ("app.quit", { "<Primary>q" });

    make_cursors ();
}

void shutdown (Gtk.Application app) {
    settings.set_int ("window-width", window_width);
    settings.set_int ("window-height", window_height);
    settings.set_boolean ("window-is-maximized", window_is_maximized);
}

Games.Scores.Category? create_category_from_key (string key) {
    string name = category_name_from_key (key);
    if (name == null)
        return null;
    return new Games.Scores.Category (key, name);
}

/**
 * Initialises the keyboard actions when the game first starts up
 **/
void init_keyboard () {
    key_controller = new EventControllerKey (window);
    key_controller.key_pressed.connect (keyboard_cb);
}

void keyboard_set (uint[] keys) {
    for (int i = 0; i < 12; ++i) {
        control_keys[i] = keys[i];
    }
}

bool keyboard_cb (EventControllerKey controller, uint keyval, uint keycode, Gdk.ModifierType state) {
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
            game.keypress ((Game.KeyboardControl)i);
            return true;
        }
    }

    return false;
}

void activate (Gtk.Application app) {
    load_properties ();

    if (window != null) {
        window.present_with_time (get_current_event_time ());
        return;
    }

    game = new Game ();

    headerbar = new HeaderBar ();
    headerbar.set_title (_("Robots"));
    headerbar.set_show_close_button (true);

    var appmenu = app.get_menu_by_id ("primary-menu");
    var menu_button = new MenuButton ();
    var icon = new Image.from_icon_name ("open-menu-symbolic", IconSize.BUTTON);
    menu_button.set_image (icon);
    menu_button.set_menu_model (appmenu);
    menu_button.show ();
    headerbar.pack_end (menu_button);

    window = new ApplicationWindow (app);
    window.set_titlebar (headerbar);
    window.configure_event.connect (window_configure_event_cb);
    window.window_state_event.connect (window_state_event_cb);
    window.set_default_size (settings.get_int ("window-width"), settings.get_int ("window-height"));
    if (settings.get_boolean ("window-is-maximized")) {
        window.maximize ();
    }

    window.add_action_entries (win_entries, app);

    Theme theme = null;
    try {
        theme = get_theme_from_properties ();
    } catch (Error e) {
        // error ("%s", e.message);
        // TODO message box
        app.quit ();
    }

    game_area = new GameArea (game, theme);
    game_area.destroy.connect (() => game_area = null);

    var gridframe = new Games.GridFrame (GAME_WIDTH, GAME_HEIGHT);
    gridframe.add (game_area);

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

    var vbox = new Box (Orientation.VERTICAL, 0);
    vbox.pack_start (gridframe, true, true, 0);
    vbox.pack_start (hbox, false, false, 0);

    window.add (vbox);

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
        app.quit ();
    }

    try {
        set_background_color (properties.bgcolour);

        load_game_graphics ();

        keyboard_set (properties.keys);
    } catch (Error e) {
        // error ("%s", e.message);
        // TODO message box
        app.quit ();
    }

    init_keyboard ();

    game.config = game_configs.find_by_name (properties.selected_config);
    game.init_game ();

    GLib.Settings.sync ();
}

public static int main (string[] args) {
    Intl.setlocale ();
    Intl.bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    Intl.bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    Intl.textdomain (GETTEXT_PACKAGE);

    var app = new Gtk.Application ("org.gnome.Robots", ApplicationFlags.FLAGS_NONE);

    app.startup.connect (() => startup (app));
    app.shutdown.connect (() => shutdown (app));
    app.activate.connect (() => activate (app));

    return app.run (args);
}

