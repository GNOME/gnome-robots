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
using Gdk;

public GameConfigs game_configs;

const string KEY_BACKGROUND_COLOR  = "background-color";
const string KEY_CONFIGURATION     = "configuration";
const string KEY_ENABLE_SOUND      = "enable-sound";
const string KEY_SAFE_MOVES        = "use-safe-moves";
const string KEY_SHOW_TOOLBAR      = "show-toolbar";
const string KEY_SUPER_SAFE_MOVES  = "use-super-safe-moves";
const string KEY_THEME             = "theme";
const string KEY_CONTROL_KEY       = "key%02d";

const int N_KEYS = 9;

struct Properties {
    bool safe_moves;
    bool super_safe_moves;
    bool sound;
    bool show_toolbar;
    Gdk.RGBA bgcolour;
    string selected_config;
    uint keys[9];
    string themename;
}

Properties properties;

/**
 * handles configuration selection messages
 **/
void type_selection (string config_name) {
    properties.selected_config = config_name;
    conf_set_configuration (config_name);

    game.config = game_configs.find_by_name (config_name);
    game.start_new_game ();
    game_area.queue_draw ();
}

/**
 * handles message from the default key buttons
 **/
void defkey_cb () {
    for (int i = 0; i < N_KEYS; ++i) {
        string key = "key%02d".printf (i);
        settings.reset (key);
        properties.keys[i] = settings.get_default_value (key).get_int32 ();
    }

    keyboard_set (properties.keys);
}

/**
 * fills the listbox with configuration names
 **/
void fill_typemenu (ComboBoxText menu) {
    int active_index = 0;
    for (int i = 0; i < game_configs.count (); ++i) {
        var config = game_configs.@get ((uint)i);

        var config_name = config.name ();
        menu.append_text (config_name);

        if (config_name == properties.selected_config) {
            active_index = i;
        }
    }
    menu.set_active (active_index);
}

class ThemePicker : ComboBox {

    private Themes themes;

    public ThemePicker (Themes themes, string current_theme) {
        Object (model: themes);
        this.themes = themes;

        var renderer = new CellRendererText ();
        pack_start (renderer, true);
        add_attribute (renderer, "text", 0);

        TreeIter? iter = themes.find_iter_by_name (current_theme);
        if (iter != null) {
            set_active_iter (iter);
        } else {
            set_active (0);
        }

        changed.connect (theme_changed_cb);
    }

    public signal void theme_changed (Theme theme);

    private void theme_changed_cb () {
        TreeIter iter;
        if (get_active_iter (out iter)) {
            string theme_name;
            string theme_path;
            themes.get_values (iter, out theme_name, out theme_path);

            try {
                var theme = new Theme.from_file (theme_path, theme_name);
                theme_changed (theme);
            } catch (Error e) {
                warning ("Cannot change theme to %s (placed at %s): %s",
                    theme_name,
                    theme_path,
                    e.message);
            }
        }
    }
}

public class PropertiesDialog : Dialog {

