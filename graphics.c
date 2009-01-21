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

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libgames-support/games-find-file.h>
#include <libgames-support/games-preimage.h>
#include <libgames-support/games-runtime.h>
#include <libgames-support/games-scores.h>
#include <libgames-support/games-scores-dialog.h>

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

static GdkGC *light_bggc = NULL;
static GdkGC *dark_bggc = NULL;

static GdkPixbuf *aieee_pixbuf = NULL;
static GdkPixbuf *yahoo_pixbuf = NULL;
static GdkPixbuf *splat_pixbuf = NULL;

static gint robot_animation = 0;
static gint player_animation = 0;
static gint player_num_waves = 0;
static gint player_wave_wait = 0;
static gint player_wave_dir = 1;

static gint bubble_xpos = 0;
static gint bubble_ypos = 0;
static gint bubble_xo = 0;
static gint bubble_yo = 0;
static gint bubble_type = BUBBLE_NONE;

gint tile_width = 0;
gint tile_height = 0;

/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/

static gboolean load_bubble_graphic (gchar *, GdkPixbuf **);
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
					14 * tile_width, tile_height);
  rerender_needed = FALSE;
}

gboolean
resize_cb (GtkWidget * w, GdkEventConfigure * e, gpointer data)
{
  gint trial_width;
  gint trial_height;

  trial_width = e->width / GAME_WIDTH;
  trial_height = e->height / GAME_HEIGHT;

  if ((trial_width != tile_width) || (trial_height != tile_height)) {
    tile_width = trial_width;
    tile_height = trial_height;
    rerender_needed = TRUE;
  }

  return FALSE;
}

