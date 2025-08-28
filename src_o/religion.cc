/*
 *  File:       religion.cc
 *  Summary:    Misc religion related functions.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *
 *   <7>   11jan2001     gdl    added M. Valvoda's changes to
 *                              god_colour() and god_name()
 *   <6>   06-Mar-2000   bwr    added penance, gift_timeout,
 *                              divine_retribution(), god_speaks()
 *   <5>   11/15/99      cdl    Fixed Daniel's yellow Xom patch  :)
 *                              Xom will sometimes answer prayers
 *   <4>   10/11/99      BCR    Added Daniel's yellow Xom patch
 *   <3>    6/13/99      BWR    Vehumet book giving code.
 *   <2>    5/20/99      BWR    Added screen redraws
 *   <1>    -/--/--      LRH    Created
 */

#include "AppHdr.h"
#include "religion.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "externs.h"

#include "abl-show.h"
#include "beam.h"
#include "debug.h"
#include "decks.h"
#include "describe.h"
#include "dungeon.h"
#include "effects.h"
#include "food.h"
#include "it_use2.h"
#include "itemname.h"
#include "items.h"
#include "misc.h"
#include "monplace.h"
#include "mutation.h"
#include "newgame.h"
#include "ouch.h"
#include "player.h"
#include "shopping.h"
#include "skills2.h"
#include "spells1.h"
#include "spells2.h"
#include "spells3.h"
#include "spl-cast.h"
#include "stuff.h"

const char *sacrifice[] = {
#ifdef JP
    "은(는) 은색으로 반짝이고는 사라졌다.",
    "은(는) 빛나는 금색으로 반짝이고는 사라졌다.",
    "은(는) 눈 깜짝할 사이에 완전히 썩어버렸다.",
    "은(는) 티끌처럼 분해되었다.",
    "은(는) 벌레에게 먹혀버렸다.",    /* Xom - no sacrifices */
    "은(는) 폭발하고 사라졌다.",
    "은(는) 불꽃의 폭발으로 구워져 버렸다.",
    "은(는) 굉음을 내며 나타난 불기둥에 휩싸여 사라져 버렸다.",
    "은(는) 잠깐 동안 희미하게 반짝이고는 사라졌다.",
    "은(는) 포효하는 불꽃의 기둥에 구워져 버렸다.",
    "은(는) 기분 나쁜 무지개색으로 반짝이다가 사라졌다.",
    "은(는) 증발했다."
#else
    " glows silver and disappears.",
    " glows a brilliant golden colour and disappears.",
    " rots away in an instant.",
    " crumbles to dust.",
    " is eaten by a bug.",    /* Xom - no sacrifices */
    " explodes into nothingness.",
    " is consumed in a burst of flame.",
    " is consumed in a roaring column of flame.",
    " glows faintly for a moment, then is gone.",
    " is consumed in a roaring column of flame.",
    " glows with a rainbow of weird colours and disappears.",
    " evaporates."
#endif
};

void altar_prayer(void);
void dec_penance(int god, int val);
void divine_retribution(int god);
void inc_penance(int god, int val);
void inc_penance(int val);

void dec_penance(int god, int val)
{
    if (you.penance[god] > 0)
    {
        if (you.penance[god] <= val)
        {
#ifdef JP
            simple_god_message("은(는) 분노를 누그러뜨린 것 같다.", god);
#else
            simple_god_message(" seems mollified.", god);
#endif
            you.penance[god] = 0;
        }
        else
            you.penance[god] -= val;
    }
}                               // end dec_penance()

void dec_penance(int val)
{
    dec_penance(you.religion, val);
}                               // end dec_penance()

void inc_penance(int god, int val)
{
    if ((int) you.penance[god] + val > 200)
        you.penance[god] = 200;
    else
        you.penance[god] += val;
}                               // end inc_penance()

void inc_penance(int val)
{
    inc_penance(you.religion, val);
}                               // end inc_penance()

static void inc_gift_timeout(int val)
{
    if ((int) you.gift_timeout + val > 200)
        you.gift_timeout = 200;
    else
        you.gift_timeout += val;
}                               // end inc_gift_timeout()

void pray(void)
{
    int            temp_rand = 0;
    unsigned char  was_praying = you.duration[DUR_PRAYER];
    bool           success = false;

    if (silenced(you.x_pos, you.y_pos))
    {
#ifdef JP
        mpr("당신은 소리를 낼 수 없다!");
#else
        mpr("You are unable to make a sound!");
#endif
        return;
    }

    // all prayers take time
    you.turn_is_over = 1;

    if (you.religion != GOD_NO_GOD
            && grd[you.x_pos][you.y_pos] == 179 + you.religion)
    {
        altar_prayer();
    }
    else if (grd[you.x_pos][you.y_pos] >= 180
                && grd[you.x_pos][you.y_pos] <= 199)
    {
        if (you.species == SP_DEMIGOD)
        {
#ifdef JP
            mpr("유감이지만, 당신 같은 지위에 있는 존재는 여기에서 기도할 수 없다.");
#else
            mpr("Sorry, a being of your status cannot worship here.");
#endif
            return;
        }
        god_pitch(grd[you.x_pos][you.y_pos] - 179);
        return;
    }

    if (you.religion == GOD_NO_GOD)
    {
#ifdef JP
        strcpy(info, "당신은 잠깐, ");
#else
        strcpy(info, "You spend a moment contemplating the meaning of ");
#endif

        if (you.is_undead)
#ifdef JP
            strcat(info, "죽음의 의미에 대해 생각했다.");
#else
            strcat(info, "un");
#endif

#ifdef JP
        else
            strcat(info, "삶의 의미에 대해 생각했다.");
#else
        strcat(info, "life.");
#endif
        mpr(info);
        return;
    }
    else if (you.religion == GOD_XOM)
    {
        if (one_chance_in(100))
        {
            // Every now and then, Xom listens
            // This is for flavour, not effect, so praying should not be
            // encouraged.

            // Xom is nicer to experienced players
            bool nice = (27 <= random2( 27 + you.experience_level ));

            // and he's not very nice even then
            int sever = (nice) ? random2( random2( you.experience_level ) )
                               : you.experience_level;

            // bad results are enforced, good are not
            bool force = !nice;

            Xom_acts( nice, 1 + sever, force );
        }
        else
#ifdef JP
            mpr("좀은 당신을 안중에도 두지 않는다.");
#else
            mpr("Xom ignores you.");
#endif

        return;
    }

#ifdef JP
    strcpy( info, "당신은 " );
#else
    strcpy( info, "You offer a prayer to " );
#endif
    strcat( info, god_name( you.religion ) );
#ifdef JP
    strcat( info, "에게 기도를 드렸다." );
#else
    strcat( info, "." );
#endif
    mpr(info);

    you.duration[DUR_PRAYER] = 9 + (random2(you.piety) / 20)
                                            + (random2(you.piety) / 20);

    if (player_under_penance())
#ifdef JP
        simple_god_message("은(는) 속죄를 요구하고 있다!");
#else
        simple_god_message(" demands penance!");
#endif
    else
    {
        strcpy(info, god_name(you.religion));
#ifdef JP
        strcat(info, "은(는) ");
#else
        strcat(info, " is ");
#endif

#ifdef JP
        strcat(info, (you.piety > 130) ? "당신의 숭배를 자랑스럽게 여기고 있다" :
                     (you.piety > 100) ? "당신에게 지극히 만족하고 있다" :
                     (you.piety >  70) ? "당신에게 크게 만족하고 있다" :
                     (you.piety >  40) ? "당신에게 꽤나 만족하고 있다" :
                     (you.piety >  20) ? "당신에게 만족하고 있다" :
                     (you.piety >   5) ? "당신에게 특별한 관심을 가지고 있지 않다"
                                       : "당신에게 불만이 있다");
#else
        strcat(info, (you.piety > 130) ? "exalted by your worship" :
                     (you.piety > 100) ? "extremely pleased with you" :
                     (you.piety >  70) ? "greatly pleased with you" :
                     (you.piety >  40) ? "most pleased with you" :
                     (you.piety >  20) ? "pleased with you" :
                     (you.piety >   5) ? "noncommittal"
                                       : "displeased");
#endif

#ifdef JP
        strcat(info, ".");
#else
        strcat(info, ".");
#endif
        god_speaks(you.religion, info);

        if (you.piety > 130)
            you.duration[DUR_PRAYER] *= 3;
        else if (you.piety > 70)
            you.duration[DUR_PRAYER] *= 2;
    }

#if DEBUG_DIAGNOSTICS
#ifdef JP
    snprintf( info, INFO_SIZE, "piety: %d", you.piety );
#else
    snprintf( info, INFO_SIZE, "piety: %d", you.piety );
#endif
    mpr( info, MSGCH_DIAGNOSTICS );
#endif

    // Consider a gift if we don't have a timeout and weren't
    // already praying when we prayed.
    if (!you.penance[you.religion] && !you.gift_timeout && !was_praying)
    {
        //   Remember to check for water/lava
        //jmf: "good" god will sometimes feed you (a la Nethack)
        if (you.religion == GOD_ZIN
            && you.hunger_state == HS_STARVING
            && random2(250) <= you.piety)
        {
#ifdef JP
            god_speaks(you.religion, "당신은 포만감을 느꼈다.");
#else
            god_speaks(you.religion, "Your stomach feels content.");
#endif
            set_hunger(6000, true);
            lose_piety(5 + random2avg(10, 2));
            inc_gift_timeout(30 + random2avg(10, 2));
            return;
        }

        if (you.religion == GOD_NEMELEX_XOBEH
            && random2(200) <= you.piety
            && (!you.attribute[ATTR_CARD_TABLE] || one_chance_in(3))
            && !you.attribute[ATTR_CARD_COUNTDOWN]
            && grd[you.x_pos][you.y_pos] != DNGN_LAVA
            && grd[you.x_pos][you.y_pos] != DNGN_DEEP_WATER)
        {
            int thing_created = NON_ITEM;
            unsigned char gift_type = MISC_DECK_OF_TRICKS;

            if (!you.attribute[ATTR_CARD_TABLE])
            {
                thing_created = items( 1, OBJ_MISCELLANY,
                                       MISC_PORTABLE_ALTAR_OF_NEMELEX,
                                       true, 1, 250 );

                if (thing_created != NON_ITEM)
                    you.attribute[ATTR_CARD_TABLE] = 1;
            }
            else
            {
                if (random2(200) <= you.piety && one_chance_in(4))
                    gift_type = MISC_DECK_OF_SUMMONINGS;
                if (random2(200) <= you.piety && coinflip())
                    gift_type = MISC_DECK_OF_WONDERS;
                if (random2(200) <= you.piety && one_chance_in(4))
                    gift_type = MISC_DECK_OF_POWER;

                thing_created = items( 1, OBJ_MISCELLANY, gift_type,
                                       true, 1, 250 );
            }

            if (thing_created != NON_ITEM)
            {
                move_item_to_grid( &thing_created, you.x_pos, you.y_pos );

#ifdef JP
                simple_god_message("은(는) 당신에게 선물을 하사했다!");
#else
                simple_god_message(" grants you a gift!");
#endif
                more();
                canned_msg(MSG_SOMETHING_APPEARS);

                you.attribute[ATTR_CARD_COUNTDOWN] = 10;
                inc_gift_timeout(5 + random2avg(9, 2));
            }
        }

        if ((you.religion == GOD_OKAWARU || you.religion == GOD_TROG)
            && you.piety > 130
            && random2(you.piety) > 120
            && grd[you.x_pos][you.y_pos] != DNGN_LAVA
            && grd[you.x_pos][you.y_pos] != DNGN_DEEP_WATER
            && one_chance_in(4))
        {
            if (you.religion == GOD_TROG
                || (you.religion == GOD_OKAWARU && coinflip()))
            {
                success = acquirement(OBJ_WEAPONS);
            }
            else
            {
                success = acquirement(OBJ_ARMOUR);
            }

            if (success)
            {
#ifdef JP
                simple_god_message("은(는) 당신에게 선물을 하사했다!");
#else
                simple_god_message(" has granted you a gift!");
#endif
                more();

                inc_gift_timeout(30 + random2avg(19, 2));
            }
        }

        if (you.religion == GOD_YREDELEMNUL
            && random2(you.piety) > 80 && one_chance_in(5))
        {
            int thing_called = MONS_PROGRAM_BUG;  // error trapping {dlb}

            temp_rand = random2(100);
            thing_called = ((temp_rand > 66) ? MONS_WRAITH :            // 33%
                            (temp_rand > 52) ? MONS_WIGHT :             // 12%
                            (temp_rand > 40) ? MONS_SPECTRAL_WARRIOR :  // 16%
                            (temp_rand > 31) ? MONS_ROTTING_HULK :      //  9%
                            (temp_rand > 23) ? MONS_SKELETAL_WARRIOR :  //  8%
                            (temp_rand > 16) ? MONS_VAMPIRE :           //  7%
                            (temp_rand > 10) ? MONS_GHOUL :             //  6%
                            (temp_rand >  4) ? MONS_MUMMY               //  6%
                                             : MONS_FLAYED_GHOST);      //  5%

            if (create_monster( thing_called, 0, BEH_FRIENDLY,
                                you.x_pos, you.y_pos,
                                you.pet_target, 250 ) != -1)
            {
#ifdef JP
                simple_god_message("은(는) 당신에게 언데드 노예를 하사했다!");
#else
                simple_god_message(" grants you an undead servant!");
#endif
                more();
                inc_gift_timeout(4 + random2avg(7, 2));
            }
        }

        if ((you.religion == GOD_KIKUBAAQUDGHA
                || you.religion == GOD_SIF_MUNA
                || you.religion == GOD_VEHUMET)
            && you.piety > 160 && random2(you.piety) > 100)
        {
            unsigned int gift = NUM_BOOKS;

            switch (you.religion)
            {
            case GOD_KIKUBAAQUDGHA:     // gives death books
                if (!you.had_book[BOOK_NECROMANCY])
                    gift = BOOK_NECROMANCY;
                else if (!you.had_book[BOOK_DEATH])
                    gift = BOOK_DEATH;
                else if (!you.had_book[BOOK_UNLIFE])
                    gift = BOOK_UNLIFE;
                else if (!you.had_book[BOOK_NECRONOMICON])
                    gift = BOOK_NECRONOMICON;
                break;

            case GOD_SIF_MUNA:
                gift = OBJ_RANDOM;      // Sif Muna - gives any
                break;

            // Vehumet - gives conj/summ. books (higher skill first)
            case GOD_VEHUMET:
                if (!you.had_book[BOOK_CONJURATIONS_I])
                    gift = give_first_conjuration_book();
                else if (!you.had_book[BOOK_POWER])
                    gift = BOOK_POWER;
                else if (!you.had_book[BOOK_ANNIHILATIONS])
                    gift = BOOK_ANNIHILATIONS;  // conj books

                if (you.skills[SK_CONJURATIONS] < you.skills[SK_SUMMONINGS]
                    || gift == NUM_BOOKS)
                {
                    if (!you.had_book[BOOK_CALLINGS])
                        gift = BOOK_CALLINGS;
                    else if (!you.had_book[BOOK_SUMMONINGS])
                        gift = BOOK_SUMMONINGS;
                    else if (!you.had_book[BOOK_DEMONOLOGY])
                        gift = BOOK_DEMONOLOGY; // summoning bks
                }
                break;
            }

            if (gift != NUM_BOOKS
                && (grd[you.x_pos][you.y_pos] != DNGN_LAVA
                    && grd[you.x_pos][you.y_pos] != DNGN_DEEP_WATER))
            {
                if (gift == OBJ_RANDOM)
                    success = acquirement(OBJ_BOOKS);
                else
                {
                    int thing_created = items(1, OBJ_BOOKS, gift, true, 1, 250);
                    if (thing_created == NON_ITEM)
                        return;

                    move_item_to_grid( &thing_created, you.x_pos, you.y_pos );

                    if (thing_created != NON_ITEM)
                        success = true;
                }

                if (success)
                {
#ifdef JP
                    simple_god_message("은(는) 당신에게 선물을 하사했다!");
#else
                    simple_god_message(" has granted you a gift!");
#endif
                    more();

                    inc_gift_timeout(40 + random2avg(19, 2));
                }


                // Vehumet gives books less readily
                if (you.religion == GOD_VEHUMET && success)
                    inc_gift_timeout(10 + random2(10));
            }                   // end of giving book
        }                       // end of book gods
    }                           // end of gift giving
}                               // end pray()

