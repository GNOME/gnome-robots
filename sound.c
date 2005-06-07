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

#include "gbdefs.h"
#include "sound.h"
#include "properties.h"


/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

/**
 * init_sound
 *
 * Description:
 * Initialises game sound
 *
 * Returns:
 * TRUE on success FALSE otherwise
 **/
gboolean
init_sound (void)
{
  return TRUE;
}


/**
 * play_sound
 * @sno: sound number
 *
 * Description:
 * Plays a game sound
 *
 * Returns:
 * TRUE on success FALSE otherwise
 **/
gboolean
play_sound (gint sno)
{

  if ((sno < 0) || (sno >= NUM_SOUNDS)) {
    return FALSE;
  }

  if (properties_sound ()) {

    switch (sno) {
    case SOUND_VICTORY:
      gnome_triggers_do ("", "program", GAME_NAME, "victory", NULL);
      break;
    case SOUND_DIE:
      gnome_triggers_do ("", "program", GAME_NAME, "die", NULL);
      break;
    case SOUND_TELEPORT:
      gnome_triggers_do ("", "program", GAME_NAME, "teleport", NULL);
      break;
    case SOUND_SPLAT:
      gnome_triggers_do ("", "program", GAME_NAME, "splat", NULL);
      break;
    case SOUND_BAD:
#if 0
      if (gnome_sound_connection >= 0) {
        gnome_triggers_do ("", "program", GAME_NAME, "bad-move", NULL);
      } else {
        gdk_beep ();
      }
#endif
      gdk_beep ();
      break;
    case SOUND_YAHOO:
      gnome_triggers_do ("", "program", GAME_NAME, "yahoo", NULL);
      break;
    }

  }
  
  return TRUE;
}

/**********************************************************************/
