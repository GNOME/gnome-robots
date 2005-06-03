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
#include <libgnomeui/gnome-window-icon.h>
#include <sys/time.h>
#include <string.h>

#include "gbdefs.h"
#include "statusbar.h"
#include "gameconfig.h"
#include "graphics.h"
#include "menu.h"
#include "sound.h"
#include "properties.h"
#include "game.h"
#include "cursors.h"

/**********************************************************************/
/* Exported Variables                                                 */
/**********************************************************************/
GtkWidget *app       = NULL;
GtkWidget *game_area = NULL;
/**********************************************************************/


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static gchar  *cmdline_scenario = NULL;
static gchar  *cmdline_config   = NULL;
static gint    session_xpos     = -1;
static gint    session_ypos     = -1;
/**********************************************************************/


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static struct poptOption options[] = {
  {"scenario", 's', POPT_ARG_STRING, &cmdline_scenario, 0, 
   N_("Set game scenario"), N_("NAME")},
  {"config", 'c', POPT_ARG_STRING, &cmdline_config, 0, 
   N_("Set game configuration"), N_("NAME")},
  {"x", 'x', POPT_ARG_INT, &session_xpos, 0, NULL, N_("X")},
  {"y", 'y', POPT_ARG_INT, &session_ypos, 0, NULL, N_("Y")},
  {NULL, '\0', 0, NULL, 0}
};
/**********************************************************************/


/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/
static gint   save_state(GnomeClient*, gint, GnomeRestartStyle, gint, GnomeInteractStyle, gint, gpointer);
static void   session_die(gpointer);
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
save_state (
            GnomeClient        *client,
            gint                phase,
            GnomeRestartStyle   save_style,
            gint                shutdown,
            GnomeInteractStyle  interact_style,
            gint                fast,
            gpointer            client_data)
{
  char *argv[20];
  int i;
  int xpos, ypos;

  gdk_window_get_origin (app->window, &xpos, &ypos);

  i = 0;
  argv[i++] = (char *)client_data;
  argv[i++] = "-x";
  argv[i++] = g_strdup_printf ("%d",xpos);
  argv[i++] = "-y";
  argv[i++] = g_strdup_printf ("%d",ypos);

  gnome_client_set_restart_command (client, i, argv);
  /* i.e. clone_command = restart_command - '--sm-client-id' */
  gnome_client_set_clone_command (client, 0, NULL);

  g_free (argv[2]);
  g_free (argv[4]);

  return TRUE;
}


/**
 * session_die
 * @client_data: client data
 *
 * Description:
 * cleans up on session death
 **/
static void
session_die (gpointer client_data)
{
  cleanup_game ();
  cleanup_sound ();
  
  gtk_widget_destroy (app);
  gtk_main_quit ();
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
  GtkWidget      *stbar;
  GtkWidget      *errordialog;
  GnomeClient    *client;
  struct timeval tv;
  gint           i;

  gettimeofday (&tv, NULL);
  srand (tv.tv_usec);

  gnome_score_init (GAME_NAME);

  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  gnome_program_init (GAME_NAME, VERSION,
 		      LIBGNOMEUI_MODULE,
 		      argc, argv,
 		      GNOME_PARAM_POPT_TABLE, options,
 		      GNOME_PARAM_APP_DATADIR, DATADIR, NULL);

  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-gnobots2.png");

  client = gnome_master_client ();

  g_object_ref (G_OBJECT (client));
  gtk_object_sink (GTK_OBJECT (client));

  g_signal_connect (G_OBJECT (client), "save_yourself",
                    G_CALLBACK (save_state), argv[0]);
  g_signal_connect (G_OBJECT(client), "die",
                    G_CALLBACK (session_die), argv[0]);

  initialize_gconf (argc, argv);

  app = gnome_app_new (GAME_NAME, _("Robots"));
  gtk_window_set_resizable (GTK_WINDOW (app), FALSE);

  g_signal_connect (G_OBJECT (app), "delete_event",
                    G_CALLBACK (exit_cb), NULL);

  stbar = gnobots_statusbar_new ();
  gnome_app_set_statusbar (GNOME_APP (app), stbar);

  create_game_menus ();
  make_cursors ();

  game_area = gtk_drawing_area_new ();
  gtk_widget_add_events (game_area, GDK_BUTTON_PRESS_MASK |
			 GDK_POINTER_MOTION_MASK);
  g_signal_connect (G_OBJECT (game_area), "button-press-event",
		    G_CALLBACK (mouse_cb), NULL);
  g_signal_connect (G_OBJECT (game_area), "motion-notify-event",
		    G_CALLBACK (move_cb), NULL);
  gnome_app_set_contents (GNOME_APP (app), game_area);
  gtk_widget_set_size_request (GTK_WIDGET (game_area), 
                               TILE_WIDTH * GAME_WIDTH,
                               TILE_HEIGHT * GAME_HEIGHT);
  gtk_widget_show (game_area);

  /* Set the window position if it was set by the session manager */
  if (session_xpos >= 0 && session_ypos >= 0){
    gtk_window_move (GTK_WINDOW (app), session_xpos, session_ypos);
  }

  if (!load_game_configs ()) {
    /* Oops, no configs, we probably haven't been installed properly. */
    errordialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_OK,
                                          "<b>%s</b>\n\n%s",
                                          _("No game data could be found."),
                                          _("The program Robots was unable to find any valid game configuration files. Please check that the program is installed correctly."));
    gtk_label_set_use_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (errordialog)->label), TRUE);
    gtk_window_set_resizable (GTK_WINDOW (errordialog), FALSE);
    gtk_dialog_run (GTK_DIALOG (errordialog));
    exit (1);
  }

  gtk_widget_show (app);
  
  if (!load_game_graphics ()) {
    /* Oops, no graphics, we probably haven't been installed properly. */
    errordialog = gtk_message_dialog_new (GTK_WINDOW (app), 
					  GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_OK,
                                          "<b>%s</b>\n\n%s",
                                          _("Some graphics files are missing or corrupt."),
                                          _("The program Robots was unable to load all the necessary graphics files. Please check that the program is installed correctly."));
    gtk_label_set_use_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (errordialog)->label), TRUE);

    gtk_dialog_run (GTK_DIALOG (errordialog));
    exit (1);
  }

  load_properties ();
  
  init_sound ();

  init_game ();

  if (cmdline_scenario) {
    for (i = 0; i < num_game_graphics (); ++i){
      if (! strcmp (cmdline_scenario, game_graphics_name (i))){
	set_game_graphics (i);
	break;
      }
    }
  }

  if (cmdline_config) {
    for(i = 0; i < num_game_configs (); ++i){
      if (! strcmp (cmdline_config, game_config_name (i))){
	properties_set_config (i);
	break;
      }
    }
  }

  update_score_state ();

  gtk_main ();

  return 0;
}

/**********************************************************************/
