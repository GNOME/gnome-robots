
/*
 * Gnome Robots II
 * written by Mark Rae <m.rae@inpharmatica.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */

#include <config.h>

#include "gnome-robots.h"

#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#include <glib/gi18n.h>
#include <glib.h>
#include <libgames-support.h>

#include "gbdefs.h"
#include "gameconfig.h"
#include "graphics.h"
#include "sound.h"
#include "properties.h"
#include "game.h"
#include "cursors.h"

/* Minimum sizes. */
#define MINIMUM_TILE_WIDTH   8
#define MINIMUM_TILE_HEIGHT  MINIMUM_TILE_WIDTH

#define KEY_GEOMETRY_GROUP "geometry"

/**********************************************************************/
/* Utility structs                                                    */
/**********************************************************************/
typedef struct
{
    gchar* key;
    gchar* name;
} key_value;
/**********************************************************************/

/**********************************************************************/
/* Exported Variables                                                 */
/**********************************************************************/
GtkWidget *window = NULL;
static gint window_width = 0, window_height = 0;
static gboolean window_is_maximized = FALSE;
GtkWidget *game_area = NULL;
GamesScoresContext *highscores;
GSettings *settings;
/**********************************************************************/

/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/
/**********************************************************************/

static void preferences_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void scores_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void help_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void about_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void quit_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void new_game_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void random_teleport_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void safe_teleport_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void wait_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data);

/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/

static const GActionEntry app_entries[] = {
  { "new-game", new_game_cb, NULL, NULL, NULL },
  { "preferences", preferences_cb, NULL, NULL, NULL },
  { "scores", scores_cb, NULL, NULL, NULL },
  { "help", help_cb, NULL, NULL, NULL },
  { "about", about_cb, NULL, NULL, NULL },
  { "quit", quit_cb, NULL, NULL, NULL },
};

static const GActionEntry win_entries[] = {
  { "random-teleport", random_teleport_cb, NULL, NULL, NULL },
  { "safe-teleport", safe_teleport_cb, NULL, NULL, NULL },
  { "wait", wait_cb, NULL, NULL, NULL },
};

static const key_value scorecats[] = {
	{"classic_robots", N_("Classic robots")},
	{"classic_robots-safe", N_("Classic robots with safe moves")},
	{"classic_robots-super-safe", N_("Classic robots with super-safe moves")},
	{"nightmare", N_("Nightmare")},
	{"nightmare-safe", N_("Nightmare with safe moves")},
	{"nightmare-super-safe", N_("Nightmare with super-safe moves")},
	{"robots2", N_("Robots2")},
	{"robots2-safe", N_("Robots2 with safe moves")},
	{"robots2-super-safe", N_("Robots2 with super-safe moves")},
	{"robots2_easy", N_("Robots2 easy")},
	{"robots2_easy-safe", N_("Robots2 easy with safe moves")},
	{"robots2_easy-super-safe", N_("Robots2 easy with super-safe moves")},
	{"robots_with_safe_teleport", N_("Robots with safe teleport")},
	{"robots_with_safe_teleport-safe", N_("Robots with safe teleport with safe moves")},
	{"robots_with_safe_teleport-super-safe", N_("Robots with safe teleport with super-safe moves")}
};
static gint no_categories = 15;

static gint safe_teleports = 0;
static GtkWidget *safe_teleports_label;
static GtkWidget *headerbar;

/**********************************************************************/

/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

void
set_move_action_sensitivity (gboolean state)
{
  GAction *action;

  action = g_action_map_lookup_action (G_ACTION_MAP (window), "random-teleport");
  g_simple_action_set_enabled (G_SIMPLE_ACTION (action), state);

  action = g_action_map_lookup_action (G_ACTION_MAP (window), "safe-teleport");
  g_simple_action_set_enabled (G_SIMPLE_ACTION (action), state && safe_teleports > 0);

  action = g_action_map_lookup_action (G_ACTION_MAP (window), "wait");
  g_simple_action_set_enabled (G_SIMPLE_ACTION (action), state);
}

void
update_game_status (gint score, gint current_level, gint safes)
{
  GAction *action;
  gchar *subtitle, *remaining_teleports_text, *button_text;

  /* Window subtitle. The first %d is the level, the second is the score. \t creates a tab. */
  subtitle = g_strdup_printf (_("Level: %d\tScore: %d"), current_level, score);
  gtk_header_bar_set_subtitle (GTK_HEADER_BAR (headerbar), subtitle);
  g_free (subtitle);

  safe_teleports = safes;

  action = g_action_map_lookup_action (G_ACTION_MAP (window), "safe-teleport");
  if (g_action_get_enabled (action))
    g_simple_action_set_enabled (G_SIMPLE_ACTION (action), safe_teleports > 0);

  /* Second line of safe teleports button label. %d is the number of teleports remaining. */
  remaining_teleports_text = g_strdup_printf (_("(Remaining: %d)"), safe_teleports);
  /* First line of safe teleports button label. */
  button_text = g_strdup_printf ("%s\n<small>%s</small>", _("Teleport _Safely"), remaining_teleports_text);
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (safe_teleports_label), button_text);

  g_free (remaining_teleports_text);
  g_free (button_text);
}

