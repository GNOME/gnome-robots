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
#define DRAW_AREA_WIDTH  176
#define DRAW_AREA_HEIGHT 48
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
  gint      selected_graphics;
  gint      selected_config;
  gint      keys[12];
};
/**********************************************************************/


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static GtkWidget         *propbox      = NULL;
static GtkWidget         *darea        = NULL;
static GtkWidget         *list_view    = NULL;
enum {
  PROPERTY_STRING,
  VALUE_STRING,
  INDEX_INT,
  NCOLS
}; /* Column indices for list */

static GtkWidget         *etext[12];
static gint                timeout_id   = -1;
static gint                anim_counter = 0;
static GnobotsProperties  properties;
static GnobotsProperties  temp_prop;

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
static void copy_properties (GnobotsProperties*, GnobotsProperties*);
static gint  timeout_cb (void*);
static void remove_timeout (void);
static void add_timeout (void);
static void clear_draw_area (void);
static gint  start_anim_cb (void*);
static void apply_changes (void);
static void apply_cb (GtkWidget*, gpointer);
static void destroy_cb (GtkWidget*, gpointer);
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
/**********************************************************************/


/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

/**
 * copy_properties
 * @p1: source properties
 * @p2: destination properties
 *
 * Description:
 * copies a GnobotsProperties structure
 **/
static void
copy_properties (GnobotsProperties *p1, GnobotsProperties *p2)
{
  gint i;

  p2->safe_moves        = p1->safe_moves;
  p2->super_safe_moves  = p1->super_safe_moves;
  p2->sound             = p1->sound;
  p2->splats            = p1->splats;
  p2->selected_graphics = p1->selected_graphics;
  p2->selected_config   = p1->selected_config;

  for (i = 0; i < 12; ++i){
    p2->keys[i] = p1->keys[i];
  }
}


/**
 * timeout_cb
 * @data: callback data
 *
 * Description:
 * timer callback
 *
 * Returns:
 * TRUE if timer is to be retriggered, FALSE otherwise
 **/
static gint
timeout_cb (void *data)
{
  gint rtile = (anim_counter%4);
  gint ptile = (anim_counter%6);
  gint pno = temp_prop.selected_graphics;

  if (ptile >= 4) ptile = 6 - ptile;

  draw_tile_pixmap (SCENARIO_ROBOT1_START+rtile, pno, 16, 16, darea);
  draw_tile_pixmap (SCENARIO_HEAP_POS,           pno, 48, 16, darea);
  draw_tile_pixmap (SCENARIO_PLAYER_START+ptile, pno, 80, 16, darea);
  draw_tile_pixmap (SCENARIO_HEAP_POS,           pno, 112, 16, darea);
  draw_tile_pixmap (SCENARIO_ROBOT2_START+rtile, pno, 144, 16, darea);

  ++anim_counter;

  return TRUE;
}


/**
 * remove_timeout
 *
 * Description:
 * Removes the timer
 **/
static void
remove_timeout (void)
{
  if (timeout_id != -1){
    gtk_timeout_remove (timeout_id);
    timeout_id = -1;
  }
}


/**
 * add_timeout
 *
 * Description:
 * creates a timer for the animation
 **/
static void
add_timeout (void)
{
  if (timeout_id != -1){
    remove_timeout ();
  }

  timeout_id = gtk_timeout_add (ANIMATION_DELAY, timeout_cb, 0);
}


/**
 * clear_draw_area
 *
 * Description:
 * Clears the area for drawing the graphics
 **/
