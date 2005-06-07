#ifndef MENU_H
#define MENU_H


/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
void create_game_menus (GtkUIManager *);
void quit_cb (GtkAction *, gpointer);
void update_score_state (void);
void set_move_menu_sensitivity (gboolean state);
void connect_handle_box_to_toolbar_toggle (GtkWidget *);
/**********************************************************************/


#endif /* MENU_H */
