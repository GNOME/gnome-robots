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
typedef struct _GraphicInfo{
  GString   *name;
  GdkPixmap *pixmap;
  GdkColor   bgcolor;
}GraphicInfo;
/**********************************************************************/


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static gint          num_graphics     = -1;
static gint          current_graphics = -1;
static GraphicInfo **game_graphic     = NULL;
static GdkPixmap    *aieee_pixmap     = NULL;
static GdkPixmap    *yahoo_pixmap     = NULL;
static GdkPixmap    *splat_pixmap     = NULL;
static GdkPixmap    *aieee_mask       = NULL;
static GdkPixmap    *yahoo_mask       = NULL;
static GdkPixmap    *splat_mask       = NULL;

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


/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/
static gboolean load_bubble_graphic(gchar*, GdkPixmap**, GdkPixmap**);
static gboolean load_bubble_graphics();
static void clear_bubble_area();
static void add_bubble(gint, gint);
/**********************************************************************/


/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

/**
 * load_bubble_graphic
 * @fname: pixmap filename
 * @pixmap: pointer to pixmap pointer
 * @mask: pointer to pixmap mask pointer
 *
 * Description:
 * loads a 'Speech Bubble' graphic - i.e. one requiring a transparency
 * mask
 *
 * Returns:
 * TRUE on success FALSE otherwise
 **/
