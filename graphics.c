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
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <games-preimage.h>
#include <games-find-file.h>

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>

#include "graphics.h"
#include "gbdefs.h"
#include "gnobots.h"
#include "properties.h"

/**********************************************************************/


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static GamesPreimage *theme_preimage = NULL;
static GdkPixbuf *theme_pixbuf = NULL;
static gboolean rerender_needed = TRUE;

static GdkPixbuf    *aieee_pixbuf     = NULL;
static GdkPixbuf    *yahoo_pixbuf     = NULL;
static GdkPixbuf    *splat_pixbuf     = NULL;

static gint          robot_animation  = 0;
static gint          player_animation = 0;
static gint          player_num_waves = 0;
static gint          player_wave_wait = 0;
static gint          player_wave_dir  = 1;

static gint          bubble_xpos = 0;
static gint          bubble_ypos = 0;
static gint          bubble_xo   = 0;
static gint          bubble_yo   = 0;
static gint          bubble_type = BUBBLE_NONE;

gint tile_width = 0;
gint tile_height = 0;

/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/

static gboolean load_bubble_graphic (gchar*, GdkPixbuf**);
static gboolean load_bubble_graphics (void);
static void clear_bubble_area (void);
static void add_bubble (gint, gint);


/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

static void
render_graphics (void)
{
  theme_pixbuf = games_preimage_render (theme_preimage,
					14*tile_width, tile_height, 
					NULL);
  rerender_needed = FALSE;
}

gint resize_cb (GtkWidget *w, GdkEventConfigure *e, gpointer data)
{
  gint trial_width;
  gint trial_height;

  trial_width = e->width/GAME_WIDTH;
  trial_height = e->height/GAME_HEIGHT;

  if ((trial_width != tile_width) || (trial_height != tile_height)) {
    tile_width = trial_width;
    tile_height = trial_height;
    rerender_needed = TRUE;
  }

  return FALSE;
}

/**
 * load_bubble_graphic
 * @fname: pixbuf filename
 * @pixbuf: pointer to pixbuf pointer
 *
 * Description:
 * loads a 'Speech Bubble' graphic - i.e. one requiring a transparency
 * mask
 *
 * Returns:
 * TRUE on success FALSE otherwise
 **/
static gboolean
load_bubble_graphic (gchar *fname, GdkPixbuf **pixbuf)
{

  if (!g_file_test (fname, G_FILE_TEST_EXISTS)) {
    printf (_("Could not find \'%s\' pixmap file\n"), fname);
    return FALSE;
  }

  *pixbuf = gdk_pixbuf_new_from_file (fname, NULL);
  
  return TRUE;
}


/**
 * load_bubble_graphics
 *
 * Description:
 * Loads all of the 'speech bubble' graphics
 *
 * Returns:
 * TRUE on success FALSE otherwise
 **/
static gboolean
load_bubble_graphics (void)
{
  gchar *buffer = NULL;
  gchar *dname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_PIXMAP,
                                            GAME_NAME, FALSE, NULL);

  buffer = g_build_filename (dname, "yahoo.png", NULL);
  if (! load_bubble_graphic (buffer, &yahoo_pixbuf))
    return FALSE;
  g_free (buffer);

  buffer = g_build_filename (dname, "aieee.png", NULL);
  if (! load_bubble_graphic (buffer, &aieee_pixbuf))
    return FALSE;

  buffer = g_build_filename (dname, "splat.png", NULL);
  if (! load_bubble_graphic (buffer, &splat_pixbuf))
    return FALSE;

  return TRUE;
}


/**
 * load_game_graphics
 *
 * Description:
 * Loads all of the game graphics
 *
 * Returns:
 * TRUE on success FALSE otherwise
 **/
gboolean
load_game_graphics (void)
{
  gchar          *filename;

  if (theme_preimage != NULL) {
    free_game_graphics ();
  }

  filename = games_find_similar_file (properties_theme_name (), PIXMAPDIR);

  theme_preimage = games_preimage_new_from_file (filename, NULL);
  g_free (filename);

  if (theme_preimage == NULL) {
    filename = games_find_similar_file ("robots", PIXMAPDIR);
    theme_preimage = games_preimage_new_from_file (filename, NULL);
    g_free (filename);
  }

  if (! load_bubble_graphics ())
    return FALSE;

  rerender_needed = TRUE;

  return TRUE;
}


/**
 * free_game_graphics
 *
 * Description:
 * Frees all of the resources used by the game graphics
 *
 * Returns:
 * TRUE on success FALSE otherwise
 **/
