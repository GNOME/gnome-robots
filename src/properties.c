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

#include <string.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "properties.h"
#include "gameconfig.h"
#include "gnome-robots.h"
#include "graphics.h"
#include "gbdefs.h"
#include "keyboard.h"
#include "game.h"
#include "menu.h"
#include "games-file-list.h"
#include "games-controls.h"
#include "games-scores.h"
#include "games-scores-dialog.h"


/**********************************************************************/
/* Defines                                                            */
/**********************************************************************/

#define KB_TEXT_WIDTH    60
#define KB_TEXT_HEIGHT   32

#define KEY_PREFERENCES_GROUP "preferences"
#define KEY_BACKGROUND_COLOR  "background-color"
#define KEY_CONFIGURATION     "configuration"
#define KEY_ENABLE_SOUND      "enable-sound"
#define KEY_SAFE_MOVES        "use-safe-moves"
#define KEY_SHOW_TOOLBAR      "show-toolbar"
#define KEY_SUPER_SAFE_MOVES  "use-super-safe-moves"
#define KEY_THEME             "theme"
#define KEY_CONTROL_KEY       "key%02d"

/**********************************************************************/


/**********************************************************************/
/* Game Properties Structure                                          */
/**********************************************************************/
typedef struct _GnobotsProperties GnobotsProperties;

#define N_KEYS 12

struct _GnobotsProperties {
  gboolean safe_moves;
  gboolean super_safe_moves;
  gboolean sound;
  gboolean show_toolbar;
  GdkRGBA bgcolour;
  gint selected_config;
  guint keys[N_KEYS];
  const gchar *themename;
};
/**********************************************************************/


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static GtkWidget *propbox = NULL;

static GamesFileList *theme_list = NULL;

static GnobotsProperties properties;

/**********************************************************************/