static gboolean load_bubble_graphic(
gchar *fname,
GdkPixmap **pixmap,
GdkPixmap **mask
){
  GdkPixbuf *image;

  if(!g_file_test (fname, G_FILE_TEST_EXISTS)){
    printf(_("Could not find \'%s\' pixmap file for Gnome Robots\n"), fname);
    return FALSE;
  }

  image = gdk_pixbuf_new_from_file(fname, NULL);
  
  gdk_pixbuf_render_pixmap_and_mask (image, pixmap, mask, 127);
  gdk_pixbuf_unref (image);
  
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
static gboolean load_bubble_graphics(
){
  gchar buffer[PATH_MAX];
  gchar *dname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,  
                                            GAME_NAME, FALSE, NULL);

  strcpy(buffer, dname);
  strcat(buffer, "/");
  strcat(buffer, "yahoo.png");
  if(!load_bubble_graphic(buffer, &yahoo_pixmap, &yahoo_mask)) return FALSE;

  strcpy(buffer, dname);
  strcat(buffer, "/");
  strcat(buffer, "aieee.png");
  if(!load_bubble_graphic(buffer, &aieee_pixmap, &aieee_mask)) return FALSE;

  strcpy(buffer, dname);
  strcat(buffer, "/");
  strcat(buffer, "splat.png");
  if(!load_bubble_graphic(buffer, &splat_pixmap, &splat_mask)) return FALSE;

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
gboolean load_game_graphics(
){
  gint             i;
  struct dirent  *dent;
  DIR            *dir;
  gchar           buffer[PATH_MAX];
  gchar          *bptr;
  GdkPixbuf      *image;
  GdkImage       *tmpimage;
  GdkPixmap      *pixmap;
  gchar          *dname = gnome_program_locate_file (NULL, 
                                                     GNOME_FILE_DOMAIN_PIXMAP,
                                                     GAME_NAME, FALSE, 
                                                     NULL);

  if(game_graphic != NULL){
    free_game_graphics();
  }

  dir = opendir(dname);
  if(!dir) return FALSE;

  num_graphics = 0;
  while((dent = readdir(dir)) != NULL){
    if(!strstr(dent->d_name, ".png")){
      continue;
    }
    num_graphics++;
  }

  game_graphic = g_new(GraphicInfo*, num_graphics);
  for(i = 0; i < num_graphics; ++i){
    game_graphic[i] = NULL;
  }

  rewinddir(dir);

  num_graphics = 0;
  while((dent = readdir(dir)) != NULL){
    if(!strstr(dent->d_name, ".png")){
      continue;
    }
    if(!strcmp(dent->d_name, "yahoo.png")){
      continue;
    }
    if(!strcmp(dent->d_name, "aieee.png")){
      continue;
    }
    if(!strcmp(dent->d_name, "splat.png")){
      continue;
    }
    if(!strcmp(dent->d_name, "gnome-gnobots2.png")){
      continue;
    }

    strcpy(buffer, dent->d_name);
    bptr = buffer;
    while(bptr){
      if(*bptr == '.'){
	*bptr = 0;
	break;
      }
      bptr++;
    }

    game_graphic[num_graphics] = g_new(GraphicInfo, 1);
    game_graphic[num_graphics]->name = g_string_new(buffer);

    
    strcpy(buffer, dname);
    strcat(buffer, "/");
    strcat(buffer, dent->d_name);

    image = gdk_pixbuf_new_from_file (buffer, NULL);
    gdk_pixbuf_render_pixmap_and_mask (image, &pixmap, NULL, 127);
    tmpimage = gdk_drawable_get_image(pixmap, 0, 0, 1, 1);
    game_graphic[num_graphics]->bgcolor.pixel = gdk_image_get_pixel(tmpimage, 0, 0);
    g_object_unref(tmpimage);
    gdk_pixbuf_unref (image);

    game_graphic[num_graphics]->pixmap = pixmap;

    num_graphics++;
  }

  closedir(dir);

  current_graphics = 0;

  if(!load_bubble_graphics()) return FALSE;

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
gboolean free_game_graphics(
){
  gint i;

  if(game_graphic == NULL){
    return FALSE;
  }

  for(i = 0; i < num_graphics; ++i){
    g_free(game_graphic[i]);
  }
  g_free(game_graphic);

  game_graphic = NULL;
  num_graphics = -1;
  current_graphics = -1;

  if(aieee_pixmap) gdk_drawable_unref(aieee_pixmap);
  aieee_pixmap = NULL;
  if(aieee_mask) gdk_drawable_unref(aieee_mask);
  aieee_mask = NULL;

  if(yahoo_pixmap) gdk_drawable_unref(yahoo_pixmap);
  yahoo_pixmap = NULL;
  if(yahoo_mask) gdk_drawable_unref(yahoo_mask);
  yahoo_mask = NULL;

  if(splat_pixmap) gdk_drawable_unref(splat_pixmap);
  splat_pixmap = NULL;
  if(splat_mask) gdk_drawable_unref(splat_mask);
  splat_mask = NULL;

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
gint num_game_graphics(
){
  if(game_graphic == NULL) return -1;

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
gchar* game_graphics_name(
gint n
){
  if(game_graphic == NULL) return NULL;

  if((n < 0) || (n >= num_graphics)) return NULL;

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
GdkColor game_graphics_background(
gint n
){
  static GdkColor nocol = {0, 0, 0, 0};

  if(game_graphic == NULL) return nocol;

  if((n < 0) || (n >= num_graphics)) return nocol;

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
gint current_game_graphics(
){
  return current_graphics;
}


/**
 * set_game_graphics
 * @ng: Game graphics number
 *
 * Description:
 * Sets the game graphics to use
 **/
gint set_game_graphics(
gint ng
){
  if((ng < 0) || (ng >= num_graphics)) return -1;

  current_graphics = ng;

  if(game_area != NULL){
    gdk_window_set_background(game_area->window, &game_graphic[current_graphics]->bgcolor);
  }

  return current_graphics;
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
void draw_tile_pixmap(
gint         tileno,
gint         pno,
gint         x,
gint         y,
GtkWidget  *area
){
  if((tileno < 0) || (tileno >= SCENARIO_PIXMAP_WIDTH)){
    gdk_window_clear_area (area->window, x, y, TILE_WIDTH, TILE_HEIGHT);
  } else {
    gdk_draw_drawable(area->window, area->style->black_gc,
		    game_graphic[pno]->pixmap, tileno*TILE_WIDTH, 
		    0, x, y, TILE_WIDTH, TILE_HEIGHT);
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
void draw_object(
gint x,
gint y,
gint type
){
  gint xpos = x * TILE_WIDTH;
  gint ypos = y * TILE_HEIGHT;

  if((game_area == NULL) || (game_graphic == NULL)) return;

  switch(type){
    case OBJECT_PLAYER:
      draw_tile_pixmap(SCENARIO_PLAYER_START+player_animation, 
		       current_graphics, xpos, ypos, game_area);
      break;
    case OBJECT_ROBOT1:
      draw_tile_pixmap(SCENARIO_ROBOT1_START+robot_animation, 
		       current_graphics, xpos, ypos, game_area);
      break;
    case OBJECT_ROBOT2:
      draw_tile_pixmap(SCENARIO_ROBOT2_START+robot_animation, 
		       current_graphics, xpos, ypos, game_area);
      break;
    case OBJECT_HEAP:
      draw_tile_pixmap(SCENARIO_HEAP_POS, 
		       current_graphics, xpos, ypos, game_area);
      break;
    case OBJECT_NONE:
      draw_tile_pixmap(-1, current_graphics, xpos, ypos, game_area);
     break;
  }
}


/**
 * clear_game_area
 *
 * Description:
 * clears the whole of the game area
 **/
void clear_game_area(
){
  if((game_area == NULL) || (game_graphic == NULL)) return;
  
  gdk_window_clear_area(game_area->window, 0, 0, 
			GAME_WIDTH*TILE_WIDTH, GAME_HEIGHT*TILE_HEIGHT);
}


/**
 * clear_bubble_area
 *
 * Description:
 * clears the area underneath a bubble
 **/
static void clear_bubble_area(
){
  if((game_area == NULL) || (game_graphic == NULL)) return;
  
  gdk_window_clear_area(game_area->window, bubble_xpos, bubble_ypos, 
			BUBBLE_WIDTH, BUBBLE_HEIGHT);
}


/**
 * reset_player_animation
 *
 * Description:
 * resets player animation to standing position
 **/
void reset_player_animation(
){
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
void player_animation_dead(
){
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
void animate_game_graphics(
){
  ++robot_animation;
  if(robot_animation >= NUM_ROBOT_ANIMATIONS){
    robot_animation = 0;
  }

  if(player_animation == NUM_PLAYER_ANIMATIONS){
    /* do nothing */
  } else if(player_wave_wait < PLAYER_WAVE_WAIT){
    ++player_wave_wait;
    player_animation = 0;
  } else {
    player_animation += player_wave_dir;
    if(player_animation >= NUM_PLAYER_ANIMATIONS){
      player_wave_dir = -1;
      player_animation -= 2;
    } else if(player_animation < 0){
      player_wave_dir = 1;
      player_animation = 1;
      ++player_num_waves;
      if(player_num_waves >= PLAYER_NUM_WAVES){
	reset_player_animation();
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
void draw_bubble(
){
  GdkPixmap *pmap;
  GdkPixmap *mask;

  if(bubble_type == BUBBLE_NONE) return;

  if(bubble_type == BUBBLE_YAHOO){
    pmap = yahoo_pixmap;
    mask = yahoo_mask;
  } else if(bubble_type == BUBBLE_AIEEE){
    pmap = aieee_pixmap;
    mask = aieee_mask;
  } else {
    pmap = splat_pixmap;
    mask = splat_mask;
  }

  gdk_gc_set_clip_origin(game_area->style->black_gc, 
			 bubble_xpos-bubble_xo, bubble_ypos-bubble_yo);
  gdk_gc_set_clip_mask(game_area->style->black_gc, mask);
  gdk_draw_drawable(game_area->window, game_area->style->black_gc,
		  pmap, bubble_xo, bubble_yo, bubble_xpos, bubble_ypos, 
		  BUBBLE_WIDTH, BUBBLE_HEIGHT);
  gdk_gc_set_clip_mask(game_area->style->black_gc, NULL);
}


/**
 * add_bubble
 * @x: x position 
 * @y: y position 
 *
 * Description:
 * adds a bubble at @x,@y
 **/
static void add_bubble(
gint x,
gint y
){
  bubble_xpos = x * TILE_WIDTH - BUBBLE_WIDTH + BUBBLE_XOFFSET;
  bubble_ypos = y * TILE_HEIGHT - BUBBLE_HEIGHT + BUBBLE_YOFFSET;

  bubble_xo = 0;
  bubble_yo = 0;

  if(bubble_ypos < 0){
    bubble_yo = BUBBLE_HEIGHT;
    bubble_ypos += BUBBLE_HEIGHT;
  }
  if(bubble_xpos < 0){
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
void remove_bubble(
){
  if(bubble_type == BUBBLE_NONE) return;

  clear_bubble_area();
  bubble_type = BUBBLE_NONE;
}


/**
 * remove_splat_bubble
 *
 * Description:
 * removes a splat bubble if there is one
 **/
void remove_splat_bubble(
){
  if(bubble_type != BUBBLE_SPLAT) return;

  clear_bubble_area();
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
void add_yahoo_bubble(
gint x,
gint y
){
  remove_bubble();
  add_bubble(x, y);
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
void add_aieee_bubble(
gint x,
gint y
){
  remove_bubble();
  add_bubble(x, y);
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
void add_splat_bubble(
gint x,
gint y
){
  remove_bubble();
  add_bubble(x, y);

  bubble_ypos += BUBBLE_YOFFSET;
  
  bubble_type = BUBBLE_SPLAT;
}

/**********************************************************************/
