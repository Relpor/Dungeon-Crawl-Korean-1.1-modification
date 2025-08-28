/*
 *  File:       direct.cc
 *  Summary:    Functions used when picking squares.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 * <5>  01/08/01       GDL   complete rewrite of direction()
 * <4>  11/23/99       LRH   Looking at monsters now
 *                           displays more info
 * <3>  5/12/99        BWR   changes to allow for space selection of target.
 *                           CR, ESC, and 't' in targeting.
 * <2>  5/09/99        JDJ   look_around no longer prints a prompt.
 * <1>  -/--/--        LRH   Created
 */

#include "AppHdr.h"
#include "direct.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "debug.h"
#include "describe.h"
#include "itemname.h"
#include "monstuff.h"
#include "mon-util.h"
#include "player.h"
#include "shopping.h"
#include "stuff.h"
#include "spells4.h"
#include "view.h"

#ifdef USE_MACROS
#include "macro.h"
#endif

#ifdef USE_TILE
#include "tiles.h"
#endif

// x and y offsets in the following order:
// SW, S, SE, W, E, NW, N, NE
static const char xcomp[9] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
static const char ycomp[9] = { 1, 1, 1, 0, 0, 0, -1, -1, -1 };
static const char dirchars[19] = { "b1j2n3h4.5l6y7k8u9" };
static const char DOSidiocy[10] = { "OPQKSMGHI" };
#ifdef JP 
static const char *aim_prompt = "조준 (커서 이동 또는 -/+/= , CTRL-F : 방법 변경,  [.] 또는 ['] 로 결정)";
#else
static const char *aim_prompt = "Aim (move cursor or -/+/=, change mode with CTRL-F, select with . or >)";
#endif

static void describe_cell(int mx, int my);
static char mons_find( unsigned char xps, unsigned char yps, 
                       FixedVector<char, 2> &mfp, char direction, 
                       int mode = TARG_ANY );

//---------------------------------------------------------------
//
// direction
//
// input: restricts : DIR_NONE      accepts keypad dir or targetting
//                    DIR_TARGET    must use targetting.
//                    DIR_DIR       must use keypad direction
//
//
// outputs: dist structure:
//
//           isValid        a valid target or direction was chosen
//           isCancel       player hit 'escape'
//           isTarget       targetting was used
//           tx,ty          target x,y or logical beam extension to
//                             edge of map if keypad direction used.
//           dx,dy          direction delta if keypad used {-1,0,1}
//
// SYNOPSIS:
//
// gets a direction, or any of the follwing:
//
// *     go to targetting mode
// +,=   go to targetting mode, next monster
// -               "          , prev monster
// t,p   auto-select previous target
//
//
// targetting mode is handled by look_around()
//---------------------------------------------------------------
void direction( struct dist &moves, int restrict, int mode )
{
    bool dirChosen = false;
    bool targChosen = false;
    int dir = 0;

    // init
    moves.isValid       = false;
    moves.isTarget      = false;
    moves.isMe          = false;
    moves.isCancel      = false;
    moves.dx = moves.dy = 0;
    moves.tx = moves.ty = 0;

    // XXX.  this is ALWAYS in relation to the player. But a bit of a hack
    // nonetheless!  --GDL

    gotoxy( 18, 9 );

    set_keyin_mode((restrict == DIR_DIR)? 
        KEYIN_MODE_TARGET_DIR:KEYIN_MODE_TARGET_PRE);
    int keyin = getch();
    set_keyin_mode(KEYIN_MODE_NONE);

#ifdef USE_TILE
    if (Options.use_tile && Options.rotate_numpad 
         && Options.use_qv_mode)
    {
        rotate_qv_key(&keyin);
    }
#endif

    if (keyin == 0)             // DOS idiocy (emulated by win32 console)
    {
        keyin = getch();        // grrr.
        if (keyin == '*')
        {
            targChosen = true;
            dir = 0;
        }
        else
        {
            if (strchr(DOSidiocy, keyin) == NULL)
                return;
            dirChosen = true;
            dir = (int)(strchr(DOSidiocy, keyin) - DOSidiocy);
        }
    }
    else
    {
        if (strchr( dirchars, keyin ) != NULL)
        {
            dirChosen = true;
            dir = (int)(strchr(dirchars, keyin) - dirchars) / 2;
        }
        else
        {
            switch (keyin)
            {
                case CONTROL('F'):
                    mode = (mode + 1) % TARG_NUM_MODES;
                    
#ifdef JP 
                    snprintf( info, INFO_SIZE, "현재의 조준 방법: %s", 
                              (mode == TARG_ANY)   ? "모두가 대상  " :
                              (mode == TARG_ENEMY) ? "적만 대상  " 
                                                   : "아군만 대상" );
#else
                    snprintf( info, INFO_SIZE, "Targeting mode is now: %s", 
                              (mode == TARG_ANY)   ? "any" :
                              (mode == TARG_ENEMY) ? "enemies" 
                                                   : "friends" );
#endif

                    mpr( info );

                    targChosen = true;
                    dir = 0;
                    break;

                case '-':
                    targChosen = true;
                    dir = -1;
                    break;

#ifdef USE_TILE
                case CMD_MOUSE_LCLICK:
                    mouse_get_cursor_info(&moves.tx, &moves.ty);
                    moves.isValid = true;
                    moves.isTarget = true;
                    moves.tx = you.x_pos + moves.tx -8;
                    moves.ty = you.y_pos + moves.ty -8;
                    return;
                    break;
#endif

                case '*':
#ifdef USE_TILE
                case CMD_MOUSE_MOVE:
#endif
                    targChosen = true;
                    dir = 0;
                    break;

                case '\t':
                case '+':
                case '=':
                    targChosen = true;
                    dir = 1;
                    break;

                case 't':
                case 'p':
                case '\r':
                    targChosen = true;
                    dir = 2;
                    break;

                case ESCAPE:
                    moves.isCancel = true;
                    return;

                default:
                    break;
            }
        }
    }

    // at this point, we know exactly the input - validate
    if (!(targChosen || dirChosen) || (targChosen && restrict == DIR_DIR))
    {
#ifdef JP 
        mpr("이상한 지시입니다.");
#else
        mpr("What an unusual direction.");
#endif
        return;
    }

    // special case: they typed a dir key, but they're in target-only mode
    if (dirChosen && restrict == DIR_TARGET)
    {
        mpr(aim_prompt);
        look_around( moves, false, dir, mode );
        return;
    }

    if (targChosen)
    {
        if (dir < 2)
        {
            mpr(aim_prompt);
            moves.prev_target = dir;
            look_around( moves, false, -1, mode );
            if (moves.prev_target != -1)      // -1 means they pressed 'p'
                return;
        }

        // chose to aim at previous target.  do we have one?
        if (you.prev_targ == MHITNOT || you.prev_targ == MHITYOU)
        {
#ifdef JP 
            mpr("당신은 목표를 가지지 못했다.");
#else
            mpr("You haven't got a target.");
#endif
            return;
        }

        // we have a valid previous target (maybe)
        struct monsters *montarget = &menv[you.prev_targ];

        if (!mons_near(montarget) || !player_monster_visible( montarget ))
        {
#ifdef JP 
            mpr("이제 당신은 그 몬스터를 볼 수 없다.");
#else
            mpr("You can't see that creature any more.");
#endif
            return;
        }
        else
        {
            moves.isValid = true;
            moves.isTarget = true;
            moves.tx = montarget->x;
            moves.ty = montarget->y;
        }
        return;
    }

    // at this point, we have a direction, and direction is allowed.
    moves.isValid = true;
    moves.isTarget = false;
    moves.dx = xcomp[dir];
    moves.dy = ycomp[dir];
    if (xcomp[dir] == 0 && ycomp[dir] == 0)
        moves.isMe = true;

    // now the tricky bit - extend the target x,y out to map edge.
    int mx, my;
    mx = my = 0;

    if (moves.dx > 0)
        mx = (GXM  - 1) - you.x_pos;
    if (moves.dx < 0)
        mx = you.x_pos;

    if (moves.dy > 0)
        my = (GYM - 1) - you.y_pos;
    if (moves.dy < 0)
        my = you.y_pos;

    if (!(mx == 0 || my == 0))
    {
        if (mx < my)
            my = mx;
        else
            mx = my;
    }
    moves.tx = you.x_pos + moves.dx * mx;
    moves.ty = you.y_pos + moves.dy * my;
}

//---------------------------------------------------------------
//
// look_around
//
// Accessible by the x key and when using cursor aiming. Lets you
// find out what symbols mean, and is the way to access monster
// descriptions.
//
// input: dist.prev_target : -1 is last monster
//                          0 is no monster selected
//                          1 is next monster
//
// input: first_move is -1 if no initial cursor move, otherwise
// make 1 move in that direction.
//
//
// output: if dist.prev_target is -1 on OUTPUT, it means that
//   player selected 'p' ot 't' for last targetted monster.
//
//   otherwise, usual dist fields are filled in (dx and dy are
//   always zero coming back from this function)
//
//---------------------------------------------------------------

