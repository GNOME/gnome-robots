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
#include <gsound.h>

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
 **/
void
play_sound (gint sno)
{

  if ((sno < 0) || (sno >= NUM_SOUNDS)) {
    return;
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
      GSoundContext *ctx;
      GError *error = NULL;

      ctx = gsound_context_new(NULL, &error);
      if (error != NULL) {
        g_warning ("Failed to create gsound context: %s", error->message);
        g_error_free (error);
        return;
      }

      filename = g_strdup_printf ("%s.ogg", name);
      path = g_build_filename (SOUND_DIRECTORY, filename, NULL);
      g_free (filename);

      gsound_context_play_simple(ctx, NULL, &error,
                                 GSOUND_ATTR_MEDIA_NAME, name,
                                 GSOUND_ATTR_MEDIA_FILENAME, path,
                                 NULL);

      if (error != NULL) {
        g_warning ("Failed to play sound \"%s\": %s", name, error->message);
        g_error_free (error);
      }

      g_object_unref (ctx);
      g_free (path);
    }
  }
}

/**********************************************************************/
