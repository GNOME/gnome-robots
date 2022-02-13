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

    public signal void theme_changed (string theme_name);

    private void theme_changed_cb () {
        TreeIter iter;
        if (get_active_iter (out iter)) {
            string theme_name;
            string theme_path;
            themes.get_values (iter, out theme_name, out theme_path);
            theme_changed (theme_name);
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
                modal: true);
        this.properties = properties;

        /* Set up notebook and add it to hbox of the gtk_dialog */
        var notebook = new Notebook ();
        notebook.margin_top = 10;
        notebook.margin_bottom = 10;
        notebook.margin_start = 10;
        notebook.margin_end = 10;
        get_content_area ().append (notebook);

        /* The configuration page */
        var cpage = form_grid ();

        var label = new Label (_("Game Type"));
        cpage.attach (label, 0, 0, 1, 1);

        var typemenu = create_game_config_picker (game_configs,
                                                  properties.selected_config);
        typemenu.changed.connect (() => {
            properties.selected_config = typemenu.get_active_text ();
        });

        cpage.attach (typemenu, 1, 0, 1, 1);

        var safe_chkbox = new CheckButton.with_mnemonic (_("_Use safe moves"));
        safe_chkbox.set_active (properties.safe_moves);
        safe_chkbox.set_tooltip_text (_("Prevent accidental moves that result in getting killed."));
        cpage.attach (safe_chkbox, 0, 1, 2, 1);

        var super_safe_chkbox = new CheckButton.with_mnemonic (_("U_se super safe moves"));
        super_safe_chkbox.set_active (properties.super_safe_moves);
        super_safe_chkbox.set_tooltip_text (_("Prevents all moves that result in getting killed."));
        super_safe_chkbox.set_sensitive (properties.safe_moves);
        cpage.attach (super_safe_chkbox, 0, 2, 2, 1);

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
        cpage.attach (sound_chkbox, 0, 3, 2, 1);

        label = new Label.with_mnemonic (_("Game"));
        notebook.append_page (cpage, label);

        /* The graphics page */
        var gpage = form_grid ();

        label = new Label.with_mnemonic (_("_Image theme:"));
        label.set_hexpand (true);
        label.set_halign (Align.START);
        gpage.attach (label, 0, 0, 1, 1);

        var theme_picker = new ThemePicker (themes, properties.theme);
        theme_picker.theme_changed.connect (theme_changed);
        label.set_mnemonic_widget (theme_picker);
        gpage.attach (theme_picker, 1, 0, 1, 1);

        label = new Label.with_mnemonic (_("_Background color:"));
        label.set_halign (Align.START);
        gpage.attach (label, 0, 1, 1, 1);

        var w = new ColorButton ();
        w.set_rgba (properties.bgcolour);
        w.color_set.connect((color) => bg_color_changed(color));
        label.set_mnemonic_widget (w);
        gpage.attach (w, 1, 1, 1, 1);

        label = new Label.with_mnemonic (_("Appearance"));
        notebook.append_page (gpage, label);

        /* The keyboard page */
        var kpage = form_grid ();

        var controls_list = new GamesControlsList (properties);
        controls_list.hexpand = true;
        controls_list.vexpand = true;
        kpage.attach (controls_list, 0, 0, 1, 1);

        var hbox = new Box (Orientation.HORIZONTAL, 12);
        kpage.attach (hbox, 0, 1, 1, 1);

        var dbut = new Button.with_mnemonic (_("_Restore Defaults"));
        dbut.clicked.connect (reset_keys);
        hbox.append (dbut);

        label = new Label.with_mnemonic (_("Keyboard"));
        notebook.append_page (kpage, label);
    }

    private void theme_changed (string theme_name) {
        /* FIXME: Should be de-suffixed. */
        properties.theme = theme_name;
    }

    private void bg_color_changed (ColorChooser color_chooser) {
        properties.bgcolour = color_chooser.get_rgba ();
    }

    private void reset_keys () {
        properties.keys.reset_all ();
    }

    public static void show_dialog (Gtk.Window parent_window,
                                    GameConfigs game_configs,
                                    Themes themes,
                                    Properties properties
    ) {
        var dlg = new PropertiesDialog (parent_window,
                                        game_configs,
                                        themes,
                                        properties);
        dlg.response.connect (() => dlg.destroy ());
        dlg.present ();
    }
}

private Grid form_grid () {
    var grid = new Grid ();
    grid.row_spacing = 6;
    grid.column_spacing = 12;
    grid.margin_top = 12;
    grid.margin_bottom = 12;
    grid.margin_start = 12;
    grid.margin_end = 12;
    return grid;
}

private ComboBoxText create_game_config_picker (GameConfigs game_configs,
                                                string current_config
) {
    var cb = new ComboBoxText ();

    int active_index = 0;
    for (int i = 0; i < game_configs.count (); ++i) {
        var config = game_configs[(uint)i];

        var config_name = config.name ();
        cb.append_text (config_name);

        if (config_name == current_config) {
            active_index = i;
        }
    }
    cb.set_active (active_index);

    return cb;
}
