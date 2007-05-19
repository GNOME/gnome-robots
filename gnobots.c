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
#include <gnome.h>
#include <sys/time.h>
#include <string.h>
#include <games-stock.h>
#include <games-scores.h>
#include <games-scores-dialog.h>
#include <games-sound.h>
#include <games-gridframe.h>

#include "gbdefs.h"
#include "statusbar.h"
#include "gameconfig.h"
#include "graphics.h"
#include "menu.h"
#include "sound.h"
#include "properties.h"
#include "game.h"
#include "cursors.h"

/* Minimum sizes. */
#define MINIMUM_TILE_WIDTH   8
#define MINIMUM_TILE_HEIGHT  MINIMUM_TILE_WIDTH

/**********************************************************************/
/* Exported Variables                                                 */
/**********************************************************************/
GtkWidget *app = NULL;
GtkWidget *game_area = NULL;
GamesScores *highscores;
/**********************************************************************/


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static gchar *cmdline_scenario = NULL;
static gchar *cmdline_config = NULL;
static gint session_xpos = -1;
static gint session_ypos = -1;
/**********************************************************************/


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static const GOptionEntry options[] = {
  {"scenario", 's', 0, G_OPTION_ARG_STRING, &cmdline_scenario,
   N_("Set game scenario"), N_("NAME")},
  {"config", 'c', 0, G_OPTION_ARG_STRING, &cmdline_config,
   N_("Set game configuration"), N_("NAME")},
  {"x", 'x', 0, G_OPTION_ARG_INT, &session_xpos,
   N_("Initial window position"), N_("X")},
  {"y", 'y', 0, G_OPTION_ARG_INT, &session_ypos,
   N_("Initial window position"), N_("Y")},
  {NULL},
};

static const GamesScoresCategory scorecats[] = { 
  {"classic_robots", N_("Classic robots")},
  {"classic_robots-safe", N_("Classic robots with safe moves")},
  {"classic_robots-super-safe", N_("Classic robots with super-safe moves")},
  {"nightmare", N_("Nightmare")},
  {"nightmare-safe", N_("Nightmare with safe moves")},
  {"nightmare.-super-safe", N_("Nightmare with super-safe moves")},
  {"robots2", N_("Robots2")},
  {"robots2-safe", N_("Robots2 with safe moves")},
  {"robots2-super-safe", N_("Robots2 with super-safe moves")},
  {"robots2_easy", N_("Robots2 easy")},
  {"robots2_easy-safe", N_("Robots2 easy with safe moves")},
  {"robots2_easy-super-safe", N_("Robots2 easy with super-safe moves")},
  {"robots_with_safe_teleport", N_("Robots with safe teleport")},
  {"robots_with_safe_teleport-safe", N_("Robots with safe teleport with safe moves")},
  {"robots_with_safe_teleport-super-safe", N_("Robots with safe teleport with super-safe moves")},
  GAMES_SCORES_LAST_CATEGORY
};

static const GamesScoresDescription scoredesc = { scorecats,
  "classic_robots",
  "gnobots2",
  GAMES_SCORES_STYLE_PLAIN_DESCENDING
};

/**********************************************************************/


/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/
static gint save_state (GnomeClient *, gint, GnomeRestartStyle, gint,
			GnomeInteractStyle, gint, gpointer);
/**********************************************************************/


/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

/**
 * save_state
 * @client: gnome client
 * @phase: phase
 * @save_style: save style
 * @shutdown: shutdown
 * @interact_style: interact style
 * @fast: fast shutdown
 * @client_data: client data
 *
 * Description:
 * saves session info
 *
 * Returns:
 * TRUE on success, FALSE otherwise
 **/
