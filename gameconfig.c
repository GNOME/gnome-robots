#include <config.h>
#include <gnome.h>

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>

#include "gameconfig.h"
#include "gbdefs.h"


/**********************************************************************/
/* File Static Variables                                              */
/**********************************************************************/
static gint         num_configs    = -1;
static GameConfig **game_configs   = NULL;
static gint         current_config = -1;
/**********************************************************************/


/**********************************************************************/
/* Function Prototypes                                                */
/**********************************************************************/
static GameConfig* load_config(gchar*);
/**********************************************************************/


/**********************************************************************/
/* Function Definitions                                               */
/**********************************************************************/

/**
 * load_config
 * @fname: config file name
 *
 * Description:
 * loads a configuraion from a file
 *
 * Returns:
 * a pointer to a GameConfig structure or NULL on failure
 **/
static GameConfig* load_config(
gchar *fname
){
  GameConfig *gcfg;
  gint        pflag = 0;
  FILE       *fp;
  gchar       buffer[PATH_MAX];
  gchar      *bptr;
  gchar      *bpstart;
  gchar      *vptr;
  gint        val;

  fp = fopen(fname, "r");
  if(fp == NULL) return NULL;

  gcfg = g_new(GameConfig, 1);
  strcpy(buffer, fname);
  bptr = bpstart = buffer;
  while(*bptr){
    if(*bptr == '.'){
      *bptr = 0;
      break;
    }
    if(*bptr == '/'){
      bpstart = bptr+1;
    }
    ++bptr;
  }
  gcfg->description = g_string_new(bpstart);
  
  while(fgets(buffer, 256, fp) != NULL){
    if(strlen(buffer) < 3) continue;

    bptr = buffer;
    vptr = NULL;
    while(bptr){
      if(*bptr == '='){
	*bptr = 0;
	vptr = ++bptr;
	break;
      }
      bptr++;
    }

    if(!vptr) continue;
    if(sscanf(vptr, "%d", &val) != 1) continue;

    if(!strcmp(buffer, "initial_type1")){
      gcfg->initial_type1 = val;
      pflag |= 0x00000001;
    }
    if(!strcmp(buffer, "initial_type2")){
      gcfg->initial_type2 = val;
      pflag |= 0x00000002;
    }
    if(!strcmp(buffer, "increment_type1")){
      gcfg->increment_type1 = val;
      pflag |= 0x00000004;
    }
    if(!strcmp(buffer, "increment_type2")){
      gcfg->increment_type2 = val;
      pflag |= 0x00000008;
    }
    if(!strcmp(buffer, "maximum_type1")){
      gcfg->maximum_type1 = val;
      pflag |= 0x00000010;
    }
    if(!strcmp(buffer, "maximum_type2")){
      gcfg->maximum_type2 = val;
      pflag |= 0x00000020;
    }
    if(!strcmp(buffer, "score_type1")){
      gcfg->score_type1 = val;
      pflag |= 0x00000040;
    }
    if(!strcmp(buffer, "score_type2")){
      gcfg->score_type2 = val;
      pflag |= 0x00000080;
    }
    if(!strcmp(buffer, "score_type1_waiting")){
      gcfg->score_type1_waiting = val;
      pflag |= 0x00000100;
    }
    if(!strcmp(buffer, "score_type2_waiting")){
      gcfg->score_type2_waiting = val;
      pflag |= 0x00000200;
    }
    if(!strcmp(buffer, "score_type1_splatted")){
      gcfg->score_type1_splatted = val;
      pflag |= 0x00000400;
    }
    if(!strcmp(buffer, "score_type2_splatted")){
      gcfg->score_type2_splatted = val;
      pflag |= 0x00000800;
    }
    if(!strcmp(buffer, "num_robots_per_safe")){
      gcfg->num_robots_per_safe = val;
      pflag |= 0x00001000;
    }
    if(!strcmp(buffer, "safe_score_boundary")){
      gcfg->safe_score_boundary = val;
      pflag |= 0x00002000;
    }
    if(!strcmp(buffer, "max_safe_teleports")){
      gcfg->max_safe_teleports = val;
      pflag |= 0x00004000;
    }
    if(!strcmp(buffer, "initial_safe_teleports")){
      gcfg->initial_safe_teleports = val;
      pflag |= 0x00008000;
    }
    if(!strcmp(buffer, "free_safe_teleports")){
      gcfg->free_safe_teleports = val;
      pflag |= 0x00010000;
    }
    if(!strcmp(buffer, "moveable_heaps")){
      gcfg->moveable_heaps = val;
      pflag |= 0x00020000;
    }

  }

  fclose(fp);

  /* Check we have got all types */
  if(pflag != 0x0003ffff){
    g_free(gcfg);
    gcfg = NULL;
  }

  return gcfg;
}


