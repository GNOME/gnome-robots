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

const int KB_TEXT_WIDTH = 60;
const int KB_TEXT_HEIGHT = 32;

const string KEY_PREFERENCES_GROUP = "preferences";
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
    int selected_config;
    uint keys[9];
    string themename;
}

Dialog propbox = null;
Properties properties;

/**
 * Applies the changes made by the user
 **/
void apply_changes () {
    load_keys ();
    keyboard_set (properties.keys);
}

/**
 * handles apply button events
 *
 * Returns:
 * TRUE if the event was handled
 **/
void apply_cb () {
    apply_changes ();

    propbox.destroy ();
    propbox = null;
}

/**
 * handles pixmap selection messages
 **/
void pmap_selection (ComboBox combo) {
    TreeIter iter;
    if (combo.get_active_iter (out iter)) {
        Themes model = combo.get_model () as Themes;

        string theme_name;
        string theme_path;
        model.get_values (iter, out theme_name, out theme_path);

        /* FIXME: Should be de-suffixed. */
        properties.themename = theme_name;

        conf_set_theme (properties.themename);

        try {
            load_game_graphics (theme_path);
        } catch (Error e) {
            // TODO
        }
        clear_game_area ();
    }
}

/**
 * handles configuration selection messages
 **/
void type_selection (int num) {
    properties.selected_config = num;

    var config = game_configs.@get ((uint)properties.selected_config);
    var config_name = config.name ();
    conf_set_configuration (config_name);

    game.config = game_configs.@get ((uint)properties.selected_config);
    game.start_new_game ();
}

/**
 * handles message from the default key buttons
 **/
void defkey_cb () {
    for (int i = 0; i < N_KEYS; ++i) {
        string key = "key%02d".printf (i);
        settings.reset (key);
        properties.keys[i] = settings.get_default_value (key).get_uint32 ();
    }

    keyboard_set (properties.keys);
}


/**
 * fills the listbox with configuration names
 **/
void fill_typemenu (ComboBoxText menu) {
    for (int i = 0; i < game_configs.count (); ++i) {
        var config = game_configs.get_name ((uint)i);
        menu.append_text (config);
    }
    menu.set_active (properties.selected_config);
}

ComboBox create_theme_picker (Themes themes, string current_theme) {
    var widget = new ComboBox.with_model (themes);
    var renderer = new CellRendererText ();
    widget.pack_start (renderer, true);
    widget.add_attribute (renderer, "text", 0);

    TreeIter? iter = themes.find_iter_by_name (current_theme);
    if (iter != null) {
        widget.set_active_iter (iter);
    } else {
        widget.set_active (0);
    }

    return widget;
}

void bg_color_callback (ColorChooser color_chooser) {
    properties.bgcolour = color_chooser.get_rgba ();
    set_background_color (properties.bgcolour);
    clear_game_area ();
    conf_set_background_color (properties.bgcolour);
}

public string properties_theme_name () {
    return properties.themename;
}

/**
 * show_properties_dialog
 *
 * Description:
 * displays the properties dialog
 **/
