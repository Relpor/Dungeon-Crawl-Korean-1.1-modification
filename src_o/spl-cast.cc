/*
 *  File:       spl-cast.cc
 *  Summary:    Spell casting and miscast functions.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <4>      1/02/00        jmf             changed values, marked //jmf:
 *      <3>      6/13/99        BWR             Added Staff auto identify code
 *      <2>      5/20/99        BWR             Added some screen redraws
 *      <1>      -/--/--        LRH             Created
 */

#include "AppHdr.h"
#include "spl-cast.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "externs.h"

#include "beam.h"
#include "cloud.h"
#include "effects.h"
#include "fight.h"
#include "food.h"
#include "it_use2.h"
#include "itemname.h"
#include "macro.h"
#include "monplace.h"
#include "monstuff.h"
#include "mutation.h"
#include "ouch.h"
#include "player.h"
#include "religion.h"
#include "skills.h"
#include "spells1.h"
#include "spells2.h"
#include "spells3.h"
#include "spells4.h"
#include "spl-book.h"
#include "spl-util.h"
#include "stuff.h"
#include "transfor.h"
#include "view.h"

#ifdef DOS
#include <conio.h>
#endif

#define WILD_MAGIC_NASTINESS 150

static void surge_power(int spell)
{
    int enhanced = 0;

    //jmf: simplified
    enhanced += spell_enhancement(spell_type(spell));

    if (enhanced)               // one way or the other {dlb}
    {
#ifdef JP
        strcpy(info, "당신은");
#else
        strcpy(info, "You feel a");
#endif

#ifdef JP
        strcat(info, (enhanced < -2)  ? " 비정상적일 정도의" :
                     (enhanced == -2) ? " 막대한" :
                     (enhanced == 2)  ? " 강한" :
                     (enhanced > 2)   ? " 끝없는"
                                      : "");
#else
        strcat(info, (enhanced < -2)  ? "n extraordinarily" :
                     (enhanced == -2) ? "n extremely" :
                     (enhanced == 2)  ? " strong" :
                     (enhanced > 2)   ? " huge"
                                      : "");
#endif

#ifdef JP
        strcat(info, (enhanced < 0) ? " 무력감을 느꼈다."
                                    : " 마력의 증폭을 느꼈다!");
#else
        strcat(info, (enhanced < 0) ? " numb sensation."
                                    : " surge of power!");
#endif
        mpr(info);
    }
}                               // end surge_power()

char list_spells(void)
{
    int j;
    int lines = 0;
    unsigned int anything = 0;
    unsigned int i;
    int ki;
    bool already = false;

    const int num_lines = get_number_of_lines();

#ifdef DOS_TERM
    char buffer[4800];

    gettext(1, 1, 80, 25, buffer);
#endif

#ifdef DOS_TERM
    window(1, 1, 80, 25);
#endif

    clrscr();

#ifdef JP
    cprintf( " 주문                             계통                          성공율    레벨" );
#else
    cprintf( " Your Spells                      Type                          Success   Level" );
#endif
    lines++;

    for (j = 0; j < 52; j++)
    {
        if (lines > num_lines - 2)
        {
            gotoxy(1, num_lines);
#ifdef JP
            cprintf("-계속-");
#else
            cprintf("-more-");
#endif

            ki = getch();

            if (ki == ESCAPE)
            {
#ifdef DOS_TERM
                puttext(1, 1, 80, 25, buffer);
#endif
                return (ESCAPE);
            }

            if (isalpha( ki ))
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

        const char letter = index_to_letter(j);
        const int  spell  = get_spell_by_letter(letter);

        if (spell != SPELL_NO_SPELL)
        {
            anything++;

            if (lines > 0)
                cprintf(EOL);

            lines++;

            cprintf( " %c - %s", letter, spell_title( spell ) );
            gotoxy(35, wherey());

            already = false;

            for (i = 0; i <= SPTYP_LAST_EXPONENT; i++)
            {
                if (spell_typematch( spell, (1 << i) ))
                {
                    if (already)
                        cprintf( "/" );

                    cprintf( spelltype_name( 1 << i ) );
                    already = true;
                }
            }

            char sval[16];

            //gotoxy(58, wherey());
            gotoxy(65, wherey());

            int spell_f = spell_fail( spell );

#ifdef JP
            cprintf( (spell_f == 100) ? "불가능"    :
                     (spell_f >   90) ? "나쁨(-2)":
                     (spell_f >   80) ? "나쁨(-1)":
                     (spell_f >   70) ? "나쁨(+0)"      :
                     (spell_f >   60) ? "보통(-1)"      :
                     (spell_f >   50) ? "보통(+0)"  :
                     (spell_f >   40) ? "보통(+1)"  :
                     (spell_f >   30) ? "좋음(+0)"      :
                     (spell_f >   20) ? "좋음(+1)":
                     (spell_f >   10) ? "좋음(+2)":
                     (spell_f >    0) ? "좋음(+3)"
                                      : "완벽함" );
#else
            cprintf( (spell_f == 100) ? "Useless"   :
                     (spell_f >   90) ? "Terrible"  :
                     (spell_f >   80) ? "Cruddy"    :
                     (spell_f >   70) ? "Bad"       :
                     (spell_f >   60) ? "Very Poor" :
                     (spell_f >   50) ? "Poor"      :
                     (spell_f >   40) ? "Fair"      :
                     (spell_f >   30) ? "Good"      :
                     (spell_f >   20) ? "Very Good" :
                     (spell_f >   10) ? "Great"     :
                     (spell_f >    0) ? "Excellent"
                                      : "Perfect" );
#endif

            gotoxy(77, wherey());

            itoa( (int) spell_difficulty( spell ), sval, 10 );
            cprintf(sval);
        }
    }                           // end of j loop

    if (anything > 0)
    {
        ki = getch();

        if (ki >= 'A' && ki <= 'z')
        {
#ifdef DOS_TERM
            puttext(1, 1, 80, 25, buffer);
#endif
            return (ki);
        }

        if (ki == 0)
            ki = getch();

#ifdef DOS_TERM
        puttext(1, 1, 80, 25, buffer);
#endif

        return (anything);
    }

#ifdef DOS_TERM
    puttext(1, 1, 80, 25, buffer);
#endif
    // was 35
    ki = getch();

    return (ki);
}                               // end list_spells()

int spell_fail(int spell)
{
    int chance = 60;
    int chance2 = 0, armour = 0;

    chance -= 6 * calc_spell_power( spell, false );
    chance -= (you.intel * 2);

    //chance -= (you.intel - 10) * abs(you.intel - 10);
    //chance += spell_difficulty(spell) * spell_difficulty(spell) * 3; //spell_difficulty(spell);

    if (you.equip[EQ_BODY_ARMOUR] != -1)
    {

        int ev_penalty = abs(property( you.inv[you.equip[EQ_BODY_ARMOUR]],
                                       PARM_EVASION ));

        // The minus 15 is to make the -1 light armours not so bad
        armour += (20 * ev_penalty) - 15;

        //jmf: armour skill now reduces failure due to armour
        //bwr: this was far too good, an armour skill of 5 was
        //     completely negating plate mail.  Plate mail should
        //     hardly be completely negated, it should still be
        //     an important consideration for even high level characters.
        //     Truth is, even a much worse penalty than the above can
        //     easily be overcome by gaining spell skills... and a lot
        //     faster than any reasonable rate of bonus here.
        int lim_str = (you.strength > 30) ? 30 :
                      (you.strength < 10) ? 10 : you.strength;

        armour -= ((you.skills[SK_ARMOUR] * lim_str) / 15);

        int race_arm = get_equip_race( you.inv[you.equip[EQ_BODY_ARMOUR]] );
        int racial_type = 0;

        if (player_genus(GENPC_DWARVEN))
            racial_type = ISFLAG_DWARVEN;
        else if (player_genus(GENPC_ELVEN))
            racial_type = ISFLAG_ELVEN;
        else if (you.species == SP_HILL_ORC)
            racial_type = ISFLAG_ORCISH;

        // Elven armour gives everyone some benefit to spellcasting,
        // Dwarven armour hinders everyone.
        switch (race_arm)
        {
        case ISFLAG_ELVEN:
            armour -= 20;
            break;
        case ISFLAG_DWARVEN:
            armour += 10;
            break;
        default:
            break;
        }

        // Armour of the same racial type reduces penalty.
        if (racial_type && race_arm == racial_type)
            armour -= 10;

        if (armour > 0)
            chance += armour;
    }

    if (you.equip[EQ_WEAPON] != -1
            && you.inv[you.equip[EQ_WEAPON]].base_type == OBJ_WEAPONS)
    {
        int wpn_penalty = (3 * (property( you.inv[you.equip[EQ_WEAPON]], PWPN_SPEED ) - 12)) / 2;

        if (wpn_penalty > 0)
            chance += wpn_penalty;
    }

    if (you.equip[EQ_SHIELD] != -1)
    {
        switch (you.inv[you.equip[EQ_SHIELD]].sub_type)
        {
        case ARM_BUCKLER:
            chance += 5;
            break;

        case ARM_SHIELD:
            chance += 15;
            break;

        case ARM_LARGE_SHIELD:
            // *BCR* Large chars now get a lower penalty for large shields
            if ((you.species >= SP_OGRE && you.species <= SP_OGRE_MAGE)
                || player_genus(GENPC_DRACONIAN))
            {
                chance += 20;
            }
            else
                chance += 30;
            break;
        }
    }

    switch (spell_difficulty(spell))
    {
    case  1: chance +=   3; break;
    case  2: chance +=  15; break;
    case  3: chance +=  35; break;
    case  4: chance +=  70; break;
    case  5: chance += 100; break;
    case  6: chance += 150; break;
    case  7: chance += 200; break;
    case  8: chance += 260; break;
    case  9: chance += 330; break;
    case 10: chance += 420; break;
    case 11: chance += 500; break;
    case 12: chance += 600; break;
    default: chance += 750; break;
    }

    //if (chance < 1 ) chance = 0;
    if (chance > 100)
        chance = 100;

    chance2 = chance;

    if (chance < 45)
        chance2 = 45;
    if (chance < 42)
        chance2 = 43;
    if (chance < 38)
        chance2 = 41;
    if (chance < 35)
        chance2 = 40;
    if (chance < 32)
        chance2 = 38;
    if (chance < 28)
        chance2 = 36;
    if (chance < 22)
        chance2 = 34;
    if (chance < 16)
        chance2 = 32;
    if (chance < 10)
        chance2 = 30;
    if (chance < 2)
        chance2 = 28;
    if (chance < -7)
        chance2 = 26;
    if (chance < -12)
        chance2 = 24;
    if (chance < -18)
        chance2 = 22;
    if (chance < -24)
        chance2 = 20;
    if (chance < -30)
        chance2 = 18;
    if (chance < -38)
        chance2 = 16;
    if (chance < -46)
        chance2 = 14;
    if (chance < -60)
        chance2 = 12;
    if (chance < -80)
        chance2 = 10;
    if (chance < -100)
        chance2 = 8;
    if (chance < -120)
        chance2 = 6;
    if (chance < -140)
        chance2 = 4;
    if (chance < -160)
        chance2 = 2;
    if (chance < -180)
        chance2 = 0;

    if (you.religion == GOD_VEHUMET
        && you.duration[DUR_PRAYER]
        && (!player_under_penance() && you.piety >= 50)
        && (spell_typematch(spell, SPTYP_CONJURATION)
            || spell_typematch(spell, SPTYP_SUMMONING)))
    {
        chance2 /= 2;
    }

    if (you.duration[DUR_TRANSFORMATION] > 0)
    {
        switch (you.attribute[ATTR_TRANSFORMATION])
        {
        case TRAN_BLADE_HANDS:
            chance2 += 20;
            break;

        case TRAN_SPIDER:
            chance2 += 10;
            break;
        }
    }

    return (chance2);
}                               // end spell_fail()


int calc_spell_power( int spell, bool apply_intel )
{
    unsigned int bit;
    int ndx;
    int power = (you.skills[SK_SPELLCASTING] / 2) + player_mag_abil(false);
    int enhanced = 0;

    unsigned int disciplines = spell_type( spell );

    //jmf: evil evil evil -- exclude HOLY bit
    disciplines &= (~SPTYP_HOLY);

    int skillcount = count_bits( disciplines );
    if (skillcount)
    {
        for (ndx = 0; ndx <= SPTYP_LAST_EXPONENT; ndx++)
        {
            bit = 1 << ndx;
            if ((bit != SPTYP_HOLY) && (disciplines & bit))
            {
                int skill = spell_type2skill( bit );

                power += (you.skills[skill] * 2) / skillcount;
            }
        }
    }

    if (apply_intel)
        power = (power * you.intel) / 10;

    enhanced = spell_enhancement( disciplines );

    if (enhanced > 0)
    {
        for (ndx = 0; ndx < enhanced; ndx++)
        {
            power *= 15;
            power /= 10;
        }
    }
    else if (enhanced < 0)
    {
        for (ndx = enhanced; ndx < 0; ndx++)
            power /= 2;
    }

    power = stepdown_value( power, 50, 50, 150, 200 );

    return (power);
}                               // end calc_spell_power()


int spell_enhancement( unsigned int typeflags )
{
    int enhanced = 0;

    if (typeflags & SPTYP_CONJURATION)
        enhanced += player_spec_conj();

    if (typeflags & SPTYP_ENCHANTMENT)
        enhanced += player_spec_ench();

    if (typeflags & SPTYP_SUMMONING)
        enhanced += player_spec_summ();

    if (typeflags & SPTYP_POISON)
        enhanced += player_spec_poison();

    if (typeflags & SPTYP_NECROMANCY)
        enhanced += player_spec_death() - player_spec_holy();

    if (typeflags & SPTYP_FIRE)
        enhanced += player_spec_fire() - player_spec_cold();

    if (typeflags & SPTYP_ICE)
        enhanced += player_spec_cold() - player_spec_fire();

    if (typeflags & SPTYP_EARTH)
        enhanced += player_spec_earth() - player_spec_air();

    if (typeflags & SPTYP_AIR)
        enhanced += player_spec_air() - player_spec_earth();

    if (you.special_wield == SPWLD_SHADOW)
        enhanced -= 2;

    // These are used in an exponential way, so we'll limit them a bit. -- bwr
    if (enhanced > 3)
        enhanced = 3;
    else if (enhanced < -3)
        enhanced = -3;

    return (enhanced);
}                               // end spell_enhancement()

// returns false if spell failed, and true otherwise
bool cast_a_spell(void)
{
    char spc = 0;
    char spc2 = 0;
    int spellh = 0;
    unsigned char keyin = 0;
    char unthing = 0;

    if (!you.spell_no)
    {
#ifdef JP
        mpr("당신은 어떤 주문도 알지 못한다.");
#else
        mpr("You don't know any spells.");
#endif
        return (false);
    }

    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return (false);
    }

    if (silenced(you.x_pos, you.y_pos))
    {
#ifdef JP
        mpr("당신은 침묵 상태에서 주문을 시전할 수 없다!");
#else
        mpr("You cannot cast spells when silenced!");
#endif
        return (false);
    }

    // first query {dlb}:
    for (;;)
    {
        //jmf: FIXME: change to reflect range of known spells
#ifdef JP
        mpr( "어떤 주문을 시전합니까? ([*][?]일람) ", MSGCH_PROMPT );
#else
        mpr( "Cast which spell ([?*] list)? ", MSGCH_PROMPT );
#endif

        keyin = get_ch();

        if (keyin == '?' || keyin == '*')
        {
            unthing = list_spells();

            redraw_screen();
            if (unthing == 2)
            {
                canned_msg(MSG_OK);
                return (false);
            }

            if ( (unthing >= 'a' && unthing <= 'z')
               ||(unthing >= 'A' && unthing <= 'Z') )
            {
                keyin = unthing;
                break;
            }
            else
                mesclr();
        }
        else
        {
            break;
        }

    }

    if (keyin == ESCAPE)
    {
        canned_msg(MSG_OK);
        return (false);
    }

    spc = (int) keyin;

    if (!isalpha(spc))
    {
#ifdef JP
        mpr("당신은 그 주문을 모른다.");
#else
        mpr("You don't know that spell.");
#endif
        return (false);
    }

    const int spell = get_spell_by_letter( spc );

    spc2 = letter_to_index(spc);

    if (spell == SPELL_NO_SPELL)
    {
#ifdef JP
        mpr("당신은 그 주문을 모른다.");
#else
        mpr("You don't know that spell.");
#endif
        return (false);
    }

    if (spell_mana( spell ) > you.magic_points)
    {
#ifdef JP
        mpr("당신은 그 주문을 시전하기 위한 충분한 마나를 가지고 있지 않다.");
#else
        mpr("You don't have enough magic to cast that spell.");
#endif
        return (false);
    }

    if (you.is_undead != US_UNDEAD
        && (you.hunger_state < HS_HUNGRY
            || you.hunger <= spell_hunger( spell )))
    {
#ifdef JP
        mpr("당신에게는 주문을 시전하기 위한 에너지가 부족하다.");
#else
        mpr("You don't have the energy to cast that spell.");
#endif
        return (false);
    }

    dec_mp( spell_mana(spell) );

    if (!player_energy() && you.is_undead != US_UNDEAD)
    {
        spellh = spell_hunger( spell );

        // I wonder if a better algorithm is called for? {dlb}
        spellh -= you.intel * you.skills[SK_SPELLCASTING];

        if (spellh < 1)
            spellh = 1;
        else
            make_hungry(spellh, true);
    }

    you.turn_is_over = 1;
    alert_nearby_monsters();

    if (you.conf)
        random_uselessness( 2 + random2(7), 0 );
    else
    {
        exercise_spell( spell, true, your_spells( spell ) );
        naughty( NAUGHTY_SPELLCASTING, 1 + random2(5) );
    }

    return (true);
}                               // end cast_a_spell()

