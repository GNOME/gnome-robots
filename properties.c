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
#define KEY_CONTROL_KEY      "/apps/gnobots2/preferences/key%02d"
#define KEY_THEME            "/apps/gnobots2/preferences/theme"
#define KEY_CONFIGURATION    "/apps/gnobots2/preferences/configuration"
#define KEY_SAFE_MOVES       "/apps/gnobots2/preferences/use_safe_moves"
#define KEY_SUPER_SAFE_MOVES "/apps/gnobots2/preferences/use_super_safe_moves"
#define KEY_ENABLE_SOUND     "/apps/gnobots2/preferences/enable_sound"
#define KEY_ENABLE_SPLATS    "/apps/gnobots2/preferences/enable_splats"
#define KEY_BACKGROUND_COLOR "/apps/gnobots2/preferences/background_color"

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
static GtkWidget         *list_view    = NULL;
enum {
  PROPERTY_STRING,
  VALUE_STRING,
  INDEX_INT,
  NCOLS
}; /* Column indices for list */

static GtkWidget         *etext[12];
static GtkWidget         *key_labels[12];
static GnobotsProperties  properties;

static gint default_keys1[12] = {
  GDK_Y, GDK_K, GDK_U, 
  GDK_H, GDK_period, GDK_L,
  GDK_B, GDK_J, GDK_N,
  GDK_T, GDK_R, GDK_Return};

static gint default_keys2[12] = {
  GDK_Q, GDK_W, GDK_E, 
  GDK_A, GDK_S, GDK_D,
  GDK_Z, GDK_X, GDK_C,
  GDK_T, GDK_R, GDK_Return};

static gint default_keys3[12] = {
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
static void fill_property_list (void);
static void pmap_selection (GtkWidget*, gpointer);
static void type_selection (GtkWidget*, gpointer);
static void safe_cb (GtkWidget*, gpointer);
static void sound_cb (GtkWidget*, gpointer);
static void splat_cb (GtkWidget*, gpointer);
static void keypad_cb (GtkWidget*, GdkEventKey*, gpointer);
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
  list_view = NULL;
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
  list_view = NULL;
  propbox = NULL;

  return FALSE;
}


/**
 * fill_property_list
 *
 * Description:
 * Fills the property list of configuration values
 **/