public void show_properties_dialog () {
    var themes = get_themes ();

    if (propbox != null)
        return;

    propbox = new Dialog.with_buttons (_("Preferences"),
                                       window,
                                       DialogFlags.USE_HEADER_BAR | DialogFlags.MODAL);

    propbox.border_width = 5;
    propbox.get_content_area ().set_spacing (2);
    propbox.destroy.connect (() => propbox = null);

    /* Set up notebook and add it to hbox of the gtk_dialog */
    var notebook = new Notebook ();
    notebook.border_width = 5;
    propbox.get_content_area ().pack_start (notebook, true, true, 0);

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
    typemenu.changed.connect ((combo) => {
        type_selection(combo.active);
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
        conf_set_use_safe_moves (properties.safe_moves);
        super_safe_chkbox.set_sensitive (properties.safe_moves);
    });
    super_safe_chkbox.toggled.connect ((toggle) => {
        properties.super_safe_moves = toggle.get_active ();
        conf_set_use_super_safe_moves (properties.super_safe_moves);
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

    var pmapmenu = create_theme_picker (themes, properties.themename);
    pmapmenu.changed.connect ((combo) => pmap_selection (combo));
    label.set_mnemonic_widget (pmapmenu);
    grid.attach (pmapmenu, 1, 0, 1, 1);

    label = new Label.with_mnemonic (_("_Background color:"));
    label.set_halign (Align.START);
    grid.attach (label, 0, 1, 1, 1);

    var w = new ColorButton ();
    w.set_rgba (properties.bgcolour);
    w.color_set.connect((color) => bg_color_callback(color));
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
    controls_list.add_control ("key00", _("Key to move NW"), settings.get_default_value ("key00").get_int32 ());
    controls_list.add_control ("key01", _("Key to move N"),  settings.get_default_value ("key01").get_int32 ());
    controls_list.add_control ("key02", _("Key to move NE"), settings.get_default_value ("key02").get_int32 ());
    controls_list.add_control ("key03", _("Key to move W"),  settings.get_default_value ("key03").get_int32 ());
    controls_list.add_control ("key04", _("Key to hold"),    settings.get_default_value ("key04").get_int32 ());
    controls_list.add_control ("key05", _("Key to move E"),  settings.get_default_value ("key05").get_int32 ());
    controls_list.add_control ("key06", _("Key to move SW"), settings.get_default_value ("key06").get_int32 ());
    controls_list.add_control ("key07", _("Key to move S"),  settings.get_default_value ("key07").get_int32 ());
    controls_list.add_control ("key08", _("Key to move SE"), settings.get_default_value ("key08").get_int32 ());

    vbox.pack_start (controls_list, true, true, 0);

    var hbox = new ButtonBox (Orientation.HORIZONTAL);
    hbox.set_layout (ButtonBoxStyle.START);
    vbox.pack_start (hbox, false, false, 0);

    var dbut = new Button.with_mnemonic (_("_Restore Defaults"));
    dbut.clicked.connect (() => defkey_cb());
    hbox.pack_start (dbut, false, false, 0);

    label = new Label.with_mnemonic (_("Keyboard"));
    notebook.append_page (kpage, label);

    propbox.delete_event.connect (() => {
        propbox = null;
        return false;
    });
    propbox.response.connect (() => apply_cb());

    propbox.show_all ();
}

/**
 * loads the game properties from a file
 **/
public void load_properties () throws Error {
    load_keys ();

    var bgcolour = settings.get_string (KEY_BACKGROUND_COLOR);
    RGBA colour = RGBA ();
    colour.parse (bgcolour);
    properties.bgcolour = colour;
    set_background_color (properties.bgcolour);

    properties.themename = settings.get_string (KEY_THEME);

    var cname = settings.get_string (KEY_CONFIGURATION);

    properties.selected_config = 0;
    for (int i = 0; i < game_configs.count (); ++i) {
        var config = game_configs.get_name ((uint)i);
        if (cname == config) {
            properties.selected_config = i;
            break;
        }
    }

    properties.safe_moves       = settings.get_boolean (KEY_SAFE_MOVES);
    properties.super_safe_moves = settings.get_boolean (KEY_SUPER_SAFE_MOVES);
    properties.sound            = settings.get_boolean (KEY_ENABLE_SOUND);
    properties.show_toolbar     = settings.get_boolean (KEY_SHOW_TOOLBAR);

    var themes = get_themes ();
    var iter = themes.find_best_match (properties.themename);

    string theme_path;
    themes.get_values (iter, out properties.themename, out theme_path);

    load_game_graphics (theme_path);

    keyboard_set (properties.keys);
}

public void load_keys () {
    for (int i = 0; i < N_KEYS; i++) {
        var key = "key%02d".printf (i);
        properties.keys[i] = settings.get_int (key);
    }
}

public void conf_set_theme (string val) {
    settings.set_string (KEY_THEME, val);
}

void conf_set_background_color (RGBA c) {
    var colour = "#%04x%04x%04x".printf ((int) (c.red * 65535 + 0.5), (int) (c.green * 65535 + 0.5), (int) (c.blue * 65535 + 0.5));
    settings.set_string (KEY_BACKGROUND_COLOR, colour);
}

public void conf_set_configuration (string val) {
    settings.set_string (KEY_CONFIGURATION, val);
}

public void conf_set_use_safe_moves (bool val) {
    settings.set_boolean (KEY_SAFE_MOVES, val);
}

public void conf_set_use_super_safe_moves (bool val) {
    settings.set_boolean (KEY_SUPER_SAFE_MOVES, val);
}

public void conf_set_enable_sound (bool val) {
    settings.set_boolean (KEY_ENABLE_SOUND, val);
}

public void conf_set_show_toolbar (bool val) {
    settings.set_boolean (KEY_SHOW_TOOLBAR, val);
}

public void conf_set_control_key (int i, uint keyval) {
    var key = "key%02d".printf (i);
    var keyval_name = keyval_name (keyval);
    settings.set_string (key, keyval_name);
}

/**
 * saves the game properties to a file
 **/
public void save_properties () {
    for (int i = 0; i < N_KEYS; i++) {
        conf_set_control_key (i, properties.keys[i]);
    }

    conf_set_theme (properties.themename);

    var config = game_configs.get_name ((uint)properties.selected_config);
    conf_set_configuration (config);

    conf_set_use_safe_moves (properties.safe_moves);
    conf_set_use_super_safe_moves (properties.super_safe_moves);
    conf_set_enable_sound (properties.sound);
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

