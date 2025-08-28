/*
 *  File:       invent.cc
 *  Summary:    Functions for inventory related commands.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <5>     10/9/99     BCR     Added wizard help screen
 *      <4>     10/1/99     BCR     Clarified help screen
 *      <3>     6/9/99      DML     Autopickup
 *      <2>     5/20/99     BWR     Extended screen lines support
 *      <1>     -/--/--     LRH     Created
 */

#include "AppHdr.h"
#include "invent.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "itemname.h"
#include "items.h"
#include "macro.h"
#include "player.h"
#include "shopping.h"
#include "stuff.h"
#include "view.h"

#ifdef USE_TILE
#include "tiles.h"
#endif

#if 1 //Slot

#include "item_use.h"
#include "it_use3.h"
#include "food.h"
#include "stash.h"

static int pushed_idx = -1;
static int pushed_count = -1;

void push_inven_idx(int idx)
{
    pushed_idx = idx;
}

void push_inven_count(int c)
{
    pushed_count = c;
}
int pop_inven_idx()
{
    int r = pushed_idx;
    pushed_idx = -1;
    return r;
}

int pop_inven_count()
{
    int r = pushed_count;
    pushed_count = -1;
    return r;
}
#endif


const char *command_string( int i );
const char *wizard_string( int i );

unsigned char get_invent( int invent_type )
{
    unsigned char nothing = invent( invent_type, false );

    redraw_screen();

    return (nothing);
}                               // end get_invent()

unsigned char invent( int item_class_inv, bool show_price )
{
    char st_pass[ ITEMNAME_SIZE ] = "";

    int i, j;
    char lines = 0;
    unsigned char anything = 0;
    char tmp_quant[20] = "";
    char yps = 0;
    char temp_id[4][50];

    const int num_lines = get_number_of_lines();

    FixedVector< int, NUM_OBJECT_CLASSES >  inv_class2;
    int inv_count = 0;
    unsigned char ki = 0;

#ifdef DOS_TERM
    char buffer[4600];

    gettext(1, 1, 80, 25, buffer);
    window(1, 1, 80, 25);
#endif

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 50; j++)
        {
            temp_id[i][j] = 1;
        }
    }

    clrscr();

#if 1 //Slot
#ifdef USE_TILE
    if (Options.use_tile)
    {
        if (item_class_inv == -1)
            TileDrawInvenAux(-2, 2);
        else
            TileDrawInvenAux(item_class_inv, 2);
    }
