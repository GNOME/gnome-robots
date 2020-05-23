#ifndef KEYBOARD_H
#define KEYBOARD_H


/**********************************************************************/
/* Exported functions                                                 */
/**********************************************************************/
void keyboard_set (guint *);
gboolean keyboard_cb (GtkEventControllerKey *controller,
                      guint                  keyval,
                      guint                  keycode,
                      GdkModifierType        state,
                      gpointer               user_data);
/**********************************************************************/


#endif /* KEYBOARD_H */