char *god_name( int which_god, bool long_name ) // mv - rewritten
{
    static char godname_buff[80];

    switch (which_god)
    {
    case GOD_NO_GOD:
#ifdef JP
        sprintf(godname_buff, "신앙없음");
#else
        sprintf(godname_buff, "No God");
#endif
        break;
    case GOD_ZIN:
#ifdef JP
        sprintf(godname_buff, "%s진", long_name ? "질서를 가져오는 자 " : "");
#else
        sprintf(godname_buff, "Zin%s", long_name ? " the Law-Giver" : "");
#endif
        break;
    case GOD_SHINING_ONE:
#ifdef JP
        sprintf(godname_buff, "샤이닝 원");
#else
        sprintf(godname_buff, "The Shining One");
#endif
        break;
    case GOD_KIKUBAAQUDGHA:
#ifdef JP
        strcpy(godname_buff, "키쿠바쿠드하");
#else
        strcpy(godname_buff, "Kikubaaqudgha");
#endif
        break;
    case GOD_YREDELEMNUL:
#ifdef JP
        sprintf(godname_buff, "%s이레델렘눌", long_name ? "암흑의 " : "");
#else
        sprintf(godname_buff, "Yredelemnul%s", long_name ? " the Dark" : "");
#endif
        break;
    case GOD_XOM:
#ifdef JP
        strcpy(godname_buff, "좀");
#else
        strcpy(godname_buff, "Xom");
#endif
        if (long_name)
        {
#ifdef JP
            strcat(godname_buff, "");
#else
            strcat(godname_buff, " ");
#endif
            switch(random2(1000))
            {
            default:
#ifdef JP
                strcpy(godname_buff, "혼돈의 좀");
#else
                strcat(godname_buff, "of Chaos");
#endif
                break;
            case 1:
#ifdef JP
                strcpy(godname_buff, "무질서의 좀");
#else
                strcat(godname_buff, "the Random");
#endif
                if (coinflip())
#ifdef JP
                    strcpy(godname_buff, coinflip()?"무질서의 주인 좀":"난수의 신 좀");
#else
                    strcat(godname_buff, coinflip()?"master":" Number God");
#endif
                break;
            case 2:
#ifdef JP
                strcpy(godname_buff, "트릭의 좀");
#else
                strcat(godname_buff, "the Tricky");
#endif
                break;
            case 3:
#ifdef JP
                sprintf( godname_buff, "%s의 좀", coinflip() ? "예측불가능"
                                                                    : "예측불능" );
#else
                sprintf( godname_buff, "Xom the %sredictible", coinflip() ? "Less-P"
                                                                    : "Unp" );
#endif
                break;
            case 4:
#ifdef JP
                strcpy(godname_buff, "수많은 문의 좀");
#else
                strcat(godname_buff, "of Many Doors");
#endif
                break;
            case 5:
#ifdef JP
                strcpy(godname_buff, "변덕의 좀");
#else
                strcat(godname_buff, "the Capricious");
#endif
                break;
            case 6:
#ifdef JP
                strcpy(godname_buff, coinflip() ? "피물든 아이디어의 좀" : "아이디어의 집행자 좀");
#else
                strcat(godname_buff, "of ");
                strcat(godname_buff, coinflip() ? "Bloodstained" : "Enforced");
                strcat(godname_buff, " Whimsey");
#endif
                break;
            case 7:
#ifdef JP
                strcpy(godname_buff, "좀『너의 사용자명이 뭐였더라?』*딸깍딸깍딸깍*");
#else
                strcat(godname_buff, "\"What was your username?\" *clickity-click*");
#endif
                break;
            case 8:
#ifdef JP
                strcpy(godname_buff, "잔인한 재치의 좀");
#else
                strcat(godname_buff, "of Bone-Dry Humour");
#endif
                break;
            case 9:
#ifdef JP
                strcpy(godname_buff, coinflip() ? "사악한 장난의 좀" : "악의 있는 농담의 좀");
#else
                strcat(godname_buff, "of ");
                strcat(godname_buff, coinflip() ? "Malevolent" : "Malicious");
                strcat(godname_buff, " Giggling");
#endif
                break;
            case 10:
#ifdef JP
                strcpy(godname_buff, coinflip() ? "광기의 좀" : "정신병질자의 좀");
#else
                strcat(godname_buff, "the Psycho");
                strcat(godname_buff, coinflip() ? "tic" : "path");
#endif
                break;
            case 11:
#ifdef JP
#else
                strcat(godname_buff, "of ");
#endif
                switch(random2(5))
                {
#ifdef JP
                case 0: strcpy(godname_buff, "장난의 좀"); break;
                case 1: strcpy(godname_buff, "무서운 좀"); break;
                case 2: strcpy(godname_buff, "변덕심의 좀"); break;
                case 3: strcpy(godname_buff, "전언 철회의 좀"); break;
                case 4: strcpy(godname_buff, "알려지지 않은 의도의 좀"); break;
#else
                case 0: strcat(godname_buff, "Gnomic"); break;
                case 1: strcat(godname_buff, "Ineffable"); break;
                case 2: strcat(godname_buff, "Fickle"); break;
                case 3: strcat(godname_buff, "Swiftly Tilting"); break;
                case 4: strcat(godname_buff, "Unknown"); break;
#endif
                }
#ifdef JP
                strcat(godname_buff, "");
#else
                strcat(godname_buff, " Intent");
#endif
                if (coinflip())
#ifdef JP
                    strcat(godname_buff, "");
#else
                    strcat(godname_buff, "ion");
#endif
                break;
            case 12:
#ifdef JP
                sprintf(godname_buff, "좀·메인스터");
#else
                sprintf(godname_buff, "The Xom-Meister");
#endif
                if (coinflip())
#ifdef JP
                    strcat(godname_buff, "좀-어-롬-어-딩-동");
#else
                    strcat(godname_buff, ", Xom-a-lom-a-ding-dong");
#endif
                else if (coinflip())
#ifdef JP
                    strcat(godname_buff, "좀-오-라마");
#else
                    strcat(godname_buff, ", Xom-o-Rama");
#endif
                else if (coinflip())
#ifdef JP
                    strcat(godname_buff, "좀-좀-보-봄, 바나나-파나-포-폼");
#else
                    strcat(godname_buff, ", Xom-Xom-bo-Bom, Banana-Fana-fo-Fom");
#endif
                break;
            case 13:
#ifdef JP
                strcpy(godname_buff, coinflip() ? "소란을 가져오는 자 좀" :
                 "무질서를 가져오는 자 좀");
#else
                strcat(godname_buff, "the Begetter of ");
                strcat(godname_buff, coinflip() ? "Turbulence" : "Discontinuities");
#endif
                break;
            }
        }
        break;
    case GOD_VEHUMET:
#ifdef JP
        strcpy(godname_buff, "베후멧");
#else
        strcpy(godname_buff, "Vehumet");
#endif
        break;
    case GOD_OKAWARU:
#ifdef JP
        sprintf(godname_buff, "%s오카와루", long_name ? "전쟁의 신 " : "");
#else
        sprintf(godname_buff, "%sOkawaru", long_name ? "Warmaster " : "");
#endif
        break;
    case GOD_MAKHLEB:
#ifdef JP
        sprintf(godname_buff, "%s마크레브", long_name ? "파괴자 " : "");
#else
        sprintf(godname_buff, "Makhleb%s", long_name ? " the Destroyer" : "");
#endif
        break;
    case GOD_SIF_MUNA:
#ifdef JP
        sprintf(godname_buff, "%s시프 무나", long_name ? "지식의 수호자 " : "");
#else
        sprintf(godname_buff, "Sif Muna%s", long_name ? " the Loreminder" : "");
#endif
        break;
    case GOD_TROG:
#ifdef JP
        sprintf(godname_buff, "%s트로그", long_name ? "진노하는 " : "");
#else
        sprintf(godname_buff, "Trog%s", long_name ? " the Wrathful" : "");
#endif
        break;
    case GOD_NEMELEX_XOBEH:
#ifdef JP
        sprintf(godname_buff, "네멜렉스%s", long_name ? "·조베" : "");
#else
        strcpy(godname_buff, "Nemelex Xobeh");
#endif
        break;
    case GOD_ELYVILON:
#ifdef JP
        sprintf(godname_buff, "%s엘리빌론", long_name ? "치유하는 자 " : "");
#else
        sprintf(godname_buff, "Elyvilon%s", long_name ? " the Healer" : "");
#endif
        break;
    default:
#ifdef JP
        sprintf(godname_buff, "버그인 (%d)", which_god);
#else
        sprintf(godname_buff, "The Buggy One (%d)", which_god);
#endif
    }

    return (godname_buff);
}                               // end god_name()

void god_speaks( int god, const char *mesg )
{
    mpr( mesg, MSGCH_GOD, god );
}                               // end god_speaks()

void Xom_acts(bool niceness, int sever, bool force_sever)
{
    // niceness = false - bad, true - nice
    int temp_rand;              // probability determination {dlb}
    bool done_bad = false;      // flag to clarify logic {dlb}
    bool done_good = false;     // flag to clarify logic {dlb}

    struct bolt beam;

    if (sever < 1)
        sever = 1;

    if (!force_sever)
        sever = random2(sever);

    if (sever == 0)
        return;

  okay_try_again:

    if (!niceness || one_chance_in(3))
    {
        // begin "Bad Things"
        done_bad = false;

        // this should always be first - it will often be called
        // deliberately, with a low sever value
        if (random2(sever) <= 2)
        {
            temp_rand = random2(4);

#ifdef JP
            god_speaks(GOD_XOM,
                (temp_rand == 0) ? "좀은 당신에게로 눈을 돌렸다." :
                (temp_rand == 1) ? "좀은 잠깐 동안 당신에게 주목했다.":
                (temp_rand == 2) ? "좀의 힘이 잠깐 동안 당신에게 닿았다."
                                 : "당신은 좀이 미친 듯이 웃는 소리를 들었다.");
#else
            god_speaks(GOD_XOM,
                (temp_rand == 0) ? "Xom notices you." :
                (temp_rand == 1) ? "Xom's attention turns to you for a moment.":
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal laughter.");
#endif

            miscast_effect( SPTYP_RANDOM, 5 + random2(10), random2(100), 0,
#ifdef JP
                            "좀의 변덕" );
#else
                            "the capriciousness of Xom" );
#endif

            done_bad = true;
        }
        else if (random2(sever) <= 2)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『괴로워해라!』" :
                (temp_rand == 1) ? "좀의 악의 있는 주시가, 잠깐 동안 당신에게 집중되었다." :
                (temp_rand == 2) ? "좀의 힘이 잠깐 동안 당신에게 닿았다."
                                 : "당신은 좀이 미친 듯이 웃는 소리를 들었다.");
#else
                (temp_rand == 0) ? "\"Suffer!\"" :
                (temp_rand == 1) ? "Xom's malign attention turns to you for a moment." :
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal laughter.");
#endif

            lose_stat(STAT_RANDOM, 1 + random2(3), true);

            done_bad = true;
        }
        else if (random2(sever) <= 2)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "좀은 당신에게로 눈을 돌렸다." :
                (temp_rand == 1) ? "좀은 잠깐 동안 당신에게 주목했다.":
                (temp_rand == 2) ? "좀의 힘이 잠깐 동안 당신에게 닿았다."
                                 : "당신은 좀이 미친 듯이 웃는 소리를 들었다.");
