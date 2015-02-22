#ifndef GNOBOTS_H
#define GNOBOTS_H

#include <games-support.h>
#include <gtk/gtk.h>

/**********************************************************************/
/* Exported Variables                                                 */
/**********************************************************************/
extern GtkWidget *window;
extern GtkWidget *game_area;
extern GamesScoresContext *highscores;
extern GSettings *settings;
/**********************************************************************/

/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
void set_move_action_sensitivity (gboolean state);
void update_game_status (gint score, gint level, gint safe_teleports);
void quit_game (void);
const gchar* category_name_from_key (const gchar *);
/**********************************************************************/

#endif /* GNOBOTS_H */
