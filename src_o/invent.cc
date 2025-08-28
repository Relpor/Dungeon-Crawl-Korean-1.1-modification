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
        cprintf("아무것도 가진게 없다.");
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
        cprintf( "  소지품: %d.%d aum (한계 %d.%d aum 의 %d%% )",
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
                    cprintf("-다음장-");
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
#ifdef JP   // 컴파일시 확인바람 (감자)
                case OBJ_WEAPONS:    cprintf("무기");            break;
                case OBJ_MISSILES:   cprintf("쏘는 무기");        break;
                case OBJ_ARMOUR:     cprintf("갑옷");            break;
                case OBJ_WANDS:      cprintf("마법봉");        break;
                case OBJ_FOOD:       cprintf("음식");            break;
                case OBJ_UNKNOWN_I:  cprintf("미확인1");            break;
                case OBJ_SCROLLS:    cprintf("마법 두루마리");            break;
                case OBJ_JEWELLERY:  cprintf("장신구");          break;
                case OBJ_POTIONS:    cprintf("물약");              break;
                case OBJ_UNKNOWN_II: cprintf("미확인2");        break;
                case OBJ_BOOKS:      cprintf("서적");            break;
                case OBJ_STAVES:     cprintf("지팡이"); break;
                case OBJ_ORBS:       cprintf("오브");        break;
                case OBJ_MISCELLANY: cprintf("기타");          break;
                case OBJ_CORPSES:    cprintf("시체");            break;
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
                        cprintf("-다음장-");
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

                        //!!!!롹궋귩듓봨띙귒궻롹귦귢궫뷼귩먗륡궳?렑궥귡긆긵긘깈깛
                        //!!!!몧뷈뭷궻귺귽긡?궼쀎궳?렑궥귡
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
                            cprintf( " 골드)" );
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
            cprintf("아무 것도 가지고 있지 않다.");
#else
            cprintf("You aren't carrying anything.");
#endif
        else
        {
            if (item_class_inv == OBJ_WEAPONS)
#ifdef JP
                cprintf("무기가 없다.");
#else
                cprintf("You aren't carrying any weapons.");
#endif
            else if (item_class_inv == OBJ_MISSILES)
#ifdef JP
                cprintf("탄약이 없다.");
#else
                cprintf("You aren't carrying any ammunition.");
#endif
            else
#ifdef JP
                cprintf("해당 아이템이 없다.");
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
                mpr( "그런 아이템을 가지고 있지 않다." );
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
                cprintf("다음장...");
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
#ifdef JP  // 컴파일시 확인바람 (감자)
    return((i ==  10) ? "a    : 획득"                         :
           (i ==  13) ? "A    : 모든 기술을 임의의 값으로 한다"     :
           (i ==  15) ? "b    : 제어 순간이동"           :
           (i ==  20) ? "B    : 어비스에 저속해진다"               :
           (i ==  20) ? "C    : ????"         :
           (i ==  30) ? "g    : 임의의 기술을 올린다"   :
           (i ==  35) ? "G    : 모든 몬스터를 없엔다"       :
           (i ==  40) ? "h/H  :치유(회복)"                 :
           (i ==  50) ? "i/I  : 소지품 이템 감정"  :
           (i ==  70) ? "l    : 미궁 입구 생성"     :
           (i ==  80) ? "m/M  : 번호/이름으로 몬스터 생성"    :
           (i ==  90) ? "o/%%  : 아이템 생성"                :
           (i == 100) ? "p    : 판데모니움 입구 생성"   :
           (i == 110) ? "x    : 레벨업"                 :
           (i == 115) ? "r    : 종족 변경"                     :
           (i == 120) ? "s    : 20000 기술 점수 획득"      :
           (i == 130) ? "S    :임의의 기술값 변경"       :
           (i == 140) ? "t    : 아이템 성능 변경"       :
           (i == 150) ? "X    : 좀의 선물 받기"           :
           (i == 160) ? "z/Z  : 번호/이름의 주문 시전"        :
           (i == 200) ? "$    : 1000 골드 획득"             :
           (i == 210) ? "</>  : 오르는/내려가는 계단 생성"          :
           (i == 220) ? "u/d  : 위/아래 층으로 이동"                :
           (i == 230) ? "~/\"  : 지정층으로 공간이동"                :
           (i == 240) ? "(    : 지형 작성"                   :
           (i == 250) ? "]    : 돌연변이 획득"               :
           (i == 260) ? "[    : 데몬스폰의 변이 획득"   :
           (i == 270) ? ":    : 던젼의 분기층 표시"     :
           (i == 280) ? "{    : 마법 지도"                   :
           (i == 290) ? "^    : 신앙치 올리기"               :
           (i == 300) ? "_    : 지정한 신을 모신다"         :
           (i == 310) ? "\'    : 아이템의 좌표 리스트"        :
           (i == 320) ? "?    : 위저드 커맨드 표시"       :
           (i == 330) ? "|    : 전 아티팩트 생성"       :
           (i == 340) ? "+    : 랜덤 아티팩트 생성" :
           (i == 350) ? "=    : 기술 점수의 합계 표시"
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
    return((i ==  10) ? "a    : 특수 능력 사용(use Ability)"     :
           (i ==  20) ? "d(#) : 아이템(#갯수) 버림(Drop)"        :
           (i ==  30) ? "e    : 음식 먹기(Eat)"                  :
           (i ==  40) ? "f    : 쏘는 무기 자동 선택 발사(Fire)"  :
           (i ==  50) ? "i    : 소지품 목록(Inventory)"          :
           (i ==  55) ? "m    : 기술 화면"                       :
           (i ==  60) ? "o/c  : 문 열기/닫기(Open/Close)"        :
           (i ==  65) ? "p    : 신에게 기도하기(Pray)"           :
           (i ==  70) ? "q    : 물약 마시기(Quaff)"              :
           (i ==  80) ? "r    : 책이나 마법 두루마리 읽기(Read)"        :
           (i ==  90) ? "s    : 주변 탐색(Search)"               :
           (i == 100) ? "t    : 아이템 투척(Throw)"              :
           (i == 110) ? "v    : 아이템의 상세 설명 보기(View)"   :
           (i == 120) ? "w    : 아이템 장비하기(Wield)"          :
           (i == 130) ? "x    : 주위의 것을 조사함(eXamine)"     :
           (i == 135) ? "z    : 마법 주문 시전"                :
           (i == 140) ? "A    : 특수 능력/변이 목록(Abilities)"  :
           (i == 141) ? "C    : 경험치 등을 보기(Check)"         :
           (i == 142) ? "D    : 시체를 해체하기(Dissect)"        :
           (i == 145) ? "E    : 장비 아이템 특수능력 발동(Evoke)":
           (i == 150) ? "M    : 주문 메모라이즈(Memorise)"       :
           (i == 155) ? "O    : 던젼 구성 보기(Overview)"        :
           (i == 160) ? "P/R  : 장신구의 착용/해제(Put/Remove)"  :
           (i == 165) ? "Q    : 게임 포기(Quit)"                 :
           (i == 168) ? "S    : 저장 후 종료(확인 있음)(Save)"   :
           (i == 179) ? "V    : 버전 정보 보기(Version)"         :
           (i == 200) ? "W/T  : 갑옷 입기/벗기(Wear/Take off)"   :
           (i == 210) ? "X    : 현재 층 지도 보기(eXamine)"      :
           (i == 220) ? "Z    : 마법봉을 사용(Zap)"                  :
           (i == 240) ? ",/g  : 아이템 줍기(Get)"                :
           (i == 242) ? "./del: 1턴 쉬기"                        :
           (i == 250) ? "</>  : 계단 올라가기/내려가기"          :
           (i == 270) ? ";    : 발 밑을 조사하기"                :
           (i == 280) ? "\\    : 확인된 아이템 목록"             :
#ifdef WIZARD
           (i == 290) ? "&    : 위저드 모드기능 사용"            :
#endif
           (i == 300) ? "+/-  : scroll up/down [level map only!]":
           (i == 310) ? "!    : 외치기 또는 부하에게 명령"       :
           (i == 325) ? "^    : 신앙 상태 보기"                  :
           (i == 337) ? "@    : 케릭터 상태 보기"                :
           (i == 340) ? "#    : 케릭터 덤프 파일 생성"           :
           (i == 350) ? "=    : 아이템 목록 또는 마법 재설정"    :
           (i == 360) ? "\'    : a무기 장착 또는 b무기로 변경"    :
#ifdef USE_MACROS
           (i == 380) ? "`    : 매크로 추가"                     :
           (i == 390) ? "~    : 매크로 저장"                     :
#endif
           (i == 400) ? "]    : 착용 방어구 보기"                :
           (i == 410) ? "\"    : 착용 장신구 보기"                :
           (i == 420) ? "Ctrl-P : 지나간 메시지 보기(Previous)"  :
#ifdef PLAIN_TERM
           (i == 430) ? "Ctrl-R : 화면 갱신(Redraw)"             :
#endif
           (i == 440) ? "Ctrl-A : 자동줍기 ON/OFF(Autopick)"     :
           (i == 450) ? "Ctrl-X : 확인 없이 저장 후 종료(eXit)"  :

#ifdef ALLOW_DESTROY_ITEM_COMMAND
           (i == 455) ? "Ctrl-D : 인벤토리내 아이템 부수기"      :
#endif

#ifdef WINDOWS
           (i == 456) ? "Ctrl-T : 폰트 변경"                     :
#endif

           (i == 460) ? "Shift & DIR : 연속 걷기"                :
           (i == 465) ? "/ DIR : 연속 걷기"                      :
           (i == 470) ? "Ctrl  & DIR : 문열기/함정해체/공격"     :
           (i == 475) ? "* DIR : 문열기/함정해체/공격"           :
           (i == 478) ? "Shift & 5(키패드 숫자) :100턴 쉬기" :
           (i == 479) ? "Ctrl-O : 어디론가 이동(정확히 모름)"   :
           (i == 480) ? "Ctrl-F : 위치 기억하기"   :
           (i == 481) ? "Ctrl-G : 기억한 위치로 자동이동"           :
           (i == 482) ? "Ctrl-E : 현위치 아이템 메시지 무시"      :
           (i == 483) ? "Ctrl-S : 아이템 위치 기록(*.lst에서 확인)"
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
      "w)무장하기", "t)투척", "W)입기", "z)마법봉 사용",

    /* foods                     scrolls       jewellry */
      "e)먹기",   "",         "r)읽기",     "P)장신구 착용",

    /* potions                   books        staves */
      "q)마시기",     "",         "r)읽기",     "w)무장하기",

    /* orbs         miscellany */
      "",           "w)쥐기", "",         "",
      "", "",

      // When Equipped
    /* weapons       missiles    armour       wands */
      "",            "",         "T)벗기",    "",

    /* foods                     scrolls       jewellry */
      "",            "",         "",          "R)제거",

    /* potions                   books  */
      "",            "",         "",           "",

    /* staves       orbs         miscellany */
      "",            "",         "E)아이템 능력 사용", "",
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
        snprintf(desc, 190, "무얼 할것인가? %s  d)버리기    0-9/마우스 휠) 갯수    ESC)취소?",
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
                mpr("취소했다");
#else
                mpr("Aborted.");
#endif
                return;
        }
    }// While
}
