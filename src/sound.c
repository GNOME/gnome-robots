/*
 * Gnome Robots II
 * written by Mark Rae <m.rae@inpharmatica.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * For more details see the file COPYING.
 */

#include <config.h>

#include <gdk/gdk.h>
#include <canberra-gtk.h>

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
    gchar *name = NULL;

    switch (sno) {
    case SOUND_VICTORY:
      name = "victory";
      break;
    case SOUND_DIE:
      name = "die";
      break;
    case SOUND_TELEPORT:
      name = "teleport";
      break;
    case SOUND_SPLAT:
      name = "splat";
      break;
    case SOUND_BAD:
      gdk_beep ();
      break;
    case SOUND_YAHOO:
      name = "yahoo";
      break;
    }

    if (name)
    {
      gchar *filename, *path;

      filename = g_strdup_printf ("%s.ogg", name);
      path = g_build_filename (SOUND_DIRECTORY, filename, NULL);
      g_free (filename);

      ca_context_play (ca_gtk_context_get_for_screen (gdk_screen_get_default ()),
                      0,
                      CA_PROP_MEDIA_NAME, name,
                      CA_PROP_MEDIA_FILENAME, path, NULL);
      g_free (path);
    }
  }

  return TRUE;
}

/**********************************************************************/
