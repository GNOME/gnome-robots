#include <config.h>
#include <gnome.h>
#include <gdk/gdkkeysyms.h>

#include "keyboard.h"
#include "keylabels.h"


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static gint control_keys[12];
/**********************************************************************/


/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

/**
 * keyboard_string
 * @ksym: KeySym
 *
 * Description:
 * Returns the text description of a keysym
 *
 * Returns:
 * String representation of a keysym
 **/
gchar* keyboard_string(
gint ksym
){
  gint i;

  for(i = 0; i < KB_MAP_SIZE; ++i){
    if(ksym == kb_map[i].ksym){
      return kb_map[i].str;
    }
  }

  return "UNK";
}


/**
 * keyboard_preferred
 * @ksym: KeySym
 *
 * Description:
 * Returns the preferred alternative keysym. e.g. converts
 * all lovercase letters to uppercase
 *
 * Returns:
 * preferred keysym
 **/
gint keyboard_preferred(
gint ksym
){
  if(ksym == GDK_a) return GDK_A;
  if(ksym == GDK_b) return GDK_B;
  if(ksym == GDK_c) return GDK_C;
  if(ksym == GDK_d) return GDK_D;
  if(ksym == GDK_e) return GDK_E;
  if(ksym == GDK_f) return GDK_F;
  if(ksym == GDK_g) return GDK_G;
  if(ksym == GDK_h) return GDK_H;
  if(ksym == GDK_i) return GDK_I;
  if(ksym == GDK_j) return GDK_J;
  if(ksym == GDK_k) return GDK_K;
  if(ksym == GDK_l) return GDK_L;
  if(ksym == GDK_m) return GDK_M;
  if(ksym == GDK_n) return GDK_N;
  if(ksym == GDK_o) return GDK_O;
  if(ksym == GDK_p) return GDK_P;
  if(ksym == GDK_q) return GDK_Q;
  if(ksym == GDK_r) return GDK_R;
  if(ksym == GDK_s) return GDK_S;
  if(ksym == GDK_t) return GDK_T;
  if(ksym == GDK_u) return GDK_U;
  if(ksym == GDK_v) return GDK_V;
  if(ksym == GDK_w) return GDK_W;
  if(ksym == GDK_x) return GDK_X;
  if(ksym == GDK_y) return GDK_Y;
  if(ksym == GDK_z) return GDK_Z;

  return ksym;
}


/**
 * keyboard_set
 * @keys: array of keysyms
 *
 * Description:
 * sets the keybaord mapping
 **/
void keyboard_set(
gint* keys
){
  gint i;

  if(keys == NULL) return;

  for(i = 0; i < 12; ++i){
    control_keys[i] = keys[i];
  }

}


/**
 * keyboard_cb
 * @widget: widget
 * @event: event
 * @data: callback data
 *
 * Description:
 * handles keyboard events
 *
 * Returns:
 * TRUE if the event is handled
 **/
gint keyboard_cb(
GtkWidget   *widget, 
GdkEventKey *event, 
gpointer     data
){
  gint i, kv;

  /* What is this for?  This causes problems for those people whose Numlock is
   * a modifier.
  if(event->state &= GDK_MODIFIER_MASK) return FALSE;
  */

  kv = keyboard_preferred(event->keyval);

  for(i = 0; i < 12; ++i){
    if(kv == control_keys[i]){
      game_keypress(i);
      break;
    }
  }

  return TRUE;
}

/**********************************************************************/
