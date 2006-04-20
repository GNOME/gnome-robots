#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <gconf/gconf-client.h>

/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
gboolean load_properties (void);
gboolean save_properties (void);
void     show_properties_dialog (void);
gboolean properties_super_safe_moves (void);
gboolean properties_safe_moves (void);
gboolean properties_sound (void);
gboolean properties_splats (void);
gboolean properties_show_toolbar (void);
gboolean properties_set_config (gint);

GConfClient *get_gconf_client (void);
void initialize_gconf (int argc, char *argv[]);
void gconf_set_theme (gchar *value);
void gconf_set_configuration (gchar *value);
void gconf_set_control_key (gint i, gchar *value);
void gconf_set_enable_sound (gboolean value);
void gconf_set_enable_splats (gboolean value);
void gconf_set_use_safe_moves (gboolean value);
void gconf_set_use_super_safe_moves (gboolean value);
void gconf_set_show_toolbar (gboolean value);

gint save_window_geometry (GtkWidget *w, GdkEventConfigure *e, gpointer data);
void set_window_geometry (GtkWidget *window);

/**********************************************************************/


#endif /* PROPERTIES_H */