static gint
save_state (GnomeClient * client,
	    gint phase,
	    GnomeRestartStyle save_style,
	    gint shutdown,
	    GnomeInteractStyle interact_style,
	    gint fast, gpointer client_data)
{
  char *argv[20];
  int i;
  int xpos, ypos;

  gdk_window_get_origin (app->window, &xpos, &ypos);

  i = 0;
  argv[i++] = (char *) client_data;
  argv[i++] = "-x";
  argv[i++] = g_strdup_printf ("%d", xpos);
  argv[i++] = "-y";
  argv[i++] = g_strdup_printf ("%d", ypos);

  gnome_client_set_restart_command (client, i, argv);
  /* i.e. clone_command = restart_command - '--sm-client-id' */
  gnome_client_set_clone_command (client, 0, NULL);

  g_free (argv[2]);
  g_free (argv[4]);

  return TRUE;
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
  GtkWidget *errordialog;
  GtkWidget *vbox, *menubar, *toolbar, *statusbar, *gridframe;
  GtkUIManager *ui_manager;
  GnomeClient *client;
  GnomeProgram *program;
  GOptionContext *option_context;
  struct timeval tv;
  gint i;

  gettimeofday (&tv, NULL);
  srand (tv.tv_usec);

  setgid_io_init ();

  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
 
  g_thread_init (NULL);

  option_context = g_option_context_new ("");
  g_option_context_add_main_entries (option_context, options,
				     GETTEXT_PACKAGE);
  g_option_context_add_group (option_context, games_sound_get_option_group ());

  program = gnome_program_init (GAME_NAME, VERSION,
				LIBGNOMEUI_MODULE,
				argc, argv,
				GNOME_PARAM_GOPTION_CONTEXT, option_context,
				GNOME_PARAM_APP_DATADIR, DATADIR,
				GNOME_PARAM_NONE);

  highscores = games_scores_new (&scoredesc);

  gtk_window_set_default_icon_name ("gnome-robots");

  client = gnome_master_client ();

  g_object_ref (G_OBJECT (client));
  gtk_object_sink (GTK_OBJECT (client));

  g_signal_connect (G_OBJECT (client), "save_yourself",
		    G_CALLBACK (save_state), argv[0]);
  g_signal_connect (G_OBJECT (client), "die",
		    G_CALLBACK (quit_game), argv[0]);

  initialize_gconf (argc, argv);

  app = gnome_app_new (GAME_NAME, _("Robots"));

  g_signal_connect (G_OBJECT (app), "delete_event",
		    G_CALLBACK (quit_game), NULL);
  g_signal_connect (G_OBJECT (app), "configure_event",
		    G_CALLBACK (save_window_geometry), NULL);
  g_signal_connect (G_OBJECT (app), "window_state_event",
		    G_CALLBACK (window_state_cb), NULL);


  set_window_geometry (app);

  statusbar = gnobots_statusbar_new ();
  ui_manager = gtk_ui_manager_new ();

  games_stock_prepare_for_statusbar_tooltips (ui_manager, statusbar);
  create_game_menus (ui_manager);
  gtk_window_add_accel_group (GTK_WINDOW (app),
			      gtk_ui_manager_get_accel_group (ui_manager));

  menubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
  toolbar = gtk_ui_manager_get_widget (ui_manager, "/Toolbar");

  make_cursors ();

  game_area = gtk_drawing_area_new ();
  gtk_widget_add_events (game_area, GDK_BUTTON_PRESS_MASK |
			 GDK_POINTER_MOTION_MASK);
  g_signal_connect (G_OBJECT (game_area), "button-press-event",
		    G_CALLBACK (mouse_cb), NULL);
  g_signal_connect (G_OBJECT (game_area), "motion-notify-event",
		    G_CALLBACK (move_cb), NULL);
  g_signal_connect (G_OBJECT (game_area), "configure-event",
		    G_CALLBACK (resize_cb), NULL);
  g_signal_connect (G_OBJECT (game_area), "expose-event",
		    G_CALLBACK (expose_cb), NULL);

  gridframe = games_grid_frame_new (GAME_WIDTH, GAME_HEIGHT);
  gtk_container_add (GTK_CONTAINER (gridframe), game_area);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), gridframe, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), statusbar, FALSE, FALSE, 0);

  gnome_app_set_contents (GNOME_APP (app), vbox);

  gtk_widget_set_size_request (GTK_WIDGET (game_area),
			       MINIMUM_TILE_WIDTH * GAME_WIDTH,
			       MINIMUM_TILE_HEIGHT * GAME_HEIGHT);

  /* Set the window position if it was set by the session manager */
  if (session_xpos >= 0 && session_ypos >= 0) {
    gtk_window_move (GTK_WINDOW (app), session_xpos, session_ypos);
  }

  gtk_widget_show_all (app);

  if (!load_game_configs ()) {
    /* Oops, no configs, we probably haven't been installed properly. */
    errordialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR,
					  GTK_BUTTONS_OK,
					  "<b>%s</b>\n\n%s",
					  _("No game data could be found."),
					  _
					  ("The program Robots was unable to find any valid game configuration files. Please check that the program is installed correctly."));
    gtk_label_set_use_markup (GTK_LABEL
			      (GTK_MESSAGE_DIALOG (errordialog)->label),
			      TRUE);
    gtk_window_set_resizable (GTK_WINDOW (errordialog), FALSE);
    gtk_dialog_run (GTK_DIALOG (errordialog));
    exit (1);
  }

  load_properties ();

  if (!load_game_graphics ()) {
    /* Oops, no graphics, we probably haven't been installed properly. */
    errordialog = gtk_message_dialog_new (GTK_WINDOW (app),
					  GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,
					  GTK_BUTTONS_OK,
					  "<b>%s</b>\n\n%s",
					  _
					  ("Some graphics files are missing or corrupt."),
					  _
					  ("The program Robots was unable to load all the necessary graphics files. Please check that the program is installed correctly."));
    gtk_label_set_use_markup (GTK_LABEL
			      (GTK_MESSAGE_DIALOG (errordialog)->label),
			      TRUE);

    gtk_dialog_run (GTK_DIALOG (errordialog));
    exit (1);
  }

  connect_toolbar_toggle (toolbar);

  init_sound ();

  init_game ();

  if (cmdline_scenario) {
    set_game_graphics (cmdline_scenario);
  }

  if (cmdline_config) {
    for (i = 0; i < num_game_configs (); ++i) {
      if (!strcmp (cmdline_config, game_config_name (i))) {
	properties_set_config (i);
	break;
      }
    }
  }

  update_score_state ();

  gtk_main ();

  gnome_accelerators_sync ();

  g_object_unref (program);

  return 0;
}

/**********************************************************************/