#endif
#endif

    for (i = 0; i < NUM_OBJECT_CLASSES; i++)
        inv_class2[i] = 0;

    for (i = 0; i < ENDOFPACK; i++)
    {
        if (you.inv[i].quantity)
        {
            inv_class2[ you.inv[i].base_type ]++;
            inv_count++;
        }
    }

    if (!inv_count)
    {
#ifdef JP
        cprintf("¾Æ¹«°Íµµ °¡Áø°Ô ¾ø´Ù.");
#else
        cprintf("You aren't carrying anything.");
#endif

        if (getch() == 0)
            getch();

        goto putty;
    }

    if (item_class_inv != -1)
    {
        for (i = 0; i < NUM_OBJECT_CLASSES; i++)
        {
            if (item_class_inv == OBJ_MISSILES && i == OBJ_WEAPONS)
                i++;

            if (item_class_inv == OBJ_WEAPONS
                && (i == OBJ_STAVES || i == OBJ_MISCELLANY))
            {
                i++;
            }

            if (item_class_inv == OBJ_SCROLLS && i == OBJ_BOOKS)
                i++;

            if (i < NUM_OBJECT_CLASSES && item_class_inv != i)
                inv_class2[i] = 0;
        }
    }

    if ((item_class_inv == -1 && inv_count > 0)
        || (item_class_inv != -1 && inv_class2[item_class_inv] > 0)
        || (item_class_inv == OBJ_MISSILES && inv_class2[OBJ_WEAPONS] > 0)
        || (item_class_inv == OBJ_WEAPONS
            && (inv_class2[OBJ_STAVES] > 0 || inv_class2[OBJ_MISCELLANY] > 0))
        || (item_class_inv == OBJ_SCROLLS && inv_class2[OBJ_BOOKS] > 0))
    {
        const int cap = carrying_capacity();

#ifdef JP
        cprintf( "  ¼ÒÁöÇ°: %d.%d aum (ÇÑ°è %d.%d aum ÀÇ %d%% )",
                 you.burden / 10, you.burden % 10,
                 cap / 10, cap % 10, (you.burden * 100) / cap );
#else
        cprintf( "  Inventory: %d.%d aum (%d%% of %d.%d aum maximum)",
                 you.burden / 10, you.burden % 10,
                 (you.burden * 100) / cap, cap / 10, cap % 10 );
#endif
        lines++;

        for (i = 0; i < 15; i++)
        {
            if (inv_class2[i] != 0)
            {
                if (lines > num_lines - 3)
                {
                    gotoxy(1, num_lines);

#ifdef JP
                    cprintf("-´ÙÀ½Àå-");
#else
                    cprintf("-more-");
#endif

                    set_keyin_mode(KEYIN_MODE_INVENT);
                    ki = getch();
                    set_keyin_mode(KEYIN_MODE_NONE);

                    if (ki == ESCAPE)
                    {
#ifdef DOS_TERM
                        puttext(1, 1, 80, 25, buffer);
#endif
                        return (ESCAPE);
                    }
                    else if (isalpha(ki) || ki == '?' || ki == '*' || ki== '$' || isdigit( ki ) )
                    {
#ifdef DOS_TERM
                        puttext(1, 1, 80, 25, buffer);
#endif
                        return (ki);
                    }

                    if (ki == 0)
                        ki = getch();

                    lines = 0;
                    clrscr();
                    gotoxy(1, 1);
                    anything = 0;

                }

                if (lines > 0)
                    cprintf(EOL " ");

                textcolor(BLUE);

                switch (i)
                {
#ifdef JP   // ÄÄÆÄÀÏ½Ã È®ÀÎ¹Ù¶÷ (°¨ÀÚ)
                case OBJ_WEAPONS:    cprintf("¹«±â");            break;
                case OBJ_MISSILES:   cprintf("½î´Â ¹«±â");        break;
                case OBJ_ARMOUR:     cprintf("°©¿Ê");            break;
                case OBJ_WANDS:      cprintf("¸¶¹ýºÀ");        break;
                case OBJ_FOOD:       cprintf("À½½Ä");            break;
                case OBJ_UNKNOWN_I:  cprintf("¹ÌÈ®ÀÎ1");            break;
                case OBJ_SCROLLS:    cprintf("¸¶¹ý µÎ·ç¸¶¸®");            break;
                case OBJ_JEWELLERY:  cprintf("Àå½Å±¸");          break;
                case OBJ_POTIONS:    cprintf("¹°¾à");              break;
                case OBJ_UNKNOWN_II: cprintf("¹ÌÈ®ÀÎ2");        break;
                case OBJ_BOOKS:      cprintf("¼­Àû");            break;
                case OBJ_STAVES:     cprintf("ÁöÆÎÀÌ"); break;
                case OBJ_ORBS:       cprintf("¿Àºê");        break;
                case OBJ_MISCELLANY: cprintf("±âÅ¸");          break;
                case OBJ_CORPSES:    cprintf("½ÃÃ¼");            break;
#else
                case OBJ_WEAPONS:    cprintf("Hand Weapons");    break;
                case OBJ_MISSILES:   cprintf("Missiles");        break;
                case OBJ_ARMOUR:     cprintf("Armour");          break;
                case OBJ_WANDS:      cprintf("Magical Devices"); break;
                case OBJ_FOOD:       cprintf("Comestibles");     break;
                case OBJ_UNKNOWN_I:  cprintf("Books");           break;
                case OBJ_SCROLLS:    cprintf("Scrolls");         break;
                case OBJ_JEWELLERY:  cprintf("Jewellery");       break;
                case OBJ_POTIONS:    cprintf("Potions");         break;
                case OBJ_UNKNOWN_II: cprintf("Gems");            break;
                case OBJ_BOOKS:      cprintf("Books");           break;
                case OBJ_STAVES:     cprintf("Magical Staves and Rods");  break;
                case OBJ_ORBS:       cprintf("Orbs of Power");   break;
                case OBJ_MISCELLANY: cprintf("Miscellaneous");   break;
                case OBJ_CORPSES:    cprintf("Carrion");         break;
#endif
                //case OBJ_GEMSTONES: cprintf("Miscellaneous"); break;
                }
                textcolor(LIGHTGREY);
                lines++;

                for (j = 0; j < ENDOFPACK; j++)
                {
                    if (lines > num_lines - 2 && inv_count > 0)
                    {
                        gotoxy(1, num_lines);

#ifdef JP
                        cprintf("-´ÙÀ½Àå-");
#else
                        cprintf("-more-");
#endif

                        set_keyin_mode(KEYIN_MODE_INVENT);
                        ki = getch();
                        set_keyin_mode(KEYIN_MODE_NONE);

                        if (ki == ESCAPE)
                        {
#ifdef DOS_TERM
                            puttext(1, 1, 80, 25, buffer);
#endif
                            return (ESCAPE);
                        }
                        else if (isalpha(ki) || ki == '?' || ki == '*' || ki== '$' || isdigit( ki ) )
                        {
#ifdef DOS_TERM
                            puttext(1, 1, 80, 25, buffer);
#endif
                            return (ki);
                        }

                        if (ki == 0)
                            ki = getch();

                        lines = 0;
                        clrscr();
                        gotoxy(1, 1);
                        anything = 0;
                    }

                    if (is_valid_item(you.inv[j]) && you.inv[j].base_type==i)
                    {
                        anything++;

                        if (lines > 0)
                            cprintf(EOL);

                        lines++;

                        yps = wherey();

                        in_name( j, DESC_INVENTORY_EQUIP, st_pass );

                        //!!!!Žô‚¢‚ðŠÅ”jÏ‚Ý‚ÌŽô‚í‚ê‚½•i‚ðÔF‚Å•\Ž¦‚·‚éƒIƒvƒVƒ‡ƒ“
                        //!!!!‘•”õ’†‚ÌƒAƒCƒeƒ€‚Í—Î‚Å•\Ž¦‚·‚é
                        if (Options.stress_cursed)
                        {
                            if ( (j == you.equip[EQ_WEAPON]     )
                               ||(j == you.equip[EQ_CLOAK]      )
                               ||(j == you.equip[EQ_HELMET]     )
                               ||(j == you.equip[EQ_GLOVES]     )
                               ||(j == you.equip[EQ_BOOTS]      )
                               ||(j == you.equip[EQ_SHIELD]     )
                               ||(j == you.equip[EQ_BODY_ARMOUR])
                               ||(j == you.equip[EQ_LEFT_RING]  )
                               ||(j == you.equip[EQ_RIGHT_RING] )
                               ||(j == you.equip[EQ_AMULET]     ) )
#ifdef WINDOWS
                                textcolor(LIGHTGREEN);
#else
                                textcolor(YELLOW);
#endif
                            if ( item_cursed(you.inv[j]) && item_ident( you.inv[j], ISFLAG_KNOW_CURSE ) )
                                textcolor(LIGHTRED);
                            cprintf( st_pass );
                            textcolor(LIGHTGREY);
                        }
                        else
                            cprintf( st_pass );
                        inv_count--;


                        if (show_price)
                        {
                            cprintf(" (");

                            itoa( item_value( you.inv[j], temp_id, true ),
                                  tmp_quant, 10 );

                            cprintf( tmp_quant );
#ifdef JP
                            cprintf( " °ñµå)" );
#else
                            cprintf( " gold)" );
#endif
                        }

                        if (wherey() != yps)
                            lines++;
                    }
                }               // end of j loop
            }                   // end of if inv_class2
        }                       // end of i loop.
    }
    else
    {
        if (item_class_inv == -1)
#ifdef JP
            cprintf("¾Æ¹« °Íµµ °¡Áö°í ÀÖÁö ¾Ê´Ù.");
#else
            cprintf("You aren't carrying anything.");
#endif
        else
        {
            if (item_class_inv == OBJ_WEAPONS)
#ifdef JP
                cprintf("¹«±â°¡ ¾ø´Ù.");
#else
                cprintf("You aren't carrying any weapons.");
#endif
            else if (item_class_inv == OBJ_MISSILES)
#ifdef JP
                cprintf("Åº¾àÀÌ ¾ø´Ù.");
#else
                cprintf("You aren't carrying any ammunition.");
#endif
            else
#ifdef JP
                cprintf("ÇØ´ç ¾ÆÀÌÅÛÀÌ ¾ø´Ù.");
#else
                cprintf("You aren't carrying any such object.");
#endif

            anything++;
        }
    }

    if (anything > 0)
    {
        set_keyin_mode(KEYIN_MODE_INVENT);
        ki = getch();
        set_keyin_mode(KEYIN_MODE_NONE);

        if (isalpha(ki) || ki == '?' || ki == '*')
        {
#ifdef DOS_TERM
            puttext(1, 1, 80, 25, buffer);
#endif
            return (ki);
        }

        if (ki == 0)
            ki = getch();
    }

  putty:
