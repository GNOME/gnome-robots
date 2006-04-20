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
#include <gdk/gdkkeysyms.h>
#include <gconf/gconf-client.h>
#include <games-frame.h>
#include <games-controls.h>

#include "properties.h"
#include "gameconfig.h"
#include "gnobots.h"
#include "graphics.h"
#include "gbdefs.h"
#include "keyboard.h"
#include "game.h"
#include "menu.h"


/**********************************************************************/
/* Defines                                                            */
/**********************************************************************/

#define KB_TEXT_WIDTH    60
#define KB_TEXT_HEIGHT   32

#define KEY_DIR              "/apps/gnobots2"
#define KEY_SHOW_TOOLBAR     "/apps/gnobots2/preferences/show_toolbar"
#define KEY_CONTROL_KEY      "/apps/gnobots2/preferences/key%02d"
#define KEY_THEME            "/apps/gnobots2/preferences/theme"
#define KEY_CONFIGURATION    "/apps/gnobots2/preferences/configuration"
#define KEY_SAFE_MOVES       "/apps/gnobots2/preferences/use_safe_moves"
#define KEY_SUPER_SAFE_MOVES "/apps/gnobots2/preferences/use_super_safe_moves"
#define KEY_ENABLE_SOUND     "/apps/gnobots2/preferences/enable_sound"
#define KEY_ENABLE_SPLATS    "/apps/gnobots2/preferences/enable_splats"
#define KEY_BACKGROUND_COLOR "/apps/gnobots2/preferences/background_color"
#define KEY_WINDOW_WIDTH     "/apps/gnobots2/geometry/width"
#define KEY_WINDOW_HEIGHT    "/apps/gnobots2/geometry/height"

/**********************************************************************/


/**********************************************************************/
/* Game Properties Structure                                          */
/**********************************************************************/
typedef struct _GnobotsProperties GnobotsProperties;

struct _GnobotsProperties {
  gboolean safe_moves;
  gboolean super_safe_moves;
  gboolean sound;
  gboolean splats;
  gboolean show_toolbar;
  GdkColor bgcolour;
  gint     selected_graphics;
  gint     selected_config;
  gint     keys[12];
};
/**********************************************************************/


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static GtkWidget         *propbox      = NULL;

static GnobotsProperties  properties;

static gint default_keys[12] = {
  GDK_KP_7, GDK_KP_8, GDK_KP_9, 
  GDK_KP_4, GDK_KP_5, GDK_KP_6,
  GDK_KP_1, GDK_KP_2, GDK_KP_3,
  GDK_KP_Add, GDK_KP_Multiply, GDK_KP_Enter};


static GConfClient *gconf_client;

/**********************************************************************/


/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/
static void apply_changes (void);
static void apply_cb (GtkWidget*, gpointer);
static gboolean delete_cb (GtkWidget*, gpointer);
static void pmap_selection (GtkWidget*, gpointer);
static void type_selection (GtkWidget*, gpointer);
static void safe_cb (GtkWidget*, gpointer);
static void sound_cb (GtkWidget*, gpointer);
static void splat_cb (GtkWidget*, gpointer);
static void defkey_cb (GtkWidget*, gpointer);
static void fill_typemenu (GtkWidget*);
static void fill_pmapmenu (GtkWidget*);
static void gconf_set_background_color (GdkColor * c);
/**********************************************************************/


/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

/**
 * apply_changes
 *
 * Description:
 * Applies the changes made by the user
 **/
static void
apply_changes (void)
{
  update_score_state ();
}


/**
 * apply_cb
 * @w: widget
 * @data: callback data
 *
 * Description:
 * handles apply button events
 *
 * Returns:
 * TRUE if the event was handled
 **/
static void
apply_cb (GtkWidget *w, gpointer data)
{
  apply_changes ();

  gtk_widget_destroy (propbox);
  propbox = NULL;
}


/**
 * destroy_cb
 * @w: widget
 * @data: callback data
 *
 * Description:
 * handles property-box destruction messages
 **/
static gboolean
delete_cb (GtkWidget *w, gpointer data)
{
  propbox = NULL;

  return FALSE;
}