static void
fill_property_list (void)
{
  gchar *entry[2];
  gchar buffer[256];
  GameConfig *gc;
  gboolean type2exist = TRUE;
  GtkListStore *list;
  GtkTreeIter iter;
  int index = 0;

  if (list_view == NULL) return;

  list = (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW (list_view));

  gc = game_config_settings (properties.selected_config);

  if ((gc->initial_type2 <= 0) && 
      (gc->increment_type2 <= 0) && 
      (gc->maximum_type2 <= 0)){
    type2exist = FALSE;
  }

  gtk_list_store_clear (list);
  /* I think this is just gtk_list_store_clear () */
  
  if (type2exist){
    entry[0] = _("Initial Number of Type 1 Robots");
  } else {
    entry[0] = _("Initial Number of Robots");
  }
  sprintf (buffer, "%d", gc->initial_type1);
  entry[1] = buffer;

  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  if (type2exist){
    entry[0] = _("Initial Number of Type 2 Robots");
    sprintf (buffer, "%d", gc->initial_type2);
    entry[1] = buffer;
    gtk_list_store_append (list, &iter);
    gtk_list_store_set (list, &iter,
                        PROPERTY_STRING, entry[0],
                        VALUE_STRING, entry[1],
                        INDEX_INT, index,
                        -1);
    index++;
  }

  if (type2exist){
    entry[0] = _("Increasing Number of Type 1 Robots per Level");
  } else {
    entry[0] = _("Increasing Number of Robots per Level");
  }
  sprintf (buffer, "%d", gc->increment_type1);
  entry[1] = buffer;
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  if (type2exist){
    entry[0] = _("Increasing Number of Type 2 Robots per Level");
    sprintf (buffer, "%d", gc->increment_type2);
    entry[1] = buffer;
    gtk_list_store_append (list, &iter);
    gtk_list_store_set (list, &iter,
                        PROPERTY_STRING, entry[0],
                        VALUE_STRING, entry[1],
                        INDEX_INT, index,
                        -1);
    index++;
  }

  if (type2exist){
    entry[0] = _("Maximum Number of Type 1 Robots");
  } else {
    entry[0] = _("Maximum Number of Robots");
  }
  if (gc->maximum_type1 > MAX_ROBOTS){
    sprintf (buffer, _("Fill Screen"));
  } else {
    sprintf (buffer, "%d", gc->maximum_type1);
  }
  entry[1] = buffer;
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  if (type2exist){
    entry[0] = _("Maximum Number of Type 2 Robots");
    if (gc->maximum_type2 > MAX_ROBOTS){
      sprintf (buffer, _("Fill Screen"));
    } else {
      sprintf (buffer, "%d", gc->maximum_type2);
    }
    entry[1] = buffer;
    gtk_list_store_append (list, &iter);
    gtk_list_store_set (list, &iter,
                        PROPERTY_STRING, entry[0],
                        VALUE_STRING, entry[1],
                        INDEX_INT, index,
                        -1);
    index++;
  }

  if (type2exist){
    entry[0] = _("Type 1 Robot Score");
  } else {
    entry[0] = _("Robot Score");
  }
  sprintf (buffer, "%d", gc->score_type1);
  entry[1] = buffer;
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  if (type2exist){
    entry[0] = _("Type 2 Robot Score");
    sprintf (buffer, "%d", gc->score_type2);
    entry[1] = buffer;
    gtk_list_store_append (list, &iter);
    gtk_list_store_set (list, &iter,
                        PROPERTY_STRING, entry[0],
                        VALUE_STRING, entry[1],
                        INDEX_INT, index,
                        -1);
    index++;
  }

  if (type2exist){
    entry[0] = _("Type 1 Robot Score When Waiting");
  } else {
    entry[0] = _("Robot Score When Waiting");
  }
  sprintf (buffer, "%d", gc->score_type1_waiting);
  entry[1] = buffer;
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  if (type2exist){
    entry[0] = _("Type 2 Robot Score When Waiting");
    sprintf (buffer, "%d", gc->score_type2_waiting);
    entry[1] = buffer;
    gtk_list_store_append (list, &iter);
    gtk_list_store_set (list, &iter,
                        PROPERTY_STRING, entry[0],
                        VALUE_STRING, entry[1],
                        INDEX_INT, index,
                        -1);
    index++;
  }

  if (type2exist){
    entry[0] = _("Type 1 Robot Score When Splatted");
  } else {
    entry[0] = _("Robot Score When Splatted");
  }
  sprintf (buffer, "%d", gc->score_type1_splatted);
  entry[1] = buffer;
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  if (type2exist){
    entry[0] = _("Type 2 Robot Score When Splatted");
    sprintf (buffer, "%d", gc->score_type2_splatted);
    entry[1] = buffer;
    gtk_list_store_append (list, &iter);
    gtk_list_store_set (list, &iter,
                        PROPERTY_STRING, entry[0],
                        VALUE_STRING, entry[1],
                        INDEX_INT, index,
                        -1);
    index++;
  }

  entry[0] = _("Initial Number of Safe Teleports");
  sprintf (buffer, "%d", gc->initial_safe_teleports);
  entry[1] = buffer;
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  entry[0] = _("Number of Free Safe Teleports per Level");
  sprintf (buffer, "%d", gc->free_safe_teleports);
  entry[1] = buffer;
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  entry[0] = _("Kills Required While Waiting to Get A Safe Teleport");
  sprintf (buffer, "%d", gc->num_robots_per_safe);
  entry[1] = buffer;
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  entry[0] = _("Score Required To Get A Safe Teleport");
  sprintf (buffer, "%d", gc->safe_score_boundary);
  entry[1] = buffer;
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  entry[0] = _("Maximum Number of Safe Teleports");
  sprintf (buffer, "%d", gc->max_safe_teleports);
  entry[1] = buffer;
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);
  index++;

  entry[0] = _("Moveable Junkheaps");
  if (gc->moveable_heaps) {
    entry[1] = _("Yes");
  } else {
    entry[1] = _("No");
  }
  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter,
                      PROPERTY_STRING, entry[0],
                      VALUE_STRING, entry[1],
                      INDEX_INT, index,
                      -1);

  /* gtk_tree_view_set_model (list_view, list); */

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
  fill_property_list ();

  gconf_set_configuration (game_config_name (properties.selected_config));

  set_game_config (properties.selected_config);
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
 * keypad_cb
 * @widget: widget
 * @event: event
 * @data: callback data
 *
 * Description:
 * handles message from the key selection widgets
 **/
