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

public void show_preferences (Gtk.Window parent_window,
                              GameConfigs game_configs,
                              GLib.ListStore themes,
                              Properties properties) {
    var window = new Adw.PreferencesWindow ();
    window.transient_for = parent_window;

    /* The configuration page */
    var configuration_page = new Adw.PreferencesPage ();
    configuration_page.title = _("Game");
    configuration_page.add (create_game_group (game_configs, properties));
    configuration_page.add (create_sound_group (properties));
    window.add (configuration_page);

    /* The graphics page */
    var graphics_page = new Adw.PreferencesPage ();
    graphics_page.title = _("Appearance");
    var appearance_group = new Adw.PreferencesGroup ();
    appearance_group.add (create_theme_picker (themes, properties));
    appearance_group.add (create_background_picker (properties));
    graphics_page.add (appearance_group);
    window.add (graphics_page);

    /* The keyboard page */
    var keyboard_page = new Adw.PreferencesPage ();
    keyboard_page.title = _("Keyboard");
    keyboard_page.add (new GamesControlsList (properties));
    window.add (keyboard_page);

    window.present ();
}

private Adw.PreferencesGroup create_game_group (GameConfigs game_configs,
                                                Properties properties) {
    var group = new Adw.PreferencesGroup ();

    var typemenu = create_game_config_picker (game_configs,
                                              properties.selected_config);
    typemenu.notify["selected-item"].connect (() => {
        properties.selected_config = ((StringObject) typemenu.selected_item).get_string ();
    });
    group.add (typemenu);

    var safe_chkbox = new Adw.SwitchRow ();
    safe_chkbox.title = _("_Use safe moves");
    safe_chkbox.use_underline = true;
    safe_chkbox.subtitle = _("Prevent accidental moves that result in getting killed.");
    safe_chkbox.active = properties.safe_moves;
    group.add (safe_chkbox);

    var super_safe_chkbox = new Adw.SwitchRow ();
    super_safe_chkbox.title = _("U_se super safe moves");
    super_safe_chkbox.use_underline = true;
    super_safe_chkbox.subtitle = _("Prevents all moves that result in getting killed.");
    super_safe_chkbox.active = properties.super_safe_moves;
    super_safe_chkbox.sensitive = properties.safe_moves;
    group.add (super_safe_chkbox);

    safe_chkbox.notify["active"].connect (() => {
        properties.safe_moves = safe_chkbox.active;
        super_safe_chkbox.set_sensitive (properties.safe_moves);
    });
    super_safe_chkbox.notify["active"].connect (() => {
        properties.super_safe_moves = super_safe_chkbox.active;
    });

    return group;
}

private Adw.PreferencesGroup create_sound_group (Properties properties) {
    var group = new Adw.PreferencesGroup ();

    var sound_chkbox = new Adw.SwitchRow ();
    sound_chkbox.title = _("_Enable sounds");
    sound_chkbox.use_underline = true;
    sound_chkbox.subtitle = _("Play sounds for events like winning a level and dying.");
    sound_chkbox.active = properties.sound;
    sound_chkbox.notify["active"].connect (() => {
        properties.sound = sound_chkbox.active;
    });
    group.add (sound_chkbox);

    return group;
}

private Adw.ComboRow create_game_config_picker (GameConfigs game_configs,
                                                string current_config) {
    var model = new StringList (null);
    int active_index = 0;
    for (int i = 0; i < game_configs.count (); ++i) {
        var config = game_configs[(uint) i];

        var config_name = config.name ();
        model.append (config_name);

        if (config_name == current_config) {
            active_index = i;
        }
    }

    var row = new Adw.ComboRow ();
    row.title = _("Game Type");
    row.model = model;
    row.selected = active_index;

    return row;
}

private Adw.ComboRow create_theme_picker (GLib.ListStore themes,
                                          Properties properties) {
    var theme_picker = new Adw.ComboRow ();
    theme_picker.title = _("_Image theme:");
    theme_picker.use_underline = true;
    theme_picker.model = themes;
    theme_picker.expression = new PropertyExpression (typeof (Theme), null, "display-name");

    var result = Themes.find_by_name (themes, properties.theme);
    if (result != null) {
        theme_picker.selected = result.index;
    } else {
        theme_picker.selected = 0;
    }

    theme_picker.notify["selected-item"].connect (() => {
        properties.theme = ((Theme) theme_picker.selected_item).name;
    });

    return theme_picker;
}

private Adw.ActionRow create_background_picker (Properties properties) {
    var color_dialog = new ColorDialog ();

    var button = new ColorDialogButton (color_dialog);
    button.set_rgba (properties.bgcolour);
    button.notify["rgba"].connect (() => {
        properties.bgcolour = button.get_rgba ();
    });

    var color_row = new Adw.ActionRow ();
    color_row.title = _("_Background color:");
    color_row.use_underline = true;
    color_row.add_suffix (button);

    return color_row;
}
