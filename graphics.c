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

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>

#include "graphics.h"
#include "gbdefs.h"
#include "gnobots.h"


/**********************************************************************/
/* GraphicInfo Structure Definition                                   */
/**********************************************************************/

typedef struct _GraphicInfo {
  GString   *name;
  GdkPixbuf *pixbuf;
  GdkColor   bgcolor;
} GraphicInfo;

/**********************************************************************/


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static gint          num_graphics     = -1;
static gint          current_graphics = -1;
static GraphicInfo **game_graphic     = NULL;
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
  gint           i;
  G_CONST_RETURN gchar* dent;
  GDir           *dir;
  gchar          *buffer;
  gchar          *bptr;
  GdkPixbuf      *image;

  gchar *dname = gnome_program_locate_file (NULL, 
                                            GNOME_FILE_DOMAIN_APP_PIXMAP,
                                            GAME_NAME, FALSE, 
                                            NULL);

  if (game_graphic != NULL) {
    free_game_graphics ();
  }

  dir = g_dir_open (dname, 0, NULL);
  if (dir == NULL)
    return FALSE;

  num_graphics = 0;
  while ((dent = g_dir_read_name (dir)) != NULL) {
    if (! g_strrstr (dent, ".png")) {
      continue;
    }
    num_graphics++;
  }

  game_graphic = g_new (GraphicInfo*, num_graphics);
  for (i = 0; i < num_graphics; ++i) {
    game_graphic[i] = NULL;
  }

  g_dir_rewind (dir);

  num_graphics = 0;
  while ((dent = g_dir_read_name (dir)) != NULL) {
    if (! g_strrstr (dent, ".png")) {
      continue;
    }
    if (! strcmp (dent, "yahoo.png")) {
      continue;
    }
    if (! strcmp (dent, "aieee.png")) {
      continue;
    }
    if (! strcmp (dent, "splat.png")) {
      continue;
    }
    if (! strcmp (dent, "gnome-gnobots2.png")) {
      continue;
    }

    buffer = g_strdup (dent);
    bptr = g_strrstr (buffer, ".png");
    if (bptr != NULL)
      *bptr = 0;

    game_graphic[num_graphics] = g_new (GraphicInfo, 1);
    game_graphic[num_graphics]->name = g_string_new (buffer);
    g_free (buffer);

    buffer = g_build_filename (dname, dent, NULL);

    image = gdk_pixbuf_new_from_file (buffer, NULL);
    g_free (buffer);

    if (image == NULL) {
      g_free (game_graphic[num_graphics]->name);
      g_free (game_graphic[num_graphics]);
    } else {
      game_graphic[num_graphics]->pixbuf = image;
      num_graphics++;
    }
  }

  if (num_graphics == 0)
    return FALSE;

  g_dir_close (dir);

  current_graphics = 0;

  if (! load_bubble_graphics ())
    return FALSE;

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
  gint i;

  if (game_graphic == NULL) {
    return FALSE;
  }

  for (i = 0; i < num_graphics; ++i) {
    g_free (game_graphic[i]);
  }
  g_free (game_graphic);

  game_graphic = NULL;
  num_graphics = -1;
  current_graphics = -1;

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
 * num_game_graphics
 *
 * Description:
 * Returns the number of different graphics scenarios available
 *
 * Returns:
 * number of graphic types
 **/
gint
num_game_graphics (void)
{
  if (game_graphic == NULL)
    return -1;

  return num_graphics;
}


/**
 * ganme_graphics_name
 * @n: game graphics number
 *
 * Description:
 * The descriptive name of game graphics number @n
 *
 * Returns:
 * a string containing the graphics name
 **/
gchar*
game_graphics_name (gint n)
{
  if (game_graphic == NULL) 
    return NULL;

  if ((n < 0) || (n >= num_graphics))
    return NULL;

  return game_graphic[n]->name->str;
}


/**
 * game_graphics_background
 * @n: game graphics number
 *
 * Description:
 * Returns the background colour for game graphics specified by @n
 *
 * Returns:
 * background colour
 **/
GdkColor
game_graphics_background (gint n)
{
  static GdkColor nocol = {0, 0, 0, 0};

  if (game_graphic == NULL) return nocol;

  if ((n < 0) || (n >= num_graphics)) return nocol;

  return game_graphic[n]->bgcolor;
}


/**
 * current_game_graphics
 *
 * Description:
 * returns the currently selected graphics
 *
 * Returns:
 * game graphics number
 **/
gint
current_game_graphics (void)
{
  return current_graphics;
}


/**
 * set_game_graphics
 * @ng: Game graphics number
 *
 * Description:
 * Sets the game graphics to use
 **/
gint
set_game_graphics (gint ng)
{
  if ((ng < 0) || (ng >= num_graphics)) return -1;

  current_graphics = ng;

  if (game_area != NULL) {
    /*set_background_color (&game_graphic[current_graphics]->bgcolor);*/
  }
  return current_graphics;
}

void
set_background_color (GdkColor color)
{
  GdkColormap *colormap;

  if (game_area != NULL) {
    colormap = gtk_widget_get_colormap (game_area);
    gdk_colormap_alloc_color (colormap, &color, FALSE, TRUE);

    gdk_window_set_background (game_area->window, &color);
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
 * Draws tile pixmap @tileno form graphics set @pno at (@x, @y) in
 * a widget @area
 **/
void
draw_tile_pixmap (gint tileno, gint pno, gint x, gint y, GtkWidget *area)
{

  gdk_window_clear_area (area->window, x, y, TILE_WIDTH, TILE_HEIGHT);

  if ((tileno < 0) || (tileno >= SCENARIO_PIXMAP_WIDTH)) {
    /* nothing */
  } else {
    gdk_draw_pixbuf (area->window, area->style->black_gc,
                     game_graphic[pno]->pixbuf,
                     tileno * TILE_WIDTH, 0,
                     x, y, TILE_WIDTH, TILE_HEIGHT,
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
  gint xpos = x * TILE_WIDTH;
  gint ypos = y * TILE_HEIGHT;

  if ((game_area == NULL) || (game_graphic == NULL)) return;

  switch (type) {
  case OBJECT_PLAYER:
    draw_tile_pixmap (SCENARIO_PLAYER_START+player_animation, 
                      current_graphics, xpos, ypos, game_area);
    break;
  case OBJECT_ROBOT1:
    draw_tile_pixmap (SCENARIO_ROBOT1_START+robot_animation, 
                      current_graphics, xpos, ypos, game_area);
    break;
  case OBJECT_ROBOT2:
    draw_tile_pixmap (SCENARIO_ROBOT2_START+robot_animation, 
                      current_graphics, xpos, ypos, game_area);
    break;
  case OBJECT_HEAP:
    draw_tile_pixmap (SCENARIO_HEAP_POS, 
                      current_graphics, xpos, ypos, game_area);
    break;
  case OBJECT_NONE:
    draw_tile_pixmap (-1, current_graphics, xpos, ypos, game_area);
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
  if ((game_area == NULL) || (game_graphic == NULL)) return;
  
  gdk_window_clear_area (game_area->window, 0, 0, 
                         GAME_WIDTH*TILE_WIDTH, GAME_HEIGHT*TILE_HEIGHT);
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
  if ((game_area == NULL) || (game_graphic == NULL)) return;
  
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
  bubble_xpos = x * TILE_WIDTH - BUBBLE_WIDTH + BUBBLE_XOFFSET;
  bubble_ypos = y * TILE_HEIGHT - BUBBLE_HEIGHT + BUBBLE_YOFFSET;

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
