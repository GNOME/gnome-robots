#ifndef GRAPHICS_H
#define GRAPHICS_H


/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
gboolean  load_game_graphics();
gboolean  free_game_graphics();
gint      num_game_graphics();
gchar*    game_graphics_name(gint);
GdkColor  game_graphics_background(gint);
gint      current_game_graphics();
gint      set_game_graphics(gint);

void      add_yahoo_bubble(gint, gint);
void      add_aiiee_bubble(gint, gint);
void      add_splat_bubble(gint, gint);
void      remove_bubble();
void      remove_splat_bubble();
void      draw_bubble();

void      draw_tile_pixmap(gint, gint, gint, gint, GtkWidget*);
void      draw_object(gint, gint, gint);
void      clear_game_area();
void      animate_game_graphics();
void      reset_player_animation();
void      player_animation_dead();
/**********************************************************************/


#endif /* GRAPHICS_H */