/**
 * load_game_configs
 *
 * Description:
 * loads all of the available configurations
 *
 * Returns:
 * TRUE on success, FALSE otherwise
 **/
gboolean load_game_configs(
){
  gint           i;
  gchar         *tmp;
  GameConfig    *gcfg;
  struct dirent *dent;
  DIR           *dir;
  gchar          buffer[PATH_MAX];
  gchar         *dname = gnome_unconditional_datadir_file(GAME_NAME);


  if(game_configs != NULL){
    free_game_configs();
  }

  dir = opendir(dname);
  if(!dir) return FALSE;

  num_configs = 0;
  while((dent = readdir(dir)) != NULL){
    if(!strstr(dent->d_name, ".cfg")){
      continue;
    }
    num_configs++;
  }

  game_configs = g_new(GameConfig*, num_configs);
  for(i = 0; i < num_configs; ++i){
    game_configs[i] = NULL;
  }

  rewinddir(dir);

  num_configs = 0;
  while((dent = readdir(dir)) != NULL){
    if(!strstr(dent->d_name, ".cfg")){
      continue;
    }
    
    strcpy(buffer, dname);
    strcat(buffer, "/");
    strcat(buffer, dent->d_name);

    gcfg = load_config(buffer);
    if(gcfg != NULL){
      game_configs[num_configs] = gcfg;
      num_configs++;
    }
  }

  closedir(dir);

  if(num_configs >= 0){
    current_config = 0;
  }

  return TRUE;
}


/**
 * free_game_configs
 *
 * Description:
 * frees the memory used by the loaded configurations
 *
 * Returns:
 * TRUE on success, FALSE otherwise
 **/
gboolean free_game_configs(
){
  gint i;

  if(game_configs == NULL){
    return FALSE;
  }

  for(i = 0; i < num_configs; ++i){
    g_free(game_configs[i]);
  }
  g_free(game_configs);

  game_configs = NULL;
  current_config = -1;
  num_configs = -1;
  
  return TRUE;
}


/**
 * num_game_configs
 *
 * Description:
 * returns the number of available game configurations
 *
 * Returns:
 * number of configurations
 **/
gint num_game_configs(
){
  if(game_configs == NULL) return -1;

  return num_configs;
}


/**
 * set_game_config
 * @n: configuraion number
 *
 * Description:
 * sets the game configuration to use
 *
 * Returns:
 * TRUE on success, FALSE otherwise
 **/
gboolean set_game_config(
gint n
){
  if(game_configs == NULL) return FALSE;

  if((n < 0) || (n >= num_configs)) return FALSE;

  current_config = n;

  return TRUE;
}


/**
 * game_config
 *
 * Description:
 * returns a pointer to the currently selected game configuration
 *
 * Returns:
 * pointer to a GameConfig structure or NULL
 **/
GameConfig* game_config(
){
  if(game_configs == NULL) return NULL;
  
  return game_configs[current_config];
}


/**
 * game_config_settings
 * @n: config number
 *
 * Description:
 * returns a pointer to a specified game configuration
 *
 * Returns:
 * pointer to a GameConfig structure or NULL
 **/
GameConfig* game_config_settings(
gint n
){
  if(game_configs == NULL) return NULL;
  
  if((n < 0) || (n >= num_configs)) return NULL;

  return game_configs[n];
}


/**
 * current_game_config
 *
 * Description:
 * returns the number of the currently selected configuration
 *
 * Returns:
 * the selected configuration or -1
 **/
gint current_game_config(
){
  if(game_configs == NULL) return -1;

  return current_config;
}


/**
 * game_config_name
 * @n: configuration number
 *
 * Description:
 * returns a string containing the name of the specified configuration
 *
 * Returns:
 * pointer to a string containing the name
 **/
gchar* game_config_name(
gint n
){
  static gchar buffer[PATH_MAX];
  gchar *ptr = buffer;

  if(game_configs == NULL) return NULL;

  if((n < 0) || (n >= num_configs)) return NULL;
  
  strcpy(buffer, game_configs[n]->description->str);

  while(*ptr){
    if(*ptr == '_'){
      *ptr = ' ';
    }
    ++ptr;
  }

  return buffer;
}


/**
 * game_config_filename
 * @n: configuration number
 *
 * Description:
 * returns a string containing the filename for the specified configuration
 *
 * Returns:
 * pointer to a string containing the filename
 **/
gchar* game_config_filename(
gint n
){
  if(game_configs == NULL) return NULL;

  if((n < 0) || (n >= num_configs)) return NULL;

  return game_configs[n]->description->str;
}

/**********************************************************************/

