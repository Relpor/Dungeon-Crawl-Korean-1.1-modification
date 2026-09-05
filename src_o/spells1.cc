/*
 *  File:       spells1.cc
 *  Summary:    Implementations of some additional spells.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <4>      06-mar-2000    bwr             confusing_touch, sure_blade
 *      <3>      9/11/99        LRH             Can't blink in the Abyss
 *      <3>      6/22/99        BWR             Removed teleport control from
 *                                              random_blink().
 *      <2>      5/20/99        BWR             Increased greatest healing.
 *      <1>      -/--/--        LRH             Created
 */

#include "AppHdr.h"
#include "spells1.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "externs.h"

#include "abyss.h"
#include "beam.h"
#include "cloud.h"
#include "direct.h"
#include "invent.h"
#include "it_use2.h"
#include "itemname.h"
#include "misc.h"
#include "monplace.h"
#include "monstuff.h"
#include "mon-util.h"
#include "player.h"
#include "skills2.h"
#include "spells3.h"
#include "spells4.h"
#include "spl-util.h"
#include "stuff.h"
#include "view.h"
#include "wpn-misc.h"

void blink(void)
{
    struct dist beam;

    // yes, there is a logic to this ordering {dlb}:
    if (scan_randarts(RAP_PREVENT_TELEPORTATION))
#ifdef JP
        mpr("당신은 기묘한 정체감을 느꼈다.");
#else
        mpr("You feel a weird sense of stasis.");
#endif
    else if (you.level_type == LEVEL_ABYSS && !one_chance_in(3))
#ifdef JP
        mpr("어비스의 힘이 당신을 그 자리에서 떼어놓지 않는다!");
#else
        mpr("The power of the Abyss keeps you in your place!");
#endif
    else if (you.conf)
        random_blink(false);
    else if (!allow_control_teleport(true))
    {
#ifdef JP
        mpr("강력한 마법이 당신의 순간이동 제어를 방해했다.");
#else
        mpr("A powerful magic interferes with your control of the blink.");
#endif
        random_blink(false);
    }
    else
    {
        // query for location {dlb}:
        for (;;)
        {
#ifdef JP
            mpr("어디로 순간이동합니까?", MSGCH_PROMPT);
#else
            mpr("Blink to where?", MSGCH_PROMPT);
#endif

            direction( beam, DIR_TARGET );

            if (!beam.isValid)
            {
                canned_msg(MSG_SPELL_FIZZLES);
                return;         // early return {dlb}
            }

            if (see_grid(beam.tx, beam.ty)
                && ( (grd[beam.tx][beam.ty] <= DNGN_LAST_SOLID_TILE)
                    || (grd[beam.tx][beam.ty] >= DNGN_SHALLOW_WATER) ) )
                break;
            else
            {
                mesclr();
#ifdef JP
                mpr("당신은 그곳으로는 순간이동할 수 없다!");
#else
                mpr("You can't blink there!");
#endif
            }
        }

        if (grd[beam.tx][beam.ty] <= DNGN_LAST_SOLID_TILE
            || mgrd[beam.tx][beam.ty] != NON_MONSTER)
        {
#ifdef JP
            mpr("우왓! 아무래도 그곳에는 무언가가 있는 것 같다.");
#else
            mpr("Oops! Maybe something was there already.");
#endif
            random_blink(false);
        }
        else if (you.level_type == LEVEL_ABYSS)
        {
            abyss_teleport( false );
            you.pet_target = MHITNOT;
        }
        else
        {
            you.x_pos = beam.tx;
            you.y_pos = beam.ty;

            // controlling teleport contaminates the player -- bwr
            contaminate_player( 1 );
        }

        if (you.duration[DUR_CONDENSATION_SHIELD] > 0)
        {
            you.duration[DUR_CONDENSATION_SHIELD] = 0;
            you.redraw_armour_class = 1;
        }

        // swimming
        if (!player_is_levitating()
            && (grd[you.x_pos][you.y_pos] == DNGN_LAVA
                || grd[you.x_pos][you.y_pos] == DNGN_DEEP_WATER
                || grd[you.x_pos][you.y_pos] == DNGN_SHALLOW_WATER))
        {
            if (you.species == SP_MERFOLK
                && grd[you.x_pos][you.y_pos] != DNGN_LAVA)
            {
#ifdef JP
                mpr("당신은 물속으로 뛰어들어서, 본래의 형태로 변화했다.");
#else
                mpr("You dive into the water and return to your normal form.");
#endif
                merfolk_start_swimming();
            }

            if (grd[you.x_pos][you.y_pos] != DNGN_SHALLOW_WATER)
                fall_into_a_pool( true, grd[you.x_pos][you.y_pos] );
        }
    }

    return;
}                               // end blink()