gboolean
free_game_graphics (void)
{
  if (theme_preimage != NULL) {
    g_object_unref (theme_preimage);
    theme_preimage = NULL;
  }

  if (theme_pixbuf != NULL) {
    g_object_unref (theme_pixbuf);
    theme_pixbuf = NULL;
  }

  if (aieee_pixbuf) 
    g_object_unref (aieee_pixbuf);
  aieee_pixbuf = NULL;

  if (yahoo_pixbuf)
    g_object_unref (yahoo_pixbuf);
  yahoo_pixbuf = NULL;

  if (splat_pixbuf)
    g_object_unref (splat_pixbuf);
  splat_pixbuf = NULL;

  return TRUE;
}

/**
 * set_game_graphics
 * @ng: Game graphics number
 *
 * Description:
 * Sets the game graphics to use
 **/
void
set_game_graphics (gchar *name)
{
  load_game_graphics ();
}

void
set_background_color (GdkColor color)
{
  GdkColormap *colormap;

  if (game_area != NULL) {
    colormap = gtk_widget_get_colormap (game_area);
    gdk_colormap_alloc_color (colormap, &color, FALSE, TRUE);

    gtk_widget_modify_bg (game_area, GTK_STATE_NORMAL, &color);
  }
}

void
set_background_color_from_name (gchar *name)
{
  GdkColor color;

  if (name == NULL)
    return;
    
  if (!gdk_color_parse (name, &color)) {
    gdk_color_parse ("#7590AE", &color);
  }
  set_background_color (color);
}

/**
 * draw_tile_pixmap
 * @tileno: Graphics tile number
 * @pno: Number of graphics set
 * @x: x position in pixels
 * @y: y position in pixels
 * @area: Pointer to drawing area widget
 *
 * Description:
 * Draws tile pixmap @tileno from graphics set @pno at (@x, @y) in
 * a widget @area
 **/
void
draw_tile_pixmap (gint tileno, gint x, gint y, GtkWidget *area)
{
  gdk_window_clear_area (area->window, x, y, tile_width, tile_height);

  if (rerender_needed)
    render_graphics ();

  if ((tileno < 0) || (tileno >= SCENARIO_PIXMAP_WIDTH)) {
    /* nothing */
  } else {
    gdk_draw_pixbuf (area->window, area->style->black_gc,
                     theme_pixbuf,
                     tileno * tile_width, 0,
                     x, y, tile_width, tile_height,
                     GDK_RGB_DITHER_NORMAL, 0, 0);
  }
}


/**
 * draw_object
 * @x: x position 
 * @y: y position 
 * @type: object type
 *
 * Description:
 * Draws graphics for an object at specified location
 **/
void
draw_object (gint x, gint y, gint type)
{
  gint xpos = x * tile_width;
  gint ypos = y * tile_height;

  if (game_area == NULL) return;

  switch (type) {
  case OBJECT_PLAYER:
    draw_tile_pixmap (SCENARIO_PLAYER_START+player_animation, 
                      xpos, ypos, game_area);
    break;
  case OBJECT_ROBOT1:
    draw_tile_pixmap (SCENARIO_ROBOT1_START+robot_animation, 
                      xpos, ypos, game_area);
    break;
  case OBJECT_ROBOT2:
    draw_tile_pixmap (SCENARIO_ROBOT2_START+robot_animation, 
                      xpos, ypos, game_area);
    break;
  case OBJECT_HEAP:
    draw_tile_pixmap (SCENARIO_HEAP_POS, 
                      xpos, ypos, game_area);
    break;
  case OBJECT_NONE:
    draw_tile_pixmap (-1, xpos, ypos, game_area);
    break;
  }
}


/**
 * clear_game_area
 *
 * Description:
 * clears the whole of the game area
 **/
void
clear_game_area (void)
{
  if (game_area == NULL) return;
  
  gdk_window_clear_area (game_area->window, 0, 0, 
                         GAME_WIDTH*tile_width, GAME_HEIGHT*tile_height);
}


/**
 * clear_bubble_area
 *
 * Description:
 * clears the area underneath a bubble
 **/
static void
clear_bubble_area (void)
{
  if (game_area == NULL) return;
  
  gdk_window_clear_area (game_area->window, bubble_xpos, bubble_ypos, 
                         BUBBLE_WIDTH, BUBBLE_HEIGHT);
}


/**
 * reset_player_animation
 *
 * Description:
 * resets player animation to standing position
 **/
void
reset_player_animation (void)
{
  player_wave_wait = 0;
  player_num_waves = 0;
  player_wave_dir  = 1;
  player_animation = 0;
}