/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/
static void load_keys (void);
static void apply_changes (void);
static void apply_cb (GtkWidget *, gpointer);
static gboolean delete_cb (GtkWidget *, gpointer);
static void pmap_selection (GtkWidget *, gpointer);
static void type_selection (GtkWidget *, gpointer);
static void safe_cb (GtkWidget *, gpointer);
static void sound_cb (GtkWidget *, gpointer);
static void defkey_cb (GtkWidget *, gpointer);
static void fill_typemenu (GtkWidget *);
static void conf_set_background_color (GdkRGBA * c);
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
  load_keys ();
  keyboard_set (properties.keys);
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
apply_cb (GtkWidget * w, gpointer data)
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
delete_cb (GtkWidget * w, gpointer data)
{
  propbox = NULL;

  return FALSE;
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
pmap_selection (GtkWidget * widget, gpointer data)
{
  gint n;

  n = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

  /* FIXME: Should be de-suffixed. */
  properties.themename = games_file_list_get_nth (theme_list, n);

  conf_set_theme (properties.themename);

  load_game_graphics ();
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
type_selection (GtkWidget * widget, gpointer data)
{
  gchar *config;
  gint num = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

  properties.selected_config = num;

  config = game_config_name (properties.selected_config);
  conf_set_configuration (config);
  g_free (config);

  set_game_config (properties.selected_config);

  start_new_game ();
}


/**
 * safe_cb
 * @widget: widget
 * @data: the super safe moves checkbox
 *
 * Description:
 * handles message from the 'safe moves' checkbox
 **/
static void
safe_cb (GtkWidget * widget, gpointer data)
{
  GtkWidget *super_safe_chkbox;
  super_safe_chkbox = GTK_WIDGET (data);
  properties.safe_moves = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
  conf_set_use_safe_moves (properties.safe_moves);
  gtk_widget_set_sensitive (super_safe_chkbox, properties.safe_moves);
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
super_safe_cb (GtkWidget * widget, gpointer data)
{
  properties.super_safe_moves = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
  conf_set_use_super_safe_moves (properties.super_safe_moves);
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
sound_cb (GtkWidget * widget, gpointer data)
{
  properties.sound = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
  conf_set_enable_sound (properties.sound);
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
defkey_cb (GtkWidget * widget, gpointer data)
{
  gint i;

  for (i = 0; i < 12; ++i) {
    GVariant *variant;
    char buffer[64];

    g_snprintf (buffer, sizeof (buffer), KEY_CONTROL_KEY, i);
    g_settings_reset (settings, buffer);
    variant = g_settings_get_default_value (settings, buffer);
    properties.keys[i] = g_variant_get_int32 (variant);
    g_variant_unref (variant);
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
fill_typemenu (GtkWidget * menu)
{
  gint i;
  gchar *config;

  for (i = 0; i < num_game_configs (); ++i) {
    config = game_config_name (i);
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (menu), _(config));
    g_free (config);
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
static GtkWidget *
make_theme_menu (void)
{
  gchar *dir;

  if (theme_list)
    g_object_unref (theme_list);

  dir = g_build_filename (DATA_DIRECTORY, "themes", NULL);
  theme_list = games_file_list_new_images (dir, NULL);
  g_free (dir);
  games_file_list_transform_basename (theme_list);

  /* FIXME: Get rid of the bubbles images from the list (preferably by
   * getting tid of the bubble pixmaps. */

  return games_file_list_create_widget (theme_list,
					properties.themename,
					GAMES_FILE_LIST_REMOVE_EXTENSION |
					GAMES_FILE_LIST_REPLACE_UNDERSCORES);
}

static void
bg_color_callback (GtkWidget * widget, gpointer data)
{
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (widget),
			      &properties.bgcolour);
  set_background_color (properties.bgcolour);
  clear_game_area ();
  conf_set_background_color (&properties.bgcolour);
}

const gchar *
properties_theme_name (void)
{
  return properties.themename;
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
  GtkWidget *safe_chkbox;
  GtkWidget *grid;
  GtkWidget *dbut;
  GtkWidget *w;
  GtkWidget *controls_list;

  if (propbox)
    return;

  propbox = gtk_dialog_new_with_buttons (_("Preferences"),
					 GTK_WINDOW (window),
					 GTK_DIALOG_USE_HEADER_BAR,
					 NULL);
  gtk_container_set_border_width (GTK_CONTAINER (propbox), 5);
  gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (propbox))), 2);
  /* Set up notebook and add it to hbox of the gtk_dialog */
  g_signal_connect (G_OBJECT (propbox), "destroy",
		    G_CALLBACK (gtk_widget_destroyed), &propbox);

  notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 5);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (propbox))),
              notebook, TRUE, TRUE, 0);

  /* The configuration page */
  cpage = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_container_set_border_width (GTK_CONTAINER (cpage), 12);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_box_pack_start (GTK_BOX (cpage), grid, FALSE, FALSE, 0);

  label = gtk_label_new (_("Game Type"));
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

  typemenu = gtk_combo_box_text_new ();
  fill_typemenu (typemenu);
  gtk_grid_attach (GTK_GRID (grid), typemenu, 1, 0, 1, 1);

  g_signal_connect (G_OBJECT (typemenu), "changed",
		    G_CALLBACK (type_selection), NULL);

  chkbox = gtk_check_button_new_with_mnemonic (_("_Use safe moves"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkbox),
				properties.safe_moves);
  gtk_grid_attach (GTK_GRID (grid), chkbox, 0, 1, 2, 1);
  gtk_widget_set_tooltip_text (chkbox,
                               _("Prevent accidental moves that result in getting killed."));
  safe_chkbox = chkbox;

  chkbox = gtk_check_button_new_with_mnemonic (_("U_se super safe moves"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkbox),
				properties.super_safe_moves);
  g_signal_connect (G_OBJECT (chkbox), "clicked",
		    (GCallback) super_safe_cb, NULL);
  gtk_grid_attach (GTK_GRID (grid), chkbox, 0, 2, 2, 1);
  gtk_widget_set_tooltip_text (chkbox,
                               _("Prevents all moves that result in getting killed."));
  gtk_widget_set_sensitive (chkbox, properties.safe_moves);

  g_signal_connect (G_OBJECT (safe_chkbox), "clicked",
		    (GCallback) safe_cb, (gpointer) chkbox);

  chkbox = gtk_check_button_new_with_mnemonic (_("_Enable sounds"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkbox), properties.sound);
  g_signal_connect (G_OBJECT (chkbox), "clicked",
		    (GCallback) sound_cb, NULL);
  gtk_grid_attach (GTK_GRID (grid), chkbox, 0, 3, 2, 1);
  gtk_widget_set_tooltip_text (chkbox,
                               _("Play sounds for events like winning a level and dying."));

  label = gtk_label_new_with_mnemonic (_("Game"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), cpage, label);


  /* The graphics page */
  gpage = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_container_set_border_width (GTK_CONTAINER (gpage), 12);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
  gtk_box_pack_start (GTK_BOX (gpage), grid, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Image theme:"));
  gtk_widget_set_hexpand (label, TRUE);
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

  pmapmenu = make_theme_menu ();
  g_signal_connect (G_OBJECT (pmapmenu), "changed",
		    G_CALLBACK (pmap_selection), NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), pmapmenu);

  gtk_grid_attach (GTK_GRID (grid), pmapmenu, 1, 0, 1, 1);

  label = gtk_label_new_with_mnemonic (_("_Background color:"));
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);

  w = gtk_color_button_new ();
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (w), &properties.bgcolour);
  g_signal_connect (G_OBJECT (w), "color_set",
		    G_CALLBACK (bg_color_callback), NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), w);

  gtk_grid_attach (GTK_GRID (grid), w, 1, 1, 1, 1);

  label = gtk_label_new_with_mnemonic (_("Appearance"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), gpage, label);

  /* The keyboard page */
  kpage = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_container_set_border_width (GTK_CONTAINER (kpage), 12);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (kpage), vbox, TRUE, TRUE, 0);

  controls_list = games_controls_list_new (settings);
  games_controls_list_add_controls (GAMES_CONTROLS_LIST (controls_list),
				    "key00", _("Key to move NW"), g_settings_get_default_value (settings, "key00"),
				    "key01", _("Key to move N"), g_settings_get_default_value (settings, "key01"),
				    "key02", _("Key to move NE"), g_settings_get_default_value (settings, "key02"),
				    "key03", _("Key to move W"), g_settings_get_default_value (settings, "key03"),
				    "key04", _("Key to hold"), g_settings_get_default_value (settings, "key04"),
				    "key05", _("Key to move E"), g_settings_get_default_value (settings, "key05"),
				    "key06", _("Key to move SW"), g_settings_get_default_value (settings, "key06"),
				    "key07", _("Key to move S"), g_settings_get_default_value (settings, "key07"),
				    "key08", _("Key to move SE"), g_settings_get_default_value (settings, "key08"),
                                    NULL);

  gtk_box_pack_start (GTK_BOX (vbox), controls_list, TRUE, TRUE, 0);

  hbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbox), GTK_BUTTONBOX_START);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  dbut = gtk_button_new_with_mnemonic (_("_Restore Defaults"));
  g_signal_connect (G_OBJECT (dbut), "clicked",
		    G_CALLBACK (defkey_cb), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), dbut, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("Keyboard"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), kpage, label);


  g_signal_connect (G_OBJECT (propbox), "delete_event",
		    G_CALLBACK (delete_cb), NULL);
  g_signal_connect (G_OBJECT (propbox), "response",
		    G_CALLBACK (apply_cb), NULL);

  gtk_widget_show_all (propbox);
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
  gchar *cname = NULL;
  gint i;
  gchar *bgcolour, *config;

  load_keys ();

  bgcolour = g_settings_get_string (settings, KEY_BACKGROUND_COLOR);
  gdk_rgba_parse (&properties.bgcolour, bgcolour);
  set_background_color (properties.bgcolour);

  properties.themename = g_settings_get_string (settings, KEY_THEME);

  cname = g_settings_get_string (settings, KEY_CONFIGURATION);

  properties.selected_config = 0;
  for (i = 0; i < num_game_configs (); ++i) {
    config = game_config_name (i);
    if (!strcmp (cname, config)) {
      g_free (config);
      properties.selected_config = i;
      break;
    }
    g_free (config);
  }
  g_free (cname);

  properties.safe_moves = g_settings_get_boolean (settings,
						  KEY_SAFE_MOVES);
  properties.super_safe_moves = g_settings_get_boolean (settings,
						        KEY_SUPER_SAFE_MOVES);
  properties.sound = g_settings_get_boolean (settings,
                                             KEY_ENABLE_SOUND);
  properties.show_toolbar = g_settings_get_boolean (settings,
                                                    KEY_SHOW_TOOLBAR);

  load_game_graphics ();
  set_game_config (properties.selected_config);
  keyboard_set (properties.keys);
  return TRUE;
}