gint save_window_geometry (GtkWidget *w, GdkEventConfigure *e, gpointer data)
{
  gconf_client_set_int (gconf_client, KEY_WINDOW_WIDTH, e->width, NULL);
  gconf_client_set_int (gconf_client, KEY_WINDOW_HEIGHT, e->height, NULL);

  return FALSE;
}

void set_window_geometry (GtkWidget *window)
{
  int w, h;

  w = gconf_client_get_int (gconf_client, KEY_WINDOW_WIDTH, NULL);
  h = gconf_client_get_int (gconf_client, KEY_WINDOW_HEIGHT, NULL);

  gtk_window_resize (GTK_WINDOW (window), w, h);
}

/**
 * pmap_selection
 * @widget: widget
 * @data: callback data
 *
 * Description:
 * handles pixmap selection messages
 **/
static void
pmap_selection (GtkWidget *widget, gpointer data)
{
  gint num = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

  properties.selected_graphics = num;

  gconf_set_theme (game_graphics_name (properties.selected_graphics));

  set_game_graphics (properties.selected_graphics);
  clear_game_area ();
}


/**
 * type_selection
 * @widget: widget
 * @data: callback data
 *
 * Description:
 * handles configuration selection messages
 **/
static void
type_selection (GtkWidget *widget, gpointer data)
{
  gint num = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

  properties.selected_config = num;

  gconf_set_configuration (game_config_name (properties.selected_config));

  set_game_config (properties.selected_config);

  start_new_game ();
}


/**
 * safe_cb
 * @widget: widget
 * @data: callback data
 *
 * Description:
 * handles message from the 'safe moves' checkbox
 **/
static void
safe_cb (GtkWidget *widget, gpointer data)
{
  properties.safe_moves = GTK_TOGGLE_BUTTON (widget)->active;
  gconf_set_use_safe_moves (properties.safe_moves);
}


/**
 * super_safe_cb
 * @widget: widget
 * @data: callback data
 *
 * Description:
 * handles message from the 'super_safe moves' checkbox
 **/
static void
super_safe_cb (GtkWidget *widget, gpointer data)
{
  properties.super_safe_moves = GTK_TOGGLE_BUTTON (widget)->active;
  gconf_set_use_super_safe_moves (properties.super_safe_moves);
}


/**
 * sound_cb
 * @widget: widget
 * @data: callback data
 *
 * Description:
 * handles message from the 'sound' checkbox
 **/
static void
sound_cb (GtkWidget *widget, gpointer data)
{
  properties.sound = GTK_TOGGLE_BUTTON (widget)->active;
  gconf_set_enable_sound (properties.sound);
}


/**
 * splat_cb
 * @widget: widget
 * @data: callback data
 *
 * Description:
 * handles message from the 'splat' checkbox
 **/
static void
splat_cb (GtkWidget *widget, gpointer data)
{
  properties.splats = GTK_TOGGLE_BUTTON (widget)->active;
  gconf_set_enable_splats (properties.splats);
}


/**
 * defkey_cb
 * @widget: widget
 * @data: callback data
 *
 * Description:
 * handles message from the default key buttons
 **/
static void
defkey_cb (GtkWidget *widget, gpointer data)
{
  gint i;
  gint *dkeys = (gint*)data;

  for (i = 0; i < 12; ++i){
    properties.keys[i] = dkeys[i];
    gconf_set_control_key (i, keyboard_string (properties.keys[i]));
  }
  keyboard_set (properties.keys);
}


/**
 * fill_typemenu
 * @menu: listbox menu
 *
 * Description:
 * fills the listbox with configuration names
 **/
static void
fill_typemenu (GtkWidget *menu)
{
  gint i;

#if 0
  /* this is just a place holder so that xgettext can found the strings to
   * translate (those are the default games types)
   */
  char *just_a_place_holder[]={ 
    N_("classic robots"),
    N_("robots2"),
    N_("robots2 easy"),
    N_("robots with safe teleport"),
    N_("nightmare")
  };
#endif

  for (i = 0; i < num_game_configs (); ++i){
    gtk_combo_box_append_text (GTK_COMBO_BOX (menu), _(game_config_name (i)));
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (menu), properties.selected_config);
}


