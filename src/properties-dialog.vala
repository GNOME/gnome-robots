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

class GameConfigPicker : ComboBoxText {

    private GameConfigs game_configs;

    public signal void game_config_changed (GameConfig game_config);

    public GameConfigPicker (GameConfigs game_configs, string current_config) {
        Object ();
        this.game_configs = game_configs;

        int active_index = 0;
        for (int i = 0; i < game_configs.count (); ++i) {
            var config = game_configs[(uint)i];

            var config_name = config.name ();
            append_text (config_name);

            if (config_name == current_config) {
                active_index = i;
            }
        }
        set_active (active_index);

        changed.connect (() => {
            var config_name = get_active_text ();
            var game_config = game_configs.find_by_name (config_name);
            game_config_changed(game_config);
        });
    }
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

    private Properties properties;

    public PropertiesDialog (Gtk.Window parent,
                             GameConfigs game_configs,
                             Themes themes,
                             Properties properties
    ) {
        Object (use_header_bar: 1,
                title: _("Preferences"),
                transient_for: parent,
                modal: true,
                border_width: 5);
        this.properties = properties;

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

        var typemenu = new GameConfigPicker (game_configs, properties.selected_config);
        typemenu.game_config_changed.connect (game_config_changed);
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
            super_safe_chkbox.set_sensitive (properties.safe_moves);
        });
        super_safe_chkbox.toggled.connect ((toggle) => {
            properties.super_safe_moves = toggle.get_active ();
        });

        var sound_chkbox = new CheckButton.with_mnemonic (_("_Enable sounds"));
        sound_chkbox.set_active (properties.sound);
        sound_chkbox.toggled.connect ((toggle) => {
            properties.sound = toggle.active;
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

        var theme_picker = new ThemePicker (themes, properties.theme);
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

        var controls_list = new GamesControlsList (properties);

        vbox.pack_start (controls_list, true, true, 0);

        var hbox = new ButtonBox (Orientation.HORIZONTAL);
        hbox.set_layout (ButtonBoxStyle.START);
        vbox.pack_start (hbox, false, false, 0);

        var dbut = new Button.with_mnemonic (_("_Restore Defaults"));
        dbut.clicked.connect (reset_keys);
        hbox.pack_start (dbut, false, false, 0);

        label = new Label.with_mnemonic (_("Keyboard"));
        notebook.append_page (kpage, label);
    }

    private void game_config_changed (GameConfig game_config) {
        properties.selected_config = game_config.name ();

        game.config = game_config;
        game.start_new_game ();
        game_area.queue_draw ();
    }

    private void theme_changed (Theme theme) {
        /* FIXME: Should be de-suffixed. */
        properties.theme = theme.name;

        game_area.theme = theme;
        game_area.queue_draw ();
    }

    private void bg_color_changed (ColorChooser color_chooser) {
        properties.bgcolour = color_chooser.get_rgba ();

        game_area.background_color = properties.bgcolour;
        game_area.queue_draw ();
    }

    private void reset_keys () {
        properties.reset_keys ();
        keyboard_set (properties.keys);
    }

    public static void show_dialog (Gtk.Window parent_window,
                                    GameConfigs game_configs,
                                    Themes themes,
                                    Properties properties
    ) {
        var dlg = new PropertiesDialog (window, game_configs, themes, properties);
        dlg.show_all ();
        dlg.run ();
        dlg.destroy ();

        keyboard_set (properties.keys);
    }
}

