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


#endif /* GNOBOTS_H */
