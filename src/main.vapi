public void keyboard_set (uint keys[9]);

[CCode (cheader_filename = "gnome-robots.h")]
public Gtk.Window window;

[CCode (cheader_filename = "gnome-robots.h")]
public Gtk.Widget game_area;

[CCode (cheader_filename = "gnome-robots.h")]
public GLib.Settings settings;

[CCode (cheader_filename = "gnome-robots.h")]
public Games.Scores.Context highscores;

public void update_game_status (int score, int level, int safe_teleports);
public void set_move_action_sensitivity (bool state);
public string category_name_from_key (string key);