void
load_keys (void)
{
  gchar buffer[256];
  gint i;

  for (i = 0; i < 12; i++) {
    g_snprintf (buffer, sizeof (buffer), KEY_CONTROL_KEY, i);
    properties.keys[i] = g_settings_get_int (settings, buffer);
  }
}

void
conf_set_theme (const gchar * value)
{
  g_settings_set_string (settings, KEY_THEME, value);
}

static void
conf_set_background_color (GdkRGBA * c)
{
  char colour[64];

  g_snprintf (colour, sizeof (colour), "#%04x%04x%04x", (int) (c->red * 65535 + 0.5), (int) (c->green * 65535 + 0.5), (int) (c->blue * 65535 + 0.5));

  g_settings_set_string (settings, KEY_BACKGROUND_COLOR, colour);
}

void
conf_set_configuration (gchar * value)
{
  g_settings_set_string (settings, KEY_CONFIGURATION, value);
}

void
conf_set_use_safe_moves (gboolean value)
{
  g_settings_set_boolean (settings, KEY_SAFE_MOVES, value);
}

void
conf_set_use_super_safe_moves (gboolean value)
{
  g_settings_set_boolean (settings, KEY_SUPER_SAFE_MOVES, value);
}

void
conf_set_enable_sound (gboolean value)
{
  g_settings_set_boolean (settings, KEY_ENABLE_SOUND, value);
}

void
conf_set_show_toolbar (gboolean value)
{
  g_settings_set_boolean (settings, KEY_SHOW_TOOLBAR, value);
}

void
conf_set_control_key (gint i, guint keyval)
{
  char buffer[64];
  gchar *keyval_name;

  g_snprintf (buffer, sizeof (buffer), KEY_CONTROL_KEY, i);
  keyval_name = gdk_keyval_name (keyval);
  g_settings_set_string (settings, buffer, keyval_name);
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
  gint i;
  gchar *config;

  for (i = 0; i < 12; i++) {
    conf_set_control_key (i, properties.keys[i]);
  }

  conf_set_theme (properties.themename);

  config = game_config_name (properties.selected_config);
  conf_set_configuration (config);
  g_free (config);

  conf_set_use_safe_moves (properties.safe_moves);
  conf_set_use_super_safe_moves (properties.super_safe_moves);
  conf_set_enable_sound (properties.sound);

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
  if (!set_game_config (n))
    return FALSE;

  properties.selected_config = n;

  return TRUE;
}

/**********************************************************************/
