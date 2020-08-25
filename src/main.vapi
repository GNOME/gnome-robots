public void keyboard_set (uint keys[9]);

public bool load_game_graphics ();
public void clear_game_area ();
public void set_background_color (Gdk.RGBA color);

public void start_new_game ();

[CCode (cheader_filename = "gnome-robots.h")]
public Gtk.Window window;

[CCode (cheader_filename = "gnome-robots.h")]
public GLib.Settings settings;