    public PropertiesDialog (Gtk.Window parent, Themes themes) {
        Object (use_header_bar: 1,
                title: _("Preferences"),
                transient_for: parent,
                modal: true,
                border_width: 5);

        get_content_area ().set_spacing (2);

        /* Set up notebook and add it to hbox of the gtk_dialog */
        var notebook = new Notebook ();
        notebook.border_width = 5;
        get_content_area ().pack_start (notebook, true, true, 0);

        /* The configuration page */
        var cpage = new Box (Orientation.VERTICAL, 18);
        cpage.border_width = 12;

        var grid = new Grid ();
        grid.set_row_spacing (6);
        grid.set_column_spacing (12);
        cpage.pack_start (grid, false, false, 0);

        var label = new Label (_("Game Type"));
        grid.attach (label, 0, 0, 1, 1);

        var typemenu = new ComboBoxText ();
        fill_typemenu (typemenu);
        typemenu.changed.connect (() => {
            type_selection(typemenu.get_active_text ());
        });
        grid.attach (typemenu, 1, 0, 1, 1);

        var safe_chkbox = new CheckButton.with_mnemonic (_("_Use safe moves"));
        safe_chkbox.set_active (properties.safe_moves);
        safe_chkbox.set_tooltip_text (_("Prevent accidental moves that result in getting killed."));
        grid.attach (safe_chkbox, 0, 1, 2, 1);

        var super_safe_chkbox = new CheckButton.with_mnemonic (_("U_se super safe moves"));
        super_safe_chkbox.set_active (properties.super_safe_moves);
        super_safe_chkbox.set_tooltip_text (_("Prevents all moves that result in getting killed."));
        super_safe_chkbox.set_sensitive (properties.safe_moves);
        grid.attach (super_safe_chkbox, 0, 2, 2, 1);

        safe_chkbox.toggled.connect ((toggle) => {
            properties.safe_moves = toggle.active;
            settings.set_boolean (KEY_SAFE_MOVES, properties.safe_moves);
            super_safe_chkbox.set_sensitive (properties.safe_moves);
        });
        super_safe_chkbox.toggled.connect ((toggle) => {
            properties.super_safe_moves = toggle.get_active ();
            settings.set_boolean (KEY_SUPER_SAFE_MOVES, properties.super_safe_moves);
        });

        var sound_chkbox = new CheckButton.with_mnemonic (_("_Enable sounds"));
        sound_chkbox.set_active (properties.sound);
        sound_chkbox.toggled.connect ((toggle) => {
            properties.sound = toggle.active;
            conf_set_enable_sound (properties.sound);
        });
        sound_chkbox.set_tooltip_text (_("Play sounds for events like winning a level and dying."));
        grid.attach (sound_chkbox, 0, 3, 2, 1);

        label = new Label.with_mnemonic (_("Game"));
        notebook.append_page (cpage, label);

        /* The graphics page */
        var gpage = new Box (Orientation.VERTICAL, 18);
        gpage.set_border_width (12);

        grid = new Grid ();
        grid.set_row_spacing (6);
        grid.set_column_spacing (12);
        gpage.pack_start (grid, false, false, 0);

        label = new Label.with_mnemonic (_("_Image theme:"));
        label.set_hexpand (true);
        label.set_halign (Align.START);
        grid.attach (label, 0, 0, 1, 1);

        var theme_picker = new ThemePicker (themes, properties.themename);
        theme_picker.theme_changed.connect (theme_changed);
        label.set_mnemonic_widget (theme_picker);
        grid.attach (theme_picker, 1, 0, 1, 1);

        label = new Label.with_mnemonic (_("_Background color:"));
        label.set_halign (Align.START);
        grid.attach (label, 0, 1, 1, 1);

        var w = new ColorButton ();
        w.set_rgba (properties.bgcolour);
        w.color_set.connect((color) => bg_color_changed(color));
        label.set_mnemonic_widget (w);
        grid.attach (w, 1, 1, 1, 1);

        label = new Label.with_mnemonic (_("Appearance"));
        notebook.append_page (gpage, label);

        /* The keyboard page */
        var kpage = new Box (Orientation.VERTICAL, 18);
        kpage.set_border_width (12);

        var vbox = new Box (Orientation.VERTICAL, 6);
        kpage.pack_start (vbox, true, true, 0);

        var controls_list = new GamesControlsList (settings);
        controls_list.add_control ("key00", _("Key to move NW"));
        controls_list.add_control ("key01", _("Key to move N"));
        controls_list.add_control ("key02", _("Key to move NE"));
        controls_list.add_control ("key03", _("Key to move W"));
        controls_list.add_control ("key04", _("Key to hold"));
        controls_list.add_control ("key05", _("Key to move E"));
        controls_list.add_control ("key06", _("Key to move SW"));
        controls_list.add_control ("key07", _("Key to move S"));
        controls_list.add_control ("key08", _("Key to move SE"));

        vbox.pack_start (controls_list, true, true, 0);

        var hbox = new ButtonBox (Orientation.HORIZONTAL);
        hbox.set_layout (ButtonBoxStyle.START);
        vbox.pack_start (hbox, false, false, 0);

        var dbut = new Button.with_mnemonic (_("_Restore Defaults"));
        dbut.clicked.connect (() => defkey_cb());
        hbox.pack_start (dbut, false, false, 0);

        label = new Label.with_mnemonic (_("Keyboard"));
        notebook.append_page (kpage, label);
    }

    private void theme_changed (Theme theme) {
        /* FIXME: Should be de-suffixed. */
        properties.themename = theme.name;
        settings.set_string (KEY_THEME, theme.name);

        game_area.theme = theme;
        game_area.queue_draw ();
    }

    private void bg_color_changed (ColorChooser color_chooser) {
        properties.bgcolour = color_chooser.get_rgba ();

        var colour = rgba_to_string (properties.bgcolour);
        settings.set_string (KEY_BACKGROUND_COLOR, colour);

        game_area.background_color = properties.bgcolour;
        game_area.queue_draw ();
    }

}

/**
 * show_properties_dialog
 *
 * Description:
 * displays the properties dialog
 **/
public void show_properties_dialog () {
    var themes = get_themes ();

    var propbox = new PropertiesDialog (window, themes);
    propbox.show_all ();
    propbox.run ();
    propbox.destroy ();

    keyboard_set (properties.keys);
}

public void load_properties () {
    for (int i = 0; i < N_KEYS; i++) {
        var key = "key%02d".printf (i);
        properties.keys[i] = settings.get_int (key);
    }
    properties.bgcolour         = string_to_rgba (settings.get_string (KEY_BACKGROUND_COLOR));
    properties.themename        = settings.get_string (KEY_THEME);
    properties.selected_config  = settings.get_string (KEY_CONFIGURATION);
    properties.safe_moves       = settings.get_boolean (KEY_SAFE_MOVES);
    properties.super_safe_moves = settings.get_boolean (KEY_SUPER_SAFE_MOVES);
    properties.sound            = settings.get_boolean (KEY_ENABLE_SOUND);
    properties.show_toolbar     = settings.get_boolean (KEY_SHOW_TOOLBAR);
}

public Theme get_theme_from_properties () throws Error {
    var themes = get_themes ();
    var iter = themes.find_best_match (properties.themename);

    string theme_path;
    themes.get_values (iter, out properties.themename, out theme_path);

    return new Theme.from_file (theme_path, properties.themename);
}

public void conf_set_configuration (string val) {
    settings.set_string (KEY_CONFIGURATION, val);
}

public void conf_set_enable_sound (bool val) {
    settings.set_boolean (KEY_ENABLE_SOUND, val);
}

/**
 * properties_safe_moves
 *
 * Description:
 * returns safe-moves setting
 *
 * Returns:
 * TRUE if safe-moves are selected
 **/
public bool properties_safe_moves () {
    return properties.safe_moves;
}


/**
 * properties_super_safe_moves
 *
 * Description:
 * returns super-safe-moves setting
 *
 * Returns:
 * TRUE if safe-moves are selected
 **/
public bool properties_super_safe_moves () {
    return properties.super_safe_moves;
}


/**
 * properties_sound
 *
 * Description:
 * returns sound setting
 *
 * Returns:
 * TRUE if sound is selected
 **/
public bool properties_sound () {
    return properties.sound;
}