/**
 * player_animation_dead
 *
 * Description:
 * sets player animation to be dead
 **/
void
player_animation_dead (void)
{
  player_wave_wait = 0;
  player_num_waves = 0;
  player_wave_dir  = 1;
  player_animation = NUM_PLAYER_ANIMATIONS;
}


/**
 * animate_game_graphics
 *
 * Description:
 * updates animation for object graphics
 **/
void
animate_game_graphics (void)
{
  ++robot_animation;
  if (robot_animation >= NUM_ROBOT_ANIMATIONS) {
    robot_animation = 0;
  }

  if (player_animation == NUM_PLAYER_ANIMATIONS) {
    /* do nothing */
  } else if (player_wave_wait < PLAYER_WAVE_WAIT) {
    ++player_wave_wait;
    player_animation = 0;
  } else {
    player_animation += player_wave_dir;
    if (player_animation >= NUM_PLAYER_ANIMATIONS) {
      player_wave_dir = -1;
      player_animation -= 2;
    } else if (player_animation < 0) {
      player_wave_dir = 1;
      player_animation = 1;
      ++player_num_waves;
      if (player_num_waves >= PLAYER_NUM_WAVES) {
	reset_player_animation ();
      }
    }
  }
}


/**
 * draw_bubble
 *
 * Description:
 * Draws a bubble if there is one
 **/
void
draw_bubble (void)
{
  GdkPixbuf *pmap;

  if (bubble_type == BUBBLE_NONE) return;

  if (bubble_type == BUBBLE_YAHOO) {
    pmap = yahoo_pixbuf;
  } else if (bubble_type == BUBBLE_AIEEE) {
    pmap = aieee_pixbuf;
  } else {
    pmap = splat_pixbuf;
  }

  gdk_draw_pixbuf (game_area->window, game_area->style->black_gc, pmap,
                   bubble_xo, bubble_yo, bubble_xpos, bubble_ypos, 
                   BUBBLE_WIDTH, BUBBLE_HEIGHT,
                   GDK_RGB_DITHER_NORMAL, 0, 0);
}


/**
 * add_bubble
 * @x: x position 
 * @y: y position 
 *
 * Description:
 * adds a bubble at @x,@y
 **/
static void
add_bubble (gint x, gint y)
{
  bubble_xpos = x * tile_width - BUBBLE_WIDTH + BUBBLE_XOFFSET;
  bubble_ypos = y * tile_height - BUBBLE_HEIGHT + BUBBLE_YOFFSET;

  bubble_xo = 0;
  bubble_yo = 0;

  if (bubble_ypos < 0) {
    bubble_yo = BUBBLE_HEIGHT;
    bubble_ypos += BUBBLE_HEIGHT;
  }
  if (bubble_xpos < 0) {
    bubble_xo = BUBBLE_WIDTH;
    bubble_xpos += BUBBLE_WIDTH;
  }

}


/**
 * remove_bubble
 *
 * Description:
 * removes all types of bubble
 **/
void
remove_bubble (void)
{
  if (bubble_type == BUBBLE_NONE) return;

  clear_bubble_area ();
  bubble_type = BUBBLE_NONE;
}


/**
 * remove_splat_bubble
 *
 * Description:
 * removes a splat bubble if there is one
 **/
void
remove_splat_bubble (void)
{
  if (bubble_type != BUBBLE_SPLAT) return;

  clear_bubble_area ();
  bubble_type = BUBBLE_NONE;
}


/**
 * add_yahoo_bubble
 * @x: x position 
 * @y: y position 
 *
 * Description:
 * adds and "Yahoo" bubble at @x,@y
 **/
void
add_yahoo_bubble (gint x, gint y)
{
  remove_bubble ();
  add_bubble (x, y);
  bubble_type = BUBBLE_YAHOO;
}


/**
 * add_aieee_bubble
 * @x: x position 
 * @y: y position 
 *
 * Description:
 * adds and "Aieee" bubble at @x,@y
 **/
void
add_aieee_bubble (gint x, gint y)
{
  remove_bubble ();
  add_bubble (x, y);
  bubble_type = BUBBLE_AIEEE;
}

/**
 * add_splat_bubble
 * @x: x position 
 * @y: y position 
 *
 * Description:
 * adds a "Splat" speech bubble at @x,@y
 **/
void
add_splat_bubble (gint x, gint y)
{
  remove_bubble ();
  add_bubble (x, y);

  bubble_ypos += BUBBLE_YOFFSET;
  
  bubble_type = BUBBLE_SPLAT;
}

/**********************************************************************/