void look_around(struct dist &moves, bool justLooking, int first_move, int mode)
{
    int keyin = 0;
    bool dirChosen = false;
    bool targChosen = false;
    int dir = 0;
    int cx = 17;
    int cy = 9;
#ifdef USE_TILE
    int oldcx = cx;
    int oldcy = cy;
#endif
    int newcx, newcy;
    int mx, my;         // actual map x,y (scratch)
    int mid;            // monster id (scratch)
    FixedVector < char, 2 > monsfind_pos;

    monsfind_pos[0] = you.x_pos;
    monsfind_pos[1] = you.y_pos;

    message_current_target();

    // setup initial keystroke
    if (first_move >= 0)
        keyin = (int)'1' + first_move;
    if (moves.prev_target == -1)
        keyin = '-';
    if (moves.prev_target == 1)
        keyin = '+';
    // reset
    moves.prev_target = 0;

    // loop until some exit criteria reached
    while(true)
    {
        dirChosen = false;
        targChosen = false;

        newcx = cx;
        newcy = cy;

#ifdef USE_TILE
	    if(Options.use_tile)
        {
            TileCursor(cx-9, cy-1, 1);
            oldcx=cx;
            oldcy=cy;
        }
        else
#endif
        {
            // move cursor to current position
#ifdef JP
            if (Options.use_zenkaku)
                gotoxy(18+(cx-17)*2, cy);
            else
#endif //JP
                gotoxy(cx+1, cy);
        }

        set_keyin_mode(KEYIN_MODE_TARGET);
        if (keyin == 0)
            keyin = getch();
        set_keyin_mode(KEYIN_MODE_NONE);

#ifdef USE_TILE
        if (Options.use_tile && Options.rotate_numpad 
             && Options.use_qv_mode)
        {
            rotate_qv_key(&keyin);
        }
#endif

        // DOS idiocy
        if (keyin == 0)
        {
            // get the extended key
            keyin = getch();

            // look for CR - change to '5' to indicate selection
            if (keyin == 13)
                keyin = 'S';

            if (strchr(DOSidiocy, keyin) == NULL)
                break;
            dirChosen = true;
            dir = (int)(strchr(DOSidiocy, keyin) - DOSidiocy);
        }
        else
        {
            if (strchr(dirchars, keyin) != NULL)
            {
                dirChosen = true;
                dir = (int)(strchr(dirchars, keyin) - dirchars) / 2;
            }
            else
            {
                // handle non-directional keys
                switch (keyin)
                {
#ifdef USE_TILE
                    case CMD_MOUSE_MOVE:
                        mouse_get_cursor_info(&newcx, &newcy);
                        newcx += 9;
                        newcy++;
                        targChosen = true;
                        break;

                    case CMD_MOUSE_LCLICK:
                        mouse_get_cursor_info(&newcx, &newcy);
                        newcx += 9;
                        newcy++;
                        cx = newcx;
                        cy = newcy;
                        dirChosen = true;
                        dir = 4;
                        break;

#endif
                    case CONTROL('F'):
                        mode = (mode + 1) % TARG_NUM_MODES;
                        
#ifdef JP 
                        snprintf( info, INFO_SIZE, "현재의 조준 방법: %s",
                                  (mode == TARG_ANY)   ? "모두가 대상  " :
                                  (mode == TARG_ENEMY) ? "적만 대상  " 
                                                       : "아군만 대상" );
#else
                        snprintf( info, INFO_SIZE, "Targeting mode is now: %s",
                                  (mode == TARG_ANY)   ? "any" :
                                  (mode == TARG_ENEMY) ? "enemies" 
                                                       : "friends" );
#endif

                        mpr( info );
                        targChosen = true;
                        break;

                    case '-':
                    case 'q':
                        if (mons_find( cx, cy, monsfind_pos, -1, mode ) == 0)
                            flush_input_buffer( FLUSH_ON_FAILURE );
                        else
                        {
                            newcx = monsfind_pos[0];
                            newcy = monsfind_pos[1];
                        }
                        targChosen = true;
                        break;

                    case '+':
                    case '=':
                    case '\t':
                        if (mons_find( cx, cy, monsfind_pos, 1, mode ) == 0)
                            flush_input_buffer( FLUSH_ON_FAILURE );
                        else
                        {
                            newcx = monsfind_pos[0];
                            newcy = monsfind_pos[1];
                        }
                        targChosen = true;
                        break;

                    case 't':
                    case 'p':
                        moves.prev_target = -1;
                        break;

                    case '?':
                        targChosen = true;
                        mx = you.x_pos + cx - 17;
                        my = you.y_pos + cy - 9;
                        mid = mgrd[mx][my];

                        if (mid == NON_MONSTER)
                            break;

#if (!DEBUG_DIAGNOSTICS)
                        if (!player_monster_visible( &menv[mid] ))
                            break;
#endif

                        describe_monsters( menv[ mid ].type, mid );
                        redraw_screen();
                        mesclr( true );
                        // describe the cell again.
                        describe_cell(you.x_pos + cx - 17, you.y_pos + cy - 9);
                        break;

                    case '\r':
                    case '\n':
                    case '>':
                    case ' ':
                    case '.':
                        dirChosen = true;
                        dir = 4;
                        break;

                    case ESCAPE:
                        moves.isCancel = true;
                        mesclr( true );
#ifdef USE_TILE
        if (Options.use_tile)
            TileCursor(0, 0, 0);
#endif
                        return;

                    default:
                        break;
                }
            }
        }

        // now we have parsed the input character completely. Reset & Evaluate:
        keyin = 0;
        if (!targChosen && !dirChosen)
            break;

        // check for SELECTION
        if (dirChosen && dir == 4)
        {
            // RULE: cannot target what you cannot see
            if (env.show[cx - 8][cy] == 0 && !(cx == 17 && cy == 9))
            {
                if (!justLooking)
#ifdef JP 
                    mpr("유감스럽지만 당신은 보이지 않는 것을 목표로 지정할수 없다.");
#else
                    mpr("Sorry, you can't target what you can't see.");
#endif
                return;
            }

            moves.isValid = true;
            moves.isTarget = true;
            moves.tx = you.x_pos + cx - 17;
            moves.ty = you.y_pos + cy - 9;

            if (moves.tx == you.x_pos && moves.ty == you.y_pos)
                moves.isMe = true;
            else
            {
                // try to set you.previous target
                mx = you.x_pos + cx - 17;
                my = you.y_pos + cy - 9;
                mid = mgrd[mx][my];

                if (mid == NON_MONSTER)
                    break;

                if (!player_monster_visible( &(menv[mid]) ))
                    break;

                you.prev_targ = mid;
            }
            break;
        }

        // check for MOVE
        if (dirChosen)
        {
            newcx = cx + xcomp[dir];
            newcy = cy + ycomp[dir];
        }

        // bounds check for newcx, newcy
        if (newcx < 9)  newcx = 9;
        if (newcx > 25) newcx = 25;
        if (newcy < 1)  newcy = 1;
        if (newcy > 17) newcy = 17;

        // no-op if the cursor doesn't move.
        if (newcx == cx && newcy == cy)
            continue;

        // CURSOR MOVED - describe new cell.
        cx = newcx;
        cy = newcy;
        mesclr( true );
        if (env.show[cx - 8][cy] == 0 && !(cx == 17 && cy == 9))
        {
#ifdef JP 
            mpr("당신은 그 장소가 보이지 않는다.");
#else
            mpr("You can't see that place.");
#endif
            continue;
        }
        
        if (!justLooking) {
#ifdef JP 
            mpr("발사([Enter][.]), 취소([Esc]['])", MSGCH_PROMPT);
#else
            mpr("Shoot([Enter][.]), Cancel([Esc]['])", MSGCH_PROMPT);
#endif
        }
        //mpr("");
        describe_cell(you.x_pos + cx - 17, you.y_pos + cy - 9);
    } // end WHILE

    mesclr( true );
}                               // end look_around()

//---------------------------------------------------------------
//
// mons_find
//
// Finds the next monster (moving in a spiral outwards from the
// player, so closer monsters are chosen first; starts to player's
// left) and puts its coordinates in mfp. Returns 1 if it found
// a monster, zero otherwise. If direction is -1, goes backwards.
//
// If the game option target_zero_exp is true, zero experience
// monsters will be targeted.
//
//---------------------------------------------------------------
static char mons_find( unsigned char xps, unsigned char yps,
                       FixedVector<char, 2> &mfp, char direction, int mode )
{
    unsigned char temp_xps = xps;
    unsigned char temp_yps = yps;
    char x_change = 0;
    char y_change = 0;

    int i, j;

    if (direction == 1 && temp_xps == 9 && temp_yps == 17)
        return (0);               // end of spiral

    while (temp_xps >= 8 && temp_xps <= 25 && temp_yps <= 17) // yps always >= 0
    {
        if (direction == -1 && temp_xps == 17 && temp_yps == 9)
            return (0);           // can't go backwards from you

        if (direction == 1)
        {
            if (temp_xps == 8)
            {
                x_change = 0;
                y_change = -1;
            }
            else if (temp_xps - 17 == 0 && temp_yps - 9 == 0)
            {
                x_change = -1;
                y_change = 0;
            }
            else if (abs(temp_xps - 17) <= abs(temp_yps - 9))
            {
                if (temp_xps - 17 >= 0 && temp_yps - 9 <= 0)
                {
                    if (abs(temp_xps - 17) > abs(temp_yps - 9 + 1))
                    {
                        x_change = 0;
                        y_change = -1;
                        if (temp_xps - 17 > 0)
                            y_change = 1;
                        goto finished_spiralling;
                    }
                }
                x_change = -1;
                if (temp_yps - 9 < 0)
                    x_change = 1;
                y_change = 0;
            }
            else
            {
                x_change = 0;
                y_change = -1;
                if (temp_xps - 17 > 0)
                    y_change = 1;
            }
        }                       // end if (direction == 1)
        else
        {
            /*
               This part checks all eight surrounding squares to find the
               one that leads on to the present square.
             */
            for (i = -1; i < 2; i++)
            {
                for (j = -1; j < 2; j++)
                {
                    if (i == 0 && j == 0)
                        continue;

                    if (temp_xps + i == 8)
                    {
                        x_change = 0;
                        y_change = -1;
                    }
                    else if (temp_xps + i - 17 == 0 && temp_yps + j - 9 == 0)
                    {
                        x_change = -1;
                        y_change = 0;
                    }
                    else if (abs(temp_xps + i - 17) <= abs(temp_yps + j - 9))
                    {
                        const int xi = temp_xps + i - 17;
                        const int yj = temp_yps + j - 9;

                        if (xi >= 0 && yj <= 0)
                        {
                            if (abs(xi) > abs(yj + 1))
                            {
                                x_change = 0;
                                y_change = -1;
                                if (xi > 0)
                                    y_change = 1;
                                goto finished_spiralling;
                            }
                        }

                        x_change = -1;
                        if (yj < 0)
                            x_change = 1;
                        y_change = 0;
                    }
                    else
                    {
                        x_change = 0;
                        y_change = -1;
                        if (temp_xps + i - 17 > 0)
                            y_change = 1;
                    }

                    if (temp_xps + i + x_change == temp_xps
                        && temp_yps + j + y_change == temp_yps)
                    {
                        goto finished_spiralling;
                    }
                }
            }
        }                       // end else


      finished_spiralling:
        x_change *= direction;
        y_change *= direction;

        temp_xps += x_change;
        if (temp_yps + y_change <= 17)  // it can wrap, unfortunately
            temp_yps += y_change;

        const int targ_x = you.x_pos + temp_xps - 17;
        const int targ_y = you.y_pos + temp_yps - 9;

        // We don't want to be looking outside the bounds of the arrays:
        if (temp_xps > 25 || temp_xps < 8 || temp_yps > 17 || temp_yps < 1)
            continue;

        if (targ_x < 0 || targ_x >= GXM || targ_y < 0 || targ_y >= GYM)
            continue;

        const int targ_mon = mgrd[ targ_x ][ targ_y ];

        if (targ_mon != NON_MONSTER 
            && env.show[temp_xps - 8][temp_yps] != 0
            && player_monster_visible( &(menv[targ_mon]) )
            && !mons_is_mimic( menv[targ_mon].type )
            && (mode == TARG_ANY
                || (mode == TARG_FRIEND && mons_friendly( &menv[targ_mon] ))
                || (mode == TARG_ENEMY 
                    && !mons_friendly( &menv[targ_mon] )
                    && 
                    (Options.target_zero_exp || 
                        !mons_flag( menv[targ_mon].type, M_NO_EXP_GAIN )) )))
        {
            //mpr("Found something!");
            //more();
            mfp[0] = temp_xps;
            mfp[1] = temp_yps;
            return (1);
        }
    }

    return (0);
}