static void
clear_draw_area (void)
{
  GdkColor bgcolor = game_graphics_background (temp_prop.selected_graphics);

  gdk_window_set_background (darea->window, &bgcolor);
  gdk_window_clear_area (darea->window, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
}


/**
 * start_anim_cb
 * @data: callback data
 *
 * Description:
 * starts the animation
 *
 * Returns:
 * TRUE if the event was handled
 **/
static gint
start_anim_cb (void *data)
{
  clear_draw_area ();

  add_timeout ();

  return TRUE;
}


/**
 * apply_changes
 *
 * Description:
 * Applies the changes made by the user
 **/
static void
apply_changes (void)
{
  copy_properties (&temp_prop, &properties);

  set_game_graphics (properties.selected_graphics);
  clear_game_area ();

  set_game_config (properties.selected_config);

  keyboard_set (properties.keys);
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

  save_properties ();

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
static void
destroy_cb (GtkWidget *w, gpointer data)
{
  remove_timeout ();

  propbox = NULL;
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

  gc = game_config_settings (temp_prop.selected_config);

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
  gint num = (gint)data;

  temp_prop.selected_graphics = num;

  clear_draw_area ();
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
  gint num = (gint)data;

  temp_prop.selected_config = num;
  fill_property_list ();
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
  temp_prop.safe_moves = GTK_TOGGLE_BUTTON (widget)->active;
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
  temp_prop.super_safe_moves = GTK_TOGGLE_BUTTON (widget)->active;
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
  temp_prop.sound = GTK_TOGGLE_BUTTON (widget)->active;
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
  temp_prop.splats = GTK_TOGGLE_BUTTON (widget)->active;
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

  temp_prop.keys[knum] = keyval;

  gtk_entry_set_text (GTK_ENTRY (widget), keyboard_string (keyval));
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
    temp_prop.keys[i] = dkeys[i];
    gtk_entry_set_text (GTK_ENTRY (etext[i]), 
                        keyboard_string (temp_prop.keys[i]));    
  }

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
  GtkWidget *item;
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
    item = gtk_menu_item_new_with_label (_(game_config_name (i)));
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (type_selection), (gpointer)i);
  }

  gtk_menu_set_active (GTK_MENU (menu), temp_prop.selected_config);
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
  GtkWidget *item;
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
    item = gtk_menu_item_new_with_label (_(game_graphics_name (i)));
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (pmap_selection), (gpointer)i);
  }

  gtk_menu_set_active (GTK_MENU (menu), temp_prop.selected_graphics);

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
  GtkWidget *menu;
  GtkWidget *chkbox;
  GtkWidget *table;
  GtkWidget *dbut;
  GtkWidget *scrolled;
  GtkWidget *frame;
  GtkListStore *list;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeSelection *selection;

  if (propbox) 
    return;

  /* Copy current setting into temporary */
  copy_properties (&properties, &temp_prop);

  propbox = gtk_dialog_new_with_buttons (_("GNOME Robots Preferences"),
                                         GTK_WINDOW  (app),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
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
    /*
    label = gtk_label_new (_("Game Type:"));
    gtk_box_pack_start_defaults (GTK_BOX (hbox), label);
    */
    typemenu = gtk_option_menu_new ();
    menu = gtk_menu_new ();
    fill_typemenu (menu);
    gtk_option_menu_set_menu (GTK_OPTION_MENU (typemenu), menu);
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
    GTK_TOGGLE_BUTTON (chkbox)->active = temp_prop.safe_moves;
    g_signal_connect (G_OBJECT (chkbox), "clicked",
                      (GtkSignalFunc)safe_cb, NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), chkbox, 0, 1, 0, 1);

    chkbox = gtk_check_button_new_with_label (_("Use super safe moves"));
    GTK_TOGGLE_BUTTON (chkbox)->active = temp_prop.super_safe_moves;
    g_signal_connect (G_OBJECT (chkbox), "clicked",
                      (GtkSignalFunc)super_safe_cb, NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), chkbox, 0, 1, 1, 2);

    chkbox = gtk_check_button_new_with_label (_("Enable sounds"));
    GTK_TOGGLE_BUTTON (chkbox)->active = temp_prop.sound;
    g_signal_connect (G_OBJECT (chkbox), "clicked",
                      (GtkSignalFunc)sound_cb, NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), chkbox, 1, 2, 0, 1);

    chkbox = gtk_check_button_new_with_label (_("Enable splats"));
    GTK_TOGGLE_BUTTON (chkbox)->active = temp_prop.splats;
    g_signal_connect (G_OBJECT (chkbox), "clicked",
                      (GtkSignalFunc)splat_cb, NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), chkbox, 1, 2, 1, 2);

  } else {
    label = gtk_label_new (_("You Cannot Change the Game Type When Playing"));
    gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  }
  label = gtk_label_new_with_mnemonic (_("_Game"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), cpage, label);


  /* The graphics page */
  gpage = gtk_vbox_new (FALSE, GNOME_PAD);
  gtk_container_set_border_width (GTK_CONTAINER (gpage), GNOME_PAD_SMALL);

  frame = games_frame_new (_("Graphics Theme"));
  gtk_box_pack_start (GTK_BOX (gpage), frame, FALSE, FALSE, GNOME_PAD);
  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtk_hbox_new (TRUE, GNOME_PAD);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, GNOME_PAD);

  /*
  label = gtk_label_new (_("Image theme:"));
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);
  */
  pmapmenu = gtk_option_menu_new ();
  menu = gtk_menu_new ();
  fill_pmapmenu (menu);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (pmapmenu), menu);
  gtk_box_pack_start_defaults (GTK_BOX (hbox), pmapmenu);

  hbox = gtk_hbox_new (TRUE, GNOME_PAD);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, GNOME_PAD);
  darea = gtk_drawing_area_new ();
  gtk_widget_set_size_request (darea, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);

  gtk_box_pack_start (GTK_BOX (hbox), darea, TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (darea), "realize",
                    G_CALLBACK (start_anim_cb), NULL);

  label = gtk_label_new_with_mnemonic (_("_Appearance"));
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

  table = gtk_table_new (5, 5, TRUE);

  /* North West */
  label = gtk_label_new (_("NW"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 0, 0, 3, 3);
  etext[0] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[0]),
                      keyboard_string (properties.keys[0]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[0]), FALSE);
  gtk_widget_set_size_request (etext[0], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[0], 1, 2, 1, 2, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[0]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)0);

  /* North */
  label = gtk_label_new (_("N"));
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1, 0, 0, 3, 3);
  etext[1] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[1]),
                      keyboard_string (properties.keys[1]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[1]), FALSE);
  gtk_widget_set_size_request (etext[1], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[1], 2, 3, 1, 2, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[1]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)1);

  /* North East */
  label = gtk_label_new (_("NE"));
  gtk_table_attach (GTK_TABLE (table), label, 4, 5, 0, 1, 0, 0, 3, 3);
  etext[2] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[2]),
                      keyboard_string (properties.keys[2]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[2]), FALSE);
  gtk_widget_set_size_request (etext[2], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[2], 3, 4, 1, 2, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[2]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)2);

  /* West */
  label = gtk_label_new (_("W"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, 0, 0, 3, 3);
  etext[3] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[3]),
                      keyboard_string (properties.keys[3]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[3]), FALSE);
  gtk_widget_set_size_request (etext[3], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[3], 1, 2, 2, 3, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[3]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)3);

  etext[4] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[4]),
                      keyboard_string (properties.keys[4]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[4]), FALSE);
  gtk_widget_set_size_request (etext[4], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[4], 2, 3, 2, 3, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[4]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)4);

  /* East */
  label = gtk_label_new (_("E"));
  gtk_table_attach (GTK_TABLE (table), label, 4, 5, 2, 3, 0, 0, 3, 3);
  etext[5] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[5]),
                      keyboard_string (properties.keys[5]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[5]), FALSE);
  gtk_widget_set_size_request (etext[5], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[5], 3, 4, 2, 3, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[5]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)5);

  /* South West */
  label = gtk_label_new (_("SW"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5, 0, 0, 3, 3);
  etext[6] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[6]),
                      keyboard_string (properties.keys[6]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[6]), FALSE);
  gtk_widget_set_size_request (etext[6], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[6], 1, 2, 3, 4, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[6]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)6);

  /* South */
  label = gtk_label_new (_("S"));
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 4, 5, 0, 0, 3, 3);
  etext[7] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[7]),
                      keyboard_string (properties.keys[7]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[7]), FALSE);
  gtk_widget_set_size_request (etext[7], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[7], 2, 3, 3, 4, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[7]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)7);

  /* South East */
  label = gtk_label_new (_("SE"));
  gtk_table_attach (GTK_TABLE (table), label, 4, 5, 4, 5, 0, 0, 3, 3);
  etext[8] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[8]),
                      keyboard_string (properties.keys[8]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[8]), FALSE);
  gtk_widget_set_size_request (etext[8], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[8], 3, 4, 3, 4, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[8]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)8);

  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, GNOME_PAD);


  table = gtk_table_new (2, 5, FALSE);

  gtk_table_set_row_spacing (GTK_TABLE (table), 0, 36);

  label = gtk_label_new (_("Teleport:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 0, 0, 3, 3);
  etext[9] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[9]),
                      keyboard_string (properties.keys[9]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[9]), FALSE);
  gtk_widget_set_size_request (etext[9], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[9], 1, 2, 1, 2, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[9]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)9);

  label = gtk_label_new (_("Random Teleport:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, 0, 0, 3, 3);
  etext[10] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[10]),
                      keyboard_string (properties.keys[10]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[10]), FALSE);
  gtk_widget_set_size_request (etext[10], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[10], 1, 2, 2, 3, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[10]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)10);

  label = gtk_label_new (_("Wait:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, 0, 0, 3, 3);
  etext[11] = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (etext[11]),
                      keyboard_string (properties.keys[11]));
  gtk_editable_set_editable (GTK_EDITABLE (etext[11]), FALSE);
  gtk_widget_set_size_request (etext[11], KB_TEXT_WIDTH, KB_TEXT_HEIGHT);
  gtk_table_attach (GTK_TABLE (table), etext[11], 1, 2, 3, 4, 0, 0, 3, 3);
  g_signal_connect (G_OBJECT (etext[11]), "key_press_event",
                    G_CALLBACK (keypad_cb), (gpointer)11);

  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, GNOME_PAD);

  hbox = gtk_hbox_new (TRUE, GNOME_PAD);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, GNOME_PAD);

  dbut = gtk_button_new_with_label (_("Standard Robots Keys"));
  g_signal_connect (G_OBJECT (dbut), "clicked",
                    G_CALLBACK (defkey_cb), (gpointer)default_keys1);  
  gtk_box_pack_start (GTK_BOX (hbox), dbut, FALSE, FALSE, GNOME_PAD);

  dbut = gtk_button_new_with_label (_("Predefined Set 1"));
  g_signal_connect (G_OBJECT (dbut), "clicked",
                    G_CALLBACK (defkey_cb), (gpointer)default_keys2);  
  gtk_box_pack_start (GTK_BOX (hbox), dbut, FALSE, FALSE, GNOME_PAD);

  dbut = gtk_button_new_with_label (_("Predefined Set 2"));
  g_signal_connect (G_OBJECT (dbut), "clicked",
                    G_CALLBACK (defkey_cb), (gpointer)default_keys3);  
  gtk_box_pack_start (GTK_BOX (hbox), dbut, FALSE, FALSE, GNOME_PAD);

  label = gtk_label_new_with_mnemonic (_("_Keyboard"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), kpage, label);


  g_signal_connect  (G_OBJECT  (propbox), "destroy",
                     G_CALLBACK (destroy_cb), NULL);
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

  for (i = 0; i < 12; i++) {
    properties.keys[i] = default_keys1[i];

    sprintf (buffer, KEY_CONTROL_KEY, i);

    str = gconf_client_get_string (get_gconf_client (), buffer, NULL);    
    if (str != NULL) {
      properties.keys[i] = gdk_keyval_from_name (str);
    }
    else {
      properties.keys[i] = 0;
    }
    g_free (str);
  }

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