// returns true if spell if spell is successfully cast for purposes of
// exercising and false otherwise (note: false == less exercise, not none).
bool your_spells( int spc2, int powc, bool allow_fail )
{
    int dem_hor = 0;
    int dem_hor2 = 0;
    int total_skill = 0;
    struct dist spd;
    struct bolt beam;

    alert_nearby_monsters();

    // Added this so that the passed in powc can have meaning -- bwr
    if (powc == 0)
        powc = calc_spell_power( spc2, true );

    if (you.equip[EQ_WEAPON] != -1
        && item_is_staff( you.inv[you.equip[EQ_WEAPON]] )
        && item_not_ident( you.inv[you.equip[EQ_WEAPON]], ISFLAG_KNOW_TYPE ))
    {
        switch (you.inv[you.equip[EQ_WEAPON]].sub_type)
        {
        case STAFF_ENERGY:
        case STAFF_WIZARDRY:
            total_skill = you.skills[SK_SPELLCASTING];
            break;
        case STAFF_FIRE:
            if (spell_typematch(spc2, SPTYP_FIRE))
                total_skill = you.skills[SK_FIRE_MAGIC];
            else if (spell_typematch(spc2, SPTYP_ICE))
                total_skill = you.skills[SK_ICE_MAGIC];
            break;
        case STAFF_COLD:
            if (spell_typematch(spc2, SPTYP_ICE))
                total_skill = you.skills[SK_ICE_MAGIC];
            else if (spell_typematch(spc2, SPTYP_FIRE))
                total_skill = you.skills[SK_FIRE_MAGIC];
            break;
        case STAFF_AIR:
            if (spell_typematch(spc2, SPTYP_AIR))
                total_skill = you.skills[SK_AIR_MAGIC];
            else if (spell_typematch(spc2, SPTYP_EARTH))
                total_skill = you.skills[SK_EARTH_MAGIC];
            break;
        case STAFF_EARTH:
            if (spell_typematch(spc2, SPTYP_EARTH))
                total_skill = you.skills[SK_EARTH_MAGIC];
            else if (spell_typematch(spc2, SPTYP_AIR))
                total_skill = you.skills[SK_AIR_MAGIC];
            break;
        case STAFF_POISON:
            if (spell_typematch(spc2, SPTYP_POISON))
                total_skill = you.skills[SK_POISON_MAGIC];
            break;
        case STAFF_DEATH:
            if (spell_typematch(spc2, SPTYP_NECROMANCY))
                total_skill = you.skills[SK_NECROMANCY];
            break;
        case STAFF_CONJURATION:
            if (spell_typematch(spc2, SPTYP_CONJURATION))
                total_skill = you.skills[SK_CONJURATIONS];
            break;
        case STAFF_ENCHANTMENT:
            if (spell_typematch(spc2, SPTYP_ENCHANTMENT))
                total_skill = you.skills[SK_ENCHANTMENTS];
            break;
        case STAFF_SUMMONING:
            if (spell_typematch(spc2, SPTYP_SUMMONING))
                total_skill = you.skills[SK_SUMMONINGS];
            break;
        }

        if (you.skills[SK_SPELLCASTING] > total_skill)
            total_skill = you.skills[SK_SPELLCASTING];

        if (random2(100) < total_skill)
        {
            char str_pass[ ITEMNAME_SIZE ];

            set_ident_flags( you.inv[you.equip[EQ_WEAPON]], ISFLAG_KNOW_TYPE );

#ifdef JP
            strcpy(info, "당신은 ");
#else
            strcpy(info, "You are wielding ");
#endif
            in_name(you.equip[EQ_WEAPON], DESC_NOCAP_A, str_pass);
            strcat(info, str_pass);
#ifdef JP
            strcat(info, "을(를) 손에 쥐고 있다.");
#else
            strcat(info, ".");
#endif
            mpr(info);

            more();

            you.wield_change = true;
        }
    }

    surge_power(spc2);

    if (allow_fail)
    {
        int spfl = random2avg(100, 3);

        if (you.religion != GOD_SIF_MUNA
            && you.penance[GOD_SIF_MUNA] && one_chance_in(40))
        {
#ifdef JP
            god_speaks(GOD_SIF_MUNA, "당신은 신의 악의가 밀려오는 것을 느꼈다.");
#else
            god_speaks(GOD_SIF_MUNA, "You feel a surge of divine spite.");
#endif

            // This will cause failure and increase the miscast effect.
            spfl = -you.penance[GOD_SIF_MUNA];

            // Reduced penenance reduction here because casting spells
            // is a player controllable act.  -- bwr
            if (one_chance_in(7))
                dec_penance(GOD_SIF_MUNA, 1);
        }

        if (spfl < spell_fail(spc2))
        {
#ifdef JP
            mpr( "당신은 주문 시전에 실패했다." );
#else
            mpr( "You miscast the spell." );
#endif
            flush_input_buffer( FLUSH_ON_FAILURE );

            if (you.religion == GOD_SIF_MUNA
                && (!player_under_penance()
                    && you.piety >= 100 && random2(150) <= you.piety))
            {
                canned_msg(MSG_NOTHING_HAPPENS);
                return (false);
            }

            unsigned int sptype = 0;

            do
            {
                sptype = 1 << (random2(SPTYP_LAST_EXPONENT+1));
            } while (!spell_typematch(spc2, sptype));

            // all spell failures give a bit of magical radiation..
            // failure is a function of power squared multiplied
            // by how badly you missed the spell.  High power
            // spells can be quite nasty: 9 * 9 * 90 / 500 = 15
            // points of contamination!
            int nastiness = spell_mana(spc2) * spell_mana(spc2)
                                    * (spell_fail(spc2) - spfl) + 250;

            int cont_points = nastiness / 500;
            // handle fraction
            if (random2(500) < (nastiness % 500))
                cont_points++;

            contaminate_player( cont_points );

            miscast_effect( sptype, spell_mana(spc2), spell_fail(spc2) - spfl, 100 );

            if (you.religion == GOD_XOM && random2(75) < spell_mana(spc2))
                Xom_acts(coinflip(), spell_mana(spc2), false);

            return (false);
        }
    }

    if (you.is_undead && spell_typematch( spc2, SPTYP_HOLY ))
    {
#ifdef JP
        mpr( "당신은 이 계통의 마법을 쓸 수 없다!" );
#else
        mpr( "You can't use this type of magic!" );
#endif
        return (false);         // XXX: still gets trained!
    }

    // Normally undead can't memorize these spells, so this check is
    // to catch those in Lich form.  As such, we allow the Lich form
    // to be extended here. -- bwr
    if (spc2 != SPELL_NECROMUTATION
        && undead_cannot_memorise( spc2, you.is_undead ))
    {
#ifdef JP
        mpr( "당신은 현재 형태에서 그 주문을 시전할 수 없다!" );
#else
        mpr( "You cannot cast that spell in your current form!" );
#endif
        return (false);         // XXX: still gets trained!
    }

    // Linley says: Condensation Shield needs some disadvantages to keep
    // it from being a no-brainer... this isn't much, but its a start -- bwr
    if (you.duration[DUR_CONDENSATION_SHIELD] > 0
        && spell_typematch( spc2, SPTYP_FIRE ))
    {
#ifdef JP
        mpr( "당신의 얼음의 방패는 흩뜨려져 버렸다!", MSGCH_DURATION );
#else
        mpr( "Your icy shield dissipates!", MSGCH_DURATION );
#endif
        you.duration[DUR_CONDENSATION_SHIELD] = 0;
        you.redraw_armour_class = 1;
    }

    if (spc2 == SPELL_SUMMON_HORRIBLE_THINGS
        || spc2 == SPELL_CALL_IMP
        || spc2 == SPELL_SUMMON_DEMON
        || spc2 == SPELL_DEMONIC_HORDE
        || spc2 == SPELL_SUMMON_GREATER_DEMON || spc2 == SPELL_HELLFIRE)
    {
        naughty(NAUGHTY_UNHOLY, 10 + spell_difficulty(spc2));
    }

    if (spell_typematch( spc2, SPTYP_NECROMANCY ))
        naughty( NAUGHTY_NECROMANCY, 10 + spell_difficulty(spc2) );

    if (spc2 == SPELL_NECROMUTATION
        && (you.religion == GOD_ELYVILON
            || you.religion == GOD_SHINING_ONE
            || you.religion == GOD_ZIN))
    {
        excommunication();
    }

    if (you.religion == GOD_SIF_MUNA
        && you.piety < 200 && random2(12) <= spell_difficulty(spc2))
    {
        gain_piety(1);
    }

#if DEBUG_DIAGNOSTICS
#ifdef JP
    snprintf( info, INFO_SIZE, "Spell #%d, power=%d", spc2, powc );
#else
    snprintf( info, INFO_SIZE, "Spell #%d, power=%d", spc2, powc );
#endif
    mpr( info, MSGCH_DIAGNOSTICS );
#endif

    switch (spc2)
    {
    case SPELL_IDENTIFY:
        identify(powc);
        break;

    case SPELL_TELEPORT_SELF:
        you_teleport();
        break;

    case SPELL_CAUSE_FEAR:
        mass_enchantment(ENCH_FEAR, powc, MHITYOU);
        break;

    case SPELL_CREATE_NOISE:  // unused, the player can shout to do this - bwr
        if (!silenced(you.x_pos, you.y_pos))
        {
#ifdef JP
            mpr("당신의 이름을 부르는 목소리가 들려왔다!");
#else
            mpr("You hear a voice call your name!");
#endif
            noisy( 25, you.x_pos, you.y_pos );
        }
        break;

    case SPELL_REMOVE_CURSE:
        remove_curse(false);
        break;

    case SPELL_MAGIC_DART:
        if (spell_direction(spd, beam) == -1)
            return (false);

        zapping(ZAP_MAGIC_DARTS, powc, beam);
        break;

    case SPELL_FIREBALL:
        fireball(powc);
        break;

    case SPELL_DELAYED_FIREBALL:
        // This spell has two main advantages over Fireball:
        //
        // (1) The release is instantaneous, so monsters will not
        //     get an action before the player... this allows the
        //     player to hit monsters with a double fireball (this
        //     is why we only allow one delayed fireball at a time,
        //     if you want to allow for more, then the release should
        //     take at least some amount of time).
        //
        //     The casting of this spell still costs a turn.  So
        //     casting Delayed Fireball and immediately releasing
        //     the fireball is only slightly different than casting
        //     a regular Fireball (monsters act in the middle instead
        //     of at the end).  This is why we allow for the spell
        //     level discount so that Fireball is free with this spell
        //     (so that it only costs 7 levels instead of 13 to have
        //     both).
        //
        // (2) When the fireball is released, it is guaranteed to
        //     go off... the spell only fails at this point.  This can
        //     be a large advantage for characters who have difficulty
        //     casting Fireball in their standard equipment.  However,
        //     the power level for the actual fireball is determined at
        //     release, so if you do swap out your enhancers you'll
        //     get a less powerful ball when its released. -- bwr
        //
        if (!you.attribute[ ATTR_DELAYED_FIREBALL ])
        {
            // okay, this message is weak but functional -- bwr
#ifdef JP
            mpr( "당신은 마력이 장전된 것을 느꼈다." );
#else
            mpr( "You feel magically charged." );
#endif
            you.attribute[ ATTR_DELAYED_FIREBALL ] = 1;
        }
        else
        {
            canned_msg( MSG_NOTHING_HAPPENS );
        }
        break;

    case SPELL_STRIKING:
        if (spell_direction( spd, beam ) == -1)
            return (false);

        zapping( ZAP_STRIKING, powc, beam );
        break;

    case SPELL_CONJURE_FLAME:
        conjure_flame(powc);
        break;

    case SPELL_DIG:
        if (spell_direction(spd, beam) == -1)
            return (false);

        if (spd.isMe)
        {
            canned_msg(MSG_UNTHINKING_ACT);
            return (false);
        }
        zapping(ZAP_DIGGING, powc, beam);
        break;

    case SPELL_BOLT_OF_FIRE:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_FIRE, powc, beam);
        break;

    case SPELL_BOLT_OF_COLD:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_COLD, powc, beam);
        break;

    case SPELL_LIGHTNING_BOLT:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_LIGHTNING, powc, beam);
        break;

    case SPELL_BOLT_OF_MAGMA:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_MAGMA, powc, beam);
        break;

    case SPELL_POLYMORPH_OTHER:
        if (spell_direction(spd, beam) == -1)
            return (false);

        if (spd.isMe)
        {
#ifdef JP
            mpr("유감이지만, 그것은 그렇게 사용할 수 없다.");
#else
            mpr("Sorry, it doesn't work like that.");
#endif
            return (false);
        }
        zapping(ZAP_POLYMORPH_OTHER, powc, beam);
        break;

    case SPELL_SLOW:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_SLOWING, powc, beam);
        break;

    case SPELL_HASTE:
        if (spell_direction(spd, beam, DIR_NONE, TARG_FRIEND) == -1)
            return (false);
        zapping(ZAP_HASTING, powc, beam);
        break;

    case SPELL_PARALYZE:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_PARALYSIS, powc, beam);
        break;

    case SPELL_CONFUSE:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_CONFUSION, powc, beam);
        break;

    case SPELL_CONFUSING_TOUCH:
        cast_confusing_touch(powc);
        break;

    case SPELL_SURE_BLADE:
        cast_sure_blade(powc);
        break;

    case SPELL_INVISIBILITY:
        if (spell_direction(spd, beam, DIR_NONE, TARG_FRIEND) == -1)
            return (false);
        zapping(ZAP_INVISIBILITY, powc, beam);
        break;

    case SPELL_THROW_FLAME:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_FLAME, powc, beam);
        break;

    case SPELL_THROW_FROST:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_FROST, powc, beam);
        break;

    case SPELL_CONTROLLED_BLINK:
        blink();
        break;

    case SPELL_FREEZING_CLOUD:
        cast_big_c(powc, CLOUD_COLD);
        break;

    case SPELL_MEPHITIC_CLOUD:
        stinking_cloud(powc);
        break;

    case SPELL_RING_OF_FLAMES:
        cast_ring_of_flames(powc);
        break;

    case SPELL_RESTORE_STRENGTH:
        restore_stat(STAT_STRENGTH, false);
        break;

    case SPELL_RESTORE_INTELLIGENCE:
        restore_stat(STAT_INTELLIGENCE, false);
        break;

    case SPELL_RESTORE_DEXTERITY:
        restore_stat(STAT_DEXTERITY, false);
        break;

    case SPELL_VENOM_BOLT:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_VENOM_BOLT, powc, beam);
        break;

    case SPELL_OLGREBS_TOXIC_RADIANCE:
        cast_toxic_radiance();
        break;

    case SPELL_TELEPORT_OTHER:
        if (spell_direction(spd, beam) == -1)
            return (false);

        if (spd.isMe)
        {
#ifdef JP
            mpr("유감이지만, 그것은 그렇게 사용할 수 없다.");
#else
            mpr("Sorry, it doesn't work like that.");
#endif
            return (false);
        }
        // teleport creature (I think)
        zapping(ZAP_TELEPORTATION, powc, beam);
        break;

    case SPELL_LESSER_HEALING:
        cast_healing(5);
        break;

    case SPELL_GREATER_HEALING:
        cast_healing(25);
        break;

    case SPELL_CURE_POISON_I:   //jmf: `healing' version? group w/ S_C_P_II?
        cast_cure_poison(powc);
        break;

    case SPELL_PURIFICATION:
        purification();
        break;

    case SPELL_DEATHS_DOOR:
        cast_deaths_door(powc);
        break;

    case SPELL_SELECTIVE_AMNESIA:
        cast_selective_amnesia(false);
        break;                  //     Sif Muna power calls with true

    case SPELL_MASS_CONFUSION:
        mass_enchantment(ENCH_CONFUSION, powc, MHITYOU);
        break;

    case SPELL_SMITING:
        cast_smiting(powc);
        break;

    case SPELL_REPEL_UNDEAD:
        turn_undead(50);
        break;

    case SPELL_HOLY_WORD:
        holy_word(50);
        break;

    case SPELL_DETECT_CURSE:
        detect_curse(false);
        break;

    case SPELL_SUMMON_SMALL_MAMMAL:
        summon_small_mammals(powc); //jmf: hmm, that's definately *plural* ;-)
        break;

    case SPELL_ABJURATION_I:    //jmf: why not group with SPELL_ABJURATION_II?
        abjuration(powc);
        break;

    case SPELL_SUMMON_SCORPIONS:
        summon_scorpions(powc);
        break;

    case SPELL_LEVITATION:
        potion_effect( POT_LEVITATION, powc );
        break;

    case SPELL_BOLT_OF_DRAINING:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_NEGATIVE_ENERGY, powc, beam);
        break;

    case SPELL_LEHUDIBS_CRYSTAL_SPEAR:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_CRYSTAL_SPEAR, powc, beam);
        break;

    case SPELL_BOLT_OF_INACCURACY:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_BEAM_OF_ENERGY, powc, beam);
        break;

    case SPELL_POISONOUS_CLOUD:
        cast_big_c(powc, CLOUD_POISON);
        break;

    case SPELL_POISON_ARROW:
        if (spell_direction(spd, beam) == -1)
            return (false);

        zapping( ZAP_POISON_ARROW, powc, beam );
        break;

    case SPELL_FIRE_STORM:
        cast_fire_storm(powc);
        break;

    case SPELL_DETECT_TRAPS:
