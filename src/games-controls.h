/*
 * games-controls.h: keyboard controls utility functions. 
 *
 * Copyright © 2004 Paolo Borelli
 *
 */

#ifndef __GAMES_CONTROLS_H__
#define __GAMES_CONTROLS_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GAMES_TYPE_CONTROLS_LIST            (games_controls_list_get_type ())
#define GAMES_CONTROLS_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAMES_TYPE_CONTROLS_LIST, GamesControlsList))
#define GAMES_CONTROLS_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GAMES_TYPE_CONTROLS_LIST, GamesControlsListClass))
#define GAMES_IS_CONTROLS_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAMES_TYPE_CONTROLS_LIST))
#define GAMES_IS_CONTROLS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GAMES_TYPE_CONTROLS_LIST))
#define GAMES_CONTROLS_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GAMES_TYPE_CONTROLS_LIST, GamesControlsListClass))

typedef struct GamesControlsListPrivate GamesControlsListPrivate;

typedef struct {
  GtkScrolledWindow parent_instance;
  /*< private >*/
  GamesControlsListPrivate *priv;
} GamesControlsList;

typedef struct {
  GtkScrolledWindowClass parent_class;
} GamesControlsListClass;

GType      games_controls_list_get_type     (void);
GtkWidget *games_controls_list_new          (GSettings *settings);
void       games_controls_list_add_control  (GamesControlsList *list,
                                             const char *conf_key,
                                             const char *label,
                                             guint default_keyval);
void       games_controls_list_add_controls (GamesControlsList *list,
                                             const char *first_conf_key,
                                             ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS
#endif /* __GAMES_CONTROLS_H__ */
