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
gboolean init_sound(
){
  return TRUE;
}


/**
 * cleanup_sound
 *
 * Description:
 * Cleans up the sound resources 
 *
 * Returns:
 * TRUE on success FALSE otherwise
 **/
gboolean cleanup_sound(
){
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
gboolean play_sound(
gint sno
){

  if((sno < 0) || (sno >= NUM_SOUNDS)){
    return FALSE;
  }

  if(properties_sound()){

    switch(sno){
      case SOUND_VICTORY:
	gnome_triggers_do("", "program", GAME_NAME, "victory", NULL);
        break;
      case SOUND_DIE:
	gnome_triggers_do("", "program", GAME_NAME, "die", NULL);
        break;
      case SOUND_TELEPORT:
	gnome_triggers_do("", "program", GAME_NAME, "teleport", NULL);
        break;
      case SOUND_SPLAT:
	gnome_triggers_do("", "program", GAME_NAME, "splat", NULL);
        break;
      case SOUND_BAD:
      	if(gnome_sound_connection >= 0){
          gnome_triggers_do("", "program", GAME_NAME, "bad-move", NULL);
	} else {
          gdk_beep();
	}
        break;
      case SOUND_YAHOO:
	gnome_triggers_do("", "program", GAME_NAME, "yahoo", NULL);
        break;
    }

  }

  return TRUE;
}

/**********************************************************************/
