#ifndef KEYBOARD_H
#define KEYBOARD_H


/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
gchar* keyboard_string(gint);
gint   keyboard_preferred(gint);
void   keyboard_set(gint*);
gint   keyboard_cb(GtkWidget*, GdkEventKey*, gpointer);
/**********************************************************************/


#endif /* KEYBOARD_H */
