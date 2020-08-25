/*
 * gbdefs.h - 'DEFINES' used through out the game
 */
#ifndef GBDEFS_H
#define GBDEFS_H

/*
 * Initial player position
 */
#define PLAYER_DEF_XPOS (GAME_WIDTH/2)
#define PLAYER_DEF_YPOS (GAME_HEIGHT/2)

/*
 * Animation
 */
#define ANIMATION_DELAY       100
#define DEAD_DELAY            30
#define CHANGE_DELAY          20
#define WAITING_DELAY         1

/*
 * Maximums for objects
 */
#define MAX_ROBOTS ((GAME_WIDTH*GAME_HEIGHT)/2)
#define MAX_HEAPS  (MAX_ROBOTS/2)

/*
 * Game states
 */
#define STATE_PLAYING     1
#define STATE_WAITING     2
#define STATE_COMPLETE    3
#define STATE_DEAD        4
#define STATE_ROBOT       5
#define STATE_TYPE2       6
#define STATE_WTYPE2      7

/*
 * Keyboard Controls
 */
#define KBD_NW   0
#define KBD_N    1
#define KBD_NE   2
#define KBD_W    3
#define KBD_STAY 4
#define KBD_E    5
#define KBD_SW   6
#define KBD_S    7
#define KBD_SE   8
#define KBD_TELE 9
#define KBD_RTEL 10
#define KBD_WAIT 11

#endif /* GBDEFS_H */
