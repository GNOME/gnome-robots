#ifndef PROPERTIES_H
#define PROPERTIES_H


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
gboolean properties_set_config (gint);
/**********************************************************************/


#endif /* PROPERTIES_H */