static void
keypad_cb (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  gint keyval = event->keyval;
  gint knum = (gint)data;

  keyval = keyboard_preferred (keyval);

  properties.keys[knum] = keyval;

  gtk_entry_set_text (GTK_ENTRY (widget), keyboard_string (keyval));
  gtk_widget_set_sensitive (widget, FALSE);
  gtk_widget_grab_focus (key_labels[knum]);
  
  gconf_set_control_key (knum, keyboard_string (keyval));
  
  keyboard_set (properties.keys);
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
    gtk_entry_set_text (GTK_ENTRY (etext[i]), 
                        keyboard_string (properties.keys[i]));    
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

static void
enable_entry_cb (GtkWidget * widget, GtkWidget * target)
{
  gtk_widget_set_sensitive (target, TRUE);
  gtk_widget_grab_focus (target);
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
  GtkWidget *scrolled;
  GtkWidget *frame;
  GtkWidget *w;
  GtkTooltips *tooltips;
  GtkListStore *list;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeSelection *selection;
  gint keylayoutx[9] = { 0, 2, 4, 0, 2, 4, 0, 2, 4 };
  gint keylayouty[9] = { 0, 0, 0, 1, 1, 1, 2, 2, 2 };
  gchar * keylabels[9] = { "NW", "N", "NE", "W", "Hold",
                            "E", "SW", "S", "SE"};
  gint i;
  
  if (propbox) 
    return;

  tooltips = gtk_tooltips_new ();

  propbox = gtk_dialog_new_with_buttons (_("GNOME Robots Preferences"),
                                         GTK_WINDOW  (app),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT,
                                         NULL);
  /* Set up notebook and add it to hbox of the gtk_dialog */
  g_signal_connect (G_OBJECT (propbox), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &propbox);
  
  notebook = gtk_notebook_new ();
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (propbox)->vbox), notebook, 
                      TRUE, TRUE, 0);

  /* The configuration page */
  cpage = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (cpage), GNOME_PAD_SMALL);

  frame = games_frame_new (_("Game Type"));
  gtk_box_pack_start (GTK_BOX (cpage), frame, TRUE, TRUE, GNOME_PAD);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtk_hbox_new (TRUE, GNOME_PAD);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, GNOME_PAD);

  if (game_state == STATE_NOT_PLAYING) {
    typemenu = gtk_combo_box_new_text ();
    g_signal_connect (G_OBJECT (typemenu), "changed",
                      G_CALLBACK (type_selection), NULL);
    fill_typemenu (typemenu);
    gtk_box_pack_start_defaults (GTK_BOX (hbox), typemenu);

    list = gtk_list_store_new (NCOLS,
                               G_TYPE_STRING, /* Property */ 
                               G_TYPE_STRING, /* Value */
                               G_TYPE_INT); /* Index - remains hidden */
    /* Create view */
    list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list));
    g_object_unref (list); /* Do I need to create a list store? */

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Property"), 
                                                       renderer, 
                                                       "text", PROPERTY_STRING, 
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list_view), column);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Value"), 
                                                       renderer, 
                                                       "text", VALUE_STRING, 
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list_view), column);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list_view));
    gtk_tree_selection_set_mode  (selection, GTK_SELECTION_SINGLE);

#if 0
    gtk_clist_set_column_width (GTK_CLIST (clist), 0, 400);
    gtk_clist_set_column_width (GTK_CLIST (clist), 1, 50);
    gtk_clist_column_titles_passive (GTK_CLIST (clist));
    gtk_clist_column_titles_show (GTK_CLIST (clist));