#else
                (temp_rand == 0) ? "Xom notices you." :
                (temp_rand == 1) ? "Xom's attention turns to you for a moment.":
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal laughter.");
#endif

            miscast_effect( SPTYP_RANDOM, 5 + random2(15), random2(250), 0,
#ifdef JP
                            "좀의 변덕" );
#else
                            "the capriciousness of Xom" );
#endif

            done_bad = true;
        }
        else if (!you.is_undead && random2(sever) <= 3)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『너에게는 약간의 수정이 필요하겠군!』" :
                (temp_rand == 1) ? "『너의 보잘것없는 몸뚱아리를 개조해주지.』" :
                (temp_rand == 2) ? "좀의 힘이 잠깐 동안 당신에게 닿았다."
                                 : "당신은 좀이 미친 듯이 웃는 소리를 들었다.");
#else
                (temp_rand == 0) ? "\"You need some minor adjustments, mortal!\"" :
                (temp_rand == 1) ? "\"Let me alter your pitiful body.\"" :
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal laughter.");
#endif

#ifdef JP
            mpr("당신의 육체는 왜곡의 에너지로 뒤덮였다.");
#else
            mpr("Your body is suffused with distortional energy.");
#endif

            set_hp(1 + random2(you.hp), false);
            deflate_hp(you.hp_max / 2, true);

            bool failMsg = true;
            for (int i = 0; i < 4; i++)
            {
                if (!mutate(100, failMsg))
                    failMsg = false;
            }

            done_bad = true;
        }
        else if (!you.is_undead && random2(sever) <= 3)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『너는 정말로 마음에 안 들어.』" :
                (temp_rand == 1) ? "『너는 무가치한 주제에 너무 기세등등해 있어.』" :
                (temp_rand == 2) ? "좀의 힘이 잠깐 동안 당신에게 닿았다."
                                 : "당신은 좀이 미친 듯이 웃는 소리를 들었다.");
#else
                (temp_rand == 0) ? "\"You have displeased me, mortal.\"" :
                (temp_rand == 1) ? "\"You have grown too confident for your meagre worth.\"" :
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal laughter.");
#endif

            if (one_chance_in(4))
            {
                drain_exp();
                if (random2(sever) > 3)
                    drain_exp();
                if (random2(sever) > 3)
                    drain_exp();
            }
            else
            {
#ifdef JP
                mpr("고통의 파동이 당신의 전신을 찢어놓았다!");
#else
                mpr("A wave of agony tears through your body!");
#endif
                set_hp(1 + (you.hp / 2), false);
            }

            done_bad = true;
        }
        else if (random2(sever) <= 3)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『즐길 시간이다!』" :
                (temp_rand == 1) ? "『싸워서 살아남아 보아라.』" :
                (temp_rand == 2) ? "『너의 힘이 살아남기에 충분한지 시험해 보자.』"
                                 : "당신은 좀이 미친 듯이 웃는 소리를 들었다.");
#else
                (temp_rand == 0) ? "\"Time to have some fun!\"" :
                (temp_rand == 1) ? "\"Fight to survive, mortal.\"" :
                (temp_rand == 2) ? "\"Let's see if it's strong enough to survive yet.\""
                                 : "You hear Xom's maniacal laughter.");
#endif

            if (one_chance_in(4))
                dancing_weapon(100, true);      // nasty, but fun
            else
            {
                create_monster(MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                    BEH_HOSTILE, you.x_pos, you.y_pos, MHITNOT, 250);

                if (one_chance_in(3))
                    create_monster(MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                        BEH_HOSTILE, you.x_pos, you.y_pos, MHITNOT, 250);

                if (one_chance_in(4))
                    create_monster(MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                        BEH_HOSTILE, you.x_pos, you.y_pos, MHITNOT, 250);

                if (one_chance_in(3))
                    create_monster(MONS_HELLION + random2(10), ENCH_ABJ_III,
                        BEH_HOSTILE, you.x_pos, you.y_pos, MHITNOT, 250);

                if (one_chance_in(4))
                    create_monster(MONS_HELLION + random2(10), ENCH_ABJ_III,
                        BEH_HOSTILE, you.x_pos, you.y_pos, MHITNOT, 250);
            }

            done_bad = true;
        }
        else if (you.your_level == 0)
        {
            // this should remain the last possible outcome {dlb}
            temp_rand = random2(3);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『너는 그 하찮은 세계에서 너무 편안하게 안주하고 있어!』" :
                (temp_rand == 1) ? "좀이 당신을 어비스에 던져 넣었다!"
                                 : "좀이 미친 듯이 웃는 소리가 귀에 울리는 것과 동시에, 세계가 빙글빙글 돌아가기 시작했다.");
#else
                (temp_rand == 0) ? "\"You have grown too comfortable in your little world, mortal!\"" :
                (temp_rand == 1) ? "Xom casts you into the Abyss!"
                                 : "The world seems to spin as Xom's maniacal laughter rings in your ears.");
#endif

            banished(DNGN_ENTER_ABYSS);

            done_bad = true;
        }
    }                           // end "Bad Things"
    else
    {
        // begin "Good Things"
        done_good = false;

// Okay, now for the nicer stuff (note: these things are not necessarily nice):
        if (random2(sever) <= 2)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『가라, 그리고 파괴해라!』" :
                (temp_rand == 1) ? "『자, 가라, 그리고 파괴해라!』" :
                (temp_rand == 2) ? "좀은 당신에게 얼마간의 호의를 보였다."
                                 : "좀은 당신에게 미소짓고 있다.");
#else
                (temp_rand == 0) ? "\"Go forth and destroy!\"" :
                (temp_rand == 1) ? "\"Go forth and destroy, mortal!\"" :
                (temp_rand == 2) ? "Xom grants you a minor favour."
                                 : "Xom smiles on you.");
#endif

            switch (random2(7))
            {
            case 0:
                potion_effect(POT_HEALING, 150);
                break;
            case 1:
                potion_effect(POT_HEAL_WOUNDS, 150);
                break;
            case 2:
                potion_effect(POT_SPEED, 150);
                break;
            case 3:
                potion_effect(POT_MIGHT, 150);
                break;
            case 4:
                potion_effect(POT_INVISIBILITY, 150);
                break;
            case 5:
                if (one_chance_in(6))
                    potion_effect(POT_EXPERIENCE, 150);
                else
                {
                    you.berserk_penalty = NO_BERSERK_PENALTY;
                    potion_effect(POT_BERSERK_RAGE, 150);
                }
                break;
            case 6:
                you.berserk_penalty = NO_BERSERK_PENALTY;
                potion_effect(POT_BERSERK_RAGE, 150);
                break;
            }

            done_good = true;
        }
        else if (random2(sever) <= 4)
        {
            temp_rand = random2(3);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『나의 아이들아, 이 자를 섬겨라.』" :
                (temp_rand == 1) ? "좀은 당신에게 일시적인 도움을 주었다."
                                 : "좀은 문을 열었다.");
#else
                (temp_rand == 0) ? "\"Serve the mortal, my children!\"" :
                (temp_rand == 1) ? "Xom grants you some temporary aid."
                                 : "Xom opens a gate.");
#endif

            create_monster( MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                            BEH_FRIENDLY, you.x_pos, you.y_pos,
                            you.pet_target, 250 );

            create_monster( MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                            BEH_FRIENDLY, you.x_pos, you.y_pos,
                            you.pet_target, 250 );

            if (random2( you.experience_level ) >= 8)
            {
                create_monster( MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                                BEH_FRIENDLY, you.x_pos, you.y_pos,
                                you.pet_target, 250 );
            }

            if (random2( you.experience_level ) >= 8)
            {
                create_monster( MONS_HELLION + random2(10), ENCH_ABJ_III,
                                BEH_FRIENDLY, you.x_pos, you.y_pos,
                                you.pet_target, 250 );
            }

            if (random2( you.experience_level ) >= 8)
            {
                create_monster( MONS_HELLION + random2(10), ENCH_ABJ_III,
                                BEH_FRIENDLY, you.x_pos, you.y_pos,
                                you.pet_target, 250 );
            }

            done_good = true;
        }
        else if (random2(sever) <= 3)
        {
            temp_rand = random2(3);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『나의 호의의 증표로 이것을 받아라.』" :
                (temp_rand == 1) ? "좀은 당신에게 선물을 하사했다!"
                                 : "좀은 자신이 통이 크다는 것을 자랑했다.");
#else
                (temp_rand == 0) ? "\"Take this token of my esteem.\"" :
                (temp_rand == 1) ? "Xom grants you a gift!"
                                 : "Xom's generous nature manifests itself.");
#endif

            if (grd[you.x_pos][you.y_pos] == DNGN_LAVA
                || grd[you.x_pos][you.y_pos] == DNGN_DEEP_WATER)
            {
                // How unfortunate. I'll bet Xom feels sorry for you.
#ifdef JP
                mpr("당신은 물보라 소리를 들었다.");
#else
                mpr("You hear a splash.");
#endif
            }
            else
            {
                int thing_created = items(1, OBJ_RANDOM, OBJ_RANDOM, true,
                                              you.experience_level * 3, 250);

                move_item_to_grid( &thing_created, you.x_pos, you.y_pos );

                if (thing_created != NON_ITEM)
                {
                    canned_msg(MSG_SOMETHING_APPEARS);
                    more();
                }
            }

            done_good = true;
        }
        else if (random2(sever) <= 4)
        {
            const int demon = (random2(you.experience_level) < 6)
                                                ? MONS_WHITE_IMP + random2(5)
                                                : MONS_NEQOXEC + random2(5);

            if (create_monster( demon, 0, BEH_FRIENDLY, you.x_pos, you.y_pos,
                                                 you.pet_target, 250 ) != -1)
            {
                temp_rand = random2(3);

                god_speaks(GOD_XOM,
#ifdef JP
                    (temp_rand == 0) ? "나의 아이야, 이 자를 섬겨라." :
                    (temp_rand == 1) ? "좀이 당신에게 악마 하인을 빌려주었다."
                                     : "좀은 문을 열었다.");
#else
                    (temp_rand == 0) ? "\"Serve the mortal, my child!\"" :
                    (temp_rand == 1) ? "Xom grants you a demonic servitor."
                                     : "Xom opens a gate.");
#endif
            }

            done_good = true;   // well, for Xom, trying == doing {dlb}
        }
        else if (random2(sever) <= 4)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『이 무기로 파괴를 일으켜라!』" :
                (temp_rand == 1) ? "『너는 선물을 받을 만한 자격이 있어.』" :
                (temp_rand == 2) ? "좀은 당신에게, 죽음을 가져오는 무기를 주었다."
                                 : "좀은 당신에게 미소짓고 있다.");
#else
                (temp_rand == 0) ? "\"Take this instrument of destruction!\"" :
                (temp_rand == 1) ? "\"You have earned yourself a gift.\"" :
                (temp_rand == 2) ? "Xom grants you an implement of death."
                                 : "Xom smiles on you.");
#endif

            if (acquirement(OBJ_WEAPONS))
                more();

            done_good = true;
        }
        else if (!you.is_undead && random2(sever) <= 5)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『너에게는 약간의 수정이 필요하겠군!』" :
                (temp_rand == 1) ? "『너의 보잘것없는 몸뚱아리를 개조해주지.』" :
                (temp_rand == 2) ? "좀의 힘이 잠깐 동안 당신에게 닿았다."
                                 : "당신은 좀이 미친 듯이 낄낄거리는 소리를 들었다.");
#else
                (temp_rand == 0) ? "\"You need some minor adjustments, mortal!\"" :
                (temp_rand == 1) ? "\"Let me alter your pitiful body.\"" :
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal chuckling.");
#endif

#ifdef JP
            mpr("당신의 육체는 왜곡의 에너지로 뒤덮였다.");
#else
            mpr("Your body is suffused with distortional energy.");