static void describe_cell(int mx, int my)
{
    int   trf;            // used for trap type??
    char  str_pass[ ITEMNAME_SIZE ];
    bool  mimic_item = false;

    if (mgrd[mx][my] != NON_MONSTER)
    {
        int i = mgrd[mx][my];

        if (grd[mx][my] == DNGN_SHALLOW_WATER)
        {
            if (!player_monster_visible(&menv[i]) && !mons_flies(&menv[i]))
            {
#ifdef JP 
                mpr("여기의 수면에는 기묘한 동요가 있다.");
#else
                mpr("There is a strange disturbance in the water here.");
#endif
            }
        }

#if DEBUG_DIAGNOSTICS
        if (!player_monster_visible( &menv[i] ))
#ifdef JP 
            mpr( "There is a non-visible monster here.", MSGCH_DIAGNOSTICS );
#else
            mpr( "There is a non-visible monster here.", MSGCH_DIAGNOSTICS );
#endif
#else
        if (!player_monster_visible( &menv[i] ))
            goto look_clouds;
#endif

        const int mon_wep = menv[i].inv[MSLOT_WEAPON];
        const int mon_arm = menv[i].inv[MSLOT_ARMOUR];

#ifdef JP 
        strcpy(info, "여기에는 ");
        strcat(info, ptr_monam( &(menv[i]), DESC_CAP_A ));
        strcat(info, " 이(가) 있다.");
#else
        strcpy(info, ptr_monam( &(menv[i]), DESC_CAP_A ));
        strcat(info, ".");
#endif
        mpr(info);

        if (menv[i].type != MONS_DANCING_WEAPON && mon_wep != NON_ITEM)
        {
#ifdef JP 
            snprintf( info, INFO_SIZE, "%s는(은) ", mons_pronoun( menv[i].type, 
#else
            snprintf( info, INFO_SIZE, "%s is wielding ", mons_pronoun( menv[i].type, 
#endif
                                                           PRONOUN_CAP ));
            it_name(mon_wep, DESC_NOCAP_A, str_pass);
            strcat(info, str_pass);

            // 2-headed ogres can wield 2 weapons
            if ((menv[i].type == MONS_TWO_HEADED_OGRE 
                    || menv[i].type == MONS_ETTIN)
                && menv[i].inv[MSLOT_MISSILE] != NON_ITEM)
            {
#ifdef JP 
                strcat( info, " 와(과) " );
#else
                strcat( info, " and " );
#endif
                it_name(menv[i].inv[MSLOT_MISSILE], DESC_NOCAP_A, str_pass);
                strcat(info, str_pass);
#ifdef JP 
                strcat(info, " 를(을) 손에 들고 있다.");
#else
                strcat(info, ".");
#endif

                mpr(info);
            }
            else
            {
#ifdef JP 
                strcat(info, " 를(을) 손에 들고 있다.");
#else
                strcat(info, ".");
#endif
                mpr(info);
            }
        }

        if (mon_arm != NON_ITEM)
        {
            it_name( mon_arm, DESC_PLAIN, str_pass );
#ifdef JP 
            snprintf( info, INFO_SIZE, "%s은(는) %s를(을) 입고 있다.", 
#else
            snprintf( info, INFO_SIZE, "%s is wearing %s.", 
#endif
                      mons_pronoun( menv[i].type, PRONOUN_CAP ),
                      str_pass );

            mpr( info );
        }


        if (menv[i].type == MONS_HYDRA)
        {
#ifdef JP 
            snprintf( info, INFO_SIZE, "그것은 %d개의 머리를 가지고 있다.", menv[i].number );
#else
            snprintf( info, INFO_SIZE, "It has %d head%s.", 
                    menv[i].number, (menv[i].number > 1? "s" : "") );
#endif
            mpr( info );
        }

        print_wounds(&menv[i]);


        if (mons_is_mimic( menv[i].type ))
            mimic_item = true;
        else if (!mons_flag(menv[i].type, M_NO_EXP_GAIN))
        {
            if (menv[i].behaviour == BEH_SLEEP)
            {
                strcpy(info, mons_pronoun(menv[i].type, PRONOUN_CAP));
#ifdef JP 
                strcat(info, "는(은) 당신을 알아차리지 못한 것 같다.");
#else
                strcat(info, " doesn't appear to have noticed you.");
#endif
                mpr(info);
            }
            // wandering hostile with no target in LOS
            else if (menv[i].behaviour == BEH_WANDER && !mons_friendly(&menv[i])
                    && menv[i].foe == MHITNOT)
            {
                // special case: batty monsters get set to BEH_WANDER as
                // part of their special behaviour.
                if (!testbits(menv[i].flags, MF_BATTY))
                {
                    strcpy(info, mons_pronoun(menv[i].type, PRONOUN_CAP));
#ifdef JP 
                    strcat(info, "는(은) 당신에게 흥미를 가지고 있지 않은 것 같다.");
#else
                    strcat(info, " doesn't appear to be interested in you.");
#endif
                    mpr(info);
                }
            }
        }

        if (menv[i].attitude == ATT_FRIENDLY)
        {
            strcpy(info, mons_pronoun(menv[i].type, PRONOUN_CAP));
#ifdef JP 
            strcat(info, "는(은) 우호적이다.");
#else
            strcat(info, " is friendly.");
#endif
            mpr(info);
        }

        for (int p = 0; p < NUM_MON_ENCHANTS; p++)
        {
            strcpy(info, mons_pronoun(menv[i].type, PRONOUN_CAP));
            switch (menv[i].enchantment[p])
            {
            case ENCH_YOUR_ROT_I:
            case ENCH_YOUR_ROT_II:
            case ENCH_YOUR_ROT_III:
            case ENCH_YOUR_ROT_IV:
#ifdef JP 
                strcat(info, "는(은) 썩어가고 있다."); //jmf: "covered in sores"?
#else
                strcat(info, " is rotting away."); //jmf: "covered in sores"?
#endif
                break;
            case ENCH_BACKLIGHT_I:
            case ENCH_BACKLIGHT_II:
            case ENCH_BACKLIGHT_III:
            case ENCH_BACKLIGHT_IV:
#ifdef JP 
                strcat(info, "는(은) 약하게 빛나고 있다.");
#else
                strcat(info, " is softly glowing.");
#endif
                break;
            case ENCH_SLOW:
#ifdef JP 
                strcat(info, "는(은) 둔하게 움직이고 있다.");
#else
                strcat(info, " is moving slowly.");
#endif
                break;
            case ENCH_HASTE:
#ifdef JP 
                strcat(info, "는(은) 매우 재빠르게 움직이고 있다.");
#else
                strcat(info, " is moving very quickly.");
#endif
                break;
            case ENCH_CONFUSION:
#ifdef JP 
                strcat(info, "는(은) 혼란스럽게 느끼고 있는 것 같다.");
#else
                strcat(info, " appears to be bewildered and confused.");
#endif
                break;
            case ENCH_INVIS:
#ifdef JP 
                strcat(info, "는(은) 약간 투명한 상태로 있다.");
#else
                strcat(info, " is slightly transparent.");
#endif
                break;
            case ENCH_CHARM:
#ifdef JP 
                strcat(info, "는(은) 당신의 노예다.");
#else
                strcat(info, " is in your thrall.");
#endif
                break;
            case ENCH_YOUR_STICKY_FLAME_I:
            case ENCH_YOUR_STICKY_FLAME_II:
            case ENCH_YOUR_STICKY_FLAME_III:
            case ENCH_YOUR_STICKY_FLAME_IV:
            case ENCH_STICKY_FLAME_I:
            case ENCH_STICKY_FLAME_II:
            case ENCH_STICKY_FLAME_III:
            case ENCH_STICKY_FLAME_IV:
#ifdef JP 
                strcat(info, "는(은) 불길과 끓어오르는 액체로 감싸여 있다.");
#else
                strcat(info, " is covered in liquid flames.");
#endif
                break;
            default:
                info[0] = '\0';
                break;
            } // end switch
            if (info[0])
                mpr(info);
        }

#if DEBUG_DIAGNOSTICS
        stethoscope(i);
#endif
    }

#if (!DEBUG_DIAGNOSTICS)
  // removing warning
  look_clouds:
#endif
    if (env.cgrid[mx][my] != EMPTY_CLOUD)
    {
        const char cloud_inspected = env.cgrid[mx][my];

        const char cloud_type = env.cloud[ cloud_inspected ].type;

#ifdef JP 
        strcpy(info, "여기에는 ");
#else
        strcpy(info, "There is a cloud of ");
#endif
        strcat(info,
            (cloud_type == CLOUD_FIRE
#ifdef JP 
              || cloud_type == CLOUD_FIRE_MON) ? "불꽃" :
#else
              || cloud_type == CLOUD_FIRE_MON) ? "flame" :
#endif
            (cloud_type == CLOUD_STINK
#ifdef JP 
              || cloud_type == CLOUD_STINK_MON) ? "유독 가스의 구름" :
#else
              || cloud_type == CLOUD_STINK_MON) ? "noxious fumes" :
#endif
            (cloud_type == CLOUD_COLD
#ifdef JP 
              || cloud_type == CLOUD_COLD_MON) ? "냉기의 구름" :
#else
              || cloud_type == CLOUD_COLD_MON) ? "freezing vapour" :
#endif
            (cloud_type == CLOUD_POISON
#ifdef JP 
              || cloud_type == CLOUD_POISON_MON) ? "독가스 구름" :
#else
              || cloud_type == CLOUD_POISON_MON) ? "poison gases" :
#endif
            (cloud_type == CLOUD_GREY_SMOKE
#ifdef JP 
              || cloud_type == CLOUD_GREY_SMOKE_MON) ? "잿빛 연기 의 구름" :
#else
              || cloud_type == CLOUD_GREY_SMOKE_MON) ? "grey smoke" :
#endif
            (cloud_type == CLOUD_BLUE_SMOKE
#ifdef JP 
              || cloud_type == CLOUD_BLUE_SMOKE_MON) ? "푸른 연기 의 구름" :
#else
              || cloud_type == CLOUD_BLUE_SMOKE_MON) ? "blue smoke" :
