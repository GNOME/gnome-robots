/*
 * Gnome Robots II
 * written by Mark Rae <m.rae@inpharmatica.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */

#include <config.h>
#include <gnome.h>
#include <gdk/gdkkeysyms.h>

#include "keyboard.h"
#include "keylabels.h"
#include "game.h"

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
gchar*
keyboard_string (gint ksym)
{
  gchar *name;

  name = gdk_keyval_name (ksym);

  return name;
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
gint
keyboard_preferred (gint ksym)
{
  return gdk_keyval_to_upper (ksym);
}


/**
 * keyboard_set
 * @keys: array of keysyms
 *
 * Description:
 * sets the keybaord mapping
 **/
void
keyboard_set (gint* keys)
{
  gint i;

  if (keys == NULL)
    return;

  for (i = 0; i < 12; ++i) {
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
gint
keyboard_cb (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  gint i, kv;

  /* What is this for?  This causes problems for those people whose Numlock is
   * a modifier.
  if(event->state &= GDK_MODIFIER_MASK) return FALSE;
  */

  kv = keyboard_preferred (event->keyval);

  for (i = 0; i < 12; ++i) {
    if (kv == control_keys[i]) {
      game_keypress (i);
      return TRUE;
    }
  }

  return FALSE;
}

/**********************************************************************/