#ifdef DOS_TERM
    puttext(1, 1, 80, 25, buffer);
#endif

    return (ki);
}                               // end invent()

/*
// Reads in digits for a count and apprends then to val, the
// return value is the character that stopped the reading.
static unsigned char get_invent_quant( unsigned char keyin, int &quant )
{
    quant = keyin - '0';
    int x, y;

    // Locate the cursor
    x = wherex();
    y = wherey();

    for(;;)
    {
        gotoxy (x, y);
#ifdef USE_TILE
        mpr_on(MODE_MPR);
        cprintf("%5d", quant);
        mpr_on(MODE_CRT);
#else
        cprintf("%5d", quant);
#endif

        keyin = get_ch();

        if ( isdigit( keyin ) )
        {
            quant *= 10;
            quant += (keyin - '0');
        }
        else if ( keyin == 8 )  // BackSpace
            quant /= 10;
        else
            break;

        if (quant > 99999)
        {
            quant = 99999;
            keyin = '\0';
            break;
        }
    }

    return (keyin);
}
*/

// Reads in digits for a count and apprends then to val, the
// return value is the character that stopped the reading.
unsigned char get_invent_quant( int &quant )
{
    int x, y;
    unsigned char keyin;

    // Locate the cursor
    x = wherex();
    y = wherey();

    for(;;)
    {
#ifdef USE_TILE
        mpr_on(MODE_MPR);
#endif
        gotoxy (x, y);
        cprintf("           ");
        if (quant >= 0 )
        {
            gotoxy (x, y);
            cprintf("%5d", quant);
        }
#ifdef USE_TILE
        mpr_on(MODE_CRT);
#endif
        keyin = get_ch();

        if ( isdigit( keyin ) )
        {
            if (quant < 0)
                quant = (keyin - '0');
            else
                quant = quant * 10 + (keyin - '0');
        }
        else if ( keyin == 8 )  // BackSpace
        {
            if (quant <= 0)
                quant = -1;
            else
                quant /= 10;
        }
        else
            break;

        if (quant > 99999)
        {
            quant = 99999;
        }
    }

    return (keyin);
}