void random_blink(bool allow_partial_control)
{
    int tx, ty;
    bool succ = false;

    if (scan_randarts(RAP_PREVENT_TELEPORTATION))
#ifdef JP
        mpr("당신은 기묘한 정체감을 느꼈다.");
#else
        mpr("You feel a weird sense of stasis.");
#endif
    else if (you.level_type == LEVEL_ABYSS && !one_chance_in(3))
    {
#ifdef JP
        mpr("어비스의 힘이 당신을 그 자리에서 떼어놓지 않는다!");
#else
        mpr("The power of the Abyss keeps you in your place!");
#endif
    }
    else if (!random_near_space(you.x_pos, you.y_pos, tx, ty))
    {
#ifdef JP
        mpr("당신은 잠깐 초조함을 느꼈다.");
#else
        mpr("You feel jittery for a moment.");
#endif
    }

#ifdef USE_SEMI_CONTROLLED_BLINK
    //jmf: add back control, but effect is cast_semi_controlled_blink(pow)
    else if (you.attribute[ATTR_CONTROL_TELEPORT] && !you.conf
             && allow_partial_control && allow_control_teleport())
    {
#ifdef JP
        mpr("당신은 전송할 곳의 방향을 지정할 수 있다.");
#else
        mpr("You may select the general direction of your translocation.");
#endif
        cast_semi_controlled_blink(100);
        succ = true;
    }
#endif

    else
    {
#ifdef JP
        mpr("당신은 순간이동했다.");
#else
        mpr("You blink.");
#endif

        succ = true;
        you.x_pos = tx;
        you.y_pos = ty;

        if (you.level_type == LEVEL_ABYSS)
        {
            abyss_teleport( false );
            you.pet_target = MHITNOT;
        }
    }

    if (succ && you.duration[DUR_CONDENSATION_SHIELD] > 0)
    {
        you.duration[DUR_CONDENSATION_SHIELD] = 0;
        you.redraw_armour_class = 1;
    }

    return;
}                               // end random_blink()

void fireball(int power)
{
    struct dist fire_ball;

    mpr( STD_DIRECTION_PROMPT, MSGCH_PROMPT );

    message_current_target();

    direction( fire_ball, DIR_NONE, TARG_ENEMY );

    if (!fire_ball.isValid)
        canned_msg(MSG_SPELL_FIZZLES);
    else
    {
        struct bolt beam;

        beam.source_x = you.x_pos;
        beam.source_y = you.y_pos;
        beam.target_x = fire_ball.tx;
        beam.target_y = fire_ball.ty;

        zapping(ZAP_FIREBALL, power, beam);
    }

    return;
}                               // end fireball()

void cast_fire_storm(int powc)
{
    struct bolt beam;
    struct dist targ;

#ifdef JP
    mpr("어디를 향해 공격합니까?");
#else
    mpr("Where?");
#endif

    direction( targ, DIR_TARGET, TARG_ENEMY );

    beam.target_x = targ.tx;
    beam.target_y = targ.ty;

    if (!targ.isValid)
    {
        canned_msg(MSG_SPELL_FIZZLES);
        return;
    }

    beam.ex_size = 2 + (random2(powc) > 75);
    beam.flavour = BEAM_LAVA;
    beam.type = SYM_ZAP;
    beam.colour = RED;
    beam.beam_source = MHITYOU;
    beam.thrower = KILL_YOU_MISSILE;
    beam.aux_source = NULL;
    beam.obviousEffect = false;
    beam.isBeam = false;
    beam.isTracer = false;
    beam.ench_power = powc;     // used for radius
#ifdef JP
    strcpy( beam.beam_name, "거대한 화염 폭풍" );
#else
    strcpy( beam.beam_name, "great blast of fire" );
#endif
    beam.hit = 20 + powc / 10;
    beam.damage = calc_dice( 6, 15 + powc );

    explosion( beam );
#ifdef JP
    mpr("맹렬한 화염의 폭풍우가 일어났다!");
#else
    mpr("A raging storm of fire appears!");
#endif

    viewwindow(1, false);
}                               // end cast_fire_storm()