#ifdef JP
        strcpy(info, "당신은 ");
        strcat(info, (detect_traps(powc) > 0) ? "함정을 탐지했다!" : "아무 것도 발견하지 못했다.");
#else
        strcpy(info, "You detect ");
        strcat(info, (detect_traps(powc) > 0) ? "some traps!" : "nothing.");
#endif
        mpr(info);
        break;

    case SPELL_BLINK:
        random_blink(true);
        break;

    case SPELL_ISKENDERUNS_MYSTIC_BLAST:
        if (spell_direction(spd, beam) == -1)
            return (false);

        zapping( ZAP_MYSTIC_BLAST, powc, beam );
        break;

    case SPELL_SWARM:
        summon_swarm( powc, false, false );
        break;

    case SPELL_SUMMON_HORRIBLE_THINGS:
        summon_things(powc);
        break;

    case SPELL_ENSLAVEMENT:
        if (spell_direction(spd, beam) == -1)
            return (false);
        if (spd.isMe)
        {
            canned_msg(MSG_UNTHINKING_ACT);
            return (false);
        }
        zapping(ZAP_ENSLAVEMENT, powc, beam);
        break;

    case SPELL_MAGIC_MAPPING:
        if (you.level_type == LEVEL_LABYRINTH || you.level_type == LEVEL_ABYSS)
#ifdef JP
            mpr("당신은 일시적인 방향감각 상실에 빠졌다.");
#else
            mpr("You feel momentarily disoriented.");
#endif
        else if (you.level_type == LEVEL_PANDEMONIUM)
#ifdef JP
            mpr("땅의 마법으로는 판데모니움의 지도를 그릴 수 없다.");
#else
            mpr("Your Earth magic cannot map Pandemonium.");
#endif
        else
        {
#ifdef JP
            mpr( "당신은 주위의 지형을 감지했다." );
#else
            mpr( "You feel aware of your surroundings." );
#endif
            powc = stepdown_value( powc, 10, 10, 40, 45 );
            magic_mapping( 5 + powc, 50 + random2avg( powc * 2, 2 ) );
        }
        break;

    case SPELL_HEAL_OTHER:
        if (spell_direction(spd, beam, DIR_NONE, TARG_FRIEND) == -1)
            return (false);

        if (spd.isMe)
        {
#ifdef JP
            mpr("유감이지만, 그것은 그렇게 사용할 수 없다.");
#else
            mpr("Sorry, it doesn't work like that.");
#endif
            return (false);
        }
        zapping(ZAP_HEALING, powc, beam);
        break;

    case SPELL_ANIMATE_DEAD:
#ifdef JP
        mpr("당신은 시체에게 노예가 될 것을 명했다.");
#else
        mpr("You call on the dead to walk for you.");
#endif
        animate_dead(powc + 1, BEH_FRIENDLY, you.pet_target, 1);
        break;

    case SPELL_PAIN:
        if (spell_direction(spd, beam) == -1)
            return (false);
        dec_hp(1, false);
        zapping(ZAP_PAIN, powc, beam);
        break;

    case SPELL_EXTENSION:
        extension(powc);
        break;

    case SPELL_CONTROL_UNDEAD:
        mass_enchantment(ENCH_CHARM, powc, MHITYOU);
        break;

    case SPELL_ANIMATE_SKELETON:
#ifdef JP
        mpr("당신은 시체에 생명을 주려고 시도했다...");
#else
        mpr("You attempt to give life to the dead...");
#endif
        animate_a_corpse(you.x_pos, you.y_pos, BEH_FRIENDLY, you.pet_target,
                         CORPSE_SKELETON);
        break;

    case SPELL_VAMPIRIC_DRAINING:
        vampiric_drain(powc);
        break;

    case SPELL_SUMMON_WRAITHS:
        summon_undead(powc);
        break;

    case SPELL_DETECT_ITEMS:
        detect_items(powc);
        break;

    case SPELL_BORGNJORS_REVIVIFICATION:
        cast_revivification(powc);
        break;

    case SPELL_BURN:
        burn_freeze(powc, BEAM_FIRE);
        break;

    case SPELL_FREEZE:
        burn_freeze(powc, BEAM_COLD);
        break;

    case SPELL_SUMMON_ELEMENTAL:
        summon_elemental(powc, 0, 2);
        break;

    case SPELL_OZOCUBUS_REFRIGERATION:
        cast_refrigeration(powc);
        break;

    case SPELL_STICKY_FLAME:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_STICKY_FLAME, powc, beam);
        break;

    case SPELL_SUMMON_ICE_BEAST:
        summon_ice_beast_etc(powc, MONS_ICE_BEAST);
        break;

    case SPELL_OZOCUBUS_ARMOUR:
        ice_armour(powc, false);
        break;

    case SPELL_CALL_IMP:
        if (one_chance_in(3))
            summon_ice_beast_etc(powc, MONS_WHITE_IMP);
        else if (one_chance_in(7))
            summon_ice_beast_etc(powc, MONS_SHADOW_IMP);
        else
            summon_ice_beast_etc(powc, MONS_IMP);
        break;

    case SPELL_REPEL_MISSILES:
        missile_prot(powc);
        break;

    case SPELL_BERSERKER_RAGE:
        cast_berserk();
        break;

    case SPELL_DISPEL_UNDEAD:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_DISPEL_UNDEAD, powc, beam);
        break;

    case SPELL_GUARDIAN:
        summon_ice_beast_etc(powc, MONS_ANGEL);
        break;

    //jmf: FIXME: SPELL_PESTILENCE?

    case SPELL_THUNDERBOLT:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_LIGHTNING, powc, beam);
        break;

    case SPELL_FLAME_OF_CLEANSING:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_CLEANSING_FLAME, powc, beam);
        break;

    //jmf: FIXME: SPELL_SHINING_LIGHT?

    case SPELL_SUMMON_DAEVA:
        summon_ice_beast_etc(powc, MONS_DAEVA);
        break;

    case SPELL_ABJURATION_II:
        abjuration(powc);
        break;

    // Remember that most holy spells above don't yet use powc!

    case SPELL_TWISTED_RESURRECTION:
        cast_twisted(powc, BEH_FRIENDLY, you.pet_target);
        break;

    case SPELL_REGENERATION:
        cast_regen(powc);
        break;

    case SPELL_BONE_SHARDS:
        cast_bone_shards(powc);
        break;

    case SPELL_BANISHMENT:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_BANISHMENT, powc, beam);
        break;

    case SPELL_CIGOTUVIS_DEGENERATION:
        if (spell_direction(spd, beam) == -1)
            return (false);

        if (spd.isMe)
        {
            canned_msg(MSG_UNTHINKING_ACT);
            return (false);
        }
        zapping(ZAP_DEGENERATION, powc, beam);
        break;

    case SPELL_STING:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_STING, powc, beam);
        break;

    case SPELL_SUBLIMATION_OF_BLOOD:
        sublimation(powc);
        break;

    case SPELL_TUKIMAS_DANCE:
        dancing_weapon(powc, false);
        break;

    case SPELL_HELLFIRE:
        // should only be available from:
        // staff of Dispater & Sceptre of Asmodeus
        if (spell_direction(spd, beam) == -1)
            return (false);

        zapping(ZAP_HELLFIRE, powc, beam);
        break;

    case SPELL_SUMMON_DEMON:
#ifdef JP
        mpr("당신은 판데모니움으로의 문을 열였다!");
#else
        mpr("You open a gate to Pandemonium!");
#endif
        summon_ice_beast_etc(powc, summon_any_demon(DEMON_COMMON));
        break;

    case SPELL_DEMONIC_HORDE:
#ifdef JP
        mpr("당신은 판데모니움으로의 문을 열였다!");
#else
        mpr("You open a gate to Pandemonium!");
#endif
        dem_hor2 = 3 + random2(5);
        for (dem_hor = 0; dem_hor < 4 + dem_hor2; dem_hor++)
        {
            summon_ice_beast_etc(powc, summon_any_demon(DEMON_LESSER));
        }
        break;

    case SPELL_SUMMON_GREATER_DEMON:
#ifdef JP
        mpr("당신은 판데모니움으로의 문을 열였다!");
#else
        mpr("You open a gate to Pandemonium!");
#endif

        dem_hor = ((random2(powc) <= 5) ? BEH_HOSTILE : BEH_CHARMED);

        if (dem_hor == BEH_CHARMED)
#ifdef JP
            mpr("이 존재에게는 그다지 호감을 가질 수 없다...");
#else
            mpr("You don't feel so good about this...");
