/*
 *  File:       direct.cc
 *  Summary:    Functions used when picking squares.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *               <1>     -/--/--        LRH             Created
 */


#ifndef DIRECT_H
#define DIRECT_H

#include "externs.h"
#include "enum.h"

#ifdef JP
#define STD_DIRECTION_PROMPT    "어디에 있는 목표물을 조준합니까? ([*][+][-][Tab][q]로 조준)"
#else
#define STD_DIRECTION_PROMPT    "Which direction ([*+-][tab][q] to target)? "
#endif
// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr - debug - effects - it_use3 - item_use - spells1 -
 *              spells2 - spells3 - spells4
 * *********************************************************************** */

#define DIR_NONE    0
#define DIR_TARGET  1
#define DIR_DIR     2

void direction( struct dist &moves, int restricts = DIR_NONE, 
                int mode = TARG_ANY );

// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr - direct
 * *********************************************************************** */
void look_around( struct dist &moves, bool justLooking, int first_move = -1,
                  int mode = TARG_ANY );


#endif
