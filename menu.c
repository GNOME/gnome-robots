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
static void exit_cb(GtkWidget *widget,gpointer  data);
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
  GNOMEUIINFO_SEPARATOR,
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
  GtkWidget *box;

  if(game_state != STATE_NOT_PLAYING){
    box = gnome_message_box_new(_("Do you really want to start a new game?"),
				GNOME_MESSAGE_BOX_QUESTION,
				GNOME_STOCK_BUTTON_YES,
				GNOME_STOCK_BUTTON_NO,
				NULL);
    gnome_dialog_set_parent (GNOME_DIALOG(box), GTK_WINDOW(app));
    gnome_dialog_set_default (GNOME_DIALOG(box), 0);
    gtk_window_set_modal (GTK_WINDOW(box), TRUE);
    gtk_signal_connect (GTK_OBJECT(box), "clicked",
			GTK_SIGNAL_FUNC(really_new_cb), NULL);
    gtk_widget_show(box);
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
static void really_exit_cb(
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
static void exit_cb(
GtkWidget *widget,
gpointer  data
){
  GtkWidget *box;

  if(game_state != STATE_NOT_PLAYING){
    box = gnome_message_box_new(_("Do you really want to quit the game?"),
				GNOME_MESSAGE_BOX_QUESTION,
				GNOME_STOCK_BUTTON_YES,
				GNOME_STOCK_BUTTON_NO,
				NULL);
    gnome_dialog_set_parent(GNOME_DIALOG(box), GTK_WINDOW(app));
    gnome_dialog_set_default(GNOME_DIALOG(box), 0);
    gtk_window_set_modal(GTK_WINDOW(box), TRUE);
    gtk_signal_connect(GTK_OBJECT(box), "clicked",
		       GTK_SIGNAL_FUNC(really_exit_cb), NULL);
    gtk_widget_show(box);
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
  
  const gchar *authors[]= {
    "Mark Rae <m.rae@inpharmatica.co.uk>",
    NULL
  };
  
  about = gnome_about_new(_("Gnobots II"), VERSION,
                          "(C) 1998 Mark Rae",
                          authors,
                          _("Gnome Robots Game"),
                          NULL);
  gnome_dialog_set_parent(GNOME_DIALOG(about), GTK_WINDOW(app));
  gtk_window_set_modal(GTK_WINDOW(about), TRUE);

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