#endif
            (cloud_type == CLOUD_PURP_SMOKE
#ifdef JP 
              || cloud_type == CLOUD_PURP_SMOKE_MON) ? "보랏빛 연기의 구름" :
#else
              || cloud_type == CLOUD_PURP_SMOKE_MON) ? "purple smoke" :
#endif
            (cloud_type == CLOUD_STEAM
#ifdef JP 
              || cloud_type == CLOUD_STEAM_MON) ? "증기의 구름" :
#else
              || cloud_type == CLOUD_STEAM_MON) ? "steam" :
#endif
            (cloud_type == CLOUD_MIASMA
#ifdef JP 
              || cloud_type == CLOUD_MIASMA_MON) ? "질병의 독기" :
#else
              || cloud_type == CLOUD_MIASMA_MON) ? "foul pestilence" :
#endif
            (cloud_type == CLOUD_BLACK_SMOKE
#ifdef JP 
              || cloud_type == CLOUD_BLACK_SMOKE_MON) ? "검은 연기의 구름"
                                                      : "버기신의 구름");
#else
              || cloud_type == CLOUD_BLACK_SMOKE_MON) ? "black smoke"
                                                      : "buggy goodness");
#endif
#ifdef JP 
        strcat(info, " 이(가) 있다.");
#else
        strcat(info, " here.");