gint
expose_cb (GtkWidget * w, GdkEventExpose * e, gpointer data)
{
  int i, j;
  int x1, y1, x2, y2;

  x1 = e->area.x / tile_width;
  y1 = e->area.y / tile_height;
  x2 = x1 + e->area.width / tile_width + 1;
  y2 = y1 + e->area.height / tile_height + 1;

  for (j = y1; j <= y2; j++) {
    for (i = x1; i <= x2; i++) {
      /* Draw a blank space. Animation fills the objects in. */
      draw_tile_pixmap (-1, i, j, w);
    }
  }

  return TRUE;
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
load_bubble_graphic (gchar * fname, GdkPixbuf ** pixbuf)
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
  const char *dname;

  dname = games_runtime_get_directory (GAMES_RUNTIME_GAME_PIXMAP_DIRECTORY);

  buffer = g_build_filename (dname, "yahoo.png", NULL);
  if (!load_bubble_graphic (buffer, &yahoo_pixbuf))
    return FALSE;
  g_free (buffer);

  buffer = g_build_filename (dname, "aieee.png", NULL);
  if (!load_bubble_graphic (buffer, &aieee_pixbuf))
    return FALSE;

  buffer = g_build_filename (dname, "splat.png", NULL);
  if (!load_bubble_graphic (buffer, &splat_pixbuf))
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
  gchar *filename;
  const char *pixmapdir;

  if (theme_preimage != NULL) {
    free_game_graphics ();
  }

  pixmapdir = games_runtime_get_directory (GAMES_RUNTIME_GAME_PIXMAP_DIRECTORY);
  filename = games_find_similar_file (properties_theme_name (), pixmapdir);

  theme_preimage = games_preimage_new_from_file (filename, NULL);
  g_free (filename);

  if (theme_preimage == NULL) {
    filename = games_find_similar_file ("robots", pixmapdir);
    theme_preimage = games_preimage_new_from_file (filename, NULL);
    g_free (filename);
  }

  if (!load_bubble_graphics ())
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
set_game_graphics (gchar * name)
{
  load_game_graphics ();
}

void
set_background_color (GdkColor color)
{
  GdkColormap *colormap;
  guint32 brightness;
  GdkColor color2;

  if (game_area == NULL)
    return;

  if (dark_bggc == NULL) {
    dark_bggc = gdk_gc_new (game_area->window);
    gdk_gc_copy (dark_bggc, game_area->style->black_gc);
    light_bggc = gdk_gc_new (game_area->window);
    gdk_gc_copy (light_bggc, game_area->style->white_gc);
  }

  /* While the two colours are labelled "light" and "dark" which one is
   * which actually depends on how light or dark the base colour is. */

  brightness = color.red + color.green + color.blue;
  if (brightness > 0xe8ba) {	/* 0xe8ba = 0x10000/1.1 */
    /* Darken light colours. */
    color2.red = 0.9 * color.red;
    color2.green = 0.9 * color.green;
    color2.blue = 0.9 * color.blue;
  } else if (brightness > 0xa00) {	/* Lighten darker colours. */
    color2.red = 1.1 * color.red;
    color2.green = 1.1 * color.green;
    color2.blue = 1.1 * color.blue;
  } else {			/* Very dark colours, add ratehr than multiply. */
    color2.red += 0xa00;
    color2.green += 0xa00;
    color2.blue += 0xa00;
  }

  colormap = gtk_widget_get_colormap (game_area);
  gdk_colormap_alloc_color (colormap, &color, FALSE, TRUE);
  gdk_gc_set_foreground (dark_bggc, &color);
  gdk_colormap_alloc_color (colormap, &color2, FALSE, TRUE);
  gdk_gc_set_foreground (light_bggc, &color2);

  clear_game_area ();
}

void
set_background_color_from_name (gchar * name)
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
 * @x: x position in grid squares
 * @y: y position in grid squares
 * @area: Pointer to drawing area widget
 *
 * Description:
 * Draws tile pixmap @tileno from graphics set @pno at (@x, @y) in
 * a widget @area
 **/
void
draw_tile_pixmap (gint tileno, gint x, gint y, GtkWidget * area)
{
  GdkGC *bg;

  if ((x & 1) ^ (y & 1)) {
    bg = dark_bggc;
  } else {
    bg = light_bggc;
  }

  x *= tile_width;
  y *= tile_height;

  gdk_draw_rectangle (area->window, bg, TRUE, x, y, tile_width, tile_height);

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
  if (game_area == NULL)
    return;

  switch (type) {
  case OBJECT_PLAYER:
    draw_tile_pixmap (SCENARIO_PLAYER_START + player_animation,
		      x, y, game_area);
    break;
  case OBJECT_ROBOT1:
    draw_tile_pixmap (SCENARIO_ROBOT1_START + robot_animation,
		      x, y, game_area);
    break;
  case OBJECT_ROBOT2:
    draw_tile_pixmap (SCENARIO_ROBOT2_START + robot_animation,
		      x, y, game_area);
    break;
  case OBJECT_HEAP:
    draw_tile_pixmap (SCENARIO_HEAP_POS, x, y, game_area);
    break;
  case OBJECT_NONE:
    draw_tile_pixmap (-1, x, y, game_area);
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
  if (game_area == NULL)
    return;

  gtk_widget_queue_draw (game_area);
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
  int t0i, t0j; /* (i,j) coordinates of bubble's top/left tile */
  int ntiles_hor, ntiles_ver; /* number of tiles hotizontal/vertically affected */
  int delta; /* pixels from tile's left/top border to bubble's left/top border */
  int i, j;

  if (game_area == NULL)
    return;

  t0i = bubble_xpos / tile_width;
  t0j = bubble_ypos / tile_height;
  ntiles_hor = (BUBBLE_WIDTH + tile_width - 1) / tile_width; /* first shot at number of tiles affected */
  delta = bubble_xpos % tile_width;
  if (delta > 0) { /* buble does not start at a tile's left boundary */
    if ((BUBBLE_WIDTH + delta) > ntiles_hor * tile_width) { /* catches an extra tile */
      ntiles_hor++;
    }
  }
  ntiles_ver = (BUBBLE_HEIGHT + tile_height - 1) / tile_height;
  delta = bubble_ypos % tile_height;
  if (delta > 0) { /* buble does not start at a tile's top boundary */
    if ((BUBBLE_HEIGHT + delta) > ntiles_ver * tile_height) { /* catches an extra tile */
      ntiles_ver++;
    }
  }
  for (i = t0i; i < t0i + ntiles_hor; ++i) {
    for (j = t0j; j < t0j + ntiles_ver; ++j) {
      draw_tile_pixmap (-1, i, j, game_area);
    }
  }
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
  player_wave_dir = 1;
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
  player_wave_dir = 1;
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

  if (bubble_type == BUBBLE_NONE)
    return;

  if (bubble_type == BUBBLE_YAHOO) {
    pmap = yahoo_pixbuf;
  } else if (bubble_type == BUBBLE_AIEEE) {
    pmap = aieee_pixbuf;
  } else {
    pmap = splat_pixbuf;
  }

  gdk_draw_pixbuf (game_area->window, game_area->style->black_gc, pmap,
		   bubble_xo, bubble_yo, bubble_xpos, bubble_ypos,
		   BUBBLE_WIDTH, BUBBLE_HEIGHT, GDK_RGB_DITHER_NORMAL, 0, 0);
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
  if (bubble_type == BUBBLE_NONE)
    return;

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
  if (bubble_type != BUBBLE_SPLAT)
    return;

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