const gchar *
category_name_from_key (const gchar* key)
{
  int i;
  for (i = 0; i < no_categories; i++)
  {
    if (g_strcmp0 (scorecats[i].key, key) == 0)
      return scorecats[i].name;
  }
  /*Return key as is if match not found*/
  return NULL;
}

static void
preferences_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  show_properties_dialog ();
}

static void
scores_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  show_scores ();
}

static void
help_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  GError *error = NULL;

  gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (window)), "help:gnome-robots", gtk_get_current_event_time (), &error);
  if (error)
    g_warning ("Failed to show help: %s", error->message);
  g_clear_error (&error);
}

static void
about_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  const gchar *authors[] = { "Mark Rae <m.rae@inpharmatica.co.uk>", "Rasoul M.P. Aghdam (player death sound)", NULL };

  const gchar *artists[] = { "Kirstie Opstad <K.Opstad@ed.ac.uk>", NULL };

  const gchar *documenters[] =
    { "Aruna Sankaranarayanan", NULL };

  gtk_show_about_dialog (GTK_WINDOW (window),
			 "name", _("Robots"),
			 "version", VERSION,
			 "copyright", "Copyright © 1998–2008 Mark Rae\nCopyright © 2014 Michael Catanzaro",
			 "license-type", GTK_LICENSE_GPL_2_0,
			 "comments", _("Based on classic BSD Robots"),
			 "authors", authors,
			 "artists", artists,
			 "documenters", documenters,
			 "translator-credits", _("translator-credits"),
			 "logo-icon-name", "gnome-robots",
			 "website",
			 "https://wiki.gnome.org/Apps/Robots",
			 NULL);
}

static void
quit_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  quit_game ();
}

static void
new_game_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  GtkWidget *dialog;
  int ret;

  dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                   _("Are you sure you want to discard the current game?"));

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          _("Keep _Playing"), GTK_RESPONSE_REJECT,
                          _("_New Game"), GTK_RESPONSE_ACCEPT,
                          NULL);

  ret = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  if (ret == GTK_RESPONSE_ACCEPT)
    start_new_game ();
}

static void
random_teleport_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  game_keypress (KBD_RTEL);
}

static void
safe_teleport_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  game_keypress (KBD_TELE);
}

static void
wait_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  game_keypress (KBD_WAIT);
}

static gboolean
window_configure_event_cb (GtkWidget *widget, GdkEventConfigure *event)
{
  if (!window_is_maximized)
  {
    window_width = event->width;
    window_height = event->height;
  }

  return FALSE;
}

static gboolean
window_state_event_cb (GtkWidget *widget, GdkEventWindowState *event)
{
  if ((event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED) != 0)
    window_is_maximized = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
  return FALSE;
}

void
quit_game (void)
{
  gtk_window_close (GTK_WINDOW (window));
}

static void
startup (GtkApplication *app, gpointer user_data)
{
  struct timeval tv;

  gettimeofday (&tv, NULL);
  srand (tv.tv_usec);

  g_set_application_name (_("Robots"));

  settings = g_settings_new ("org.gnome.robots");

  gtk_window_set_default_icon_name ("gnome-robots");
}

static void
shutdown (GtkApplication *app, gpointer user_data)
{
  g_settings_set_int (settings, "window-width", window_width);
  g_settings_set_int (settings, "window-height", window_height);
  g_settings_set_boolean (settings, "window-is-maximized", window_is_maximized);
}

static GamesScoresCategory
*create_category_from_key (GamesScoresContext *context, const char *key, gpointer user_data)
{
  const gchar *name = category_name_from_key (key);
  if (name == NULL)
    return NULL;
  return games_scores_category_new (key, name);
}