#endif
        mpr(info);
    }

    int targ_item = igrd[ mx ][ my ];

    if (targ_item != NON_ITEM)
    {
        // If a mimic is on this square, we pretend its the first item -- bwr
        if (mimic_item)
#ifdef JP 
            mpr("여기에는 무엇인가가 있다!");
#else
            mpr("There is something else lying underneath.");
#endif
        else
        {
            if (mitm[ targ_item ].base_type == OBJ_GOLD)
            {
#ifdef JP 
                mpr( "한 무더기의 금화" );
#else
                mpr( "A pile of gold coins." );
#endif
            }
            else
            {
#ifdef JP 
                strcpy(info, "여기에는 ");
#else
                strcpy(info, "You see ");
#endif
                it_name( targ_item, DESC_NOCAP_A, str_pass);
                strcat(info, str_pass);
#ifdef JP 
                strcat(info, " 이(가) 있다.");
#else
                strcat(info, " here.");
#endif
                mpr(info);
            }

            if (mitm[ targ_item ].link != NON_ITEM)
#ifdef JP 
                mpr("여기에는 무엇인가가 있다.");
#else
                mpr("There is something else lying underneath.");
#endif
        }
    }

    switch (grd[mx][my])
    {
    case DNGN_STONE_WALL:
#ifdef JP 
        mpr("지형:돌벽");
#else
        mpr("A stone wall.");
#endif
        break;
    case DNGN_ROCK_WALL:
    case DNGN_SECRET_DOOR:
        if (you.level_type == LEVEL_PANDEMONIUM)
#ifdef JP 
            mpr("지형:판데모니움을 구성하는 기분 나쁜 재질의 벽");
#else
            mpr("A wall of the weird stuff which makes up Pandemonium.");
#endif
        else
#ifdef JP 
            mpr("지형:암벽");
#else
            mpr("A rock wall.");
#endif
        break;
    case DNGN_PERMAROCK_WALL:
#ifdef JP 
        mpr("지형:비정상적으로 단단한 암벽");
#else
        mpr("An unnaturally hard rock wall.");
#endif
        break;    
    case DNGN_CLOSED_DOOR:
#ifdef JP 
        mpr("지형:닫혀진 문");
#else
        mpr("A closed door.");
#endif
        break;
    case DNGN_METAL_WALL:
#ifdef JP 
        mpr("지형:금속의 벽");
#else
        mpr("A metal wall.");
#endif
        break;
    case DNGN_GREEN_CRYSTAL_WALL:
#ifdef JP 
        mpr("지형:녹색수정의 벽");
#else
        mpr("A wall of green crystal.");
#endif
        break;
    case DNGN_ORCISH_IDOL:
#ifdef JP 
        mpr("지형:오크의 상");
#else
        mpr("An orcish idol.");
#endif
        break;
    case DNGN_WAX_WALL:
#ifdef JP 
        mpr("지형:밀납의 벽");
#else
        mpr("A wall of solid wax.");
#endif
        break;
    case DNGN_SILVER_STATUE:
#ifdef JP 
        mpr("지형:은의 상");
#else
        mpr("A silver statue.");
#endif
        break;
    case DNGN_GRANITE_STATUE:
#ifdef JP 
        mpr("지형:화강암의 상");
#else
        mpr("A granite statue.");
#endif
        break;
    case DNGN_ORANGE_CRYSTAL_STATUE:
#ifdef JP 
        mpr("지형:오렌지색 수정의 상");
#else
        mpr("An orange crystal statue.");
#endif
        break;
    case DNGN_LAVA:
#ifdef JP 
        mpr("지형:용암");
#else
        mpr("Some lava.");
#endif
        break;
    case DNGN_DEEP_WATER:
#ifdef JP 
        mpr("지형:깊은 물");
#else
        mpr("Some deep water.");
#endif
        break;
    case DNGN_SHALLOW_WATER:
#ifdef JP 
        mpr("지형:얕은 물");
#else
        mpr("Some shallow water.");
#endif
        break;
    case DNGN_UNDISCOVERED_TRAP:
    case DNGN_FLOOR:
#ifdef JP 
        mpr("지형:지면");
#else
        mpr("Floor.");
#endif
        break;
    case DNGN_OPEN_DOOR:
#ifdef JP 
        mpr("지형:열린 문");
#else
        mpr("An open door.");
#endif
        break;
    case DNGN_ROCK_STAIRS_DOWN:
#ifdef JP 
        mpr("지형:밑으로 향하는 바위계단");
#else
        mpr("A rock staircase leading down.");
#endif
        break;
    case DNGN_STONE_STAIRS_DOWN_I:
    case DNGN_STONE_STAIRS_DOWN_II:
    case DNGN_STONE_STAIRS_DOWN_III:
#ifdef JP 
        mpr("지형:밑으로 향하는 돌계단");
#else
        mpr("A stone staircase leading down.");
#endif
        break;
    case DNGN_ROCK_STAIRS_UP:
#ifdef JP 
        mpr("지형:위로 향하는 바위계단");
#else
        mpr("A rock staircase leading upwards.");
#endif
        break;
    case DNGN_STONE_STAIRS_UP_I:
    case DNGN_STONE_STAIRS_UP_II:
    case DNGN_STONE_STAIRS_UP_III:
#ifdef JP 
        mpr("지형:위로 향하는 돌계단");
#else
        mpr("A stone staircase leading up.");
#endif
        break;
    case DNGN_ENTER_HELL:
#ifdef JP 
        mpr("지형:지옥으로 가는 문");
#else
        mpr("A gateway to hell.");
#endif
        break;
    case DNGN_BRANCH_STAIRS:
#ifdef JP 
        mpr("지형:분기층으로의 계단");
#else
        mpr("A staircase to a branch level.");
#endif
        break;
    case DNGN_TRAP_MECHANICAL:
    case DNGN_TRAP_MAGICAL:
    case DNGN_TRAP_III:
        for (trf = 0; trf < MAX_TRAPS; trf++)
        {
            if (env.trap[trf].x == mx
                && env.trap[trf].y == my)
            {
                break;
            }

            if (trf == MAX_TRAPS - 1)
            {
#ifdef JP 
                mpr("Error - couldn't find that trap.");
#else
                mpr("Error - couldn't find that trap.");
#endif
                error_message_to_player();
                break;
            }
        }

        switch (env.trap[trf].type)
        {
        case TRAP_DART:
#ifdef JP 
            mpr("지형:다트 함정");
#else
            mpr("A dart trap.");
#endif
            break;
        case TRAP_ARROW:
#ifdef JP 
            mpr("지형:화살 함정");
#else
            mpr("An arrow trap.");
#endif
            break;
        case TRAP_SPEAR:
#ifdef JP 
            mpr("지형:창 함정");
#else
            mpr("A spear trap.");
#endif
            break;
        case TRAP_AXE:
#ifdef JP 
            mpr("지형:도끼 함정");
#else
            mpr("An axe trap.");
#endif
            break;
        case TRAP_TELEPORT:
#ifdef JP 
            mpr("지형:공간이동의 함정");
#else
            mpr("A teleportation trap.");
#endif
            break;
        case TRAP_AMNESIA:
#ifdef JP 
            mpr("지형:기억상실의 함정");
#else
            mpr("An amnesia trap.");
#endif
            break;
        case TRAP_BLADE:
#ifdef JP 
            mpr("지형:칼날 함정");
#else
            mpr("A blade trap.");
#endif
            break;
        case TRAP_BOLT:
#ifdef JP 
            mpr("지형:석궁 함정");
#else
            mpr("A bolt trap.");
#endif
            break;
        case TRAP_ZOT:
#ifdef JP 
            mpr("지형:조트의 함정");
#else
            mpr("A Zot trap.");
#endif
            break;
        case TRAP_NEEDLE:
#ifdef JP 
            mpr("지형:바늘 함정");
#else
            mpr("A needle trap.");
#endif
            break;
        default:
#ifdef JP 
            mpr("지형:정의되지 않은 함정!!!!");
#else
            mpr("An undefined trap. Huh?");
#endif
            error_message_to_player();
            break;
        }
        break;
    case DNGN_ENTER_SHOP:
        mpr(shop_name(mx, my));
        break;
    case DNGN_ENTER_LABYRINTH:
#ifdef JP 
        mpr("지형:미궁으로의 입구");
#else
        mpr("A labyrinth entrance.");
#endif
        break;
    case DNGN_ENTER_DIS:
#ifdef JP 
        mpr("지형:철의 도시 디스로의 문");
#else
        mpr("A gateway to the Iron City of Dis.");
#endif
        break;
    case DNGN_ENTER_GEHENNA:
#ifdef JP 
        mpr("지형:게헨나로 통하는 문");
#else
        mpr("A gateway to Gehenna.");
#endif
        break;
    case DNGN_ENTER_COCYTUS:
#ifdef JP 
        mpr("지형:얼어붙은 폐허 코키투스로의 문");
#else
        mpr("A gateway to the freezing wastes of Cocytus.");
#endif
        break;
    case DNGN_ENTER_TARTARUS:
#ifdef JP 
        mpr("지형:썩어가는 지옥 타르타루스로의 문");
#else
        mpr("A gateway to the decaying netherworld of Tartarus.");
#endif
        break;
    case DNGN_ENTER_ABYSS:
#ifdef JP 
        mpr("지형:무한지옥 어비스로의 문");
#else
        mpr("A gateway to the infinite Abyss.");
#endif
        break;
    case DNGN_EXIT_ABYSS:
#ifdef JP 
        mpr("지형:어비스의 탈출구");
#else
        mpr("A gateway leading out of the Abyss.");
#endif
        break;
    case DNGN_STONE_ARCH:
#ifdef JP 
        mpr("지형:고대의 돌로 만들어진 아치");
#else
        mpr("An empty arch of ancient stone.");
#endif
        break;
    case DNGN_ENTER_PANDEMONIUM:
#ifdef JP 
        mpr("지형:판데모니움의 큰 방으로 이어지는 문");
#else
        mpr("A gate leading to the halls of Pandemonium.");
#endif
        break;
    case DNGN_EXIT_PANDEMONIUM:
#ifdef JP 
        mpr("판데모니움의 탈출구");
#else
        mpr("A gate leading out of Pandemonium.");
#endif
        break;
    case DNGN_TRANSIT_PANDEMONIUM:
#ifdef JP 
        mpr("지형:판데모니움의 다른지역으로의 문");
#else
        mpr("A gate leading to another region of Pandemonium.");
#endif
        break;
    case DNGN_ENTER_ORCISH_MINES:
#ifdef JP 
        mpr("지형:오크 광산으로의 계단");
#else
        mpr("A staircase to the Orcish Mines.");
#endif
        break;
    case DNGN_ENTER_HIVE:
#ifdef JP 
        mpr("지형:벌집으로의 계단");
#else
        mpr("A staircase to the Hive.");
#endif
        break;
    case DNGN_ENTER_LAIR:
#ifdef JP 
        mpr("지형:짐승소굴로의 계단");
#else
        mpr("A staircase to the Lair.");
#endif
        break;
    case DNGN_ENTER_SLIME_PITS:
#ifdef JP 
        mpr("지형:슬라임의 소굴 로의 계단");
#else
        mpr("A staircase to the Slime Pits.");
#endif
        break;
    case DNGN_ENTER_VAULTS:
#ifdef JP 
        mpr("지형:보물창고로의 계단");
#else
        mpr("A staircase to the Vaults.");
#endif
        break;
    case DNGN_ENTER_CRYPT:
#ifdef JP 
        mpr("지형:납골당으로의 계단");
#else
        mpr("A staircase to the Crypt.");
#endif
        break;
    case DNGN_ENTER_HALL_OF_BLADES:
#ifdef JP 
        mpr("지형:칼날의 방으로의 계단");
#else
        mpr("A staircase to the Hall of Blades.");
#endif
        break;
    case DNGN_ENTER_ZOT:
#ifdef JP 
        mpr("지형:조트의 방으로의 문");
#else
        mpr("A gate to the Realm of Zot.");
#endif
        break;
    case DNGN_ENTER_TEMPLE:
#ifdef JP 
        mpr("지형:모든 종교의 사원으로의 계단");
#else
        mpr("A staircase to the Ecumenical Temple.");
#endif
        break;
    case DNGN_ENTER_SNAKE_PIT:
#ifdef JP 
        mpr("지형:뱀의 소굴로의 계단");
#else
        mpr("A staircase to the Snake Pit.");
#endif
        break;
    case DNGN_ENTER_ELVEN_HALLS:
#ifdef JP 
        mpr("지형:엘프의 방로의 계단");
#else
        mpr("A staircase to the Elven Halls.");
#endif
        break;
    case DNGN_ENTER_TOMB:
#ifdef JP 
        mpr("지형:무덤으로의 계단");
#else
        mpr("A staircase to the Tomb.");
#endif
        break;
    case DNGN_ENTER_SWAMP:
#ifdef JP 
        mpr("지형:늪지로의 계단");
#else
        mpr("A staircase to the Swamp.");
#endif
        break;
    case DNGN_RETURN_FROM_ORCISH_MINES:
    case DNGN_RETURN_FROM_HIVE:
    case DNGN_RETURN_FROM_LAIR:
    case DNGN_RETURN_FROM_VAULTS:
    case DNGN_RETURN_FROM_TEMPLE:
#ifdef JP 
        mpr("지형:던젼으로 돌아가는 계단");
#else
        mpr("A staircase back to the Dungeon.");
#endif
        break;
    case DNGN_RETURN_FROM_SLIME_PITS:
    case DNGN_RETURN_FROM_SNAKE_PIT:
    case DNGN_RETURN_FROM_SWAMP:
#ifdef JP 
        mpr("짐승 소굴로 돌아가는 계단");
#else
        mpr("A staircase back to the Lair.");
#endif
        break;
    case DNGN_RETURN_FROM_CRYPT:
    case DNGN_RETURN_FROM_HALL_OF_BLADES:
#ifdef JP 
        mpr("지형:보물창고로 돌아가는 계단");
#else
        mpr("A staircase back to the Vaults.");
#endif
        break;
    case DNGN_RETURN_FROM_ELVEN_HALLS:
#ifdef JP 
        mpr("지형:오크 광산으로 돌아가는 계단");
#else
        mpr("A staircase back to the Mines.");
#endif
        break;
    case DNGN_RETURN_FROM_TOMB:
#ifdef JP 
        mpr("지형:무덤으로 돌아가는 계단");
#else
        mpr("A staircase back to the Crypt.");
#endif
        break;
    case DNGN_RETURN_FROM_ZOT:
#ifdef JP 
        mpr("지형:이 장소에서 밖으로 나가는 문");
#else
        mpr("A gate leading back out of this place.");
#endif
        break;
    case DNGN_ALTAR_ZIN:
#ifdef JP 
        mpr("지형: 『진』의 빛나는 흰 대리석 제단");
#else
        mpr("A glowing white marble altar of Zin.");
#endif
        break;
    case DNGN_ALTAR_SHINING_ONE:
#ifdef JP 
        mpr("지형:『샤이닝 원』 의 빛나는 황금제단");
#else
        mpr("A glowing golden altar of the Shining One.");
#endif
        break;
    case DNGN_ALTAR_KIKUBAAQUDGHA:
#ifdef JP 
        mpr("지형:『키쿠바쿠드하』 의 고대의 뼈로 만들어진 제단 ");
#else
        mpr("An ancient bone altar of Kikubaaqudgha.");
#endif
        break;
    case DNGN_ALTAR_YREDELEMNUL:
#ifdef JP 
        mpr("지형:『이레델렘눌 』 의 현무암으로 만들어진 제단");
#else
        mpr("A basalt altar of Yredelemnul.");
#endif
        break;
    case DNGN_ALTAR_XOM:
#ifdef JP 
        mpr("지형:『죰』 의 미광이 흔들리는 제단");
#else
        mpr("A shimmering altar of Xom.");
#endif
        break;
    case DNGN_ALTAR_VEHUMET:
#ifdef JP 
        mpr("지형:『베후멧』 의 빛을 발하는 제단");
#else
        mpr("A shining altar of Vehumet.");
#endif
        break;
    case DNGN_ALTAR_OKAWARU:
#ifdef JP 
        mpr("지형:『오카와루』 의 철의 제단 ");
#else
        mpr("An iron altar of Okawaru.");
#endif
        break;
    case DNGN_ALTAR_MAKHLEB:
#ifdef JP 
        mpr("지형:『마크레브』 의 불길이 이는 제단");
#else
        mpr("A burning altar of Makhleb.");
#endif
        break;
    case DNGN_ALTAR_SIF_MUNA:
#ifdef JP 
        mpr("지형:『시프 무나』 의 감청색의 제단");
#else
        mpr("A deep blue altar of Sif Muna.");
#endif
        break;
    case DNGN_ALTAR_TROG:
#ifdef JP 
        mpr("지형:『트로그』의 피에 물든 제단");
#else
        mpr("A bloodstained altar of Trog.");
#endif
        break;
    case DNGN_ALTAR_NEMELEX_XOBEH:
#ifdef JP 
        mpr("지형:『네멜렉스 조베』 의 빛나는 제단");
#else
        mpr("A sparkling altar of Nemelex Xobeh.");
#endif
        break;
    case DNGN_ALTAR_ELYVILON:
#ifdef JP 
        mpr("『엘리빌론』 의 은 제단 ");
#else
        mpr("A silver altar of Elyvilon.");
#endif
        break;
    case DNGN_BLUE_FOUNTAIN:
#ifdef JP 
        mpr("지형:맑고 푸른 물의 샘");
#else
        mpr("A fountain of clear blue water.");
#endif
        break;
    case DNGN_SPARKLING_FOUNTAIN:
#ifdef JP 
        mpr("지형:빛나는 물의 샘");
#else
        mpr("A fountain of sparkling water.");
#endif
        break;
    case DNGN_DRY_FOUNTAIN_I:
    case DNGN_DRY_FOUNTAIN_II:
    case DNGN_DRY_FOUNTAIN_IV:
    case DNGN_DRY_FOUNTAIN_VI:
    case DNGN_DRY_FOUNTAIN_VIII:
    case DNGN_PERMADRY_FOUNTAIN:
#ifdef JP 
        mpr("지형:바싹 마른 샘");
#else
        mpr("A dry fountain.");
#endif
        break;
    }
}

