#ifndef STATUSBAR_H
#define STATUSBAR_H


/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
GtkWidget* gnobots_statusbar_new();
gboolean   gnobots_statusbar_delete();
gboolean   gnobots_statusbar_set_score(gint);
gboolean   gnobots_statusbar_set_level(gint);
gboolean   gnobots_statusbar_set_safe_teleports(gint);
gboolean   gnobots_statusbar_set_remaining(gint, gint);
gboolean   gnobots_statusbar_set(gint, gint, gint, gint, gint);
gboolean   gnobots_statusbar_reset();
gboolean   gnobots_statusbar_show_both(gboolean);
/**********************************************************************/


#endif /* STATUSBAR_H */
