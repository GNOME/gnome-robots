#ifndef MENU_H
#define MENU_H


/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
gboolean create_game_menus (void);
void really_exit_cb (GtkWidget *, gpointer);
void exit_cb (GtkWidget *, gpointer);
void update_score_state (void);
void set_move_menu_sensitivity (gboolean state);
/**********************************************************************/


#endif /* MENU_H */