#ifdef USE_TILE
char *look_directly(int cx, int cy){
    int mx = you.x_pos + cx - 9;
    int my = you.y_pos + cy - 9;

    int   trf;            // used for trap type??
    char  str_pass[ ITEMNAME_SIZE ];
    bool  mimic_item = false;
    static char tipbuf[1024];

    if((cx < 1) || (cx > 17) || (cy < 1) || (cy > 17)) return NULL;

    tipbuf[0]=0;
    info[0]=0;

    if (mgrd[mx][my] != NON_MONSTER)
    {
        int i = mgrd[mx][my];

        if (grd[mx][my] == DNGN_SHALLOW_WATER)
        {
            if (!player_monster_visible(&menv[i]) && !mons_flies(&menv[i]))
            {
#ifdef JP 
                strcat(tipbuf, "기묘한 수면의 요동");
#else
                strcat(tipbuf, "Strange disturbance in the water.");
#endif
            }
        }

        if (!player_monster_visible( &menv[i] ))
            goto look_clouds_tip;

        const int mon_wep = menv[i].inv[MSLOT_WEAPON];
        const int mon_arm = menv[i].inv[MSLOT_ARMOUR];

#ifdef JP 
        strcpy(info, ptr_monam( &(menv[i]), DESC_CAP_A ));
#else
        strcpy(info, ptr_monam( &(menv[i]), DESC_CAP_A ));
#endif
        strcat(tipbuf,info);

        if (menv[i].type != MONS_DANCING_WEAPON && mon_wep != NON_ITEM)
        {
            it_name(mon_wep, DESC_NOCAP_A, str_pass);

#ifdef JP
	    // "오우거"
            strcpy( info, "," );
#else
	    // "An Ogre, wielding "
            strcpy( info, ", wielding " );
#endif
            strcat(info, str_pass);

            // 2-headed ogres can wield 2 weapons
            if ((menv[i].type == MONS_TWO_HEADED_OGRE 
                    || menv[i].type == MONS_ETTIN)
                && menv[i].inv[MSLOT_MISSILE] != NON_ITEM)
            {
#ifdef JP 
		// "오우거,곤봉과"
                strcat( info, " 과(와) " );
#else
		// "An Ogre, wielding a club and "
                strcat( info, " and " );
#endif
                it_name(menv[i].inv[MSLOT_MISSILE], DESC_NOCAP_A, str_pass);
                strcat(info, str_pass);
#ifdef JP 
                strcat(info, " 를(을) 장비");
#endif
		// "긆?긊갂궞귪?궴긽귽긚귩몧뷈"
		// "An Ogre, wielding a club and a mace"
                strcat(tipbuf,info);
            }
            else
            {
#ifdef JP 
		// "오거, 곤봉을 장비"
		// "An Ogre, wielding a club"
                strcat(info, " 을(를) 장비");
#endif
                strcat(tipbuf,info);
            }
        }

        if (mon_arm != NON_ITEM)
        {
            it_name( mon_arm, DESC_PLAIN, str_pass );
#ifdef JP 
	    // "오거, 곤봉을 장비, 갑옷을 착용"
            snprintf( info, INFO_SIZE, ", %s 을(를) 착용", 
#else
	    // "An Ogre, wielding a club, wearing armour"
            snprintf( info, INFO_SIZE, ", wearing %s", 
#endif
                      str_pass );

            strcat(tipbuf, info );
        }


        if (menv[i].type == MONS_HYDRA)
        {
#ifdef JP 
            snprintf( info, INFO_SIZE, ", %d개의 머리", menv[i].number );
#else
            snprintf( info, INFO_SIZE, ", with %d heads", menv[i].number );
#endif
            strcat(tipbuf, info );
        }

        if (mons_is_mimic( menv[i].type ))
            mimic_item = true;
        else if (!mons_flag(menv[i].type, M_NO_EXP_GAIN))
        {
            if (menv[i].behaviour == BEH_SLEEP)
            {
#ifdef JP 
                strcat(tipbuf, ", 당신을 알아차리지 못했다.");
#else
                strcat(tipbuf, ", not aware of you");
#endif
            }
            // wandering hostile with no target in LOS
            else if (menv[i].behaviour == BEH_WANDER && !mons_friendly(&menv[i])
                    && menv[i].foe == MHITNOT)
            {
                // special case: batty monsters get set to BEH_WANDER as
                // part of their special behaviour.
                if (!testbits(menv[i].flags, MF_BATTY))
                {
#ifdef JP 
                    strcat(tipbuf, ", 당신에게 흥미를 가지지 않는다.");
#else
                    strcat(tipbuf, ", not interested in you");
#endif
                }
            }
        }

        if (menv[i].attitude == ATT_FRIENDLY)
        {
#ifdef JP 
            strcat(tipbuf, ", 우호적");
#else
            strcat(tipbuf, ", friendly");
#endif
        }

        for (int p = 0; p < NUM_MON_ENCHANTS; p++)
        {
	    info[0]='\0';
            switch (menv[i].enchantment[p])
            {
            case ENCH_YOUR_ROT_I:
            case ENCH_YOUR_ROT_II:
            case ENCH_YOUR_ROT_III:
            case ENCH_YOUR_ROT_IV:
#ifdef JP 
                strcat(info, ", 썩어가고 있다."); //jmf: "covered in sores"?
#else
                strcat(info, ", rotting away"); //jmf: "covered in sores"?
#endif
                break;
            case ENCH_BACKLIGHT_I:
            case ENCH_BACKLIGHT_II:
            case ENCH_BACKLIGHT_III:
            case ENCH_BACKLIGHT_IV:
#ifdef JP 
                strcat(info, ", 부드럽게 빛나고 있다.");
#else
                strcat(info, ", softly glowing");
#endif
                break;
            case ENCH_SLOW:
#ifdef JP 
                strcat(info, ", 둔하게 움직이고 있다.");
#else
                strcat(info, ", moving slowly");
#endif
                break;
            case ENCH_HASTE:
#ifdef JP 
                strcat(info, ", 재빠르게 움직이고 있다.");
#else
                strcat(info, ", moving very quickly");
#endif
                break;
            case ENCH_CONFUSION:
#ifdef JP 
                strcat(info, ", 혼란스럽다고 느끼고 있다.");
#else
                strcat(info, ", bewildered and confused");
#endif
                break;
            case ENCH_INVIS:
#ifdef JP 
                strcat(info, ", 약간 투명");
#else
                strcat(info, ", slightly transparent");
#endif
                break;
            case ENCH_CHARM:
#ifdef JP 
                strcat(info, ", 당신의 노예");
#else
                strcat(info, ", in your thrall");
#endif
                break;
            case ENCH_YOUR_STICKY_FLAME_I:
            case ENCH_YOUR_STICKY_FLAME_II:
            case ENCH_YOUR_STICKY_FLAME_III:
            case ENCH_YOUR_STICKY_FLAME_IV:
            case ENCH_STICKY_FLAME_I:
            case ENCH_STICKY_FLAME_II:
            case ENCH_STICKY_FLAME_III:
            case ENCH_STICKY_FLAME_IV:
#ifdef JP 
                strcat(info, ", 불타고 끓어오르는 액체로 감싸져 있다.");
#else
                strcat(info, ", covered in liquid flames");
#endif
                break;
            default:
                info[0] = '\0';
                break;
            } // end switch
            if (info[0])
                strcat(tipbuf,info);
        }
    }

    if (tipbuf[0])
    {
        if (!mimic_item)
        {
#ifdef JP
            strcat(tipbuf, " (오른쪽 마우스 클릭으로 상세정보)");
#else
            strcat(tipbuf, " (R-Click for detail)");
#endif
        }
        return tipbuf;
    }


  look_clouds_tip:

    if (env.cgrid[mx][my] != EMPTY_CLOUD)
    {
        const char cloud_inspected = env.cgrid[mx][my];

        const char cloud_type = env.cloud[ cloud_inspected ].type;

#ifdef JP 
        info[0]=0;
#else
        strcpy(info, "A cloud of ");
#endif
        strcat(info,
            (cloud_type == CLOUD_FIRE
#ifdef JP 
              || cloud_type == CLOUD_FIRE_MON) ? "불꽃" :
#else
              || cloud_type == CLOUD_FIRE_MON) ? "flame" :
#endif
            (cloud_type == CLOUD_STINK
#ifdef JP 
              || cloud_type == CLOUD_STINK_MON) ? "유독한 연기의 구름 " :
#else
              || cloud_type == CLOUD_STINK_MON) ? "noxious fumes" :
#endif
            (cloud_type == CLOUD_COLD
#ifdef JP 
              || cloud_type == CLOUD_COLD_MON) ? "얼어붙는 기체의 구름" :
#else
              || cloud_type == CLOUD_COLD_MON) ? "freezing vapour" :
#endif
            (cloud_type == CLOUD_POISON
#ifdef JP 
              || cloud_type == CLOUD_POISON_MON) ? "독가스 구름" :
#else
              || cloud_type == CLOUD_POISON_MON) ? "poison gases" :
#endif
            (cloud_type == CLOUD_GREY_SMOKE
#ifdef JP 
              || cloud_type == CLOUD_GREY_SMOKE_MON) ? "잿빛 연기의 구름" :
#else
              || cloud_type == CLOUD_GREY_SMOKE_MON) ? "grey smoke" :
#endif
            (cloud_type == CLOUD_BLUE_SMOKE
#ifdef JP 
              || cloud_type == CLOUD_BLUE_SMOKE_MON) ? "푸른 연기의 구름" :
#else
              || cloud_type == CLOUD_BLUE_SMOKE_MON) ? "blue smoke" :
#endif
            (cloud_type == CLOUD_PURP_SMOKE
#ifdef JP 
              || cloud_type == CLOUD_PURP_SMOKE_MON) ? "보라색 연기의 구름" :
#else
              || cloud_type == CLOUD_PURP_SMOKE_MON) ? "purple smoke" :
#endif
            (cloud_type == CLOUD_STEAM
#ifdef JP 
              || cloud_type == CLOUD_STEAM_MON) ? "증기의 구름" :
#else
              || cloud_type == CLOUD_STEAM_MON) ? "steam" :
#endif
            (cloud_type == CLOUD_MIASMA
#ifdef JP 
              || cloud_type == CLOUD_MIASMA_MON) ? "질병의 장독" :
#else
              || cloud_type == CLOUD_MIASMA_MON) ? "foul pestilence" :
#endif
            (cloud_type == CLOUD_BLACK_SMOKE
#ifdef JP 
              || cloud_type == CLOUD_BLACK_SMOKE_MON) ? "검은 연기 구름"
                                                      : "버기신의 구름?");
#else
              || cloud_type == CLOUD_BLACK_SMOKE_MON) ? "black smoke"
                                                      : "buggy goodness");