#endif

            set_hp(1 + random2(you.hp), false);
            deflate_hp(you.hp_max / 2, true);

            if (coinflip() || !give_cosmetic_mutation())
                give_good_mutation();

            done_good = true;
        }
        else if (random2(sever) <= 2)
        {
            // this should remain the last possible outcome {dlb}
            if (!one_chance_in(8))
                you.attribute[ATTR_DIVINE_LIGHTNING_PROTECTION] = 1;

#ifdef JP
            god_speaks(GOD_XOM, "이 일대에 신의 번개가 내리쳤다!");
#else
            god_speaks(GOD_XOM, "The area is suffused with divine lightning!");
#endif

            beam.beam_source = NON_MONSTER;
            beam.type = SYM_BURST;
            beam.damage = dice_def( 3, 30 );
            beam.flavour = BEAM_ELECTRICITY;
            beam.target_x = you.x_pos;
            beam.target_y = you.y_pos;
#ifdef JP
            strcpy(beam.beam_name, "번개 폭풍");
#else
            strcpy(beam.beam_name, "blast of lightning");
#endif
            beam.colour = LIGHTCYAN;
            beam.thrower = KILL_YOU;    // your explosion
#ifdef JP
            beam.aux_source = "좀의 뇌격";
#else
            beam.aux_source = "Xom's lightning strike";
#endif
            beam.ex_size = 2;
            beam.isTracer = false;

            explosion(beam);

            if (you.attribute[ATTR_DIVINE_LIGHTNING_PROTECTION] == 1)
            {
#ifdef JP
                mpr("당신에게서 신의 보호가 사라졌다.");
#else
                mpr("Your divine protection wanes.");
#endif
                you.attribute[ATTR_DIVINE_LIGHTNING_PROTECTION] = 0;
            }

            done_good = true;
        }
    }                           // end "Good Things"

    if (done_bad || done_good || one_chance_in(4))
        return;
    else
        goto okay_try_again;
}                               // end Xom_acts()

void done_good(char thing_done, int pgain)
{
    if (you.religion == GOD_NO_GOD)
        return;

    switch (thing_done)
    {
    case GOOD_KILLED_LIVING:
        switch (you.religion)
        {
        case GOD_ELYVILON:
#ifdef JP
            simple_god_message("은(는) 그것을 기뻐하지 않았다!");
#else
            simple_god_message(" did not appreciate that!");
#endif
            naughty(NAUGHTY_KILLING, 10);
            break;
        case GOD_KIKUBAAQUDGHA:
        case GOD_YREDELEMNUL:
        case GOD_VEHUMET:
        case GOD_OKAWARU:
        case GOD_MAKHLEB:
        case GOD_TROG:
#ifdef JP
            simple_god_message("은(는) 당신의 살생을 기뻐했다.");
#else
            simple_god_message(" accepts your kill.");
#endif
            if (random2(18 + pgain) > 5)
                gain_piety(1);
            break;
        }
        break;

    case GOOD_KILLED_UNDEAD:
        switch (you.religion)
        {
        case GOD_ZIN:
        case GOD_SHINING_ONE:
        case GOD_VEHUMET:
        case GOD_MAKHLEB:
        case GOD_OKAWARU:
#ifdef JP
            simple_god_message("은(는) 당신의 살생을 기뻐했다.");
#else
            simple_god_message(" accepts your kill.");
#endif
            if (random2(18 + pgain) > 4)
                gain_piety(1);
            break;
        }
        break;

    case GOOD_KILLED_DEMON:
        switch (you.religion)
        {
        case GOD_ZIN:
        case GOD_SHINING_ONE:
        case GOD_VEHUMET:
        case GOD_MAKHLEB:
        case GOD_OKAWARU:
#ifdef JP
            simple_god_message("은(는) 당신의 살생을 기뻐했다.");
#else
            simple_god_message(" accepts your kill.");
#endif
            if (random2(18 + pgain) > 3)
                gain_piety(1);
            break;
        }
        break;

    case GOOD_KILLED_ANGEL_I:
    case GOOD_KILLED_ANGEL_II:
        switch (you.religion)
        {
        case GOD_ZIN:
        case GOD_SHINING_ONE:
        case GOD_ELYVILON:
#ifdef JP
            simple_god_message("은(는) 그것을 비난했다!");
#else
            simple_god_message(" did not appreciate that!");
#endif
            naughty(NAUGHTY_ATTACK_HOLY, (you.conf ? 3 : pgain * 3));
            break;
        }
        break;

    case GOOD_KILLED_WIZARD:
        // hooking this up, but is it too good?
        // enjoy it while you can -- bwr
        if (you.religion == GOD_TROG)
        {
#ifdef JP
            simple_god_message( "은(는) 당신이 마법 사용자를 죽인 것을 기뻐했다." );
#else
            simple_god_message( " appreciates your killing of a magic user." );
#endif

            if (random2( 5 + pgain ) > 5)
                gain_piety(1);
        }
        break;

    case GOOD_HACKED_CORPSE:    // NB - pgain is you.experience_level (maybe)
        switch (you.religion)
        {
        // case GOD_KIKUBAAQUDGHA:
        case GOD_OKAWARU:
        case GOD_MAKHLEB:
        case GOD_TROG:
#ifdef JP
            simple_god_message("은(는) 당신의 공물을 받아들였다.");
#else
            simple_god_message(" accepts your offering.");
#endif
            if (random2(10 + pgain) > 5)
                gain_piety(1);
            break;

        // case GOD_ZIN:
        // case GOD_SHINING_ONE:
        case GOD_ELYVILON:
#ifdef JP
            simple_god_message("은(는) 그것을 기뻐하지 않았다!");
#else
            simple_god_message(" did not appreciate that!");
#endif

            naughty(NAUGHTY_BUTCHER, 8);
            break;
        }
        break;

    case GOOD_OFFER_STUFF:
#ifdef JP
        simple_god_message("은(는) 당신의 공물에 만족했다.");
#else
        simple_god_message(" is pleased with your offering.");
#endif

        gain_piety(1);
        break;

    case GOOD_SLAVES_KILL_LIVING:
        switch (you.religion)
        {
        case GOD_KIKUBAAQUDGHA:
        case GOD_YREDELEMNUL:
        case GOD_VEHUMET:
#ifdef JP
            simple_god_message("은(는) 당신의 노예가 적을 죽인 것을 기뻐했다.");
#else
            simple_god_message(" accepts your slave's kill.");
#endif

            if (random2(pgain + 18) > 5)
                gain_piety(1);
            break;
        }
        break;

    case GOOD_SERVANTS_KILL:
        switch (you.religion)
        {
        case GOD_VEHUMET:
        case GOD_MAKHLEB:
#ifdef JP
            simple_god_message("은(는) 당신의 부수적인 살생을 기뻐했다.");
#else
            simple_god_message(" accepts your collateral kill.");
#endif

            if (random2(pgain + 18) > 5)
                gain_piety(1);
            break;
        }
        break;

    case GOOD_CARDS:
        switch (you.religion)
        {
        case GOD_NEMELEX_XOBEH:
            gain_piety(pgain);
            break;
        }
        break;
    // Offering at altars is covered in another function.
    }
}                               // end done_good()

