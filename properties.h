#ifndef PROPERTIES_H
#define PROPERTIES_H


/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
gboolean load_properties();
gboolean save_properties();
void     show_properties_dialog();
gboolean properties_super_safe_moves();
gboolean properties_safe_moves();
gboolean properties_sound();
gboolean properties_splats();
gboolean properties_set_config(gint);
/**********************************************************************/


#endif /* PROPERTIES_H */