#endif

        strcat(tipbuf, info);
    }

    int targ_item = igrd[ mx ][ my ];

    if (targ_item != NON_ITEM)
    {
        if (mitm[ targ_item ].base_type == OBJ_GOLD)
        {
#ifdef JP 
            strcat(tipbuf, "한 무더기의 금화" );
#else
            strcat(tipbuf, "A pile of gold coins" );
#endif
        }
        else
        {
            it_name( targ_item, DESC_NOCAP_A, info);
            strcat(tipbuf,info);
        }

/***
        if (mitm[ targ_item ].link != NON_ITEM)
#ifdef JP 
            strcat(tipbuf, "아래에 무엇인가 있다.");
#else
            strcat(tipbuf, ", something lies underneath.");
#endif
***/
    } // != NON_ITEM

    if(tipbuf[0])return tipbuf;

    switch (grd[mx][my])
    {
    case DNGN_STONE_WALL:
#ifdef JP 
        strcat(tipbuf,"지형:돌벽");
#else
        strcat(tipbuf,"A stone wall.");
#endif
        break;
    case DNGN_ROCK_WALL:
    case DNGN_SECRET_DOOR:
        if (you.level_type == LEVEL_PANDEMONIUM)
#ifdef JP 
            strcat(tipbuf,"지형:판데모니움을 구성하는 기분 나쁜 재질의 벽");
#else
            strcat(tipbuf,"A wall of the weird stuff which makes up Pandemonium.");
#endif
        else
#ifdef JP 
            strcat(tipbuf,"지형:암벽");
#else
            strcat(tipbuf,"A rock wall.");
#endif
        break;
    case DNGN_PERMAROCK_WALL:
#ifdef JP 
        strcat(tipbuf,"지형:비정상적으로 단단한 암벽");
#else
        strcat(tipbuf,"An unnaturally hard rock wall.");
#endif
        break;    
    case DNGN_CLOSED_DOOR:
#ifdef JP 
        strcat(tipbuf,"지형:닫혀진 문");
#else
        strcat(tipbuf,"A closed door.");
#endif
        break;
    case DNGN_METAL_WALL:
#ifdef JP 
        strcat(tipbuf,"지형:금속의 벽");
#else
        strcat(tipbuf,"A metal wall.");
#endif
        break;
    case DNGN_GREEN_CRYSTAL_WALL:
#ifdef JP 
        strcat(tipbuf,"지형:녹색수정의 벽");
#else
        strcat(tipbuf,"A wall of green crystal.");
#endif
        break;
    case DNGN_ORCISH_IDOL:
#ifdef JP 
        strcat(tipbuf,"지형:오크의 상");
#else
        strcat(tipbuf,"An orcish idol.");
#endif
        break;
    case DNGN_WAX_WALL:
#ifdef JP 
        strcat(tipbuf,"지형:밀납의 벽");
#else
        strcat(tipbuf,"A wall of solid wax.");
#endif
        break;
    case DNGN_SILVER_STATUE:
#ifdef JP 
        strcat(tipbuf,"지형:은의 상");
#else
        strcat(tipbuf,"A silver statue.");
#endif
        break;
    case DNGN_GRANITE_STATUE:
#ifdef JP 
        strcat(tipbuf,"지형:화강암의 상");
#else
        strcat(tipbuf,"A granite statue.");
#endif
        break;
    case DNGN_ORANGE_CRYSTAL_STATUE:
#ifdef JP 
        strcat(tipbuf,"지형:오렌지색 수정의 상");
#else
        strcat(tipbuf,"An orange crystal statue.");
#endif
        break;
    case DNGN_LAVA:
#ifdef JP 
        strcat(tipbuf,"지형:용암");
#else
        strcat(tipbuf,"Some lava.");
#endif
        break;
    case DNGN_DEEP_WATER:
#ifdef JP 
        strcat(tipbuf,"지형:깊은 물");
#else
        strcat(tipbuf,"Some deep water.");
#endif
        break;
    case DNGN_SHALLOW_WATER:
#ifdef JP 
        strcat(tipbuf,"지형:얕은 물");
#else
        strcat(tipbuf,"Some shallow water.");
#endif
        break;
    case DNGN_UNDISCOVERED_TRAP:
    case DNGN_FLOOR:
#ifdef JP 
        strcat(tipbuf,"지형:지면");
#else
        strcat(tipbuf,"Floor.");
#endif
        break;
    case DNGN_OPEN_DOOR:
#ifdef JP 
        strcat(tipbuf,"지형:열린 문");
#else
        strcat(tipbuf,"An open door.");
#endif
        break;
    case DNGN_ROCK_STAIRS_DOWN:
#ifdef JP 
        strcat(tipbuf,"지형:밑으로 향하는 바위계단");
#else
        strcat(tipbuf,"A rock staircase leading down.");
#endif
        break;
    case DNGN_STONE_STAIRS_DOWN_I:
    case DNGN_STONE_STAIRS_DOWN_II:
    case DNGN_STONE_STAIRS_DOWN_III:
#ifdef JP 
        strcat(tipbuf,"지형:밑으로 향하는 돌계단");
#else
        strcat(tipbuf,"A stone staircase leading down.");
#endif
        break;
    case DNGN_ROCK_STAIRS_UP:
#ifdef JP 
        strcat(tipbuf,"지형:위로 향하는 바위계단");
#else
        strcat(tipbuf,"A rock staircase leading upwards.");
#endif
        break;
    case DNGN_STONE_STAIRS_UP_I:
    case DNGN_STONE_STAIRS_UP_II:
    case DNGN_STONE_STAIRS_UP_III:
#ifdef JP 
        strcat(tipbuf,"지형:위로 향하는 돌계단");
#else
        strcat(tipbuf,"A stone staircase leading up.");
#endif
        break;
    case DNGN_ENTER_HELL:
#ifdef JP 
        strcat(tipbuf,"지형:지옥으로 가는 문");
#else
        strcat(tipbuf,"A gateway to hell.");
#endif
        break;
    case DNGN_BRANCH_STAIRS:
#ifdef JP 
        strcat(tipbuf,"지형:분기층으로의 계단");
#else
        strcat(tipbuf,"A staircase to a branch level.");
#endif
        break;
    case DNGN_TRAP_MECHANICAL:
    case DNGN_TRAP_MAGICAL:
    case DNGN_TRAP_III:
        for (trf = 0; trf < MAX_TRAPS; trf++)
        {
            if (env.trap[trf].x == mx
                && env.trap[trf].y == my)
            {
                break;
            }

            if (trf == MAX_TRAPS - 1)
            {
#ifdef JP 
                strcat(tipbuf,"Error - couldn't find that trap.");
#else
                strcat(tipbuf,"Error - couldn't find that trap.");
#endif
                error_message_to_player();
                break;
            }
        }

        switch (env.trap[trf].type)
        {
        case TRAP_DART:
#ifdef JP 
            strcat(tipbuf,"지형:다트 함정");
#else
            strcat(tipbuf,"A dart trap.");
#endif
            break;
        case TRAP_ARROW:
#ifdef JP 
            strcat(tipbuf,"지형:화살 함정");
#else
            strcat(tipbuf,"An arrow trap.");
#endif
            break;
        case TRAP_SPEAR:
#ifdef JP 
            strcat(tipbuf,"지형:창 함정");
#else
            strcat(tipbuf,"A spear trap.");
#endif
            break;
        case TRAP_AXE:
#ifdef JP 
            strcat(tipbuf,"지형:도끼 함정");
#else
            strcat(tipbuf,"An axe trap.");
#endif
            break;
        case TRAP_TELEPORT:
#ifdef JP 
            strcat(tipbuf,"지형:공간이동의 함정");
#else
            strcat(tipbuf,"A teleportation trap.");
#endif
            break;
        case TRAP_AMNESIA:
#ifdef JP 
            strcat(tipbuf,"지형:기억상실의 함정");
#else
            strcat(tipbuf,"An amnesia trap.");
#endif
            break;
        case TRAP_BLADE:
#ifdef JP 
            strcat(tipbuf,"지형:칼날 함정");
#else
            strcat(tipbuf,"A blade trap.");
#endif
            break;
        case TRAP_BOLT:
#ifdef JP 
            strcat(tipbuf,"지형:석궁 함정");
#else
            strcat(tipbuf,"A bolt trap.");
#endif
            break;
        case TRAP_ZOT:
#ifdef JP 
            strcat(tipbuf,"지형:조트의 함정");
#else
            strcat(tipbuf,"A Zot trap.");
#endif
            break;
        case TRAP_NEEDLE:
#ifdef JP 
            strcat(tipbuf,"지형:바늘 함정");
#else
            strcat(tipbuf,"A needle trap.");
#endif
            break;
        default:
#ifdef JP 
            strcat(tipbuf,"지형:정의되지 않은 함정!!!!");
#else
            strcat(tipbuf,"An undefined trap. Huh?");
#endif
            error_message_to_player();
            break;
        }
        break;
    case DNGN_ENTER_SHOP:
        strcat(tipbuf,shop_name(mx, my));
        break;
    case DNGN_ENTER_LABYRINTH:
#ifdef JP 
        strcat(tipbuf,"지형:미궁으로의 입구");
#else
        strcat(tipbuf,"A labyrinth entrance.");
#endif
        break;
    case DNGN_ENTER_DIS:
#ifdef JP 
        strcat(tipbuf,"지형:철의 도시 디스로의 문");
#else
        strcat(tipbuf,"A gateway to the Iron City of Dis.");
#endif
        break;
    case DNGN_ENTER_GEHENNA:
#ifdef JP 
        strcat(tipbuf,"지형:게헨나로 통하는 문");
#else
        strcat(tipbuf,"A gateway to Gehenna.");
#endif
        break;
    case DNGN_ENTER_COCYTUS:
#ifdef JP 
        strcat(tipbuf,"지형:얼어붙은 폐허 코키투스로의 문");
#else
        strcat(tipbuf,"A gateway to the freezing wastes of Cocytus.");
#endif
        break;
    case DNGN_ENTER_TARTARUS:
#ifdef JP 
        strcat(tipbuf,"지형:썩어가는 지옥 타르타루스로의 문");
#else
        strcat(tipbuf,"A gateway to the decaying netherworld of Tartarus.");
#endif
        break;
    case DNGN_ENTER_ABYSS:
#ifdef JP 
        strcat(tipbuf,"지형:무한지옥 어비스로의 문");
#else
        strcat(tipbuf,"A gateway to the infinite Abyss.");
#endif
        break;
    case DNGN_EXIT_ABYSS:
#ifdef JP 
        strcat(tipbuf,"지형:어비스의 탈출구");
#else
        strcat(tipbuf,"A gateway leading out of the Abyss.");
#endif
        break;
    case DNGN_STONE_ARCH:
#ifdef JP 
        strcat(tipbuf,"지형:고대의 돌로 만들어진 아치");
#else
        strcat(tipbuf,"An empty arch of ancient stone.");
#endif
        break;
    case DNGN_ENTER_PANDEMONIUM:
#ifdef JP 
        strcat(tipbuf,"지형:판데모니움의 큰 방으로 이어지는 문");
#else
        strcat(tipbuf,"A gate leading to the halls of Pandemonium.");
#endif
        break;
    case DNGN_EXIT_PANDEMONIUM:
#ifdef JP 
        strcat(tipbuf,"판데모니움으로부터의 출구");
#else
        strcat(tipbuf,"A gate leading out of Pandemonium.");
#endif
        break;
    case DNGN_TRANSIT_PANDEMONIUM:
#ifdef JP 
        strcat(tipbuf,"지형:판데모니움의 다른지역으로의 문");
#else
        strcat(tipbuf,"A gate leading to another region of Pandemonium.");
#endif
        break;
    case DNGN_ENTER_ORCISH_MINES:
#ifdef JP 
        strcat(tipbuf,"지형:오크 광산으로의 계단");
#else
        strcat(tipbuf,"A staircase to the Orcish Mines.");
#endif
        break;
    case DNGN_ENTER_HIVE:
#ifdef JP 
        strcat(tipbuf,"지형:벌집으로의 계단");
#else
        strcat(tipbuf,"A staircase to the Hive.");
#endif
        break;
    case DNGN_ENTER_LAIR:
#ifdef JP 
        strcat(tipbuf,"지형:짐승소굴로의 계단");
#else
        strcat(tipbuf,"A staircase to the Lair.");
#endif
        break;
    case DNGN_ENTER_SLIME_PITS:
#ifdef JP 
        strcat(tipbuf,"지형:슬라임의 소굴 로의 계단");
#else
        strcat(tipbuf,"A staircase to the Slime Pits.");
#endif
        break;
    case DNGN_ENTER_VAULTS:
#ifdef JP 
        strcat(tipbuf,"지형:보물창고로의 계단");
#else
        strcat(tipbuf,"A staircase to the Vaults.");
#endif
        break;
    case DNGN_ENTER_CRYPT:
#ifdef JP 
        strcat(tipbuf,"지형:납골당으로의 계단");
#else
        strcat(tipbuf,"A staircase to the Crypt.");
#endif
        break;
    case DNGN_ENTER_HALL_OF_BLADES:
#ifdef JP 
        strcat(tipbuf,"지형:칼날의 방으로의 계단");
#else
        strcat(tipbuf,"A staircase to the Hall of Blades.");
#endif
        break;
    case DNGN_ENTER_ZOT:
#ifdef JP 
        strcat(tipbuf,"지형:조트의 방으로의 문");
#else
        strcat(tipbuf,"A gate to the Realm of Zot.");
#endif
        break;
    case DNGN_ENTER_TEMPLE:
#ifdef JP 
        strcat(tipbuf,"지형:모든 종교의 사원으로의 계단");
#else
        strcat(tipbuf,"A staircase to the Ecumenical Temple.");
#endif
        break;
    case DNGN_ENTER_SNAKE_PIT:
#ifdef JP 
        strcat(tipbuf,"지형:뱀의 소굴로의 계단");
#else
        strcat(tipbuf,"A staircase to the Snake Pit.");
#endif
        break;
    case DNGN_ENTER_ELVEN_HALLS:
#ifdef JP 
        strcat(tipbuf,"지형:엘프의 방으로의 계단");
#else
        strcat(tipbuf,"A staircase to the Elven Halls.");
#endif
        break;
    case DNGN_ENTER_TOMB:
#ifdef JP 
        strcat(tipbuf,"지형:무덤으로의 계단");
#else
        strcat(tipbuf,"A staircase to the Tomb.");
#endif
        break;
    case DNGN_ENTER_SWAMP:
#ifdef JP 
        strcat(tipbuf,"지형:늪지로의 계단");
#else
        strcat(tipbuf,"A staircase to the Swamp.");
#endif
        break;
    case DNGN_RETURN_FROM_ORCISH_MINES:
    case DNGN_RETURN_FROM_HIVE:
    case DNGN_RETURN_FROM_LAIR:
    case DNGN_RETURN_FROM_VAULTS:
    case DNGN_RETURN_FROM_TEMPLE:
#ifdef JP 
        strcat(tipbuf,"지형:던젼으로 돌아가는 출구의 계단");
#else
        strcat(tipbuf,"A staircase back to the Dungeon.");
#endif
        break;
    case DNGN_RETURN_FROM_SLIME_PITS:
    case DNGN_RETURN_FROM_SNAKE_PIT:
    case DNGN_RETURN_FROM_SWAMP:
#ifdef JP 
        strcat(tipbuf,"짐승 소굴로 돌아가는 출구의 계단");
#else
        strcat(tipbuf,"A staircase back to the Lair.");
#endif
        break;
    case DNGN_RETURN_FROM_CRYPT:
    case DNGN_RETURN_FROM_HALL_OF_BLADES:
#ifdef JP 
        strcat(tipbuf,"지형:보물창고로 돌아가는 출구의 계단");
#else
        strcat(tipbuf,"A staircase back to the Vaults.");
#endif
        break;
    case DNGN_RETURN_FROM_ELVEN_HALLS:
#ifdef JP 
        strcat(tipbuf,"지형:오크 광산으로 돌아가는 출구의 계단");
#else
        strcat(tipbuf,"A staircase back to the Mines.");
#endif
        break;
    case DNGN_RETURN_FROM_TOMB:
#ifdef JP 
        strcat(tipbuf,"지형:무덤으로 돌아가는 출구의 계단");
#else
        strcat(tipbuf,"A staircase back to the Crypt.");
#endif
        break;
    case DNGN_RETURN_FROM_ZOT:
#ifdef JP 
        strcat(tipbuf,"지형: 이 장소에서 밖으로 나가는 문");
#else
        strcat(tipbuf,"A gate leading back out of this place.");
#endif
        break;
    case DNGN_ALTAR_ZIN:
#ifdef JP 
        strcat(tipbuf,"지형: 『진』의 빛나는 흰 대리석 제단");
#else
        strcat(tipbuf,"A glowing white marble altar of Zin.");
#endif
        break;
    case DNGN_ALTAR_SHINING_ONE:
#ifdef JP 
        strcat(tipbuf,"지형:『샤이닝 원』 의 빛나는 황금제단");
#else
        strcat(tipbuf,"A glowing golden altar of the Shining One.");
#endif
        break;
    case DNGN_ALTAR_KIKUBAAQUDGHA:
#ifdef JP 
        strcat(tipbuf,"지형:『키쿠바쿠드하』 의 고대의 뼈로 만들어진 제단");
#else
        strcat(tipbuf,"An ancient bone altar of Kikubaaqudgha.");
#endif
        break;
    case DNGN_ALTAR_YREDELEMNUL:
#ifdef JP 
        strcat(tipbuf,"지형:『이레델렘눌 』 의 현무암으로 만들어진 제단");
#else
        strcat(tipbuf,"A basalt altar of Yredelemnul.");
#endif
        break;
    case DNGN_ALTAR_XOM:
#ifdef JP 
        strcat(tipbuf,"지형:『죰』 의 미광이 흔들리는 제단");
#else
        strcat(tipbuf,"A shimmering altar of Xom.");
#endif
        break;
    case DNGN_ALTAR_VEHUMET:
#ifdef JP 
        strcat(tipbuf,"지형:『베후멧』 의 빛을 발하는 제단");
#else
        strcat(tipbuf,"A shining altar of Vehumet.");
#endif
        break;
    case DNGN_ALTAR_OKAWARU:
#ifdef JP 
        strcat(tipbuf,"지형:『오카와루』 의 철의 제단 ");
#else
        strcat(tipbuf,"An iron altar of Okawaru.");
#endif
        break;
    case DNGN_ALTAR_MAKHLEB:
#ifdef JP 
        strcat(tipbuf,"지형:『마크레브』 의 불길이 이는 제단");
#else
        strcat(tipbuf,"A burning altar of Makhleb.");
#endif
        break;
    case DNGN_ALTAR_SIF_MUNA:
#ifdef JP 
        strcat(tipbuf,"지형:『시프 무나』 의 감청색의 제단");
#else
        strcat(tipbuf,"A deep blue altar of Sif Muna.");
#endif
        break;
    case DNGN_ALTAR_TROG:
#ifdef JP 
        strcat(tipbuf,"지형:『트로그』의 피에 물든 제단");
#else
        strcat(tipbuf,"A bloodstained altar of Trog.");
#endif
        break;
    case DNGN_ALTAR_NEMELEX_XOBEH:
#ifdef JP 
        strcat(tipbuf,"지형:『네메렉스 조베』 의 빛나는 제단");
#else
        strcat(tipbuf,"A sparkling altar of Nemelex Xobeh.");
#endif
        break;
    case DNGN_ALTAR_ELYVILON:
#ifdef JP 
        strcat(tipbuf,"『엘리빌론』 의 은 제단");
#else
        strcat(tipbuf,"A silver altar of Elyvilon.");
#endif
        break;
    case DNGN_BLUE_FOUNTAIN:
#ifdef JP 
        strcat(tipbuf,"지형:맑고 푸른 물의 샘");
#else
        strcat(tipbuf,"A fountain of clear blue water.");
#endif
        break;
    case DNGN_SPARKLING_FOUNTAIN:
#ifdef JP 
        strcat(tipbuf,"지형:빛나는 물의 샘");
#else
        strcat(tipbuf,"A fountain of sparkling water.");
#endif
        break;
    case DNGN_DRY_FOUNTAIN_I:
    case DNGN_DRY_FOUNTAIN_II:
    case DNGN_DRY_FOUNTAIN_IV:
    case DNGN_DRY_FOUNTAIN_VI:
    case DNGN_DRY_FOUNTAIN_VIII:
    case DNGN_PERMADRY_FOUNTAIN:
#ifdef JP 
        strcat(tipbuf,"지형:바싹 마른 샘");
#else
        strcat(tipbuf,"A dry fountain.");
#endif
        break;
    }

    return tipbuf;
}
#endif

