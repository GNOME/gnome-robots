#include <config.h>
#include <gnome.h>

#include "gbdefs.h"
#include "menu.h"
#include "game.h"
#include "gnobots.h"


/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/
static void new_cb(GtkWidget *widget,gpointer data);
static void properties_cb(GtkWidget *widget,gpointer data);
static void scores_cb(GtkWidget *widget,gpointer data);
void exit_cb(GtkWidget *widget,gpointer  data);
static void about_cb(GtkWidget *widget, gpointer data);
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
  GNOMEUIINFO_MENU_SETTINGS_TREE(prefmenu),
  GNOMEUIINFO_MENU_HELP_TREE(helpmenu),
  GNOMEUIINFO_END
};
/**********************************************************************/

/**********************************************************************/


/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

/**
 * really_new_cb
 * @widget: Pointer to widget
 * @data: Callback data
 *
 * Description:
 * Callback to really start new game
 *
 * Returns:
 **/
static void really_new_cb(
GtkWidget *widget,
gpointer  data
){
  gint button = GPOINTER_TO_INT(data);
    
  if(button != 0) return;

  start_new_game();
}


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
static void new_cb(
GtkWidget *widget,
gpointer  data
){

  if(game_state != STATE_NOT_PLAYING){
    gnome_app_ok_cancel_modal (GNOME_APP(app),
                               _("Applying this change to Player Selection\nwill end the current game"),
                               (GnomeReplyCallback)really_new_cb,
                               NULL);
  } else {
    really_new_cb(widget, (gpointer)0);
  }
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
static void properties_cb(
GtkWidget *widget,
gpointer  data
){
  show_properties_dialog();
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
static void scores_cb(
GtkWidget *widget,
gpointer  data
){
  show_scores(0);
}


/**
 * really_exit_cb
 * @widget: Pointer to widget
 * @data: Callback data
 *
 * Description:
 * Callback to really exit game
 *
 * Returns:
 **/
void really_exit_cb(
GtkWidget *widget,
gpointer  data
){
  gint button = GPOINTER_TO_INT(data);
    
  if(button != 0) return;

  gtk_main_quit();
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
void exit_cb(
GtkWidget *widget,
gpointer  data
){

  if(game_state != STATE_NOT_PLAYING){
    gnome_app_ok_cancel_modal (GNOME_APP(app),
                               _("Do you really want to quit the game?"),
                               (GnomeReplyCallback)really_exit_cb,
                               NULL);
  } else {
    really_exit_cb(widget, (gpointer)0);
  }
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
static void about_cb(
GtkWidget *widget, 
gpointer data
){
  GtkWidget *about = NULL;
  GdkPixbuf *pixbuf = NULL;
  
  const gchar *authors[]= {
    "Mark Rae <m.rae@inpharmatica.co.uk>",
    NULL
  };
  gchar *documenters[] = {
                	  NULL
        		  };
  /* Translator credits */
  gchar *translator_credits = _("translator_credits");

  {
	  char *filename = NULL;

	  filename = gnome_program_locate_file (NULL,
			  GNOME_FILE_DOMAIN_APP_PIXMAP,  ("gnome-gnobots2.png"),
			  TRUE, NULL);
	  if (filename != NULL)
	  {
		  pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
		  g_free (filename);
	  }
  }
  
  about = gnome_about_new(_("Gnobots II"), VERSION,
                          "(C) 1998 Mark Rae",
                          _("Gnome Robots Game"),
                          (const char **)authors,
                          (const char **)documenters,
                          strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
                          pixbuf);
  gtk_window_set_transient_for (GTK_WINDOW (about), GTK_WINDOW(app));
  
  gtk_widget_show(about);
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
gboolean create_game_menus(
){
  gnome_app_create_menus(GNOME_APP(app), mainmenu);
  gnome_app_install_menu_hints(GNOME_APP(app), mainmenu);

  return TRUE;
}

/**********************************************************************/