// This function prompts the user for an item, handles the '?' and '*'
// listings, and returns the inventory slot to the caller (which if
// must_exist is true (the default) will be an assigned item, with
// a positive quantity.
//
// It returns PROMPT_ABORT       if the player hits escape.
// It returns PROMPT_GOT_SPECIAL if the player hits the "other_valid_char".
//
// Note: This function never checks if the item is appropriate.
int prompt_invent_item( const char *prompt, int type_expect,
                        bool must_exist, bool allow_auto_list,
                        bool allow_easy_quit,
                        const char other_valid_char,
                        int *const count )
{
    unsigned char  keyin = 0;
    int            ret = -1;

    bool           need_redraw = false;
    bool           need_prompt = true;
    bool           need_getch  = true;

#if 1 //Slot
    int r = pop_inven_idx();
    if (r != -1)
    {
        int c = pop_inven_count();
        if (c != -1) *count = c;
        return r;
    }
#endif

    if (Options.auto_list && allow_auto_list)
    {
        // pretend the player has hit '?' and setup state.
        keyin = invent( type_expect, false );

        need_getch = false;

        // Don't redraw if we're just going to display another listing
        need_redraw = (keyin != '?' && keyin != '*');

        // A prompt is nice for when we're moving to "count" mode.
        need_prompt = (count != NULL && isdigit( keyin ));
    }

    for (;;)
    {
        if (need_redraw)
        {
            redraw_screen();
            mesclr( true );
        }

        if (need_prompt)
            mpr( prompt, MSGCH_PROMPT );

#if 1 //Slot
        set_keyin_mode(KEYIN_MODE_INVENT);
#endif
        if (need_getch)
            keyin = get_ch();
#if 1 //Slot
        set_keyin_mode(KEYIN_MODE_NONE);
#endif

        need_redraw = false;
        need_prompt = true;
        need_getch  = true;

        // Note:  We handle any "special" character first, so that
        //        it can be used to override the others.
        if (other_valid_char != '\0' && keyin == other_valid_char)
        {
            ret = PROMPT_GOT_SPECIAL;
            break;
        }
        else if (keyin == '?' || keyin == '*')
        {
            // The "view inventory listing" mode.
            if (keyin == '*')
                keyin = invent( -1, false );
            else
                keyin = invent( type_expect, false );

            need_getch  = false;

            // Don't redraw if we're just going to display another listing
            need_redraw = (keyin != '?' && keyin != '*');

            // A prompt is nice for when we're moving to "count" mode.
            need_prompt = (count != NULL && isdigit( keyin ) );
        }
        else if (count != NULL && isdigit( keyin ) )
        {
            // The "read in quantity" mode
            //keyin = get_invent_quant( keyin, *count );
            *count = keyin - '0';
            keyin = get_invent_quant( *count );

            need_prompt = false;
            need_getch  = false;
        }
        else if (keyin == ESCAPE
                || (Options.easy_quit_item_prompts
                    && allow_easy_quit
                    && keyin == ' '))
        {
            ret = PROMPT_ABORT;
            break;
        }
        else if (isalpha( keyin ))
        {
            ret = letter_to_index( keyin );

            if (must_exist && !is_valid_item( you.inv[ret] ))
#ifdef JP
                mpr( "±×·± ¾ÆÀÌÅÛÀ» °¡Áö°í ÀÖÁö ¾Ê´Ù." );
#else
                mpr( "You do not have any such object." );
#endif
            else
                break;
        }
        else if (!isspace( keyin ))
        {
            // we've got a character we don't understand...
            canned_msg( MSG_HUH );
        }
    }

    return (ret);
}

void list_commands(bool wizard)
{
    const char *line;
    int j = 0;

#ifdef DOS_TERM
    char buffer[4800];

    window(1, 1, 80, 25);
    gettext(1, 1, 80, 25, buffer);
#endif

    clrscr();

    // BCR - Set to screen length - 1 to display the "more" string
    int moreLength = (get_number_of_lines() - 1) * 2;

    for (int i = 0; i < 500; i++)
    {
        if (wizard)
            line = wizard_string( i );
        else
            line = command_string( i );

        if (strlen( line ) != 0)
        {
            // BCR - If we've reached the end of the screen, clear
            if (j == moreLength)
            {
                gotoxy(2, j / 2 + 1);
#ifdef JP
                cprintf("´ÙÀ½Àå...");
#else
                cprintf("More...");
#endif
                getch();
                clrscr();
                j = 0;
            }

            gotoxy( ((j % 2) ? 40 : 2), ((j / 2) + 1) );
            cprintf( line );

            j++;
        }
    }

    getch();

#ifdef DOS_TERM
    puttext(1, 1, 80, 25, buffer);
#endif

    return;
}                               // end list_commands()