#endif

        create_monster( summon_any_demon(DEMON_GREATER), ENCH_ABJ_V, dem_hor,
                        you.x_pos, you.y_pos, MHITYOU, 250 );

        break;

    case SPELL_CORPSE_ROT:
        corpse_rot(0);
        break;

    case SPELL_TUKIMAS_VORPAL_BLADE:
        if (!brand_weapon(SPWPN_VORPAL, powc))
            canned_msg(MSG_SPELL_FIZZLES);
        break;

    case SPELL_FIRE_BRAND:
        if (!brand_weapon(SPWPN_FLAMING, powc))
            canned_msg(MSG_SPELL_FIZZLES);
        break;

    case SPELL_FREEZING_AURA:
        if (!brand_weapon(SPWPN_FREEZING, powc))
            canned_msg(MSG_SPELL_FIZZLES);
        break;

    case SPELL_LETHAL_INFUSION:
        if (!brand_weapon(SPWPN_DRAINING, powc))
            canned_msg(MSG_SPELL_FIZZLES);
        break;

    case SPELL_MAXWELLS_SILVER_HAMMER:
        if (!brand_weapon(SPWPN_DUMMY_CRUSHING, powc))
            canned_msg(MSG_SPELL_FIZZLES);
        break;

    case SPELL_POISON_WEAPON:
        if (!brand_weapon(SPWPN_VENOM, powc))
            canned_msg(MSG_SPELL_FIZZLES);
        break;

    case SPELL_CRUSH:
        burn_freeze(powc, BEAM_MISSILE);
        break;

    case SPELL_BOLT_OF_IRON:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_IRON_BOLT, powc, beam);
        break;

    case SPELL_STONE_ARROW:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_STONE_ARROW, powc, beam);
        break;

    case SPELL_TOMB_OF_DOROKLOHE:
        entomb();
        break;

    case SPELL_STONEMAIL:
        stone_scales(powc);
        break;

    case SPELL_SHOCK:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_ELECTRICITY, powc, beam);
        break;

    case SPELL_SWIFTNESS:
        cast_swiftness(powc);
        break;

    case SPELL_FLY:
        cast_fly(powc);
        break;

    case SPELL_INSULATION:
        cast_insulation(powc);
        break;

    case SPELL_ORB_OF_ELECTROCUTION:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_ORB_OF_ELECTRICITY, powc, beam);
        break;

    case SPELL_DETECT_CREATURES:
        detect_creatures(powc);
        break;

    case SPELL_CURE_POISON_II:  // poison magic version of cure poison
        cast_cure_poison(powc);
        break;

    case SPELL_CONTROL_TELEPORT:
        cast_teleport_control(powc);
        break;

    case SPELL_POISON_AMMUNITION:
        cast_poison_ammo();
        break;

    case SPELL_RESIST_POISON:
        cast_resist_poison(powc);
        break;

    case SPELL_PROJECTED_NOISE:
        project_noise();
        break;

    case SPELL_ALTER_SELF:
        if (!enough_hp( you.hp_max / 2, true ))
        {
#ifdef JP
            mpr( "당신의 몸은 이 주문이 기능하기에는 무리가 있는 상태다." );
#else
            mpr( "Your body is in too poor a condition "
                 "for this spell to function." );
#endif

            return (false);
        }

#ifdef JP
        mpr("당신의 몸은 변이 에너지로 뒤덮였다!");
#else
        mpr("Your body is suffused with transfigurative energy!");
#endif

        set_hp( 1 + random2(you.hp), false );

        if (!mutate(100, false))
#ifdef JP
            mpr("이상하다... 당신은 아무것도 변화하지 않은 것 같다.");
#else
            mpr("Odd... you don't feel any different.");
#endif
        break;

    case SPELL_DEBUGGING_RAY:
        if (spell_direction(spd, beam, DIR_NONE, TARG_ANY) == -1)
            return (false);
        zapping(ZAP_DEBUGGING_RAY, powc, beam);
        break;

    case SPELL_RECALL:
        recall(0);
        break;

    case SPELL_PORTAL:
        portal();
        break;

    case SPELL_AGONY:
        if (spell_direction(spd, beam) == -1)
            return (false);

        if (spd.isMe)
        {
            canned_msg(MSG_UNTHINKING_ACT);
            return (false);
        }
        zapping(ZAP_AGONY, powc, beam);
        break;

    case SPELL_SPIDER_FORM:
        transform(powc, TRAN_SPIDER);
        break;

    case SPELL_DISRUPT:
        if (spell_direction(spd, beam) == -1)
            return (false);

        zapping(ZAP_DISRUPTION, powc, beam);
        break;

    case SPELL_DISINTEGRATE:
        if (spell_direction(spd, beam) == -1)
            return (false);

        if (spd.isMe)
        {
            canned_msg(MSG_UNTHINKING_ACT);
            return (false);
        }
        zapping(ZAP_DISINTEGRATION, powc, beam);
        break;

    case SPELL_BLADE_HANDS:
        transform(powc, TRAN_BLADE_HANDS);
        break;

    case SPELL_STATUE_FORM:
        transform(powc, TRAN_STATUE);
        break;

    case SPELL_ICE_FORM:
        transform(powc, TRAN_ICE_BEAST);
        break;

    case SPELL_DRAGON_FORM:
        transform(powc, TRAN_DRAGON);
        break;

    case SPELL_NECROMUTATION:
        transform(powc, TRAN_LICH);
        break;

    case SPELL_DEATH_CHANNEL:
        cast_death_channel(powc);
        break;

    case SPELL_SYMBOL_OF_TORMENT:
        if (you.is_undead || you.mutation[MUT_TORMENT_RESISTANCE])
        {
#ifdef JP
            mpr("다른 자를 괴롭히기 위해서는, 먼저 자신이 고통을 알지 않으면 안 된다.");
#else
            mpr("To torment others, one must first know what torment means. ");
#endif
            return (false);
        }
        torment(you.x_pos, you.y_pos);
        break;

    case SPELL_DEFLECT_MISSILES:
        deflection(powc);
        break;

    case SPELL_ORB_OF_FRAGMENTATION:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_ORB_OF_FRAGMENTATION, powc, beam);
        break;

    case SPELL_ICE_BOLT:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_ICE_BOLT, powc, beam);
        break;

    case SPELL_ARC:
        burn_freeze(powc, BEAM_ELECTRICITY);
        break;

    case SPELL_AIRSTRIKE:
        airstrike(powc);
        break;

    case SPELL_ICE_STORM:
        if (spell_direction(spd, beam) == -1)
            return (false);
        zapping(ZAP_ICE_STORM, powc, beam);
        break;

    case SPELL_SHADOW_CREATURES:
#ifdef JP
        mpr( "흔들리는 그림자가 당신의 주위에서 선회하고 있다..." );
#else
        mpr( "Wisps of shadow whirl around you..." );
#endif
        create_monster( RANDOM_MONSTER, ENCH_ABJ_II, BEH_FRIENDLY,
                        you.x_pos, you.y_pos, you.pet_target, 250 );
        break;

    //jmf: new spells 19mar2000
    case SPELL_FLAME_TONGUE:
        if (spell_direction(spd, beam) == -1)
            return (false);

        if (spd.isMe)
        {
            canned_msg(MSG_UNTHINKING_ACT);
            return (false);
        }
        zapping(ZAP_FLAME_TONGUE, powc, beam);

        /*
        // This is not the place for this sort of power adjustment,
        // it just makes it harder to balance -- bwr
        zapping(ZAP_FLAME_TONGUE, 2 + random2(4) + random2(4)
                                            + random2(powc) / 25, beam);
        */
        break;

    case SPELL_PASSWALL:
        cast_passwall(powc);
        break;

    case SPELL_IGNITE_POISON:
        cast_ignite_poison(powc);
        break;

    case SPELL_STICKS_TO_SNAKES:
        cast_sticks_to_snakes(powc);
        break;

    case SPELL_SUMMON_LARGE_MAMMAL:
        cast_summon_large_mammal(powc);
        break;

    case SPELL_SUMMON_DRAGON:
        cast_summon_dragon(powc);
        break;

    case SPELL_TAME_BEASTS:
        cast_tame_beasts(powc);
        break;

    case SPELL_SLEEP:
        if (spell_direction(spd, beam) == -1)
            return (false);

        if (spd.isMe)
        {
            canned_msg(MSG_UNTHINKING_ACT);
            return (false);
        }

        zapping(ZAP_SLEEP, powc, beam);
        break;

    case SPELL_MASS_SLEEP:
        cast_mass_sleep(powc);
        break;

//!!!!
    case SPELL_DETECT_MAGIC:
#ifdef JP
        mpr("FIXME: implement!");
#else
        mpr("FIXME: implement!");
#endif
        break;

    case SPELL_DETECT_SECRET_DOORS:
        cast_detect_secret_doors(powc);
        break;

    case SPELL_SEE_INVISIBLE:
        cast_see_invisible(powc);
        break;

    case SPELL_FORESCRY:
        cast_forescry(powc);
        break;

    case SPELL_SUMMON_BUTTERFLIES:
        cast_summon_butterflies(powc);
        break;

    case SPELL_WARP_BRAND:
        if (!brand_weapon(SPWPN_DISTORTION, powc))
            canned_msg(MSG_SPELL_FIZZLES);
        break;

    case SPELL_SILENCE:
        cast_silence(powc);
        break;

    case SPELL_SHATTER:
        cast_shatter(powc);
        break;

    case SPELL_DISPERSAL:
        cast_dispersal(powc);
        break;

    case SPELL_DISCHARGE:
        cast_discharge(powc);
        break;

    case SPELL_BEND:
        cast_bend(powc);
        break;

    case SPELL_BACKLIGHT:
        if (spell_direction(spd, beam) == -1)
            return (false);
        if (spd.isMe)
        {
            canned_msg(MSG_UNTHINKING_ACT);
            return (false);
        }
        zapping(ZAP_BACKLIGHT, powc + 10, beam);
        break;

    case SPELL_INTOXICATE:
        cast_intoxicate(powc);
        break;

    case SPELL_GLAMOUR:
        cast_glamour(powc);
        break;

    case SPELL_EVAPORATE:
        cast_evaporate(powc);
        break;

    case SPELL_FULSOME_DISTILLATION:
        cast_fulsome_distillation(powc);
        break;

    case SPELL_FRAGMENTATION:
        cast_fragmentation(powc);
        break;

    case SPELL_AIR_WALK:
        transform(powc, TRAN_AIR);
        break;

    case SPELL_SANDBLAST:
        cast_sandblast(powc);
        break;

    case SPELL_ROTTING:
        cast_rotting(powc);
        break;

    case SPELL_SHUGGOTH_SEED:
        cast_shuggoth_seed(powc);
        break;

    case SPELL_CONDENSATION_SHIELD:
        cast_condensation_shield(powc);
        break;

    case SPELL_SEMI_CONTROLLED_BLINK:
        cast_semi_controlled_blink(powc);       //jmf: powc is ignored
        break;

    case SPELL_STONESKIN:
        cast_stoneskin(powc);
        break;

    case SPELL_SIMULACRUM:
        simulacrum(powc);
        break;

    case SPELL_CONJURE_BALL_LIGHTNING:
        cast_conjure_ball_lightning(powc);
        break;

    case SPELL_TWIST:
        cast_twist(powc);
        break;

    case SPELL_FAR_STRIKE:
        cast_far_strike(powc);
        break;

    case SPELL_SWAP:
        cast_swap(powc);
        break;

    case SPELL_APPORTATION:
        cast_apportation(powc);
        break;

    default:
#ifdef JP
        mpr("효과 없는 주문이다!");
#else
        mpr("Invalid spell!");
#endif
        break;
    }                           // end switch

    return (true);
}                               // end you_spells()

void exercise_spell( int spell, bool spc, bool success )
{
    // (!success) reduces skill increase for miscast spells
    int ndx = 0;
    int skill;
    int workout = 0;

    unsigned int disciplines = spell_type(spell);

    //jmf: evil evil evil -- exclude HOLY bit
    disciplines &= (~SPTYP_HOLY);

    int skillcount = count_bits( disciplines );

    if (!success)
        skillcount += 4 + random2(10);

    for (ndx = 0; ndx <= SPTYP_LAST_EXPONENT; ndx++)
    {
        if (!spell_typematch( spell, 1 << ndx ))
            continue;

        skill = spell_type2skill( 1 << ndx );
        workout = (random2(1 + spell_difficulty(spell)) / skillcount);

        if (!one_chance_in(5))
            workout++;       // most recently, this was an automatic add {dlb}

        exercise( skill, workout );
    }

    /* ******************************************************************
       Other recent formulae for the above:

       * workout = random2(spell_difficulty(spell_ex)
       * (10 + (spell_difficulty(spell_ex) * 2 )) / 10 / spellsy + 1);

       * workout = spell_difficulty(spell_ex)
       * (15 + spell_difficulty(spell_ex)) / 15 / spellsy;

       spellcasting had also been generally exercised at the same time
       ****************************************************************** */

    if (spc)
    {
        exercise(SK_SPELLCASTING, one_chance_in(3) ? 1
                            : random2(1 + random2(spell_difficulty(spell))));
    }

    //+ (coinflip() ? 1 : 0) + (skillcount ? 1 : 0));

/* ******************************************************************
   3.02 was:

    if ( spc && spellsy )
      exercise(SK_SPELLCASTING, random2(random2(spell_difficulty(spell_ex) + 1) / spellsy)); // + 1);
    else if ( spc )
      exercise(SK_SPELLCASTING, random2(random2(spell_difficulty(spell_ex)))); // + 1);
****************************************************************** */

}                               // end exercise_spell()

/* This determines how likely it is that more powerful wild magic effects
 * will occur. Set to 100 for the old probabilities (although the individual
 * effects have been made much nastier since then).
 */