bool identify(int power)
{
    int id_used = 1;
    int item_slot;
    bool identified = false;
    char str_pass[ ITEMNAME_SIZE ];

    // scrolls of identify *may* produce "extra" identifications {dlb}:
    if (power == -1 && one_chance_in(5))
        id_used += (coinflip()? 1 : 2);

    do
    {
#ifdef JP
        item_slot = prompt_invent_item( "어떤 아이템을 감정합니까? ([*][?]로 일람)", -1, true,
#else
        item_slot = prompt_invent_item( "Identify which item?", -1, true,
#endif
                                        false, false );
        if (item_slot == PROMPT_ABORT)
        {
            canned_msg( MSG_OK );
            return (identified);
        }

        if ((you.inv[item_slot].flags & ISFLAG_IDENT_MASK) == ISFLAG_IDENT_MASK)
        {
            canned_msg(MSG_NOTHING_HAPPENS);
            continue;
        }

        set_ident_type( you.inv[item_slot].base_type,
                        you.inv[item_slot].sub_type, ID_KNOWN_TYPE );

        set_ident_flags( you.inv[item_slot], ISFLAG_IDENT_MASK );

        // output identified item
        in_name( item_slot, DESC_INVENTORY_EQUIP, str_pass );
        mpr( str_pass );

        if (item_slot == you.equip[EQ_WEAPON])
            you.wield_change = true;

        identified = true;
        id_used--;
    }
    while (id_used > 0);

    return (identified);
}                               // end identify()

void conjure_flame(int pow)
{
    struct dist spelld;

    bool done_first_message = false;

    for (;;)
    {
        if (done_first_message)
#ifdef JP
            mpr("어디에 발생시킵니까?", MSGCH_PROMPT);
#else
            mpr("Where would you like to place the cloud?", MSGCH_PROMPT);
#endif
        else
        {
#ifdef JP
            mpr("당신은 불꽃의 구름의 주문을 외웠다! 어디에 발생시킵니까?", MSGCH_PROMPT);
#else
            mpr("You cast a flaming cloud spell! But where?", MSGCH_PROMPT);
#endif
            done_first_message = true;
        }

        direction( spelld, DIR_TARGET, TARG_ENEMY );

        if (!spelld.isValid)
        {
            canned_msg(MSG_SPELL_FIZZLES);
            return;
        }

        if (!see_grid(spelld.tx, spelld.ty))
        {
#ifdef JP
            mpr("당신에게는 그 장소가 보이지 않는다!");
#else
            mpr("You can't see that place!");
#endif
            continue;
        }

        if (grd[ spelld.tx ][ spelld.ty ] <= DNGN_LAST_SOLID_TILE
            || mgrd[ spelld.tx ][ spelld.ty ] != NON_MONSTER
            || env.cgrid[ spelld.tx ][ spelld.ty ] != EMPTY_CLOUD)
        {
#ifdef JP
            mpr( "그곳에는 이미 무언가가 있다!" );
#else
            mpr( "There's already something there!" );
#endif
            continue;
        }

        break;
    }

    int durat = 5 + (random2(pow) / 2) + (random2(pow) / 2);

    if (durat > 23)
        durat = 23;

    place_cloud( CLOUD_FIRE, spelld.tx, spelld.ty, durat );
}                               // end cast_conjure_flame()

void stinking_cloud( int pow )
{
    struct dist spelld;
    struct bolt beem;

    mpr( STD_DIRECTION_PROMPT, MSGCH_PROMPT );

    message_current_target();

    direction( spelld, DIR_NONE, TARG_ENEMY );

    if (!spelld.isValid)
    {
        canned_msg(MSG_SPELL_FIZZLES);
        return;
    }

    beem.target_x = spelld.tx;
    beem.target_y = spelld.ty;

    beem.source_x = you.x_pos;
    beem.source_y = you.y_pos;

#ifdef JP
    strcpy(beem.beam_name, "증기의 구");
#else
    strcpy(beem.beam_name, "ball of vapour");
#endif
    beem.colour = GREEN;
    beem.range = 6;
    beem.rangeMax = 6;
    beem.damage = dice_def( 1, 0 );
    beem.hit = 20;
    beem.type = SYM_ZAP;
    beem.flavour = BEAM_MMISSILE;
    beem.ench_power = pow;
    beem.beam_source = MHITYOU;
    beem.thrower = KILL_YOU;
    beem.aux_source = NULL;
    beem.isBeam = false;
    beem.isTracer = false;

    fire_beam(beem);
}                               // end stinking_cloud()

void cast_big_c(int pow, char cty)
{
    struct dist cdis;

#ifdef JP
    mpr("어디에 마법을 발생시킵니까?", MSGCH_PROMPT);
#else
    mpr("Where do you want to put it?", MSGCH_PROMPT);
#endif
    direction( cdis, DIR_TARGET, TARG_ENEMY );

    if (!cdis.isValid)
    {
        canned_msg(MSG_SPELL_FIZZLES);
        return;
    }

    big_cloud( cty, cdis.tx, cdis.ty, pow, 8 + random2(3) );
}                               // end cast_big_c()

void big_cloud(char clouds, char cl_x, char cl_y, int pow, int size)
{
    apply_area_cloud(make_a_normal_cloud, cl_x, cl_y, pow, size, clouds);
}                               // end big_cloud()

static char healing_spell( int healed )
{
    int mgr = 0;
    struct monsters *monster = 0;       // NULL {dlb}
    struct dist bmove;

#ifdef JP
    mpr("어느 방향에?", MSGCH_PROMPT);
#else
    mpr("Which direction?", MSGCH_PROMPT);
#endif
    direction( bmove, DIR_DIR, TARG_FRIEND );

    if (!bmove.isValid)
    {
        canned_msg( MSG_HUH );
        return 0;
    }

    mgr = mgrd[you.x_pos + bmove.dx][you.y_pos + bmove.dy];

    if (bmove.dx == 0 && bmove.dy == 0)
    {
#ifdef JP
        mpr("당신은 회복했다.");
#else
        mpr("You are healed.");
#endif
        inc_hp(healed, false);
        return 1;
    }

    if (mgr == NON_MONSTER)
    {
#ifdef JP
        mpr("그곳에는 아무것도 없다.");
#else
        mpr("There isn't anything there!");
#endif
        return -1;
    }

    monster = &menv[mgr];

    if (heal_monster(monster, healed, false))
    {
#ifdef JP
        strcpy(info, "당신은 ");
#else
        strcpy(info, "You heal ");
#endif
        strcat(info, ptr_monam( monster, DESC_NOCAP_THE ));
#ifdef JP
        strcat(info, "의 체력을 회복했다.");
#else
        strcat(info, ".");
#endif
        mpr(info);

        if (monster->hit_points == monster->max_hit_points)
#ifdef JP
            simple_monster_message( monster, "은(는) 완전히 체력을 회복했다." );
#else
            simple_monster_message( monster, " is completely healed." );
#endif
        else
            print_wounds(monster);
    }
    else
    {
        canned_msg(MSG_NOTHING_HAPPENS);
    }

    return 1;
}                               // end healing_spell()

#if 0
char cast_lesser_healing( int pow )
{
    return healing_spell(5 + random2avg(7, 2));
}                               // end lesser healing()

char cast_greater_healing( int pow )
{
    return healing_spell(15 + random2avg(29, 2));
}                               // end cast_greater_healing()

char cast_greatest_healing( int pow )
{
    return healing_spell(50 + random2avg(49, 2));
}                               // end cast_greatest_healing()
#endif

char cast_healing( int pow )
{
    if (pow > 50)
        pow = 50;

    return (healing_spell( pow + roll_dice( 2, pow ) - 2 ));
}

bool cast_revivification(int power)
{
    int loopy = 0;              // general purpose loop variable {dlb}
    bool success = false;
    int loss = 0;

    if (you.hp == you.hp_max)
        canned_msg(MSG_NOTHING_HAPPENS);
    else if (you.hp_max < 21)
#ifdef JP
        mpr("당신은 이 주문을 외우기에는 체력이 부족하다.");
#else
        mpr("You lack the resilience to cast this spell.");
#endif
    else
    {
#ifdef JP
        mpr("당신의 육체는 무시무시한 고통을 주는 방법으로 치료되었다.");
#else
        mpr("Your body is healed in an amazingly painful way.");
#endif

        loss = 2;
        for (loopy = 0; loopy < 9; loopy++)
        {
            if (random2(power) < 8)
                loss++;
        }

        dec_max_hp( loss );
        set_hp( you.hp_max, false );
        success = true;
    }

    return (success);
}                               // end cast_revivification()

void cast_cure_poison(int mabil)
{
    if (!you.poison)
        canned_msg(MSG_NOTHING_HAPPENS);
    else
        reduce_poison_player( 2 + random2(mabil) + random2(3) );

    return;
}                               // end cast_cure_poison()

void purification(void)
{
#ifdef JP
    mpr("당신의 육체는 정화되었다!");
#else
    mpr("You feel purified!");
#endif

    you.poison = 0;
    you.rotting = 0;
    you.conf = 0;
    you.slow = 0;
    you.disease = 0;
    you.paralysis = 0;          // can't currently happen -- bwr
}                               // end purification()

int allowed_deaths_door_hp(void)
{
    int hp = you.skills[SK_NECROMANCY] / 2;

    if (you.religion == GOD_KIKUBAAQUDGHA && !player_under_penance())
        hp += you.piety / 15;

    return (hp);
}

void cast_deaths_door(int pow)
{
    if (you.is_undead)
#ifdef JP
        mpr("당신은 원래 죽은자다!");
#else
        mpr("You're already dead!");
#endif
    else if (you.deaths_door)
#ifdef JP
        mpr("당신의 연명을 위한 탄원은 거부당했다.");
#else
        mpr("Your appeal for an extension has been denied.");
#endif
    else
    {
#ifdef JP
        mpr("당신은 무적이 된 것 같다!");
        mpr("당신은 모래시계의 모래가 흘러 떨어지는 소리를 들은 듯한 기분이 들었다.");
#else
        mpr("You feel invincible!");
        mpr("You seem to hear sand running through an hourglass...");
#endif

        set_hp( allowed_deaths_door_hp(), false );
        deflate_hp( you.hp_max, false );

        you.deaths_door = 10 + random2avg(13, 3) + (random2(pow) / 10);

        if (you.deaths_door > 25)
            you.deaths_door = 23 + random2(5);
    }

    return;
}

// can't use beam variables here, because of monster_die and the puffs of smoke
void abjuration(int pow)
{
    struct monsters *monster = 0;       // NULL {dlb}

#ifdef JP
    mpr("송환의 주문이 발동했다!");
#else
    mpr("Send 'em back where they came from!");
#endif

    for (int ab = 0; ab < MAX_MONSTERS; ab++)
    {
        monster = &menv[ab];

        int abjLevel;

        if (monster->type == -1 || !mons_near(monster))
            continue;

        if (mons_friendly(monster))
            continue;

        abjLevel = mons_del_ench(monster, ENCH_ABJ_I, ENCH_ABJ_VI);
        if (abjLevel != ENCH_NONE)
        {
            abjLevel -= 1 + (random2(pow) / 8);

            if (abjLevel < ENCH_ABJ_I)
                monster_die(monster, KILL_RESET, 0);
            else
            {
#ifdef JP
                simple_monster_message(monster, "은(는) 몸서리쳤다.");
#else
                simple_monster_message(monster, " shudders.");
#endif
                mons_add_ench(monster, abjLevel);
            }
        }
    }
}                               // end abjuration()

// Antimagic is sort of an anti-extension... it sets a lot of magical
// durations to 1 so it's very nasty at times (and potentially lethal,
// that's why we reduce levitation to 2, so that the player has a chance
// to stop insta-death... sure the others could lead to death, but that's
// not as direct as falling into deep water) -- bwr
void antimagic( void )
{
    if (you.haste)
        you.haste = 1;

    if (you.slow)
        you.slow = 1;

    if (you.paralysis)
        you.paralysis = 1;

    if (you.conf)
        you.conf = 1;

    if (you.might)
        you.might = 1;

    if (you.levitation > 2)
        you.levitation = 2;

    if (you.invis)
        you.invis = 1;

    if (you.duration[DUR_WEAPON_BRAND])
        you.duration[DUR_WEAPON_BRAND] = 1;

    if (you.duration[DUR_ICY_ARMOUR])
        you.duration[DUR_ICY_ARMOUR] = 1;

    if (you.duration[DUR_REPEL_MISSILES])
        you.duration[DUR_REPEL_MISSILES] = 1;

    if (you.duration[DUR_REGENERATION])
        you.duration[DUR_REGENERATION] = 1;

    if (you.duration[DUR_DEFLECT_MISSILES])
        you.duration[DUR_DEFLECT_MISSILES] = 1;

    if (you.fire_shield)
        you.fire_shield = 1;

    if (you.duration[DUR_SWIFTNESS])
        you.duration[DUR_SWIFTNESS] = 1;

    if (you.duration[DUR_INSULATION])
        you.duration[DUR_INSULATION] = 1;

    if (you.duration[DUR_STONEMAIL])
        you.duration[DUR_STONEMAIL] = 1;

    if (you.duration[DUR_CONTROLLED_FLIGHT])
        you.duration[DUR_CONTROLLED_FLIGHT] = 1;

    if (you.duration[DUR_CONTROL_TELEPORT])
        you.duration[DUR_CONTROL_TELEPORT] = 1;

    if (you.duration[DUR_RESIST_POISON])
        you.duration[DUR_RESIST_POISON] = 1;

    if (you.duration[DUR_TRANSFORMATION])
        you.duration[DUR_TRANSFORMATION] = 1;

    //jmf: added following
    if (you.duration[DUR_STONESKIN])
        you.duration[DUR_STONESKIN] = 1;

    if (you.duration[DUR_FORESCRY])
        you.duration[DUR_FORESCRY] = 1;

    if (you.duration[DUR_SEE_INVISIBLE])
        you.duration[DUR_SEE_INVISIBLE] = 1;

    if (you.duration[DUR_SILENCE])
        you.duration[DUR_SILENCE] = 1;

    if (you.duration[DUR_CONDENSATION_SHIELD])
        you.duration[DUR_CONDENSATION_SHIELD] = 1;

    contaminate_player( -1 * (1+random2(5)));
}                               // end antimagic()

void extension(int pow)
{
    int contamination = random2(2);

    if (you.haste)
    {
        potion_effect(POT_SPEED, pow);
        contamination++;
    }

    if (you.slow)
        potion_effect(POT_SLOWING, pow);

#if 0
    if (you.paralysis)
        potion_effect(POT_PARALYSIS, pow);  // how did you cast extension?

    if (you.conf)
        potion_effect(POT_CONFUSION, pow);  // how did you cast extension?
#endif

    if (you.might)
    {
        potion_effect(POT_MIGHT, pow);
        contamination++;
    }

    if (you.levitation)
        potion_effect(POT_LEVITATION, pow);

    if (you.invis)
    {
        potion_effect(POT_INVISIBILITY, pow);
        contamination++;
    }

    if (you.duration[DUR_ICY_ARMOUR])
        ice_armour(pow, true);

    if (you.duration[DUR_REPEL_MISSILES])
        missile_prot(pow);

    if (you.duration[DUR_REGENERATION])
        cast_regen(pow);

    if (you.duration[DUR_DEFLECT_MISSILES])
        deflection(pow);

    if (you.fire_shield)
    {
        you.fire_shield += random2(pow / 20);

        if (you.fire_shield > 50)
            you.fire_shield = 50;

#ifdef JP
        mpr("당신의 불꽃의 고리는 새로운 활력을 얻어 포효했다!");
#else
        mpr("Your ring of flames roars with new vigour!");
#endif
    }

    if ( !(you.duration[DUR_WEAPON_BRAND] < 1
                 || you.duration[DUR_WEAPON_BRAND] > 80) )
    {
        you.duration[DUR_WEAPON_BRAND] += 5 + random2(8);
    }

    if (you.duration[DUR_SWIFTNESS])
        cast_swiftness(pow);

    if (you.duration[DUR_INSULATION])
        cast_insulation(pow);

    if (you.duration[DUR_STONEMAIL])
        stone_scales(pow);

    if (you.duration[DUR_CONTROLLED_FLIGHT])
        cast_fly(pow);

    if (you.duration[DUR_CONTROL_TELEPORT])
        cast_teleport_control(pow);

    if (you.duration[DUR_RESIST_POISON])
        cast_resist_poison(pow);

    if (you.duration[DUR_TRANSFORMATION])
    {
#ifdef JP
        mpr("당신의 변신 시간은 연장되었다.");
#else
        mpr("Your transformation has been extended.");
#endif
        you.duration[DUR_TRANSFORMATION] += random2(pow);
        if (you.duration[DUR_TRANSFORMATION] > 100)
            you.duration[DUR_TRANSFORMATION] = 100;
    }

    //jmf: added following
    if (you.duration[DUR_STONESKIN])
        cast_stoneskin(pow);

    if (you.duration[DUR_FORESCRY])
        cast_forescry(pow);

    if (you.duration[DUR_SEE_INVISIBLE])
        cast_see_invisible(pow);

    if (you.duration[DUR_SILENCE])   //how precisely did you cast extension?
        cast_silence(pow);

    if (you.duration[DUR_CONDENSATION_SHIELD])
        cast_condensation_shield(pow);

    if (contamination)
        contaminate_player( contamination );
}                               // end extension()

void ice_armour(int pow, bool extending)
{
    if (!player_light_armour())
    {
        if (!extending)
#ifdef JP
            mpr("당신은 방어구를 너무 많이 장비하고 있다.");
#else
            mpr("You are wearing too much armour.");
#endif

        return;
    }

    if (you.duration[DUR_STONEMAIL] || you.duration[DUR_STONESKIN])
    {
        if (!extending)
#ifdef JP
            mpr("이 주문은 현재 발동 중인 다른 주문과 양립되지 않는다.");
#else
            mpr("The spell conflicts with another spell still in effect.");
#endif

        return;
    }

    if (you.duration[DUR_ICY_ARMOUR])
#ifdef JP
        mpr( "당신의 얼음의 갑옷은 더욱 두꺼워졌다." );
#else
        mpr( "Your icy armour thickens." );
#endif
    else
    {
        if (you.attribute[ATTR_TRANSFORMATION] == TRAN_ICE_BEAST)
#ifdef JP
            mpr( "당신의 얼음의 육체는 더욱 단단해졌다." );
#else
            mpr( "Your icy body feels more resilient." );
#endif
        else
#ifdef JP
            mpr( "얼음의 막이 당신의 몸을 방어했다!" );
#else
            mpr( "A film of ice covers your body!" );
#endif

        you.redraw_armour_class = 1;
    }

    you.duration[DUR_ICY_ARMOUR] += 20 + random2(pow) + random2(pow);

    if (you.duration[DUR_ICY_ARMOUR] > 50)
        you.duration[DUR_ICY_ARMOUR] = 50;
}                               // end ice_armour()

void stone_scales(int pow)
{
    int dur_change = 0;

    if (you.duration[DUR_ICY_ARMOUR] || you.duration[DUR_STONESKIN])
    {
#ifdef JP
        mpr("이 주문은 현재 발동 중인 다른 주문과 양립되지 않는다.");
#else
        mpr("The spell conflicts with another spell still in effect.");
#endif
        return;
    }

    if (you.duration[DUR_STONEMAIL])
#ifdef JP
        mpr("당신의 비늘 갑옷은 더욱 빈틈없어졌다.");
#else
        mpr("Your scaly armour looks firmer.");
#endif
    else
    {
        if (you.attribute[ATTR_TRANSFORMATION] == TRAN_STATUE)
#ifdef JP
            mpr( "당신의 돌의 육체는 더욱 단단해졌다." );
#else
            mpr( "Your stone body feels more resilient." );
#endif
        else
#ifdef JP
            mpr( "돌로 된 비늘이 당신의 몸을 방어했다!" );
#else
            mpr( "A set of stone scales covers your body!" );
#endif

        you.redraw_evasion = 1;
        you.redraw_armour_class = 1;
    }

    dur_change = 20 + random2(pow) + random2(pow);

    if (dur_change + you.duration[DUR_STONEMAIL] >= 100)
        you.duration[DUR_STONEMAIL] = 100;
    else
        you.duration[DUR_STONEMAIL] += dur_change;

    burden_change();
}                               // end stone_scales()

void missile_prot(int pow)
{
#ifdef JP
    mpr("당신은 원거리 공격으로부터 보호되고 있는 것 같다.");
#else
    mpr("You feel protected from missiles.");
#endif

    you.duration[DUR_REPEL_MISSILES] += 8 + roll_dice( 2, pow );

    if (you.duration[DUR_REPEL_MISSILES] > 100)
        you.duration[DUR_REPEL_MISSILES] = 100;
}                               // end missile_prot()

void deflection(int pow)
{
#ifdef JP
    mpr("당신은 원거리 공격으로부터 매우 안전하게 지켜지고 있는 것 같다.");
#else
    mpr("You feel very safe from missiles.");
#endif

    you.duration[DUR_DEFLECT_MISSILES] += 15 + random2(pow);

    if (you.duration[DUR_DEFLECT_MISSILES] > 100)
        you.duration[DUR_DEFLECT_MISSILES] = 100;
}                               // end cast_deflection()

void cast_regen(int pow)
{
    //if (pow > 150) pow = 150;
#ifdef JP
    mpr("당신의 피부는 근질근질해왔다.");
#else
    mpr("Your skin crawls.");
#endif

    you.duration[DUR_REGENERATION] += 5 + roll_dice( 2, pow / 3 + 1 );

    if (you.duration[DUR_REGENERATION] > 100)
        you.duration[DUR_REGENERATION] = 100;
}                               // end cast_regen()

void cast_berserk(void)
{
    go_berserk(true);
}                               // end cast_berserk()

void cast_swiftness(int power)
{
    int dur_incr = 0;

    if (player_in_water())
    {
        if (you.species == SP_MERFOLK)
#ifdef JP
            mpr("당신이 헤엄치고 있는 동안에는, 이 주문은 효과가 없다!");
#else
            mpr("This spell will not benefit you while you're swimming!");
#endif
        else
#ifdef JP
            mpr("당신이 물속에 있는 동안에는, 이 주문은 효과가 없다!");
#else
            mpr("This spell will not benefit you while you're in water!");
#endif

        return;
    }

    if (!you.duration[DUR_SWIFTNESS] && player_movement_speed() <= 6)
    {
#ifdef JP
        mpr( "당신은 지금보다 더 빨리 움직일 수는 없다." );
#else
        mpr( "You can't move any more quickly." );
#endif
        return;
    }

    // Reduced the duration:  -- bwr
    // dur_incr = random2(power) + random2(power) + 20;
    dur_incr = 20 + random2( power );

    // Centaurs do have feet and shouldn't get here anyways -- bwr
#ifdef JP
    snprintf( info, INFO_SIZE, "당신은 재빨라진 것 같다." );
#else
    snprintf( info, INFO_SIZE, "You feel quick%s",
              (you.species == SP_NAGA) ? "." : " on your feet." );
#endif

    mpr(info);

    if (dur_incr + you.duration[DUR_SWIFTNESS] > 100)
        you.duration[DUR_SWIFTNESS] = 100;
    else
        you.duration[DUR_SWIFTNESS] += dur_incr;
}                               // end cast_swiftness()

void cast_fly(int power)
{
    int dur_change = 25 + random2(power) + random2(power);

    if (!player_is_levitating())
#ifdef JP
        mpr("당신은 공중으로 날아올랐다.");
#else
        mpr("You fly up into the air.");
#endif
    else
#ifdef JP
        mpr("당신은 몸이 더 가벼워졌다. ");
#else
        mpr("You feel more buoyant.");
#endif

    if (you.levitation + dur_change > 100)
        you.levitation = 100;
    else
        you.levitation += dur_change;

    if (you.duration[DUR_CONTROLLED_FLIGHT] + dur_change > 100)
        you.duration[DUR_CONTROLLED_FLIGHT] = 100;
    else
        you.duration[DUR_CONTROLLED_FLIGHT] += dur_change;

    // duration[DUR_CONTROLLED_FLIGHT] makes the game think player
    // wears an amulet of controlled flight

    burden_change();
}

void cast_insulation(int power)
{
    int dur_incr = 10 + random2(power);

#ifdef JP
    mpr("당신은 절연화된 것 같다.");
#else
    mpr("You feel insulated.");
#endif

    if (dur_incr + you.duration[DUR_INSULATION] > 100)
        you.duration[DUR_INSULATION] = 100;
    else
        you.duration[DUR_INSULATION] += dur_incr;
}                               // end cast_insulation()

void cast_resist_poison(int power)
{
    int dur_incr = 10 + random2(power);

#ifdef JP
    mpr("당신은 독에 대한 내성을 얻었다.");
#else
    mpr("You feel resistant to poison.");
#endif

    if (dur_incr + you.duration[DUR_RESIST_POISON] > 100)
        you.duration[DUR_RESIST_POISON] = 100;
    else
        you.duration[DUR_RESIST_POISON] += dur_incr;
}                               // end cast_resist_poison()

void cast_teleport_control(int power)
{
    int dur_incr = 10 + random2(power);

    if (you.duration[DUR_CONTROL_TELEPORT] == 0)
        you.attribute[ATTR_CONTROL_TELEPORT]++;

#ifdef JP
    mpr("당신은 공간이동을 제어할 수 있게 된 것 같다.");
#else
    mpr("You feel in control.");
#endif

    if (dur_incr + you.duration[DUR_CONTROL_TELEPORT] >= 50)
        you.duration[DUR_CONTROL_TELEPORT] = 50;
    else
        you.duration[DUR_CONTROL_TELEPORT] += dur_incr;
}                               // end cast_teleport_control()

void cast_ring_of_flames(int power)
{
    you.fire_shield += 5 + (power / 10) + (random2(power) / 5);

    if (you.fire_shield > 50)
        you.fire_shield = 50;

#ifdef JP
    mpr("당신 주위의 대기가 불꽃으로 변했다!");
#else
    mpr("The air around you leaps into flame!");
#endif

    manage_fire_shield();
}                               // end cast_ring_of_flames()

void cast_confusing_touch(int power)
{
#ifdef JP
    snprintf( info, INFO_SIZE, "당신의 %s은(는) %s",
              your_hand(true), (you.confusing_touch ? "빛을 더했다." : "붉게 빛났다.") );
#else
    snprintf( info, INFO_SIZE, "Your %s begin to glow %s.",
              your_hand(true), (you.confusing_touch ? "brighter" : "red") );
#endif

    mpr( info );

    you.confusing_touch += 5 + (random2(power) / 5);

    if (you.confusing_touch > 50)
        you.confusing_touch = 50;

}                               // end cast_confusing_touch()

bool cast_sure_blade(int power)
{
    bool success = false;

    if (you.equip[EQ_WEAPON] == -1)
#ifdef JP
        mpr("당신은 무기를 손에 쥐고 있지 않다!");
#else
        mpr("You aren't wielding a weapon!");
#endif
    else if (weapon_skill( you.inv[you.equip[EQ_WEAPON]].base_type,
                     you.inv[you.equip[EQ_WEAPON]].sub_type) != SK_SHORT_BLADES)
    {
#ifdef JP
        mpr("당신은 이 무기와 결합할 수 없다.");
#else
        mpr("You cannot bond with this weapon.");
#endif
    }
    else
    {
        if (!you.sure_blade)
#ifdef JP
            mpr("당신은 무기와 일체화되었다.");
#else
            mpr("You become one with your weapon.");
#endif
        else if (you.sure_blade < 25)
#ifdef JP
            mpr("당신의 결합은 보다 강고해졌다.");
#else
            mpr("Your bond becomes stronger.");
#endif

        you.sure_blade += 8 + (random2(power) / 10);

        if (you.sure_blade > 25)
            you.sure_blade = 25;

        success = true;
    }

    return (success);
}                               // end cast_sure_blade()

void manage_fire_shield(void)
{
    you.fire_shield--;

    if (!you.fire_shield)
        return;

    char stx = 0, sty = 0;

    for (stx = -1; stx < 2; stx++)
    {
        for (sty = -1; sty < 2; sty++)
        {
            if (sty == 0 && stx == 0)
                continue;

            //if ( one_chance_in(3) ) beam.range ++;

            if (grd[you.x_pos + stx][you.y_pos + sty] > DNGN_LAST_SOLID_TILE
                && env.cgrid[you.x_pos + stx][you.y_pos + sty] == EMPTY_CLOUD)
            {
                place_cloud( CLOUD_FIRE, you.x_pos + stx, you.y_pos + sty,
                             1 + random2(6) );
            }
        }
    }
}                               // end manage_fire_shield()