#endif

    fill_property_list ();

    /*gtk_widget_set_size_request (list_view, 300, 200);*/

    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type 
      (GTK_SCROLLED_WINDOW (scrolled), GTK_SHADOW_IN);
    gtk_widget_set_size_request (scrolled, 400, 200);
    gtk_container_add (GTK_CONTAINER (scrolled), list_view);

    gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, GNOME_PAD_SMALL);

    frame = games_frame_new (_("Options"));
    gtk_box_pack_start (GTK_BOX (cpage), frame, TRUE, TRUE, GNOME_PAD);
    vbox = gtk_vbox_new (TRUE, 6);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

    table = gtk_table_new (2, 2, TRUE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table), 6);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, TRUE, GNOME_PAD);

    chkbox = gtk_check_button_new_with_label (_("Use safe moves"));
    GTK_TOGGLE_BUTTON (chkbox)->active = properties.safe_moves;
    g_signal_connect (G_OBJECT (chkbox), "clicked",
                      (GtkSignalFunc)safe_cb, NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), chkbox, 0, 1, 0, 1);
    gtk_tooltips_set_tip (tooltips, chkbox,
			  _("Prevent some dangerous moves"), 
			  _("Prevent accidental moves that result in getting killed."));

    chkbox = gtk_check_button_new_with_label (_("Use super safe moves"));
    GTK_TOGGLE_BUTTON (chkbox)->active = properties.super_safe_moves;
    g_signal_connect (G_OBJECT (chkbox), "clicked",
                      (GtkSignalFunc)super_safe_cb, NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), chkbox, 0, 1, 1, 2);
    gtk_tooltips_set_tip (tooltips, chkbox,
			  _("Prevent all dangerous moves"), 
			  _("Prevents all moves that result in getting killed."));

    chkbox = gtk_check_button_new_with_label (_("Enable sounds"));
    GTK_TOGGLE_BUTTON (chkbox)->active = properties.sound;
    g_signal_connect (G_OBJECT (chkbox), "clicked",
                      (GtkSignalFunc)sound_cb, NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), chkbox, 1, 2, 0, 1);
    gtk_tooltips_set_tip (tooltips, chkbox,
			  _("Play sounds for major events"), 
			  _("Play sounds for events like winning a level and dying."));

    chkbox = gtk_check_button_new_with_label (_("Enable splats"));
    GTK_TOGGLE_BUTTON (chkbox)->active = properties.splats;
    g_signal_connect (G_OBJECT (chkbox), "clicked",
                      (GtkSignalFunc)splat_cb, NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), chkbox, 1, 2, 1, 2);
    gtk_tooltips_set_tip (tooltips, chkbox,
			  _("Play a sound when two robots collied"), 
			  _("Play the most common, and potentially the most annoying, sound."));

  } else {
    label = gtk_label_new (_("You Cannot Change the Game Type When Playing"));
    gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  }
  label = gtk_label_new_with_mnemonic (_("Game"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), cpage, label);


  /* The graphics page */
  gpage = gtk_vbox_new (FALSE, GNOME_PAD);
  gtk_container_set_border_width (GTK_CONTAINER (gpage), GNOME_PAD_SMALL);

  frame = games_frame_new (_("Graphics Theme"));
  gtk_box_pack_start (GTK_BOX (gpage), frame, FALSE, FALSE, GNOME_PAD);

  table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new (_("Image theme:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

  pmapmenu = gtk_combo_box_new_text ();
  g_signal_connect (G_OBJECT (pmapmenu), "changed",
                    G_CALLBACK (pmap_selection), NULL);
  fill_pmapmenu (pmapmenu);
  gtk_table_attach_defaults (GTK_TABLE (table), pmapmenu, 1, 2, 0, 1);

  label = gtk_label_new (_("Background color:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

  w  = gtk_color_button_new ();
  gtk_color_button_set_color (GTK_COLOR_BUTTON (w), &properties.bgcolour);
  g_signal_connect (G_OBJECT (w), "color_set",
                    G_CALLBACK (bg_color_callback), NULL);

  gtk_table_attach_defaults (GTK_TABLE (table), w, 1, 2, 1, 2);

  label = gtk_label_new_with_mnemonic (_("Appearance"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), gpage, label);

  /* The keyboard page */
  kpage = gtk_vbox_new (FALSE, GNOME_PAD);
  gtk_container_set_border_width (GTK_CONTAINER (kpage), GNOME_PAD_SMALL);

  frame = games_frame_new (_("Keyboard Controls"));
  gtk_box_pack_start (GTK_BOX (kpage), frame, FALSE, FALSE, GNOME_PAD);
  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtk_hbox_new (FALSE, GNOME_PAD);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, GNOME_PAD);

  table = gtk_table_new (8, 3, TRUE);

  for (i = 0; i<9; i++) {
    key_labels[i] = gtk_button_new_with_label (keylabels[i]);
    gtk_table_attach (GTK_TABLE (table), key_labels[i], keylayoutx[i],
                      keylayoutx[i]+1, keylayouty[i], keylayouty[i]+1,
                      GTK_FILL, GTK_FILL, 3, 3);
    etext[i] = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (etext[i]),
                        keyboard_string (properties.keys[i]));
    gtk_editable_set_editable (GTK_EDITABLE (etext[i]), FALSE);
    gtk_widget_set_sensitive (etext[i], FALSE);
    gtk_widget_set_size_request (etext[i], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
    gtk_table_attach (GTK_TABLE (table), etext[i],
                      keylayoutx[i]+1 , keylayoutx[i]+2, keylayouty[i],
                      keylayouty[i]+1, GTK_FILL, GTK_FILL, 3, 3);
    g_signal_connect (G_OBJECT (etext[i]), "key_press_event",
                      G_CALLBACK (keypad_cb), (gpointer)i);
    g_signal_connect (G_OBJECT (key_labels[i]), "clicked",
                      G_CALLBACK (enable_entry_cb), etext[i]);
  }
  
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, GNOME_PAD);

  table = gtk_table_new (2, 3, FALSE);

  key_labels[9] = gtk_button_new_with_label (_("Teleport"));
  gtk_table_attach (GTK_TABLE (table), key_labels[9], 0, 1, 0, 1, GTK_FILL, GTK_FILL, 3, 3);
  etext[9] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[9]),
                      keyboard_string (properties.keys[9]));
  gtk_widget_set_sensitive (etext[9], FALSE);
  gtk_editable_set_editable (GTK_EDITABLE (etext[9]), FALSE);
  gtk_widget_set_size_request (etext[9], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[9], 1, 2, 0, 1, GTK_FILL, GTK_FILL, 3, 3);
  g_signal_connect (G_OBJECT (etext[9]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)9);
  g_signal_connect (G_OBJECT (key_labels[9]), "clicked",
                    G_CALLBACK (enable_entry_cb), etext[9]);
  
  key_labels[10] = gtk_button_new_with_label (_("Random Teleport"));
  gtk_table_attach (GTK_TABLE (table), key_labels[10], 0, 1, 1, 2, GTK_FILL, GTK_FILL, 3, 3);
  etext[10] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[10]),
                      keyboard_string (properties.keys[10]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[10]), FALSE);
  gtk_widget_set_sensitive (etext[10], FALSE);
  gtk_widget_set_size_request (etext[10], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[10], 1, 2, 1, 2, GTK_FILL, GTK_FILL, 3, 3);
  g_signal_connect (G_OBJECT (etext[10]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)10);
  g_signal_connect (G_OBJECT (key_labels[10]), "clicked",
                    G_CALLBACK (enable_entry_cb), etext[10]);

  key_labels[11] = gtk_button_new_with_label (_("Wait"));
  gtk_table_attach (GTK_TABLE (table), key_labels[11], 0, 1, 2, 3, GTK_FILL, GTK_FILL, 3, 3);
  etext[11] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[11]),
                      keyboard_string (properties.keys[11]));
  gtk_widget_set_sensitive (etext[11], FALSE);
  gtk_editable_set_editable (GTK_EDITABLE (etext[11]), FALSE);
  gtk_widget_set_size_request (etext[11], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[11], 1, 2, 2, 3, GTK_FILL, GTK_FILL, 3, 3);
  g_signal_connect (G_OBJECT (etext[11]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)11);
  g_signal_connect (G_OBJECT (key_labels[11]), "clicked",
                    G_CALLBACK (enable_entry_cb), etext[11]);

  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, GNOME_PAD);

  hbox = gtk_hbox_new (TRUE, GNOME_PAD);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, GNOME_PAD);

  dbut = gtk_button_new_with_label (_("Use the Keypad"));
  g_signal_connect (G_OBJECT (dbut), "clicked",
                    G_CALLBACK (defkey_cb), (gpointer)default_keys3);  
  gtk_box_pack_start (GTK_BOX (hbox), dbut, FALSE, TRUE, GNOME_PAD);

  dbut = gtk_button_new_with_label (_("Use Left Hand Keys"));
  g_signal_connect (G_OBJECT (dbut), "clicked",
                    G_CALLBACK (defkey_cb), (gpointer)default_keys2);  
  gtk_box_pack_start (GTK_BOX (hbox), dbut, FALSE, TRUE, GNOME_PAD);

  dbut = gtk_button_new_with_label (_("Use Original Robots Keys"));
  g_signal_connect (G_OBJECT (dbut), "clicked",
                    G_CALLBACK (defkey_cb), (gpointer)default_keys1);  
  gtk_box_pack_start (GTK_BOX (hbox), dbut, FALSE, TRUE, GNOME_PAD);

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
    properties.keys[i] = default_keys1[i];

    sprintf (buffer, KEY_CONTROL_KEY, i);

    str = gconf_client_get_string (get_gconf_client (), buffer, NULL);    
    if (str != NULL) {
      properties.keys[i] = gdk_keyval_from_name (str);
    }
    if ((str == NULL) || (properties.keys[i] == GDK_VoidSymbol)) {
      properties.keys[i] = default_keys3[i];
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

  name = g_strdup_printf ("#%02x%02x%02x", c->red, c->green, c->blue);
  
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