const char *wizard_string( int i )
{
    UNUSED( i );

#ifdef WIZARD
#ifdef JP  // ÄÄÆÄÀÏ½Ã È®ÀÎ¹Ù¶÷ (°¨ÀÚ)
    return((i ==  10) ? "a    : È¹µæ"                         :
           (i ==  13) ? "A    : ¸ðµç ±â¼úÀ» ÀÓÀÇÀÇ °ªÀ¸·Î ÇÑ´Ù"     :
           (i ==  15) ? "b    : Á¦¾î ¼ø°£ÀÌµ¿"           :
           (i ==  20) ? "B    : ¾îºñ½º¿¡ Àú¼ÓÇØÁø´Ù"               :
           (i ==  20) ? "C    : ????"         :
           (i ==  30) ? "g    : ÀÓÀÇÀÇ ±â¼úÀ» ¿Ã¸°´Ù"   :
           (i ==  35) ? "G    : ¸ðµç ¸ó½ºÅÍ¸¦ ¾ø¿£´Ù"       :
           (i ==  40) ? "h/H  :Ä¡À¯(È¸º¹)"                 :
           (i ==  50) ? "i/I  : ¼ÒÁöÇ° ÀÌÅÛ °¨Á¤"  :
           (i ==  70) ? "l    : ¹Ì±Ã ÀÔ±¸ »ý¼º"     :
           (i ==  80) ? "m/M  : ¹øÈ£/ÀÌ¸§À¸·Î ¸ó½ºÅÍ »ý¼º"    :
           (i ==  90) ? "o/%%  : ¾ÆÀÌÅÛ »ý¼º"                :
           (i == 100) ? "p    : ÆÇµ¥¸ð´Ï¿ò ÀÔ±¸ »ý¼º"   :
           (i == 110) ? "x    : ·¹º§¾÷"                 :
           (i == 115) ? "r    : Á¾Á· º¯°æ"                     :
           (i == 120) ? "s    : 20000 ±â¼ú Á¡¼ö È¹µæ"      :
           (i == 130) ? "S    :ÀÓÀÇÀÇ ±â¼ú°ª º¯°æ"       :
           (i == 140) ? "t    : ¾ÆÀÌÅÛ ¼º´É º¯°æ"       :
           (i == 150) ? "X    : Á»ÀÇ ¼±¹° ¹Þ±â"           :
           (i == 160) ? "z/Z  : ¹øÈ£/ÀÌ¸§ÀÇ ÁÖ¹® ½ÃÀü"        :
           (i == 200) ? "$    : 1000 °ñµå È¹µæ"             :
           (i == 210) ? "</>  : ¿À¸£´Â/³»·Á°¡´Â °è´Ü »ý¼º"          :
           (i == 220) ? "u/d  : À§/¾Æ·¡ ÃþÀ¸·Î ÀÌµ¿"                :
           (i == 230) ? "~/\"  : ÁöÁ¤ÃþÀ¸·Î °ø°£ÀÌµ¿"                :
           (i == 240) ? "(    : ÁöÇü ÀÛ¼º"                   :
           (i == 250) ? "]    : µ¹¿¬º¯ÀÌ È¹µæ"               :
           (i == 260) ? "[    : µ¥¸ó½ºÆùÀÇ º¯ÀÌ È¹µæ"   :
           (i == 270) ? ":    : ´øÁ¯ÀÇ ºÐ±âÃþ Ç¥½Ã"     :
           (i == 280) ? "{    : ¸¶¹ý Áöµµ"                   :
           (i == 290) ? "^    : ½Å¾ÓÄ¡ ¿Ã¸®±â"               :
           (i == 300) ? "_    : ÁöÁ¤ÇÑ ½ÅÀ» ¸ð½Å´Ù"         :
           (i == 310) ? "\'    : ¾ÆÀÌÅÛÀÇ ÁÂÇ¥ ¸®½ºÆ®"        :
           (i == 320) ? "?    : À§Àúµå Ä¿¸Çµå Ç¥½Ã"       :
           (i == 330) ? "|    : Àü ¾ÆÆ¼ÆÑÆ® »ý¼º"       :
           (i == 340) ? "+    : ·£´ý ¾ÆÆ¼ÆÑÆ® »ý¼º" :
           (i == 350) ? "=    : ±â¼ú Á¡¼öÀÇ ÇÕ°è Ç¥½Ã"
#else
    return((i ==  10) ? "a    : acquirement"                  :
           (i ==  13) ? "A    : set all skills to level"      :
           (i ==  15) ? "b    : controlled blink"             :
           (i ==  20) ? "B    : banish yourself to the Abyss" :
           (i ==  30) ? "g    : add a skill"                  :
           (i ==  35) ? "G    : remove all monsters"          :
           (i ==  40) ? "h/H  : heal yourself (super-Heal)"   :
           (i ==  50) ? "i/I  : identify/unidentify inventory":
           (i ==  70) ? "l    : make entrance to labyrinth"   :
           (i ==  80) ? "m/M  : create monster by number/name":
           (i ==  90) ? "o/%%  : create an object"            :
           (i == 100) ? "p    : make entrance to pandemonium" :
           (i == 110) ? "x    : gain an experience level"     :
           (i == 115) ? "r    : change character's species"   :
           (i == 120) ? "s    : gain 20000 skill points"      :
           (i == 130) ? "S    : set skill to level"           :
           (i == 140) ? "t    : tweak object properties"      :
           (i == 150) ? "X    : Receive a gift from Xom"      :
           (i == 160) ? "z/Z  : cast any spell by number/name":
           (i == 200) ? "$    : get 1000 gold"                :
           (i == 210) ? "</>  : create up/down staircase"     :
           (i == 220) ? "u/d  : shift up/down one level"      :
           (i == 230) ? "~/\"  : goto a level"                :
           (i == 240) ? "(    : create a feature"             :
           (i == 250) ? "]    : get a mutation"               :
           (i == 260) ? "[    : get a demonspawn mutation"    :
           (i == 270) ? ":    : find branch"                  :
           (i == 280) ? "{    : magic mapping"                :
           (i == 290) ? "^    : gain piety"                   :
           (i == 300) ? "_    : gain religion"                :
           (i == 310) ? "\'    : list items"                  :
           (i == 320) ? "?    : list wizard commands"         :
           (i == 330) ? "|    : acquire all unrand artefacts" :
           (i == 340) ? "+    : turn item into random artefact" :
           (i == 350) ? "=    : sum skill points"
#endif
                      : "");

#else
    return ("");
#endif
}                               // end wizard_string()

const char *command_string( int i )
{
    /*
     * BCR - Command printing, case statement
     * Note: The numbers in this case indicate the order in which the
     *       commands will be printed out.  Make sure none of these
     *       numbers is greater than 500, because that is the limit.
     *
     * Arranged alpha lower, alpha upper, punctuation, ctrl.
     *
     */

#ifdef JP
    return((i ==  10) ? "a    : Æ¯¼ö ´É·Â »ç¿ë(use Ability)"     :
           (i ==  20) ? "d(#) : ¾ÆÀÌÅÛ(#°¹¼ö) ¹ö¸²(Drop)"        :
           (i ==  30) ? "e    : À½½Ä ¸Ô±â(Eat)"                  :
           (i ==  40) ? "f    : ½î´Â ¹«±â ÀÚµ¿ ¼±ÅÃ ¹ß»ç(Fire)"  :
           (i ==  50) ? "i    : ¼ÒÁöÇ° ¸ñ·Ï(Inventory)"          :
           (i ==  55) ? "m    : ±â¼ú È­¸é"                       :
           (i ==  60) ? "o/c  : ¹® ¿­±â/´Ý±â(Open/Close)"        :
           (i ==  65) ? "p    : ½Å¿¡°Ô ±âµµÇÏ±â(Pray)"           :
           (i ==  70) ? "q    : ¹°¾à ¸¶½Ã±â(Quaff)"              :
           (i ==  80) ? "r    : Ã¥ÀÌ³ª ¸¶¹ý µÎ·ç¸¶¸® ÀÐ±â(Read)"        :
           (i ==  90) ? "s    : ÁÖº¯ Å½»ö(Search)"               :
           (i == 100) ? "t    : ¾ÆÀÌÅÛ ÅõÃ´(Throw)"              :
           (i == 110) ? "v    : ¾ÆÀÌÅÛÀÇ »ó¼¼ ¼³¸í º¸±â(View)"   :
           (i == 120) ? "w    : ¾ÆÀÌÅÛ ÀåºñÇÏ±â(Wield)"          :
           (i == 130) ? "x    : ÁÖÀ§ÀÇ °ÍÀ» Á¶»çÇÔ(eXamine)"     :
           (i == 135) ? "z    : ¸¶¹ýºÀÀ» »ç¿ë(Zap)"                :
           (i == 140) ? "A    : Æ¯¼ö ´É·Â/º¯ÀÌ ¸ñ·Ï(Abilities)"  :
           (i == 141) ? "C    : °æÇèÄ¡ µîÀ» º¸±â(Check)"         :
           (i == 142) ? "D    : ½ÃÃ¼¸¦ ÇØÃ¼ÇÏ±â(Dissect)"        :
           (i == 145) ? "E    : Àåºñ ¾ÆÀÌÅÛ Æ¯¼ö´É·Â ¹ßµ¿(Evoke)":
           (i == 150) ? "M    : ÁÖ¹® ¸Þ¸ð¶óÀÌÁî(Memorise)"       :
           (i == 155) ? "O    : ´øÁ¯ ±¸¼º º¸±â(Overview)"        :
           (i == 160) ? "P/R  : Àå½Å±¸ÀÇ Âø¿ë/ÇØÁ¦(Put/Remove)"  :
           (i == 165) ? "Q    : °ÔÀÓ Æ÷±â(Quit)"                 :
           (i == 168) ? "S    : ÀúÀå ÈÄ Á¾·á(È®ÀÎ ÀÖÀ½)(Save)"   :
           (i == 179) ? "V    : ¹öÀü Á¤º¸ º¸±â(Version)"         :
           (i == 200) ? "W/T  : °©¿Ê ÀÔ±â/¹þ±â(Wear/Take off)"   :
           (i == 210) ? "X    : ÇöÀç Ãþ Áöµµ º¸±â(eXamine)"      :
           (i == 220) ? "Z    : ¸¶¹ý ÁÖ¹® ½ÃÀü"                  :
           (i == 240) ? ",/g  : ¾ÆÀÌÅÛ ÁÝ±â(Get)"                :
           (i == 242) ? "./del: 1ÅÏ ½¬±â"                        :
           (i == 250) ? "</>  : °è´Ü ¿Ã¶ó°¡±â/³»·Á°¡±â"          :
           (i == 270) ? ";    : ¹ß ¹ØÀ» Á¶»çÇÏ±â"                :
           (i == 280) ? "\\    : È®ÀÎµÈ ¾ÆÀÌÅÛ ¸ñ·Ï"             :
#ifdef WIZARD
           (i == 290) ? "&    : À§Àúµå ¸ðµå±â´É »ç¿ë"            :
#endif
           (i == 300) ? "+/-  : scroll up/down [level map only!]":
           (i == 310) ? "!    : ¿ÜÄ¡±â ¶Ç´Â ºÎÇÏ¿¡°Ô ¸í·É"       :
           (i == 325) ? "^    : ½Å¾Ó »óÅÂ º¸±â"                  :
           (i == 337) ? "@    : ÄÉ¸¯ÅÍ »óÅÂ º¸±â"                :
           (i == 340) ? "#    : ÄÉ¸¯ÅÍ ´ýÇÁ ÆÄÀÏ »ý¼º"           :
           (i == 350) ? "=    : ¾ÆÀÌÅÛ ¸ñ·Ï ¶Ç´Â ¸¶¹ý Àç¼³Á¤"    :
           (i == 360) ? "\'    : a¹«±â ÀåÂø ¶Ç´Â b¹«±â·Î º¯°æ"    :
#ifdef USE_MACROS
           (i == 380) ? "`    : ¸ÅÅ©·Î Ãß°¡"                     :
           (i == 390) ? "~    : ¸ÅÅ©·Î ÀúÀå"                     :
#endif
           (i == 400) ? "]    : Âø¿ë ¹æ¾î±¸ º¸±â"                :
           (i == 410) ? "\"    : Âø¿ë Àå½Å±¸ º¸±â"                :
           (i == 420) ? "Ctrl-P : Áö³ª°£ ¸Þ½ÃÁö º¸±â(Previous)"  :
#ifdef PLAIN_TERM
           (i == 430) ? "Ctrl-R : È­¸é °»½Å(Redraw)"             :
#endif
           (i == 440) ? "Ctrl-A : ÀÚµ¿ÁÝ±â ON/OFF(Autopick)"     :
           (i == 450) ? "Ctrl-X : È®ÀÎ ¾øÀÌ ÀúÀå ÈÄ Á¾·á(eXit)"  :

#ifdef ALLOW_DESTROY_ITEM_COMMAND
           (i == 455) ? "Ctrl-D : ÀÎº¥Åä¸®³» ¾ÆÀÌÅÛ ºÎ¼ö±â"      :
#endif

#ifdef WINDOWS
           (i == 456) ? "Ctrl-T : ÆùÆ® º¯°æ"                     :
#endif

           (i == 460) ? "Shift & DIR : ¿¬¼Ó °È±â"                :
           (i == 465) ? "/ DIR : ¿¬¼Ó °È±â"                      :
           (i == 470) ? "Ctrl  & DIR : ¹®¿­±â/ÇÔÁ¤ÇØÃ¼/°ø°Ý"     :
           (i == 475) ? "* DIR : ¹®¿­±â/ÇÔÁ¤ÇØÃ¼/°ø°Ý"           :
           (i == 478) ? "Shift & 5(Å°ÆÐµå ¼ýÀÚ) :100ÅÏ ½¬±â" :
           (i == 479) ? "Ctrl-O : ¾îµð·Ð°¡ ÀÌµ¿(Á¤È®È÷ ¸ð¸§)"   :
           (i == 480) ? "Ctrl-F : À§Ä¡ ±â¾ïÇÏ±â"   :
           (i == 481) ? "Ctrl-G : ±â¾ïÇÑ À§Ä¡·Î ÀÚµ¿ÀÌµ¿"           :
           (i == 482) ? "Ctrl-E : ÇöÀ§Ä¡ ¾ÆÀÌÅÛ ¸Þ½ÃÁö ¹«½Ã"      :
           (i == 483) ? "Ctrl-S : ¾ÆÀÌÅÛ À§Ä¡ ±â·Ï(*.lst¿¡¼­ È®ÀÎ)"
#else /* JP */
    return((i ==  10) ? "a    : use special ability"              :
           (i ==  20) ? "d(#) : drop (exact quantity of) items"   :
           (i ==  30) ? "e    : eat food"                         :
           (i ==  40) ? "f    : fire first available missile"     :
           (i ==  50) ? "i    : inventory listing"                :
           (i ==  55) ? "m    : check skills"                     :
           (i ==  60) ? "o/c  : open / close a door"              :
           (i ==  65) ? "p    : pray"                             :
           (i ==  70) ? "q    : quaff a potion"                   :
           (i ==  80) ? "r    : read a scroll or book"            :
           (i ==  90) ? "s    : search adjacent tiles"            :
           (i == 100) ? "t    : throw/shoot an item"              :
           (i == 110) ? "v    : view item description"            :
           (i == 120) ? "w    : wield an item"                    :
           (i == 130) ? "x    : examine visible surroundings"     :
           (i == 135) ? "z    : zap a wand"                       :
           (i == 140) ? "A    : list abilities/mutations"         :
           (i == 141) ? "C    : check experience"                 :
           (i == 142) ? "D    : dissect a corpse"                 :
           (i == 145) ? "E    : evoke power of wielded item"      :
           (i == 150) ? "M    : memorise a spell"                 :
           (i == 155) ? "O    : overview of the dungeon"          :
           (i == 160) ? "P/R  : put on / remove jewellery"        :
           (i == 165) ? "Q    : quit without saving"              :
           (i == 168) ? "S    : save game and exit"               :
           (i == 179) ? "V    : version information"              :
           (i == 200) ? "W/T  : wear / take off armour"           :
           (i == 210) ? "X    : examine level map"                :
           (i == 220) ? "Z    : cast a spell"                     :
           (i == 240) ? ",/g  : pick up items"                    :
           (i == 242) ? "./del: rest one turn"                    :
           (i == 250) ? "</>  : ascend / descend a staircase"     :
           (i == 270) ? ";    : examine occupied tile"            :
           (i == 280) ? "\\    : check item knowledge"            :
#ifdef WIZARD
           (i == 290) ? "&    : invoke your Wizardly powers"      :
#endif
           (i == 300) ? "+/-  : scroll up/down [level map only]"  :
           (i == 310) ? "!    : shout or command allies"          :
           (i == 325) ? "^    : describe religion"                :
           (i == 337) ? "@    : status"                           :
           (i == 340) ? "#    : dump character to file"           :
           (i == 350) ? "=    : reassign inventory/spell letters" :
           (i == 360) ? "\'    : wield item a, or switch to b"    :
#ifdef USE_MACROS
           (i == 380) ? "`    : add macro"                        :
           (i == 390) ? "~    : save macros"                      :
#endif
           (i == 400) ? "]    : display worn armour"              :
           (i == 410) ? "\"    : display worn jewellery"          :
           (i == 420) ? "Ctrl-P : see old messages"               :
#ifdef PLAIN_TERM
           (i == 430) ? "Ctrl-R : Redraw screen"                  :
#endif
           (i == 440) ? "Ctrl-A : toggle Autopickup"              :
           (i == 450) ? "Ctrl-X : Save game without query"        :

#ifdef ALLOW_DESTROY_ITEM_COMMAND
           (i == 455) ? "Ctrl-D : Destroy inventory item"         :
#endif

#ifdef WINDOWS
           (i == 456) ? "Ctrl-T : change the font"                :
#endif

           (i == 460) ? "Shift & DIR : long walk"                 :
           (i == 465) ? "/ DIR : long walk"                       :
           (i == 470) ? "Ctrl  & DIR : door; untrap; attack"      :
           (i == 475) ? "* DIR : door; untrap; attack"            :
           (i == 478) ? "Shift & 5 on keypad : rest 100 turns"
#endif /* JP */
                      : "");
}                               // end command_string()

// Interactive menu for item drop/use
void use_item(int idx)
{
    int type = you.inv[idx].base_type;
    const char *action_list[] = {
#ifdef JP
    /* weapons       missiles    armour       wands */
      "w)¹«ÀåÇÏ±â", "t)ÅõÃ´", "W)ÀÔ±â", "z)¸¶¹ýºÀ »ç¿ë",

    /* foods                     scrolls       jewellry */
      "e)¸Ô±â",   "",         "r)ÀÐ±â",     "P)Àå½Å±¸ Âø¿ë",

    /* potions                   books        staves */
      "q)¸¶½Ã±â",     "",         "r)ÀÐ±â",     "w)¹«ÀåÇÏ±â",

    /* orbs         miscellany */
      "",           "w)Áã±â", "",         "",
      "", "",

      // When Equipped
    /* weapons       missiles    armour       wands */
      "",            "",         "T)¹þ±â",    "",

    /* foods                     scrolls       jewellry */
      "",            "",         "",          "R)Á¦°Å",

    /* potions                   books  */
      "",            "",         "",           "",

    /* staves       orbs         miscellany */
      "",            "",         "E)¾ÆÀÌÅÛ ´É·Â »ç¿ë", "",
      "", ""
#else
      "w) Wield", "t) Throw", "W) Wear", "z) Zap",
      "e) Eat",   "",         "r) Read", "P) Put",
      "q) Quaff", "",         "r) Read", "w) Wield",
      "", "", "", "", "", "",
      // When Equipped
      "",         "",         "T) Take off", "",
      "",         "",         "",            "R) Remove",
      "", "", "", "", "", "", "", "", "", ""
#endif
    };

    if (idx >= ENDOFPACK) return;

    int count = -1;
    int maxcount = you.inv[idx].quantity;
    static char desc[200];
    int i;

    // Equipped?
    for (i=0; i< NUM_EQUIP;i++)
    {
        if (you.equip[i] == idx)
        {
            type += 18;
            break;
        }
    }

    char act = action_list[type][0];
    int key;
    in_name(idx, DESC_INVENTORY_EQUIP, desc);
    int namelen = strlen(desc);

    redraw_screen();
    mesclr();
    mpr(desc);
#ifdef JP
        snprintf(desc, 190, "¹«¾ó ÇÒ°ÍÀÎ°¡? %s  d)¹ö¸®±â    0-9/¸¶¿ì½º ÈÙ) °¹¼ö    ESC)Ãë¼ÒŽ~",
#else
        snprintf(desc, 190, "What to do?  %s  d) Drop   0-9/mouse wheel) Quantity  ESC) Abort",
#endif
                  action_list[type]);
    mpr(desc);

    while (1)
    {
        // avoid recurse
        set_keyin_mode(KEYIN_MODE_ITEMLIST_COMMAND);
        key = getch();
        set_keyin_mode(KEYIN_MODE_NONE);

#ifdef USE_TILE
        if (act != 0 && (key == act || key == CMD_MOUSE_RCLICK))
#else
        if (act != 0 && key == act)
#endif
        {
            // Use it
            switch (type)
            {
                case OBJ_WEAPONS:
                case OBJ_STAVES:
                case OBJ_MISCELLANY:
                    push_inven_idx(idx);
                    wield_weapon(false);
                    return;

                case OBJ_MISCELLANY + 18:
                    push_inven_idx(idx);
                    if (!evoke_wielded())
                        flush_input_buffer( FLUSH_ON_FAILURE );
                    return;

                case OBJ_MISSILES:
                    push_inven_idx(idx);
                    throw_anything();
                    return;

                case OBJ_ARMOUR:
                    push_inven_idx(idx);
                    wear_armour();
                    return;

                case OBJ_ARMOUR + 18:
                    takeoff_armour(idx);
                    return;

                case OBJ_WANDS:
                    push_inven_idx(idx);
                    zap_wand();
                    return;

                case OBJ_FOOD:
                    push_inven_idx(idx);
                    eat_food();
                    return;

                case OBJ_SCROLLS:
                case OBJ_BOOKS:
                    push_inven_idx(idx);
                    read_scroll();
                    return;

                case OBJ_JEWELLERY:
                    push_inven_idx(idx);
                    puton_ring();
                    return;

                case OBJ_JEWELLERY + 18:
                    push_inven_idx(idx);
                    remove_ring();
                    return;

                case OBJ_POTIONS:
                    push_inven_idx(idx);
                    drink();
                    return;

                default:
                    return;
            }
        }

        if (key >='0' && key <='9')
        {
            if (count < 0)
                count = (key - '0');
            else
                count = count * 10 + (key - '0');
            if (count > maxcount) count = maxcount;
        }
        else if (key == 8 )  // BackSpace
        {
            if (count <= 0)
                count = -1;
            else
                count /= 10;
        }

#ifdef USE_TILE
        if (key == CMD_MOUSE_WHEEL_UP)
            count ++;
        if (key == CMD_MOUSE_WHEEL_DOWN && count >= 0)
            count --;
#endif

        //if (count < 0) count = 0;
        if (count > maxcount) count = maxcount;
        if (maxcount == 1) count = 0;

#ifdef USE_TILE
        mpr_on(MODE_MPR);
#endif
        gotoxy(2 + namelen, 18);
        if (count >= 0)
            cprintf("%3d    ", count);
        else
            cprintf("       ");

        if (key == 'd')
        {
            push_inven_idx(idx);
            if (count < 0)
                push_inven_count(maxcount);
            else if (count != 0)
                push_inven_count(count);
            drop();
#ifdef STASH_TRACKING
        if (Options.stash_tracking >= STM_DROPPED)
            stashes.add_stash();
#endif
            return;
        }

        if (key == ESCAPE || key == 'q')
        {
#ifdef JP
                mpr("Ãë¼ÒÇß´Ù");
#else
                mpr("Aborted.");
#endif
                return;
        }
    }// While
}