bool miscast_effect( unsigned int sp_type, int mag_pow, int mag_fail,
                     int force_effect, const char *cause )
{
/*  sp_type is the type of the spell
 *  mag_pow is overall power of the spell or effect (ie its level)
 *  mag_fail is the degree to which you failed
 *  force_effect forces a certain effect to occur. Currently unused.
 */
    struct bolt beam;
    bool failMsg = true;

    int loopj = 0;
    int spec_effect = 0;
    int hurted = 0;

    if (sp_type == SPTYP_RANDOM)
        sp_type = 1 << (random2(12));

    spec_effect = (mag_pow * mag_fail * (10 + mag_pow) / 7
                                                * WILD_MAGIC_NASTINESS) / 100;

    if (force_effect == 100
        && random2(40) > spec_effect && random2(40) > spec_effect)
    {
        canned_msg(MSG_NOTHING_HAPPENS);
        return (false);
    }

    // setup beam
    beam.isTracer = false;

    spec_effect = spec_effect / 100;

#if DEBUG_DIAGNOSTICS
    const int old_fail = spec_effect;
#endif

    spec_effect = random2(spec_effect);

    if (spec_effect > 3)
        spec_effect = 3;
    else if (spec_effect < 0)
        spec_effect = 0;

#if DEBUG_DIAGNOSTICS
#ifdef JP
    snprintf( info, INFO_SIZE, "Sptype: %d, failure1: %d, failure2: %d",
#else
    snprintf( info, INFO_SIZE, "Sptype: %d, failure1: %d, failure2: %d",
#endif
              sp_type, old_fail, spec_effect );
    mpr( info, MSGCH_DIAGNOSTICS );
#endif

    if (force_effect != 100)
        spec_effect = force_effect;

    switch (sp_type)
    {
    case SPTYP_CONJURATION:
        switch (spec_effect)
        {
        case 0:         // just a harmless message
            switch (random2(10))
            {
            case 0:
#ifdef JP
                snprintf( info, INFO_SIZE, "당신의 %s에서 불꽃이 튀었다!",
#else
                snprintf( info, INFO_SIZE, "Sparks fly from your %s!",
#endif
                          your_hand(true) );
                mpr(info);
                break;

            case 1:
#ifdef JP
                mpr("당신 주위의 대기가 에너지로 파직파직 튀었다!");
#else
                mpr("The air around you crackles with energy!");
#endif
                break;

            case 2:
#ifdef JP
                snprintf( info, INFO_SIZE, "당신의 %s에서 한 줄기의 연기가 피어 올랐다.",
#else
                snprintf( info, INFO_SIZE, "Wisps of smoke drift from your %s.",
#endif
                          your_hand(true));
                mpr(info);
                break;
            case 3:
#ifdef JP
                mpr("당신은 기묘한 에너지의 고조를 느꼈다!");
#else
                mpr("You feel a strange surge of energy!");
#endif
                break;
            case 4:
#ifdef JP
                mpr("당신은 잠깐 섬광 때문에 눈이 부셨다!");
#else
                mpr("You are momentarily dazzled by a flash of light!");
#endif
                break;
            case 5:
#ifdef JP
                mpr("기묘한 에너지가 당신의 몸을 질주해 나갔다.");
#else
                mpr("Strange energies run through your body.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("당신은 피부가 따끔따끔하다.");
#else
                mpr("Your skin tingles.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("당신의 피부는 잠깐 빛났다.");
#else
                mpr("Your skin glows momentarily.");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
                // josh declares mummies cannot smell {dlb}
                if (you.species != SP_MUMMY)
#ifdef JP
                    mpr("당신은 무언가 기묘한 냄새를 맡았다.");
#else
                    mpr("You smell something strange.");
#endif
                else
#ifdef JP
                    mpr("당신의 붕대가 펄럭펄럭 나부꼈다.");
#else
                    mpr("Your bandages flutter.");
#endif
            }
            break;

        case 1:         // a bit less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                snprintf( info, INFO_SIZE, "당신의 %s에서 연기가 피어 올랐다!",
#else
                snprintf( info, INFO_SIZE, "Smoke pours from your %s!",
#endif
                          your_hand(true));
                mpr(info);

                big_cloud( CLOUD_GREY_SMOKE, you.x_pos, you.y_pos, 20,
                           7 + random2(7) );
                break;
            case 1:
#ifdef JP
                mpr("맹렬한 에너지의 파도가 당신의 전신에 파고들었다.");
#else
                mpr("A wave of violent energy washes through your body!");
#endif
                ouch(6 + random2avg(7, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            }
            break;

        case 2:         // rather less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("에너지가 당신의 몸을 찢어발겼다!");
#else
                mpr("Energy rips through your body!");
#endif
                ouch(9 + random2avg(17, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            case 1:
#ifdef JP
                mpr("당신은 맹렬한 폭발에 휩쓸렸다!");
#else
                mpr("You are caught in a violent explosion!");
#endif
                beam.type = SYM_BURST;
                beam.damage = dice_def( 3, 12 );
                beam.flavour = BEAM_MISSILE; // unsure about this
                // BEAM_EXPLOSION instead? {dlb}

                beam.target_x = you.x_pos;
                beam.target_y = you.y_pos;
#ifdef JP
                strcpy(beam.beam_name, "폭발");
#else
                strcpy(beam.beam_name, "explosion");
#endif
                beam.colour = random_colour();
                beam.beam_source = NON_MONSTER;
                beam.thrower = (cause) ? KILL_MISC : KILL_YOU;
                beam.aux_source = cause;
                beam.ex_size = 1;

                explosion(beam);
                break;
            }
            break;

        case 3:         // considerably less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("당신은 마법 에너지의 방출에 맞았다!");
#else
                mpr("You are blasted with magical energy!");
#endif
                ouch(12 + random2avg(29, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            case 1:
#ifdef JP
                mpr("마법 에너지가 갑자기 폭발했다!");
#else
                mpr("There is a sudden explosion of magical energy!");
#endif
                beam.type = SYM_BURST;
                beam.damage = dice_def( 3, 20 );
                beam.flavour = BEAM_MISSILE; // unsure about this
                // BEAM_EXPLOSION instead? {dlb}
                beam.target_x = you.x_pos;
                beam.target_y = you.y_pos;
#ifdef JP
                strcpy(beam.beam_name, "폭발");
#else
                strcpy(beam.beam_name, "explosion");
#endif
                beam.colour = random_colour();
                beam.beam_source = NON_MONSTER;
                beam.thrower = (cause) ? KILL_MISC : KILL_YOU;
                beam.aux_source = cause;
                beam.ex_size = coinflip()?1:2;

                explosion(beam);
                break;
            }
            break;
        }
        break;                  // end conjuration

    case SPTYP_ENCHANTMENT:
        switch (spec_effect)
        {
        case 0:         // harmless messages only
            switch (random2(10))
            {
            case 0:
#ifdef JP
                snprintf( info, INFO_SIZE, "당신의 %s은(는) 잠깐 빛났다.",
#else
                snprintf( info, INFO_SIZE, "Your %s glow momentarily.",
#endif
                          your_hand(true) );
                mpr(info);
                break;
            case 1:
#ifdef JP
                mpr("당신 주위의 대기가 에너지로 파직파직 튀었다!");
#else
                mpr("The air around you crackles with energy!");
#endif
                break;
            case 2:
#ifdef JP
                mpr("형형색색의 빛이 당신의 눈앞에서 춤추었다!");
#else
                mpr("Multicolored lights dance before your eyes!");
#endif
                break;
            case 3:
#ifdef JP
                mpr("당신은 기묘한 에너지의 고조를 느꼈다!");
#else
                mpr("You feel a strange surge of energy!");
#endif
                break;
            case 4:
#ifdef JP
                mpr("빛의 파동이 당신의 몸 위로 퍼져 나갔다.");
#else
                mpr("Waves of light ripple over your body.");
#endif
                break;
            case 5:
#ifdef JP
                mpr("기묘한 에너지가 당신의 몸을 질주해 나갔다.");
#else
                mpr("Strange energies run through your body.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("당신은 피부가 따끔따끔하다.");
#else
                mpr("Your skin tingles.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("당신의 피부는 잠깐 빛났다.");
#else
                mpr("Your skin glows momentarily.");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
                if (!silenced(you.x_pos, you.y_pos))
#ifdef JP
                    mpr("당신은 무언가 기묘한 소리를 들었다.");
#else
                    mpr("You hear something strange.");
#endif
                else if (you.attribute[ATTR_TRANSFORMATION] != TRAN_AIR)
#ifdef JP
                    mpr("당신의 두개골은 가볍게 진동했다.");
#else
                    mpr("Your skull vibrates slightly.");
#endif
                else
                    canned_msg(MSG_NOTHING_HAPPENS);
                break;
            }
            break;

        case 1:         // slightly annoying
            switch (random2(2))
            {
            case 0:
                potion_effect(POT_LEVITATION, 20);
                break;
            case 1:
                random_uselessness(2 + random2(7), 0);
                break;
            }
            break;

        case 2:         // much more annoying
            switch (random2(7))
            {
            case 0:
            case 1:
            case 2:
#ifdef JP
                mpr("당신은 악의 있는 영기를 느꼈다.");
#else
                mpr("You sense a malignant aura.");
#endif
                curse_an_item(0, 0);
                break;
            case 3:
            case 4:
            case 5:
                potion_effect(POT_SLOWING, 10);
                break;
            case 6:
                potion_effect(POT_BERSERK_RAGE, 10);
                break;
            }
            break;

        case 3:         // potentially lethal
            switch (random2(4))
            {
            case 0:
                do
                {
                    curse_an_item(0, 0);
                    loopj = random2(3);
                }
                while (loopj != 0);

#ifdef JP
                mpr("당신은 압도적의 악의의 영기를 느꼈다!");
#else
                mpr("You sense an overwhelmingly malignant aura!");
#endif
                break;
            case 1:
                potion_effect(POT_PARALYSIS, 10);
                break;
            case 2:
                potion_effect(POT_CONFUSION, 10);
                break;
            case 3:
#ifdef JP
                mpr("당신은 제어할 수 없는 에너지의 침투를 느꼈다!");
#else
                mpr("You feel saturated with unharnessed energies!");
#endif
                you.magic_contamination += random2avg(19,3);
                break;
            }
            break;
        }
        break;                  // end enchantments

    case SPTYP_TRANSLOCATION:
        switch (spec_effect)
        {
        case 0:         // harmless messages only
            switch (random2(10))
            {
            case 0:
#ifdef JP
                mpr("당신 주위의 공간이 일그러졌다.");
#else
                mpr("Space warps around you.");
#endif
                break;
            case 1:
#ifdef JP
                mpr("당신 주위의 대기가 에너지로 파직파직 튀었다!");
#else
                mpr("The air around you crackles with energy!");
#endif
                break;
            case 2:
#ifdef JP
                mpr("당신은 비틀린 감각을 맛보았다.");
#else
                mpr("You feel a wrenching sensation.");
#endif
                break;
            case 3:
#ifdef JP
                mpr("당신은 기묘한 에너지의 고조를 느꼈다!");
#else
                mpr("You feel a strange surge of energy!");
#endif
                break;
            case 4:
#ifdef JP
                mpr("당신은 그 자리에서 빙글빙글 회전했다.");
#else
                mpr("You spin around.");
#endif
                break;
            case 5:
#ifdef JP
                mpr("기묘한 에너지가 당신의 몸을 질주해 나갔다.");
#else
                mpr("Strange energies run through your body.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("당신은 피부가 따끔따끔하다.");
#else
                mpr("Your skin tingles.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("세계가 잠깐 일그러져 보였다!");
#else
                mpr("The world appears momentarily distorted!");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
#ifdef JP
                mpr("당신은 기분 나쁨을 맛보았다.");
#else
                mpr("You feel uncomfortable.");
#endif
                break;
            }
            break;

        case 1:         // mostly harmless
            switch (random2(6))
            {
            case 0:
            case 1:
            case 2:
#ifdef JP
                mpr("당신은 국지적인 공간 왜곡의 장소에 붙들렸다.");
#else
                mpr("You are caught in a localised field of spatial distortion.");
#endif
                ouch(4 + random2avg(9, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            case 3:
            case 4:
#ifdef JP
                mpr("당신 주위의 공간이 구부러졌다!");
#else
                mpr("Space bends around you!");
#endif
                random_blink(false);
                ouch(4 + random2avg(7, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            case 5:
#ifdef JP
                mpr("공간이 소용돌이치며 일그러졌다!");
#else
                mpr("Space twists in upon itself!");
#endif
                create_monster( MONS_SPATIAL_VORTEX, ENCH_ABJ_III, BEH_HOSTILE,
                                you.x_pos, you.y_pos, MHITYOU, 250 );
                break;
            }
            break;

        case 2:         // less harmless
            switch (random2(7))
            {
            case 0:
            case 1:
            case 2:
#ifdef JP
                mpr("당신은 강력한 국지적 공간 왜곡의 장소에 붙들렸다.");
#else
                mpr("You are caught in a strong localised spatial distortion.");
#endif
                ouch(9 + random2avg(23, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            case 3:
            case 4:
#ifdef JP
                mpr("당신 주위의 공간이 일그러졌다!");
#else
                mpr("Space warps around you!");
#endif

                if (one_chance_in(3))
                    you_teleport2( true );
                else
                    random_blink( false );

                ouch(5 + random2avg(9, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                potion_effect(POT_CONFUSION, 40);
                break;
            case 5:
#ifdef JP
                mpr("공간이 소용돌이치며 일그러졌다!");
#else
                mpr("Space twists in upon itself!");
#endif

                for (loopj = 0; loopj < 2 + random2(3); loopj++)
                {
                    create_monster( MONS_SPATIAL_VORTEX, ENCH_ABJ_III,
                                    BEH_HOSTILE, you.x_pos, you.y_pos,
                                    MHITYOU, 250 );
                }
                break;
            case 6:
#ifdef JP
                mpr("당신은 어비스에 던져 넣어졌다!");
#else
                mpr("You are cast into the Abyss!");
#endif
                more();
                banished(DNGN_ENTER_ABYSS);     // sends you to the abyss
                break;
            }
            break;

        case 3:         // much less harmless

            switch (random2(4))
            {
            case 0:
#ifdef JP
                mpr("당신은 대단히 강력한 국지적 공간 왜곡의 장소에 붙들렸다!");
#else
                mpr("You are caught in an extremely strong localised spatial distortion!");
#endif
                ouch(15 + random2avg(29, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            case 1:
#ifdef JP
                mpr("당신 주위의 공간이 미친 듯이 왜곡되었다!");
#else
                mpr("Space warps crazily around you!");
#endif
                you_teleport2( true );

                ouch(9 + random2avg(17, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                potion_effect(POT_CONFUSION, 60);
                break;
            case 2:
#ifdef JP
                mpr("당신은 어비스에 던져 넣어졌다!");
#else
                mpr("You are cast into the Abyss!");
#endif
                more();
                banished(DNGN_ENTER_ABYSS);     // sends you to the abyss
                break;
            case 3:
#ifdef JP
                mpr("당신은 제어할 수 없는 에너지의 침투를 느꼈다!");
#else
                mpr("You feel saturated with unharnessed energies!");
#endif
                you.magic_contamination += random2avg(19,3);
                break;
            }
            break;
        }
        break;                  // end translocations

    case SPTYP_SUMMONING:
        switch (spec_effect)
        {
        case 0:         // harmless messages only
            switch (random2(10))
            {
            case 0:
#ifdef JP
                mpr("그림자 같은 윤곽이 당신 주위의 공중에 나타나, 사라졌다.");
#else
                mpr("Shadowy shapes form in the air around you, then vanish.");
#endif
                break;
            case 1:
                if (!silenced(you.x_pos, you.y_pos))
#ifdef JP
                    mpr("당신은 기묘한 목소리를 들었다.");
#else
                    mpr("You hear strange voices.");
#endif
                else
#ifdef JP
                    mpr("당신은 잠깐 눈이 부셨다.");
#else
                    mpr("You feel momentarily dizzy.");
#endif
                break;
            case 2:
#ifdef JP
                mpr("당신은 머리가 아팠다.");
#else
                mpr("Your head hurts.");
#endif
                break;
            case 3:
#ifdef JP
                mpr("당신은 기묘한 에너지의 고조를 느꼈다!");
#else
                mpr("You feel a strange surge of energy!");
#endif
                break;
            case 4:
#ifdef JP
                mpr("당신은 뇌를 다쳤다!");
#else
                mpr("Your brain hurts!");
#endif
                break;
            case 5:
#ifdef JP
                mpr("기묘한 에너지가 당신의 몸을 질주해 나갔다.");
#else
                mpr("Strange energies run through your body.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("잠깐, 세계가 일그러져 보였다.");
#else
                mpr("The world appears momentarily distorted.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("당신 주위의 공간이 일그러졌다.");
#else
                mpr("Space warps around you.");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
#ifdef JP
                mpr("멀리서의 목소리가 당신을 불러 세웠다!");
#else
                mpr("Distant voices call out to you!");
#endif
                break;
            }
            break;

        case 1:         // a little bad
            switch (random2(6))
            {
            case 0:             // identical to translocation
            case 1:
            case 2:
#ifdef JP
                mpr("당신은 국지적인 공간 왜곡에 붙들렸다.");
#else
                mpr("You are caught in a localised spatial distortion.");
#endif
                ouch(5 + random2avg(9, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;

            case 3:
#ifdef JP
                mpr("공간이 소용돌이치며 일그러졌다!");
#else
                mpr("Space twists in upon itself!");
#endif
                create_monster( MONS_SPATIAL_VORTEX, ENCH_ABJ_III, BEH_HOSTILE,
                                you.x_pos, you.y_pos, MHITYOU, 250 );
                break;

            case 4:
            case 5:
                if (create_monster( summon_any_demon(DEMON_LESSER), ENCH_ABJ_V,
                                    BEH_HOSTILE, you.x_pos, you.y_pos,
                                    MHITYOU, 250 ) != -1)
                {
#ifdef JP
                    mpr("섬광 속에서 무언가가 나타났다!");
#else
                    mpr("Something appears in a flash of light!");
#endif
                }
                break;
            }

        case 2:         // more bad
            switch (random2(6))
            {
            case 0:
#ifdef JP
                mpr("공간이 소용돌이치며 일그러졌다!");
#else
                mpr("Space twists in upon itself!");
#endif

                for (loopj = 0; loopj < 2 + random2(3); loopj++)
                {
                    create_monster( MONS_SPATIAL_VORTEX, ENCH_ABJ_III,
                                    BEH_HOSTILE, you.x_pos, you.y_pos,
                                    MHITYOU, 250 );
                }
                break;

            case 1:
            case 2:
                if (create_monster( summon_any_demon(DEMON_COMMON), ENCH_ABJ_V,
                                    BEH_HOSTILE, you.x_pos, you.y_pos,
                                    MHITYOU, 250) != -1)
                {
#ifdef JP
                    mpr("누군가가 어딘가에서 나타났다!");
#else
                    mpr("Something forms out of thin air!");
#endif
                }
                break;

            case 3:
            case 4:
            case 5:
#ifdef JP
                mpr("재잘재잘거리는 소리의 합창이 당신을 부르고 있다!");
#else
                mpr("A chorus of chattering voices calls out to you!");
#endif
                create_monster( summon_any_demon(DEMON_LESSER), ENCH_ABJ_V,
                                BEH_HOSTILE, you.x_pos, you.y_pos,
                                MHITYOU, 250 );

                create_monster( summon_any_demon(DEMON_LESSER), ENCH_ABJ_V,
                                BEH_HOSTILE, you.x_pos, you.y_pos,
                                MHITYOU, 250 );

                if (coinflip())
                {
                    create_monster( summon_any_demon(DEMON_LESSER), ENCH_ABJ_V,
                                    BEH_HOSTILE, you.x_pos, you.y_pos,
                                    MHITYOU, 250 );
                }

                if (coinflip())
                {
                    create_monster( summon_any_demon(DEMON_LESSER), ENCH_ABJ_V,
                                    BEH_HOSTILE, you.x_pos, you.y_pos,
                                    MHITYOU, 250 );
                }
                break;
            }
            break;

        case 3:         // more bad
            switch (random2(4))
            {
            case 0:
                if (create_monster( MONS_ABOMINATION_SMALL, 0, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250 ) != -1)
                {
#ifdef JP
                    mpr("누군가가 어딘가에서 나타났다.");
#else
                    mpr("Something forms out of thin air.");
#endif
                }
                break;

            case 1:
                if (create_monster( summon_any_demon(DEMON_GREATER), 0,
                                    BEH_HOSTILE, you.x_pos, you.y_pos,
                                    MHITYOU, 250 ) != -1)
                {
#ifdef JP
                    mpr("당신은 적대적인 존재를 눈치챘다.");
#else
                    mpr("You sense a hostile presence.");
#endif
                }
                break;

            case 2:
#ifdef JP
                mpr("누군가가 당신에게 그 악의 있는 주시를 향했다.");
#else
                mpr("Something turns its malign attention towards you...");
#endif

                create_monster( summon_any_demon(DEMON_COMMON), ENCH_ABJ_III,
                                BEH_HOSTILE, you.x_pos, you.y_pos,
                                MHITYOU, 250 );

                create_monster( summon_any_demon(DEMON_COMMON), ENCH_ABJ_III,
                                BEH_HOSTILE, you.x_pos, you.y_pos,
                                MHITYOU, 250);

                if (coinflip())
                {
                    create_monster(summon_any_demon(DEMON_COMMON), ENCH_ABJ_III,
                                   BEH_HOSTILE, you.x_pos, you.y_pos,
                                   MHITYOU, 250);
                }
                break;

            case 3:
#ifdef JP
                mpr("당신은 어비스에 던져 넣어졌다!");
#else
                mpr("You are cast into the Abyss!");
#endif
                banished(DNGN_ENTER_ABYSS);
                break;
            }
            break;
        }                       // end Summonings
        break;

    case SPTYP_DIVINATION:
        switch (spec_effect)
        {
        case 0:         // just a harmless message
            switch (random2(10))
            {
            case 0:
#ifdef JP
                mpr("기분 나쁜 이미지가 당신의 마음을 질주해 나갔다.");
#else
                mpr("Weird images run through your mind.");
#endif
                break;
            case 1:
                if (!silenced(you.x_pos, you.y_pos))
#ifdef JP
                    mpr("당신은 기묘한 목소리를 들었다.");
#else
                    mpr("You hear strange voices.");
#endif
                else
#ifdef JP
                    mpr("당신의 코가 씰룩거렸다.");
#else
                    mpr("Your nose twitches.");
#endif
                break;
            case 2:
#ifdef JP
                mpr("당신은 머리가 아팠다.");
#else
                mpr("Your head hurts.");
#endif
                break;
            case 3:
#ifdef JP
                mpr("당신은 기묘한 에너지의 고조를 느꼈다!");
#else
                mpr("You feel a strange surge of energy!");
#endif
                break;
            case 4:
#ifdef JP
                mpr("당신은 뇌를 다쳤다!");
#else
                mpr("Your brain hurts!");
#endif
                break;
            case 5:
#ifdef JP
                mpr("기묘한 에너지가 당신의 몸을 질주해 나갔다.");
#else
                mpr("Strange energies run through your body.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("순간 모든 것이 희미해져 보였다.");
#else
                mpr("Everything looks hazy for a moment.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("당신은 무언가 잊어버린 듯한 기분이 들지만, 그것이 무엇이였는지를 기억해낼 수 없다!");
#else
                mpr("You seem to have forgotten something, but you can't remember what it was!");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
#ifdef JP
                mpr("당신은 기분 나쁨을 맛보았다.");
#else
                mpr("You feel uncomfortable.");
#endif
                break;
            }
            break;

        case 1:         // more annoying things
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("당신은 가벼운 방향감각 상실에 걸렸다.");
#else
                mpr("You feel slightly disoriented.");
#endif
                forget_map(10 + random2(10));
                break;
            case 1:
                potion_effect(POT_CONFUSION, 10);
                break;
            }
            break;

        case 2:         // even more annoying things
            switch (random2(2))
            {
            case 0:
                if (you.is_undead)
#ifdef JP
                    mpr("당신은 갑자기 지금까지의 인생을 생각해 냈다!");
#else
                    mpr("You suddenly recall your previous life!");
#endif
                else if (lose_stat(STAT_INTELLIGENCE, 1 + random2(3)))
#ifdef JP
                    mpr("당신은 뇌에 손상을 입었다!");
#else
                    mpr("You have damaged your brain!");
#endif
                else
#ifdef JP
                    mpr("당신은 심한 두통에 시달렸다.");
#else
                    mpr("You have a terrible headache.");
#endif
                break;
            case 1:
#ifdef JP
                mpr("당신은 길을 잃었다.");
#else
                mpr("You feel lost.");
#endif
                forget_map(40 + random2(40));
                break;
            }

            potion_effect(POT_CONFUSION, 1);  // common to all cases here {dlb}
            break;

        case 3:         // nasty
            switch (random2(3))
            {
            case 0:
#ifdef JP
                mpr( forget_spell() ? "당신은 주문을 잊어버렸다!"
                                    : "당신은 격렬한 두통에 시달렸다!" );
#else
                mpr( forget_spell() ? "You have forgotten a spell!"
                                    : "You get a splitting headache." );
#endif
                break;
            case 1:
#ifdef JP
                mpr("당신은 완전히 길을 잃었다.");
#else
                mpr("You feel completely lost.");
#endif
                forget_map(100);
                break;
            case 2:
                if (you.is_undead)
#ifdef JP
                    mpr("당신은 갑자기 지금까지의 인생을 생각해 냈다.");
#else
                    mpr("You suddenly recall your previous life.");
#endif
                else if (lose_stat(STAT_INTELLIGENCE, 3 + random2(3)))
#ifdef JP
                    mpr("당신은 뇌에 손상을 입었다!");
#else
                    mpr("You have damaged your brain!");
#endif
                else
#ifdef JP
                    mpr("당신은 심한 두통에 시달렸다.");
#else
                    mpr("You have a terrible headache.");
#endif
                break;
            }

            potion_effect(POT_CONFUSION, 1);  // common to all cases here {dlb}
            break;
        }
        break;                  // end divinations

    case SPTYP_NECROMANCY:
        if (you.religion == GOD_KIKUBAAQUDGHA
            && (!player_under_penance() && you.piety >= 50
                && random2(150) <= you.piety))
        {
            canned_msg(MSG_NOTHING_HAPPENS);
            break;
        }

        switch (spec_effect)
        {
        case 0:
            switch (random2(10))
            {
            case 0:
                // mummies cannot smell {dlb}
                if (you.species != SP_MUMMY)
#ifdef JP
                    mpr("당신은 지독한 부패의 냄새를 맡았다.");
#else
                    mpr("You smell decay.");
#endif
                break;
            case 1:
                if (!silenced(you.x_pos, you.y_pos))
#ifdef JP
                    mpr("당신은 멀리서의 기묘한 목소리를 들었다.");
#else
                    mpr("You hear strange and distant voices.");
#endif
                else
#ifdef JP
                    mpr("당신은 자신의 집이 그리워졌다.");
#else
                    mpr("You feel homesick.");
#endif
                break;
            case 2:
#ifdef JP
                mpr("고통이 당신의 몸을 꿰뚫었다.");
#else
                mpr("Pain shoots through your body.");
#endif
                break;
            case 3:
#ifdef JP
                mpr("당신은 뼈가 쑤셨다.");
#else
                mpr("Your bones ache.");
#endif
                break;
            case 4:
#ifdef JP
                mpr("당신은 주위의 세계가 잠시 어두침침해졌던 것 같다고 느꼈다.");
#else
                mpr("The world around you seems to dim momentarily.");
#endif
                break;
            case 5:
#ifdef JP
                mpr("기묘한 에너지가 당신의 몸을 질주해 나갔다.");
#else
                mpr("Strange energies run through your body.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("당신은 추위에 떨었다.");
#else
                mpr("You shiver with cold.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("당신은 악의 있는 영기를 느꼈다.");
#else
                mpr("You sense a malignant aura.");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
#ifdef JP
                mpr("당신은 극심한 기분 나쁨을 맛보았다.");
#else
                mpr("You feel very uncomfortable.");
#endif
                break;
            }
            break;

        case 1:         // a bit nasty
            switch (random2(3))
            {
            case 0:
                if (you.is_undead)
                {
#ifdef JP
                    mpr("당신은 잠깐 동안 기묘한 기분을 느꼈다.");
#else
                    mpr("You feel weird for a moment.");
#endif
                    break;
                }
#ifdef JP
                mpr("고통이 당신의 몸을 꿰뚫었다.");
#else
                mpr("Pain shoots through your body!");
#endif
                ouch(5 + random2avg(15, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            case 1:
#ifdef JP
                mpr("당신은 심한 무력감을 느꼈다.");
#else
                mpr("You feel horribly lethargic.");
#endif
                potion_effect(POT_SLOWING, 15);
                break;
            case 2:
                // josh declares mummies cannot smell {dlb}
                if (you.species != SP_MUMMY)
                {
#ifdef JP
                    mpr("당신은 지독한 부패의 냄새를 맡았다."); // identical to a harmless message
#else
                    mpr("You smell decay."); // identical to a harmless message
#endif
                    you.rotting++;
                }
                break;
            }
            break;

        case 2:         // much nastier
            switch (random2(3))
            {
            case 0:
#ifdef JP
                mpr("흔들리는 그림자가 당신을 둘러쌌다.");
#else
                mpr("Flickering shadows surround you.");
#endif

                create_monster( MONS_SHADOW, ENCH_ABJ_II, BEH_HOSTILE,
                                you.x_pos, you.y_pos, MHITYOU, 250 );

                if (coinflip())
                {
                    create_monster( MONS_SHADOW, ENCH_ABJ_II, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250 );
                }

                if (coinflip())
                {
                    create_monster( MONS_SHADOW, ENCH_ABJ_II, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250 );
                }
                break;

            case 1:
                if (!player_prot_life() && one_chance_in(3))
                {
                    drain_exp();
                    break;
                }               // otherwise it just flows through...

            case 2:
                if (you.is_undead)
                {
#ifdef JP
                    mpr("당신은 잠깐 동안 기묘한 기분을 느꼈다.");
#else
                    mpr("You feel weird for a moment.");
#endif
                    break;
                }
#ifdef JP
                mpr("전신을 고통으로 찢어발겨지며, 당신은 힘없이 경련했다!");
#else
                mpr("You convulse helplessly as pain tears through your body!");
#endif
                ouch(15 + random2avg(23, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            }
            break;

        case 3:         // even nastier
            switch (random2(6))
            {
            case 0:
                if (you.is_undead)
                {
#ifdef JP
                    mpr("무언가가 지금 당신의 무덤 위를 걸어갔다. 아니, 정말이라니까!");
#else
                    mpr("Something just walked over your grave. No, really!");
#endif
                    break;
                }
#ifdef JP
                mpr("당신의 몸은 고통으로 경련했다!");
#else
                mpr("Your body is wracked with pain!");
#endif

                dec_hp((you.hp / 2) - 1, false);
                break;

            case 1:
#ifdef JP
                mpr("당신은 음의 에너지로 뒤덮였다!");
#else
                mpr("You are engulfed in negative energy!");
#endif

                if (!player_prot_life())
                {
                    drain_exp();
                    break;
                }               // otherwise it just flows through...

            case 2:
                lose_stat(STAT_RANDOM, 1 + random2avg(7, 2));
                break;

            case 3:
                if (you.is_undead)
                {
#ifdef JP
                    mpr("당신은 끔찍한 기분이 들었다.");
#else
                    mpr("You feel terrible.");
#endif
                    break;
                }

                rot_player( random2avg(7, 2) + 1 );
                break;

            case 4:
                if (create_monster( MONS_SOUL_EATER, ENCH_ABJ_IV, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250) != -1)
                {
#ifdef JP
                    mpr("누군가가 당신을 향해 기어 나왔다...");
#else
                    mpr("Something reaches out for you...");
#endif
                }
                break;

            case 5:
                if (create_monster( MONS_REAPER, ENCH_ABJ_IV, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250) != -1)
                {
#ifdef JP
                    mpr("사신이 당신을 맞아들이러 왔다...");
#else
                    mpr("Death has come for you...");
#endif
                }
                break;
            }
            break;
        }
        break;                  // end necromancy

    case SPTYP_TRANSMIGRATION:
        switch (spec_effect)
        {
        case 0:         // just a harmless message
            switch (random2(10))
            {
            case 0:
#ifdef JP
                snprintf( info, INFO_SIZE, "당신의 %s은(는) 잠깐 빛났다.",
#else
                snprintf( info, INFO_SIZE, "Your %s glow momentarily.",
#endif
                          your_hand(true));
                mpr(info);
                break;
            case 1:
#ifdef JP
                mpr("당신 주위의 대기가 에너지로 파직파직 튀었다!");
#else
                mpr("The air around you crackles with energy!");
#endif
                break;
            case 2:
#ifdef JP
                mpr("형형색색의 빛이 당신의 눈앞에서 춤추었다!");
#else
                mpr("Multicolored lights dance before your eyes!");
#endif
                break;
            case 3:
#ifdef JP
                mpr("당신은 기묘한 에너지의 고조를 느꼈다!");
#else
                mpr("You feel a strange surge of energy!");
#endif
                break;
            case 4:
#ifdef JP
                mpr("빛의 파동이 당신의 몸 위로 퍼져 나갔다.");
#else
                mpr("Waves of light ripple over your body.");
#endif
                break;
            case 5:
#ifdef JP
                mpr("기묘한 에너지가 당신의 몸을 질주해 나갔다.");
#else
                mpr("Strange energies run through your body.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("당신은 피부가 따끔따끔하다.");
#else
                mpr("Your skin tingles.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("당신의 피부는 잠깐 빛났다.");
#else
                mpr("Your skin glows momentarily.");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
                // mummies cannot smell
                if (you.species != SP_MUMMY)
#ifdef JP
                    mpr("당신은 무언가 기묘한 냄새를 맡았다.");
#else
                    mpr("You smell something strange.");
#endif
                break;
            }
            break;

        case 1:         // slightly annoying
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("당신의 육체는 고통스럽게 비틀렸다.");
#else
                mpr("Your body is twisted painfully.");
#endif
                ouch(1 + random2avg(11, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            case 1:
                random_uselessness(2 + random2(7), 0);
                break;
            }
            break;

        case 2:         // much more annoying
            switch (random2(4))
            {
            case 0:
#ifdef JP
                mpr("당신의 육체는 극히 고통스럽게 비틀렸다!");
#else
                mpr("Your body is twisted very painfully!");
#endif
                ouch(3 + random2avg(23, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            case 1:
#ifdef JP
                mpr("당신은 제어할 수 없는 에너지의 침투를 느꼈다!");
#else
                mpr("You feel saturated with unharnessed energies!");
#endif
                you.magic_contamination += random2avg(19,3);
                break;
            case 2:
                potion_effect(POT_PARALYSIS, 10);
                break;
            case 3:
                potion_effect(POT_CONFUSION, 10);
                break;
            }
            break;

        case 3:         // even nastier

            switch (random2(3))
            {
            case 0:
#ifdef JP
                mpr("당신의 육체는 왜곡의 에너지로 넘쳐흘렀다!");
#else
                mpr("Your body is flooded with distortional energies!");
#endif
                you.magic_contamination += random2avg(35, 3);

                ouch(3 + random2avg(18, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;

            case 1:
#ifdef JP
                mpr("당신은 매우 기묘한 기분이 들었다.");
#else
                mpr("You feel very strange.");
#endif
                delete_mutation(100);
                ouch(5 + random2avg(23, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;

            case 2:
#ifdef JP
                mpr("당신의 육체는 기괴하고 무시무시한 모양으로 일그러졌다!");
#else
                mpr("Your body is distorted in a weirdly horrible way!");
#endif
                failMsg = !give_bad_mutation();
                if (coinflip())
                    give_bad_mutation(false, failMsg);

                ouch(5 + random2avg(23, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            }
            break;
        }
        break;                  // end transmigrations

    case SPTYP_FIRE:
        switch (spec_effect)
        {
        case 0:         // just a harmless message
            switch (random2(10))
            {
            case 0:
#ifdef JP
                snprintf( info, INFO_SIZE, "당신의 %s에서 불꽃이 튀었다!",
#else
                snprintf( info, INFO_SIZE, "Sparks fly from your %s!",
#endif
                          your_hand(true));
                mpr(info);
                break;
            case 1:
#ifdef JP
                mpr("당신 주위의 대기가 에너지로 타올랐다!");
#else
                mpr("The air around you burns with energy!");
#endif
                break;
            case 2:
#ifdef JP
                snprintf( info, INFO_SIZE, "당신의 %s에서 한 줄기의 연기가 피어 올랐다.",
#else
                snprintf( info, INFO_SIZE, "Wisps of smoke drift from your %s.",
#endif
                          your_hand(true));
                mpr(info);
                break;
            case 3:
#ifdef JP
                mpr("당신은 기묘한 에너지의 고조를 느꼈다!");
#else
                mpr("You feel a strange surge of energy!");
#endif
                break;
            case 4:
                // mummies cannot smell
                if (you.species != SP_MUMMY)
#ifdef JP
                    mpr("당신은 연기 냄새를 맡았다.");
#else
                    mpr("You smell smoke.");
#endif
                break;
            case 5:
#ifdef JP
                mpr("열기가 당신의 몸을 질주해 나갔다.");
#else
                mpr("Heat runs through your body.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("당신은 불쾌한 더위를 느꼈다.");
#else
                mpr("You feel uncomfortably hot.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("미적지근한 불꽃이 당신의 몸 위로 흘러 나갔다.");
#else
                mpr("Lukewarm flames ripple over your body.");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
                if (!silenced(you.x_pos, you.y_pos))
#ifdef JP
                    mpr("당신은 지글지글거리는 소리를 들었다.");
#else
                    mpr("You hear a sizzling sound.");
#endif
                else
#ifdef JP
                    mpr("당신은 가슴 쓰림을 느꼈다.");
#else
                    mpr("You feel like you have heartburn.");
#endif
                break;
            }
            break;

        case 1:         // a bit less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                snprintf( info, INFO_SIZE, "연기가 당신의 %s에서 피어 올랐다!",
#else
                snprintf( info, INFO_SIZE, "Smoke pours from your %s!",
#endif
                          your_hand(true) );
                mpr(info);

                big_cloud( CLOUD_GREY_SMOKE + random2(3),
                           you.x_pos, you.y_pos, 20, 7 + random2(7) );
                break;

            case 1:
#ifdef JP
                mpr("불꽃이 당신의 살을 태웠다.");
#else
                mpr("Flames sear your flesh.");
#endif
                scrolls_burn(3, OBJ_SCROLLS);

                if (player_res_fire() < 0)
                {
                    ouch(2 + random2avg(13, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                }
                break;
            }
            break;

        case 2:         // rather less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("당신은 화염 폭풍에 휩쓸렸다.");
#else
                mpr("You are blasted with fire.");
#endif

                ouch( check_your_resists( 5 + random2avg(29, 2), 2 ), 0,
                      KILLED_BY_WILD_MAGIC, cause );

                scrolls_burn( 5, OBJ_SCROLLS );
                break;

            case 1:
#ifdef JP
                mpr("당신은 불꽃의 폭발에 휩쓸렸다!");
#else
                mpr("You are caught a fiery explosion!");
#endif

                beam.type = SYM_BURST;
                beam.damage = dice_def( 3, 14 );
                beam.flavour = BEAM_FIRE;
                beam.target_x = you.x_pos;
                beam.target_y = you.y_pos;
#ifdef JP
                strcpy(beam.beam_name, "폭발");
#else
                strcpy(beam.beam_name, "explosion");
#endif
                beam.colour = RED;
                beam.beam_source = NON_MONSTER;
                beam.thrower = (cause) ? KILL_MISC : KILL_YOU;
                beam.aux_source = cause;
                beam.ex_size = 1;

                explosion(beam);
                break;
            }
            break;

        case 3:         // considerably less harmless stuff
            switch (random2(3))
            {
            case 0:
#ifdef JP
                mpr("당신은 타오르는 듯한 불꽃에 맞았다!");
#else
                mpr("You are blasted with searing flames!");
#endif

                ouch( check_your_resists( 9 + random2avg(33, 2), 2 ), 0,
                      KILLED_BY_WILD_MAGIC, cause );

                scrolls_burn( 10, OBJ_SCROLLS );
                break;
            case 1:
#ifdef JP
                mpr("갑자기 맹렬한 불꽃의 폭발이 일어났다!");
#else
                mpr("There is a sudden and violent explosion of flames!");
#endif

                beam.type = SYM_BURST;
                beam.damage = dice_def( 3, 20 );
                beam.flavour = BEAM_FIRE;
                beam.target_x = you.x_pos;
                beam.target_y = you.y_pos;
#ifdef JP
                strcpy( beam.beam_name, "화염구" );
#else
                strcpy( beam.beam_name, "fireball" );
#endif
                beam.colour = RED;
                beam.beam_source = NON_MONSTER;
                beam.thrower = (cause) ? KILL_MISC : KILL_YOU;
                beam.aux_source = cause;
                beam.ex_size = coinflip()?1:2;

                explosion(beam);
                break;

            case 2:
#ifdef JP
                mpr("당신은 끓어오르는 액체에 맞았다!");
#else
                mpr("You are covered in liquid fire!");
#endif
                you.duration[DUR_LIQUID_FLAMES] += random2avg(7, 3) + 1;
                break;
            }
            break;
        }
        break;                  // end fire

    case SPTYP_ICE:
        switch (spec_effect)
        {
        case 0:         // just a harmless message
            switch (random2(10))
            {
            case 0:
#ifdef JP
                mpr("당신은 추위에 떨었다.");
#else
                mpr("You shiver with cold.");
#endif
                break;
            case 1:
#ifdef JP
                mpr("냉기가 당신의 몸을 질주해 나갔다.");
#else
                mpr("A chill runs through your body.");
#endif
                break;
            case 2:
#ifdef JP
                snprintf( info, INFO_SIZE, "당신의 %s에서 이슬이 떨어졌다.",
#else
                snprintf( info, INFO_SIZE, "Wisps of condensation drift from your %s.",
#endif
                          your_hand(true));
                mpr(info);
                break;
            case 3:
#ifdef JP
                mpr("당신은 기묘한 에너지의 고조를 느꼈다!");
#else
                mpr("You feel a strange surge of energy!");
#endif
                break;
            case 4:
#ifdef JP
                snprintf( info, INFO_SIZE,"당신의 %s은(는) 추위로 마비되었다.",
#else
                snprintf( info, INFO_SIZE,"Your %s feel numb with cold.",
#endif
                          your_hand(true));
                mpr(info);
                break;
            case 5:
#ifdef JP
                mpr("냉기가 당신의 몸을 질주해 나갔다.");
#else
                mpr("A chill runs through your body.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("당신은 불쾌한 추위를 느꼈다.");
#else
                mpr("You feel uncomfortably cold.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("서리가 당신의 몸을 뒤덮었다.");
#else
                mpr("Frost covers your body.");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
                if (!silenced(you.x_pos, you.y_pos))
#ifdef JP
                    mpr("당신은 파삭파삭 소리를 들었다.");
#else
                    mpr("You hear a crackling sound.");
#endif
                else
#ifdef JP
                    mpr("하나의 눈송이가 당신의 코 위에 내려왔다.");
#else
                    mpr("A snowflake lands on your nose.");
#endif
                break;
            }
            break;

        case 1:         // a bit less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("당신은 극심한 추위를 느꼈다.");
#else
                mpr("You feel extremely cold.");
#endif
                break;
            case 1:
#ifdef JP
                mpr("당신은 얇은 얼음층으로 뒤덮혀 버렸다.");
#else
                mpr("You are covered in a thin layer of ice");
#endif
                scrolls_burn(2, OBJ_POTIONS);

                if (player_res_cold() < 0)
                    ouch(4 + random2avg(5, 2), 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            }
            break;

        case 2:         // rather less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("당신의 몸에서 열을 빼앗겼다.");
#else
                mpr("Heat is drained from your body.");
#endif

                ouch(check_your_resists(5 + random2(6) + random2(7), 3), 0,
                     KILLED_BY_WILD_MAGIC, cause);

                scrolls_burn(4, OBJ_POTIONS);
                break;

            case 1:
#ifdef JP
                mpr("당신은 얼음과 서리의 폭발에 휩쓸렸다.");
#else
                mpr("You are caught in an explosion of ice and frost!");
#endif

                beam.type = SYM_BURST;
                beam.damage = dice_def( 3, 11 );
                beam.flavour = BEAM_COLD;
                beam.target_x = you.x_pos;
                beam.target_y = you.y_pos;
#ifdef JP
                strcpy(beam.beam_name, "폭발");
#else
                strcpy(beam.beam_name, "explosion");
#endif
                beam.colour = WHITE;
                beam.beam_source = NON_MONSTER;
                beam.thrower = (cause) ? KILL_MISC : KILL_YOU;
                beam.aux_source = cause;
                beam.ex_size = 1;

                explosion(beam);
                break;
            }
            break;

        case 3:         // less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("당신은 얼음에 맞았다!");
#else
                mpr("You are blasted with ice!");
#endif

                ouch(check_your_resists(9 + random2avg(23, 2), 3), 0,
                     KILLED_BY_WILD_MAGIC, cause);

                scrolls_burn(9, OBJ_POTIONS);
                break;
            case 1:
#ifdef JP
                snprintf( info, INFO_SIZE,"당신의 %s에서 냉동 가스가 새어 나왔다!",
#else
                snprintf( info, INFO_SIZE,"Freezing gasses pour from your %s!",
#endif
                          your_hand(true));
                mpr(info);

                big_cloud(CLOUD_COLD, you.x_pos, you.y_pos, 20,
                          8 + random2(4));
                break;
            }
            break;
        }
        break;                  // end ice

    case SPTYP_EARTH:
        switch (spec_effect)
        {
        case 0:         // just a harmless message
        case 1:
            switch (random2(10))
            {
            case 0:
#ifdef JP
                mpr("당신은 흙냄새를 맡았다.");
#else
                mpr("You feel earthy.");
#endif
                break;
            case 1:
#ifdef JP
                mpr("당신은 모래의 미립자에 휩쓸렸다.");
#else
                mpr("You are showered with tiny particles of grit.");
#endif
                break;
            case 2:
#ifdef JP
                snprintf( info, INFO_SIZE,"당신의 %s에서 모래가 흘러 나왔다.",
#else
                snprintf( info, INFO_SIZE,"Sand pours from your %s.",
#endif
                          your_hand(true));
                mpr(info);
                break;
            case 3:
#ifdef JP
                mpr("당신은 대지에서의 에너지의 고양을 느꼈다.");
#else
                mpr("You feel a surge of energy from the ground.");
#endif
                break;
            case 4:
                if (!silenced(you.x_pos, you.y_pos))
#ifdef JP
                    mpr("당신은 멀리에서의 대지의 울림을 들었다.");
#else
                    mpr("You hear a distant rumble.");
#endif
                else
#ifdef JP
                    mpr("당신은 돌멩이와 교감했다.");
#else
                    mpr("You sympathise with the stones.");
#endif
                break;
            case 5:
#ifdef JP
                mpr("당신은 용기가 끓어올랐다.");
#else
                mpr("You feel gritty.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("당신은 잠깐 무력감을 느꼈다.");
#else
                mpr("You feel momentarily lethargic.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("모래 먼지가 당신의 눈앞에서 빙글빙글 춤추었다.");
#else
                mpr("Motes of dust swirl before your eyes.");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
#ifdef JP
                strcpy(info, "당신의 ");
#else
                strcpy(info, "Your ");
#endif
#ifdef JP
                strcat(info, (you.species == SP_NAGA)    ? "하반신" :
                             (you.species == SP_CENTAUR) ? "발굽"
                                                         : "발");
                strcat(info, "은(는) 대지의 온기를 느꼈다.");
#else
                strcat(info, (you.species == SP_NAGA)    ? "underbelly feels" :
                             (you.species == SP_CENTAUR) ? "hooves feel"
                                                         : "feet feel");
                strcat(info, " warm.");
#endif
                mpr(info);
                break;
            }
            break;

        case 2:         // slightly less harmless stuff
            switch (random2(1))
            {
            case 0:
                switch (random2(3))
                {
                case 0:
#ifdef JP
                    mpr("당신은 날아온 바위에 부딪쳤다!");
#else
                    mpr("You are hit by flying rocks!");
#endif
                    break;
                case 1:
#ifdef JP
                    mpr("당신은 모래 폭풍에 휩쓸렸다!");
#else
                    mpr("You are blasted with sand!");
#endif
                    break;
                case 2:
#ifdef JP
                    mpr("어디에서인가 바위가 당신에게 떨어져 왔다!");
#else
                    mpr("Rocks fall onto you out of nowhere!");
#endif
                    break;
                }

                hurted = random2avg(13, 2) + 10;
                hurted -= random2(1 + player_AC());

                ouch(hurted, 0, KILLED_BY_WILD_MAGIC, cause);
                break;
            }
            break;

        case 3:         // less harmless stuff
            switch (random2(1))
            {
            case 0:
#ifdef JP
                mpr("당신은 어지럽게 날아다니는 파편의 폭발에 휩쓸렸다!");
#else
                mpr("You are caught in an explosion of flying shrapnel!");
#endif

                beam.type = SYM_BURST;
                beam.damage = dice_def( 3, 15 );
                beam.flavour = BEAM_FRAG;
                beam.target_x = you.x_pos;
                beam.target_y = you.y_pos;
#ifdef JP
                strcpy(beam.beam_name, "폭발");
#else
                strcpy(beam.beam_name, "explosion");
#endif
                beam.colour = CYAN;

                if (one_chance_in(5))
                    beam.colour = BROWN;
                if (one_chance_in(5))
                    beam.colour = LIGHTCYAN;

                beam.beam_source = NON_MONSTER;
                beam.thrower = (cause) ? KILL_MISC : KILL_YOU;
                beam.aux_source = cause;
                beam.ex_size = 1;

                explosion(beam);
                break;
            }
            break;
        }
        break;                  // end Earth

    case SPTYP_AIR:
        switch (spec_effect)
        {
        case 0:         // just a harmless message
            switch (random2(10))
            {
            case 0:
#ifdef JP
                mpr("아야! 당신은 자신에게 전기 쇼크를 주어 버렸다.");
#else
                mpr("Ouch! You gave yourself an electric shock.");
#endif
                break;
            case 1:
#ifdef JP
                mpr("당신은 잠깐 동안 무중력을 느꼈다.");
#else
                mpr("You feel momentarily weightless.");
#endif
                break;
            case 2:
#ifdef JP
                snprintf( info, INFO_SIZE, "증기가 당신의 %s에서 뿜어져 나왔다.",
#else
                snprintf( info, INFO_SIZE, "Wisps of vapour drift from your %s.",
#endif
                          your_hand(true));
                mpr(info);
                break;
            case 3:
#ifdef JP
                mpr("당신은 기묘한 에너지의 고조를 느꼈다!");
#else
                mpr("You feel a strange surge of energy!");
#endif
                break;
            case 4:
#ifdef JP
                mpr("당신은 흥분되는 것을 느꼈다!");
#else
                mpr("You feel electric!");
#endif
                break;
            case 5:
#ifdef JP
                snprintf( info, INFO_SIZE, "당신의 %s 사이에서 전기불꽃이 춤추었다.",
#else
                snprintf( info, INFO_SIZE, "Sparks of electricity dance between your %s.",
#endif
                          your_hand(true));
                mpr(info);
                break;
            case 6:
#ifdef JP
                mpr("당신은 폭풍에 휩쓸렸다!");
#else
                mpr("You are blasted with air!");
#endif
                break;
            case 7:
                // mummies cannot smell
                if (!silenced(you.x_pos, you.y_pos))
#ifdef JP
                    mpr("당신은 휭휭 소리를 들었다.");
#else
                    mpr("You hear a whooshing sound.");
#endif
                else if (you.species != SP_MUMMY)
#ifdef JP
                    mpr("당신은 오존 냄새를 맡았다.");
#else
                    mpr("You smell ozone.");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
                // mummies cannot smell
                if (!silenced(you.x_pos, you.y_pos))
#ifdef JP
                    mpr("당신은 파삭파삭 소리를 들었다.");
#else
                    mpr("You hear a crackling sound.");
#endif
                else if (you.species != SP_MUMMY)
#ifdef JP
                    mpr("당신은 무언가 케케묵은 냄새를 맡았다.");
#else
                    mpr("You smell something musty.");
#endif
                break;
            }
            break;

        case 1:         // a bit less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("짧고 급격한 불꽃이 분수처럼 쏟아져 내렸다.");
#else
                mpr("There is a short, sharp shower of sparks.");
#endif
                break;
            case 1:
#ifdef JP
                snprintf( info, INFO_SIZE, "당신 주위에서 바람이 %s!",
                         silenced(you.x_pos, you.y_pos) ? "펄럭거렸다" : "울부짖었다");
#else
                snprintf( info, INFO_SIZE, "The wind %s around you!",
                         silenced(you.x_pos, you.y_pos) ? "whips" : "howls");
#endif
                mpr(info);
                break;
            }
            break;

        case 2:         // rather less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("전기가 당신의 몸에 뜨겁게 흘렀다.");
#else
                mpr("Electricity courses through your body.");
#endif
                ouch(check_your_resists(4 + random2avg(9, 2), 5), 0,
                     KILLED_BY_WILD_MAGIC, cause);
                break;
            case 1:
#ifdef JP
                snprintf( info, INFO_SIZE, "유독 가스가 당신의 %s에서 새어 나왔다!",
#else
                snprintf( info, INFO_SIZE, "Noxious gasses pour from your %s!",
#endif
                          your_hand(true));
                mpr(info);

                big_cloud(CLOUD_STINK, you.x_pos, you.y_pos, 20,
                          9 + random2(4));
                break;
            }
            break;

        case 3:         // less harmless stuff
            switch (random2(2))
            {
            case 0:
#ifdef JP
                mpr("당신은 방전의 폭발에 휩쓸렸다!");
#else
                mpr("You are caught in an explosion of electrical discharges!");
#endif

                beam.type = SYM_BURST;
                beam.damage = dice_def( 3, 8 );
                beam.flavour = BEAM_ELECTRICITY;
                beam.target_x = you.x_pos;
                beam.target_y = you.y_pos;
#ifdef JP
                strcpy(beam.beam_name, "폭발");
#else
                strcpy(beam.beam_name, "explosion");
#endif
                beam.colour = LIGHTBLUE;
                beam.beam_source = NON_MONSTER;
                beam.thrower = (cause) ? KILL_MISC : KILL_YOU;
                beam.aux_source = cause;
                beam.ex_size = one_chance_in(4)?1:2;

                explosion(beam);
                break;
            case 1:
#ifdef JP
                snprintf( info, INFO_SIZE, "유독 가스가 당신의 %s에서 새어 나왔다!",
#else
                snprintf( info, INFO_SIZE, "Venomous gasses pour from your %s!",
#endif
                          your_hand(true));
                mpr(info);

                big_cloud( CLOUD_POISON, you.x_pos, you.y_pos, 20,
                           8 + random2(5) );
                break;
            }
            break;
        }
        break;                  // end air

    case SPTYP_POISON:
        switch (spec_effect)
        {
        case 0:         // just a harmless message
            switch (random2(10))
            {
            case 0:
#ifdef JP
                mpr("당신은 조금 구역질이 났다.");
#else
                mpr("You feel mildly nauseous.");
#endif
                break;
            case 1:
#ifdef JP
                mpr("당신은 가벼운 구역질이 났다.");
#else
                mpr("You feel slightly ill.");
#endif
                break;
            case 2:
#ifdef JP
                snprintf( info, INFO_SIZE, "독가스가 당신의 %s에서 떠돌아다녔다.",
#else
                snprintf( info, INFO_SIZE, "Wisps of poison gas drift from your %s.",
#endif
                          your_hand(true) );
                mpr(info);
                break;
            case 3:
#ifdef JP
                mpr("당신은 기묘한 에너지의 고조를 느꼈다!");
#else
                mpr("You feel a strange surge of energy!");
#endif
                break;
            case 4:
#ifdef JP
                mpr("당신은 잠깐 현기증이 났다.");
#else
                mpr("You feel faint for a moment.");
#endif
                break;
            case 5:
#ifdef JP
                mpr("당신은 구역질이 났다.");
#else
                mpr("You feel sick.");
#endif
                break;
            case 6:
#ifdef JP
                mpr("당신은 기묘한 기분이 들었다.");
#else
                mpr("You feel odd.");
#endif
                break;
            case 7:
#ifdef JP
                mpr("당신은 잠깐 쇠약함을 느꼈다.");
#else
                mpr("You feel weak for a moment.");
#endif
                break;
            case 8:
                canned_msg(MSG_NOTHING_HAPPENS);
                break;
            case 9:
                if (!silenced(you.x_pos, you.y_pos))
#ifdef JP
                    mpr("당신은 벌컥벌컥 소리를 들었다.");
#else
                    mpr("You hear a slurping sound.");
#endif
                else if (you.species != SP_MUMMY)
#ifdef JP
                    mpr("당신은 아몬드 맛을 느꼈다.");
#else
                    mpr("You taste almonds.");
#endif
                break;
            }
            break;

        case 1:         // a bit less harmless stuff
            switch (random2(2))
            {
            case 0:
                if (player_res_poison())
                {
                    canned_msg(MSG_NOTHING_HAPPENS);
                    return (false);
                }

#ifdef JP
                mpr("당신은 몸 상태가 좋지 않다.");
#else
                mpr("You feel sick.");
#endif
                poison_player( 2 + random2(3) );
                break;

            case 1:
#ifdef JP
                snprintf( info, INFO_SIZE, "유독 가스가 당신의 %s에서 새어 나왔다!",
#else
                snprintf( info, INFO_SIZE, "Noxious gasses pour from your %s!",
#endif
                          your_hand(true) );
                mpr(info);

                place_cloud(CLOUD_STINK, you.x_pos, you.y_pos,
                            2 + random2(4));
                break;
            }
            break;

        case 2:         // rather less harmless stuff
            switch (random2(3))
            {
            case 0:
                if (player_res_poison())
                {
                    canned_msg(MSG_NOTHING_HAPPENS);
                    return (false);
                }

#ifdef JP
                mpr("당신은 심하게 몸 상태가 좋지 않다.");
#else
                mpr("You feel very sick.");
#endif
                poison_player( 3 + random2avg(9, 2) );
                break;

            case 1:
#ifdef JP
                mpr("유독 가스가 당신의 손에서 새어 나왔다!");
#else
                mpr("Noxious gasses pour from your hands!");
#endif
                big_cloud(CLOUD_STINK, you.x_pos, you.y_pos, 20,
                          8 + random2(5));
                break;

            case 2:
                if (player_res_poison())
                {
                    canned_msg(MSG_NOTHING_HAPPENS);
                    return (false);
                }
                lose_stat(STAT_RANDOM, 1);
                break;
            }
            break;

        case 3:         // less harmless stuff
            switch (random2(3))
            {
            case 0:
                if (player_res_poison())
                {
                    canned_msg(MSG_NOTHING_HAPPENS);
                    return (false);
                }

#ifdef JP
                mpr("당신은 탈진할 정도로 몸 상태가 좋지 않다.");
#else
                mpr("You feel incredibly sick.");
#endif
                poison_player( 10 + random2avg(19, 2) );
                break;
            case 1:
#ifdef JP
                snprintf( info, INFO_SIZE, "맹독 가스가 당신의 %s에서 새어 나왔다!",
#else
                snprintf( info, INFO_SIZE, "Venomous gasses pour from your %s!",
#endif
                          your_hand(true));
                mpr(info);

                big_cloud(CLOUD_POISON, you.x_pos, you.y_pos, 20,
                          7 + random2(7));
                break;
            case 2:
                if (player_res_poison())
                {
                    canned_msg(MSG_NOTHING_HAPPENS);
                    return (false);
                }

                lose_stat(STAT_RANDOM, 1 + random2avg(5, 2));
                break;
            }
            break;
        }
        break;                  // end poison
    }

    return (true);
}                               // end miscast_effect()