/**
 * fill_pmapmenu
 * @menu: menu
 *
 * Description:
 * fills the listbox with pixmap names
 **/
static void
fill_pmapmenu (GtkWidget *menu)
{
  gint i;

#if 0
  /* this is just a place holder so that xgettext can found the strings to
   * translate (those are the default graphic styles)
   */
  char *just_a_place_holder[]={
    N_("robots"),
    N_("cows"),
    N_("eggs"),
    N_("gnomes"),
    N_("mice"),
    N_("windows"),
  };
#endif

  for (i = 0; i < num_game_graphics (); ++i) {
    gtk_combo_box_append_text (GTK_COMBO_BOX (menu),
                               _(game_graphics_name (i)));
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (menu),
                            properties.selected_graphics);

}

static void
bg_color_callback (GtkWidget *widget, gpointer data)
{
  gtk_color_button_get_color (GTK_COLOR_BUTTON (widget),
                              &properties.bgcolour);  
  set_background_color (properties.bgcolour);
  clear_game_area ();
  gconf_set_background_color (&properties.bgcolour);
}

/**
 * show_properties_dialog
 *
 * Description:
 * displays the properties dialog
 **/
void 
show_properties_dialog (void)
{
  GtkWidget *notebook;
  GtkWidget *cpage;
  GtkWidget *gpage;
  GtkWidget *kpage;
  GtkWidget *label;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *typemenu;
  GtkWidget *pmapmenu;
  GtkWidget *chkbox;
  GtkWidget *table;
  GtkWidget *dbut;
  GtkWidget *frame;
  GtkWidget *w;
  GtkWidget *controls_list;
  GtkTooltips *tooltips;

  if (propbox) 
    return;

  tooltips = gtk_tooltips_new ();

  propbox = gtk_dialog_new_with_buttons (_("Robots Preferences"),
                                         GTK_WINDOW  (app),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT,
                                         NULL);
  gtk_dialog_set_has_separator (GTK_DIALOG (propbox), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (propbox), 5);
  gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (propbox)->vbox), 2);
  /* Set up notebook and add it to hbox of the gtk_dialog */
  g_signal_connect (G_OBJECT (propbox), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &propbox);
  
  notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 5);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (propbox)->vbox), notebook, 
                      TRUE, TRUE, 0);

  /* The configuration page */
  cpage = gtk_vbox_new (FALSE, 18);
  gtk_container_set_border_width (GTK_CONTAINER (cpage), 12);

  frame = games_frame_new (_("Game Type"));
  gtk_box_pack_start (GTK_BOX (cpage), frame, TRUE, TRUE, 0);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  typemenu = gtk_combo_box_new_text ();
  g_signal_connect (G_OBJECT (typemenu), "changed",
		    G_CALLBACK (type_selection), NULL);
  fill_typemenu (typemenu);
  gtk_box_pack_start_defaults (GTK_BOX (hbox), typemenu);
  
  frame = games_frame_new (_("Options"));
  gtk_box_pack_start (GTK_BOX (cpage), frame, FALSE, FALSE, 0);
  vbox = gtk_vbox_new (TRUE, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

  chkbox = gtk_check_button_new_with_mnemonic (_("_Use safe moves"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkbox), 
				properties.safe_moves);
  g_signal_connect (G_OBJECT (chkbox), "clicked",
		    (GtkSignalFunc)safe_cb, NULL);
  gtk_table_attach (GTK_TABLE (table), chkbox, 0, 1, 0, 1, GTK_FILL, 0, 0, 0);
  gtk_tooltips_set_tip (tooltips, chkbox,
			_("Prevent some dangerous moves"), 
			_("Prevent accidental moves that result in getting killed."));

  chkbox = gtk_check_button_new_with_mnemonic (_("U_se super safe moves"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkbox), properties.super_safe_moves);
  g_signal_connect (G_OBJECT (chkbox), "clicked",
		    (GtkSignalFunc)super_safe_cb, NULL);
  gtk_table_attach (GTK_TABLE (table), chkbox, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
  gtk_tooltips_set_tip (tooltips, chkbox,
			_("Prevent all dangerous moves"), 
			_("Prevents all moves that result in getting killed."));

  chkbox = gtk_check_button_new_with_mnemonic (_("_Enable sounds"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkbox), properties.sound);
  g_signal_connect (G_OBJECT (chkbox), "clicked",
		    (GtkSignalFunc)sound_cb, NULL);
  gtk_table_attach (GTK_TABLE (table), chkbox, 1, 2, 0, 1, GTK_FILL, 0, 0, 0);
  gtk_tooltips_set_tip (tooltips, chkbox,
			_("Play sounds for major events"), 
			_("Play sounds for events like winning a level and dying."));

  chkbox = gtk_check_button_new_with_mnemonic (_("E_nable splats"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkbox), properties.splats);
  g_signal_connect (G_OBJECT (chkbox), "clicked",
		    (GtkSignalFunc)splat_cb, NULL);
  gtk_table_attach (GTK_TABLE (table), chkbox, 1, 2, 1, 2, GTK_FILL, 0, 0, 0);
  gtk_tooltips_set_tip (tooltips, chkbox,
			_("Play a sound when two robots collide"), 
			_("Play the most common, and potentially the most annoying, sound."));

  label = gtk_label_new_with_mnemonic (_("Game"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), cpage, label);


  /* The graphics page */
  gpage = gtk_vbox_new (FALSE, 18);
  gtk_container_set_border_width (GTK_CONTAINER (gpage), 12);

  frame = games_frame_new (_("Graphics Theme"));
  gtk_box_pack_start (GTK_BOX (gpage), frame, FALSE, FALSE, 0);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new_with_mnemonic (_("_Image theme:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, GTK_FILL, 0, 0, 0);

  pmapmenu = gtk_combo_box_new_text ();
  g_signal_connect (G_OBJECT (pmapmenu), "changed",
                    G_CALLBACK (pmap_selection), NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), pmapmenu);

  fill_pmapmenu (pmapmenu);
  gtk_table_attach_defaults (GTK_TABLE (table), pmapmenu, 1, 2, 0, 1);

  label = gtk_label_new_with_mnemonic (_("_Background color:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);

  w  = gtk_color_button_new ();
  gtk_color_button_set_color (GTK_COLOR_BUTTON (w), &properties.bgcolour);
  g_signal_connect (G_OBJECT (w), "color_set",
                    G_CALLBACK (bg_color_callback), NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), w);

  gtk_table_attach_defaults (GTK_TABLE (table), w, 1, 2, 1, 2);

  label = gtk_label_new_with_mnemonic (_("Appearance"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), gpage, label);

  /* The keyboard page */
  kpage = gtk_vbox_new (FALSE, 18);
  gtk_container_set_border_width (GTK_CONTAINER (kpage), 12);

  frame = games_frame_new (_("Keyboard Controls"));
  gtk_box_pack_start (GTK_BOX (kpage), frame, TRUE, TRUE, 0);
  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  controls_list = games_controls_list_new ();
  games_controls_list_add_controls (GAMES_CONTROLS_LIST (controls_list), 
				    "/apps/gnobots2/preferences/key00",
				    "/apps/gnobots2/preferences/key01",
				    "/apps/gnobots2/preferences/key02",
				    "/apps/gnobots2/preferences/key03",
				    "/apps/gnobots2/preferences/key05",
				    "/apps/gnobots2/preferences/key06",
				    "/apps/gnobots2/preferences/key07",
				    "/apps/gnobots2/preferences/key08",
				    "/apps/gnobots2/preferences/key04",
				    "/apps/gnobots2/preferences/key09",
				    "/apps/gnobots2/preferences/key10",
				    "/apps/gnobots2/preferences/key11",
				    NULL);

  gtk_box_pack_start (GTK_BOX (vbox), controls_list, TRUE, TRUE, 0);

  hbox = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbox), GTK_BUTTONBOX_START);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  dbut = gtk_button_new_with_mnemonic (_("_Restore Defaults"));
  g_signal_connect (G_OBJECT (dbut), "clicked",
                    G_CALLBACK (defkey_cb), (gpointer)default_keys);  
  gtk_box_pack_start (GTK_BOX (hbox), dbut, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("Keyboard"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), kpage, label);


  g_signal_connect  (G_OBJECT  (propbox), "delete_event",
                     G_CALLBACK (delete_cb), NULL);
  g_signal_connect (G_OBJECT (propbox), "response",
                    G_CALLBACK (apply_cb), NULL);

  gtk_widget_show_all (propbox);
}


GConfClient *
get_gconf_client ()
{
  if (!gconf_client) 
    gconf_client = gconf_client_get_default ();
  return gconf_client;
}

void
initialize_gconf (int argc, char *argv[])
{
  gconf_init (argc, argv, NULL);
  gconf_client = get_gconf_client ();
  gconf_client_add_dir (gconf_client, KEY_DIR,
                        GCONF_CLIENT_PRELOAD_NONE, NULL);
}

/**
 * load_properties
 *
 * Description:
 * loads the game properties from a file
 *
 * Returns:
 * TRUE if the properties can be loaded, FALSE otherwise
 **/
gboolean
load_properties (void)
{
  gchar buffer[256];
  gchar *sname = NULL;
  gchar *cname = NULL;
  gint i;
  gchar *str;
  gchar * bgcolour;

  for (i = 0; i < 12; i++) {
    properties.keys[i] = default_keys[i];

    sprintf (buffer, KEY_CONTROL_KEY, i);

    str = gconf_client_get_string (get_gconf_client (), buffer, NULL);    
    if (str != NULL) {
      properties.keys[i] = gdk_keyval_from_name (str);
    }
    if ((str == NULL) || (properties.keys[i] == GDK_VoidSymbol)) {
      properties.keys[i] = default_keys[i];
    }
    g_free (str);
  }

  bgcolour = gconf_client_get_string (get_gconf_client (),
                                      KEY_BACKGROUND_COLOR, NULL);
  if (bgcolour == NULL)
    bgcolour = g_strdup ("#7590AE");
  
  gdk_color_parse (bgcolour, &properties.bgcolour);
  set_background_color (properties.bgcolour);
  
  sname = gconf_client_get_string (get_gconf_client (), KEY_THEME, NULL);
  if (sname == NULL)
    sname = g_strdup ("robots");

  properties.selected_graphics = 0;
  for (i = 0; i < num_game_graphics (); ++i) {
    if (! strcmp (sname, game_graphics_name (i))) {
      properties.selected_graphics = i;
      break;
    }
  }
  g_free (sname);

  cname = gconf_client_get_string (get_gconf_client (), KEY_CONFIGURATION, NULL);
  if (cname == NULL)
    cname = g_strdup ("classic_robots");

  properties.selected_config = 0;
  for (i = 0; i < num_game_configs (); ++i) {
    if (! strcmp (cname, game_config_name (i))) {
      properties.selected_config = i;
      break;
    }
  }
  g_free (cname);

  properties.safe_moves = gconf_client_get_bool (get_gconf_client (),
                                                 KEY_SAFE_MOVES, NULL);
  properties.super_safe_moves = gconf_client_get_bool (get_gconf_client (),
                                                       KEY_SUPER_SAFE_MOVES, NULL);
  properties.sound = gconf_client_get_bool (get_gconf_client (),
                                            KEY_ENABLE_SOUND, NULL);
  properties.splats = gconf_client_get_bool (get_gconf_client (),
                                             KEY_ENABLE_SPLATS, NULL);
  properties.show_toolbar = gconf_client_get_bool (get_gconf_client (),
						   KEY_SHOW_TOOLBAR, NULL);

  set_game_graphics (properties.selected_graphics);
  set_game_config (properties.selected_config);
  keyboard_set (properties.keys);
  update_score_state ();
  return TRUE;
}

void
gconf_set_theme (gchar *value)
{
  gconf_client_set_string (get_gconf_client (), KEY_THEME,
                           value, NULL);
}

static void
gconf_set_background_color (GdkColor * c)
{
  gchar * name;

  name = g_strdup_printf ("#%04x%04x%04x", c->red, c->green, c->blue);
  
  gconf_client_set_string (get_gconf_client (), KEY_BACKGROUND_COLOR,
                           name, NULL);

  g_free (name);
}

void
gconf_set_configuration (gchar *value)
{
  gconf_client_set_string (get_gconf_client (), KEY_CONFIGURATION,
                           value, NULL);
}

void
gconf_set_use_safe_moves (gboolean value)
{
  gconf_client_set_bool (get_gconf_client (), KEY_SAFE_MOVES,
                         value, NULL);
}

void
gconf_set_use_super_safe_moves (gboolean value)
{
  gconf_client_set_bool (get_gconf_client (), KEY_SUPER_SAFE_MOVES,
                         value, NULL);
}

void
gconf_set_enable_sound (gboolean value)
{
  gconf_client_set_bool (get_gconf_client (), KEY_ENABLE_SOUND,
                         value, NULL);
}

void
gconf_set_enable_splats (gboolean value)
{
  gconf_client_set_bool (get_gconf_client (), KEY_ENABLE_SPLATS,
                         value, NULL);
}

void
gconf_set_show_toolbar (gboolean value)
{
  gconf_client_set_bool (get_gconf_client (), KEY_SHOW_TOOLBAR,
			 value, NULL);
}

void
gconf_set_control_key (gint i, gchar *value)
{
  gchar *buffer;
  buffer = g_strdup_printf (KEY_CONTROL_KEY, i);
  gconf_client_set_string (get_gconf_client (), buffer,
                           value, NULL);
  g_free (buffer);
}

/**
 * save_properties
 *
 * Description:
 * saves the game properties to a file
 *
 * Returns:
 * TRUE if the properties can be saved, FALSE otherwise
 **/
gboolean
save_properties (void)
{
  gint   i;

  for (i = 0; i < 12; i++) {
    gconf_set_control_key (i, gdk_keyval_name (properties.keys[i]));
  }
  
  gconf_set_theme (game_graphics_name (properties.selected_graphics));
  gconf_set_configuration (game_config_name (properties.selected_config));
  gconf_set_use_safe_moves (properties.safe_moves);
  gconf_set_use_super_safe_moves (properties.super_safe_moves);
  gconf_set_enable_sound (properties.sound);
  gconf_set_enable_splats (properties.splats);

  return TRUE;
}


/**
 * properties_safe_moves
 *
 * Description:
 * returns safe-moves setting
 *
 * Returns:
 * TRUE if safe-moves are selected
 **/
gboolean
properties_safe_moves (void)
{
  return properties.safe_moves;
}


/**
 * properties_super_safe_moves
 *
 * Description:
 * returns super-safe-moves setting
 *
 * Returns:
 * TRUE if safe-moves are selected
 **/
gboolean
properties_super_safe_moves (void)
{
  return properties.super_safe_moves;
}


/**
 * properties_sound
 *
 * Description:
 * returns sound setting
 *
 * Returns:
 * TRUE if sound is selected
 **/
gboolean
properties_sound (void)
{
  return properties.sound;
}


/**
 * properties_splats
 *
 * Description:
 * returns splat setting
 *
 * Returns:
 * TRUE if splats are selected
 **/
gboolean
properties_splats (void)
{
  return properties.splats;
}


/**
 * properties_show_toolbar
 *
 * Description:
 * returns toolbar setting
 *
 * Returns:
 * TRUE if splats are selected
 **/
gboolean
properties_show_toolbar (void)
{
  return properties.show_toolbar;
}


/**
 * properties_set_config
 * @n: config number
 *
 * Description:
 * sets the current configuration
 *
 * Returns:
 * TRUE if successful, FALSE otherwise
 **/
gboolean
properties_set_config (gint n)
{
  if (! set_game_config (n))
    return FALSE;

  properties.selected_config = n;

  return TRUE;
}

/**********************************************************************/
