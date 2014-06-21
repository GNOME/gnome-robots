#ifndef GNOBOTS_H
#define GNOBOTS_H

#include <gtk/gtk.h>

#include "games-scores.h"

/**********************************************************************/
/* Exported Variables                                                 */
/**********************************************************************/
extern GtkWidget *window;
extern GtkWidget *game_area;
extern GamesScores *highscores;
extern GSettings *settings;
/**********************************************************************/

/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
void set_move_action_sensitivity (gboolean state);
void update_game_status (gint score, gint level, gint safe_teleports);
void quit_game (void);
/**********************************************************************/

#endif /* GNOBOTS_H */
