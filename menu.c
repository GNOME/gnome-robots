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
#include <string.h>

#include "gbdefs.h"
#include "menu.h"
#include "game.h"
#include "gnobots.h"
#include "properties.h"
#include "gameconfig.h"

/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/
static void new_cb (GtkWidget *widget,gpointer data);
static void properties_cb (GtkWidget *widget,gpointer data);
static void scores_cb (GtkWidget *widget,gpointer data);
void exit_cb (GtkWidget *widget,gpointer  data);
static void about_cb (GtkWidget *widget, gpointer data);
static void teleport_cb  (GtkWidget *widget, gpointer data);
static void randteleport_cb  (GtkWidget *widget, gpointer data);
static void wait_cb (GtkWidget *widget, gpointer data);
/**********************************************************************/


/**********************************************************************/
/* Menu entries                                                       */
/**********************************************************************/

/**********************************************************************/
/* Game menu entries                                                  */
/**********************************************************************/
GnomeUIInfo gamemenu[] = {
  GNOMEUIINFO_MENU_NEW_GAME_ITEM(new_cb, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_SCORES_ITEM(scores_cb, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_EXIT_ITEM(exit_cb, NULL),
  GNOMEUIINFO_END
};
/**********************************************************************/

GnomeUIInfo movemenu[] = {
  GNOMEUIINFO_ITEM_STOCK (N_("Teleport"), N_("Teleport, safely if possible"), teleport_cb, GTK_STOCK_JUMP_TO),
  GNOMEUIINFO_ITEM_STOCK (N_("Random"), N_("Teleport randomly"), randteleport_cb, GTK_STOCK_JUMP_TO),
  GNOMEUIINFO_ITEM_STOCK (N_("Wait"), N_("Wait for the robots"), wait_cb, GTK_STOCK_STOP),
  GNOMEUIINFO_END
};


/**********************************************************************/
/* Preferences menu entries                                           */
/**********************************************************************/
GnomeUIInfo prefmenu[] = {
  GNOMEUIINFO_MENU_PREFERENCES_ITEM(properties_cb, NULL),
  GNOMEUIINFO_END
};
/**********************************************************************/


/**********************************************************************/
/* Help menu entries                                                  */
/**********************************************************************/
GnomeUIInfo helpmenu[] = {
  GNOMEUIINFO_HELP(GAME_NAME),
  GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb, NULL),
  GNOMEUIINFO_END
};
/**********************************************************************/


/**********************************************************************/
/* Main menu                                                          */
/**********************************************************************/
GnomeUIInfo mainmenu[] = {
  GNOMEUIINFO_MENU_GAME_TREE(gamemenu),
  GNOMEUIINFO_SUBTREE (N_("_Move"), movemenu),
  GNOMEUIINFO_MENU_SETTINGS_TREE(prefmenu),
  GNOMEUIINFO_MENU_HELP_TREE(helpmenu),
  GNOMEUIINFO_END
};
/**********************************************************************/

GnomeUIInfo toolbar[] = {
  GNOMEUIINFO_ITEM_STOCK(N_("New"), N_("Start a new game"),
                         new_cb, GTK_STOCK_NEW),
  GNOMEUIINFO_SEPARATOR,

  /* FIXME: Someday these should be part of a gnome-games icon theme. */

  { GNOME_APP_UI_ITEM, N_("Teleport"), N_("Teleport, safely if possible"), 
    teleport_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME, "teleport.png",
    0, 0, NULL},

  { GNOME_APP_UI_ITEM, N_("Random"), N_("Teleport randomly"), 
    randteleport_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME, "rteleport.png",
    0, 0, NULL},

  GNOMEUIINFO_ITEM_STOCK (N_("Wait"), N_("Wait for the robots"), wait_cb, GTK_STOCK_STOP),
  GNOMEUIINFO_END
};

/**********************************************************************/


/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

/**
 * new_cb
 * @widget: Pointer to widget
 * @data: Callback data
 *
 * Description:
 * Callback for new menu entry
 *
 * Returns:
 **/
static void
new_cb (GtkWidget *widget, gpointer  data)
{
    start_new_game ();
}


/**
 * properties_cb
 * @widget: Pointer to widget
 * @data: Callback data
 *
 * Description:
 * Callback for properties menu entry
 *
 * Returns:
 **/
static void
properties_cb (GtkWidget *widget, gpointer  data)
{
  show_properties_dialog ();
}


/**
 * scores_cb
 * @widget: Pointer to widget
 * @data: Callback data
 *
 * Description:
 * Callback for scores menu entry
 *
 * Returns:
 **/
static void
scores_cb (GtkWidget *widget, gpointer data)
{
  show_scores (0);
}


/**
 * exit_cb
 * @widget: Pointer to widget
 * @data: Callback data
 *
 * Description:
 * Callback for exit menu entry
 *
 * Returns:
 **/
void
exit_cb (GtkWidget *widget, gpointer  data)
{
  cleanup_game ();
  gtk_main_quit ();
}


/**
 * about_cb
 * @widget: Pointer to widget
 * @data: Callback data
 *
 * Description:
 * Callback for about menu entry
 *
 * Returns:
 **/
static void
about_cb (GtkWidget *widget, gpointer data)
{
  static GtkWidget *about = NULL;
  GdkPixbuf *pixbuf = NULL;
  
  const gchar *authors[]= {
    "Mark Rae <m.rae@inpharmatica.co.uk>",
    NULL
  };
  gchar *documenters[] = {
                	  NULL
        		  };
  /* Translator credits */
  gchar *translator_credits = _("translator-credits");

  if (about != NULL) {
    gtk_window_present (GTK_WINDOW (about));
    return;
  }
  {
    char *filename = NULL;
    
    filename = gnome_program_locate_file (NULL,
                                          GNOME_FILE_DOMAIN_APP_PIXMAP,
                                          ("gnome-gnobots2.png"),
                                          TRUE, NULL);
    if (filename != NULL) {
      pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
      g_free (filename);
    }
  }
  
  about = gnome_about_new(_("GNOME Robots"), VERSION,
                          "Copyright \xc2\xa9 1998-2004 Mark Rae",
                          _("GNOME Robots Game"),
                          (const char **)authors,
                          (const char **)documenters,
                          strcmp (translator_credits, "translator-credits") != 0 ? translator_credits : NULL,
                          pixbuf);
  if (pixbuf != NULL)
    gdk_pixbuf_unref (pixbuf);
	
  gtk_window_set_transient_for (GTK_WINDOW (about), GTK_WINDOW (app));
  g_signal_connect (G_OBJECT (about), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &about);

  gtk_widget_show (about);
}

static void teleport_cb  (GtkWidget *widget, gpointer data)
{
  game_keypress (KBD_TELE);
}

static void randteleport_cb  (GtkWidget *widget, gpointer data)
{
  game_keypress (KBD_RTEL);
}

static void wait_cb (GtkWidget *widget, gpointer data)
{
  game_keypress (KBD_WAIT);
}


/**
 * create_game_menus
 * @ap: application pointer
 *
 * Description:
 * Creates the menus for application @ap
 *
 * Returns:
 * %TRUE if successful, %FALSE otherwise
 **/
gboolean
create_game_menus (void)
{
  gnome_app_create_menus (GNOME_APP (app), mainmenu);
  gnome_app_install_menu_hints (GNOME_APP (app), mainmenu);
  gnome_app_create_toolbar (GNOME_APP (app), toolbar);

  return TRUE;
}

/**********************************************************************/

/**
 * update_score_state
 *
 * Description:
 * Changes menu item enabled/disabled state depending on high score availability
 **/
void
update_score_state (void)
{
  gchar **names = NULL;
  gfloat *scores = NULL;
  time_t *scoretimes = NULL;
  gint top;
  gchar *sbuf = NULL;
  
  if (properties_super_safe_moves ()) {
    sbuf = g_strdup_printf ("%s-super-safe",
                            game_config_filename (current_game_config ()));
  } else if (properties_safe_moves ()) {
    sbuf = g_strdup_printf ("%s-safe", 
                            game_config_filename (current_game_config ()));
  } else {
    sbuf = g_strdup_printf ("%s",
                            game_config_filename (current_game_config ()));
  }
  
  top = gnome_score_get_notable (GAME_NAME, sbuf,
                                 &names, &scores, &scoretimes);
  g_free (sbuf);
  if (top > 0) {
    gtk_widget_set_sensitive (gamemenu[2].widget, TRUE);
    g_strfreev (names);
    g_free (scores);
    g_free (scoretimes);
  } else {
    gtk_widget_set_sensitive (gamemenu[2].widget, FALSE);
  }
}

void set_move_menu_sensitivity (gboolean state)
{
  int i;

  for (i=0; i<3; i++) {
    gtk_widget_set_sensitive (movemenu[i].widget, state);
    gtk_widget_set_sensitive (toolbar[i+2].widget, state);
  }
}