void gain_piety(char pgn)
{
    // check to see if we owe anything first
    if (you.penance[you.religion] > 0)
    {
        dec_penance(pgn);
        return;
    }
    else if (you.gift_timeout > 0)
    {
        if (you.gift_timeout > pgn)
            you.gift_timeout -= pgn;
        else
            you.gift_timeout = 0;

        // Slow down piety gain to account for the fact that gifts
        // no longer have a piety cost for getting them
        if (!one_chance_in(8))
            return;
    }

    // slow down gain at upper levels of piety
    if (you.piety > 199
        || (you.piety > 150 && one_chance_in(3))
        || (you.piety > 100 && one_chance_in(3)))
        return;

    int old_piety = you.piety;

    you.piety += pgn;

    if (you.piety >= 30 && old_piety < 30)
    {
        switch (you.religion)
        {
        case GOD_NO_GOD:
        case GOD_XOM:
        case GOD_NEMELEX_XOBEH:
        case GOD_SIF_MUNA:
            break;
        default:
#ifdef JP
            strcpy(info, "당신은 이제 ");
#else
            strcpy(info, "You can now ");
#endif
            strcat(info,
                    (you.religion == GOD_ZIN || you.religion == GOD_SHINING_ONE)
#ifdef JP
                            ? "언데드를 물리칠 수 있다" :
#else
                            ? "repel the undead" :
#endif

                    (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                            ? "언데드 하인을 근처에 불러모을 수 있다" :
#else
                            ? "recall your undead slaves" :
#endif
                    (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                            ? "시체를 조종할 수 있다" :
#else
                            ? "animate corpses" :
#endif
                    (you.religion == GOD_VEHUMET)
#ifdef JP
                            ? "베후멧의 이름으로 한 살생으로 마나를 모을 수 있다" :
#else
                            ? "gain power from killing in Vehumet's name" :
#endif
                    (you.religion == GOD_MAKHLEB)
#ifdef JP
                            ? "마크레브의 이름으로 한 살생으로 마나를 모을 수 있다" :
#else
                            ? "gain power from killing in Makhleb's name" :
#endif
                    (you.religion == GOD_OKAWARU)
#ifdef JP
                            ? "일시적이기는 하지만 강대한 힘을 끌어낼 수 있다" :
#else
                            ? "give your great, but temporary, body strength" :
#endif
                    (you.religion == GOD_TROG)
#ifdef JP
                            ? "마음대로 광전사 상태가 될 수 있다" :
#else
                            ? "go berserk at will" :
#endif
                    (you.religion == GOD_ELYVILON)
#ifdef JP
                            ? "미약한 치유를 엘리빌론에게 청할 수 있다"
#else
                            ? "call upon Elyvilon for minor healing"
#endif
                // Unknown god
#ifdef JP
                            : "이 프로그램 버그를 견뎌낸다 @30");
#else
                            : "endure this program bug @30");
#endif

#ifdef JP
            strcat(info, ".");
#else
            strcat(info, ".");
#endif
            god_speaks(you.religion, info);
            break;
        }
    }

    if (you.piety >= 50 && old_piety < 50)
    {
        switch (you.religion)
        {
        case GOD_NO_GOD:
        case GOD_XOM:
        case GOD_NEMELEX_XOBEH:
            break;
        case GOD_KIKUBAAQUDGHA:
#ifdef JP
            simple_god_message("이(가) 당신을 죽음의 마술로 인한 부작용으로부터 보호해주고 있다.");
#else
            simple_god_message(" is protecting you from some side-effects of death magic.");
#endif
            break;

        case GOD_VEHUMET:
#ifdef JP
            god_speaks(you.religion, "당신은 베후멧에게 기도하여 파괴의 마법의 도움을 청할 수 있다.");
#else
            god_speaks(you.religion, "You can call upon Vehumet to aid your destructive magics with prayer.");
#endif
            break;

        default:
#ifdef JP
            strcpy(info, "당신은 이제 ");
#else
            strcpy(info, "You can now ");
#endif

            strcat(info,
                   (you.religion == GOD_ZIN)
#ifdef JP
                           ? "미약한 치유를 진에게 청할 수 있다" :
#else
                           ? "call upon Zin for minor healing" :
#endif
                   (you.religion == GOD_SHINING_ONE)
#ifdef JP
                           ? "적에게 강타를 날릴 수 있다" :
#else
                           ? "smite your foes" :
#endif
                   (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                           ? "언데드 하인을 근처에 불러모을 수 있다" :
#else
                           ? "recall your undead slaves" :
#endif
                   (you.religion == GOD_OKAWARU)
#ifdef JP
                           ? "미약한 치유를 오카와루에게 청할 수 있다" :
#else
                           ? "call upon Okawaru for minor healing" :
#endif
                   (you.religion == GOD_MAKHLEB)
#ifdef JP
                           ? "마크레브의 파괴의 힘을 다룰 수 있다" :
#else
                           ? "harness Makhleb's destructive might" :
#endif
                   (you.religion == GOD_SIF_MUNA)
#ifdef JP
                           ? "새로운 주문을 기억하기 위해 기억을 말소시킬 수 있다" :
#else
                           ? "freely open your mind to new spells" :
#endif
                   (you.religion == GOD_TROG)
#ifdef JP
                           ? "일시적이기는 하지만 강대한 힘을 끌어낼 수 있다" :
#else
                           ? "give your body great, but temporary, strength" :
#endif
                   (you.religion == GOD_ELYVILON)
#ifdef JP
                           ? "육체의 청정화를 엘리빌론에게 청할 수 있다"
#else
                           ? "call upon Elyvilon for purification"
#endif
                   // Unknown god
#ifdef JP
                           : "이 프로그램 버그를 견뎌낸다 @50");
#else
                           : "endure this program bug @50");
#endif

#ifdef JP
            strcat(info, ".");
#else
            strcat(info, ".");
#endif
            god_speaks(you.religion, info);
            break;
        }
    }

    if (you.piety >= 75 && old_piety < 75)
    {
        switch (you.religion)
        {
        case GOD_NO_GOD:
        case GOD_XOM:
        case GOD_OKAWARU:
        case GOD_NEMELEX_XOBEH:
        case GOD_SIF_MUNA:
        case GOD_TROG:
            break;
        case GOD_VEHUMET:
#ifdef JP
            god_speaks(you.religion,"당신은 기도할 때, 소환된 몬스터로부터 어느 정도 보호받는다.");
#else
            god_speaks(you.religion,"During prayer you have some protection from summoned creatures.");
#endif
            break;

        default:
#ifdef JP
            strcpy(info, "당신은 이제 ");
#else
            strcpy(info, "You can now ");
#endif
            strcat(info,
                     (you.religion == GOD_ZIN)
#ifdef JP
                                 ? "전염병의 발생을 청할 수 있다" :
#else
                                 ? "call down a plague" :
#endif
                     (you.religion == GOD_SHINING_ONE)
#ifdef JP
                                 ? "언데드를 멸할 수 있다" :
#else
                                 ? "dispel the undead" :
#endif
                     (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                                 ? "언데드를 계속적인 노예로 만들 수 있다" :
#else
                                 ? "permanently enslave the undead" :
#endif
                     (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                                 ? "죽은자의 군대를 부릴 수 있다" :
#else
                                 ? "animate legions of the dead" :
#endif
                     (you.religion == GOD_MAKHLEB)
#ifdef JP
                                 ? "마크레브의 미약한 하인을 소환할 수 있다" :
#else
                                 ? "summon a lesser servant of Makhleb" :
#endif
                     (you.religion == GOD_ELYVILON)
#ifdef JP
                                 ? "치유를 엘리빌론에게 청할 수 있다"
#else
                                 ? "call upon Elyvilon for moderate healing"
#endif
                     // Unknown god
#ifdef JP
                                 : "이 프로그램 버그를 견뎌낸다 @75");
#else
                                 : "endure this program bug @75");
#endif
#ifdef JP
            strcat(info, ".");
#else
            strcat(info, ".");
#endif
            god_speaks(you.religion, info);
            break;
        }
    }

    if (you.piety >= 100 && old_piety < 100)
    {
        switch (you.religion)
        {
        case GOD_NO_GOD:
        case GOD_XOM:
        case GOD_OKAWARU:
        case GOD_NEMELEX_XOBEH:
        case GOD_KIKUBAAQUDGHA:
            break;
        case GOD_SIF_MUNA:
            simple_god_message
#ifdef JP
                ("이(가) 당신을 주문 시전의 부작용으로부터 보호해주고 있다.");
#else
                (" is protecting you from some side-effects of spellcasting.");
#endif
            break;

        default:
#ifdef JP
            strcpy(info, "당신은 이제 ");
#else
            strcpy(info, "You can now ");
#endif

            strcat(info,
                        (you.religion == GOD_ZIN)
#ifdef JP
                                ? "성스러운 말씀을 입에 담을 수 있다" :
#else
                                ? "utter a Holy Word" :
#endif
                        (you.religion == GOD_SHINING_ONE)
#ifdef JP
                                ? "신의 진노의 화살을 날릴 수 있다" :
#else
                                ? "hurl bolts of divine anger" :
#endif
                        (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                                ? "주위에서 생명력을 빨아들일 수 있다" :
#else
                                ? "drain ambient lifeforce" :
#endif
                        (you.religion == GOD_VEHUMET)
#ifdef JP
                                ? "주변에서 마력을 빨아들일 수 있다" :
#else
                                ? "tap ambient magical fields" :
#endif
                        (you.religion == GOD_MAKHLEB)
#ifdef JP
                                ? "마크레브의 강력한 파괴를 날릴 수 있다" :
#else
                                ? "hurl Makhleb's greater destruction" :
#endif
                        (you.religion == GOD_TROG)
#ifdef JP
                                ? "자신을 가속시킬 수 있다" :
#else
                                ? "haste yourself" :
#endif
                        (you.religion == GOD_ELYVILON)
#ifdef JP
                                ? "능력의 회복을 엘리빌론에게 청할 수 있다"
#else
                                ? "call upon Elyvilon to restore your abilities"
#endif
                        // Unknown god
#ifdef JP
                                : "이 프로그램 버그를 견뎌낸다 @100");
#else
                                : "endure this program bug @100");
#endif

#ifdef JP
            strcat(info, ".");
#else
            strcat(info, ".");
#endif
            god_speaks(you.religion, info);
            break;
        }
    }

    if (you.piety >= 120 && old_piety < 120)
    {
        switch (you.religion)
        {
        case GOD_NO_GOD:
        case GOD_XOM:
        case GOD_NEMELEX_XOBEH:
        case GOD_VEHUMET:
        case GOD_SIF_MUNA:
        case GOD_TROG:
            break;
        default:
#ifdef JP
            strcpy(info, "당신은 이제 ");
#else
            strcpy(info, "You can now ");
#endif

            strcat(info,
                     (you.religion == GOD_ZIN)
#ifdef JP
                                ? "수호천사를 불러낼 수 있다" :
#else
                                ? "summon a guardian angel" :
#endif
                     (you.religion == GOD_SHINING_ONE)
#ifdef JP
                                ? "성스러운 전사를 불러낼 수 있다" :
#else
                                ? "summon a divine warrior" :
#endif
                     (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                                ? "죽음의 사자를 불러낼 수 있다" :
#else
                                ? "summon an emissary of Death" :
#endif
                     (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                                ? "언데드를 지배할 수 있다" :
#else
                                ? "control the undead" :
#endif
                     (you.religion == GOD_OKAWARU)
#ifdef JP
                                ? "자신을 가속시킬 수 있다" :
#else
                                ? "haste yourself" :
#endif
                     (you.religion == GOD_MAKHLEB)
#ifdef JP
                                ? "마크레브의 강력한 하인을 소환할 수 있다" :
#else
                                ? "summon a greater servant of Makhleb" :
#endif
                     (you.religion == GOD_ELYVILON)
#ifdef JP
                                ? "강력한 치유를 엘리빌론에게 청할 수 있다"
#else
                                ? "call upon Elyvilon for incredible healing"
#endif
                     // Unknown god
#ifdef JP
                                : "이 프로그램 버그를 견뎌낸다 @120");
#else
                                : "endure this program bug @120");
#endif

#ifdef JP
            strcat(info, ".");
#else
            strcat(info, ".");
#endif
            god_speaks(you.religion, info);
            break;
        }
    }
}                               // end gain_piety()

void naughty(char type_naughty, int naughtiness)
{
    int penance = 0;
    int piety_loss = 0;

    // if you currently worship no deity in particular, exit function {dlb}
    if (you.religion == GOD_NO_GOD)
        return;

    switch (you.religion)
    {
    case GOD_ZIN:
        switch (type_naughty)
        {
        case NAUGHTY_NECROMANCY:
        case NAUGHTY_UNHOLY:
        case NAUGHTY_ATTACK_HOLY:
            piety_loss = naughtiness;
            penance = piety_loss * 2;
            break;
        case NAUGHTY_ATTACK_FRIEND:
            piety_loss = naughtiness;
            penance = piety_loss * 3;
            break;
        case NAUGHTY_FRIEND_DIES:
            piety_loss = naughtiness;
            break;
        case NAUGHTY_BUTCHER:
            piety_loss = naughtiness;
            if (one_chance_in(3))
                penance = piety_loss;
            break;
        }
        break;

    case GOD_SHINING_ONE:
        switch (type_naughty)
        {
        case NAUGHTY_NECROMANCY:
        case NAUGHTY_UNHOLY:
        case NAUGHTY_ATTACK_HOLY:
            piety_loss = naughtiness;
            penance = piety_loss;
            break;
        case NAUGHTY_ATTACK_FRIEND:
            piety_loss = naughtiness;
            penance = piety_loss * 3;
            break;
        case NAUGHTY_FRIEND_DIES:
            piety_loss = naughtiness;
            break;
        case NAUGHTY_BUTCHER:
            piety_loss = naughtiness;
            if (one_chance_in(3))
                penance = piety_loss;
            break;
        case NAUGHTY_STABBING:
            piety_loss = naughtiness;
            if (one_chance_in(5))       // can be accidental so we're nice here
                penance = piety_loss;
            break;
        case NAUGHTY_POISON:
            piety_loss = naughtiness;
            penance = piety_loss * 2;
            break;
        }
        break;

    case GOD_ELYVILON:
        switch (type_naughty)
        {
        case NAUGHTY_NECROMANCY:
        case NAUGHTY_UNHOLY:
        case NAUGHTY_ATTACK_HOLY:
            piety_loss = naughtiness;
            penance = piety_loss;
            break;
        case NAUGHTY_KILLING:
            piety_loss = naughtiness;
            penance = piety_loss * 2;
            break;
        case NAUGHTY_ATTACK_FRIEND:
            piety_loss = naughtiness;
            penance = piety_loss * 3;
            break;
        // Healer god gets a bit more upset since you should have
        // used your healing powers to save them.
        case NAUGHTY_FRIEND_DIES:
            piety_loss = naughtiness;
            penance = piety_loss;
            break;
        case NAUGHTY_BUTCHER:
            piety_loss = naughtiness;
            if (one_chance_in(3))
                penance = piety_loss;
            break;
        }
        break;

    case GOD_OKAWARU:
        switch (type_naughty)
        {
        case NAUGHTY_ATTACK_FRIEND:
            piety_loss = naughtiness;
            penance = piety_loss * 3;
            break;
        case NAUGHTY_FRIEND_DIES:
            piety_loss = naughtiness;
            break;
        }
        break;

    case GOD_TROG:
        switch (type_naughty)
        {
        case NAUGHTY_SPELLCASTING:
            piety_loss = naughtiness;
            // This penance isn't so bad since its much easier to
            // gain piety with Trog than the other gods in this function.
            penance = piety_loss * 10;
            break;
        }
        break;
    }

    // exit function early iff piety loss is zero:
    if (piety_loss < 1)
        return;

    // output guilt message:
#ifdef JP
    strcpy(info, "당신은");
#else
    strcpy(info, "You feel");
#endif

#ifdef JP
    strcat(info, (piety_loss == 1) ? " 아주 약간 " :
                 (piety_loss <  5) ? " " :
                 (piety_loss < 10) ? " 강하게 "
                                   : " 극심하게 ");
#else
    strcat(info, (piety_loss == 1) ? " a little " :
                 (piety_loss <  5) ? " " :
                 (piety_loss < 10) ? " very "
                                   : " extremely ");
#endif

#ifdef JP
    strcat(info, "죄의식을 느꼈다.");
#else
    strcat(info, "guilty.");
#endif
    mpr(info);

    lose_piety(piety_loss);

    if (you.piety < 1)
        excommunication();
    else if (penance)       // Don't bother unless we're not kicking them out
    {
        //jmf: FIXME: add randomness to following message:
#ifdef JP
        god_speaks(you.religion, "『너는 죄의 대가를 치르게 될 것이다!』");
#else
        god_speaks(you.religion, "\"You will pay for your transgression, mortal!\"");
#endif
        inc_penance(penance);
    }
}                               // end naughty()

void lose_piety(char pgn)
{
    int old_piety = you.piety;

    if (you.piety - pgn < 0)
        you.piety = 0;
    else
        you.piety -= pgn;

    // Don't bother printing out these messages if you're under
    // penance, you wouldn't notice since all these abilities
    // are withheld.
    if (!player_under_penance() && you.piety != old_piety)
    {
        if (you.piety < 120 && old_piety >= 120)
        {
            switch (you.religion)
            {
            case GOD_NO_GOD:
            case GOD_XOM:
            case GOD_NEMELEX_XOBEH:
            case GOD_VEHUMET:
            case GOD_SIF_MUNA:
            case GOD_TROG:
                break;
            default:
#ifdef JP
                strcpy(info, "당신은 이제 ");
#else
                strcpy(info, "You can no longer ");
#endif

                strcat(info,
                           (you.religion == GOD_ZIN)
#ifdef JP
                                ? "수호천사를 불러낼 수 없다" :
#else
                                ? "summon guardian angels" :
#endif
                           (you.religion == GOD_SHINING_ONE)
#ifdef JP
                                ? "성스러운 전사를 불러낼 수 없다" :
#else
                                ? "summon divine warriors" :
#endif
                           (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                                ? "죽음의 사자를 불러낼 수 없다" :
#else
                                ? "summon Death's emissaries" :
#endif
                           (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                                ? "언데드를 지배할 수 없다" :
#else
                                ? "control undead beings" :
#endif
                           (you.religion == GOD_OKAWARU)
#ifdef JP
                                ? "자신을 가속시킬 수 없다" :
#else
                                ? "haste yourself" :
#endif
                           (you.religion == GOD_MAKHLEB)
#ifdef JP
                                ? "마크레브의 강력한 하인을 소환할 수 없다" :
#else
                                ? "summon a greater servant of Makhleb" :
#endif
                           (you.religion == GOD_ELYVILON)
#ifdef JP
                                ? "강력한 치유를 엘리빌론에게 청할 수 없다"
#else
                                ? "call upon Elyvilon for incredible healing"
#endif
                           // Unknown god
#ifdef JP
                                : "이 프로그램 버그를 견뎌낼 수 없다 @120");
#else
                                : "endure this program bug @120");
#endif

#ifdef JP
                strcat(info, ".");
#else
                strcat(info, ".");
#endif
                god_speaks(you.religion, info);
                break;
            }
        }

        if (you.piety < 100 && old_piety >= 100)
        {
            switch (you.religion)
            {
            case GOD_NO_GOD:
            case GOD_XOM:
            case GOD_OKAWARU:
            case GOD_NEMELEX_XOBEH:
            case GOD_KIKUBAAQUDGHA:
                break;
            case GOD_SIF_MUNA:
#ifdef JP
                god_speaks(you.religion,"시프 무나는 이제 당신을 주문의 실패로부터 지켜줄 수 없다.");
#else
                god_speaks(you.religion,"Sif Muna is no longer protecting you from miscast magic.");
#endif
                break;
            default:
#ifdef JP
                strcpy(info, "당신은 이제 ");
#else
                strcpy(info, "You can no longer ");
#endif
                strcat(info,
                        (you.religion == GOD_ZIN)
#ifdef JP
                            ? "성스러운 말씀을 입에 담을 수 없다" :
#else
                            ? "utter a Holy Word" :
#endif
                        (you.religion == GOD_ELYVILON)
#ifdef JP
                            ? "능력의 회복을 엘리빌론에게 청할 수 없다" :
#else
                            ? "call upon Elyvilon to restore your abilities" :
#endif
                        (you.religion == GOD_SHINING_ONE)
#ifdef JP
                            ? "신의 진노의 화살을 날릴 수 없다" :
#else
                            ? "hurl bolts of divine anger" :
#endif
                        (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                            ? "주위에서 생명력을 빨아들일 수 없다" :
#else
                            ? "drain ambient life force" :
#endif
                        (you.religion == GOD_VEHUMET)
#ifdef JP
                            ? "주변에서 마력을 빨아들일 수 없다" :
#else
                            ? "tap ambient magical fields" :
#endif
                        (you.religion == GOD_MAKHLEB)
#ifdef JP
                            ? "마크레브의 강력한 파괴를 날릴 수 없다" :
#else
                            ? "direct Makhleb's greater destructive powers" :
#endif
                        (you.religion == GOD_TROG)
#ifdef JP
                            ? "자신을 가속시킬 수 없다"
#else
                            ? "haste yourself"
#endif
                        // Unknown god
#ifdef JP
                            : "이 프로그램 버그를 견뎌낼 수 없다 @100");
#else
                            : "endure this program bug @100");
#endif

#ifdef JP
                strcat(info, ".");
#else
                strcat(info, ".");
#endif
                god_speaks(you.religion, info);
                break;
            }
        }

        if (you.piety < 75 && old_piety >= 75)
        {
            switch (you.religion)
            {
            case GOD_NO_GOD:
            case GOD_XOM:
            case GOD_OKAWARU:
            case GOD_NEMELEX_XOBEH:
            case GOD_SIF_MUNA:
            case GOD_TROG:
                break;
            case GOD_VEHUMET:
#ifdef JP
                simple_god_message("은(는) 이제 소환된 몬스터로부터 당신을 지켜줄 수 없다.");
#else
                simple_god_message(" will longer shield you from summoned creatures.");
#endif
                break;
            default:
#ifdef JP
                strcpy(info, "당신은 이제 ");
#else
                strcpy(info, "You can no longer ");
#endif

                strcat(info,
                       (you.religion == GOD_ZIN)
#ifdef JP
                                ? "전염병의 발생을 청할 수 없다" :
#else
                                ? "call down a plague" :
#endif
                       (you.religion == GOD_SHINING_ONE)
#ifdef JP
                                ? "언데드를 멸할 수 없다" :
#else
                                ? "dispel undead" :
#endif
                       (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                                ? "언데드를 노예로 만들 수 없다" :
#else
                                ? "enslave undead" :
#endif
                       (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                                ? "죽은자의 군대를 부릴 수 없다" :
#else
                                ? "animate legions of the dead" :
#endif
                       (you.religion == GOD_MAKHLEB)
#ifdef JP
                                ? "마크레브의 하인을 소환할 수 없다" :
#else
                                ? "summon a servant of Makhleb" :
#endif
                       (you.religion == GOD_ELYVILON)
#ifdef JP
                                ? "치유를 엘리빌론에게 청할 수 없다"
#else
                                ? "call upon Elyvilon for moderate healing"
#endif
                       // Unknown god
#ifdef JP
                                : "이 프로그램 버그를 견뎌낸다 @75");
#else
                                : "endure this program bug @75");
#endif

#ifdef JP
                strcat(info, ".");
#else
                strcat(info, ".");
#endif
                god_speaks(you.religion, info);
                break;
            }
        }

        if (you.piety < 50 && old_piety >= 50)
        {
            switch (you.religion)
            {
            case GOD_NO_GOD:
            case GOD_XOM:
            case GOD_NEMELEX_XOBEH:
                break;
            case GOD_KIKUBAAQUDGHA:
#ifdef JP
                simple_god_message("은(는) 이제 당신을 죽음의 마술의 실패로부터 지켜줄 수 없다.");
#else
                simple_god_message(" is no longer shielding you from miscast death magic.");
#endif
                break;
            case GOD_VEHUMET:
#ifdef JP
                simple_god_message("은(는) 이제 당신의 파괴의 마법을 도와줄 수 없다.");
#else
                simple_god_message(" will no longer aid your destructive magics.");
#endif
                break;

            default:
#ifdef JP
                strcpy(info, "당신은 이제 ");
#else
                strcpy(info, "You can no longer ");
#endif

                strcat(info,
                       (you.religion == GOD_ZIN)
#ifdef JP
                            ? "미약한 치유를 진에게 청할 수 없다" :
#else
                            ? "call upon Zin for minor healing" :
#endif
                       (you.religion == GOD_SHINING_ONE)
#ifdef JP
                            ? "적에게 강타를 날릴 수 없다" :
#else
                            ? "smite your foes" :
#endif
                       (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                            ? "언데드 하인을 근처에 불러모을 수 없다" :
#else
                            ? "recall your undead slaves" :
#endif
                       (you.religion == GOD_OKAWARU)
#ifdef JP
                            ? "미약한 치유를 오카와루에게 청할 수 없다" :
#else
                            ? "call upon Okawaru for minor healing" :
#endif
                       (you.religion == GOD_MAKHLEB)
#ifdef JP
                            ? "마크레브의 파괴의 힘을 다룰 수 없다" :
#else
                            ? "hurl Makhleb's destruction" :
#endif
                       (you.religion == GOD_SIF_MUNA)
#ifdef JP
                            ? "마음대로 주문을 잊어버릴 수 없다" :
#else
                            ? "forget spells at will" :
#endif
                       (you.religion == GOD_TROG)
#ifdef JP
                            ? "강대한 힘을 끌어낼 수 없다" :
#else
                            ? "give your body great, but temporary, strength" :
#endif
                       (you.religion == GOD_ELYVILON)
#ifdef JP
                            ? "육체의 청정화를 엘리빌론에게 청할 수 없다"
#else
                            ? "call upon Elyvilon for Purification"
#endif
                       // Unknown god
#ifdef JP
                            : "이 프로그램 버그를 견뎌낼 수 없다 @50");
#else
                            : "endure this program bug @50");
#endif

#ifdef JP
                strcat(info, ".");
#else
                strcat(info, ".");
#endif
                god_speaks(you.religion, info);
                break;
            }
        }

        if (you.piety < 30 && old_piety >= 30)
        {
            switch (you.religion)
            {
            case GOD_NO_GOD:
            case GOD_XOM:
            case GOD_NEMELEX_XOBEH:
            case GOD_SIF_MUNA:
                break;
            default:
#ifdef JP
                strcpy(info, "당신은 이제 ");
#else
                strcpy(info, "You can no longer ");
#endif

                strcat(info,
                    (you.religion == GOD_ZIN || you.religion == GOD_SHINING_ONE)
#ifdef JP
                            ? "언데드를 물리칠 수 없다" :
#else
                            ? "repel the undead" :
#endif
                    (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                            ? "언데드 하인을 근처에 불러모을 수 없다" :
#else
                            ? "recall your undead slaves" :
#endif
                    (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                            ? "시체를 조종할 수 없다" :
#else
                            ? "animate corpses" :
#endif
                    (you.religion == GOD_VEHUMET)
#ifdef JP
                            ? "베후멧의 이름으로 한 살생으로 더이상 마나를 모을 수 없다" :
#else
                            ? "gain power from killing in Vehumet's name" :
#endif
                    (you.religion == GOD_MAKHLEB)
#ifdef JP
                            ? "마크레브의 이름으로 한 살생으로 더이상 마나를 모을 수 없다" :
#else
                            ? "gain power from killing in Makhleb's name" :
#endif
                    (you.religion == GOD_OKAWARU)
#ifdef JP
                            ? "강대한 힘을 끌어낼 수 없다" :
#else
                            ? "give your body great, but temporary, strength" :
#endif
                    (you.religion == GOD_TROG)
#ifdef JP
                            ? "마음대로 광전사 상태가 될 수 없다" :
#else
                            ? "go berserk at will" :
#endif
                    (you.religion == GOD_ELYVILON)
#ifdef JP
                            ? "미약한 치유를 엘리빌론에게 청할 수 없다"
#else
                            ? "call upon Elyvilon for minor healing."
#endif
                    // Unknown god
#ifdef JP
                            : "이 프로그램 버그를 견뎌낼 수 없다 @30");
#else
                            : "endure this program bug @30");
#endif

#ifdef JP
                strcat(info, ".");
#else
                strcat(info, ".");
#endif
                god_speaks(you.religion, info);
                break;
            }
        }
    }
}                               // end lose_piety()

void divine_retribution( int god )
{
    ASSERT(god != GOD_NO_GOD);

    int loopy = 0;              // general purpose loop variable {dlb}
    int temp_rand = 0;          // probability determination {dlb}
    int punisher = MONS_PROGRAM_BUG;
    bool success = false;
    int how_many = 0;
    int divine_hurt = 0;

    // Good gods don't use divine retribution on their followers, they
    // will consider it for those who have gone astray however.
    if (god == you.religion)
    {
        if (god == GOD_SHINING_ONE || god == GOD_ZIN || god == GOD_ELYVILON)
            return;
    }

    // Just the thought of retribution (getting this far) mollifies the
    // god by a point... the punishment might reduce penance further.
    dec_penance( god, 1 + random2(3) );

    switch (god)
    {
    case GOD_XOM:
        {
            // One in ten chance that Xom might do something good...
            // but that isn't forced, bad things are though
            bool nice = one_chance_in(10);
            bool force = !nice;

            Xom_acts(nice, you.experience_level, force);
        }
        break;

    case GOD_SHINING_ONE:
        // daeva/smiting theme
        // Doesn't care unless you've gone over to evil/destructive gods
        if (you.religion == GOD_KIKUBAAQUDGHA || you.religion == GOD_MAKHLEB
            || you.religion == GOD_YREDELEMNUL || you.religion == GOD_VEHUMET)
        {
            if (coinflip())
            {
                success = false;
                how_many = 1 + random2(you.experience_level / 5) + random2(3);

                for (loopy = 0; loopy < how_many; loopy++)
                {
                    if (create_monster( MONS_DAEVA, 0, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250) != -1)
                    {
                        success = true;
                    }
                }

                if (success)
                {
#ifdef JP
                    simple_god_message( "은(는) 당신의 악행을 처벌하기 위해 신의 대행자를 보냈다!", god );
#else
                    simple_god_message( " sends the divine host to punish you for your evil ways!", god );
#endif
                }
            }
            else
            {
                divine_hurt = 10 + random2(10);

                for (loopy = 0; loopy < 5; loopy++)
                    divine_hurt += random2( you.experience_level );

                if (!player_under_penance() && you.piety > random2(400))
                {
#ifdef JP
                    strcpy(info, "샤이닝 원의 분노를 피해 갈 수 있게 주지"
#else
                    strcpy(info, "Mortal, I have averted the wrath of "
#endif
#ifdef JP
                        "...이번에는.");
#else
                        "the Shining One... this time.");
#endif
                    god_speaks(you.religion, info);
                }
                else
                {
#ifdef JP
                    simple_god_message( "은(는) 당신에게 일격을 날렸다!", god );
#else
                    simple_god_message( " smites you!", god );
#endif
                    ouch( divine_hurt, 0, KILLED_BY_TSO_SMITING );
                    dec_penance( GOD_SHINING_ONE, 1 );
                }
            }
        }
        break;

    case GOD_ZIN:
        // angels/creeping doom theme:
        // Doesn't care unless you've gone over to evil
        if (you.religion == GOD_KIKUBAAQUDGHA || you.religion == GOD_MAKHLEB
            || you.religion == GOD_YREDELEMNUL || you.religion == GOD_VEHUMET)
        {
            if (random2(you.experience_level) > 7 && !one_chance_in(5))
            {
                success = false;
                how_many = 1 + (you.experience_level / 10) + random2(3);

                for (loopy = 0; loopy < how_many; loopy++)
                {
                    if (create_monster(MONS_ANGEL, 0, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250) != -1)
                    {
                        success = true;
                    }
                }

                if (success)
                {
#ifdef JP
                    simple_god_message("은(는) 당신의 악행을 처벌하기 위해 신의 대행자를 보냈다!", god);
#else
                    simple_god_message(" sends the divine host to punish you for your evil ways!", god);
#endif
                }
            }
            else
            {
                // god_gift == false gives unfriendly
                summon_swarm( you.experience_level * 20, true, false );
#ifdef JP
                simple_god_message("은(는) 당신에게 전염병을 발산했다!", god);
#else
                simple_god_message(" sends a plague down upon you!", god);
#endif
            }
        }
        break;

    case GOD_MAKHLEB:
        // demonic servant theme
        if (random2(you.experience_level) > 7 && !one_chance_in(5))
        {
            if (create_monster(MONS_EXECUTIONER + random2(5), 0,
                               BEH_HOSTILE, you.x_pos, you.y_pos,
                               MHITYOU, 250) != -1)
            {
#ifdef JP
                simple_god_message("은(는) 당신의 후임자로 강력한 하인을 보냈다!",
#else
                simple_god_message(" sends a greater servant after you!",
#endif
                                   god);
            }
        }
        else
        {
            success = false;
            how_many = 1 + (you.experience_level / 7);

            for (loopy = 0; loopy < how_many; loopy++)
            {
                if (create_monster(MONS_NEQOXEC + random2(5), 0, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250) != -1)
                {
                    success = true;
                }
            }

            if (success)
#ifdef JP
                simple_god_message("은(는) 당신을 처벌하기 위해 부하를 보냈다.", god);
#else
                simple_god_message(" sends minions to punish you.", god);
#endif
        }
        break;

    case GOD_KIKUBAAQUDGHA:
        // death/necromancy theme
        if (random2(you.experience_level) > 7 && !one_chance_in(5))
        {
            success = false;
            how_many = 1 + (you.experience_level / 5) + random2(3);

            for (loopy = 0; loopy < how_many; loopy++)
            {
                if (create_monster(MONS_REAPER, 0, BEH_HOSTILE, you.x_pos,
                                   you.y_pos, MHITYOU, 250) != -1)
                {
                    success = true;
                }
            }

            if (success)
#ifdef JP
                simple_god_message("은(는) 당신에게 사신을 보냈다!", god);
#else
                simple_god_message(" unleashes Death upon you!", god);
#endif
        }
        else
        {
#ifdef JP
            god_speaks(god, (coinflip()) ? "당신은 키쿠바쿠드하가 드높은 웃음소리를 내는 것을 들었다."
                                         : "키쿠바쿠드하의 악의가 당신에게 집중됐다.");
#else
            god_speaks(god, (coinflip()) ? "You hear Kikubaaqudgha cackling."
                                         : "Kikubaaqudgha's malice focuses upon you.");
#endif

            miscast_effect( SPTYP_NECROMANCY, 5 + you.experience_level,
#ifdef JP
                            random2avg(88, 3), 100, "키쿠바쿠드하의 악의" );
#else
                            random2avg(88, 3), 100, "the malice of Kikubaaqudgha" );
#endif
        }
        break;

    case GOD_YREDELEMNUL:
        // undead theme
        if (random2(you.experience_level) > 4)
        {
            success = false;
            how_many = 1 + random2(1 + (you.experience_level / 5));

            for (loopy = 0; loopy < how_many; loopy++)
            {
                temp_rand = random2(100);

                punisher = ((temp_rand > 66) ? MONS_WRAITH :            // 33%
                            (temp_rand > 52) ? MONS_WIGHT :             // 12%
                            (temp_rand > 40) ? MONS_SPECTRAL_WARRIOR :  // 16%
                            (temp_rand > 31) ? MONS_ROTTING_HULK :      //  9%
                            (temp_rand > 23) ? MONS_SKELETAL_WARRIOR :  //  8%
                            (temp_rand > 16) ? MONS_VAMPIRE :           //  7%
                            (temp_rand > 10) ? MONS_GHOUL :             //  6%
                            (temp_rand >  4) ? MONS_MUMMY               //  6%
                                             : MONS_FLAYED_GHOST);      //  5%

                if (create_monster( punisher, 0, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250 ) != -1)
                {
                    success = true;
                }
            }

            if (success)
#ifdef JP
                simple_god_message("은(는) 당신을 처벌하기 위해 하인을 보냈다.", god);
#else
                simple_god_message(" sends a servant to punish you.", god);
#endif
        }
        else
        {
#ifdef JP
            simple_god_message("의 진노가 잠깐 당신에게 향해졌다.",
#else
            simple_god_message("'s anger turns toward you for a moment.",
#endif
                               god);

            miscast_effect( SPTYP_NECROMANCY, 5 + you.experience_level,
#ifdef JP
                            random2avg(88, 3), 100, "이레델렘눌의 진노" );
#else
                            random2avg(88, 3), 100, "the anger of Yredelemnul" );
#endif
        }
        break;

    case GOD_TROG:
        // physical/berserk theme
        switch (random2(6))
        {
        case 0:
        case 1:
        case 2:
            {
                // Would be better if berserking monsters were available,
                // we just send some big bruisers for now.
                success = false;

                int points = 3 + you.experience_level * 3;

                while (points > 0)
                {
                    if (points > 20 && coinflip())
                    {
                        // quick reduction for large values
                        punisher = MONS_DEEP_TROLL;
                        points -= 15;
                        break;
                    }
                    else
                    {
                        switch (random2(10))
                        {
                        case 0:
                            punisher = MONS_IRON_TROLL;
                            points -= 10;
                            break;

                        case 1:
                            punisher = MONS_ROCK_TROLL;
                            points -= 10;
                            break;

                        case 2:
                            punisher = MONS_TROLL;
                            points -= 6;
                            break;

                        case 3:
                        case 4:
                            punisher = MONS_MINOTAUR;
                            points -= 3;
                            break;

                        case 5:
                        case 6:
                            punisher = MONS_TWO_HEADED_OGRE;
                            points -= 4;
                            break;

                        case 7:
                        case 8:
                        case 9:
                        default:
                            punisher = MONS_OGRE;
                            points -= 3;
                        }
                    }

                    if (create_monster(punisher, 0, BEH_HOSTILE, you.x_pos,
                                       you.y_pos, MHITYOU, 250) != -1)
                    {
                        success = true;
                    }
                }

                if (success)
#ifdef JP
                    simple_god_message("은(는) 당신을 처벌하기 위해 몬스터를 보냈다.", god);
#else
                    simple_god_message(" sends monsters to punish you.", god);
#endif
            }
            break;

        case 3:
        case 4:
#ifdef JP
            simple_god_message("의 목소리가 울려 퍼졌다. 『나의 분노를 뼈저리게 느껴 봐라!』", god );
#else
            simple_god_message("'s voice booms out, \"Feel my wrath!\"", god );
#endif

            // A collection of physical effects that might be better
            // suited to Trog than wild fire magic... messages could
            // be better here... something more along the lines of apathy
            // or loss of rage to go with the anti-berzerk effect-- bwr
            switch (random2(6))
            {
            case 0:
                potion_effect( POT_DECAY, 100 );
                break;

            case 1:
            case 2:
                lose_stat(STAT_STRENGTH, 1 + random2(you.strength / 5), true);
                break;

            case 3:
                if (!you.paralysis)
                {
                    dec_penance(GOD_TROG, 3);
#ifdef JP
                    mpr( "당신은 갑자기 정신을 잃었다!", MSGCH_WARN );
#else
                    mpr( "You suddenly pass out!", MSGCH_WARN );
#endif
                    you.paralysis = 2 + random2(6);
                }
                break;

            case 4:
            case 5:
                if (you.slow < 90)
                {
                    dec_penance( GOD_TROG, 1 );
#ifdef JP
                    mpr( "당신은 갑자기 몹시 지쳐 버렸다!", MSGCH_WARN );
#else
                    mpr( "You suddenly feel exhausted!", MSGCH_WARN );
#endif
                    you.exhausted = 100;
                    slow_player( 100 );
                }
                break;
            };
            break;
        //jmf: returned Trog's old Fire damage
        // -- actually, this function partially exists to remove that,
        //    we'll leave this effect in, but we'll remove the wild
        //    fire magic. -- bwr
        case 5:
            dec_penance(GOD_TROG, 2);
#ifdef JP
            mpr( "당신은 트로그의 불타오르는 진노가 당신에게 향해지는 것을 느꼈다!", MSGCH_WARN );
#else
            mpr( "You feel Trog's fiery rage upon you!", MSGCH_WARN );
#endif
            miscast_effect( SPTYP_FIRE, 8 + you.experience_level,
#ifdef JP
                            random2avg(98, 3), 100, "트로그의 불타오르는 진노" );
#else
                            random2avg(98, 3), 100, "the fiery rage of Trog" );
#endif
            break;
        }
        break;

    case GOD_OKAWARU:
        {
            // warrior theme:
            success = false;
            how_many = 1 + (you.experience_level / 5);

            for (loopy = 0; loopy < how_many; loopy++)
            {
                temp_rand = random2(100);

                punisher = ((temp_rand > 84) ? MONS_ORC_WARRIOR :
                            (temp_rand > 69) ? MONS_ORC_KNIGHT :
                            (temp_rand > 59) ? MONS_NAGA_WARRIOR :
                            (temp_rand > 49) ? MONS_CENTAUR_WARRIOR :
                            (temp_rand > 39) ? MONS_STONE_GIANT :
                            (temp_rand > 29) ? MONS_FIRE_GIANT :
                            (temp_rand > 19) ? MONS_FROST_GIANT :
                            (temp_rand >  9) ? MONS_CYCLOPS :
                            (temp_rand >  4) ? MONS_HILL_GIANT
                                             : MONS_TITAN);

                if (create_monster(punisher, 0, BEH_HOSTILE,
                                   you.x_pos, you.y_pos, MHITYOU, 250) != -1)
                {
                    success = true;
                }
            }

            if (success)
#ifdef JP
                simple_god_message("이(가) 당신에게 군대를 보냈다!", god);
#else
                simple_god_message(" sends forces against you!", god);
#endif
        }
        break;

    case GOD_VEHUMET:
        // conjuration and summoning theme
#ifdef JP
        simple_god_message("의 보복이 당신을 찾아냈다.", god);
#else
        simple_god_message("'s vengence finds you.", god);
#endif
        miscast_effect( coinflip() ? SPTYP_CONJURATION : SPTYP_SUMMONING,
                        8 + you.experience_level, random2avg(98, 3), 100,
#ifdef JP
                        "베후멧의 진노" );
#else
                        "the wrath of Vehumet" );
#endif
        break;

    case GOD_NEMELEX_XOBEH:
        // like Xom, this might actually help the player -- bwr`
#ifdef JP
        simple_god_message("은(는) 당신이 처벌의 덱에서 카드를 뽑도록 만들었다.",
#else
        simple_god_message(" makes you to draw from the Deck of Punishment.",
#endif
                           god);
        deck_of_cards(DECK_OF_PUNISHMENT);
        break;

    case GOD_SIF_MUNA:
#ifdef JP
        simple_god_message("의 진노가 당신을 찾아냈다.", god);
#else
        simple_god_message("'s wrath finds you.", god);
#endif
        dec_penance(GOD_SIF_MUNA, 1);

        // magic and intelligence theme:
        switch (random2(10))
        {
        case 0:
        case 1:
            lose_stat(STAT_INTELLIGENCE, 1 + random2( you.intel / 5 ), true);
            break;

        case 2:
        case 3:
        case 4:
            confuse_player( 3 + random2(10), false );
            break;

        case 5:
        case 6:
#ifdef JP
            //miscast_effect(SK_DIVINATIONS, 9, 90, 100, "시프 무나의 의지");
            miscast_effect(SPTYP_DIVINATION, 9, 90, 100, "시프 무나의 의지"); //fixed 2005/04/02
#else
            //miscast_effect(SK_DIVINATIONS, 9, 90, 100, "the will of Sif Muna");
            miscast_effect(SPTYP_DIVINATION, 9, 90, 100, "the will of Sif Muna"); //fixed 2005/04/02
#endif
            break;

        case 7:
        case 8:
            if (you.magic_points)
            {
                dec_mp( 100 );  // this should zero it.
#ifdef JP
                mpr( "당신은 갑자기 마력을 흡수당했다!",
#else
                mpr( "You suddenly feel drained of magical energy!",
#endif
                     MSGCH_WARN );
            }
            break;

        case 9:
            // This will set all the extendable duration spells to
            // a duration of one round, thus potentially exposing
            // the player to real danger.
            antimagic();
#ifdef JP
            mpr( "당신은 마법의 기세가 꺾이는 것을 느꼈다.", MSGCH_WARN );
#else
            mpr( "You sense a dampening of magic.", MSGCH_WARN );
#endif
            break;
        }
        break;

    case GOD_ELYVILON:  // Elyvilon doesn't seek revenge
    default:
        return;
    }

    // Sometimes divine experiences are overwelming...
    if (one_chance_in(5) && you.experience_level < random2(37))
    {
        if (coinflip())
            confuse_player( 3 + random2(10) );
        else
        {
            if (you.slow < 90)
            {
#ifdef JP
                mpr( "신과의 접촉이 당신을 몹시 지쳐 버리게했다!",
#else
                mpr( "The divine experience leaves you feeling exhausted!",
#endif
                     MSGCH_WARN );

                slow_player( random2(20) );
            }
        }
    }

    return;
}                               // end divine_retribution()

void excommunication(void)
{
    const int old_god = you.religion;

    you.duration[DUR_PRAYER] = 0;
    you.religion = GOD_NO_GOD;
    you.piety = 0;
    redraw_skill( you.your_name, player_title() );

#ifdef JP
    mpr("당신은 지금까지의 신앙을 버렸다!");
#else
    mpr("You have lost your religion!");
#endif
    more();

    switch (old_god)
    {
    case GOD_XOM:
        Xom_acts( false, (you.experience_level * 2), true );
        inc_penance( old_god, 50 );
        break;

    case GOD_KIKUBAAQUDGHA:
#ifdef JP
        simple_god_message( "은(는) 당신이 신앙을 파기한 것을 기분 좋게 생각하지 않는다!", old_god );
#else
        simple_god_message( " does not appreciate desertion!", old_god );
#endif
        miscast_effect( SPTYP_NECROMANCY, 5 + you.experience_level,
#ifdef JP
                        random2avg(88, 3), 100, "키쿠바쿠드하의 악의" );
#else
                        random2avg(88, 3), 100, "the malice of Kikubaaqudgha" );
#endif
        inc_penance( old_god, 30 );
        break;

    case GOD_YREDELEMNUL:
#ifdef JP
        simple_god_message( "은(는) 당신이 신앙을 파기한 것을 기분 좋게 생각하지 않는다!", old_god );
#else
        simple_god_message( " does not appreciate desertion!", old_god );
#endif
        miscast_effect( SPTYP_NECROMANCY, 5 + you.experience_level,
#ifdef JP
                        random2avg(88, 3), 100, "이레델렘눌의 역정" );
#else
                        random2avg(88, 3), 100, "the anger of Yredelemnul" );
#endif
        inc_penance( old_god, 30 );
        break;

    case GOD_VEHUMET:
#ifdef JP
        simple_god_message( "은(는) 당신이 신앙을 파기한 것을 기분 좋게 생각하지 않는다!", old_god );
#else
        simple_god_message( " does not appreciate desertion!", old_god );
#endif
        miscast_effect( (coinflip() ? SPTYP_CONJURATION : SPTYP_SUMMONING),
                        8 + you.experience_level, random2avg(98, 3), 100,
#ifdef JP
                        "베후멧의 분노" );
#else
                        "the wrath of Vehumet" );
#endif
        inc_penance( old_god, 25 );
        break;

    case GOD_MAKHLEB:
#ifdef JP
        simple_god_message( "은(는) 당신이 신앙을 파기한 것을 기분 좋게 생각하지 않는다!", old_god );
#else
        simple_god_message( " does not appreciate desertion!", old_god );
#endif
        miscast_effect( (coinflip() ? SPTYP_CONJURATION : SPTYP_SUMMONING),
                        8 + you.experience_level, random2avg(98, 3), 100,
#ifdef JP
                        "마크레브의 격분" );
#else
                        "the fury of Makhleb" );
#endif
        inc_penance( old_god, 25 );
        break;

    case GOD_TROG:
#ifdef JP
        simple_god_message( "은(는) 당신이 신앙을 파기한 것을 기분 좋게 생각하지 않는다!", old_god );
#else
        simple_god_message( " does not appreciate desertion!", old_god );
#endif

        // Penence has to come before retribution to prevent "mollify"
        inc_penance( old_god, 50 );
        divine_retribution( old_god );
        break;

    // these like to haunt players for a bit more than the standard
    case GOD_NEMELEX_XOBEH:
    case GOD_SIF_MUNA:
        inc_penance( old_god, 50 );
        break;

    default:
        inc_penance( old_god, 25 );
        break;

    case GOD_ELYVILON:  // never seeks revenge
        break;
    }
}                               // end excommunication()


void altar_prayer(void)
{
    int   i, j, next;
    char  subst_id[4][50];
    char str_pass[ ITEMNAME_SIZE ];

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 50; j++)
        {
            subst_id[i][j] = 1;
        }
    }

#ifdef JP
    mpr( "당신은 제단에 무릎꿇고, 기도를 드렸다." );
#else
    mpr( "You kneel at the altar and pray." );
#endif

    if (you.religion == GOD_SHINING_ONE || you.religion == GOD_XOM)
        return;

    i = igrd[you.x_pos][you.y_pos];
    while (i != NON_ITEM)
    {
        if (one_chance_in(1000))
            break;

        next = mitm[i].link;  // in case we can't get it later.

        const int value = item_value( mitm[i], subst_id, true );

        switch (you.religion)
        {
        case GOD_ZIN:
        case GOD_OKAWARU:
        case GOD_MAKHLEB:
        case GOD_NEMELEX_XOBEH:
#ifdef JP
            it_name(i, DESC_PLAIN, str_pass);
#else
            it_name(i, DESC_CAP_THE, str_pass);
#endif
            strcpy(info, str_pass);
            strcat(info, sacrifice[you.religion - 1]);
            mpr(info);

            if (mitm[i].base_type == OBJ_CORPSES
                || random2(value) >= 50
                || player_under_penance())
            {
                gain_piety(1);
            }

            destroy_item(i);
            break;

        case GOD_SIF_MUNA:
#ifdef JP
            it_name(i, DESC_PLAIN, str_pass);
#else
            it_name(i, DESC_CAP_THE, str_pass);
#endif
            strcpy(info, str_pass);
            strcat(info, sacrifice[you.religion - 1]);
            mpr(info);

            if (value >= 150)
                gain_piety(1 + random2(3));

            destroy_item(i);
            break;

        case GOD_KIKUBAAQUDGHA:
        case GOD_TROG:
            if (mitm[i].base_type != OBJ_CORPSES)
                break;

#ifdef JP
            it_name(i, DESC_PLAIN, str_pass);
#else
            it_name(i, DESC_CAP_THE, str_pass);
#endif
            strcpy(info, str_pass);
            strcat(info, sacrifice[you.religion - 1]);
            mpr(info);

            gain_piety(1);
            destroy_item(i);
            break;

        case GOD_ELYVILON:
            if (mitm[i].base_type != OBJ_WEAPONS
                && mitm[i].base_type != OBJ_MISSILES)
            {
                break;
            }

#ifdef JP
            it_name(i, DESC_PLAIN, str_pass);
#else
            it_name(i, DESC_CAP_THE, str_pass);
#endif
            strcpy(info, str_pass);
            strcat(info, sacrifice[you.religion - 1]);
            mpr(info);

            if (random2(value) >= random2(50)
                || (mitm[i].base_type == OBJ_WEAPONS
                    && (you.piety < 30 || player_under_penance())))
            {
                gain_piety(1);
            }

            destroy_item(i);
            break;

        default:
            break;
        }

        i = next;
    }
}                               // end altar_prayer()

void god_pitch(unsigned char which_god)
{
#ifdef JP
    strcpy(info, "당신은 ");
#else
    strcpy(info, "You kneel at the altar of ");
#endif
    strcat(info, god_name(which_god));
#ifdef JP
    strcat(info, "의 제단에 무릎꿇었다.");
#else
    strcat(info, ".");
#endif
    mpr(info);

    more();

    // Note: using worship we could make some gods not allow followers to
    // return, or not allow worshippers from other religions.  -- bwr

    if ((you.is_undead || you.species == SP_DEMONSPAWN)
        && (which_god == GOD_ZIN || which_god == GOD_SHINING_ONE
            || which_god == GOD_ELYVILON))
    {
#ifdef JP
        simple_god_message("은(는) 당신 같은 존재로부터의 숭배는 받아들이지 않는다!",
#else
        simple_god_message(" does not accept worship from those such as you!",
#endif
                           which_god);
        return;
    }

    describe_god( which_god, false );

#ifdef JP
    snprintf( info, INFO_SIZE, "당신은 이 종파를 %s신앙하는 것을 희망합니까?",
              (you.worshipped[which_god]) ? "다시 " : "" );
#else
    snprintf( info, INFO_SIZE, "Do you wish to %sjoin this religion?",
              (you.worshipped[which_god]) ? "re" : "" );
#endif

    if (!yesno( info ))
    {
        redraw_screen();
        return;
    }

#ifdef JP
    if (!yesno("신앙의 의사는 확실합니까?"))
#else
    if (!yesno("Are you sure?"))
#endif
    {
        redraw_screen();
        return;
    }

    redraw_screen();
    if (you.religion != GOD_NO_GOD)
        excommunication();

    you.religion = which_god;   //jmf: moved up so god_speaks gives right colour
    you.piety = 15;             // to prevent near instant excommunication
    you.gift_timeout = 0;
    set_god_ability_slots();    // remove old god's slots, reserve new god's

#ifdef JP
    snprintf( info, INFO_SIZE, "은 당신의 입교를 %s 환영했다!",
              (you.worshipped[which_god]) ? "다시" : "" );
#else
    snprintf( info, INFO_SIZE, " welcomes you%s!",
              (you.worshipped[which_god]) ? " back" : "" );
#endif

    simple_god_message( info );
    more();

    if (you.worshipped[you.religion] < 100)
        you.worshipped[you.religion]++;

    // Currently penance is just zeroed, this could be much more interesting.
    you.penance[you.religion] = 0;

    if (you.religion == GOD_KIKUBAAQUDGHA || you.religion == GOD_YREDELEMNUL
        || you.religion == GOD_VEHUMET || you.religion == GOD_MAKHLEB)
    {
        // Note:  Using worshipped[] we could make this sort of grudge
        // permanent instead of based off of penance. -- bwr
        if (you.penance[GOD_SHINING_ONE] > 0)
        {
            inc_penance(GOD_SHINING_ONE, 30);
#ifdef JP
            god_speaks(GOD_SHINING_ONE, "『너는 악업의 대가를 치르게 될 것이다!』");
#else
            god_speaks(GOD_SHINING_ONE, "\"You will pay for your evil ways, mortal!\"");
#endif
        }
    }
    redraw_skill( you.your_name, player_title() );
}                               // end god_pitch()

void offer_corpse(int corpse)
{
    char str_pass[ ITEMNAME_SIZE ];
#ifdef JP
    it_name(corpse, DESC_PLAIN, str_pass);
#else
    it_name(corpse, DESC_CAP_THE, str_pass);
#endif
    strcpy(info, str_pass);
    strcat(info, sacrifice[you.religion - 1]);
    mpr(info);

    done_good(GOOD_HACKED_CORPSE, 10);
}                               // end offer_corpse()

//jmf: moved stuff from items::handle_time()
void handle_god_time(void)
{
    if (one_chance_in(100))
    {
        // Choose a god randomly from those to whom we owe penance.
        //
        // Proof: (By induction)
        //
        // 1) n = 1, probability of choosing god is one_chance_in(1)
        // 2) Asuume true for n = k (ie. prob = 1 / n for all n)
        // 3) For n = k + 1,
        //
        //      P:new-found-god = 1 / n (see algorithm)
        //      P:other-gods = (1 - P:new-found-god) * P:god-at-n=k
        //                             1        1
        //                   = (1 - -------) * ---
        //                           k + 1      k
        //
        //                          k         1
        //                   = ----------- * ---
        //                        k + 1       k
        //
        //                       1       1
        //                   = -----  = ---
        //                     k + 1     n
        //
        // Therefore, by induction the probability is uniform.  As for
        // why we do it this way... it requires only one pass and doesn't
        // require an array.

        int which_god = GOD_NO_GOD;
        unsigned int count = 0;

        for (int i = GOD_NO_GOD; i < NUM_GODS; i++)
        {
            if (you.penance[i])
            {
                count++;
                if (one_chance_in(count))
                    which_god = i;
            }
        }

        if (which_god != GOD_NO_GOD)
            divine_retribution(which_god);
    }

    // Update the god's opinion of the player
    if (you.religion != GOD_NO_GOD)
    {
        switch (you.religion)
        {
        case GOD_XOM:
            if (one_chance_in(75))
                Xom_acts(true, you.experience_level + random2(15), true);
            break;

        case GOD_ZIN:           // These gods like long-standing worshippers
        case GOD_ELYVILON:
            if (you.piety < 150 && one_chance_in(20))
                gain_piety(1);
            break;

        case GOD_SHINING_ONE:
            if (you.piety < 150 && one_chance_in(15))
                gain_piety(1);
            break;

        case GOD_YREDELEMNUL:
        case GOD_KIKUBAAQUDGHA:
        case GOD_VEHUMET:
            if (one_chance_in(17))
                lose_piety(1);
            if (you.piety < 1)
                excommunication();
            break;

        case GOD_OKAWARU: // These gods accept corpses, so they time-out faster
        case GOD_TROG:
            if (one_chance_in(14))
                lose_piety(1);
            if (you.piety < 1)
                excommunication();
            break;

        case GOD_MAKHLEB:
            if (one_chance_in(16))
                lose_piety(1);
            if (you.piety < 1)
                excommunication();
            break;

        case GOD_SIF_MUNA:
            if (one_chance_in(20))
                lose_piety(1);
            if (you.piety < 1)
                excommunication();
            break;

        case GOD_NEMELEX_XOBEH: // relatively patient
            if (one_chance_in(35))
                lose_piety(1);
            if (you.attribute[ATTR_CARD_COUNTDOWN] > 0 && coinflip())
                you.attribute[ATTR_CARD_COUNTDOWN]--;
            if (you.piety < 1)
                excommunication();
            break;

        default:
#ifdef JP
            DEBUGSTR("형편없는 신이다. 주교님도 없잖아!");
#else
            DEBUGSTR("Bad god, no bishop!");
#endif
        }
    }
}                               // end handle_god_time()

// yet another wrapper for mpr() {dlb}:
void simple_god_message(const char *event, int which_deity)
{
    char buff[ INFO_SIZE ];

    if (which_deity == GOD_NO_GOD)
        which_deity = you.religion;

    snprintf( buff, sizeof(buff), "%s%s", god_name( which_deity ), event );

    god_speaks( which_deity, buff );
}

char god_colour( char god ) //mv - added
{
    switch (god)
    {
    case GOD_SHINING_ONE:
    case GOD_ZIN:
    case GOD_ELYVILON:
    case GOD_OKAWARU:
        return(CYAN);

    case GOD_YREDELEMNUL:
    case GOD_KIKUBAAQUDGHA:
    case GOD_MAKHLEB:
    case GOD_VEHUMET:
    case GOD_TROG:
        return(LIGHTRED);

    case GOD_XOM:
        return(YELLOW);

    case GOD_NEMELEX_XOBEH:
        return(LIGHTMAGENTA);

    case GOD_SIF_MUNA:
        return(LIGHTBLUE);

    case GOD_NO_GOD:
    default:
        break;
    }

    return(YELLOW);
}
