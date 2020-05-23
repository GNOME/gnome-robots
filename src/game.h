#ifndef GAME_H
#define GAME_H

#include "gbdefs.h"

/**********************************************************************/
/* Game Object Structure                                              */
/**********************************************************************/
typedef struct _GameObject GameObject;

struct _GameObject {
  gint x;
  gint y;
  gint type;
  gint oldx;
  gint oldy;
  GameObject *next;
  GameObject *prev;
};
/**********************************************************************/


/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
void init_game (void);
void quit_game (void);
void game_keypress (gint);
void show_scores (void);
void start_new_game (void);
void mouse_cb (GtkGestureMultiPress *gesture,
               gint                  n_press,
               gdouble               x,
               gdouble               y,
               gpointer              user_data);
gboolean move_cb (GtkWidget * widget, GdkEventMotion * e, gpointer data);
/**********************************************************************/


/**********************************************************************/
/* Exported Variables                                                 */
/**********************************************************************/
extern gint game_state;
extern gint arena[GAME_WIDTH][GAME_HEIGHT];
/**********************************************************************/


#endif /* GAME_H */