static void
activate (GtkApplication *app, gpointer user_data)
{
  GtkWidget *errordialog, *vbox, *hbox, *label, *button, *gridframe;
  GtkSizeGroup *size_group;
  GtkStyleContext *style_context;

  headerbar = gtk_header_bar_new ();
  gtk_header_bar_set_title (GTK_HEADER_BAR (headerbar), _("Robots"));
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (headerbar), TRUE);

  window = gtk_application_window_new (app);
  gtk_window_set_titlebar (GTK_WINDOW (window), headerbar);
  g_signal_connect (GTK_WINDOW (window), "configure-event", G_CALLBACK (window_configure_event_cb), NULL);
  g_signal_connect (GTK_WINDOW (window), "window-state-event", G_CALLBACK (window_state_event_cb), NULL);
  gtk_window_set_default_size (GTK_WINDOW (window), g_settings_get_int (settings, "window-width"), g_settings_get_int (settings, "window-height"));
  if (g_settings_get_boolean (settings, "window-is-maximized"))
    gtk_window_maximize (GTK_WINDOW (window));

  g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app);
  g_action_map_add_action_entries (G_ACTION_MAP (window), win_entries, G_N_ELEMENTS (win_entries), app);

  make_cursors ();

  game_area = gtk_drawing_area_new ();
  gtk_widget_add_events (game_area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			 GDK_POINTER_MOTION_MASK);
  g_signal_connect (G_OBJECT (game_area), "button-press-event",
		    G_CALLBACK (mouse_cb), NULL);
  g_signal_connect (G_OBJECT (game_area), "motion-notify-event",
		    G_CALLBACK (move_cb), NULL);
  g_signal_connect (G_OBJECT (game_area), "configure-event",
		    G_CALLBACK (resize_cb), NULL);
  g_signal_connect (G_OBJECT (game_area), "draw",
		    G_CALLBACK (draw_cb), NULL);
  g_signal_connect (G_OBJECT (game_area), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &game_area);

  gridframe = games_grid_frame_new (GAME_WIDTH, GAME_HEIGHT);
  gtk_container_add (GTK_CONTAINER (gridframe), game_area);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  size_group = gtk_size_group_new (GTK_SIZE_GROUP_BOTH);

  style_context = gtk_widget_get_style_context (hbox);
  gtk_style_context_add_class (style_context, "linked");

  label = gtk_label_new_with_mnemonic (_("Teleport _Randomly"));
  gtk_widget_set_margin_top (label, 15);
  gtk_widget_set_margin_bottom (label, 15);
  button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_actionable_set_action_name (GTK_ACTIONABLE (button), "win.random-teleport");
  gtk_size_group_add_widget (size_group, button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  safe_teleports_label = gtk_label_new (NULL);
  gtk_label_set_justify (GTK_LABEL (safe_teleports_label), GTK_JUSTIFY_CENTER);
  gtk_widget_set_margin_top (label, 15);
  gtk_widget_set_margin_bottom (label, 15);
  button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER (button), safe_teleports_label);
  gtk_actionable_set_action_name (GTK_ACTIONABLE (button), "win.safe-teleport");
  gtk_size_group_add_widget (size_group, button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  label = gtk_label_new_with_mnemonic (_("_Wait for Robots"));
  gtk_widget_set_margin_top (label, 15);
  gtk_widget_set_margin_bottom (label, 15);
  button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_actionable_set_action_name (GTK_ACTIONABLE (button), "win.wait");
  gtk_size_group_add_widget (size_group, button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_object_unref (size_group);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (vbox), gridframe, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (window), vbox);

  gtk_widget_set_size_request (GTK_WIDGET (game_area),
			       MINIMUM_TILE_WIDTH * GAME_WIDTH,
			       MINIMUM_TILE_HEIGHT * GAME_HEIGHT);

  highscores = games_scores_context_new ("gnome-robots",
                                         /* Label on the scores dialog, next to map type dropdown */
                                         _("Game Type:"),
                                         GTK_WINDOW (window),
                                         GAMES_SCORES_STYLE_PLAIN_DESCENDING);
  g_signal_connect (highscores, "category-request", G_CALLBACK (create_category_from_key), NULL);
  gtk_widget_show_all (window);

  if (!load_game_configs ()) {
    /* Oops, no configs, we probably haven't been installed properly. */
    errordialog = gtk_message_dialog_new_with_markup (NULL, 0, GTK_MESSAGE_ERROR,
					  GTK_BUTTONS_OK,
					  "<b>%s</b>\n\n%s",
					  _("No game data could be found."),
					  _
					  ("The program Robots was unable to find any valid game configuration files. Please check that the program is installed correctly."));
    gtk_window_set_resizable (GTK_WINDOW (errordialog), FALSE);

    gtk_dialog_run (GTK_DIALOG (errordialog));
    exit (1);
  }

  load_properties ();

  if (!load_game_graphics ()) {
    /* Oops, no graphics, we probably haven't been installed properly. */
    errordialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (window),
					  GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,
					  GTK_BUTTONS_OK,
					  "<b>%s</b>\n\n%s",
					  _
					  ("Some graphics files are missing or corrupt."),
					  _
					  ("The program Robots was unable to load all the necessary graphics files. Please check that the program is installed correctly."));
    gtk_dialog_run (GTK_DIALOG (errordialog));
    exit (1);
  }

  init_sound ();

  init_game ();

  g_settings_sync ();
}

/**
 * main
 * @argc: number of arguments
 * @argv: arguments
 *
 * Description:
 * main
 *
 * Returns:
 * exit code
 **/
int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  GtkApplication *app;

  app = gtk_application_new ("org.gnome.robots", G_APPLICATION_FLAGS_NONE);

  g_signal_connect (app, "startup", G_CALLBACK (startup), NULL);
  g_signal_connect (app, "shutdown", G_CALLBACK (shutdown), NULL);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}

/**********************************************************************/
