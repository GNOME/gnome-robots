#ifndef GAME_H
#define GAME_H


/**********************************************************************/
/* Game Object Structure                                              */
/**********************************************************************/
typedef struct _GameObject GameObject;

struct _GameObject {
  gint        x;
  gint        y;
  gint        type;
  gint        oldx;
  gint        oldy;
  GameObject *next;
  GameObject *prev;
};
/**********************************************************************/


/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
void init_game (void);
void cleanup_game (void);
void game_keypress (gint);
void show_scores (guint);
void start_new_game (void);
/**********************************************************************/


/**********************************************************************/
/* Exported Variables                                                 */
/**********************************************************************/
extern gint game_state;
/**********************************************************************/


#endif /* GAME_H */






