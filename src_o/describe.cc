/*
 *  File:       describe.cc
 *  Summary:    Functions used to print information about various game objects.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <4>      10/14/99     BCR     enummed describe_god()
 *      <3>      10/13/99     BCR     Added GOD_NO_GOD case in describe_god()
 *      <2>      5/20/99      BWR     Replaced is_artifact with
 *                                    is_dumpable_artifact
 *      <1>      4/20/99      JDJ     Reformatted, uses string objects,
 *                                    split out 10 new functions from
 *                                    describe_item(), added
 *                                    get_item_description and
 *                                    is_artifact.
 */

#include "AppHdr.h"
#include "describe.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "abl-show.h"
#include "debug.h"
#include "fight.h"
#include "itemname.h"
#include "macro.h"
#include "mon-util.h"
#include "player.h"
#include "randart.h"
#include "religion.h"
#include "skills2.h"
#include "stuff.h"
#include "wpn-misc.h"
#include "spl-util.h"

// ========================================================================
//      Internal Functions
// ========================================================================

//---------------------------------------------------------------
//
// append_value
//
// Appends a value to the string. If plussed == 1, will add a + to
// positive values (itoa always adds - to -ve ones).
//
//---------------------------------------------------------------
static void append_value( std::string & description, int valu, bool plussed )
{
    if (valu >= 0 && plussed == 1)
        description += "+";

    char value_str[80];

    itoa( valu, value_str, 10 );

    description += value_str;
}                               // end append_value()

//---------------------------------------------------------------
//
// print_description
//
// Takes a descpr string filled up with stuff from other functions,
// and displays it with minor formatting to avoid cut-offs in mid
// word and such. The character $ is interpreted as a CR.
//
//---------------------------------------------------------------
static void print_description( const std::string &d )
{
    unsigned int  nextLine = std::string::npos;
    unsigned int  currentPos = 0;

#ifdef DOS
    const unsigned int lineWidth = 52;
#else
    const unsigned int lineWidth = 70;
#endif

    bool nlSearch = true;       // efficiency

    textcolor(LIGHTGREY);

    while(currentPos < d.length())
    {
        if (currentPos != 0)
        {
#ifdef PLAIN_TERM
            gotoxy(1, wherey() + 1);
#endif
#ifdef DOS_TERM
            cprintf(EOL);
#endif
        }

        // see if $ sign is within one lineWidth
        if (nlSearch)
        {
            nextLine = d.find('$', currentPos);

            if (nextLine >= currentPos && nextLine < currentPos + lineWidth)
            {
                cprintf((d.substr(currentPos, nextLine - currentPos)).c_str());
                currentPos = nextLine + 1;
                continue;
            }

            if (nextLine == std::string::npos)
                nlSearch = false;       // there are no newlines, don't search again.
        }

        // no newline -- see if rest of string will fit.
        if (currentPos + lineWidth >= d.length())
        {
            cprintf((d.substr(currentPos)).c_str());
            return;
        }


        // ok.. try to truncate at space.
        nextLine = d.rfind(' ', currentPos + lineWidth);

        if (nextLine != std::string::npos)
        {
            cprintf((d.substr(currentPos, nextLine - currentPos)).c_str());
            currentPos = nextLine + 1;
            continue;
        }

        // oops.  just truncate.
        nextLine = currentPos + lineWidth;

        if (nextLine > d.length())
            nextLine = d.length();

        cprintf((d.substr(currentPos, nextLine - currentPos)).c_str());
        currentPos = nextLine;
    }
}

//!!!! description += "xxxxxxx "; ÇÕï∂éöóÒññîˆÇ…îºäpÉXÉyÅ[ÉXÇì¸ÇÍÇ»Ç¢Ç∆ÉtÉäÅ[ÉYÇãNÇ±Ç∑Ç±Ç∆Ç™Ç†ÇÈÅB
//     ïKÇ∏â¸çsÇµÇΩÇ¢èÍçáÇ…ÇÕ$Çì¸ÇÍÇÈÇ™ÅA" $"Ç∆Ç∑ÇÈÇ±Ç∆Ç…ÇµÇƒÇ®Ç≠ÅB

//---------------------------------------------------------------
//
// randart_descpr
//
// Appends the various powers of a random artefact to the description
// string.
//
//---------------------------------------------------------------
static void randart_descpr( std::string &description, const item_def &item )
{
    unsigned int old_length = description.length();

    FixedVector< char, RA_PROPERTIES > proprt;
    randart_wpn_properties( item, proprt );

    if (proprt[ RAP_AC ])
    {
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¿⁄Ω≈¿« ACø° øµ«‚¿ª ¡ÿ¥Ÿ.(";
#else
        description += "$It affects your AC (";
#endif
        append_value(description, proprt[ RAP_AC ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_EVASION ])
    {
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« »∏««∑¬ø° øµ«‚¿ª ¡ÿ¥Ÿ.(";
#else
        description += "$It affects your evasion (";
#endif
        append_value(description, proprt[ RAP_EVASION ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_STRENGTH ])
    {
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« »˚ø° øµ«‚¿ª ¡ÿ¥Ÿ.(";
#else
        description += "$It affects your strength (";
#endif
        append_value(description, proprt[ RAP_STRENGTH ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_INTELLIGENCE ])
    {
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« ¡ˆ¥…ø° øµ«‚¿ª ¡ÿ¥Ÿ.(";
#else
        description += "$It affects your intelligence (";
#endif
        append_value(description, proprt[ RAP_INTELLIGENCE ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_DEXTERITY ])
    {
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« πŒ√∏ø° øµ«‚¿ª ¡ÿ¥Ÿ.(";
#else
        description += "$It affects your dexterity (";
#endif
        append_value(description, proprt[ RAP_DEXTERITY ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_ACCURACY ])
    {
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« ∏Ì¡ﬂ∑¸ø° øµ«‚¿ª ¡ÿ¥Ÿ.(";
#else
        description += "$It affects your accuracy (";
#endif
        append_value(description, proprt[ RAP_ACCURACY ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_DAMAGE ])
    {
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« ∞¯∞›∑¬ø° øµ«‚¿ª ¡ÿ¥Ÿ.(";
#else
        description += "$It affects your damage-dealing abilities (";
#endif
        append_value(description, proprt[ RAP_DAMAGE ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_FIRE ] < -2)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ∫“ ∞¯∞›ø° ±ÿµµ∑Œ æ‡«ÿ¡ˆ∞‘ «—¥Ÿ.";
#else
        description += "$It makes you highly vulnerable to fire. ";
#endif
    else if (proprt[ RAP_FIRE ] == -2)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ∫“ ∞¯∞›ø° ∏≈øÏ æ‡«œ∞‘ «—¥Ÿ.";
#else
        description += "$It makes you greatly susceptible to fire. ";
#endif
    else if (proprt[ RAP_FIRE ] == -1)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ∫“ ∞¯∞›ø° ¥Ÿº“ æ‡«œ∞‘ «—¥Ÿ.";
#else
        description += "$It makes you susceptible to fire. ";
#endif
    else if (proprt[ RAP_FIRE ] == 1)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ∫“∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ.";
#else
        description += "$It protects you from fire. ";
#endif
    else if (proprt[ RAP_FIRE ] == 2)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ∫Ÿ∑Œ∫Œ≈Õ ∞≠«œ∞‘ ∫∏»£«—¥Ÿ.";
#else
        description += "$It greatly protects you from fire. ";
#endif
    else if (proprt[ RAP_FIRE ] > 2)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ∫“∑Œ∫Œ≈Õ ∞≈¿« øœ¿¸»˜ ¡ˆƒ—¡ÿ¥Ÿ.";
#else
        description += "$It renders you almost immune to fire. ";
#endif

    if (proprt[ RAP_COLD ] < -2)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ≥√±‚ø° ±ÿµµ∑Œ æ‡«ÿ¡ˆ∞‘ «—¥Ÿ.";
#else
        description += "$It makes you highly susceptible to cold. ";
#endif
    else if (proprt[ RAP_COLD ] == -2)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ≥√±‚ø° ∏≈øÏ æ‡«ÿ¡ˆ∞‘ «—¥Ÿ.";
#else
        description += "$It makes you greatly susceptible to cold. ";
#endif
    else if (proprt[ RAP_COLD ] == -1)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ≥√±‚ø° ¥Ÿº“ æ‡«ÿ¡ˆ∞‘ «—¥Ÿ.";
#else
        description += "$It makes you susceptible to cold. ";
#endif
    else if (proprt[ RAP_COLD ] == 1)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ≥√±‚∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ.";
#else
        description += "$It protects you from cold. ";
#endif
    else if (proprt[ RAP_COLD ] == 2)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ≥√±‚∑Œ∫Œ≈Õ ∞≠«œ∞‘ ∫∏»£«—¥Ÿ.";
#else
        description += "$It greatly protects you from cold. ";
#endif
    else if (proprt[ RAP_COLD ] > 2)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ≥√±‚∑Œ∫Œ≈Õ ∞≈¿« øœ¿¸»˜ ¡ˆƒ—¡ÿ¥Ÿ.";
#else
        description += "$It renders you almost immune to cold. ";
#endif

    if (proprt[ RAP_ELECTRICITY ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ¿¸±‚ ∞¯∞›ø° ∏Èø™¿Ã µ«∞‘ «—¥Ÿ.";
#else
        description += "$It insulates you from electricity. ";
#endif

    if (proprt[ RAP_POISON ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª µ∂¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ.";
#else
        description += "$It protects you from poison. ";
#endif

    if (proprt[ RAP_NEGATIVE_ENERGY ] == 1)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ¿Ωø°≥ ¡ˆ∑Œ∫Œ≈Õ æÓ¥¿¡§µµ ∫∏»£«—¥Ÿ.";
#else
        description += "$It partially protects you from negative energy. ";
#endif
    else if (proprt[ RAP_NEGATIVE_ENERGY ] == 2)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ¿Ωø°≥ ¡ˆ∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ.";
#else
        description += "$It protects you from negative energy. ";
#endif
    else if (proprt[ RAP_NEGATIVE_ENERGY ] > 2)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ¿Ωø°≥ ¡ˆø° ∞≈¿« ∏Èø™¿Ã µ«∞‘ «—¥Ÿ.";
#else
        description += "$It renders you almost immune negative energy. ";
#endif

    if (proprt[ RAP_MAGIC ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ∏∂π˝¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ.";
#else
        description += "$It protects you from magic. ";
#endif

    if (proprt[ RAP_STEALTH ] < 0)
    {
        if (proprt[ RAP_STEALTH ] < -20)
#ifdef JP
            description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« ¿∫π–«— øÚ¡˜¿”¿ª ≈©∞‘ πÊ«ÿ«—¥Ÿ.";
#else
            description += "$It makes you much less stealthy. ";
#endif
        else
#ifdef JP
            description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« ¿∫π–«— øÚ¡˜¿”¿ª πÊ«ÿ«—¥Ÿ.";
#else
            description += "$It makes you less stealthy. ";
#endif
    }
    else if (proprt[ RAP_STEALTH ] > 0)
    {
        if (proprt[ RAP_STEALTH ] > 20)
#ifdef JP
            description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« øÚ¡˜¿”¿ª ∏≈øÏ ¡∂øÎ«œ∞‘ ∏∏µÁ¥Ÿ.";
#else
            description += "$It makes you much more stealthy. ";
#endif
        else
#ifdef JP
            description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« øÚ¡˜¿”¿ª ¡∂øÎ«œ∞‘ ∏∏µÁ¥Ÿ.";
#else
            description += "$It makes you more stealthy. ";
#endif
    }

    if (proprt[ RAP_EYESIGHT ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« Ω√∑¬¿ª ∞≠»≠Ω√≈≤¥Ÿ.";
#else
        description += "$It enhances your eyesight. ";
#endif

    if (proprt[ RAP_INVISIBLE ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« ∏ˆ¿Ã ≈ı∏Ì»≠«“ ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ.";
#else
        description += "$It lets you turn invisible. ";
#endif

    if (proprt[ RAP_LEVITATE ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿Ã ∞¯¡ﬂ∫ŒæÁ¿ª «“ ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ.";
#else
        description += "$It lets you levitate. ";
#endif

    if (proprt[ RAP_BLINK ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿Ã º¯∞£¿Ãµø¿ª «“ ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ.";
#else
        description += "$It lets you blink. ";
#endif

    if (proprt[ RAP_CAN_TELEPORT ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿Ã ∞¯∞£¿Ãµø¿ª «“ ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ.";
#else
        description += "$It lets you teleport. ";
#endif

    if (proprt[ RAP_BERSERK ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿Ã ±§∆˜»≠∏¶ «“ ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ.";
#else
        description += "$It lets you go berserk. ";
#endif

    if (proprt[ RAP_MAPPING ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿Ã ¡÷∫Ø¿ª ∞®¡ˆ«“ ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ.";
#else
        description += "$It lets you sense your surroundings. ";
#endif

    if (proprt[ RAP_NOISES ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ º“¿Ω¿ª πﬂª˝Ω√≈≤¥Ÿ.";
#else
        description += "$It makes noises. ";
#endif

    if (proprt[ RAP_PREVENT_SPELLCASTING ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¡÷πÆ Ω√¿¸¿ª πÊ«ÿ«—¥Ÿ.";
#else
        description += "$It prevents spellcasting. ";
#endif

    if (proprt[ RAP_CAUSE_TELEPORTATION ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ∞¯∞£¿Ãµø¿ª ¿œ¿∏≈≤¥Ÿ.";
#else
        description += "$It causes teleportation. ";
#endif

    if (proprt[ RAP_PREVENT_TELEPORTATION ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥Î∫Œ∫–¿« ∞¯∞£¿Ãµø¿ª πÊ«ÿ«—¥Ÿ.";
#else
        description += "$It prevents most forms of teleportation. ";
#endif

    if (proprt[ RAP_ANGRY ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿ª ∫–≥Î«œ∞‘ ∏∏µÁ¥Ÿ.";
#else
        description += "$It makes you angry. ";
#endif

    if (proprt[ RAP_METABOLISM ] >= 3)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« Ω≈¡¯¥ÎªÁ º”µµ∏¶ ≈©∞‘ ∞°º”Ω√≈≤¥Ÿ.";
#else
        description += "$It greatly speeds your metabolism. ";
#endif
    else if (proprt[ RAP_METABOLISM ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« Ω≈¡¯¥ÎªÁ º”µµ∏¶ ∞°º”Ω√≈≤¥Ÿ.";
#else
        description += "$It speeds your metabolism. ";
#endif

    if (proprt[ RAP_MUTAGENIC ] > 3)
#ifdef JP
        description += "$¿Ã∞Õ¿∫ µπø¨∫Ø¿Ã∏¶ ¿Øπﬂ«œ¥¬ πÊªÁ¥…¿ª ∞≠«œ∞‘ πÊ√‚«œ∞Ì ¿÷¥Ÿ."; // Dio ∞≠æ‡¿« ¬˜¿Ã∏¶ æÀ±‚ ¿ß«ÿº≠ ¡ª πŸ≤Â¥¬µ•... »Ï.
#else
        description += "$It glows with mutagenic radiation.";
#endif
    else if (proprt[ RAP_MUTAGENIC ])
#ifdef JP
        description += "$¿Ã∞Õ¿∫ µπø¨∫Ø¿Ã∏¶ ¿Øπﬂ«œ¥¬ πÊªÁ¥…¿ª πÊ√‚«œ∞Ì ¿÷¥Ÿ.";
#else
        description += "$It emits mutagenic radiations.";
#endif

    if (old_length != description.length())
        description += "$";

    if (is_unrandom_artefact( item ))
    {
        const char *desc = unrandart_descrip( 0, item );
        if (strlen( desc ) > 0)
        {
            description += desc;
            description += "$";
        }
    }
}


//---------------------------------------------------------------
//
// describe_demon
//
// Describes the random demons you find in Pandemonium.
//
//---------------------------------------------------------------
static std::string describe_demon(void)
{
    long globby = 0;

    for (unsigned int i = 0; i < strlen( ghost.name ); i++)
        globby += ghost.name[i];

    globby *= strlen( ghost.name );

    srand( globby );

#ifdef JP
    std::string description = "∞≠∑¬«— æ«∏∂ ";
    description += "";
#else
    std::string description = "A powerful demon, ";
#endif

    description += ghost.name;
#ifdef JP
    description += "¿∫(¥¬)";
#else
    description += " has a";
#endif

    switch (random2(31))
    {
    case 0:
#ifdef JP
        description += " ∞≈¥Î«œ∞Ì ø¯≈Î√≥∑≥ ª˝±‰ ";
#else
        description += " huge, barrel-shaped ";
#endif
        break;
    case 1:
#ifdef JP
        description += " æ»∞≥øÕ ∞∞¿Ã »ÒπÃ«— ";
#else
        description += " wispy, insubstantial ";
#endif
        break;
    case 2:
#ifdef JP
        description += " ∞°¥√∞Ì ±‰ ";
#else
        description += " spindly ";
#endif
        break;
    case 3:
#ifdef JP
        description += " «ÿ∞Ò∑Œ µ«æÓ ¿÷¥¬ ";
#else
        description += " skeletal ";
#endif
        break;
    case 4:
#ifdef JP
        description += " »‰√¯«œ∞‘ ±‚«¸¿Œ ";
#else
        description += " horribly deformed ";
#endif
        break;
    case 5:
#ifdef JP
        description += " ∞°Ω√∑Œ µ§»˘ ";
#else
        description += " spiny ";
#endif
        break;
    case 6:
#ifdef JP
        description += " ∫Œ∂˚¿⁄ ∞∞¿∫ ";
#else
        description += " waif-like ";
#endif
        break;
    case 7:
#ifdef JP
        description += " ∫Ò¥√∑Œ µ§»˘ ";
#else
        description += " scaly ";
#endif
        break;
    case 8:
#ifdef JP
        description += " ≤˚ƒ¢«œ∞‘ ±‚«¸¿Œ ";
#else
        description += " sickeningly deformed ";
#endif
        break;
    case 9:
#ifdef JP
        description += " ∫ŒªÛ¿ª ¿‘æÓ ««∏¶ »Í∏Æ¥¬ ";
#else
        description += " bruised and bleeding ";
#endif
        break;
    case 10:
#ifdef JP
        description += " ≤˚ƒ¢«— ";
#else
        description += " sickly ";
#endif
        break;
    case 11:
#ifdef JP
        description += " ºˆ∏π¿∫ √Àºˆ∞° µ∏æ∆ ¿÷¥¬ ";
#else
        description += " mass of writhing tentacles for a ";
#endif
        break;
    case 12:
#ifdef JP
        description += " ≤ˆ¿˚∞≈∏Æ¥¬ µ¢ƒ¿Ã ºˆæ¯¿Ã µ∏æ∆ ¿÷¥¬ ";
#else
        description += " mass of ropey tendrils for a ";
#endif
        break;
    case 13:
#ifdef JP
        description += " ≥™π´ ¡Ÿ±‚√≥∑≥ ª˝±‰ ";
#else
        description += " tree trunk-like ";
#endif
        break;
    case 14:
#ifdef JP
        description += " ≈–¿Ã ≥™ ¿÷¥¬ ";
#else
        description += " hairy ";
#endif
        break;
    case 15:
#ifdef JP
        description += " ≈–∑Œ µ⁄µ§»˘ ";
#else
        description += " furry ";
#endif
        break;
    case 16:
#ifdef JP
        description += " ºÿ≈–¿Ã ≥™ ¿÷¥¬ ";
#else
        description += " fuzzy ";
#endif
        break;
    case 17:
#ifdef JP
        description += " ≈Î≈Î«— "; // Dio ¿Ã∞« π´Ωº∏ª¿Œ¡ˆ ¿ﬂ...
#else
        description += "n obese ";
#endif
        break;
    case 18:
#ifdef JP
        description += " ªÏ¬ ";
#else
        description += " fat ";
#endif
        break;
    case 19:
#ifdef JP
        description += " ¡°æ◊¡˙¿« ";
#else
        description += " slimy ";
#endif
        break;
    case 20:
#ifdef JP
        description += " ¬ﬁ±€¬ﬁ±€ ¡÷∏ß¡¯ ";
#else
        description += " wrinkled ";
#endif
        break;
    case 21:
#ifdef JP
        description += " ±›º”¿« ";
#else
        description += " metallic ";
#endif
        break;
    case 22:
#ifdef JP
        description += " ¿Ø∏Æ¿« ";
#else
        description += " glassy ";
#endif
        break;
    case 23:
#ifdef JP
        description += " ºˆ¡§¿« ";
#else
        description += " crystalline ";
#endif
        break;
    case 24:
#ifdef JP
        description += " ±Ÿ¿∞¡˙¿« ";
#else
        description += " muscular ";
#endif
        break;
    case 25:
#ifdef JP
        description += " ≤ˆ¿˚≤ˆ¿˚«— ";
#else
        description += "n icky ";
#endif
        break;
    case 26:
#ifdef JP
        description += " ∫Œ«ÆæÓ ¿÷¥¬ ";
#else
        description += " swollen ";
#endif
        break;
    case 27:
#ifdef JP
        description += " øÔ≈¸∫“≈¸«— ";
#else
        description += " lumpy ";
#endif
        break;
    case 28:
#ifdef JP
        description += " ∞©ø ¿ª µŒ∏• ";
#else
        description += " armoured ";
#endif
        break;
    case 29:
#ifdef JP
        description += " ∞©∞¢¿∏∑Œ µ—∑ØΩŒ¿Œ ";
#else
        description += " carapaced ";
#endif
        break;
    case 30:
#ifdef JP
        description += " »£∏Æ»£∏Æ«— ";
#else
        description += " slender ";
#endif
        break;
    }

#ifdef JP
    description += "∏ˆ≈Î";    // ∞®¿⁄∞° ºˆ¡§
#else                                                                      // ∏¬¥¬¡ˆ¥¬ ¿ﬂ ∏∏£∞⁄∞Ì, æ»µ«∏È ¥Ÿ∏• πÊπ˝¿ª..
    description += "body";
#endif


    switch (ghost.values[GVAL_DEMONLORD_FLY])
    {
    case 1: // proper flight
        switch (random2(10))
        {
        case 0:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ¿€¿∫ ∞Ô√Ê¿« ≥Ø∞≥∞° ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with small insectoid wings";
#endif
            break;
        case 1:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ƒø¥Ÿ∂ı ∞Ô√Ê¿« ≥Ø∞≥∞° ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with large insectoid wings";
#endif
            break;
        case 2:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ≥™πÊ ≥Ø∞≥ ∞∞¿∫ ∞Õ¿Ã ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with moth-like wings";
#endif
            break;
        case 3:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ≥™∫Ò ≥Ø∞≥∞° ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with butterfly wings";
#endif
            break;
        case 4:
#ifdef JP
            description += " , ¿Ã∞Õ¿∫ π⁄¡„ ≥Ø∞≥ ∞∞¿∫ ∞Õ¿Ã ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with huge, bat-like wings";
#endif
            break;
        case 5:
#ifdef JP
            description += " , ¿Ã∞Õ¿∫ ªÏ∞Ø¿∏∑Œ ¿Ã∑ÁæÓ¡¯ ≥Ø∞≥∞° ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with fleshy wings";
#endif
            break;
        case 6:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ¡∂±◊∏∂«— π⁄¡„ ≥Ø∞≥ ∞∞¿∫ ∞Õ¿Ã ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with small, bat-like wings";
#endif
            break;
        case 7:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ≈–∫πº˛¿Ã¿« ≥Ø∞≥∞° ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with hairy wings";
#endif
            break;
        case 8:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ∞≈¥Î«— ±Í≈– ≥Ø∞≥∞° ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with great feathered wings";
#endif
            break;
        case 9:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ∫˚≥™¥¬ ±›º” ≥Ø∞≥∞° ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with shiny metal wings";
#endif
            break;
        default:
            break;
        }
        break;

    case 2: // levitation
        if (coinflip())
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ∞¯¡ﬂ¿ª ∂∞¥Ÿ¥—¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " which hovers in mid-air";
#endif
        else
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ µÓµ⁄∑Œ  ∞°Ω∫∞° ≥™ø¿¥¬ ∞°πÊ¿Ã ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " with sacs of gas hanging from its back";
#endif
        break;

    default:  // does not fly
        switch (random2(40))
        {
        default:
#ifdef JP
            description += ". "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#endif
            break;
        case 12:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ±‚æÓ¥Ÿ¥œ¥¬ ¡∂±◊∏∂«— ∞≈πÃµÈ∑Œ µ§«Ù¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " covered in tiny crawling spiders";
#endif
            break;
        case 13:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ±‚æÓ¥Ÿ¥œ¥¬ ¡∂±◊∏∂«— π˙∑πµÈ∑Œ µ§«Ù¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " covered in tiny crawling insects";
#endif
            break;
        case 14:
#ifdef JP
            description += ", ±◊∏Æ∞Ì æ«æÓ¿« ∏”∏Æ∏¶ «œ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and the head of a crocodile";
#endif
            break;
        case 15:
#ifdef JP
            description += ", ±◊∏Æ∞Ì «œ∏∂¿« ∏”∏Æ∏¶ «œ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and the head of a hippopotamus";
#endif
            break;
        case 16:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ¿‘¿∫ ¿‹»§«œ∞‘ ±¡¿∫ ∫Œ∏Æ∑Œ µ«æÓ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and a cruel curved beak for a mouth";
#endif
            break;
        case 17:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ¿‘¿∫ ∞∞‘ ño¿∫ ª«¡∑«— ∫Œ∏Æ∑Œ µ«æÓ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and a straight sharp beak for a mouth";
#endif
            break;
        case 18:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ∏”∏Æ∞° æ¯¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and no head at all";
#endif
            break;
        case 19:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ¿‘ø°¥¬ π´Ω√π´Ω√«— √Àºˆ∞° æ˚ƒ—¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and a hideous tangle of tentacles for a mouth";
#endif
            break;
        case 20:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ƒ⁄≥¢∏Æ ∞∞¿∫ ∏ˆ≈Î¿ª «œ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and an elephantine trunk";
#endif
            break;
        case 21:
#ifdef JP
            description += ", ±◊∏Æ∞Ì æ«∏∂√≥∑≥ ª˝±‰ ƒ⁄∏¶ «œ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and an evil-looking proboscis";
#endif
            break;
        case 22:
#ifdef JP
            description += ", ±◊∏Æ∞Ì 12∞≥¿« ¥´¿ª «œ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and dozens of eyes";
#endif
            break;
        case 23:
#ifdef JP
            description += ", ±◊∏Æ∞Ì √ﬂ«— µŒ ∞≥¿« ∏”∏Æ∏¶ ∞Æ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and two ugly heads";
#endif
            break;
        case 24:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ±Ê¥Ÿ∂ı µ∂ªÁ¿« ≤ø∏Æ∏¶ «œ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and a long serpentine tail";
#endif
            break;
        case 25:
#ifdef JP
            description += ", ±◊∏Æ∞Ì æ∆∑ß≈Œ¿∏∑Œ∫Œ≈Õ ∞≈¥Î«— æÓ±›¥œ «— Ω÷¿Ã ≥™øÕ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and a pair of huge tusks growing from its jaw";
#endif
            break;
        case 26:
            description +=
#ifdef JP
                ", ±◊∏Æ∞Ì æÛ±º¿« ¡§∏Èø° ƒø¥Ÿ∂ı ø‹¥´¿Ã ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
                " and a single huge eye, in the centre of its forehead";
#endif
            break;
        case 27:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ∞À¿∫ ±›º”¿∏∑Œµ» ª«¡”«— ¿Ãª°¿Ã ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and spikes of black metal for teeth";
#endif
            break;
        case 28:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ∏”∏Æø° ø¯π›«¸ »Ì∆«¿Ã ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and a disc-shaped sucker for a head";
#endif
            break;
        case 29:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ≈©∞Ì ∆Ú∆Ú«— ±Õ∏¶ ∞°¡ˆ∞Ì ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and huge, flapping ears";
#endif
            break;
        case 30:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ∞°Ωø «—∞°øÓµ• ¿Ãª°¿Ã ≥≠ ¡÷µ’¿Ã∞° ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and a huge, toothy maw in the centre of its chest";
#endif
            break;
        case 31:
#ifdef JP
            description += ", ±◊∏Æ∞Ì µÓµ⁄ø° ƒø¥Ÿ∂ı ¥ﬁ∆ÿ¿Ã ≤Æ¡˙¿Ã ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and a giant snail shell on its back";
#endif
            break;
        case 32:
#ifdef JP
            description += ", ±◊∏Æ∞Ì 12∞≥¿« ∏”∏Æ∞° ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and a dozen heads";
#endif
            break;
        case 33:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ¿⁄ƒÆ¿« ∏”∏Æ∏¶ «œ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and the head of a jackal";
#endif
            break;
        case 34:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ∫Ò∫Òø¯º˛¿Ã¿« ∏”∏Æ∏¶ «œ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and the head of a baboon";
#endif
            break;
        case 35:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ƒß¿Ã ¡˙¡˙ »Â∏£¥¬ ƒø¥Ÿ∂ı «Ù∏¶ ≥ª≥ı∞Ì ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and a huge, slobbery tongue";
#endif
            break;
        case 36:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ ±Ì¿∫ ªÛ√≥∑Œ µ⁄µ§«Ù¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " which is covered in oozing lacerations";
#endif
            break;
        case 37:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ∞≥±∏∏Æ¿« ∏”∏Æ∏¶ «œ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and the head of a frog";
#endif
            break;
        case 38:
#ifdef JP
            description += ", ±◊∏Æ∞Ì æﬂ≈©¿« ∏”∏Æ∏¶ «œ∞Ì¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and the head of a yak";
#endif
            break;
        case 39:
#ifdef JP
            description += ", ±◊∏Æ∞Ì ¥ﬁ∆ÿ¿Ã ¥´ ∞∞¿∫∞‘ ≥™øÕ¿÷¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " and eyes out on stalks";
#endif
            break;
        }
        break;
    }

#ifdef JP
    //description += ".";
#else
    description += ".";
#endif

    switch (random2(40) + (you.species == SP_MUMMY ? 3 : 0))
    {
    case 0:
#ifdef JP
        description += " ¿Ø»≤≥øªı∞° ¡ˆµ∂«œ∞‘ «≥∞‹ø¬¥Ÿ.";
#else
        description += " It stinks of brimstone.";
#endif
        break;
    case 1:
#ifdef JP
        description += " Ω‚æÓ∞°¥¬ µÌ«— æ«√Î∞° π–∑¡ø¬¥Ÿ";
#else
        description += " It smells like rotting flesh";
#endif
        if (you.species == SP_GHOUL)
#ifdef JP
            description += " - ∏‘¿Ω¡˜Ω∫∑ØøÓµ•!";
#else
            description += " - yum!";
#endif
        else
#ifdef JP
            description += ".";
#else
            description += ".";
#endif
        break;
    case 2:
#ifdef JP
        description += " ¿Ã æ«∏∂∑Œ∫Œ≈Õ ≈‰«“∞Õ ∞∞¿∫ æ«√Î∞° ¡ˆµ∂«œ∞‘ «≥∞‹ø¬¥Ÿ.";
#else
        description += " It is surrounded by a sickening stench.";
#endif
        break;
    case 3:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ∏µÁ ªÏæ∆¿÷¥¬ ∞Õø° ¥Î«— ¡ıø¿∑Œ ∫“≈∏ø¿∏£∞Ì ¿÷¥Ÿ.";
#else
        description += " It seethes with hatred of the living.";
#endif
        break;
    case 4:
#ifdef JP
        description += " ¿€¿∫ ¡÷»≤ªˆ µµ±˙∫Ò∫“¿Ã ¡÷¿ß∏¶ √„√ﬂ∏Á ≥Øæ∆¥Ÿ¥—¥Ÿ.";
#else
        description += " Tiny orange flames dance around it.";
#endif
        break;
    case 5:
#ifdef JP
        description += " ¿€¿∫ ∫∏∂Ûªˆ µµ±˙∫Ò∫“¿Ã ¡÷¿ß∏¶ √„√ﬂ∏Á ≥Øæ∆¥Ÿ¥—¥Ÿ.";
#else
        description += " Tiny purple flames dance around it.";
#endif
        break;
    case 6:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ±‚∫–≥™ª€ æ»∞≥∑Œ µ—∑¡ΩŒø© ¿÷¥Ÿ.";
#else
        description += " It is surrounded by a weird haze.";
#endif
        break;
    case 7:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ªÁæ««— ∫˚¿∏∑Œ µ—∑ØΩŒ¿Ã∞Ì ¿÷¥Ÿ.";
#else
        description += " It glows with a malevolent light.";
#endif
        break;
    case 8:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ∫–≥Î∑Œ ∞°µÊ¬˜ ¿÷¥Ÿ.";
#else
        description += " It looks incredibly angry.";
#endif
        break;
    case 9:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ¡°æ◊¿ª »Í∏Æ∞Ì ¿÷¥Ÿ.";
#else
        description += " It oozes with slime.";
#endif
        break;
    case 10:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ≤˜¿”æ¯¿Ã ƒß¿ª »Í∏Æ∞Ì ¿÷¥Ÿ.";
#else
        description += " It dribbles constantly.";
#endif
        break;
    case 11:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ¿¸Ω≈¿Ã ∞ı∆Œ¿Ã∑Œ µ⁄µ§ø© ¿÷¥Ÿ.";
#else
        description += " Mould grows all over it.";
#endif
        break;
    case 12:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ∫¥µÈæÓ ∫∏¿Œ¥Ÿ.";
#else
        description += " It looks diseased.";
#endif
        break;
    case 13:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ¥ÁΩ≈¿ª π´º≠øˆ«œ∞Ì ¿÷¥Ÿ. "
                       " ¥ÁΩ≈¿Ã ≥‡ºÆ¿ª π´º≠øˆ«œ∞Ì ¿÷µÌ¿Ã...";
#else
        description += " It looks as frightened of you as you are of it.";
#endif
        break;
    case 14:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ Ω…«— ∞Ê∑√¿ª ∞Ëº” ¿œ¿∏≈∞∏Èº≠ øÚ¡˜¿Œ¥Ÿ.";
#else
        description += " It moves in a series of hideous convulsions.";
#endif
        break;
    case 15:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ πœæÓ¡ˆ¡ˆ æ ¿ª ¡§µµ∑Œ øÏæ∆«œ∞‘ øÚ¡˜¿Œ¥Ÿ.";
#else
        description += " It moves with an unearthly grace.";
#endif
        break;
    case 16:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ¥ÁΩ≈¿« »•¿ª ≈Ω≥ª∞Ì ¿÷¥Ÿ!";
#else
        description += " It hungers for your soul!";
#endif
        break;
    case 17:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ¿±±‚∞° »Â∏£¥¬ πÃ≤Ù∑ØøÓ ±‚∏ß¿∏∑Œ µ» πﬂ¿⁄±π¿ª ≥≤±‚∞Ì ¿÷¥Ÿ.";
#else
        description += " It leaves a glistening oily trail.";
#endif
        break;
    case 18:
#ifdef JP
        description += " ¿Ã æ«∏∂¿« ∏Ω¿¿∫ ∏≈øÏ π›¬¶¿Ã∞Ì ¿÷¥Ÿ.";
#else
        description += " It shimmers before your eyes.";
#endif
        break;
    case 19:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ ∫˚≥™¥¬ º∂±§¿∏∑Œ µ—∑ØΩŒø© ¿÷¥Ÿ.";
#else
        description += " It is surrounded by a brilliant glow.";
#endif
        break;
    case 20:
#ifdef JP
        description += " ¿Ã æ«∏∂¥¬ æˆ√ª≥≠ »˚¿« ø¿∂Û∏¶ πﬂªÍ«œ∞Ì ¿÷¥Ÿ.";
#else
        description += " It radiates an aura of extreme power.";
#endif
        break;
    default:
        break;
    }

    return description;
}                               // end describe_demon()


//---------------------------------------------------------------
//
// describe_weapon
//
//---------------------------------------------------------------
static std::string describe_weapon( const item_def &item, char verbose)
{
    std::string description;

    description.reserve(200);

    description = "";

    if (is_fixed_artefact( item ))
    {
        if (item_ident( item, ISFLAG_KNOW_PROPERTIES ))
        {
            description += "$";

            switch (item.special)
            {
            case SPWPN_SINGING_SWORD:
#ifdef JP
                description += "¿Ã √‡∫ππﬁ¿∫ π´±‚¥¬ ¡÷¿Œø°∞‘ ≥Î∑°∏¶ ∫Œ∏£¥¬ ∞Õ∏∏¿ª ¡¡æ∆«—¥Ÿ. "
                               "π∞∑– ¡÷¿Œ¿« ¿«¡ˆøÕ¥¬ ªÛ∞¸æ¯¿Ã.. ";
#else
                description += "This blessed weapon loves nothing more "
                    "than to sing to its owner, "
                    "whether they want it to or not. ";
#endif
                break;
            case SPWPN_WRATH_OF_TROG:
#ifdef JP
                description += "¿“æÓπˆ∏Æ±‚ ¿¸±Ó¡ˆ, ∞Ì¥Î¿« Ω≈ ∆Æ∑Œ±◊∞° æ÷øÎ«œ¥¯ π´±‚¥Ÿ. "
                               "¿Ã π´±‚∏¶ »÷µŒ∏£¥¬ ¿⁄ø°∞‘ ««ø° ±æ¡÷∏∞ ±§∆˜«— ∫–≥Î∏¶ ¿œ¿∏≈≤¥Ÿ. ";
#else
                description += "This was the favourite weapon of "
                    "the old god Trog, before he lost it one day. "
                    "It induces a bloodthirsty berserker rage in "
                    "anyone who uses it to strike another. ";
#endif
                break;
            case SPWPN_SCYTHE_OF_CURSES:
#ifdef JP
                description += "¿Ã π´±‚ø°¥¬ ≤˚¬Ô«œ∞Ì ∏ˆº≠∏Æ√ƒ¡ˆ¥¬ ¿˙¡÷∞° ≥ª∑¡ ¿÷¥Ÿ.";
#else
                description += "This weapon carries a "
                    "terrible and highly irritating curse. ";
#endif
                break;
            case SPWPN_MACE_OF_VARIABILITY:
#ifdef JP
                description += "¿Ã π´±‚¥¬ ∫∞∑Œ πœ∞Ì ΩÕ¡ˆ æ ¥Ÿ.";
#else
                description += "It is rather unreliable. ";
#endif
                break;
            case SPWPN_GLAIVE_OF_PRUNE:
#ifdef JP
                description += "¿Ã π´±‚¥¬ ¡§Ω≈≥™∞£ Ω≈¿« √¢¡∂π∞∑Œ, "
                               "¥©±∏µÁ¡ˆ µÈ∞Ì ¿÷¿∏∏È ∫“±∏¿⁄∑Œ ∏∏µÈæÓ πˆ∏∞¥Ÿ. "
                               "¥Ÿ«‡»˜µµ ¿˙¡÷¥¬ √µ√µ»˜ ¡¯«‡µ«π«∑Œ, "
                               "¿·±Ò ªÁøÎ«œ¥¬ ∞Õ ª”¿Ã∂Û∏È ««∫Œ∞° ªÏ¬¶ ∫∏∂Ûªˆ¿Ã µ«∞Ì "
                               "æ‡∞£ ¡÷∏ß¿Ã ¡ˆ¥¬ ¡§µµ∑Œ ≥°≥æ ºˆ ¿÷¥Ÿ. ";
#else
                description += "It is the creation of a mad god, and "
                    "carries a curse which transforms anyone "
                    "possessing it into a prune. Fortunately, "
                    "the curse works very slowly, and one can "
                    "use it briefly with no consequences "
                    "worse than slightly purple skin and a few wrinkles. ";
#endif
                break;
            case SPWPN_SCEPTRE_OF_TORMENT:
#ifdef JP
                description += "¿Ã ªÁæ««— π´±‚¥¬ ¡¯¡§«— ¡ˆø¡¿« ∞ÌπÆµµ±∏¥Ÿ. ";
#else
                description += "This truly accursed weapon is "
                    "an instrument of Hell. ";
#endif
                break;
            case SPWPN_SWORD_OF_ZONGULDROK:
#ifdef JP
                description += "¿Ã ¿˝∏¡¿« π´±‚¥¬ ªÁøÎ¿⁄∏¶ ∏Í∏¡¿∏∑Œ ¿Ã≤ˆ¥Ÿ. ";
#else
                description += "This dreadful weapon is used "
                    "at the user's peril. ";
#endif
                break;
            case SPWPN_SWORD_OF_CEREBOV:
#ifdef JP
                description += "±‚∫–≥™ª€ ∫“±Ê¿Ã µ⁄∆≤∏∞ ƒÆ≥Ø¿ª µ—∑ØΩŒ∞Ì ¿÷¥Ÿ. ";
#else
                description += "Eerie flames cover its twisted blade. ";
#endif
                break;
            case SPWPN_STAFF_OF_DISPATER:
#ifdef JP
                description += "¿Ã ¿¸º≥¿« π´±‚¥¬ ¡ˆø¡¿« ∫–≥Î∏¶ πﬂ«“ ºˆ ¿÷¥Ÿ. ";
#else
                description += "This legendary item can unleash "
                    "the fury of Hell. ";
#endif
                break;
            case SPWPN_SCEPTRE_OF_ASMODEUS:
#ifdef JP
                description += "¿Ã π´±‚¥¬ æ«∏∂¿« ºˆ¿Â æ∆Ω∫∏µ•øÏΩ∫¿« »˚¿ª æ‡∞£ «∞∞Ì ¿÷¥Ÿ. ";
#else
                description += "It carries some of the powers of "
                    "the arch-fiend Asmodeus. ";
#endif
                break;
            case SPWPN_SWORD_OF_POWER:
#ifdef JP
                description += "¿Ã π´±‚¥¬ ∞≠¿⁄ø°∞‘ ∞≠∑¬«— »˚¿ª, "
                               "æ‡¿⁄ø°∞‘¥¬ π´∑¬«‘¿ª º±ªÁ«—¥Ÿ. ";
#else
                description += "It rewards the powerful with power "
                    "and the meek with weakness. ";
#endif
                break;
            case SPWPN_KNIFE_OF_ACCURACY:
#ifdef JP
                description += "¿Ã π´±‚¥¬ ∞≈¿« ∫¯≥™∞°¡ˆ æ ¥¬¥Ÿ. ";
#else
                description += "It is almost unerringly accurate. ";
#endif
                break;
            case SPWPN_STAFF_OF_OLGREB:
#ifdef JP
                description += "¥Î∏∂π˝ªÁ ø√±◊∑π∫Í∞° ¥¯¡Ø æÓµÚ∞°ø°º≠ ¡◊¿Ω¿ª ∏¬¿Ã«œ±‚ ¿¸±Ó¡ˆ "
                               "ªÁøÎ«œ¥¯ π´±‚∑Œ, ªÁøÎ¿⁄ø°∞‘ µ∂ø° ¥Î«— ≥ªº∫¿ª ∫Œø©«œ∏Á "
                               "∆˜¿Ã¡ ∞Ëø≠ ∏∂π˝¿ª ∞≠»≠Ω√≈≤¥Ÿ. "
                               "π´±‚ æ»ø° ¿·µÈæÓ ¿÷¥¬ ∏∂π˝¿ª πﬂµøΩ√ƒ— ªÁøÎ«“ ºˆµµ ¿÷¥Ÿ. ";
#else
                description += "It was the magical weapon wielded by the "
                    "mighty wizard Olgreb before he met his "
                    "fate somewhere within these dungeons. It "
                    "grants its wielder resistance to the "
                    "effects of poison and increases their "
                    "ability to use venomous magic, and "
                    "carries magical powers which can be evoked. ";
#endif
                break;
            case SPWPN_VAMPIRES_TOOTH:
#ifdef JP
                description += "π´Ω√π´Ω√«— »Ì«˜π´±‚. ";
#else
                description += "It is lethally vampiric. ";
#endif
                break;
            case SPWPN_STAFF_OF_WUCAD_MU:
#ifdef JP
                description += "ªÁøÎ¿⁄¿« ¡ˆ¥… ºˆƒ°ø° µ˚∂Û π´±‚¿« ¿ß∑¬¿Ã ∞·¡§µ»¥Ÿ. "
                               "¿Ã π´±‚∏¶ ªÁøÎ«œ¥¬ µ•¥¬ æ‡∞£¿« ¿ß«Ë¿Ã µ˚∏•¥Ÿ. ";
#else
                description += "Its power varies in proportion to "
                    "its wielder's intelligence. "
                    "Using it can be a bit risky. ";
#endif
                break;
            }

            description += "$";
        }
        else if (item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            // We know it's an artefact type weapon, but not what it does.
#ifdef JP
            description += "¿Ã π´±‚ø°¥¬ π∫∞° º˚∞‹¡¯ ¥…∑¬¿Ã ¿÷¿ª ∞Õ ∞∞¥Ÿ.";
#else
            description += "$This weapon may have some hidden properties.$";
#endif
        }
    }
    else if (is_unrandom_artefact( item )
        && strlen(unrandart_descrip(1, item)) != 0)
    {
        description += unrandart_descrip(1, item);
        description += "$";
    }
    else
    {
        if (verbose == 1)
        {
            switch (item.sub_type)
            {
            case WPN_CLUB:
#ifdef JP
                description += "π¨¡˜«— ≥™π´ ∞Ô∫¿. ";
#else
                description += "A heavy piece of wood. ";
#endif
                break;

            case WPN_MACE:
#ifdef JP
                description += "±‰ º’¿‚¿Ã ≥°ø° π´∞≈øÓ ºËπ∂ƒ°∞° ¥ﬁ∑¡¿÷¥Ÿ. ";
#else
                description += "A long handle "
                    "with a heavy lump on one end. ";
#endif
                break;

            case WPN_FLAIL:
#ifdef JP
                description += "∏ﬁ¿ÃΩ∫øÕ ∫ÒΩ¡«— π´±‚¡ˆ∏∏, º’¿‚¿ÃøÕ ºËπ∂ƒ°∞° ºËªÁΩΩ∑Œ ø¨∞·µ«æÓ ¿÷¥Ÿ¥¬ ¡°¿Ã ¥Ÿ∏£¥Ÿ. ";
#else
                description += "Like a mace, but with a length of chain "
                    "between the handle and the lump of metal. ";
#endif
                break;

            case WPN_DAGGER:
#ifdef JP
                description += "≥™¿Ã«¡∑Œº≠¥¬ ±‰ ∆Ì¿Ã∞Ì, ∞À¿∏∑Œº≠¥¬ ∏≈øÏ ¬™¿∫ ∆Ì¿Ã¥Ÿ. "
                               "µÈ∞Ì ¿˚¿ª ∫ß ºˆµµ ¿÷¿∏∏Á ¿˚¿ª «‚«ÿ ¥¯¡˙ ºˆµµ ¿÷¥Ÿ. ";
#else
                description += "A long knife or a very short sword, "
                    "which can be held or thrown. ";
#endif
                break;

            case WPN_KNIFE:
#ifdef JP
                description += "∞£¥‹«— æﬂøµøÎ ≥™¿Ã«¡∑Œ, π´±‚∫∏¥Ÿ¥¬ µµ±∏∑Œº≠ ¿˚«’«œ¥Ÿ. "
                               "∞Ì±‚∏¶ √≥∏Æ«œ¥¬µ• ∏≈øÏ ¿ØøÎ«œ¥Ÿ. ";
#else
                description += "A simple survival knife. "
                    "Designed more for utility than combat, "
                    "it looks quite capable of butchering a corpse. ";
#endif
                break;

            case WPN_MORNINGSTAR:
#ifdef JP
                description += "ªœ¡∑«— ∏¯¿∏∑Œ µ⁄µ§¿Œ ∏ﬁ¿ÃΩ∫. ";
#else
                description += "A mace covered in spikes. ";
#endif
                break;

            case WPN_SHORT_SWORD:
#ifdef JP
                description += "∫£±‚øÎ¿∏∑Œ ∏∏µÈæÓ¡¯ ¬™¿∫ ≥Ø¿« ∞À. ";
#else
                description += "A sword with a short, slashing blade. ";
#endif
                break;

            case WPN_LONG_SWORD:
#ifdef JP
                description += "∫£±‚øÎ¿∏∑Œ ∏∏µÈæÓ¡¯ ±‰ ≥Ø¿« ∞À. ";
#else
                description += "A sword with a long, slashing blade. ";
#endif
                break;

            case WPN_GREAT_SWORD:
#ifdef JP
                description += "≥Ø¿« ±Ê¿Ã∞° ∏≈øÏ ±Ê∏Á π´∞≈øÓ ∞À. º’¿‚¿Ã ∂««— ±Ê¥Ÿ. ";
#else
                description += "A sword with a very long, heavy blade "
                    "and a long handle. ";
#endif
                break;

            case WPN_SCIMITAR:
#ifdef JP
                description += "ƒÆ≥Ø¿Ã ±∏∫Œ∑Ø¡¯ ±Ê¥Ÿ∂ı ∞À. ";
#else
                description += "A long sword with a curved blade. ";
#endif
                break;

            case WPN_HAND_AXE:
#ifdef JP
                description += "πÈ∫¥¿¸¿Ã≥™ ≈ı√¥ø° øÎ¿Ã«œµµ∑œ ∏∏µÈæÓ¡¯ ¿€¿∫ µµ≥¢. ";
#else
                description += "An small axe designed for either hand combat "
                               "or throwing. ";
#endif
                               // "It might also make a good tool.";
                break;

            case WPN_BATTLEAXE:
#ifdef JP
                description += "æÁ≥Ø¿« ƒø¥Ÿ∂ı µµ≥¢. ";
#else
                description += "A large axe with a double-headed blade. ";
#endif
                break;

            case WPN_SPEAR:
#ifdef JP
                description += "º’ø° µÈ∞Ì ΩŒøÏ∞≈≥™ ≈ı√¥¿ª ¿ß«ÿ ∏∏µÈæÓ¡¯ "
                               "≥°ø° ªœ¡∑«— ≥Ø¿Ã ∫ŸæÓ ¿÷¥¬ ±‰ ∏∑¥Î±‚. ";
#else
                description += "A long stick with a pointy blade on one end, "
                    "to be held or thrown. ";
#endif
                break;

            case WPN_TRIDENT:
                description +=
#ifdef JP
                    "«—¬  ≥°¿« ≥Ø¿Ã º¬¿∏∑Œ ∞•∂Û¡¯ √¢. ";
#else
                    "A hafted weapon with three points at one end. ";
#endif
                break;

            case WPN_HALBERD:
                description +=
#ifdef JP
                    "«—¬  ≥°ø° µµ≥¢øÕ ªœ¡∑«— ∞°Ω√∞° µ∏æ∆ ¿÷¥¬ ±‰ ∫¿. ";
#else
                    "A long pole with a spiked axe head on one end. ";
#endif
                break;

            case WPN_SLING:
                description +=
#ifdef JP
                    "√µ∞˙ ∞°¡◊¿∏∑Œ ∏∏µÈæÓ¡¯, µπ¿ª ≥Ø∏Æ±‚ ¿ß«— µµ±∏. "
                    "≈´ ≈∏∞›¿∫ ±‚¥Î«“ ºˆ æ¯¥Ÿ.";
#else
                    "A piece of cloth and leather for launching stones, "
                    "which do a small amount of damage on impact. ";
#endif
                break;

            case WPN_BOW:
#ifdef JP
                description += "»≠ªÏ¿ª ΩÓ±‚ ¿ß«— µµ±∏. ±∏∫Œ∑Ø¡¯ ≥™π´øÕ «ˆ¿∏∑Œ ¿Ã∑ÁæÓ¡Æ ¿÷¥Ÿ. "
                               "¿¸≈ıΩ√ ªÛ¥Á«— «««ÿ∏¶ ¿‘»˙ ºˆ ¿÷¿∏∏Á "
                               "º˜∑√µµ∞° ≥Ù¿ªºˆ∑œ ¿ß∑¬¿Ã ∞≠«ÿ¡¯¥Ÿ. ";
#else
                description += "A curved piece of wood and string, "
                    "for shooting arrows. It does good damage in combat, "
                    "and a skilled user can use it to great effect. ";
#endif
                break;

            case WPN_BLOWGUN:
#ifdef JP
                description += "æÁ¬ ¿Ã ∂’∏∞ ∞°∫±∞Ì ±‰ ∞¸. ¿ß∑¬¿∫ ∏≈øÏ æ‡«œ∏Á, "
                               "¡÷∑Œ µ∂¿ª πŸ∏• πŸ¥√ µ˚¿ß∏¶ ∏’ ∞˜±Ó¡ˆ ΩÓ±‚ ¿ß«ÿ ªÁøÎµ»¥Ÿ. "
                               "º“¿Ω¿Ã ∞≈¿« æ¯¥Ÿ. ";
#else
                description += "A long, light tube, open at both ends.  Doing "
                    "very little damage,  its main use is to fire poisoned "
                    "needles from afar.  It makes very little noise. ";
#endif
                break;

            case WPN_CROSSBOW:
#ifdef JP
                description += "»≠ªÏ¿ª ΩÓ±‚ ¿ß«ÿ ∏∏µÈæÓ¡¯ ¿Âƒ°∑Œ, "
                               "πﬂªÁµ«±‚±Ó¡ˆ æ‡∞£ Ω√∞£¿Ã « ø‰«œ¥Ÿ. "
                               "¿¸≈ıΩ√ ≈´ ¿ß∑¬¿ª πﬂ»÷«—¥Ÿ. ";
#else
                description += "A piece of machinery used for firing bolts, "
                    "which takes some time to load and fire. "
                    "It does very good damage in combat. ";
#endif
                break;

            case WPN_HAND_CROSSBOW:
#ifdef JP
                description += "¥Ÿ∆Æ∏¶ πﬂªÁ«œ±‚ ¿ß«— ¿€¿∫ ºÆ±√. ";
#else
                description += "A small crossbow, for firing darts. ";
#endif
                break;

            case WPN_GLAIVE:
                description +=
#ifdef JP
                    "≥°ø° ≈©∞Ì π´∞≈øÓ ƒÆ≥Ø¿Ã ∫ŸæÓ ¿÷¥¬ ±‰ ∫¿. ";
#else
                    "A pole with a large, heavy blade on one end. ";
#endif
                break;

            case WPN_QUARTERSTAFF:
#ifdef JP
                description += "∞ﬂ∞Ì«— ≥™π´ ∏∑¥Î±‚. ";
#else
                description += "A sturdy wooden pole. ";
#endif
                break;

            case WPN_SCYTHE:
                description +=
#ifdef JP
                    "≥Û±‚±∏∑Œº≠, ¿¸≈ıø°¥¬ ∫Œ¿˚«’«œ¥Ÿ. ";
#else
                    "A farm implement, usually unsuited to combat. ";
#endif
                break;

            case WPN_GIANT_CLUB:
#ifdef JP
                description += "∞≈¥Î«— ≥™π´ ∞Ô∫¿¿Ã¥Ÿ. "
                               "ø¿øÏ∞≈¿« º’ø° ∏¬∞‘ ∏∏µÈæÓ¡Æ ¿÷¥Ÿ. ";
#else
                description += "A giant lump of wood, "
                    "shaped for an ogre's hands. ";
#endif
                break;

            case WPN_GIANT_SPIKED_CLUB:
                description +=
#ifdef JP
                    "∞≈¥Î«— ≥™π´ ∞Ô∫¿¿« «—¬  ≥°ø° ∞°Ω√∞° π⁄«Ù ¿÷¥Ÿ. ";
#else
                    "A giant lump of wood with sharp spikes at one end. ";
#endif
                break;

            case WPN_EVENINGSTAR:
#ifdef JP
                description += "∏¥◊Ω∫≈∏¿« π›¥Îµ«¥¬ π´±‚. ";
#else
                description += "The opposite of a morningstar. ";
#endif
                break;

            case WPN_QUICK_BLADE:
#ifdef JP
                description += "∏≈øÏ ¿€∞Ì ∫Ò¡§ªÛ¿˚¿∏∑Œ ¿Á∫¸∏• ∞À. "; // dio "¿ÃªÛ«“ ¡§µµ∑Œ" ¥¬ æÓ∂≤¡ˆ..
#else
                description += "A small and magically quick sword. ";
#endif
                break;

            case WPN_KATANA:
#ifdef JP
                description += "∏≈øÏ ±Õ«œ∞Ì ¿ß∑¬¿˚¿Œ ¿Ã±π¿« π´±‚¥Ÿ. "
                               "ƒÆ≥Ø¿Ã ∏≈øÏ ±Ê∞Ì, «—¬ ø°∏∏ ≥Ø¿Ã º≠ ¿÷¥Ÿ. ";
#else
                description += "A very rare and extremely effective "
                    "imported weapon, featuring a long "
                    "single-edged blade. ";
#endif
                break;

            case WPN_EXECUTIONERS_AXE:
#ifdef JP
                description += "∞≈¥Î«— µµ≥¢. ";
#else
                description += "A huge axe. ";
#endif
                break;

            case WPN_DOUBLE_SWORD:
                description +=
#ifdef JP
                    "2∞≥¿« ∏≈øÏ øπ∏Æ«— ƒÆ≥Ø¿ª ∞°¡¯ ∏∂π˝π´±‚. ";
#else
                    "A magical weapon with two razor-sharp blades. ";
#endif
                break;

            case WPN_TRIPLE_SWORD:
#ifdef JP
                description += "3∞≥¿« ∞≈¥Î«œ∞Ì ∏≈øÏ øπ∏Æ«— ƒÆ≥Ø¿ª ∞°¡ˆ∞Ì ¿÷¥¬ ∏∂π˝π´±‚. ";
#else
                description += "A magical weapon with three "
                    "great razor-sharp blades. ";
#endif
                break;

            case WPN_HAMMER:
#ifdef JP
                description += "¡÷∑Œ ∏¯¿ª π⁄¥¬µ• ªÁøÎµ«¥¬ µµ±∏¿Ã¡ˆ∏∏, ¿¸≈ıø° ¿˚«’«œµµ∑œ ∞≥∑Æµ«æ˙¥Ÿ. ";
#else
                description += "The kind of thing you hit nails with, "
                    "adapted for battle. ";
#endif
                break;

            case WPN_ANCUS:
#ifdef JP
                description += "≥Øƒ´∑ŒøÓ ¿Ãª°¿Ã Ω…æÓ¡Æ ¿÷¥¬ ƒø¥Ÿ∂ı ∞Ô∫¿¿Ã¥Ÿ. ";
#else
                description += "A large and vicious toothed club. ";
#endif
                break;

            case WPN_WHIP:
#ifdef JP
                description += "∫∏≈Î¿« √§¬Ô¿Ã¥Ÿ. ";
#else
                description += "A whip. ";
#endif
                break;

            case WPN_SABRE:
#ifdef JP
                description += "¡ﬂ∞£ ±Ê¿Ã¿« ∫£±‚ ¿ß«— ∞À. ";
#else
                description += "A sword with a medium length slashing blade. ";
#endif
                break;

            case WPN_DEMON_BLADE:
                description +=
#ifdef JP
                    "¡ˆø¡¿« ∫“±Ê∑Œ ¡¶∑√µ» π´±‚π´±‚«— π´±‚¥Ÿ. ";
#else
                    "A terrible weapon, forged in the fires of Hell. ";
#endif
                break;

            case WPN_DEMON_WHIP:
#ifdef JP
                description += "¡ˆø¡∫“¿« Ω…ø¨ø°º≠ ∏∏µÈæÓ¡¯ π´Ω√π´Ω√«— π´±‚¥Ÿ. ";
#else
                description += "A terrible weapon, woven "
                    "in the depths of the inferno. ";
#endif
                break;

            case WPN_DEMON_TRIDENT:
                description +=
#ifdef JP
                    "∫“∞˙ ¿Ø»≤¿∏∑Œ ∫˙æÓ≥Ω π´Ω√π´Ω√«— π´±‚¥Ÿ. ";
#else
                    "A terrible weapon, molded by fire and brimstone. ";
#endif
                break;

            case WPN_BROAD_AXE:
#ifdef JP
                description += "ƒø¥Ÿ∂ı ≥Ø¿« µµ≥¢. ";
#else
                description += "An axe with a large blade. ";
#endif
                break;

            case WPN_WAR_AXE:
#ifdef JP
                description += "πÈ∫¥¿¸¿ª ¿ß«ÿ ∏∏µÈæÓ¡¯ µµ≥¢. ";
#else
                description += "An axe intended for hand to hand combat. ";
#endif
                break;

            case WPN_SPIKED_FLAIL:
                description +=
#ifdef JP
                    "ºÌµ¢æÓ∏Æ ∫Œ∫–ø° ƒø¥Ÿ∂ı ∞°Ω√µÈ¿Ã µ∏æ∆ ¿÷¥¬ «¡∑π¿œ. ";
#else
                    "A flail with large spikes on the metal lump. ";
#endif
                break;

            case WPN_GREAT_MACE:
#ifdef JP
                description += "ƒø¥Ÿ∂˛∞Ì π´∞≈øÓ ∏ﬁ¿ÃΩ∫¥Ÿ. ";
#else
                description += "A large and heavy mace. ";
#endif
                break;

            case WPN_GREAT_FLAIL:
#ifdef JP
                description += "∏≈øÏ ƒø¥Ÿ∂˛∞Ì π´∞≈øÓ «¡∑π¿œ¿Ã¥Ÿ. ";
#else
                description += "A large and heavy flail. ";
#endif
                break;

            case WPN_FALCHION:
#ifdef JP
                description += "≥Ø¿Ã ≥–¿∫, ∫£±‚ ¿ß«— ∞À. ";
#else
                description += "A sword with a broad slashing blade. ";
#endif
                break;

            default:
#ifdef JP
                DEBUGSTR("æÀºˆæ¯¥¬ π´±‚");
#else
                DEBUGSTR("Unknown weapon");
#endif
            }

            description += "$";
        }
    }

    if (verbose == 1 && !launches_things( item.sub_type ))
    {
#ifdef JP
        description += "$µ•πÃ¡ˆ µÓ±ﬁ : ";
#else
        description += "$Damage rating: ";
#endif
        append_value(description, property( item, PWPN_DAMAGE ), false);

#ifdef JP
        description += "$∏Ì¡ﬂ∑¸ µÓ±ﬁ : ";
#else
        description += "$Accuracy rating: ";
#endif
        append_value(description, property( item, PWPN_HIT ), true);

#ifdef JP
        description += "$±‚∫ª ∞¯∞› µÙ∑π¿Ã : ";
#else
        description += "$Base attack delay: ";
#endif
        append_value(description, property( item, PWPN_SPEED ) * 10, false);
        description += "%%";
    }
    description += "$";

    if (!is_fixed_artefact( item ))
    {
        int spec_ench = get_weapon_brand( item );

        if (!is_random_artefact( item ) && verbose == 0)
            spec_ench = SPWPN_NORMAL;

        // special weapon descrip
        if (spec_ench != SPWPN_NORMAL && item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            description += "$";

            switch (spec_ench)
            {
            case SPWPN_FLAMING:
#ifdef JP
                description += "º’ø° ¡„æ˙¿ª ∂ß ∫“±Ê¿ª ≥ªª’¿∏∏Á, ¥Î∫Œ∫–¿« ¿˚ø°∞‘ √ﬂ∞° «««ÿ∏¶ ¿‘»˜∞Ì "
                               "∫“ø° æ‡«— ¿˚ø°∞‘¥¬ √÷¥Î µŒπË¿« µ•πÃ¡ˆ∏¶ ¡ÿ¥Ÿ. ";
#else
                description += "It emits flame when wielded, "
                    "causing extra injury to most foes "
                    "and up to double damage against "
                    "particularly susceptible opponents. ";
#endif
                break;
            case SPWPN_FREEZING:
#ifdef JP
                description += "¿˚¿ª æÛ∑¡ √ﬂ∞°¿˚¿Œ «««ÿ∏¶ ¿‘»˜µµ∑œ ∏∂π˝¿Ã ∫Œø©µ«æÓ ¿÷¥Ÿ. "
                               "≥√±‚ø° æ‡«— ¿˚ø°∞‘¥¬ √÷¥Î µŒπË¿« µ•πÃ¡ˆ∏¶ ¿‘»˙ ºˆ ¿÷¥Ÿ. ";
#else
                description += "It has been specially enchanted to "
                    "freeze those struck by it, causing "
                    "extra injury to most foes and "
                    "up to double damage against "
                    "particularly susceptible opponents. ";
#endif
                break;
            case SPWPN_HOLY_WRATH:
#ifdef JP
                description += "°∏ª˛¿Ã¥◊ ø¯°π¿« √‡∫π¿ª πﬁ¿∫ π´±‚∑Œ, "
                               "æµ•µÂø°∞‘ √ﬂ∞°¿˚¿Œ ≈∏∞›¿ª ¡Ÿ ºˆ ¿÷¿∏∏Á "
                               "¡ˆø¡¿Ã≥™ ∆«µ•∏¥œøÚ¿« ªÁæ««— ¡∏¿ÁµÈø°∞‘¥¬ ¥ıøÌ ¥ı ∞≠∑¬«— «««ÿ∏¶ ¿‘»˙ ºˆ ¿÷¥Ÿ.";
#else
                description += "It has been blessed by the Shining One "
                    "to harm undead and cause great damage to "
                    "the unholy creatures of Hell or Pandemonium. ";
#endif
                break;
            case SPWPN_ELECTROCUTION:
#ifdef JP
                description += "¿˚ø°∞‘ ∏Ì¡ﬂ«ﬂ¿ª ∂ß ¿Ãµ˚±›æø ¿¸±‚ ø°≥ ¡ˆ∏¶ πÊ√‚«œø© "
                               "ƒ°∏Ì¿˚¿Œ «««ÿ∏¶ ¿‘»˙ ºˆ ¿÷¥Ÿ.";
#else
                description += "Occasionally upon striking a foe "
                    "it will discharge some electrical energy "
                    "and cause terrible harm. ";
#endif
                break;
            case SPWPN_ORC_SLAYING:
#ifdef JP
                description += "ø¿≈©¡∑ø°∞‘ ∆Ø»˜ »ø∞˙¿˚¿Ã¥Ÿ.";
#else
                description += "It is especially effective against "
                    "all of orcish descent. ";
#endif
                break;
            case SPWPN_VENOM:
                if (launches_things( item.sub_type ))
#ifdef JP
                    description += "πﬂªÁµ«¥¬ »≠ªÏ¿Ã≥™ ¥Ÿ∆Æ µÓø° µ∂¿ª ∫Œø©«—¥Ÿ.";
#else
                    description += "It poisons the unbranded ammo it fires. ";
#endif
                else
#ifdef JP
                    description += "¿Ã∞Õø° ªÛ√≥∏¶ ¿‘¿∏∏È µ∂ø° ∞…∏Æ∞‘ µ»¥Ÿ.";
#else
                    description += "It poisons the flesh of those it strikes. ";
#endif
                break;
            case SPWPN_PROTECTION:
#ifdef JP
                description += "ªÁøÎ¿⁄∏¶ «««ÿ∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ(+5 AC). ";
#else
                description += "It protects the one who wields it against "
                    "injury (+5 to AC). ";
#endif
                break;
            case SPWPN_DRAINING:
#ifdef JP
                description += "¿˚¿« ª˝∏Ì∑¬¿ª »Ìºˆ«œ¥¬, "
                               "Ω«∑Œ π´Ω√π´Ω√«— π´±‚¥Ÿ.";
#else
                description += "A truly terrible weapon, "
                    "it drains the life of those it strikes. ";
#endif
                break;
            case SPWPN_SPEED:
                if (launches_things( item.sub_type ))
                {
#ifdef JP
                    description += "ªÁøÎ¿⁄∞° µŒπË¿« º”µµ∑Œ ªÁ∞›¿ª «“ ºˆ ¿÷µµ∑œ «—¥Ÿ.";
#else
                    description += "It allows its wielder to fire twice when "
                           "they would otherwise have fired only once. ";
#endif
                }
                else
                {
#ifdef JP
                    description += "ªÁøÎ¿⁄∞° µŒπË¿« º”µµ∑Œ ∞¯∞›¿ª «“ ºˆ ¿÷µµ∑œ «—¥Ÿ.";
#else
                    description += "It allows its wielder to attack twice when "
                           "they would otherwise have struck only once. ";
#endif
                }
                break;
            case SPWPN_VORPAL:
#ifdef JP
                description += "¿˚ø°∞‘ √ﬂ∞° ≈∏∞›¿ª ¿‘»˘¥Ÿ.";
#else
                description += "It inflicts extra damage upon your enemies. ";
#endif
                break;
            case SPWPN_FLAME:
#ifdef JP
                description += "πﬂªÁµ«¥¬ »≠ªÏ¿Ã≥™ ¥Ÿ∆Æ µÓ¿ª »≠ø∞¿∏∑Œ ∞®Ω—¥Ÿ.";
#else
                description += "It turns projectiles fired from it into "
                    "bolts of fire. ";
#endif
                break;
            case SPWPN_FROST:
#ifdef JP
                description += "πﬂªÁµ«¥¬ »≠ªÏ¿Ã≥™ ¥Ÿ∆Æ µÓ¿ª ≥√±‚∑Œ ∞®Ω—¥Ÿ.";
#else
                description += "It turns projectiles fired from it into "
                    "bolts of frost. ";
#endif
                break;
            case SPWPN_VAMPIRICISM:
#ifdef JP
                description += "√ﬂ∞° ≈∏∞›¿ª ¿‘»˜¡ˆ¥¬ æ ¡ˆ∏∏, ªÏæ∆¿÷¥¬ ªÛ¥Îø°∞‘ "
                               "«««ÿ∏¶ ¿‘»˙ ∞ÊøÏ ªÁøÎ¿⁄¿« √º∑¬¿ª æ‡∞£ »∏∫πΩ√≈≤¥Ÿ.";
#else
                description += "It inflicts no extra harm, "
                    "but heals its wielder somewhat when "
                    "he or she strikes a living foe. ";
#endif
                break;
            case SPWPN_DISRUPTION:
#ifdef JP
                description += "°∏¡¯°π¿« √‡∫π¿Ã ≥ª∑¡¡¯ π´±‚∑Œ, æµ•µÂ∏¶ ªÛ¥Î«“ ∞ÊøÏ "
                               "√÷¥Î ≥◊πË¿« µ•πÃ¡ˆ±Ó¡ˆ ¿‘»˙ ºˆ∞° ¿÷¥Ÿ.";
#else
                description += "It is a weapon blessed by Zin, "
                    "and can inflict up to fourfold damage "
                    "when used against the undead. ";
#endif
                break;
            case SPWPN_PAIN:
#ifdef JP
                description += "∞≠∑…º˙ ∏∂π˝ø° º˜∑√µ» ¿⁄∞° ªÁøÎ«œ∏È "
                               "ªÏæ∆¿÷¥¬ ¿˚ø°∞‘ √ﬂ∞° µ•πÃ¡ˆ∏¶ ¿‘»˙ ºˆ ¿÷¥Ÿ.";
#else
                description += "In the hands of one skilled in "
                    "necromantic magic it inflicts "
                    "extra damage on living creatures. ";
#endif
                break;
            case SPWPN_DISTORTION:
#ifdef JP
                description += "¡÷¿ß¿« ∞¯∞£¿ª ∫Ò∆≤∞Ì ø÷∞ÓΩ√≈≤¥Ÿ.";
#else
                description += "It warps and distorts space around it. ";
#endif
                break;
            case SPWPN_REACHING:
#ifdef JP
                description += "π´±‚∏¶ πﬂµøΩ√ƒ—º≠ ªÁ¡§∞≈∏Æ∏¶ ¥√¿œ ºˆ ¿÷¥Ÿ.";
#else
                description += "It can be evoked to extend its reach. ";
#endif
                break;
            }
        }

        if (is_random_artefact( item ))
        {
            if (item_ident( item, ISFLAG_KNOW_PROPERTIES ))
            {
                unsigned int old_length = description.length();
                randart_descpr( description, item );

                if (description.length() == old_length)
                    description += "$";
            }
            else if (item_ident( item, ISFLAG_KNOW_TYPE ))
            {
#ifdef JP
                description += "$¿Ã π´±‚ø°¥¬ øÿ¡ˆ º˚∞‹¡¯ ¥…∑¬¿Ã ¿÷¿ª ∞Õ ∞∞¥Ÿ.";
#else
                description += "$This weapon may have some hidden properties.$";
#endif
            }
        }
        else if (spec_ench != SPWPN_NORMAL && item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            description += "$";
        }
    }

    if (item_known_cursed( item ))
    {
#ifdef JP
        description += "$¿Ã π´±‚¥¬ ¿˙¡÷∞° ∞…∑¡ ¿÷¥Ÿ.";
#else
        description += "$It has a curse placed upon it.";
#endif
    }

    if (verbose == 1 && !launches_things( item.sub_type ))
    {
#ifdef USE_NEW_COMBAT_STATS
        const int str_weight = weapon_str_weight( item.base_type, item.sub_type );

        if (str_weight >= 8)
#ifdef JP
            description += "$¿Ã π´±‚¥¬ »˚¿Ã ∞≠«— ¿⁄ø°∞‘ ∏≈øÏ ¿˚«’«œ¥Ÿ.";
#else
            description += "$This weapon is best used by the strong.";
#endif
        else if (str_weight > 5)
#ifdef JP
            description += "$¿Ã π´±‚¥¬ »˚¿Ã ∞≠«— ¿⁄ø°∞‘ ¿˚«’«œ¥Ÿ.";
#else
            description += "$This weapon is better for the strong.";
#endif
        else if (str_weight <= 2)
#ifdef JP
            description += "$¿Ã π´±‚¥¬ øÚ¡˜¿”¿Ã πŒ√∏«— ¿⁄ø°∞‘ ∏≈øÏ ¿˚«’«œ¥Ÿ.";
#else
            description += "$This weapon is best used by the dexterous.";
#endif
        else if (str_weight < 5)
#ifdef JP
            description += "$¿Ã π´±‚¥¬ øÚ¡˜¿”¿Ã πŒ√∏«— ¿⁄ø°∞‘ ¿˚«’«œ¥Ÿ.";
#else
            description += "$This weapon is better for the dexterous.";
#endif
#endif

        switch (hands_reqd_for_weapon(item.base_type, item.sub_type))
        {
        case HANDS_ONE_HANDED:
#ifdef JP
            description += "$«— º’ π´±‚¥Ÿ.";
#else
            description += "$It is a one handed weapon.";
#endif
            break;
        case HANDS_ONE_OR_TWO_HANDED:
#ifdef JP
            description += "$«— º’¿∏∑Œ ªÁøÎ«“ ºˆµµ ¿÷¡ˆ∏∏, µŒ º’¿∏∑Œ(πÊ∆– æ¯¿Ã) "
                           "ªÁøÎ«“ ∞ÊøÏ ¥ıøÌ »ø∞˙¿˚¿Ã¥Ÿ.";
#else
            description += "$It can be used with one hand, or more "
                    "effectively with two (i.e. when not using a shield).";
#endif
            break;
        case HANDS_TWO_HANDED:
#ifdef JP
            description += "$æÁº’ π´±‚¥Ÿ.";
#else
            description += "$It is a two handed weapon.";
#endif
            break;
        }
    }

    if (!is_random_artefact( item ))
    {
        switch (get_equip_race( item ))
        {
        case ISFLAG_DWARVEN:
#ifdef JP
            description += "$ªÛ¥Á»˜ ∂ŸæÓ≥≠ ºÿææ∑Œ ∏∏µÈæÓ¡Æ, ∏≈øÏ ∆∞∆∞«œ¥Ÿ.";
#else
            description += "$It is well-crafted and very durable.";
#endif
            break;
        }

        if (launches_things( item.sub_type ))
        {
            switch (get_equip_race( item ))
            {
            case ISFLAG_DWARVEN:
#ifdef JP
                description += "$µÂøˆ«¡¡∑ »≠ªÏ¿Ã≥™ ¥Ÿ∆Æ µÓ¿ª ªÁøÎ«“ ∂ß "
                               "∞°¿Â ≈´ ¿ß∑¬¿ª πﬂ»÷«—¥Ÿ.";
#else
                description += "$It is most deadly when used with "
                    "dwarven ammunition.";
#endif
                break;
            case ISFLAG_ELVEN:
#ifdef JP
                description += "$ø§«¡¡∑ »≠ªÏ¿Ã≥™ ¥Ÿ∆Æ µÓ¿ª ªÁøÎ«“ ∂ß "
                               "∞°¿Â ≈´ ¿ß∑¬¿ª πﬂ»÷«—¥Ÿ.";
#else
                description += "$It is most deadly when used with "
                    "elven ammunition.";
#endif
                break;
            case ISFLAG_ORCISH:
#ifdef JP
                description += "$ø¿≈©¡∑ »≠ªÏ¿Ã≥™ ¥Ÿ∆Æ µÓ¿ª ªÁøÎ«“ ∂ß "
                               "∞°¿Â ≈´ ¿ß∑¬¿ª πﬂ»÷«—¥Ÿ.";
#else
                description += "$It is most deadly when used with "
                    "orcish ammunition.";
#endif
                break;
            }
        }
    }

    if (verbose == 1)
    {
#ifdef JP
        description += "$¿Ã∞Õ¿∫ ";
#else
        description += "$It falls into the";
#endif

        switch (item.sub_type)
        {
        case WPN_SLING:
#ifdef JP
            description += "≈ıºÆ±‚(µπ∆»∏≈)¿« «— ¡æ∑˘¿Ã¥Ÿ. "; // dio ¿˚¥Á«— øÎæÓ º±≈√¡ª.. m¥©∏£∏È ≥™ø¿¥¬ ±‚º˙ ∏Ò∑œø° ªÁøÎµ» øÎæÓøÕ ∞∞æ∆æﬂ «“µÌ
#else
            description += " 'slings' category. ";
#endif
            break;
        case WPN_BOW:
#ifdef JP
            description += "»∞∑˘∑Œ ∫–∑˘µ»¥Ÿ. "; // dio ¬ﬁøÌ.. ∏∂¬˘∞°¡ˆ
#else
            description += " 'bows' category. ";
#endif
            break;
        case WPN_HAND_CROSSBOW:
        case WPN_CROSSBOW:
#ifdef JP
            description += "ºÆ±√∑˘∑Œ ∫–∑˘µ»¥Ÿ. ";
#else
            description += " 'crossbows' category. ";
#endif
            break;
        case WPN_BLOWGUN:
#ifdef JP
            description += "¥Ÿ∆Æ∑˘∑Œ ∫–∑˘µ»¥Ÿ. ";
#else
            description += " 'darts' category. ";
#endif
            break;
        default:
            // Melee weapons
            switch (weapon_skill(item.base_type, item.sub_type))
            {
            case SK_SHORT_BLADES:
#ifdef JP
                description += "¥‹∞À∑˘∑Œ ∫–∑˘µ»¥Ÿ. ";
#else
                description += " 'short blades' category. ";
#endif
                break;
            case SK_LONG_SWORDS:
#ifdef JP
                description += "¿Â∞À∑˘∑Œ ∫–∑˘µ»¥Ÿ. ";
#else
                description += " 'long swords' category. ";
#endif
                break;
            case SK_AXES:
#ifdef JP
                description += "µµ≥¢∑˘∑Œ ∫–∑˘µ»¥Ÿ. ";
#else
                description += " 'axes' category. ";
#endif
                break;
            case SK_MACES_FLAILS:
#ifdef JP
                description += "µ–±‚∑˘∑Œ ∫–∑˘µ»¥Ÿ. ";
#else
                description += " 'maces and flails' category. ";
#endif
                break;
            case SK_POLEARMS:
#ifdef JP
                description += "√¢∫¿∑˘ π´±‚∑Œ ∫–∑˘µ»¥Ÿ. ";
#else
                description += " 'pole-arms' category. ";
#endif
                break;
            case SK_STAVES:
#ifdef JP
                description += "¡ˆ∆Œ¿Ã∑Œ ∫–∑˘µ»¥Ÿ. "; // dio Ω∫≈¬«¡..? ¡ˆ∆Œ¿Ã...?
#else
                description += " 'staves' category. ";
#endif
                break;
            default:
#ifdef JP
                description += "πˆ±◊∑Œ ∫–∑˘µ»¥Ÿ.";
                DEBUGSTR("Unknown weapon type");
#else
                description += " 'bug' category. ";
                DEBUGSTR("Unknown weapon type");
#endif
                break;
            }
        }
    }

    return (description);
}


//---------------------------------------------------------------
//
// describe_ammo
//
//---------------------------------------------------------------
static std::string describe_ammo( const item_def &item )
{
    std::string description;

    description.reserve(64);

    switch (item.sub_type)
    {
    case MI_STONE:
#ifdef JP
        description += "¿€¿∫ µπ. ";
#else
        description += "A stone. ";
#endif
        break;
    case MI_ARROW:
#ifdef JP
        description += "»≠ªÏ. ";
#else
        description += "An arrow. ";
#endif
        break;
    case MI_NEEDLE:
#ifdef JP
        description += "πŸ¥√. ";
#else
        description += "A needle. ";
#endif
        break;
    case MI_BOLT:
#ifdef JP
        description += "ºÆ±√øÎ »≠ªÏ. ";
#else
        description += "A crossbow bolt. ";
#endif
        break;
    case MI_DART:
#ifdef JP
        description += "º“«¸¿« ≈ı√¥ π´±‚. ";
#else
        description += "A small throwing weapon. ";
#endif
        break;
    case MI_LARGE_ROCK:
#ifdef JP
        description += "πŸ¿ß. ∞≈¿ŒµÈ¿Ã ¥¯¡ˆ¥¬ µ• ªÁøÎ«—¥Ÿ. ";
#else
        description += "A rock, used by giants as a missile. ";
#endif
        break;
    case MI_EGGPLANT:
#ifdef JP
        description += "∫∏∂Ûªˆ æﬂ√§¿« «— ¡æ∑˘. "
                       "∞‘¿” ¡ﬂ ¿Ã π∞√º∞° µÓ¿Â«ﬂ¥Ÿ¥¬ ∞Õ¿∫ πˆ±◊¿« ¡∏¿Á "
                       "(∂«¥¬ ∞‘¿Ã∏”¿« ∫Œ¡§«‡¿ß)∏¶ ≥™≈∏≥Ω¥Ÿ. ";
#else
        description += "A purple vegetable. "
            "The presence of this object in the game "
            "indicates a bug (or some kind of cheating on your part). ";
#endif
        break;
    default:
#ifdef JP
        DEBUGSTR("æÀ∑¡¡ˆ¡ˆ æ ¿∫ »≠ªÏ, ¥Ÿ∆Æ, µπ µÓ¿« ≈∫æ‡. ");
#else
        DEBUGSTR("Unknown ammo type");
#endif
        break;
    }

    if (item.special != 0 && item_ident( item, ISFLAG_KNOW_TYPE ))
    {
        switch (item.special)
        {
        case 1:
#ifdef JP
            description += "$¿˚¿˝«— µµ±∏∏¶ ªÁøÎ«œø© πﬂªÁ«ﬂ¿ª ∞ÊøÏ, ∫“¿« »≠ªÏ¿Ã µ«æÓ ≥™∞£¥Ÿ. ";
#else
            description += "$When fired from an appropriate launcher, "
                "it turns into a bolt of flame. ";
#endif
            break;
        case 2:
#ifdef JP
            description += "$¿˚¿˝«— µµ±∏∏¶ ªÁøÎ«œø© πﬂªÁ«ﬂ¿ª ∞ÊøÏ, æÛ¿Ω¿« »≠ªÏ¿Ã µ«æÓ ≥™∞£¥Ÿ. ";
#else
            description += "$When fired from an appropriate launcher, "
                "it turns into a bolt of ice. ";
#endif
            break;
        case 3:
        case 4:
#ifdef JP
            description += "$µ∂¿Ã πﬂ∂Û¡Æ ¿÷¥Ÿ. ";
#else
            description += "$It is coated with poison. ";
#endif
            break;
        }
    }

    description += "$";

    return (description);
}


//---------------------------------------------------------------
//
// describe_armour
//
//---------------------------------------------------------------
static std::string describe_armour( const item_def &item, char verbose )
{
    std::string description;

    description.reserve(200);

    if (is_unrandom_artefact( item )
        && strlen(unrandart_descrip(1, item)) != 0)
    {
        description += "$";
        description += unrandart_descrip(1, item);
        description += "$$";
    }
    else
    {
        if (verbose == 1)
        {
            switch (item.sub_type)
            {
            case ARM_ROBE:
#ifdef JP
                description += "√µ¿∏∑Œ ∏∏µÈæÓ¡¯ ∑Œ∫Í. ";
#else
                description += "A cloth robe. ";
#endif
                break;
            case ARM_LEATHER_ARMOUR:
#ifdef JP
                description += "∞°¡◊¿ª ∞Ê»≠Ω√ƒ— ∏∏µÁ ∞©ø . ";
#else
                description += "A suit made of hardened leather. ";
#endif
                break;
            case ARM_RING_MAIL:
#ifdef JP
                description += "∞°¡◊ ∞©ø  ¿ßø° ¿€¿∫ ±›º” ∞Ì∏ÆµÈ¿Ã µ§ø© ¿÷¥Ÿ. ";
#else
                description += "A leather suit covered in little rings. ";
#endif
                break;
            case ARM_SCALE_MAIL:
                description +=
#ifdef JP
                    "∞°¡◊ ∞©ø  ¿ßø° ¿€¿∫ √∂∞©µÈ¿Ã µ§ø© ¿÷¥Ÿ. ";
#else
                    "A leather suit covered in little metal plates. ";
#endif
                break;
            case ARM_CHAIN_MAIL:
#ifdef JP
                description += "±›º” ªÁΩΩ∑Œ ¬•ø©¡¯ ∞©ø . ";
#else
                description += "A suit made of interlocking metal rings. ";
#endif
                break;
            case ARM_SPLINT_MAIL:
#ifdef JP
                description += "±›º” πÃ¥√∑Œ ∏∏µÈæÓ¡¯ ∞©ø . ";
#else
                description += "A suit made of splints of metal. ";
#endif
                break;
            case ARM_BANDED_MAIL:
#ifdef JP
                description += "±›º” ∂Ï∑Œ ∏∏µÈæÓ¡¯ ∞©ø . ";
#else
                description += "A suit made of bands of metal. ";
#endif
                break;
            case ARM_PLATE_MAIL:
#ifdef JP
                description += "ºËπÃ¥√∞˙ ƒø¥Ÿ∂ı ∆«±›¿∏∑Œ ∏∏µÈæÓ¡¯ ∞©ø . ";
#else
                description += "A suit of mail and large plates of metal. ";
#endif
                break;
            case ARM_SHIELD:
                description +=
#ifdef JP
                    "¿˚¿« ∞¯∞›¿ª ∏∑±‚ ¿ß«ÿ ∆»ø° π≠æÓ ¿Â¬¯«œ¥¬ ±›º” πÊ∆–. "
                    "±◊ ¡ﬂ∑Æ ∂ßπÆø° øÚ¡˜¿”¿Ã µ–»≠µ«æÓ "
                    "∞¯∞› º”µµ∞° æ‡∞£ ¥¿∑¡¡˙ ºˆ ¿÷¥Ÿ. ";
#else
                    "A piece of metal, to be strapped on one's arm. "
                    "It is cumbersome to wear, and slightly slows "
                    "the rate at which you may attack. ";
#endif
                break;
            case ARM_CLOAK:
#ifdef JP
                description += "√µ¿∏∑Œ ∏∏µÈæÓ¡¯ ∏¡≈‰. ";
#else
                description += "A cloth cloak. ";
#endif
                break;

            case ARM_HELMET:
                switch (get_helmet_type( item ))
                {
                case THELM_HELMET:
                case THELM_HELM:
#ifdef JP
                    description += "±›º”¡¶ ≈ı±∏. ";
#else
                    description += "A piece of metal headgear. ";
#endif
                    break;
                case THELM_CAP:
#ifdef JP
                    description += "√µ¿Ã≥™ ∞°¡◊¿∏∑Œ ∏∏µÈæÓ¡¯ ∏¿⁄. ";
#else
                    description += "A cloth or leather cap. ";
#endif
                    break;
                case THELM_WIZARD_HAT:
#ifdef JP
                    description += "ø¯ª‘ ∏æÁ¿« √µ¿∏∑Œ ∏∏µÈæÓ¡¯ ∏¿⁄. ";
#else
                    description += "A conical cloth hat. ";
#endif
                    break;
                }
                break;

            case ARM_GLOVES:
#ifdef JP
                description += "«— Ω÷¿« ¿Â∞©. ";
#else
                description += "A pair of gloves. ";
#endif
                break;
            case ARM_BOOTS:
                if (item.plus2 == TBOOT_NAGA_BARDING)
#ifdef JP
                    description += "≥™∞°¡∑¿ª ¿ß«ÿ ∆Ø∫∞»˜ ∏∏µÈæÓ¡¯ ∞©ø ¿Ã¥Ÿ. ≤ø∏Æ ¿ßø° ¬¯øÎ«“ ºˆ ¿÷¥Ÿ. ";
#else
                    description += "A special armour made for Nagas, "
                        "to wear over their tails. ";
#endif
                else if (item.plus2 == TBOOT_CENTAUR_BARDING)
#ifdef JP
                    description += "ºæ≈∏øÏ∏£∏¶ ¿ß«ÿ ∏∏µÈæÓ¡¯ ∞©ø . "
                                   "«œπ›Ω≈ø° ¬¯øÎ«“ ºˆ ¿÷¥Ÿ.";
#else
                    description += "An armour made for centaurs, "
                        "to wear over their equine half. ";
#endif
                else
#ifdef JP
                    description += "«— ƒ”∑π¿« ∆∞∆∞«— ¿Â»≠. ";
#else
                    description += "A pair of sturdy boots. ";
#endif
                break;
            case ARM_BUCKLER:
#ifdef JP
                description += "¿€¿∫ πÊ∆–. ";
#else
                description += "A small shield. ";
#endif
                break;
            case ARM_LARGE_SHIELD:
#ifdef JP
                description += "≈©¥Ÿ¥¬ ¡°∏∏ ¡¶ø‹«œ∏È, ∫∏≈Î¿« πÊ∆–øÕ ∞∞¥Ÿ. ";
#else
                description += "Like a normal shield, only larger. ";
#endif
                if (you.species == SP_TROLL || you.species == SP_OGRE
                    || you.species == SP_OGRE_MAGE
                    || player_genus(GENPC_DRACONIAN))
                {
#ifdef JP
                    description += "¥ÁΩ≈¿Ã ¬¯øÎ«œ±‚ø° µ¸ ∏¬¿ª ∞Õ ∞∞¥Ÿ. ";
#else
                    description += "It looks like it would fit you well. ";
#endif
                }
                else
                {
#ifdef JP
                    description += "æ≤±‚ø° ∏≈øÏ ∞≈√ﬂ¿ÂΩ∫∑ØøÏ∏Á, ∞¯∞› º”µµ∞° ¥¿∑¡¡˙ ºˆ ¿÷¥Ÿ. ";
#else
                    description += "It is very cumbersome to wear, and "
                        "slows the rate at which you may attack. ";
#endif
                }
                break;
            case ARM_DRAGON_HIDE:
#ifdef JP
                description += "∫Ò¥√∑Œ µ§»˘ øÎ¿« ∞°¡◊. "
                               "¿‘¿∏∑¡∏È ¿‘¿ª ºˆµµ ¿÷¥Ÿ. ";
#else
                description += "The scaly skin of a dragon. I suppose "
                    "you could wear it if you really wanted to. ";
#endif
                break;
            case ARM_TROLL_HIDE:
#ifdef JP
                description += "ªªªª«œ∞Ì »§¿Ã ¡¯ ∆Æ∑—¿« ∞°¡◊. "
                               "¿‘¿∏∑¡∏È ¿‘¿ª ºˆµµ ¿÷¥Ÿ. ";
#else
                description += "The stiff and knobbly hide of a troll. "
                    "I suppose you could wear it "
                    "if you really wanted to. ";
#endif
                break;
            case ARM_CRYSTAL_PLATE_MAIL:
#ifdef JP
                description += "æˆ√ª≥™∞‘ π´∞Ã¡ˆ∏∏ πÊæÓ∑¬∏∏≈≠¿∫ ∂ŸæÓ≥≠ ºˆ¡§ ∞©ø . "
                               "∫ŒΩƒ¿Ã≥™ ƒßΩƒø° æ‡∞£ ¿˙«◊∑¬¿Ã ¿÷¥Ÿ. ";
#else
                description += "An incredibly heavy but extremely effective "
                    "suit of crystalline armour. "
                    "It is somewhat resistant to corrosion. ";
#endif
                break;
            case ARM_DRAGON_ARMOUR:
#ifdef JP
                description += "∫“¿ª ≈‰«œ¥¬ øÎ¿« ∫Ò¥√∑Œ ∏∏µÈæÓ¡¯ ∏∂π˝¿« ∞©ø . "
                               "ªÁøÎ¿⁄ø°∞‘ ∫“ø° ¥Î«— ∞≠«— ¿˙«◊∑¬¿ª ¡¶∞¯«œ¡ˆ∏∏, "
                               "π›¥Î∑Œ ≥√±‚¿« ∞¯∞›ø°¥¬ √Îæ‡«ÿ¡¯¥Ÿ. ";
#else
                description += "A magical armour, made from the scales of "
                    "a fire-breathing dragon. It provides "
                    "great protection from the effects of fire, "
                    "but renders its wearer more susceptible to "
                    "the effects of cold. ";
#endif
                break;
            case ARM_TROLL_LEATHER_ARMOUR:
#ifdef JP
                description += "∫∏≈Î ∆Æ∑—¿« ªªªª«œ∞Ì »§¡¯ ∞°¡◊¿∏∑Œ ∏∏µÈæÓ¡¯ ∏∂π˝¿« ∞©ø . "
                               "(¿ÃπÃ ∆Æ∑—¿Ã æ∆¥— ¿ÃªÛ) ªÁøÎ¿⁄¿« ªÛ√≥∏¶ º≠º≠»˜ »∏∫πΩ√≈≤¥Ÿ. ";
#else
                description += "A magical armour, made from the stiff and "
                    "knobbly skin of a common troll. It magically regenerates "
                    "its wearer's flesh at a fairly slow rate "
                    "(unless already a troll). ";
#endif
                break;
            case ARM_ICE_DRAGON_HIDE:
#ifdef JP
                description += "∫Ò¥√∑Œ µ§»˘ øÎ¿« ∞°¡◊. "
                               "¿‘¿∏∑¡∏È ¿‘¿ª ºˆµµ ¿÷¥Ÿ. ";
#else
                description += "The scaly skin of a dragon. I suppose "
                    "you could wear it if you really wanted to. ";
#endif
                break;
            case ARM_ICE_DRAGON_ARMOUR:
#ifdef JP
                description += "≥√±‚∏¶ ≈‰«œ¥¬ øÎ¿« ∫Ò¥√∑Œ ∏∏µÈæÓ¡¯ ∏∂π˝¿« ∞©ø . "
                               "ªÁøÎ¿⁄ø°∞‘ ≥√±‚ø° ¥Î«— ∞≠«— ¿˙«◊∑¬¿ª ¡¶∞¯«œ¡ˆ∏∏, "
                               "π›¥Î∑Œ ∫“¿« ∞¯∞›ø°¥¬ √Îæ‡«ÿ¡¯¥Ÿ. ";
#else
                description += "A magical armour, made from the scales of "
                    "a cold-breathing dragon. It provides "
                    "great protection from the effects of cold, "
                    "but renders its wearer more susceptible to "
                    "the effects of fire and heat. ";
#endif
                break;
            case ARM_STEAM_DRAGON_HIDE:
#ifdef JP
                description += "∫ŒµÂ∑¥∞Ì ¿Øø¨«— ¡ı±‚ øÎ¿« ∞°¡◊. "
                               "¿‘¿∏∑¡∏È ¿‘¿ª ºˆµµ ¿÷¥Ÿ. ";
#else
                description += "The soft and supple scaley skin of "
                    "a steam dragon. I suppose you could "
                    "wear it if you really wanted to. ";
#endif
                break;
            case ARM_STEAM_DRAGON_ARMOUR:
#ifdef JP
                description += "¡ı±‚∏¶ ≈‰«œ¥¬ øÎ¿« ∫Ò¥√∑Œ ∏∏µÈæÓ¡¯ ∏∂π˝¿« ∞©ø . "
                               "¥Ÿ∏• ƒø¥Ÿ∂ı øÎ¿« ∞©ø √≥∑≥ ∏∂π˝¿˚¿Œ ∫∏»£∏¶ ¡¶∞¯«œ¡ˆ¥¬ æ ¡ˆ∏∏, "
                               "√µ∞˙ ∞∞¿Ã ∏≈øÏ ∞Ê∑Æ¿Ã∏Á ¿Øø¨«œ∞Ì æ„¥Ÿ. ";
#else
                description += "A magical armour, made from the scales of "
                    "a steam-breathing dragon. Although unlike "
                    "the armour made from the scales of some "
                    "larger dragons it does not provide its wearer "
                    "with much in the way of special magical "
                    "protection, it is extremely light and "
                    "as supple as cloth. ";
#endif
                break;          /* Protects from steam */
            case ARM_MOTTLED_DRAGON_HIDE:
#ifdef JP
                description += "æÛ∑Ëπ´¥Ã øÎ¿« ±‚π¶«— ∫Ò¥√¿Ã µ§»˘ ∞°¡◊. "
                               "¿‘¿∏∑¡∏È ¿‘¿ª ºˆµµ ¿÷¥Ÿ. ";
#else
                description += "The weirdly-patterned scaley skin of "
                    "a mottled dragon. I suppose you could "
                    "wear it if you really wanted to. ";
#endif
                break;
            case ARM_MOTTLED_DRAGON_ARMOUR:
#ifdef JP
                description += "æÛ∑Ëπ´¥Ã øÎ¿« ∫Ò¥√∑Œ ∏∏µÈæÓ¡¯ ∏∂π˝¿« ∞©ø . "
                               "¥Ÿ∏• ƒø¥Ÿ∂ı øÎ¿« ∞©ø √≥∑≥ ∏∂π˝¿˚¿Œ ∫∏»£∏¶ ¡¶∞¯«œ¡ˆ¥¬ æ ¡ˆ∏∏, "
                               "∞°¡◊ ∞©ø ∞˙ ∞∞¿Ã ∞Ê∑Æ¿Ã∏Á øÚ¡˜¿Ã±‚ ∆Ì«œ¥Ÿ. ";
#else
                description += "A magical armour made from the scales of a "
                    "mottled dragon. Although unlike the armour "
                    "made from the scales of some larger dragons "
                    "it does not provide its wearer with much in "
                    "the way of special magical protection, it is "
                    "as light and relatively uncumbersome as "
                    "leather armour. ";
#endif
                break;          /* Protects from napalm */
            case ARM_STORM_DRAGON_HIDE:
#ifdef JP
                description += "∆¯«≥ øÎ¿« ∏≈øÏ ¥‹¥‹«— «™∏• ∫Ò¥√¿Ã µ§»˘ ∞°¡◊. "
                               "¿‘¿∏∑¡∏È ¿‘¿ª ºˆµµ ¿÷¥Ÿ. ";
#else
                description += "The hide of a storm dragon, covered in "
                    "extremely hard blue scales. I suppose "
                    "you could wear it if you really wanted to. ";
#endif
                break;
            case ARM_STORM_DRAGON_ARMOUR:
#ifdef JP
                description += "π¯∞≥∏¶ ≈‰«œ¥¬ ∆¯«≥ øÎ¿« ∫Ò¥√∑Œ ∏∏µÈæÓ¡¯ ∏∂π˝¿« ∞©ø . "
                               "¥Î∫Œ∫–¿« øÎ ∫Ò¥√ ∞©ø ∫∏¥Ÿµµ π´∞Ã¡ˆ∏∏, ªÁøÎ¿⁄ø°∞‘ "
                               "¿¸±‚ πÊ¿¸ø° ¥Î«— ∞≠∑¬«— ¿˙«◊∑¬¿ª ∫Œø©«—¥Ÿ. ";
#else
                description += "A magical armour made from the scales of "
                    "a lightning-breathing dragon. It is heavier "
                    "than most dragon scale armours, but gives "
                    "its wearer great resistance to "
                    "electrical discharges. ";
#endif
                break;
            case ARM_GOLD_DRAGON_HIDE:
#ifdef JP
                description += "¬˘∂ı«œ∞‘ ∫˚≥™¥¬ ±›∫˚ ∫Ò¥√¿Ã µ§»˘ ∞°¡◊. "
                               "∏≈øÏ ¡˙±‚∏Á ±≤¿Â»˜ π´∞Ã¥Ÿ. "
                               "¿‘¿∏∑¡∏È ¿‘¿ª ºˆµµ ¿÷¥Ÿ. ";
#else
                description += "The extremely tough and heavy skin of a "
                    "golden dragon, covered in shimmering golden "
                    "scales. I suppose you could wear it if "
                    "you really wanted to. ";
#endif
                break;
            case ARM_GOLD_DRAGON_ARMOUR:
#ifdef JP
                description += "»≤±› øÎ¿« ±›∫˚ ∫Ò¥√∑Œ ∏∏µÈæÓ¡¯ ∏∂π˝¿« ∞©ø . "
                               "∏≈øÏ π´∞Ã∞Ì øÚ¡˜¿Ã±‚ ∫“∆Ì«œ¡ˆ∏∏, ªÁøÎ¿⁄ø°∞‘ "
                               "∫“, ≥√±‚, µ∂ø° ¥Î«— ¿˙«◊∑¬¿ª ∫Œø©«—¥Ÿ. ";
#else
                description += "A magical armour made from the golden scales "
                    "of a golden dragon. It is extremely heavy and "
                    "cumbersome, but confers resistances to fire, "
                    "cold, and poison on its wearer. ";
#endif
                break;
            case ARM_ANIMAL_SKIN:
#ifdef JP
                description += "ø©∑Ø µøπ∞¿« ∞°¡◊. ";
#else
                description += "The skins of several animals. ";
#endif
                break;
            case ARM_SWAMP_DRAGON_HIDE:
#ifdef JP
                description += "¥À¡ˆ øÎ¿« πÃ≤ˆπÃ≤ˆ∞≈∏Æ";
                if (you.species != SP_MUMMY)
                    description += "∞Ì ¿ÃªÛ«— ≥øªı∞° ≥™";
                description += "¥¬ ∞°¡◊. "
                               "¿‘¿∏∑¡∏È ¿‘¿ª ºˆµµ ¿÷¥Ÿ. ";
#else
                description += "The slimy";
                if (you.species != SP_MUMMY)
                    description += ", smelly";
                description += " skin of a swamp-dwelling dragon. I suppose "
                    "you could wear it if you really wanted to. ";
#endif
                break;
            case ARM_SWAMP_DRAGON_ARMOUR:
#ifdef JP
                description += "¥À¡ˆ øÎ¿« ∫Ò¥√∑Œ ∏∏µÈæÓ¡¯ ∏∂π˝¿« ∞©ø . "
                               "ªÁøÎ¿⁄ø°∞‘ µ∂ø° ¥Î«— ≥ªº∫¿ª ∫Œø©«—¥Ÿ. ";
#else
                description += "A magical armour made from the scales of "
                    "a swamp dragon. It confers resistance to "
                    "poison on its wearer. ";
#endif
                break;
            default:
#ifdef JP
                DEBUGSTR("æÀ∑¡¡ˆ¡ˆ æ ¿∫ ∞©ø . ");
#else
                DEBUGSTR("Unknown armour");
#endif
            }

            description += "$";
        }
    }

    if (verbose == 1
            && item.sub_type != ARM_SHIELD
            && item.sub_type != ARM_BUCKLER
            && item.sub_type != ARM_LARGE_SHIELD)
    {
#ifdef JP
        description += "$πÊæÓ µÓ±ﬁ : ";
#else
        description += "$Armour rating: ";
#endif

        if (item.sub_type == ARM_HELMET
            && (get_helmet_type( item ) == THELM_CAP
                || get_helmet_type( item ) == THELM_WIZARD_HAT))
        {
            // caps and wizard hats don't have a base AC
            append_value(description, 0, false);
        }
        else if (item.sub_type == ARM_BOOTS && item.plus2 != TBOOT_BOOTS)
        {
            // Barding has AC value 4.
            append_value(description, 4, false);
        }
        else
        {
            append_value(description, property( item, PARM_AC ), false);
        }

#ifdef JP
        description += "$»∏««∑¬ ºˆ¡§ƒ°: ";
#else
        description += "$Evasion modifier: ";
#endif
        append_value(description, property( item, PARM_EVASION ), true);
        description += "$";
    }

    int ego = get_armour_ego_type( item );
    if (ego != SPARM_NORMAL
        && item_ident( item, ISFLAG_KNOW_TYPE )
        && verbose == 1)
    {
        description += "$";

        switch (ego)
        {
        case SPARM_RUNNING:
#ifdef JP
            description += "¬¯øÎ¿⁄∏¶ ∫¸∏• º”µµ∑Œ ¥ﬁ∏± ºˆ ¿÷∞‘ «ÿ ¡ÿ¥Ÿ. ";
#else
            description += "It allows its wearer to run at a great speed. ";
#endif
            break;
        case SPARM_FIRE_RESISTANCE:
#ifdef JP
            description += "¬¯øÎ¿⁄∏¶ ∫“∞˙ ø≠±‚∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. ";
#else
            description += "It protects its wearer from heat and fire. ";
#endif
            break;
        case SPARM_COLD_RESISTANCE:
#ifdef JP
            description += "¬¯øÎ¿⁄∏¶ ≥√±‚∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. ";
#else
            description += "It protects its wearer from cold. ";
#endif
            break;
        case SPARM_POISON_RESISTANCE:
#ifdef JP
            description += "¬¯øÎ¿⁄∏¶ µ∂¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. ";
#else
            description += "It protects its wearer from poison. ";
#endif
            break;
        case SPARM_SEE_INVISIBLE:
#ifdef JP
            description += "¬¯øÎ¿⁄∞° ≈ı∏Ì«— π∞√º∏¶ ∫º ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ. ";
#else
            description += "It allows its wearer to see invisible things. ";
#endif
            break;
        case SPARM_DARKNESS:
#ifdef JP
            description += "πﬂµøΩ√≈∞∏È ¬¯øÎ¿⁄¥¬ ≈ı∏Ì»≠«œø© ∫∏¿Ã¡ˆ æ ∞‘ µ«¡ˆ∏∏, "
                           "µøΩ√ø° Ω≈¡¯¥ÎªÁ¿« º”µµµµ ≈´ ∆¯¿∏∑Œ ¡ı∞°«—¥Ÿ. ";
#else
            description += "When activated it hides its wearer from "
                "the sight of others, but also increases "
                "their metabolic rate by a large amount. ";
#endif
            break;
        case SPARM_STRENGTH:
#ifdef JP
            description += "¬¯øÎ¿⁄¿« π∞∏Æ¿˚ »˚¿ª ¡ı∞°Ω√≈≤¥Ÿ. (+3 »˚) ";
#else
            description += "It increases the physical power of its wearer (+3 to strength). ";
#endif
            break;
        case SPARM_DEXTERITY:
#ifdef JP
            description += "¬¯øÎ¿⁄¿« πŒ√∏º∫¿ª ¡ı∞°Ω√≈≤¥Ÿ. (+3 πŒ√∏º∫) ";
#else
            description += "It increases the dexterity of its wearer (+3 to dexterity). ";
#endif
            break;
        case SPARM_INTELLIGENCE:
#ifdef JP
            description += "¬¯øÎ¿⁄∏¶ ¥ıøÌ ∂»∂»«œ∞‘ ∏∏µÈæÓ ¡ÿ¥Ÿ. (+3 ¡ˆ¥…) ";
#else
            description += "It makes you more clever (+3 to intelligence). ";
#endif
            break;
        case SPARM_PONDEROUSNESS:
#ifdef JP
            description += "øÚ¡˜¿”ø° ∏≈øÏ πÊ«ÿµ»¥Ÿ. (-2 »∏««, ¿Ãµøº”µµ ∞®º“) ";
#else
            description += "It is very cumbersome (-2 to EV, slows movement). ";
#endif
            break;
        case SPARM_LEVITATION:
#ifdef JP
            description += "πﬂµøΩ√ƒ— ¬¯øÎ¿⁄∏¶ ∞¯¡ﬂ¿∏∑Œ ∂∞ø¿∏£∞‘ «“ ºˆ ¿÷¥Ÿ.";
                           "∞¯¡ﬂø° ∂∞ ¿÷¥¬ ªÛ≈¬¥¬ æÓ¥¿¡§µµ ¡ˆº”µ»¥Ÿ.";
#else
            description += "It can be activated to allow its wearer to "
                "float above the ground and remain so indefinitely. ";
#endif
            break;
        case SPARM_MAGIC_RESISTANCE:
#ifdef JP
            description += "¬¯øÎ¿⁄ø°∞‘ ∏∂π˝ø° ¥Î«— ¿˙«◊∑¬¿ª ∞≠»≠Ω√≈≤¥Ÿ.";
#else
            description += "It increases its wearer's resistance "
                "to enchantments. ";
#endif
            break;
        case SPARM_PROTECTION:
#ifdef JP
            description += "¬¯øÎ¿⁄∏¶ π∞∏Æ¿˚¿Œ «««ÿ∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. (+3 πÊæÓ) ";
#else
            description += "It protects its wearer from harm (+3 to AC). ";
#endif
            break;
        case SPARM_STEALTH:
#ifdef JP
            description += "¬¯øÎ¿⁄∞° ¥ıøÌ ¥ı ¿∫π–«œ∞‘ øÚ¡˜¿œ ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ. ";
#else
            description += "It enhances the stealth of its wearer. ";
#endif
            break;
        case SPARM_RESISTANCE:
#ifdef JP
            description += "¬¯øÎ¿⁄∏¶ ≥√±‚øÕ ∫“∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. ";
#else
            description += "It protects its wearer from the effects "
                "of both cold and heat. ";
#endif
            break;

        // these two are robes only:
        case SPARM_POSITIVE_ENERGY:
#ifdef JP
            description += "¬¯øÎ¿⁄∏¶ ¿Ωø°≥ ¡ˆ∑Œ∫Œ≈Õ æ‡∞£ ∫∏»£«—¥Ÿ. ";
#else
            description += "It partially protects its wearer from "
                "the effects of negative energy. ";
#endif
            break;
        case SPARM_ARCHMAGI:
#ifdef JP
            description += "¬¯øÎ¿⁄∞° Ω√¿¸«œ¥¬ ∏∂π˝ ¡÷πÆ¿« ¿ß∑¬¿ª ≈©∞‘ ∞≠»≠Ω√≈≤¥Ÿ. "
                           "¥‹, ¥ı πËøÔ ∞Õ¿Ã ∞≈¿« ≥≤æ∆¿÷¡ˆ æ ¿∫ ¿⁄µÈ¿ª ¿ß«ÿ ∏∏µÈæÓ¡≥¥Ÿ. ";
#else
            description += "It greatly increases the power of its "
                "wearer's magical spells, but is only "
                "intended for those who have " "very little left to learn. ";
#endif
            break;

        case SPARM_PRESERVATION:
#ifdef JP
            description += "¬¯øÎ¿⁄¿« º“¡ˆ«∞¿ª º’ªÛ∞˙ ∆ƒ±´∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. ";
#else
            description += "It protects its wearer's possessions "
                "from damage and destruction. ";
#endif
            break;
        }
        description += "$";
    }

    if (is_random_artefact( item ))
    {
        if (item_ident( item, ISFLAG_KNOW_PROPERTIES ))
            randart_descpr( description, item );
        else if (item_ident( item, ISFLAG_KNOW_TYPE ))
#ifdef JP
            description += "$¿Ã ∞©ø ¿∫ øÿ¡ˆ º˚∞‹¡¯ ¥…∑¬¿Ã ¿÷¿ª ∞Õ ∞∞¥Ÿ. ";
#else
            description += "$This armour may have some hidden properties.$";
#endif
    }
    else
    {
        switch (get_equip_race( item ))
        {
        case ISFLAG_ELVEN:
            //jmf: not light
#ifdef JP
            description += "$ªÛ¥Á»˜ »«∏¢«— ºÿææ∑Œ ∏∏µÈæÓ¡Æ, ¬¯øÎ¿⁄∞° øÚ¡˜¿Ã±‚ ∆Ì«œµµ∑œ µ«æÓ ¿÷";
#else
            description += "$It is well-crafted and unobstructive";
#endif

            if (item.sub_type == ARM_CLOAK || item.sub_type == ARM_BOOTS)
#ifdef JP
                description += "¿∏∏Á, ¿∫π–«œ∞‘ «‡µø«œ¥¬ µ•ø° µµøÚ¿Ã µ»";
#else
                description += ", and helps its wearer avoid being noticed";
#endif

#ifdef JP
            description += "¥Ÿ.";
#else
            description += ".";
#endif
            break;

        case ISFLAG_DWARVEN:
#ifdef JP
            description += "$ªÛ¥Á»˜ ∂ŸæÓ≥≠ ºÿææ∑Œ ∏∏µÈæÓ¡Æ, ≥ª±∏º∫¿Ã ∏≈øÏ ¡¡¥Ÿ.";
#else
            description += "$It is well-crafted and very durable.";
#endif
            break;

        case ISFLAG_ORCISH:
        default:
            break;
        }
    }

    if (item_known_cursed( item ))
    {
#ifdef JP
        description += "$¿Ã ∞©ø ¿∫ ¿˙¡÷∞° ∞…∑¡ ¿÷¥Ÿ.";
#else
        description += "$It has a curse placed upon it.";
#endif
    }

#ifdef JP
    if ( (verbose == 1)
       &&(item.sub_type != ARM_SHIELD)&&(item.sub_type != ARM_CLOAK)&&(item.sub_type != ARM_HELMET)
       &&(item.sub_type != ARM_GLOVES)&&(item.sub_type != ARM_BOOTS)&&(item.sub_type != ARM_BUCKLER)
       &&(item.sub_type != ARM_LARGE_SHIELD) )
    {
        if ( is_light_armour(item) )
            description += "$¿Ã∞Õ¿∫ ∞Ê∑Æ∞©ø ¿∏∑Œ ∫–∑˘µ»¥Ÿ. ";
        else
            description += "$¿Ã∞Õ¿∫ ∞©ø ¿∏∑Œ ∫–∑˘µ»¥Ÿ. ";
    }
#else
#endif

    return description;
}

//---------------------------------------------------------------
//
// describe_stick
//
//---------------------------------------------------------------
static std::string describe_stick( const item_def &item )
{
    std::string description;

    description.reserve(64);

    if (get_ident_type( OBJ_WANDS, item.sub_type ) != ID_KNOWN_TYPE)
#ifdef JP
        description += "∆Úπ¸«— ∏∑¥Î±‚. ∏∂π˝¿Ã º˚æÓ¿÷¿ª¡ˆµµ ∏∏•¥Ÿ. $";
#else
        description += "A stick. Maybe it's magical. ";
#endif
    else
    {
#ifdef JP
        description += "";
#else
        description += "A magical device which ";
#endif
        switch (item.sub_type)
        {
        case WAND_FLAME:
#ifdef JP
            description += "¿€¿∫ ∫“±Ê¿ª ≥ªª’¥¬ ∏∂π˝∫¿. $";
#else
            description += "throws little bits of flame. ";
#endif
            break;

        case WAND_FROST:
#ifdef JP
            description += "æ‡∞£¿« ≥√±‚∏¶ ≥ªª’¥¬ ∏∂π˝∫¿. $";
#else
            description += "throws little bits of frost. ";
#endif
            break;

        case WAND_SLOWING:
#ifdef JP
            description += "∏Ò«•π∞¿« øÚ¡˜¿”¿ª µ–»≠Ω√≈∞¥¬ ∏∂π˝¿Ã ¥„±‰ ∏∂π˝∫¿. $";
#else
            description += "casts enchantments to slow down the actions of "
                "a creature at which it is directed. ";
#endif
            break;

        case WAND_HASTING:
#ifdef JP
            description += "∏Ò«•π∞¿« øÚ¡˜¿”¿ª ∞°º”Ω√≈∞¥¬ ∏∂π˝¿Ã ¥„±‰ ∏∂π˝∫¿. $";
#else
            description += "casts enchantments to speed up the actions of "
                "a creature at which it is directed. ";
#endif
            break;

        case WAND_MAGIC_DARTS:
#ifdef JP
            description += "¿€¿∫ ∆ƒ±´º∫ ø°≥ ¡ˆ¿« »≠ªÏ¿ª πﬂªÁ«œ¥¬ ∏∂π˝∫¿. $";
#else
            description += "throws small bolts of destructive energy. ";
#endif
            break;

        case WAND_HEALING:
#ifdef JP
            description += "∏Ò«•π∞¿« ªÛ√≥∏¶ ƒ°∑·«œ¥¬ ∏∂π˝∫¿. $";
#else
            description += "can heal a creature's wounds. ";
#endif
            break;

        case WAND_PARALYSIS:
#ifdef JP
            description += "∏Ò«•π∞¿ª ∏∂∫ÒΩ√≈∞¥¬ ∏∂π˝∫¿. $";
#else
            description += "can render a creature immobile. ";
#endif
            break;

        case WAND_FIRE:
#ifdef JP
            description += "∞≈¥Î«— ∫“±Ê¿« »≠ªÏ¿ª πﬂªÁ«œ¥¬ ∏∂π˝∫¿. $";
#else
            description += "throws great bolts of fire. ";
#endif
            break;

        case WAND_COLD:
#ifdef JP
            description += "∞≈¥Î«— ≥√±‚¿« »≠ªÏ¿ª πﬂªÁ«œ¥¬ ∏∂π˝∫¿. $";
#else
            description += "throws great bolts of cold. ";
#endif
            break;

        case WAND_CONFUSION:
#ifdef JP
            description += "∏Ò«•π∞¿ª »•∂ıø° ∫¸¡ˆ∞‘ «œ¥¬ ∏∂π˝∫¿.$";
#else
            description += "induces confusion and bewilderment in "
                "a target creature. ";
#endif
            break;

        case WAND_INVISIBILITY:
#ifdef JP
            description += "∏Ò«•π∞¿ª ≈ı∏Ì»≠Ω√≈∞¥¬ ∏∂π˝∫¿. $";
#else
            description += "hides a creature from the view of others. ";
#endif
            break;

        case WAND_DIGGING:
#ifdef JP
            description += "∞°∞¯µ«¡ˆ æ ¿∫ µπ¿Ã≥™ πŸ¿ß∏¶ ∂’æÓ ≈Õ≥Œ¿ª ∂’¥¬ ∏∂π˝∫¿. $";
#else
            description += "drills tunnels through unworked rock. ";
#endif
            break;

        case WAND_FIREBALL:
#ifdef JP
            description += "∆¯πﬂ«œ¥¬ ∫“±Ê¿« ∆¯«≥¿ª πﬂª˝Ω√≈∞¥¬ ∏∂π˝∫¿. $";
#else
            description += "throws exploding blasts of flame. ";
#endif
            break;

        case WAND_TELEPORTATION:
#ifdef JP
            description += "∏Ò«•π∞¿ª π´¿€¿ß ∞¯∞£¿Ãµø Ω√≈∞¥¬ ∏∂π˝∫¿. $";
#else
            description += "causes a creature to be randomly translocated. ";
#endif
            break;

        case WAND_LIGHTNING:
#ifdef JP
            description += "∞≈¥Î«— π¯∞≥¿« »≠ªÏ¿ª πﬂªÁ«œ¥¬ ∏∂π˝∫¿. $";
#else
            description += "throws great bolts of lightning. ";
#endif
            break;

        case WAND_POLYMORPH_OTHER:
#ifdef JP
            description += "∏Ò«•π∞¿ª ¥Ÿ∏• «¸≈¬∑Œ ∫Ø»≠Ω√≈∞¥¬ ∏∂π˝∫¿. $"
                           "¿⁄Ω≈ø°∞‘¥¬ º“øÎ¿Ã æ¯¿∏π«∑Œ, ∏∂π˝∫¿¿« ∏∂≥™∏¶ ≥∂∫Ò«œ¡ˆ ∏ª¿⁄. $";
#else
            description += "causes a creature to be transmogrified into "
                "another form. "
                "It doesn't work on you, so don't even try. ";
#endif
            break;

        case WAND_ENSLAVEMENT:
#ifdef JP
            description += "∏Ò«•π∞¿ª ≥Îøπ»≠«œø© ∫π¡æΩ√≈∞¥¬ ∏∂π˝∫¿. $";
#else
            description += "causes slavish obedience in a creature. ";
#endif
            break;

        case WAND_DRAINING:
#ifdef JP
            description += "¿Ωø°≥ ¡ˆ¿« »≠ªÏ¿ª πﬂªÁ«œø© ∏Ò«• ª˝π∞¿«$"
                           "ª˝∏Ì∑¬¿ª »Ìºˆ«œ¥¬ ∏∂π˝∫¿. $"
                           "æµ•µÂø°∞‘¥¬ »ø∞˙∞° æ¯¥Ÿ. $";
#else
            description += "throws a bolt of negative energy which "
                "drains the life essences of living creatures, "
                "but is useless against the undead. ";
#endif
            break;

        case WAND_RANDOM_EFFECTS:
#ifdef JP
            description += "π´¿€¿ß¿« »ø∞˙∏¶ ¿œ¿∏≈∞¥¬ ∏∂π˝∫¿. $";
#else
            description += "can produce a variety of effects. ";
#endif
            break;

        case WAND_DISINTEGRATION:
#ifdef JP
            description += "∏Ò«•π∞¿« π∞∏Æ¿˚ ±∏¡∂(∆Ø»˜ ¥ÎªÛ¿« ∏ˆ)∏¶ ∫ÿ±´Ω√≈∞¥¬ ∏∂π˝∫¿. $";
#else
            description += "disrupts the physical structure of "
                "an object, especially a creature's body. ";
#endif
            break;

        default:
#ifdef JP
            DEBUGSTR("æÀ∑¡¡ˆ¡ˆ æ ¿∫ ∏∑¥Î±‚.");
#else
            DEBUGSTR("Unknown stick");
#endif
        }

        if (item_ident( item, ISFLAG_KNOW_PLUSES ) && item.plus == 0)
#ifdef JP
            description += "æ»≈∏±ı∞‘µµ ¿Ã ∏∂π˝∫¿ø°¥¬ ∏∂≥™∞° ≥≤æ∆¿÷¡ˆ æ ¥Ÿ.";
#else
            description += "Unfortunately, it has no charges left. ";
#endif
    }

    return description;
}


//---------------------------------------------------------------
//
// describe_food
//
//---------------------------------------------------------------
static std::string describe_food( const item_def &item )
{
    std::string description;

    description.reserve(100);

    switch (item.sub_type)
    {
    // rations
    case FOOD_MEAT_RATION:
    case FOOD_BREAD_RATION:
#ifdef JP
        description += "«—≥¢ ΩƒªÁ∫–¿« ";
#else
        description += "A filling ration of ";
#endif
        switch (item.sub_type)
        {
        case FOOD_MEAT_RATION:
#ifdef JP
            description += "∏ª∏Æ∞≈≥™ »∆¡¶Ω√ƒ— ∫∏¡∏µ» ∞Ì±‚";
#else
            description += "dried and preserved meats";
#endif
            break;
        case FOOD_BREAD_RATION:
#ifdef JP
            description += "ªß";
#else
            description += "breads";
#endif
            break;
        }
#ifdef JP
        description += ". ";
#else
        description += ". ";
#endif
        break;

    // fruits
    case FOOD_PEAR:
    case FOOD_APPLE:
    case FOOD_APRICOT:
    case FOOD_ORANGE:
    case FOOD_BANANA:
    case FOOD_STRAWBERRY:
    case FOOD_RAMBUTAN:
    case FOOD_LEMON:
    case FOOD_GRAPE:
    case FOOD_LYCHEE:
    case FOOD_SULTANA:
#ifdef JP
        description += "¿Ã∞Õ¿∫";
#else
        description += "A";
#endif
        switch (item.sub_type)
        {
        case FOOD_PEAR:
#ifdef JP
            description += " ∏¿¿÷∞Ì ¡Û¿Ã ∏π¿∫";
#else
            description += " delicious juicy";
#endif
            break;
        case FOOD_APPLE:
#ifdef JP
            description += " ∏¿¿÷¥¬ ª°∞≠ªˆ¿Ã≥™ √ ∑œªˆ¿«";
#else
            description += " delicious red or green";
#endif
            break;
        case FOOD_APRICOT:
#ifdef JP
            description += " ∏¿¿÷¥¬ ¡÷»≤ªˆ¿«";
#else
            description += " delicious orange";
#endif
            break;
        case FOOD_ORANGE:
#ifdef JP
            description += " ¡Û¿Ã ∏π∞Ì ∏¿¿÷¥¬ ¡÷»≤ªˆ¿«";
#else
            description += " delicious juicy orange";
#endif
            break;
        case FOOD_BANANA:
#ifdef JP
            description += " ≥Î∂ıªˆ¿« ∏¿¿÷¥¬";
#else
            description += " delicious yellow";
#endif
            break;
        case FOOD_STRAWBERRY:
#ifdef JP
            description += " ¿€¡ˆ∏∏ ∏¿¿÷¥¬ ∫”¿∫ªˆ¿«";
#else
            description += " small but delicious red";
#endif
            break;
        case FOOD_RAMBUTAN:
#ifdef JP
            description += " ø≠¥Î¡ˆπÊ¿« ¿€¡ˆ∏∏ ∏¿¿÷¥¬";
#else
            description += " small but delicious tropical";
#endif
            break;
        case FOOD_LEMON:
#ifdef JP
            description += " ≥Î∂ıªˆ¿«";
#else
            description += " yellow";
#endif
            break;
        case FOOD_GRAPE:
#ifdef JP
            description += " ¿€¿∫";
#else
            description += " small";
#endif
            break;
        case FOOD_LYCHEE:
#ifdef JP
            description += " ø≠¥Î¡ˆπÊ¿«";
#else
            description += " tropical";
#endif
            break;
        case FOOD_SULTANA:
#ifdef JP
            description += " ∏ª∏∞";
#else
            description += " dried";
#endif
            break;
        }

#ifdef JP
        description += " ∞˙¿œ. $";
#else
        description += " fruit";
#endif

        switch (item.sub_type)
        {
        case FOOD_BANANA:
#ifdef JP
            description += //∞®¿⁄∞°øµπÆ¿∏∑Œ
                ", æ∆∏∂ ∫“∆ÚµÓ«— π´ø™«˘¡§¿ª πŸ≈¡¿∏∑Œ "
                "µµ¥ˆ¿˚ ∞¸≥‰¿Ã ∫∞∑Œ æ¯¥¬ ¥Ÿ±π¿˚±‚æ˜ø° ¿««ÿ "
                "¿ÁπË, ºˆ¿‘µ«æ˙¿ª ∞Õ¿Ã¥Ÿ";
#else
            description += ", probably grown and imported by "
                "some amoral multinational as the "
                "result of a corrupt trade deal";
#endif
            break;
        case FOOD_RAMBUTAN:
#ifdef JP
            description += //∞®¿⁄∞°øµπÆ¿∏∑Œ
                ". ¿Ã∞Õ¿Ã æÓ∂ª∞‘ ¿Ã ¥¯¡Ø±Ó¡ˆ µÈæÓøÕ ¿÷¥¬¡ˆ "
                "¥©±∏µµ æÀ ºˆ æ¯¥Ÿ";
#else
            description += ". How it got into this dungeon "
                "is anyone's guess";
#endif
            break;
        case FOOD_SULTANA:
#ifdef JP
            description += " æ∆∏∂µµ ∆˜µµ¿« ¿œ¡æ¿Ã∂Û∞Ì ª˝∞¢µ»¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += " of some sort, possibly a grape";
#endif
            break;
        }
#ifdef JP
        //description += ". ";
#else
        description += ". ";
#endif
        break;

    // vegetables
    case FOOD_CHOKO:
    case FOOD_SNOZZCUMBER:
#ifdef JP
        //description += "";
#else
        description += "A";
#endif
        switch (item.sub_type)
        {
        case FOOD_CHOKO:
#ifdef JP
            description += "∞≈¿« ∏¿¿Ã æ¯¥¬ ≥Ïªˆ¿« æﬂ√§¿Ã¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += "n almost tasteless green";
#endif
            break;
        case FOOD_SNOZZCUMBER:
#ifdef JP
            description += "±∏≈‰∏¶ ≥™∞‘«œ¥¬ ∏¿¿«, ø¿¿Ã∏¶ ¥‡¿∫ «¸≈¬¿« æﬂ√§¿Ã¥Ÿ. "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
            description += "ø¿-æﬂªÁΩ√∂Û¥¬ ∞≈¿Œ¿Ã ¡÷Ωƒ¿∏∑Œ «œ∞Ì ¿÷¥Ÿ";
#else
            description += " repulsive cucumber-shaped";
#endif
            break;
        }
#ifdef JP
        description += "";
#else
        description += " vegetable";
#endif
        switch (item.sub_type)
        {
        case FOOD_CHOKO:
#ifdef JP
            description += ", ¿Ã∞Õ¿∫ µ¢±º¿Ã µ»¥Ÿ"; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += ", which grows on a vine";
#endif
            break;
        }
#ifdef JP
        //description += ". ";
#else
        description += ". ";
#endif
        break;

    // lumps, slices, chunks, and strips
    case FOOD_HONEYCOMB:
    case FOOD_ROYAL_JELLY:
    case FOOD_PIZZA:
    case FOOD_CHEESE:
    case FOOD_BEEF_JERKY:
    case FOOD_SAUSAGE:
    case FOOD_CHUNK:
#ifdef JP
        description += "¿Ã";
#else
        description += "A";
#endif
        switch (item.sub_type)
        {
        case FOOD_SAUSAGE:
#ifdef JP
            description += " ±Ê¥Ÿ∂ı";
#else
            description += "n elongated";
#endif
            break;
        }
        switch (item.sub_type)
        {
        //case FOOD_HONEYCOMB:
        //case FOOD_ROYAL_JELLY:
        case FOOD_CHEESE:
        //case FOOD_SAUSAGE:
#ifdef JP
            description += " µ¢æÓ∏Æ¥¬";
#else
            description += " lump";
#endif
            break;
        case FOOD_PIZZA:
#ifdef JP
            description += " ¡∂∞¢¿∫";
#else
            description += " slice";
#endif
            break;
        case FOOD_BEEF_JERKY:
#ifdef JP
            description += " æ„¿∫ ∞Õ¿∫";
#else
            description += " strip";
#endif
            break;
        case FOOD_CHUNK:
#ifdef JP
            description += " ¡∂∞¢¿∫";
#else
            description += " piece";
#endif
        }
#ifdef JP
        description += " ";
#else
        description += " of ";
#endif
        switch (item.sub_type)
        {
        case FOOD_SAUSAGE:
#ifdef JP
            description += //∞®¿⁄∞°øµπÆ¿∏∑Œ
                "ø¨∞Ò, ≥ª¿Â, ∞Óπ∞ µÓ¿ª √¢¿⁄ø° √§øÓ ¿ΩΩƒ¿Ã¥Ÿ. ";
#else
            description += "low-grade gristle, entrails and "
                "cereal products encased in an intestine";
#endif
            break;
        case FOOD_HONEYCOMB:
#ifdef JP
            description += "∞≈¥Î«— ≤‹π˙¿Ã ∏∏µÁ ∏¿¿÷¥¬ π˙¡˝¿Ã¥Ÿ. "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += "the delicious honeycomb made by giant bees";
#endif
            break;
        case FOOD_ROYAL_JELLY:
#ifdef JP
            description += "∞≈¥Î«— ≤‹π˙¿Ã ±◊µÈ¿« ø©ø’π˙ø°∞‘ ∏‘¿Ã±‚ ¿ß«ÿ ∏∏µÁ "
                           "Ω≈∫Ò«— π∞¡˙¿Ã¥Ÿ. "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += "the magical substance produced by giant bees "
                "to be fed to their queens";
#endif
            break;
        case FOOD_PIZZA:
#ifdef JP
            description += "««¿⁄¥Ÿ. "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += "pizza";
#endif
            break;
        case FOOD_CHEESE:
#ifdef JP
            description += "ƒ°¡Ó¥Ÿ. "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += "cheese";
#endif
            break;
        case FOOD_BEEF_JERKY:
#ifdef JP
            description += "∫∏¡∏¿ª ¿ß«ÿ ∞°∞¯¿Ã ≥°≥≠ ºË∞Ì±‚¿Ã¥Ÿ. "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += "preserved dead cow or bull";
#endif
            break;
        case FOOD_CHUNK:
#ifdef JP
            description += "¥¯¡Øø°º≠ ±∏«“ ºˆ ¿÷¥¬ ∞Ì±‚¥Ÿ. "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += "dungeon meat";
#endif
            break;
        }
#ifdef JP
        //description += ". ";
#else
        description += ". ";
#endif
        switch (item.sub_type)
        {
        case FOOD_SAUSAGE:
#ifdef JP
            description += "æ‰! "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += "Yum! ";
#endif
            break;
        case FOOD_PIZZA:
#ifdef JP
            description += "±◊ ¿ΩΩƒ¿Ã ππ≥ƒ∞Ì ≥™ø°∞‘ πØ¡ˆ ∏ªæ∆¡‡! "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
            description += "Don't tell me you don't know what that is! ";
#endif
            break;
        case FOOD_CHUNK:
            if (you.species != SP_GHOUL)
#ifdef JP
                description += "¿Ã ∞Ì±‚¥¬ æ∆∏∂µµ ∏¿æ¯¿ª ∞Õ ∞∞¥Ÿ. "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
                description += "It looks rather unpleasant. ";
#endif

            if (item.special < 100)
            {
                if (you.species == SP_GHOUL)
#ifdef JP
                    description += "¿Ã ∞Ì±‚¥¬ »«∏¢«œ∞‘ º˜º∫µ» ªˆ¿Ã¥Ÿ. "; //∞®¿⁄∞°øµπÆ¿∏∑Œ
#else
                    description += "It looks nice and ripe. ";
#endif
                else if (you.species != SP_MUMMY)
                {
#ifdef JP
                    description += //∞®¿⁄∞°øµπÆ¿∏∑Œ
                        "ªÁΩ«, ¿Ã ∞Ì±‚¥¬ ∫Œ∆–∞° ¡¯«‡µ«∞Ì ¿÷¥Ÿ. "
                        "¿Ã∞Õ¿ª ∏‘¥¬ ∞Õ¿∫ «ˆ∏Ì«œ¡ˆ æ ¿∫ «‡µø¿Ã¥Ÿ. ";
#else
                    description += "In fact, it is "
                        "rotting away before your eyes. "
                        "Eating it would probably be unwise. ";
#endif
                }
            }
            break;
        }
        break;

    default:
#ifdef JP
        DEBUGSTR("æÀ∑¡¡ˆ¡ˆ æ ¿∫ Ωƒ«∞");
#else
        DEBUGSTR("Unknown food");
#endif
    }

    description += "$";

    return (description);
}

//---------------------------------------------------------------
//
// describe_potion
//
//---------------------------------------------------------------
static std::string describe_potion( const item_def &item )
{
    std::string description;

    description.reserve(64);

    if (get_ident_type( OBJ_POTIONS, item.sub_type ) != ID_KNOWN_TYPE)
#ifdef JP
        description += "æ◊√º∞° µÁ ¿€¿∫ ∫¥¿Ã¥Ÿ.";
#else
        description += "A small bottle of liquid.";
#endif
    else
    {
#ifdef JP
        description += "¿Ã∞Õ¿∫";
#else
        description += "A";
#endif

        switch (item.sub_type)
        {
        case POT_HEALING:
#ifdef JP
            description += " √‡∫πµ«æÓ ¿÷¥¬";
#else
            description += " blessed";
#endif
            break;
        case POT_HEAL_WOUNDS:
#ifdef JP
            description += " ∏∂π˝¿˚ ƒ°∑·¿«";
#else
            description += " magical healing";
#endif
            break;
        case POT_SPEED:
#ifdef JP
            description += " ∏”π˝¿Ã ¥„±‰";
#else
            description += "n enchanted";
#endif
            break;
        case POT_MIGHT:
#ifdef JP
            description += " ∏∂π˝¿«";
#else
            description += " magic";
#endif
            break;
        case POT_POISON:
#ifdef JP
            description += " ø™«— µ∂º∫¿«";
#else
            description += " nasty poisonous";
#endif
            break;
        case POT_PORRIDGE:
#ifdef JP
            description += " πË∏¶ ∫Œ∏£∞‘«œ¥¬";
#else
            description += " filling";
#endif
            break;
        case POT_DEGENERATION:
#ifdef JP
            description += " ¿Ø«ÿ«—";
#else
            description += " noxious";
#endif
            break;
        case POT_DECAY:
#ifdef JP
            description += " ªÁæ««— ∫Œ∆–¿« ¿˙¡÷∞° ∞…∑¡¿÷¥¬";
#else
            description += " vile and putrid cursed";
#endif
            break;
        case POT_WATER:
#ifdef JP
            description += " ¿Ø¿œ«—";
#else
            description += " unique";
#endif
            break;
        case POT_EXPERIENCE:
#ifdef JP
            description += " ¡§∏ª »«∏¢«œ∞Ì ∏≈øÏ ±Õ¡ﬂ«—";
#else
            description += " truly wonderful and very rare";
#endif
            break;
        case POT_MAGIC:
#ifdef JP
            description += " ¿ØøÎ«—";
#else
            description += " valuable";
#endif
            break;
        case POT_STRONG_POISON:
#ifdef JP
            description += " π´º≠øÓ ∏Õµ∂¿«";
#else
            description += " terribly venomous";
#endif
            break;
        }

        description += " ";

        switch (item.sub_type)
        {
        case POT_MIGHT:
        case POT_GAIN_STRENGTH:
        case POT_GAIN_DEXTERITY:
        case POT_GAIN_INTELLIGENCE:
        case POT_LEVITATION:
        case POT_SLOWING:
        case POT_PARALYSIS:
        case POT_CONFUSION:
        case POT_INVISIBILITY:
        case POT_PORRIDGE:
        case POT_MAGIC:
        case POT_RESTORE_ABILITIES:
        case POT_STRONG_POISON:
        case POT_BERSERK_RAGE:
        case POT_CURE_MUTATION:
        case POT_MUTATION:
#ifdef JP
            description += "π∞æ‡¿Ã¥Ÿ.";
#else
            description += "potion";
#endif
            break;
        case POT_HEALING:
#ifdef JP
            description += "æ◊√º¥Ÿ.";
#else
            description += "fluid";
#endif
            break;
        case POT_HEAL_WOUNDS:
#ifdef JP
            description += "ø§∏Øº≠¥Ÿ.";
#else
            description += "elixir";
#endif
            break;
        case POT_SPEED:
#ifdef JP
            description += "¿Ω∑·¥Ÿ.";
#else
            description += "beverage";
#endif
            break;
        case POT_POISON:
        case POT_DECAY:
#ifdef JP
            description += "æ◊√º¥Ÿ.";
#else
            description += "liquid";
#endif
            break;
        case POT_DEGENERATION:
#ifdef JP
            description += "»•«’π∞¿Ã¥Ÿ.";
#else
            description += "concoction";
#endif
            break;
        case POT_WATER:
#ifdef JP
            description += "π∞¡˙¿Ã¥Ÿ.";
#else
            description += "substance";
#endif
            break;
        case POT_EXPERIENCE:
#ifdef JP
            description += "¿Ω∑·¿Ã¥Ÿ.";
#else
            description += "drink";
#endif
            break;
        }

        switch (item.sub_type)
        {
        case POT_HEALING:
        case POT_HEAL_WOUNDS:
        case POT_SPEED:
        case POT_MIGHT:
        case POT_LEVITATION:
        case POT_SLOWING:
        case POT_PARALYSIS:
        case POT_CONFUSION:
        case POT_INVISIBILITY:
        case POT_DEGENERATION:
        case POT_DECAY:
        case POT_MAGIC:
        case POT_RESTORE_ABILITIES:
        case POT_BERSERK_RAGE:
        case POT_CURE_MUTATION:
        case POT_MUTATION:
#ifdef JP
            description += " ¿Ã∞Õ¿∫ ";
#else
            description += " which ";
#endif
            break;
        case POT_GAIN_STRENGTH:
        case POT_GAIN_DEXTERITY:
        case POT_GAIN_INTELLIGENCE:
        case POT_PORRIDGE:
#ifdef JP
            description += "";
#else
            description += " of ";
#endif
            break;
        }

        switch (item.sub_type)
        {
        case POT_HEALING:
#ifdef JP
            description += "æ‡∞£¿« ªÛ√≥∏¶ ƒ°∑·«œ∞Ì, ¡§Ω≈¿ª ¡§»≠«œ∏Á, ∫¥¿ª ≥¥∞‘ «—¥Ÿ.";
#else
            description += "heals some wounds, clears the mind, "
                "and cures diseases";
#endif
            break;
        case POT_HEAL_WOUNDS:
#ifdef JP
            description += "º¯Ωƒ∞£ø° ªÛ√≥∏¶ ƒ°∑·«—¥Ÿ.";
#else
            description += "causes wounds to close and heal "
                "almost instantly";
#endif
            break;
        case POT_SPEED:
#ifdef JP
            description += "∏∂Ω≈ ªÁ∂˜¿« µø¿€¿ª ∞°º”Ω√≈≤¥Ÿ.";
#else
            description += "speeds the actions of anyone who drinks it";
#endif
            break;
        case POT_MIGHT:
#ifdef JP
            description += "∏∂Ω≈ ªÁ∂˜¿« »˚∞˙ π∞∏Æ¿˚¿Œ »˚¿ª ∏≈øÏ ¡ı∞°Ω√≈≤¥Ÿ.";
#else
            description += "greatly increases the strength and "
                "physical power of one who drinks it";
#endif
            break;
        case POT_GAIN_STRENGTH:
        case POT_GAIN_DEXTERITY:
        case POT_GAIN_INTELLIGENCE:
#ifdef JP
            description += "¿ØøÎ«— ∫Ø¿Ã∏¶ ¿œ¿∏≈≤¥Ÿ.";
#else
            description += "beneficial mutation";
#endif
            break;
        case POT_LEVITATION:
#ifdef JP
            description += "∏∂Ω≈ ªÁ∂˜¿Ã ∞≠«œ∞‘ ∫Œ¿Ø«“ ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ.";
#else
            description += "confers great buoyancy on one who consumes it";
#endif
            break;
        case POT_SLOWING:
#ifdef JP
            description += "¥ÁΩ≈¿« µø¿€¿ª ¥¿∏Æ∞‘ «—¥Ÿ.";
#else
            description += "slows your actions";
#endif
            break;
        case POT_PARALYSIS:
#ifdef JP
            description += "¥ÁΩ≈¿« ¿¸Ω≈¿ª ∏∂∫ÒΩ√≈≤¥Ÿ.";
#else
            description += "eliminates your control over your own body";
#endif
            break;
        case POT_CONFUSION:
#ifdef JP
            description += "¥ÁΩ≈¿« ¿Œ¡ˆ¥…∑¬¿ª »•∂ı π◊ ∞®º“Ω√ƒ— "
                           "¿⁄Ω≈¿« «‡µø¿ª ¡¶æÓ«“ ºˆ æ¯∞‘ «—¥Ÿ.";
#else
            description += "confuses your perceptions and reduces "
                "your control over your own actions";
#endif
            break;
        case POT_INVISIBILITY:
#ifdef JP
            description += "¥Ÿ∏• ¿⁄¿« Ω√æﬂø°º≠ ¥ÁΩ≈¿ª ∫∏¿Ã¡ˆ æ ∞‘ «—¥Ÿ.";
#else
            description += "hides you from the sight of others";
#endif
            break;
        case POT_PORRIDGE:
#ifdef JP
            description += "∞Óπ∞∞˙ º∂¿Ø¡˙¿Ã ∏π¿Ã ∆˜«‘µ» ¡◊¿Ã¥Ÿ.";
#else
            description += "sludge, high in cereal fibre";
#endif
            break;
        case POT_DEGENERATION:
#ifdef JP
            description += "¥ÁΩ≈¿« ∏ˆ∞˙ ≥˙, π›ªÁΩ≈∞Êø° "
                           "≤˚¬Ô«— «««ÿ∏¶ ∞°¡Æø¬¥Ÿ.";
#else
            description += "can do terrible things to your "
                "body, brain and reflexes";
#endif
            break;
        case POT_DECAY:
#ifdef JP
            description += "¥ÁΩ≈¿« ªÏ¿ª Ω‚∞‘ «—¥Ÿ.";
#else
            description += "causes your flesh to decay "
                "before your very eyes";
#endif
            break;
        case POT_WATER:
#ifdef JP
            description += "¥Î∫Œ∫–¿« ª˝∏Ì√ºø°∞‘ ¡ˆ±ÿ»˜ ¡ﬂø‰«— ∞Õ¿Ã¥Ÿ. ";
#else
            description += ", vital for the existence of most life";
#endif
            break;
        case POT_MAGIC:
#ifdef JP
            description += "∏∂Ω≈ ªÁ∂˜ø°∞‘ ∏∂≥™∏¶ √§øˆ¡ÿ¥Ÿ. ";
#else
            description += "grants a person with an "
                "infusion of magical energy";
#endif
            break;
        case POT_RESTORE_ABILITIES:
#ifdef JP
            description += "∏∂Ω≈ ªÁ∂˜¿« ¥…∑¬ƒ°∏¶ »∏∫π«—¥Ÿ.";
#else
            description += "restores the abilities of one who drinks it";
#endif
            break;
        case POT_BERSERK_RAGE:
#ifdef JP
            description += "∏∂Ω≈ ªÁ∂˜¿∫ ∫–≥Îø° ∫¸¡ˆ∞‘ µ»¥Ÿ.";
#else
            description += "can send one into an incoherent rage";
#endif
            break;
        case POT_CURE_MUTATION:
#ifdef JP
            description += "∏∂Ω≈ ªÁ∂˜¿« ∫Ø¿Ã∏¶ ∏Ó∞≥ »§¿∫ ∏µŒ æ¯æÿ¥Ÿ.";
#else
            description += "removes some or all of any mutations "
                "which may be afflicting you";
#endif
            break;
        case POT_MUTATION:
#ifdef JP
            description += "¥ÁΩ≈ø°∞‘ ∏≈øÏ ¿ÃªÛ«œ∞‘ ¿€øÎ«—¥Ÿ.";
#else
            description += "does very strange things to you";
#endif
            break;
        }

#ifdef JP
        description += " ";
#else
        description += ". ";
#endif

        switch (item.sub_type)
        {
        case POT_HEALING:
        case POT_HEAL_WOUNDS:
#ifdef JP
            description += "∏∏æ‡ ¿Ã æ‡¿ª ƒ¡µº«¿Ã √÷ªÛ¿œ∂ß≥™ ∞≈¿« √÷ªÛ¿œ∂ß ∏∂Ω√∏È "
                           "¡ˆº”¿˚¿∏∑Œ ∫ŒªÛ¿ª ";
#else
            description += "If one uses it when they are "
                "at or near full health, it can also ";
#endif

            if (item.sub_type == POT_HEALING)
#ifdef JP
                description += "¡∂±›æø ";
            description += "ƒ°¿Ø«—¥Ÿ. ";
#else
                description += "slightly ";
            description += "repair permanent injuries. ";
#endif
            break;
        }

        //default:
        //    DEBUGSTR("Unknown potion");          // I had no idea where to put this back 16jan2000 {dlb}
    }

    description += "$";

    return (description);
}


//---------------------------------------------------------------
//
// describe_scroll
//
//---------------------------------------------------------------
static std::string describe_scroll( const item_def &item )
{
    std::string description;

    description.reserve(64);

    if (get_ident_type( OBJ_SCROLLS, item.sub_type ) != ID_KNOWN_TYPE)
#ifdef JP
        description += "∏∂π˝¿« πÆ¿Â¿∏∑Œ µ§»˘ ¡æ¿Ã µŒ∑Á∏∂∏Æ¿Ã¥Ÿ.";
#else
        description += "A scroll of paper covered in magical writing.";
#endif
    else
    {
        switch (item.sub_type)
        {
        case SCR_IDENTIFY:
#ifdef JP
            description += "æÓ∂∞«— π∞«∞¿Ã∂Ûµµ ∞®¡§«ÿ≥æ ºˆ ¿÷¥¬ $"
                           "¿ØøÎ«— ∏∂π˝ µŒ∑Á∏∂∏Æ¥Ÿ. ";
#else
            description += "This useful magic scroll allows you to "
                "determine the properties of any object. ";
#endif
            break;

        case SCR_TELEPORTATION:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æø° æ≤¿Œ ±€¿ª ¿–¿∏∏È ¥ÁΩ≈¿∫ ∑£¥˝«— ¡ˆ¡°¿∏∑Œ º¯∞£¿Ãµøµ»¥Ÿ. ";
#else
            description += "Reading the words on this scroll "
                "translocates you to a random position. ";
#endif
            break;

        case SCR_FEAR:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ∏¶ ¿–¿∫ ¿⁄¿« Ω√æﬂ ≥ªø° ¿÷¥¬ ¿⁄¥¬ $"
                           "±ÿΩ…«— ∞¯∆˜ø° »€ΩŒ¿Œ¥Ÿ. ";
#else
            description += "This scroll causes great fear in those "
                "who see the one who reads it. ";
#endif
            break;

        case SCR_NOISE:
#ifdef JP
            description += "¿Ã ∏¯µ» ¿Â≥≠¿« µŒ∑Á∏∂∏Æ¥¬, ¡æ¡æ ªÁæ««— Ω«Ω¿ª˝ø° ¿««ÿ $"
                           "∏∂π˝ªÁ¿« ¡¸ æ»ø° ≥÷æÓ¡Æ ±ÿΩ…«— º“¿Ω¿ª ¿œ¿∏≈≤¥Ÿ. $"
                           "±◊ π€ø°¥¬ øÎµµ∞° æ¯¥Ÿ. ";
#else
            description += "This prank scroll, often slipped into a wizard's "
                "backpack by a devious apprentice, causes a loud noise. "
                "It is not otherwise noted for its usefulness. ";
#endif
            break;

        case SCR_REMOVE_CURSE:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ∏¶ ¿–¿∏∏È ¥ÁΩ≈¿Ã ¿Â∫Ò«— æ∆¿Ã≈€ø° ∞…∑¡¿÷¥¯ $"
                           "¿˙¡÷∞° ¡¶∞≈µ»¥Ÿ. ";
#else
            description += "Reading this scroll removes curses from "
                "the items you are using. ";
#endif
            break;

        case SCR_DETECT_CURSE:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ ¥ÁΩ≈¿« º“¡ˆ«∞ ¡ﬂø°º≠ $"
                           "¿˙¡÷πﬁ¿∫ π∞«∞¿ª æÀæ∆≥æ ºˆ ¿÷∞‘ «—¥Ÿ. ";
#else
            description += "This scroll allows you to detect the presence "
                "of cursed items among your possessions. ";
#endif
            break;

        case SCR_SUMMONING:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ∏¶ ¿–¿∏∏È æÓ∫ÒΩ∫∑Œ¿« ¿‘±∏∞° ø≠∑¡ "
                           "π´º≠øÓ ±´π∞¿ª «—¡§µ» Ω√∞£µøæ» ∫“∑Øø¬¥Ÿ. ";
#else
            description += "This scroll opens a conduit to the Abyss "
                "and draws a terrible beast to this world "
                "for a limited time. ";
#endif
            break;

        case SCR_ENCHANT_WEAPON_I:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ π´±‚ø° ∏∂π˝¿ª ∞…æÓ ¿¸≈ıø°º≠ ¥ı ¿˚¿ª ¿ﬂ ∏¬√‚ ºˆ ¿÷∞‘ «—¥Ÿ, $"
                           "¿ÃπÃ ∏≈øÏ ∞≠»≠ µ«æÓ ¿÷¥¬ π´±‚ø°¥¬ Ω«∆–«“ ºˆµµ ¿÷¥Ÿ. ";
#else
            description += "This scroll places an enchantment on a weapon, "
                "making it more accurate in combat. It may fail "
                "to affect weapons already heavily enchanted. ";
#endif
            break;

        case SCR_ENCHANT_ARMOUR:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ πÊæÓ±∏∏¶ ∏∂π˝¿∏∑Œ ∞≠»≠«—¥Ÿ. ";
#else
            description += "This scroll places an enchantment "
                "on a piece of armour. ";
#endif
            break;

        case SCR_TORMENT:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ ±ÿΩ…«— ∞Ì≈Î¿ª ¿Øπﬂ«œ¥¬ ¡ˆø¡¿« »˚¿ª ∫“∑ØøÕ $"
                           "±Ÿ√≥ø° ¿÷¥¬ ª˝∏Ì√ºø°∞‘ «««ÿ∏¶ ¡ÿ¥Ÿ. $"
                           "¥ÁΩ≈¿ª ∆˜«‘«ÿº≠! ";
#else
            description += "This scroll calls on the powers of Hell to "
                "inflict great pain on any nearby creature - "
                "including you! ";
#endif
            break;

        case SCR_RANDOM_USELESSNESS:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ æÓ∂∞«— ±‚¥…¿Ã ¿÷¥¬ ∞Õ√≥∑≥ ∫∏ø©¡Æ $"
                           "±◊ øœ∫Æ«œ∞‘ æµ∏æ¯¿Ω¿ª ¥ÁΩ≈¿∏∑Œ∫Œ≈Õ º˚±‚∞Ì ¿÷¥Ÿ. ";
#else
            description += "It is easy to be blinded to the essential "
                "uselessness of this scroll by the sense of achievement "
                "you get from getting it to work at all.";
#endif
                // -- The Hitchhiker's Guide to the Galaxy (paraphrase)
            break;

        case SCR_CURSE_WEAPON:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ π´±‚ø° ¿˙¡÷∏¶ ∞«¥Ÿ. ";
#else
            description += "This scroll places a curse on a weapon. ";
#endif
            break;

        case SCR_CURSE_ARMOUR:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ πÊæÓ±∏ø° ¿˙¡÷∏¶ ∞«¥Ÿ. ";
#else
            description += "This scroll places a curse "
                "on a piece of armour. ";
#endif
            break;

        case SCR_IMMOLATION:
#ifdef JP
            description += "∏∂π˝ µŒ∑Á∏∂∏Æ¿« µ⁄ø° ¿€¿∫ πÆ±∏∞° ªı∞‹¡Æ¿÷¥Ÿ. $ $"
                           "\"∞Ê∞Ì:≥ª∫Œø° æ–∑¬¿Ã ¿÷Ω¿¥œ¥Ÿ. $"
                           "∫“¿Ã ∫Ÿ±‚ Ω¨øÓ π∞√ºø∑ø°º≠ ªÁøÎ«œ¡ˆ ∏∂Ω Ω√ø¿.\"";
#else
            description += "Small writing on the back of the scroll reads: "
                "\"Warning: contents under pressure.  Do not use near"
                " flammable objects.\"";
#endif
            break;

        case SCR_BLINKING:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ ¿–¿∫ ªÁ∂˜ø°∞‘ ¡¶æÓ∞°¥…«— ¥‹∞≈∏Æ¿« $"
                           "º¯∞£¿Ãµø¿ª ¿œ¿∏≈≤¥Ÿ. $"
                           "¥Ÿ∏∏ ¡¶æÓ∞°¥…«— º¯∞£¿Ãµø¿∫ $"
                           "ªÁøÎ¿⁄ø°∞‘ ∏∂π˝¿˚¿Œ ø¿ø∞¿ª ¿œ¿∏≈≥ ºˆ ¿÷¿∏¥œ ¡÷¿««ÿæﬂ «—¥Ÿ. ";
#else
            description += "This scroll allows its reader to teleport "
                "a short distance, with precise control.  Be wary that "
                "controlled teleports will cause the subject to "
                "become contaminated with magical energy. ";
#endif
            break;

        case SCR_PAPER:
#ifdef JP
            description += "«•¡ˆ∏∏¿ª ∫¡µµ, ¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ ∫ÒæÓ¿÷¥Ÿ. ";
#else
            description += "Apart from a label, this scroll is blank. ";
#endif
            break;

        case SCR_MAGIC_MAPPING:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ ¿–¥¬ ªÁ∂˜¿« ¡÷∫Ø ¡ˆ«¸¿ª π‡»˘¥Ÿ. ";
#else
            description += "This scroll reveals the nearby surroundings "
                "of one who reads it. ";
#endif
            break;

        case SCR_FORGETFULNESS:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ ¬•¡ı≥Ø ¡§µµ∑Œ πÊ«‚∞®∞¢¿ª ªÛΩ«Ω√≈≤¥Ÿ. ";
#else
            description += "This scroll induces "
                "an irritating disorientation. ";
#endif
            break;

        case SCR_ACQUIREMENT:
#ifdef JP
            description += "¿Ã »«∏¢«— ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ $"
                           "¿–¿∫ ªÁ∂˜¿« ¥´ æ’ø° $"
                           "∞°ƒ°¿÷¥¬ æ∆¿Ã≈€¿ª ª˝º∫Ω√≈≤¥Ÿ. $"
                           "¿Ã∞Õ¿∫ ∆Ø»˜ ∏∂π˝∏∏¿ª ∆Ø»≠«— ∏∂π˝ªÁø°∞‘ ¡ﬂø‰Ω√ µ«¥¬µ• $"
                           "ø÷≥ƒ«œ∏È ¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ∏¶ ≈Î«ÿº≠ ∞≠∑¬«— ∏∂π˝ µŒ∑Á∏∂∏Æ∏¶ æÚ¥¬ ∞Õµµ ∞°¥…«œ±‚ ∂ßπÆ¿Ã¥Ÿ.  ";
#else
            description += "This wonderful scroll causes the "
                "creation of a valuable item to "
                "appear before the reader. "
                "It is especially treasured by specialist "
                "magicians, as they can use it to obtain "
                "the powerful spells of their specialty. ";
#endif
            break;

        case SCR_ENCHANT_WEAPON_II:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ π´±‚ø° ∏∂π˝¿ª ∞…æÓ ¿¸≈ıø°º≠ $"
                           "¿˚ø°∞‘ ¥ı ≈´ µ•πÃ¡ˆ∏¶ ¡Ÿ ºˆ ¿÷∞‘ «—¥Ÿ. $"
                           "¿ÃπÃ ∏≈øÏ ∞≠»≠ µ«æÓ ¿÷¥¬ π´±‚ø°¥¬ Ω«∆–«“ ºˆµµ ¿÷¥Ÿ.";
#else
            description += "This scroll places an enchantment on a weapon, "
                "making it inflict greater damage in combat. "
                "It may fail to affect weapons already "
                "heavily enchanted. ";
#endif
            break;

        case SCR_VORPALISE_WEAPON:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ ¿ß∑¬¿ª ∫Òæ‡¿˚¿∏∑Œ ªÛΩ¬Ω√≈∞¥¬ ∏∂π˝¿∏∑Œ π´±‚∏¶ ∞≠»≠«—¥Ÿ. $"
                           "π´±‚∞° ¿ÃπÃ æÓ∂≤ ∏∂π˝¿˚ º”º∫¿ª ∞°¡ˆ∞Ì ¿÷¥¬ ∞ÊøÏø°¥¬ $"
                           "(¡Ô º”º∫π´±‚≥™ æ∆∆º∆—∆Æø°¥¬) $"
                           "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ∏¶ ªÁøÎ«œ¥¬ ∞Õ¿∫ √ﬂ√µ«œ¡ˆ æ ¥¬¥Ÿ. ";
#else
            description += "This scroll enchants a weapon so as to make "
                "it far more effective at inflicting harm on "
                "its wielder's enemies. Using it on a weapon "
                "already affected by some kind of special "
                "enchantment (other than that produced by a "
                "normal scroll of enchant weapon) is not advised. ";
#endif
            break;

        case SCR_RECHARGING:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ∏¶ ¿–¿∫ ªÁ∂˜¿Ã º’ø° µÈ∞Ì ¿÷¥¬ ∏∂π˝∫¿¿« ªÁøÎ »∏ºˆ∏¶ √Ê¿¸Ω√≈≤¥Ÿ.";
#else
            description += "This scroll restores the charges of "
                "any magical wand wielded by its reader. ";
#endif
            break;

        case SCR_ENCHANT_WEAPON_III:
#ifdef JP
            description += "¿Ã ∏∂π˝ µŒ∑Á∏∂∏Æ¥¬ π´±‚∏¶ ∏∂π˝¿∏∑Œ ∞≠»≠«œø© $"
                           "¿¸≈ıø°º≠ ∫∏¥Ÿ ≥Ù¿∫ º∫¥…¿ª πﬂ»÷«“ ºˆ ¿÷∞‘ «—¥Ÿ. $"
                           "¿ÃπÃ ∏≈øÏ ∞≠»≠ µ«æÓ ¿÷¥¬ π´±‚ø°¥¬ Ω«∆–«“ ºˆµµ ¿÷¥Ÿ. ";
#else
            description += "This scroll enchants a weapon to be "
                "far more effective in combat. Although "
                "it can be used in the creation of especially "
                "enchanted weapons, it may fail to affect those "
                "already heavily enchanted. ";
#endif
            break;

        default:
#ifdef JP
            DEBUGSTR("æÀ∑¡¡ˆ¡ˆ æ ¿∫ ∏∂π˝ µŒ∑Á∏∂∏Æ");
#else
            DEBUGSTR("Unknown scroll");
#endif
        }
    }

    description += "$";

    return (description);
}


//---------------------------------------------------------------
//
// describe_jewellery
//
//---------------------------------------------------------------
static std::string describe_jewellery( const item_def &item, char verbose)
{
    std::string description;

    description.reserve(200);

    if (is_unrandom_artefact( item ) && strlen(unrandart_descrip(1, item)) != 0)
    {
        description += "$";
        description += unrandart_descrip(1, item);
        description += "$$";
    }
    else if ((!is_random_artefact( item )
            && get_ident_type( OBJ_JEWELLERY, item.sub_type ) != ID_KNOWN_TYPE)
            || (is_random_artefact( item )
            && item_not_ident( item, ISFLAG_KNOW_TYPE )))
    {
#ifdef JP
        description += "¿Ã∞Õ¿∫ ¿ÂΩ≈±∏¥Ÿ. ";
#else
        description += "A piece of jewellery.";
#endif
    }
    else if (verbose == 1 || is_random_artefact( item ))
    {
        switch (item.sub_type)
        {
        case RING_REGENERATION:
#ifdef JP
            description += "¿Ã ≥Ó∂ÛøÓ π›¡ˆ∏¶ ¬¯øÎ¿⁄«— ¿⁄ø°∞‘ ¿Áª˝∑¬¿ª ∞≠»≠Ω√≈∞¡ˆ∏∏ "
                           "æ∆øÔ∑Øº≠ Ω≈¡¯¥ÎªÁ¿« º“∫Ò º”µµµµ ∞°º”Ω√≈≤¥Ÿ. ";
#else
            description += "This wonderful ring greatly increases the "
                "recuperative powers of its wearer, but also "
                "considerably speeds his or her metabolism. ";
#endif
            break;

        case RING_PROTECTION:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄∏¶ ∫ŒªÛ¿∏∑Œ∫Œ≈Õ ∫∏»£«œµÁ∞° »§¿∫ ∫ŒªÛ¿ª πﬁ±‚ Ω±∞‘ «—¥Ÿ. "
                "»ø∞˙¥¬ π›¡ˆ¿« ºˆƒ°ø° ¿«¡∏«—¥Ÿ. ";
#else
                "This ring either protects its wearer from harm or makes "
                "them more vulnerable to injury, to a degree dependent "
                "on its power. ";
#endif
            break;

        case RING_PROTECTION_FROM_FIRE:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄∏¶ ø≠±‚øÕ »≠ø∞¿∏∑Œ∫Œ≈Õ ∫∏»£«ÿ¡ÿ¥Ÿ. ";
#else
                "This ring provides protection from heat and fire. ";
#endif
            break;

        case RING_POISON_RESISTANCE:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄∏¶ µ∂¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. ";
#else
                "This ring provides protection from the effects of poisons and venom. ";
#endif
            break;

        case RING_PROTECTION_FROM_COLD:
#ifdef JP
            description += "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄∏¶ ≥√±‚∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. ";
#else
            description += "This ring provides protection from cold. ";
#endif
            break;

        case RING_STRENGTH:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄¿« »˚¿ª ¡ı∞°Ω√≈∞∞≈≥™ ∞®º“Ω√≈≤¥Ÿ. "
                "»ø∞˙¥¬ π›¡ˆ¿« ºˆƒ°ø° ¥ﬁ∑¡¿÷¥Ÿ. ";
#else
                "This ring increases or decreases the physical strength "
                "of its wearer, to a degree dependent on its power. ";
#endif
            break;

        case RING_SLAYING:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄¿« ±Ÿ∞≈∏ÆøÕ ø¯∞≈∏Æ ¿¸≈ı¥…∑¬¿ª ∞≠»≠Ω√≈≤¥Ÿ. ";
#else
                "This ring increases the hand-to-hand and missile combat "
                "skills of its wearer.";
#endif
            break;

        case RING_SEE_INVISIBLE:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ∏∂π˝¿∏∑Œ ¬¯øÎ¿⁄ø°∞‘ ≈ı∏Ì√º∏¶ "
                "∫º ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ. ";
#else
                "This ring allows its wearer to see those things hidden "
                "from view by magic. ";
#endif
            break;

        case RING_INVISIBILITY:
            description +=
#ifdef JP
                "¿Ã ∞≠∑¬«— π›¡ˆ¥¬ πﬂµø«œ¥¬ ∞Õ¿∏∑Œ ¬¯øÎ¿⁄∏¶ ∫∏¿Ã¡ˆ æ ∞‘ «œ¡ˆ∏∏ "
                "≈ı∏Ì»≠¡ﬂø°¥¬ Ω≈¡¯¥ÎªÁ¿« º”µµ∞° ≈´ ∆¯¿∏∑Œ ¡ı∞°«—¥Ÿ. ";
#else
                "This powerful ring can be activated to hide its wearer "
                "from the view of others, but increases the speed of his "
                "or her metabolism greatly while doing so. ";
#endif
            break;

        case RING_HUNGER:
            description +=
#ifdef JP
                "¿Ã ¿˙¡÷πﬁ¿∫ π›¡ˆ¥¬ ¬¯øÎ¿⁄∏¶ ∏≈øÏ ª°∏Æ πË∞Ì«¡∞‘ ∏∏µÁ¥Ÿ. ";
#else
                "This accursed ring causes its wearer to hunger "
                "considerably more quickly. ";
#endif
            break;

        case RING_TELEPORTATION:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ∞°≤˚æø ¬¯øÎ¿⁄∏¶ ∑£¥˝«œ∞‘ ¿ÃµøΩ√≈≤¥Ÿ. "
                "¿«µµ¿˚¿∏∑Œ º¯∞£¿Ãµø¿ª ªÁøÎ«œ¥¬ ∞Õµµ ∞°¥…«œ∞‘ «—¥Ÿ. ";
#else
                "This ring occasionally exerts its power to randomly "
                "translocate its wearer to another place, and can be "
                "deliberately activated for the same effect. ";
#endif
            break;

        case RING_EVASION:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄¿« »∏««¥…∑¬¿ª ∞≠»≠«œ∞≈≥™ æ‡»≠«—¥Ÿ. "
                "»ø∞˙¥¬ π›¡ˆ¿« ºˆƒ°ø° ¥ﬁ∑¡¿÷¥Ÿ. ";
#else
                "This ring makes its wearer either more or less capable "
                "of avoiding attacks, depending on its degree "
                "of enchantment. ";
#endif
            break;

        case RING_SUSTAIN_ABILITIES:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄∏¶ »˚, πŒ√∏, ¡ˆ¥…¿« º’Ω«∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. ";
#else
                "This ring protects its wearer from the loss of their "
                "strength, dexterity and intelligence. ";
#endif
            break;

        case RING_SUSTENANCE:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄ø°∞‘ ø°≥ ¡ˆ∏¶ ∞¯±ﬁ«œ±‚ ∂ßπÆø° "
                "¬¯øÎ¿⁄¥¬ ∫∏¥Ÿ ¥˙ πË∞Ì«¡∞‘ µ»¥Ÿ. ";
#else
                "This ring provides energy to its wearer, so that they "
                "need eat less often. ";
#endif
            break;

        case RING_DEXTERITY:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄¿« πŒ√∏ ¥…∑¬ƒ°∏¶ ¡ı∞°Ω√≈∞∞≈≥™ ∞®º“Ω√≈≤¥Ÿ. "
                "»ø∞˙¥¬ π›¡ˆ¿« ºˆƒ°ø° ¥ﬁ∑¡¿÷¥Ÿ. ";
#else
                "This ring increases or decreases the dexterity of its "
                "wearer, depending on the degree to which it has been "
                "enchanted. ";
#endif
            break;

        case RING_INTELLIGENCE:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄¿« ¡ˆ¥… ¥…∑¬ƒ°∏¶ ¡ı∞°Ω√≈∞∞≈≥™ ∞®º“Ω√≈≤¥Ÿ. "
                "»ø∞˙¥¬ π›¡ˆ¿« ºˆƒ°ø° ¥ﬁ∑¡¿÷¥Ÿ. ";
#else
                "This ring increases or decreases the mental ability of "
                "its wearer, depending on the degree to which it has "
                "been enchanted. ";
#endif
            break;

        case RING_WIZARDRY:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄¿« ¡÷πÆ ªÁøÎ ¥…∑¬¿ª ¡ı∞°Ω√≈≤¥Ÿ. ";
#else
                "This ring increases the ability of its wearer to use "
                "magical spells. ";
#endif
            break;

        case RING_MAGICAL_POWER:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄¿« ∏∂∑¬¿ª ¡ı∞°Ω√≈≤¥Ÿ. ";
#else
                "This ring increases its wearer's reserves of magical "
                "power. ";
#endif
            break;

        case RING_LEVITATION:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄ø°∞‘ ∫Œ¿Ø¥…∑¬¿ª ªÁøÎ«“ ºˆ ¿÷∞‘ «—¥Ÿ.  ";
#else
                "This ring allows its wearer to hover above the floor. ";
#endif
            break;

        case RING_LIFE_PROTECTION:
            description +=
#ifdef JP
                "¿Ã √‡∫ππﬁ¿∫ π›¡ˆ¥¬ ¬¯øÎ¿⁄∏¶ ¿Ωø°≥ ¡ˆ∑Œ∫Œ≈Õ ∫∏»£«œø© "
                "æµ•µÂøÕ ∞≠∑…º˙ ∏∂π˝¿∏∑Œ ¿Œ«— ª˝∏Ì»Ìºˆø° ¥Î«ÿ æÓ¥¿¡§µµ¿« ∏Èø™¿ª ¡ÿ¥Ÿ. ";
#else
                "This blessed ring protects the life-force of its wearer "
                "from negative energy, making them partially immune to "
                "the draining effects of undead and necromantic magic. ";
#endif
            break;

        case RING_PROTECTION_FROM_MAGIC:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄ø°∞‘ ¿˚¥Î¿˚¿Œ ∏∂π˝ø° ¥Î«— ∏∂π˝¿˙«◊¿ª ¡ı∞°Ω√≈≤¥Ÿ. ";
#else
                "This ring increases its wearer's resistance to "
                "hostile enchantments. ";
#endif
            break;

        case RING_FIRE:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄∏¶ »≠ø∞¿« »˚∞˙ ¥ı ∞°±ı∞‘ ∏∏µÈæÓ ¡ÿ¥Ÿ. "
                "¬¯øÎ¿⁄¥¬ »≠ø∞ø° ¥Î«— ≥ªº∫¿ª æÚ¿Ω∞˙ µøΩ√ø° "
                "»≠ø∞ ∏∂π˝¿ª ¥ı »ø∞˙¿˚¿∏∑Œ ªÁøÎ«“ ºˆ ¿÷∞‘ µ«¡ˆ∏∏ "
                "≥√±‚ø° ¥Î«— «««ÿ∏¶ ¥ı ∏π¿Ã ¿‘∞‘ µ»¥Ÿ. ";
#else
                "This ring brings its wearer more in contact with "
                "the powers of fire. He or she gains resistance to "
                "heat and can use fire magic more effectively, but "
                "becomes more vulnerable to the effects of cold. ";
#endif
            break;

        case RING_ICE:
            description +=
#ifdef JP
                "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄∏¶ ≥√±‚¿« »˚∞˙ ¥ı ∞°±ı∞‘ ∏∏µÈæÓ ¡ÿ¥Ÿ. "
                "¬¯øÎ¿⁄¥¬ ≥√±‚ø° ¥Î«— ≥ªº∫¿ª æÚ¿Ω∞˙ µøΩ√ø° "
                "≥√±‚ ∏∂π˝¿ª ¥ı »ø∞˙¿˚¿∏∑Œ ªÁøÎ«“ ºˆ ¿÷∞‘ µ«¡ˆ∏∏ "
                "»≠ø∞ø° ¥Î«— «««ÿ∏¶ ¥ı ∏π¿Ã ¿‘∞‘ µ»¥Ÿ. ";
#else
                "This ring brings its wearer more in contact with "
                "the powers of cold and ice. He or she gains resistance "
                "to cold and can use ice magic more effectively, but "
                "becomes more vulnerable to the effects of fire. ";
#endif
            break;

        case RING_TELEPORT_CONTROL:
#ifdef JP
            description += "¿Ã π›¡ˆ¥¬ ¬¯øÎ¿⁄∞° æÓ∂∞«— º¯∞£¿Ãµøø° ¥Î«ÿº≠µµ ∏Ò¿˚¡ˆ∏¶ "
                           "¡¶æÓ«“ ºˆ ¿÷∞‘ «—¥Ÿ, ¥Ÿ∏∏ øœ∫Æ«œ∞‘ ¡¶æÓ«“ ºˆ¥¬ æ¯¥Ÿ. "
                           "∞Ì√ºæ»¿∏∑Œ º¯∞£¿Ãµø¿ª Ω√µµ«ﬂ¿ª ∞ÊøÏ¥¬ ∑£¥˝«— º¯∞£¿Ãµø¿Ã µ»¥Ÿ." 
                           "∂« º¯∞£¿Ãµø¿« ¡¶æÓ¥¬ ∏∂π˝ø°≥ ¡ˆø° ¿««ÿ "
                           "ªÁøÎ¿⁄∞° ø¿ø∞µ… ºˆ ¿÷¿Ω¿ª º˜¡ˆ«ÿæﬂ «—¥Ÿ. ";
#else
            description += "This ring allows its wearer to control the "
                "destination of any teleportation, although without "
                "perfect accuracy.  Trying to teleport into a solid "
                "object will result in a random teleportation, at "
                "least in the case of a normal teleportation.  Also "
                "be wary that controlled teleports will contaminate "
                "the subject with residual magical energy.";
#endif
            break;

        case AMU_RAGE:
            description +=
#ifdef JP
                "¿Ã ∫Œ¿˚¿∫ ¬¯øÎ¿⁄∏¶ ±§∆¯»≠¿« ªÛ≈¬∑Œ µÈæÓ∞• ºˆ ¿÷∞‘ «—¥Ÿ. "
                "∂« ±§∆¯»≠¿« πﬂµø¿ª º∫∞¯Ω√≈≥ »Æ∑¸µµ ¡ı∞°Ω√≈≤¥Ÿ.  "
                "∞‘¥Ÿ∞° ±§∆¯»≠∞° ±˙æ˙¿ª ∂ß ±‚¿˝«“ »Æ∑¸µµ ∞®º“Ω√≈≤¥Ÿ. ";
#else
                "This amulet enables its wearer to attempt to enter "
                "a state of berserk rage, and increases their chance "
                "of successfully doing so.  It also partially protects "
                "the user from passing out when coming out of that rage. ";
#endif
            break;

        case AMU_RESIST_SLOW:
            description +=
#ifdef JP
                "¿Ã ∫Œ¿˚¿∫ ¬¯øÎ¿⁄∏¶ ∏∂π˝¿˚¿Œ ∞®º”¿∏∑Œ∫Œ≈Õ ∫∏»£«œ∏Á "
                "∂««— ∞°º” ∏∂π˝¿« ¡ˆº” Ω√∞£¿ª ¡ı∞°Ω√ƒ—¡ÿ¥Ÿ. ";
#else
                "This amulet protects its wearer from some magically "
                "induced forms of slowness, and increases the duration "
                "of enchantments which speed his or her actions. ";
#endif
            break;

        case AMU_CLARITY:
            description +=
#ifdef JP
                "¿Ã ∫Œ¿˚¿∫ ¬¯øÎ¿⁄∏¶ »•∂ı¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. ";
#else
                "This amulet protects its wearer from some forms of "
                "mental confusion. ";
#endif
            break;

        case AMU_WARDING:
            description +=
#ifdef JP
                "¿Ã ∫Œ¿˚¿∫ ∏∂π˝¿∏∑Œ º“»Øµ» ∏ÛΩ∫≈Õ∑Œ∫Œ≈Õ¿« ∞¯∞›¿ª ∏∑æ∆¡ÿ¥Ÿ.  ";
#else
                "This amulet repels some of the attacks of creatures "
                "which have been magically summoned. ";
#endif
            break;

        case AMU_RESIST_CORROSION:
            description +=
#ifdef JP
                "¿Ã ∫Œ¿˚¿∫ ¬¯øÎ¿⁄¿« πÊæÓ±∏øÕ π´±‚∏¶ ªÍ¿∏∑Œ ¿Œ«— ∫ŒΩƒ¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. "
                "¥Ÿ∏∏ ∫∏»£∞° øœ∫Æ«œ¥Ÿ∞Ì¥¬ «“ ºˆ æ¯¥Ÿ. ";
#else
                "This amulet protects the armour and weaponry of its "
                "wearer from corrosion caused by acids, although not "
                "infallibly so. ";
#endif
            break;

        case AMU_THE_GOURMAND:
            description +=
#ifdef JP
                "¿Ã ∫Œ¿˚¿∫ ¬¯øÎ¿⁄∞° ∫Œ∆–«— ∞Ì±‚∏¶ πÆ¡¶æ¯¿Ã ∏‘∞‘ «ÿ¡ÿ¥Ÿ. "
                "±◊∑Ø≥™ µ∂¿Ã ¿÷∞≈≥™, ¿˙¡÷πﬁ¿∫ ∞Ì±‚¥¬ ø©¿¸»˜ ¿ß«Ë«œ¥Ÿ. ";
#else
                "This amulet allows its wearer to consume meat in "
                "various states of decay without suffering unduly as "
                "a result. Poisonous or cursed flesh is still not "
                "recommended. ";
#endif
            break;

        case AMU_CONSERVATION:
            description +=
#ifdef JP
                "¿Ã ∫Œ¿˚¿∫ ¬¯øÎ¿⁄¿« º“¡ˆ«∞¿ª ∆ƒ±´∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. "
                "±◊∑Ø≥™ ∫∏»£∞° øœ∫Æ«œ¥Ÿ∞Ì¥¬ «“ ºˆ æ¯¥Ÿ. ";
#else
                "This amulet protects some of the possessions of "
                "its wearer from outright destruction, but not "
                "infallibly so. ";
#endif
            break;

        case AMU_CONTROLLED_FLIGHT:
            description +=
#ifdef JP
                "¿Ã ∫Œ¿˚¿« ¬¯øÎ¿⁄¥¬ ∏∂π˝¿˚¿Œ »˚¿∏∑Œ ∞¯¡ﬂø° ∂ﬂ∞Ì ¿÷¿ª ∂ßø° "
                "∫Ò«‡ øÓµø¿ª æÓ∂≤ ¡§µµ ¡¶æÓ«“ ºˆ∞° ¿÷¥Ÿ. "
                "øπ∏¶ µÈ∏È, ¬¯øÎ¿⁄∞° ∞¯¡ﬂ ∫ŒæÁ¿ª ¡§¡ˆ«œ¥¬ ∞Õ æ¯¿Ã "
                "∞Ë¥‹¿ª ≈Î«ÿ ≥ª∑¡∞°∞≈≥™ πŸ¥⁄ø° ¿÷¥¬ æ∆¿Ã≈€¿ª ¡÷øÔ ºˆ∞° ¿÷¥Ÿ.";
#else
                "Should the wearer of this amulet be levitated "
                "by magical means, he or she will be able to exercise "
                "some control over the resulting motion. This allows "
                "the descent of staircases and the retrieval of items "
                "lying on the ground, for example, but does not "
                "deprive the wearer of the benefits of levitation. ";
#endif
            break;

        case AMU_INACCURACY:
            description +=
#ifdef JP
                "¿Ã ∫Œ¿˚¿∫ ¬¯øÎ¿⁄∞° ±Ÿ¡¢¿¸≈ıø°º≠ ¥ı ¿ﬂ ∏¬√ﬂ¡ˆ ∏¯«œ∞‘ «—¥Ÿ. ";
#else
                "This amulet makes its wearer less accurate in hand combat. ";
#endif
            break;

        case AMU_RESIST_MUTATION:
            description +=
#ifdef JP
                "¿Ã ∫Œ¿˚¿∫ ¬¯øÎ¿⁄∏¶ ∫Ø¿Ã∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. "
                "±◊∑Ø≥™ ∫∏»£∞° øœ∫Æ«— ∞Õ¿∫ æ∆¥œ¥Ÿ. ";
#else
                "This amulet protects its wearer from mutations, "
                "although not infallibly so. ";
#endif
            break;

        default:
#ifdef JP
            DEBUGSTR("æÀ∑¡¡ˆ¡ˆ æ ¿∫ ∫∏ºÆ∑˘");
#else
            DEBUGSTR("Unknown jewellery");
#endif
        }

        description += "$";
    }

    if ((verbose == 1 || is_random_artefact( item ))
        && item_ident( item, ISFLAG_KNOW_PLUSES ))
    {
        // Explicit description of ring power (useful for randarts)
        // Note that for randarts we'll print out the pluses even
        // in the case that its zero, just to avoid confusion. -- bwr
        if (item.plus != 0
            || (item.sub_type == RING_SLAYING && item.plus2 != 0)
            || is_random_artefact( item ))
        {
            switch (item.sub_type)
            {
            case RING_PROTECTION:
#ifdef JP
                description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« ACø° øµ«‚¿ª ≥¢ƒ£¥Ÿ.(";
#else
                description += "$It affects your AC (";
#endif
                append_value( description, item.plus, true );
#ifdef JP
                description += ")";
#else
                description += ").";
#endif
                break;

            case RING_EVASION:
#ifdef JP
                description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« »∏««∑¬ø° øµ«‚¿ª ≥¢ƒ£¥Ÿ.(";
#else
                description += "$It affects your evasion (";
#endif
                append_value( description, item.plus, true );
#ifdef JP
                description += ")";
#else
                description += ").";
#endif
                break;

            case RING_STRENGTH:
#ifdef JP
                description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« »˚ø° øµ«‚¿ª ≥¢ƒ£¥Ÿ.(";
#else
                description += "$It affects your strength (";
#endif
                append_value( description, item.plus, true );
#ifdef JP
                description += ")";
#else
                description += ").";
#endif
                break;

            case RING_INTELLIGENCE:
#ifdef JP
                description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« ¡ˆ¥…ø° øµ«‚¿ª ≥¢ƒ£¥Ÿ.(";
#else
                description += "$It affects your intelligence (";
#endif
                append_value( description, item.plus, true );
#ifdef JP
                description += ")";
#else
                description += ").";
#endif
                break;

            case RING_DEXTERITY:
#ifdef JP
                description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« πŒ√∏ø° øµ«‚¿ª ≥¢ƒ£¥Ÿ.(";
#else
                description += "$It affects your dexterity (";
#endif
                append_value( description, item.plus, true );
#ifdef JP
                description += ")";
#else
                description += ").";
#endif
                break;

            case RING_SLAYING:
                if (item.plus != 0 || is_random_artefact( item ))
                {
#ifdef JP
                    description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« ∏Ì¡ﬂ∑¸ø° øµ«‚¿ª ≥¢ƒ£¥Ÿ.(";
#else
                    description += "$It affects your accuracy (";
#endif
                    append_value( description, item.plus, true );
#ifdef JP
                    description += ")";
#else
                    description += ").";
#endif
                }

                if (item.plus2 != 0 || is_random_artefact( item ))
                {
#ifdef JP
                    description += "$¿Ã∞Õ¿∫ ¥ÁΩ≈¿« µ•πÃ¡ˆø° øµ«‚¿ª ≥¢ƒ£¥Ÿ.(";
#else
                    description += "$It affects your damage-dealing abilities (";
#endif
                    append_value( description, item.plus2, true );
#ifdef JP
                    description += ")";
#else
                    description += ").";
#endif
                }
                break;

            default:
                break;
            }
        }
    }

    // randart properties
    if (is_random_artefact( item ))
    {
        if (item_ident( item, ISFLAG_KNOW_PROPERTIES ))
            randart_descpr( description, item );
        else if (item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            if (item.sub_type >= AMU_RAGE)
#ifdef JP
                description += "$¿Ã ∫Œ¿˚ø°¥¬ ∏Ó∞°¡ˆ º˚∞‹¡¯ ¥…∑¬¿Ã ¿÷¿ª¡ˆ ∏∏•¥Ÿ. $";
#else
                description += "$This amulet may have hidden properties.$";
#endif
            else
#ifdef JP
                description += "$¿Ã π›¡ˆ¥¬ º˚∞‹¡¯ ¥…∑¬¿Ã ¿÷¿ª¡ˆ ∏∏•¥Ÿ. $";
#else
                description += "$This ring may have hidden properties.$";
#endif
        }
    }

    if (item_known_cursed( item ))
    {
#ifdef JP
        description += "$$¿Ã∞Õø°¥¬ ¿˙¡÷∞° ∞…∑¡¿÷¥Ÿ.";
#else
        description += "$It has a curse placed upon it.";
#endif
    }

    return (description);
}                               // end describe_jewellery()

//---------------------------------------------------------------
//
// describe_staff
//
//---------------------------------------------------------------
static std::string describe_staff( const item_def &item )
{
    std::string description;

    description.reserve(200);

    if (item_ident( item, ISFLAG_KNOW_TYPE ))
    {
        // NB: the leading space is here {dlb}
#ifdef JP
        description += "¿Ã " + std::string( item_is_staff( item ) ? "¡ˆ∆Œ¿Ã¥¬ "
                                                                    : "∏∂π˝ ∏∑¥Î¥¬ " );
#else
        description += "This " + std::string( item_is_staff( item ) ? "staff "
                                                                    : "rod " );
#endif

        switch (item.sub_type)
        {
        case STAFF_WIZARDRY:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ¿⁄¿« ∏∂π˝º˜∑√µµ∏¶ ≈´∆¯¿∏∑Œ ªÛΩ¬Ω√ƒ—, "
                "¡÷πÆ¿« ¿ß∑¬¿ª ∞≠»≠Ω√≈≤¥Ÿ. ";
#else
                "increases the magical proficiency of its wielder by "
                "a considerable degree, increasing the power of their spells. ";
#endif
            break;

        case STAFF_POWER:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ¿⁄ø°∞‘ √ﬂ∞° ∏∂∑¬¿ª ¡¶∞¯«—¥Ÿ. ";
#else
                "provides a reservoir of magical power to its wielder. ";
#endif
            break;

        case STAFF_FIRE:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿« »≠ø∞∏∂π˝¿« ¿ß∑¬¿ª ∞≠»≠«œ∏Á, "
                "∂««— »≠ø∞ø° ¥Î«— øµ«‚¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. "
                "¿Ã ¡ˆ∆Œ¿Ã¥¬ ∞¯∞›πﬁ¿∫ ªÛ¥Î∏¶ ≈¬øˆπˆ∏± ºˆ ¿÷¥Ÿ. ";
#else
                "increases the power of fire spells cast by its wielder, "
                "and protects him or her from the effects of heat and fire. "
                "It can burn those struck by it. ";
#endif
            break;

        case STAFF_COLD:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿« ≥√±‚∏∂π˝¿« ¿ß∑¬¿ª ∞≠»≠«œ∏Á, "
                "∂««— ≥√±‚ø° ¥Î«— øµ«‚¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. "
                "¿Ã ¡ˆ∆Œ¿Ã¥¬ ∞¯∞›πﬁ¿∫ ªÛ¥Î∏¶ æÛ∑¡πˆ∏± ºˆ ¿÷¥Ÿ. ";
#else
                "increases the power of ice spells cast by its wielder, "
                "and protects him or her from the effects of cold. It can "
                "freeze those struck by it. ";
#endif
            break;

        case STAFF_POISON:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿« µ∂∏∂π˝¿« ¿ß∑¬¿ª ∞≠»≠«œ∏Á "
                "∂««— µ∂¿« øµ«‚¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. "
                "¿Ã ¡ˆ∆Œ¿Ã¥¬ ∞¯∞›πﬁ¿∫ ªÛ¥Î∏¶ µ∂ø° ∞…∏Æ∞‘ «“ ºˆ ¿÷¥Ÿ. ";
#else
                "increases the power of poisoning spells cast by its "
                "wielder, and protects him or her from the effects of "
                "poison. It can poison those struck by it. ";
#endif
            break;

        case STAFF_ENERGY:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿Ã ¿ΩΩƒ¿ª º“∏«œ¡ˆ æ ∞Ì "
                "¡÷πÆ¿ª Ω√¿¸ «“ ºˆ ¿÷∞‘ «—¥Ÿ. ";
#else
                "allows its wielder to cast magical spells without "
                "hungering as a result. ";
#endif
            break;

        case STAFF_DEATH:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿« ∞≠∑…º˙ ∏∂π˝¿« ¿ß∑¬¿ª ∞≠»≠«—¥Ÿ. "
                "¿Ã ¡ˆ∆Œ¿Ã¥¬ ∞¯∞› πﬁ¿∫ ªÁ∂˜ø°∞‘ æˆ√ª≥≠ ∞Ì≈Î¿ª ¡Ÿ ºˆ ¿÷¥Ÿ. ";
#else
                "increases the power of necromantic spells cast by its "
                "wielder. It can cause great pain in those living souls "
                "its wielder strikes. ";
#endif
            break;

        case STAFF_CONJURATION:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿« ø‰º˙¿« ¿ß∑¬¿ª ∞≠»≠«—¥Ÿ. ";
#else
                "increases the power of conjurations cast by its wielder. ";
#endif
            break;

        case STAFF_ENCHANTMENT:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿« ¡÷º˙¿« ¿ß∑¬¿ª ∞≠»≠«—¥Ÿ. ";
#else
                "increases the power of enchantments cast by its wielder. ";
#endif
            break;

        case STAFF_SUMMONING:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿« º“»Øº˙ ∏∂π˝¿« ¿ß∑¬¿ª ∞≠»≠«—¥Ÿ. ";
#else
                "increases the power of summonings cast by its wielder. ";
#endif
            break;

        case STAFF_SMITING:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿Ã ø¯∞≈∏Æø°º≠ ¿˚¿ª ∞≠≈∏«“ ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ. "
                "¿Ã ¥…∑¬¿ª ªÁøÎ«œ±‚ ¿ß«ÿº≠¥¬ √÷º“ 4∑π∫ß¿Ã µ«æÓæﬂ «œ∞Ì "
                "4¡°¿« ∏∂≥™∏¶ º“∏«—¥Ÿ. ";
#else
                "allows its wielder to smite foes from afar. The wielder "
                "must be at least level four to safely use this ability, "
                "which costs 4 magic points. ";
#endif
            break;

        case STAFF_STRIKING:
#ifdef JP
            description += "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿Ã ø¯∞≈∏Æø°º≠ ¿˚¿ª ƒ• ºˆ ¿÷∞‘ «—¥Ÿ. ";
#else
            description += "allows its wielder to strike foes from afar. ";
#endif
            break;

        case STAFF_SPELL_SUMMONING:
#ifdef JP
            description += "º“»Øø° ∞¸«— ¡÷πÆ¿ª ¥„∞Ì ¿÷¥Ÿ. ";
#else
            description += "contains spells of summoning. ";
#endif
            break;

        case STAFF_WARDING:
            description +=
#ifdef JP
                "¿˚¿ª ≈ƒ°«œ±‚ ¿ß«— ¡÷πÆµÈ¿ª ¥„∞Ì ¿÷¥Ÿ. ";
#else
                "contains spells designed to repel one's enemies. ";
#endif
            break;

        case STAFF_DISCOVERY:
            description +=
#ifdef JP
                "¡÷∫Ø¿ª ∞®¡ˆ«œ¥¬ ¡÷πÆ¿ª ¥„∞Ì ¿÷¥Ÿ. ";
#else
                "contains spells which reveal various aspects of "
                "an explorer's surroundings to them. ";
#endif
            break;

        case STAFF_AIR:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿« ∞¯±‚∏∂π˝¿« ¿ß∑¬¿ª ∞≠»≠«—¥Ÿ. "
                "¿Ã ¡ˆ∆Œ¿Ã¥¬ ∞¯∞›πﬁ¿∫ ªÛ¥Îø°∞‘ ºÓ≈©∏¶ ¡Ÿ ºˆ ¿÷¥Ÿ. ";
#else
                "increases the power of air spells cast by its wielder. "
                "It can shock those struck by it. ";
#endif
            break;

        case STAFF_EARTH:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿« ¥Î¡ˆ∏∂π˝¿« ¿ß∑¬¿ª ∞≠»≠«—¥Ÿ. "
                "¿Ã ¡ˆ∆Œ¿Ã¥¬ ∞¯∞›πﬁ¿∫ ªÛ¥Î∏¶ ∫ŒºˆæÓ πˆ∏± ºˆ ¿÷¥Ÿ. ";
#else
                "increases the power of earth spells cast by its wielder. "
                "It can crush those struck by it. ";
#endif
            break;

        case STAFF_CHANNELING:
            description +=
#ifdef JP
                "µÈ∞Ì ¿÷¥¬ ªÁ∂˜¿« ¡÷∫Ø ø°≥ ¡ˆ∏¶ √§≥Œ∏µ«œø© "
                "∏∂≥™∏¶ »∏∫π«“ ºˆ ¿÷∞‘ «—¥Ÿ. ";
#else
                "allows its caster to channel ambient magical energy for "
                "his or her own purposes. ";
#endif
            break;

        default:
            description +=
#ifdef JP
                "∆ƒ±´øÕ »≤∆Û¿« ¡÷πÆ¿ª ¥„∞Ì ¿÷¥Ÿ. ";
#else
                "contains spells of mayhem and destruction. ";
#endif
            break;
        }

        if (item_is_rod( item ))
        {
            description +=
#ifdef JP
                "$¿Ã∞Õ¿∏∑Œ Ω√¿¸«œ¥¬ ¡÷πÆ¿∫ ¿ΩΩƒ¿ª º“∏«œ¡ˆ æ ∞Ì Ω«∆–«œ¡ˆµµ æ ¥¬¥Ÿ. $";
#else
                "Casting a spell from it consumes no food, and will not fail.$";
#endif
        }
        else
        {
            description +=
#ifdef JP
                "$$«««ÿ¿≤: 7 $∏Ì¡ﬂ∑¸: +6 $∞¯∞› µÙ∑π¿Ã: 120%%";
#else
                "$$Damage rating: 7 $Accuracy rating: +6 $Attack delay: 120%%";
#endif

#ifdef JP
            description += "$$¿Ã∞Õ¿∫ ¡ˆ∆Œ¿Ã∑˘∑Œ ∫–∑˘µ»¥Ÿ. ";
#else
            description += "$$It falls into the 'staves' category. ";
#endif
        }
    }
    else
    {
#ifdef JP
        description += "∏∂π˝¿Ã ∫¿«ÿ¡¯ ¡ˆ∆Œ¿Ã¥Ÿ. $";
#else
        description += "A stick imbued with magical properties.$";
#endif
    }

    return (description);
}


//---------------------------------------------------------------
//
// describe_misc_item
//
//---------------------------------------------------------------
static std::string describe_misc_item( const item_def &item )
{
    std::string description;

    description.reserve(100);

    if (item_ident( item, ISFLAG_KNOW_TYPE ))
    {
        switch (item.sub_type)
        {
        case MISC_BOTTLED_EFREET:
            description +=
#ifdef JP
                "æÓ∂≤ ∏∂π˝ªÁø° ¿««œø© √ªµø¿« ∫¥ø° ∞°µ÷¡¯ ∞≠∑¬«— ¿Ã«¡∏Æ∆Æ¥Ÿ. $"
                "∫¥¿« ∫¿¿Œ¿ª ∂‚¿∏∏È ¿Ã«¡∏Æ∆Æ¥¬ «Æ∑¡≥™º≠, ∆ƒ±´∏¶ ∞°¡Æø√ ∞Õ¿Ã¥Ÿ. $"
                "æÓ¬º∏È ¥ÁΩ≈ ¿⁄Ω≈ø°∞‘µµ! ";
#else
                "A mighty efreet, captured by some wizard and bound into "
                "a bronze flask. Breaking the flask's seal will release it "
                "to wreak havoc - possibly on you. ";
#endif
            break;
        case MISC_CRYSTAL_BALL_OF_SEEING:
            description +=
#ifdef JP
                "¡÷¿ß¿« ¡ˆ«¸¿ª ∫º ºˆ ¿÷∞‘ «œ¥¬ ∏∂π˝µµ±∏¿Ã¥Ÿ. $"
                "¿Ã∞Õ¿ª ¡¶¥Î∑Œ ªÁøÎ«œ∑¡∏È æÓ¥¿¡§µµ¿« ∏∂π˝¿˚ ±‚∑Æ¿Ã « ø‰«œ¥Ÿ. $"
                "±◊∑∏¡ˆ æ ¥Ÿ∏È øπ√¯«“ ºˆ æ¯¥¬ (æ∆∏∂µµ ¿Ø«ÿ«—) ∞·∞˙∞° πﬂª˝«“ ∞Õ¿Ã¥Ÿ. ";
#else
                "A magical device which allows one to see the layout of "
                "their surroundings. It requires a degree of magical "
                "ability to be used reliably, otherwise it can produce "
                "unpredictable and possibly harmful results. ";
#endif
            break;
        case MISC_AIR_ELEMENTAL_FAN:
#ifdef JP
            description +=
                "¥Î±‚¿« ¡§∑…¿ª º“»Ø«œ±‚ ¿ß«— ∏∂π˝µµ±∏¥Ÿ. $"
                "æ‡∞£ Ω≈∑⁄º∫¿Ã ∫Œ¡∑«œø© ¿ﬂ πﬂµøµ… ∂ß±Ó¡ˆ $"
                "æÓ¥¿¡§µµ Ω√µµ«ÿæﬂ «œ¥¬ ∞ÊøÏ∞° ¿÷¥Ÿ. $"
                "ªÁøÎø°¥¬ ¿ß«Ë¿ª ºˆπ›«œ¡ˆ∏∏, ªÁøÎ¿⁄∞° ±◊ ¡§∑… ∞Ëø≠ø° $"
                "¿Õº˜«œ¥Ÿ∏È ±◊ ¿ß«Ë¿∫ ∞®º“«—¥Ÿ. ";
#else
            description += "A magical device for summoning air "
                "elementals. It is rather unreliable, and usually requires "
                "several attempts to function correctly. Using it carries "
                "an element of risk, which is reduced if one is skilled in "
                "the appropriate elemental magic. ";
#endif
            break;
        case MISC_LAMP_OF_FIRE:
#ifdef JP
            description +=
                "∫“¿« ¡§∑…¿ª º“»Ø«œ±‚ ¿ß«— ∏∂π˝µµ±∏¥Ÿ. $"
                "æ‡∞£ Ω≈∑⁄º∫¿Ã ∫Œ¡∑«œø© ¿ﬂ πﬂµøµ… ∂ß±Ó¡ˆ $"
                "æÓ¥¿¡§µµ Ω√µµ«ÿæﬂ «œ¥¬ ∞ÊøÏ∞° ¿÷¥Ÿ. $"
                "ªÁøÎø°¥¬ ¿ß«Ë¿ª ºˆπ›«œ¡ˆ∏∏, ªÁøÎ¿⁄∞° ±◊ ¡§∑… ∞Ëø≠ø° $"
                "¿Õº˜«œ¥Ÿ∏È ±◊ ¿ß«Ë¿∫ ∞®º“«—¥Ÿ. ";
#else
            description += "A magical device for summoning fire "
                "elementals. It is rather unreliable, and usually "
                "requires several attempts to function correctly. Using "
                "it carries an element of risk, which is reduced if one "
                "is skilled in the appropriate elemental magic.";
#endif
            break;
        case MISC_STONE_OF_EARTH_ELEMENTALS:
#ifdef JP
            description +=
                "¥Î¡ˆ¿« ¡§∑…¿ª º“»Ø«œ±‚ ¿ß«— ∏∂π˝µµ±∏¥Ÿ. $"
                "æ‡∞£ Ω≈∑⁄º∫¿Ã ∫Œ¡∑«œø© ¿ﬂ πﬂµøµ… ∂ß±Ó¡ˆ $"
                "æÓ¥¿¡§µµ Ω√µµ«ÿæﬂ «œ¥¬ ∞ÊøÏ∞° ¿÷¥Ÿ. $"
                "ªÁøÎø°¥¬ ¿ß«Ë¿ª ºˆπ›«œ¡ˆ∏∏, ªÁøÎ¿⁄∞° ±◊ ¡§∑… ∞Ëø≠ø° $"
                "¿Õº˜«œ¥Ÿ∏È ±◊ ¿ß«Ë¿∫ ∞®º“«—¥Ÿ. ";
#else
            description += "A magical device for summoning earth "
                "elementals. It is rather unreliable, and usually "
                "requires several attempts to function correctly. "
                "Using it carries an element of risk, which is reduced "
                "if one is skilled in the appropriate elemental magic.";
#endif
            break;
        case MISC_LANTERN_OF_SHADOWS:
            description +=
#ifdef JP
                "¿Ã ªÁæ««— µµ±∏¥¬ æ‡∞£¿« ¥Ò∞° ¥ÎΩ≈ø° $"
                "ªÁøÎ¿⁄∏¶ µΩ¥¬ æÓµ“¿« »˚¿ª ∫“∑Øø¬¥Ÿ. ";
#else
                "An unholy device which calls on the powers of darkness "
                "to assist its user, with a small cost attached. ";
#endif
            break;
        case MISC_HORN_OF_GERYON:
            description +=
#ifdef JP
                "¡ˆø¡¿« ¿‘±∏∏¶ ¡ˆ≈∞¥¬ ∞‘∏£øÊ¿« ª‘≥™∆»¿Ã¥Ÿ. $"
                "¿¸º≥ø° ¿««œ∏È, ø©∑Ø ¡ˆø¡¿ª πÊπÆ«œ±Ê ø¯«œ¥¬ « ∏Í¿⁄µÈ¿∫ $"
                "æ»¿∏∑Œ µÈæÓ∞°±‚ ¿ß«ÿº≠ ¿Ã ª‘≥™∆»¿ª ªÁøÎ«œ¡ˆ æ ¿∏∏È æ»µ»¥Ÿ∞Ì «—¥Ÿ. ";
#else
                "The horn belonging to Geryon, guardian of the Vestibule "
                "of Hell. Legends say that a mortal who desires access "
                "into one of the Hells must use it in order to gain entry. ";
#endif
            break;
        case MISC_BOX_OF_BEASTS:
            description +=
#ifdef JP
                "æﬂª˝¿« ¡¸Ω¬µÈ¿Ã µÈæÓ∞£ ∏∂π˝¿« ªÛ¿⁄¥Ÿ. $"
                "ªÛ¿⁄¿« ∂—≤±¿ª ø≠∏È ¡¸Ω¬µÈ¿ª ¿⁄¿Ø∑”∞‘ «“ ºˆ ¿÷¿ª ∞Õ¿Ã¥Ÿ. ";
#else
                "A magical box containing many wild beasts. One may "
                "allow them to escape by opening the box's lid. ";
#endif
            break;
        case MISC_DECK_OF_WONDERS:
            description +=
#ifdef JP
                "∏≈øÏ Ω≈∫Ò«— ∏∂π˝¿« ƒ´µÂ 1ºº∆Æ¿Ã¥Ÿ. $"
                "ƒ´µÂ∏¶ ªÃ¿ª ∂ßø°¥¬, ¿œæÓ≥Ø ∞·∞˙ø° ¥Î«ÿ ∞¢ø¿∞° « ø‰«“ ∞Õ¿Ã¥Ÿ!   ";
#else
                "A deck of highly mysterious and magical cards. One may "
                "draw a random card from it, but should be prepared to "
                "suffer the possible consequences! ";
#endif
            break;
        case MISC_DECK_OF_SUMMONINGS:
            description +=
#ifdef JP
                "∏∂π˝¿« ƒ´µÂ 1ºº∆Æ¿Ã¥Ÿ. $"
                "ø©∑Ø∞°¡ˆ Ω≈±‚«— ª˝π∞µÈ¿Ã ±◊∑¡¡Æ¿÷¥Ÿ. ";
#else
                "A deck of magical cards, depicting a range of weird and "
                "wondrous creatures. ";
#endif
            break;
        case MISC_CRYSTAL_BALL_OF_ENERGY:
            description +=
#ifdef JP
                "ªÁøÎ«œ¥¬ ¿⁄¿« ∏∂≥™∏¶ »∏∫πΩ√≈∞¥¬ ∏∂π˝µµ±∏¿Ã¥Ÿ. $"
                "±◊∑Ø≥™ ∏∂≥™∏¶ ¿¸∫Œ ª°∑¡πˆ∏Æ¥¬ ¿ß«Ëµµ ºˆπ›«—¥Ÿ. $"
                "∏∂≥™∏¶ ª©æ—±‚¥¬ ¿ß«Ë¿« »Æ∑¸¿∫ ªÁøÎ¿⁄ ∏∂≥™¿« √÷¥Î∞™∞˙ «ˆ¿Á∞™¿« ∫Ò«ÿ $"
                "π›∫Ò∑ «—¥Ÿ. $"
                "¡Ô, ∏∂≥™∞° √÷¥Îƒ°ø° ∞°±ÓøÔ ∂ß ªÁøÎ«œ¥¬ ∞Õ¿Ã ∞°¿Â ¿Ø∏Æ«œ¥Ÿ. ";
#else
                "A magical device which can be used to restore one's "
                "reserves of magical energy, but the use of which carries "
                "the risk of draining all of those energies completely. "
                "This risk varies inversely with the proportion of their "
                "maximum energy which the user possesses; a user near his "
                "or her full potential will find this item most beneficial. ";
#endif
            break;
        case MISC_EMPTY_EBONY_CASKET:
#ifdef JP
            description += "∏∂∑¬¿ª ¥Ÿ Ω·πˆ∏∞ ∏∂π˝¿« ªÛ¿⁄¿Ã¥Ÿ. ";
#else
            description += "A magical box after its power is spent. ";
#endif
            break;
        case MISC_CRYSTAL_BALL_OF_FIXATION:
            description +=
#ifdef JP
                "¿ß«Ë«— π∞∞«¿Ã¥Ÿ. $"
                "¿Ã ºˆ¡§±∏¥¬ ¿Ã∞Õ¿ª µÈø©¥Ÿ∫∏¥¬ ¡÷¿« ±Ì¡ˆ æ ¿∫ ªÁøÎ¿⁄ø°∞‘ √÷∏È¿ª ∞…æÓ $"
                "¿ÂΩ√∞£ø° ∞…√ƒ π´∑¬«— ªÛ≈¬∑Œ ∏∏µÈæÓπˆ∏∞¥Ÿ. ";
#else
                "A dangerous item which hypnotises anyone so unwise as "
                "to gaze into it, leaving them helpless for a significant "
                "length of time. ";
#endif
            break;
        case MISC_DISC_OF_STORMS:
            description +=
#ifdef JP
                "¿Ã ¡ˆ±ÿ»˜ ∞≠∑¬«— æ∆¿Ã≈€¿∫, ∆ƒ±´¿˚¿Œ ¿¸∞›¿« ∆¯«≥øÏ∏¶ ª˝º∫«—¥Ÿ. $"
                "¿Ã∞Õ¿∫ ¥Î±‚¿« ¡§∑… ∏∂π˝ º˜∑√¿⁄∞° ªÁøÎ«œ∏È »ø∞˙¿˚¿Ã¥Ÿ. $"
                "±◊∑Ø≥™ ªÁøÎ¿⁄∞° ¿¸±‚∞° ≈Î«œ¡ˆ æ ¥¬ ªÛ≈¬∂Û∏È ªÁøÎ«“ ºˆ æ¯¥Ÿ.";
#else
                "This extremely powerful item can unleash a destructive "
                "storm of electricity. It is especially effective in the "
                "hands of one skilled in air elemental magic, but cannot "
                "be used by one who is not a conductor. ";
#endif
            break;
        case MISC_RUNE_OF_ZOT:
            description +=
#ifdef JP
                "¡∂∆Æ¿« πÊ¿∏∑Œ ∞• ºˆ ¿÷∞‘ «ÿ¡÷¥¬ ∫Œ¿˚¿Ã¥Ÿ. ";
#else
                "A talisman which allows entry into Zot's domain. ";
#endif
            break;
        case MISC_DECK_OF_TRICKS:
            description +=
#ifdef JP
                "¿ÁπÃ¿÷¥¬ ∆Æ∏Ø¿∏∑Œ ∞°µÊ¬˘ ∏∂π˝¿« ƒ´µÂ 1ºº∆Æ¿Ã¥Ÿ. ";
#else
                "A deck of magical cards, full of amusing tricks. ";
#endif
            break;
        case MISC_DECK_OF_POWER:
#ifdef JP
            description += "∞≠∑¬«— ∏∂π˝¿« ƒ´µÂ 1ºº∆Æ¥Ÿ. ";
#else
            description += "A deck of powerful magical cards. ";
#endif
            break;
        case MISC_PORTABLE_ALTAR_OF_NEMELEX:
            description +=
#ifdef JP
                "∫–∏Æ π◊ ¡∂∏≥¿Ã ∞£∆Ì«œµµ∑œ ∏∏µÈæÓ¡¯ ≥◊∏·∑∫Ω∫ ¡∂∫£¿« ¡¶¥‹¿Ã¥Ÿ. $"
                "πŸ¥⁄ø° æ∆π´∞Õµµ æ¯¥Ÿ∏È æÓµº≠µÁ º≥ƒ°∞° ∞°¥…«œ¥Ÿ. $"
                "±◊∏Æ∞Ì ±‚µµ∞° ≥°≥µ¿∏∏È ¥ŸΩ√ øÓπ›«“ ºˆ ¿÷¥Ÿ. ";
#else
                "An altar to Nemelex Xobeh, built for easy assembly and "
                "disassembly.  Evoke it to place it on a clear patch of floor, "
                "then pick it up again when you've finished. ";
#endif
            break;
        default:
#ifdef JP
            DEBUGSTR("æÀ∑¡¡ˆ¡ˆ æ ¿∫ ¿‚µøªÁ¥œ (2)");
#else
            DEBUGSTR("Unknown misc item (2)");
#endif
        }
    }
    else
    {
        switch (item.sub_type)
        {
        case MISC_BOTTLED_EFREET:
#ifdef JP
            description += "π´∞≈øÓ √ªµø «√∂ÛΩ∫≈©¥Ÿ. ∏∏¡ˆ∏È µ˚∂Ê«œ¥Ÿ. ";
#else
            description += "A heavy bronze flask, warm to the touch. ";
#endif
            break;
        case MISC_CRYSTAL_BALL_OF_ENERGY:
        case MISC_CRYSTAL_BALL_OF_FIXATION:
        case MISC_CRYSTAL_BALL_OF_SEEING:
#ifdef JP
            description += "≈ı∏Ì«— ºˆ¡§±∏¥Ÿ.";
#else
            description += "A sphere of clear crystal. ";
#endif
            break;
        case MISC_AIR_ELEMENTAL_FAN:
#ifdef JP
            description += "¿Ã∞Õ¿∫ ∫Œ√§¥Ÿ. ";
#else
            description += "A fan. ";
#endif
            break;
        case MISC_LAMP_OF_FIRE:
#ifdef JP
            description += "¿Ã∞Õ¿∫ ∑•«¡¥Ÿ. ";
#else
            description += "A lamp. ";
#endif
            break;
        case MISC_STONE_OF_EARTH_ELEMENTALS:
#ifdef JP
            description += "¿Ã∞Õ¿∫ µπ µ¢æÓ∏Æ¥Ÿ. ";
#else
            description += "A lump of rock. ";
#endif
            break;
        case MISC_LANTERN_OF_SHADOWS:
#ifdef JP
            description += "¿Ã∞Õ¿∫ ø¿∑°µ» ª¿∑Œ ∏∏µÈæÓ¡¯ ¿ÃªÛ«— ∑£≈œ¿Ã¥Ÿ. ";
#else
            description += "A strange lantern made out of ancient bones. ";
#endif
            break;
        case MISC_HORN_OF_GERYON:
#ifdef JP
            description += "≤œ ≈´ ¿∫¡¶ ª‘¿Ã¥Ÿ. ªÁæ««— ø°≥ ¡ˆ∏¶ πÊ√‚«œ∞Ì ¿÷¥Ÿ. ";
#else
            description += "A great silver horn, radiating unholy energies. ";
#endif
            break;
        case MISC_BOX_OF_BEASTS:
        case MISC_EMPTY_EBONY_CASKET:
#ifdef JP
            description += "¿€∞Ì ∞À¿∫ ªÛ¿⁄¥Ÿ. æ»ø° π´æ˘¿Ã µÈæ˙¿ª±Ó? ";
#else
            description += "A small black box. I wonder what's inside? ";
#endif
            break;
        case MISC_DECK_OF_WONDERS:
        case MISC_DECK_OF_TRICKS:
        case MISC_DECK_OF_POWER:
        case MISC_DECK_OF_SUMMONINGS:
#ifdef JP
            description += "ƒ´µÂ «— ºº∆Æ¥Ÿ. ";
#else
            description += "A deck of cards. ";
#endif
            break;
        case MISC_RUNE_OF_ZOT:
#ifdef JP
            description += "æÓ∂≤ ¡æ∑˘¿« ∫Œ¿˚¿Ã¥Ÿ. ";
#else
            description += "A talisman of some sort. ";
#endif
            break;
        case MISC_DISC_OF_STORMS:
#ifdef JP
            description += "»∏ªˆ¿« ø¯π›¿Ã¥Ÿ. ";
#else
            description += "A grey disc. ";
#endif
            break;
        case MISC_PORTABLE_ALTAR_OF_NEMELEX:
            description +=
#ifdef JP
                "∫–∏Æ π◊ ¡∂∏≥¿Ã ∞£∆Ì«œµµ∑œ ∏∏µÈæÓ¡¯ ≥◊∏·∑∫Ω∫ ¡∂∫£¿« ¡¶¥‹¿Ã¥Ÿ. $"
                "πŸ¥⁄ø° æ∆π´∞Õµµ æ¯¥Ÿ∏È æÓµº≠µÁ º≥ƒ°∞° ∞°¥…«œ¥Ÿ. $"
                "±◊∏Æ∞Ì ±‚µµ∞° ≥°≥µ¿∏∏È ¥ŸΩ√ øÓπ›«“ ºˆ ¿÷¥Ÿ. ";
#else
                "An altar to Nemelex Xobeh, built for easy assembly and "
                "disassembly.  Evoke it to place on a clear patch of floor, "
                "then pick it up again when you've finished. ";
#endif
            break;
        default:
#ifdef JP
            DEBUGSTR("æÀ∑¡¡ˆ¡ˆ æ ¿∫ ¿‚µøªÁ¥œ");
#else
            DEBUGSTR("Unknown misc item");
#endif
        }
    }

    description += "$";

    return (description);
}

#if MAC
#pragma mark -
#endif

// ========================================================================
//      Public Functions
// ========================================================================

bool is_dumpable_artifact( const item_def &item, char verbose)
{
    bool ret = false;

    if (is_random_artefact( item ) || is_fixed_artefact( item ))
    {
        ret = item_ident( item, ISFLAG_KNOW_PROPERTIES );
    }
    else if (item.base_type == OBJ_ARMOUR
        && (verbose == 1 && item_ident( item, ISFLAG_KNOW_TYPE )))
    {
        const int spec_ench = get_armour_ego_type( item );
        ret = (spec_ench >= SPARM_RUNNING && spec_ench <= SPARM_PRESERVATION);
    }
    else if (item.base_type == OBJ_JEWELLERY
        && (verbose == 1
            && get_ident_type(OBJ_JEWELLERY, item.sub_type) == ID_KNOWN_TYPE))
    {
        ret = true;
    }

    return (ret);
}                               // end is_dumpable_artifact()


//---------------------------------------------------------------
//
// get_item_description
//
// Note that the string will include dollar signs which should
// be interpreted as carriage returns.
//
//---------------------------------------------------------------
std::string get_item_description( const item_def &item, char verbose, bool dump )
{
    std::string description;
    description.reserve(500);

    if (!dump)
    {
        char str_pass[ ITEMNAME_SIZE ];
        item_name( item, DESC_INVENTORY_EQUIP, str_pass );
        description += std::string(str_pass);
    }

    description += "$$";

#if DEBUG_DIAGNOSTICS
    if (!dump)
    {
        snprintf( info, INFO_SIZE,
#ifdef JP
                  "base: %d; sub: %d; plus: %d; plus2: %d; special: %ld$"
                  "quant: %d; colour: %d; flags: 0x%08lx$"
                  "x: %d; y: %d; link: %d$ident_type: %d$$",
                  item.base_type, item.sub_type, item.plus, item.plus2,
                  item.special, item.quantity, item.colour, item.flags,
                  item.x, item.y, item.link,
                  get_ident_type( item.base_type, item.sub_type ) );
#else
                  "base: %d; sub: %d; plus: %d; plus2: %d; special: %ld$"
                  "quant: %d; colour: %d; flags: 0x%08lx$"
                  "x: %d; y: %d; link: %d$ident_type: %d$$",
                  item.base_type, item.sub_type, item.plus, item.plus2,
                  item.special, item.quantity, item.colour, item.flags,
                  item.x, item.y, item.link,
                  get_ident_type( item.base_type, item.sub_type ) );
#endif

        description += info;
    }
#endif

    switch (item.base_type)
    {
    case OBJ_WEAPONS:
        description += describe_weapon( item, verbose );
        break;
    case OBJ_MISSILES:
        description += describe_ammo( item );
        break;
    case OBJ_ARMOUR:
        description += describe_armour( item, verbose );
        break;
    case OBJ_WANDS:
        description += describe_stick( item );
        break;
    case OBJ_FOOD:
        description += describe_food( item );
        break;
    case OBJ_SCROLLS:
        description += describe_scroll( item );
        break;
    case OBJ_JEWELLERY:
        description += describe_jewellery( item, verbose );
        break;
    case OBJ_POTIONS:
        description += describe_potion( item );
        break;
    case OBJ_STAVES:
        description += describe_staff( item );
        break;

    case OBJ_BOOKS:
        switch (item.sub_type)
        {
        case BOOK_DESTRUCTION:
#ifdef JP
            description += "∏≈øÏ ∞≠∑¬«œ¡ˆ∏∏ øπ√¯«“ ºˆ æ¯¥¬ ∏∂π˝ µŒ∑Á∏∂∏Æ¥Ÿ. ";
#else
            description += "An extremely powerful but unpredictable book "
                "of magic. ";
#endif
            break;

        case BOOK_MANUAL:
#ifdef JP
            description +=
                "∆Ø¡§«— ±‚º˙¿ª ∞Ëº” πËøÔ ºˆ ¿÷¥¬ √•¿Ã¥Ÿ. $"
                "ªÁøÎ«œ∏È ªÁøÎ«“ ∂ß∏∂¥Ÿ ∫ŒΩ∫∑Ø¡Æ, ∞·±π¿∫ æ¯æÓ¡Æ πˆ∏∞¥Ÿ.  ";
#else
            description += "A valuable book of magic which allows one to "
                "practise a certain skill greatly. As it is used, it gradually "
                "disintegrates and will eventually fall apart. ";
#endif
            break;

        default:
#ifdef JP
            description +=
                "∏∂π˝ ¡÷πÆø° ¥Î«— √•¿Ã¥Ÿ. $"
                "∞≠∑¬«— ∏∂π˝ø°¥¬ ¿ß«Ë¿Ã ºˆπ›µ«π«∑Œ ¡÷¿«∞° « ø‰«œ¥Ÿ. ";
#else
            description += "A book of magic spells. Beware, for some of the "
                "more powerful grimoires are not to be toyed with. ";
#endif
            break;
        }
        break;

    case OBJ_ORBS:
#ifdef JP
        description +=
            "¿Ã µ—µµ æ¯¥¬ æ∆∆º∆—∆Æ∏¶ ∞°¡ˆ∞Ì ¡ˆªÛ¿∏∑Œ ø√∂Û∞°∏È $"
            "¥ÁΩ≈¿« ∏«Ë¿∫ ≥°¿Ã ≥≠¥Ÿ.";
#else
        description += "Once you have escaped to the surface with "
            "this invaluable artefact, your quest is complete. ";
#endif
        break;

    case OBJ_MISCELLANY:
        description += describe_misc_item( item );
        break;

    case OBJ_CORPSES:
        description +=
#ifdef JP
            ((item.sub_type == CORPSE_BODY) ? "Ω√√º¥Ÿ. $"
                                        : "Ω‚∞Ì ¿÷¥¬ «ÿ∞Ò¿Ã¥Ÿ. $");
#else
            ((item.sub_type == CORPSE_BODY) ? "A corpse. "
                                        : "A decaying skeleton. ");
#endif
        break;

    default:
#ifdef JP
        DEBUGSTR("Bad item class");//∞À≈‰ - ±∏¿Ã - ∏ﬁ¿Ãµ•¿Ã∂Û¥œ...
        description += "¿Ã æ∆¿Ã≈€¿∫ ¡∏¿Á«“ ºˆ æ¯¥Ÿ. ±∏¡∂πŸ∂˜! ±∏¡∂πŸ∂˜! $";
#else
        DEBUGSTR("Bad item class");
        description += "This item should not exist. Mayday! Mayday! ";
#endif
    }

    if (verbose == 1)
    {
#ifdef JP
        description += "$¡ﬂ∑Æ ";
#else
        description += "$It weighs around ";
#endif

        const int mass = mass_item( item );

        char item_mass[16];
        itoa( mass / 10, item_mass, 10 );

        for (int i = 0; i < 14; i++)
        {
            if (item_mass[i] == '\0')
            {
                item_mass[i] = '.';
                item_mass[i+1] = (mass % 10) + '0';
                item_mass[i+2] = '\0';
                break;
            }
        }

        description += item_mass;
#ifdef JP
        description += " aum. ";        // arbitrary unit of mass
#else
        description += " aum. ";        // arbitrary unit of mass
#endif
    }

    return (description);
}                               // end get_item_description()


//---------------------------------------------------------------
//
// describe_item
//
// Describes all items in the game.
//
//---------------------------------------------------------------
void describe_item( const item_def &item )
{
#ifdef DOS_TERM
    char buffer[3400];

    gettext(25, 1, 80, 25, buffer);

    window(25, 1, 80, 25);
#endif

    clrscr();

    std::string description = get_item_description( item, 1 );

    print_description(description);

    set_keyin_mode(KEYIN_MODE_MORE);
    if (getch() == 0)
        getch();
    set_keyin_mode(KEYIN_MODE_NONE);

#ifdef DOS_TERM
    puttext(25, 1, 80, 25, buffer);
    window(1, 1, 80, 25);
#endif
}                               // end describe_item()


//---------------------------------------------------------------
//
// describe_spell
//
// Describes (most) every spell in the game.
//
//---------------------------------------------------------------
void describe_spell(int spelled)
{
    std::string description;

    description.reserve(500);

#ifdef DOS_TERM
    char buffer[3400];

    gettext(25, 1, 80, 25, buffer);
    window(25, 1, 80, 25);
#endif

    clrscr();
    description += spell_title( spelled );
#ifdef JP
    description += "$$¿Ã ¡÷πÆ¿∫ ";   // NB: the leading space is here {dlb}
#else
    description += "$$This spell ";   // NB: the leading space is here {dlb}
#endif

    switch (spelled)
    {
    case SPELL_IDENTIFY:
#ifdef JP
        description += "Ω√¿¸«— ªÁ∂˜ø°∞‘ ∏∂π˝ π∞∞«¿« ∞®¡§¿ª ∞°¥…ƒ…«—¥Ÿ. ";
#else
        description += "allows the caster to determine the properties of "
            "an otherwise inscrutable magic item. ";
#endif
        break;

    case SPELL_TELEPORT_SELF:
#ifdef JP
        description += "Ω√¿¸«— ªÁ∂˜¿ª ∑£¥˝«— ¡ˆ¡°¿∏∑Œ º¯∞£¿Ãµø Ω√≈≤¥Ÿ. ";
#else
        description += "teleports the caster to a random location. ";
#endif
        break;

    case SPELL_CAUSE_FEAR:
#ifdef JP
        description += "Ω√¿¸«— ªÁ∂˜ ¡÷∫Øø° ∞¯∆˜∏¶ ¿œ¿∏≈≤¥Ÿ. ";
#else
        description += "causes fear in those near to the caster. ";
#endif
        break;

    case SPELL_CREATE_NOISE:
#ifdef JP
        description += "Ω√≤Ù∑ØøÓ º“¿Ω¿ª ≥™∞‘ «—¥Ÿ. ";
#else
        description += "causes a loud noise to be heard. ";
#endif
        break;

    case SPELL_REMOVE_CURSE:
#ifdef JP
        description += "Ω√¿¸«— ªÁ∂˜¿Ã ªÁøÎ¡ﬂ¿Œ æ∆¿Ã≈€¿« ¿˙¡÷∏¶ ¡¶∞≈«—¥Ÿ. ";
#else
        description += "removes curses from any items which are "
            "being used by the caster. ";
#endif
        break;

    case SPELL_MAGIC_DART:
#ifdef JP
        description += "∏∂π˝¿« ø°≥ ¡ˆ∑Œ ¿Ã∑ÁæÓ¡¯ ¿€¿∫ »≠ªÏ¿ª πﬂªÁ«—¥Ÿ. ";
#else
        description += "hurls a small bolt of magical energy. ";
#endif
        break;

    case SPELL_FIREBALL:
#ifdef JP
        description += "∆¯πﬂ«œ¥¬ »≠ø∞¿« µ¢æÓ∏Æ∏¶ ≥Ø∏∞¥Ÿ. $"
                       "∏∏æ‡ ¿Ã ¡÷πÆ¿ª πËøÏ¥¬ ¿⁄∞° ¿ÃπÃ ¡ˆø¨µ» »≠ø∞±∏∏¶ æÀ∞Ì ¿÷¥Ÿ∏È $"
                       "¿Ã ¡÷πÆ¿∫ ¡÷πÆ ∑π∫ß¿ª º“∫Ò«œ¡ˆ æ ∞Ì ¿Õ»˙ ºˆ ¿÷¥Ÿ.";
#else
        description += "hurls an exploding bolt of fire.  This spell "
            "does not cost additional spell levels if the learner already "
            "knows Delayed Fireball. ";
#endif
        break;

    case SPELL_DELAYED_FIREBALL:
#ifdef JP
        description = "$$Ω√¿¸ø° º∫∞¯«œ∏È, Ω√¿¸¿⁄¥¬ ¿⁄Ω≈¿Ã ø¯«œ¥¬ ∂ßø° »≠ø∞±∏∏¶ $"
                      "«ÆæÓ≥ı¿ª ºˆ ¿÷¥Ÿ. $"
                      "¿Ã ¡÷πÆ¿ª æÀ∞Ì ¿÷¥¬ ∞ÊøÏ Ω√¿¸¿⁄¥¬, $"
                      "»≠ø∞±∏ ¡÷πÆ¿ª ¡÷πÆ∑π∫ß º“∫Ò æ¯¿Ã ¿Õ»˙ ºˆ ¿÷¥Ÿ. ";
#else
        description = "$$Successfully casting this spell gives the caster "
            "the ability to instantaneously release a fireball at a later "
            "time.  Knowing this spell allows the learner to memorise "
            "Fireball for no additional spell levels. ";
#endif
        break;

    case SPELL_BOLT_OF_MAGMA:
#ifdef JP
        description += "≤˙æÓø¿∏£¥¬ øÎæœ¿« »≠ªÏ¿ª πﬂªÁ«—¥Ÿ. ";
#else
        description += "hurls a sizzling bolt of molten rock. ";
#endif
        break;

// spells 7 through 12 ??? {dlb}

    case SPELL_CONJURE_FLAME:
#ifdef JP
        description += "∏Õ∑ƒ»˜ ∫“≈∏¥¬ »≠ø∞¿« ±‚µ’¿ª ª˝º∫«—¥Ÿ. ";
#else
        description += "creates a column of roaring flame. ";
#endif
        break;

    case SPELL_DIG:
#ifdef JP
        description += "∞≠»≠ µ«æÓ ¿÷¡ˆ æ ¿∫ ∫Æø° ±∏∏€¿ª ∂’¥¬¥Ÿ. ";
#else
        description += "digs a tunnel through unworked rock. ";
#endif
        break;

    case SPELL_BOLT_OF_FIRE:
#ifdef JP
        description += "∞≠∑¬«— »≠ø∞¿« »≠ªÏ¿ª πﬂªÁ«—¥Ÿ.";
#else
        description += "hurls a great bolt of flames. ";
#endif
        break;

    case SPELL_BOLT_OF_COLD:
#ifdef JP
        description += "∞≠∑¬«— æÛ¿Ω∞˙ ≥√±‚¿« »≠ªÏ¿ª πﬂªÁ«—¥Ÿ. ";
#else
        description += "hurls a great bolt of ice and frost. ";
#endif
        break;

    case SPELL_LIGHTNING_BOLT:
#ifdef JP
        description += "∞≠∑¬«— π¯∞≥¿« »≠ªÏ¿ª πﬂªÁ«—¥Ÿ. $"
                       "∫ÒΩ¡«— »≠ø∞∞˙ æÛ¿Ω ¡÷πÆ∞˙ ∫Ò±≥«œ∏È µ•πÃ¡ˆ¥¬ ¿˚¡ˆ∏∏ $"
                       "«—¡Ÿ∑Œ º± ∏ÛΩ∫≈Õ ¿¸√º∏¶ ∞¯∞›«“ ºˆ ¿÷¥Ÿ. ";
#else
        description += "hurls a mighty bolt of lightning. "
            "Although this spell inflicts less damage than "
            "similar fire and ice spells, it can at once "
            "rip through whole rows of creatures. ";
#endif
        break;

// spells 18 and 19 ??? {dlb}

    case SPELL_POLYMORPH_OTHER:
#ifdef JP
        description += "¥ÎªÛ¿ª ∑£¥˝«œ∞‘ ¥Ÿ∏• ∏ÛΩ∫≈Õ∑Œ ∫Ø»≠Ω√≈≤¥Ÿ. ";
#else
        description += "randomly alters the form of another creature. ";
#endif
        break;

    case SPELL_SLOW:
#ifdef JP
        description += "¥ÎªÛ¿« µø¿€¿ª ¥¿∏Æ∞‘ «—¥Ÿ. ";
#else
        description += "slows the actions of a creature. ";
#endif
        break;

    case SPELL_HASTE:
#ifdef JP
        description += "¥ÎªÛ¿« øÚ¡˜¿”¿ª ∞°º”Ω√≈≤¥Ÿ. ";
#else
        description += "speeds the actions of a creature. ";
#endif
        break;

    case SPELL_PARALYZE:
#ifdef JP
        description += "¥ÎªÛ¿ª øÚ¡˜¿Ã¡ˆ ∏¯«œ∞‘ «—¥Ÿ. ";
#else
        description += "prevents a creature from moving. ";
#endif
        break;

    case SPELL_CONFUSING_TOUCH:
#ifdef JP
        description += "Ω√¿¸«— ªÁ∂˜¿« º’ø° ∏∂π˝ ø°≥ ¡ˆ∏¶ ∫Œø©«—¥Ÿ. $"
                       "¿Ã ø°≥ ¡ˆ¥¬ ∏ÛΩ∫≈Õ∞° ∏«º’ø° ¥Í¿∏∏È ∆€¡Æ≥™∞° $"
                       "∏ÛΩ∫≈Õø°∞‘ »•∂ı ªÛ≈¬∏¶ ∞°¡Æø¬¥Ÿ. ";
#else
        description += "enchants the casters hands with magical energy. "
            "This energy is released when the caster touches "
            "a monster with their bare hands, and may induce "
            "a state of confusing in the monster. ";
#endif
        break;

    case SPELL_CONFUSE:
#ifdef JP
        description += "¥ÎªÛ¿« ¡§Ω≈¿ª »•∂ıΩ∫∑¥∞‘ «—¥Ÿ. ";
#else
        description += "induces a state of bewilderment and confusion "
            "in a creature's mind. ";
#endif
        break;

    case SPELL_SURE_BLADE:
#ifdef JP
        description += "Ω√¿¸¿⁄øÕ µÈ∞Ì ¿÷¥¬ ºÙ ∫Ì∑π¿ÃµÂ∏¶ ∏∂π˝¿˚¿∏∑Œ ∞·«’Ω√ƒ— "
                       "¥ı »÷µŒ∏£±‚ Ω±∞‘ ∏∏µÈæÓ¡ÿ¥Ÿ. ";
#else
        description += "forms a mystical bond between the caster and "
            "a wielded short blade, making the blade much " "easier to use. ";
#endif
        break;

    case SPELL_INVISIBILITY:
#ifdef JP
        description += "¥ÎªÛ¿ª ¥Ÿ∏• ¿⁄¿« Ω√æﬂø°º≠ ∫∏¿Ã¡ˆ æ ∞‘ «—¥Ÿ. ";
#else
        description += "hides a creature from the sight of others. ";
#endif
        break;

    case SPELL_THROW_FLAME:
#ifdef JP
        description += "¿€¿∫ ∫“±Ê¿« »≠ªÏ¿ª ¥¯¡¯¥Ÿ. ";
#else
        description += "throws a small bolt of flame. ";
#endif
        break;

    case SPELL_THROW_FROST:
#ifdef JP
        description += "¿€¿∫ æÛ¿Ω¿« »≠ªÏ¿ª ¥¯¡¯¥Ÿ. ";
#else
        description += "throws a small bolt of frost. ";
#endif
        break;

    case SPELL_CONTROLLED_BLINK:
        description +=
#ifdef JP
            "¡§»Æ«œ∞‘ ¡¶æÓ∞°¥…«— ¥‹∞≈∏Æ º¯∞£¿Ãµø¿ª ¿œ¿∏≈≤¥Ÿ. $"
            "¡¶æÓ∞°¥…«— º¯∞£¿Ãµø¿∫ Ω√¿¸¿⁄ø°∞‘ $"
            "∏∂π˝¿˚¿Œ ø¿ø∞¿ª ∞°¡Æø√ ºˆ ¿÷¿∏¥œ ¡÷¿««ÿæﬂ «—¥Ÿ. ";
#else
            "allows short-range translocation, with precise control. "
            "Be wary that controlled teleports will cause the subject to "
            "become contaminated with magical energy. ";
#endif
        break;

    case SPELL_FREEZING_CLOUD:
#ifdef JP
        description += "ƒ°∏Ì¿˚¿Œ ≥√±‚¿« ±∏∏ß¿ª ª˝º∫«—¥Ÿ. ";
#else
        description += "conjures up a large cloud of lethally cold vapour. ";
#endif
        break;

    case SPELL_MEPHITIC_CLOUD:
        description +=
#ifdef JP
                        "±§π¸¿ß«œ¡ˆ∏∏ ¡ˆº”Ω√∞£¿Ã ¬™¿∫ µ∂±‚¿« ±∏∏ß¿ª ª˝º∫«—¥Ÿ.  ";
#else
            "conjures up a large but short-lived cloud of vile fumes. ";
#endif
        break;

    case SPELL_RING_OF_FLAMES:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶, µ˚∂Û¥Ÿ¥œ∏Á ¿Ãµø«œ¥¬ ∫“±Ê∑Œ µ—∑ØΩŒ $"
                       "¥Ÿ∏• ∏∂π˝ªÁø° ¿««— ∫“±Ê∞˙ »≠ø∞¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ Ω√¿¸¿⁄∏¶ ∫“±Ê¿« »˚∞˙ µø»≠Ω√ƒ—, »≠ø∞ ∏∂π˝¿ª ∞≠»≠«œ∞Ì $"
                       "∫“∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ.$"
                       "±◊∑∏¡ˆ∏∏, ø™Ω√ æÛ¿Ω¿« »˚ø° «««ÿ∏¶ ¿‘±‚ Ω±∞‘ ∏∏µÁ¥Ÿ.";
                       // well, if it survives the fire wall it's a risk -- bwr
#else
        description += "surrounds the caster with a mobile ring of searing "
            "flame, and keeps other fire clouds away from the caster.  "
            "This spell attunes the caster to the forces of fire, "
            "increasing their fire magic and giving protection from fire.  "
            "However, it also makes them much more susceptible to the forces "
            "of ice. "; // well, if it survives the fire wall it's a risk -- bwr
#endif
        break;

    case SPELL_RESTORE_STRENGTH:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« π∞∏Æ¿˚¿Œ »˚¿ª »∏∫π«—¥Ÿ. ";
#else
        description += "restores the physical strength of the caster. ";
#endif
        break;

    case SPELL_RESTORE_INTELLIGENCE:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« ¡ˆ¥…¿ª »∏∫π«—¥Ÿ. ";
#else
        description += "restores the intelligence of the caster. ";
#endif
        break;

    case SPELL_RESTORE_DEXTERITY:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« πŒ√∏¿ª »∏∫π«—¥Ÿ. ";
#else
        description += "restores the dexterity of the caster. ";
#endif
        break;

    case SPELL_VENOM_BOLT:
#ifdef JP
        description += "µ∂¿∏∑Œ µ» »≠ªÏ¿ª ¥¯¡¯¥Ÿ. ";
#else
        description += "throws a bolt of poison. ";
#endif
        break;

    case SPELL_POISON_ARROW:
        description +=
#ifdef JP
            "∏≈øÏ µ∂º∫¿Ã ∞≠«— ∏∂π˝¿« »≠ªÏ¿ª πﬂªÁ«—¥Ÿ. $"
            "∫Òª˝∏Ì√º¥¬ ¿Ã »≠ªÏø° ¥Î«œø© ∏Èø™¿Ã¥Ÿ. ";
#else
            "hurls a magical arrow of the most vile and noxious toxin.  "
            "No living thing is completely immune to it's effects. ";
#endif
        break;

    case SPELL_OLGREBS_TOXIC_RADIANCE:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄¿« ¡÷∫Øø° µ∂º∫¿Ã ¿÷¥¬ ≥Ïªˆ¿« ∫˚¿ª ∞®Ω—¥Ÿ. ";
#else
            "bathes the caster's surroundings in poisonous green light. ";
#endif
        break;

    case SPELL_TELEPORT_OTHER:
#ifdef JP
        description += "¥Ÿ∏• ∏ÛΩ∫≈Õ∏¶ ∑£¥˝«œ∞‘ ∞¯∞£¿ÃµøΩ√≈≤¥Ÿ. ";
#else
        description += "randomly translocates another creature. ";
#endif
        break;

    case SPELL_LESSER_HEALING:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄¿« ∏ˆ¿ª º“∑Æ ƒ°∑·«—¥Ÿ. ";
#else
            "heals a small amount of damage to the caster's body. ";
#endif
        break;

    case SPELL_GREATER_HEALING:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄¿« ∏ˆ¿ª ¥Î∑Æ »∏∫πΩ√≈≤¥Ÿ. ";
#else
            "heals a large amount of damage to the caster's body. ";
#endif
        break;

    case SPELL_CURE_POISON_I:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« √º≥ªø° ¿÷¥¬ µ∂¿ª ¡¶∞≈«—¥Ÿ. ";
#else
        description += "removes poison from the caster's system. ";
#endif
        break;

    case SPELL_PURIFICATION:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« Ω≈√º∏¶ ¡§»≠«œø© µ∂, ¡˙∫¥, ¿Ø«ÿ«— ∏∂π˝¿ª ¡¶∞≈«—¥Ÿ.  ";
#else
        description += "purifies the caster's body, removing "
            "poison, disease, and certain malign enchantments. ";
#endif
        break;

    case SPELL_DEATHS_DOOR:
#ifdef JP
        description += "¡ˆ±ÿ»˜ ∞≠∑¬«œ¡ˆ∏∏ ªÛ¥Á«— ¿ß«Ë¿ª ºˆπ›«œ¥¬ ¡÷πÆ¿Ã¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ ¬™¿∫ Ω√∞£µøæ» Ω√¿¸¿⁄∏¶ ∞≈¿« π´¿˚¿∏∑Œ ∏∏µÁ¥Ÿ. $"
                       "±◊∑Ø≥™ Ω√¿¸¿⁄∏¶ æ∆¡÷ ¿ß«Ë«“ ¡§µµ∑Œ ¡◊¿Ωø° ±Ÿ¡¢«œ∞‘ «—¥Ÿ. $"
                       "(æÓ¥¿ ¡§µµ∑Œ ¿ß«Ë«—¡ˆ¥¬ ∞≠∑…º˙ ∏∂π˝ ªÁøÎ ¥…∑¬ø° ¥ﬁ∑¡¿÷¥Ÿ.) $"
                       "¿Ã ¡÷πÆ¿∫ æ¡¶∂Ûµµ ƒ°¿Ø¡÷πÆ¿ª ¿⁄Ω≈ø°∞‘ ªÁøÎ«œ¥¬ ∞Õ¿∏∑Œ $"
                       "√Îº“«“ ºˆ ¿÷¥Ÿ. $"
                       "¡÷πÆ¿« »ø∞˙∞° ªÁ∂Û¡ˆ±‚ ¡˜¿¸ø° Ω√¿¸¿⁄¥¬ 1»∏ ∞Ê∞Ì∏¶ πﬁ¥¬¥Ÿ. $"
                       "æµ•µÂ¥¬ ¿Ã ¡÷πÆ¿ª ªÁøÎ«“ ºˆ æ¯¥Ÿ. ";
#else
        description += "is extremely powerful, but carries a degree of risk. "
            "It renders living casters nigh invulnerable to harm "
            "for a brief period, but can bring them dangerously "
            "close to death (how close depends on one's necromantic "
            "abilities). The spell can be cancelled at any time by "
            "any healing effect, and the caster will receive one "
            "warning shortly before the spell expires. "
            "Undead cannot use this spell. ";
#endif
        break;

    case SPELL_SELECTIVE_AMNESIA:
#ifdef JP
        description += "¿Ã ¡÷πÆ¿∫ ±‚æÔ¿∏∑Œ∫Œ≈Õ ¡÷πÆ¿ª º±≈√¿˚¿∏∑Œ º“∞≈«ÿ $"
                       "±◊ ¡÷πÆ¿ª ø‹øÏ¥¬µ• ΩË¥¯ ∏∂π˝ ±‚º˙ ¡°ºˆ∏¶ µ«√£¿ª ºˆ ¿÷∞‘ «—¥Ÿ. $"
                       "∏∂π˝ªÁ¥¬ ¡§Ω≈¿Ã ¡÷πÆ¿∏∑Œ ∞°µÊ ¬˜ ¿÷¿ª ∂ßø°µµ$"
                       "≥≤¿∫ ¡÷πÆ ∑π∫ß∏∏ √Ê∫–«œ¥Ÿ∏È ¿Ã ¡÷πÆ¿ª ±‚æÔ«“ ºˆ ¿÷¥Ÿ.$"
                       "(¡Ô, ¿ÃπÃ «—∞Ë±Ó¡ˆ πËø¸¥ı∂Ûµµ ¿Ã ¡÷πÆ∏∏¿∫ ¥ı πËøÔ ºˆ ¿÷¥Ÿ.) ";
#else
        description += "allows the caster to selectively erase one spell "
            "from memory to recapture the magical energy bound "
            "up with it. Casters will be able to memorise this "
            "spell should even their minds be otherwise full of "
            "magic (i.e., already possessing the maximum number "
            "of spells). ";
#endif
        break;

    case SPELL_MASS_CONFUSION:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ ∫ª ¥ÎªÛø°∞‘ »•∂ı¿ª ∞«¥Ÿ. ";
#else
        description += "causes confusion in all who gaze upon the caster. ";
#endif
        break;

    case SPELL_STRIKING:
#ifdef JP
        description += "∏∂∑¬¿« ¿€¿∫ »≠ªÏ¿ª πﬂªÁ«—¥Ÿ. ";
#else
        description += "hurls a small bolt of force. ";
#endif
        break;

    case SPELL_SMITING:
#ifdef JP
        description += "Ω√¿¸¿⁄∞° º±≈√«— «— ¥ÎªÛ¿ª »ƒ∑¡ƒ£¥Ÿ. ";
#else
        description += "smites one creature of the caster's choice. ";
#endif
        break;

    case SPELL_REPEL_UNDEAD:
#ifdef JP
        description += "∫Œ¡§«— ¡∏¿Á∏¶ ≈ƒ°«œ±‚ ¿ß«— º∫Ω∫∑ØøÓ »˚¿ª ∫“∑Ø≥Ω¥Ÿ. ";
#else
        description += "calls on a divine power to repel the unholy. ";
#endif
        break;

    case SPELL_HOLY_WORD:
#ifdef JP
        description += "»˚¿Ã ¥„±‰ ææÓ∏¶ ø‹ƒß∞˙ µøΩ√ø° $"
                       "ªÁæ««— ¿˚¿ª ≈ƒ°«“ ºˆ ¿÷¥Ÿ. ";
#else
        description += "involves the intonation of a word of power "
            "which repels and can destroy unholy creatures. ";
#endif
        break;

    case SPELL_DETECT_CURSE:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« º“¡ˆ«∞ø° ¿˙¡÷∞° ∞…∑¡ ¿÷¥¬¡ˆ æÀ∑¡¡ÿ¥Ÿ. ";
#else
        description += "alerts the caster to the presence of curses "
            "on his or her possessions. ";
#endif
        break;

    case SPELL_SUMMON_SMALL_MAMMAL:
#ifdef JP
        description += "Ω√¿¸¿⁄ø°∞‘ µµøÚ¿Ã µ«¥¬ ¿€¿∫ ª˝π∞ ∏Ó ∏∂∏Æ∏¶ º“»Ø«—¥Ÿ. ";
#else
        description += "summons one or more "
            "small creatures to the caster's aid. ";
#endif
        break;

    case SPELL_ABJURATION_I:
#ifdef JP
        description += "º“»Øµ» ¿˚¥Î¿˚¿Œ ∏ÛΩ∫≈Õ∏¶ ø¯∑° ¿÷¥¯ ¿Âº“∑Œ µπ∑¡∫∏≥Ω¥Ÿ. $"
                       "»§¿∫ ¿˚æÓµµ º“»Ø¿« ¡ˆº”Ω√∞£¿ª ¬™∞‘ «—¥Ÿ. ";
#else
        description += "attempts to send hostile summoned creatures to "
            "the place from whence they came, or at least "
            "shorten their stay in the caster's locality. ";
#endif
        break;

    case SPELL_SUMMON_SCORPIONS:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ µµøÔ ∞≈¥Î«— ¿¸∞•¿ª ∏Ó ∏∂∏Æ º“»Ø«—¥Ÿ. ";
#else
        description += "summons one or more "
            "giant scorpions to the caster's assistance. ";
#endif
        break;

    case SPELL_LEVITATION:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ ∞¯¡ﬂø° ∂∞ø¿∏£∞‘ «—¥Ÿ. ";
#else
        description += "allows the caster to float in the air. ";
#endif
        break;

    case SPELL_BOLT_OF_DRAINING:
#ifdef JP
        description += "∫Œ¡§¿˚¿Œ ø°≥ ¡ˆ¿« ƒ°∏Ì¿˚¿Œ »≠ªÏ¿ª πﬂªÁ«—¥Ÿ. $"
                       "∏Ì¡ﬂ«œ∏È ªÏæ∆¿÷¥¬ ¥ÎªÛ¿« ª˝∏Ì∑¬¿ª »Ìºˆ«—¥Ÿ. ";
#else
        description += "hurls a deadly bolt of negative energy, "
            "which drains the life from any living creature " "it strikes. ";
#endif
        break;

    case SPELL_LEHUDIBS_CRYSTAL_SPEAR:
#ifdef JP
        description += "≥Øƒ´∑”∞Ì ƒ°∏Ì¿˚¿Œ ºˆ¡§ »≠ªÏ¿ª πﬂªÁ«—¥Ÿ. ";
#else
        description += "hurls a lethally sharp bolt of crystal. ";
#endif
        break;

    case SPELL_BOLT_OF_INACCURACY:
#ifdef JP
        description += "æˆ√ª≥≠ ø°≥ ¡ˆ∑Œ ¿Ã∑ÁæÓ¡¯ »≠ªÏ¿ª Ω«√º»≠Ω√ƒ—  $"
                       "∏Ì¡ﬂ«— ¥ÎªÛø°∞‘ æÓ∏∂æÓ∏∂«— «««ÿ∏¶ ¿‘»˘¥Ÿ.  $"
                       "∫“«‡»˜µµ ¿Ã ¡÷πÆ¿∫ ¡∂¡ÿ¿Ã ∏≈øÏ æÓ∑∆∞Ì $"
                       "±ÿ»˜ µÂπ∞∞‘ ∏Ì¡ﬂ«—¥Ÿ. æ»≈∏±ı∞‘µµ. ";
#else
        description += "inflicts enormous damage upon any creature struck "
            "by the bolt of incandescent energy conjured into "
            "existence. Unfortunately, it is very difficult to "
            "aim and very rarely hits anything. Pity, that. ";
#endif
        break;

    case SPELL_POISONOUS_CLOUD:
#ifdef JP
        description += "ƒ°∏Ì¿˚¿Œ ∞°Ω∫∑Œ ¿Ã∑ÁæÓ¡¯ ±∏∏ß¿ª πﬂª˝Ω√≈≤¥Ÿ. ";
#else
        description += "conjures forth a great cloud of lethal gasses. ";
#endif
        break;

    case SPELL_FIRE_STORM:
#ifdef JP
        description += "∏Õ∑ƒ»˜ ≈∏ø¿∏£¥¬ ∞≠∑¬«— ∫“±Ê¿« ∆¯«≥øÏ∏¶ ∏∏µÈæÓ≥Ω¥Ÿ. ";
#else
        description += "creates a mighty storm of roaring flame. ";
#endif
        break;

    case SPELL_DETECT_TRAPS:
#ifdef JP
        description += "Ω√¿¸¿⁄ ∫Œ±Ÿø° ¿÷¥¬ «‘¡§¿ª √£æ∆≥Ω¥Ÿ. ";
#else
        description += "reveals traps in the caster's vicinity. ";
#endif
        break;

    case SPELL_BLINK:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ ¥‹∞≈∏Æ º¯∞£¿ÃµøΩ√≈≤¥Ÿ. ";
#else
        description += "randomly translocates the caster a short distance. ";
#endif
        break;

    case SPELL_ISKENDERUNS_MYSTIC_BLAST:
#ifdef JP
        description += "∆ƒ±´¿˚¿Œ ø°≥ ¡ˆ∑Œ ¿Ã∑ÁæÓ¡¯ ±∏√º∏¶ ¥¯¡¯¥Ÿ. ";
#else
        description += "throws a crackling sphere of destructive energy. ";
#endif
        break;

    case SPELL_SWARM:
#ifdef JP
        description += "¿Ø«ÿ«— ∞Ô√Ê¿« π´∏Æ∏¶ º“»Ø«—¥Ÿ. ";
#else
        description += "summons forth a pestilential swarm. ";
#endif
        break;

    case SPELL_SUMMON_HORRIBLE_THINGS:
#ifdef JP
        description += "æÓ∫ÒΩ∫∑Œ¿« πÆ¿ª ø≠æÓ ±◊ ∞˜¿∏∑Œ∫Œ≈Õ "
                       "«œ≥™ »§¿∫ ø©∑µ¿« ≤˚¬Ô«— ¡∏¿Á∏¶ º“»Ø«—¥Ÿ. "
                       "¿Ã º“»Øø° ¿¿«œ¥¬ ¡∏¿Á¥¬ ±◊ ¥Ò∞°∑Œ "
                       "Ω√¿¸¿⁄¿« ¡ˆ¥…¿« ¿œ∫Œ∏¶ ø‰±∏«—¥Ÿ. ";
#else
        description += "opens a gate to the Abyss and calls through "
            "one or more hideous abominations from that dreadful place."
            "  The powers who answer this invocation require of casters "
            "a portion of their intellect in exchange for this service.";
#endif
        break;

    case SPELL_ENSLAVEMENT:
#ifdef JP
        description += "¿˚¥Î¿˚¿Œ ∏ÛΩ∫≈Õ∏¶ ¿·Ω√µøæ»∏∏ æ∆±∫¿∏∑Œ ∏∏µÁ¥Ÿ. ";
#else
        description += "causes an otherwise hostile creature "
            "to fight on your side for a while. ";
#endif
        break;

    case SPELL_MAGIC_MAPPING:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« ¡÷∫Ø ¡ˆ«¸¿ª π‡»˘¥Ÿ. ";
#else
        description += "reveals details about the caster's surroundings. ";
#endif
        break;

    case SPELL_HEAL_OTHER:
#ifdef JP
        description += "∂≥æÓ¡¯ ¿Âº“ø° ¿÷¥¬ ∏ÛΩ∫≈Õ∏¶ ƒ°∑·«—¥Ÿ. ";
#else
        description += "heals another creature from a distance. ";
#endif
        break;

    case SPELL_ANIMATE_DEAD:
#ifdef JP
        description += "Ω√√º∏¶ º“ª˝Ω√ƒ— Ω√¿¸¿⁄¿« ∫Œ«œ∑Œ ∏∏µÁ¥Ÿ. $"
                       "Ω√¿¸¿⁄¿« ¡÷∫Øø° ¿÷¥¬ ∏µÁ Ω√√º∞° ¥ÎªÛ¿Ã µ»¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∏∑Œ ∞≠∑¬«— Ω√¿¸¿⁄¥¬ øµ»•æ¯¥¬ æµ•µÂ¿« $"
                       "±∫¥Î∏¶ ∏∏µÈ ºˆ ¿÷¥Ÿ. ";
#else
        description += "causes the dead to rise up and serve the caster; "
            "every corpse within a certain distance of the caster "
            "is affected. By means of this spell, powerful casters "
            "could press into service an army of the mindless undead. ";
#endif
        break;

    case SPELL_PAIN:
#ifdef JP
        description += "«— ª˝∏Ì√ºø°∞‘ ±ÿΩ…«— ∞Ì≈Î¿ª ¡÷¥¬ «««ÿ∏¶ πﬂª˝Ω√≈≤¥Ÿ.  $";
#else
        description += "inflicts an extremely painful injury "
            "upon one living creature. ";
#endif
        break;

    case SPELL_EXTENSION:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄ø°∞‘ øµ«‚¿ª ¡÷¥¬ ¿Ø¿Õ«— ∏∂π˝¿« $"
            "¡ˆº”Ω√∞£¿ª ø¨¿ÂΩ√≈≤¥Ÿ. ";
#else
            "extends the duration of most beneficial enchantments "
            "affecting the caster. ";
#endif
        break;

    case SPELL_CONTROL_UNDEAD:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄¿« ¡÷∫Øø° ¿÷¥¬ æµ•µÂ∏¶ ¡∂¡æ«—¥Ÿ. ";
#else
            "attempts to enslave any undead in the vicinity of the caster. ";
#endif
        break;

    case SPELL_ANIMATE_SKELETON:
#ifdef JP
        description += "∏ÛΩ∫≈Õ¿« ª¿∏¶ ª˝∏Ì¿Ã æ¯¥¬ ¡∏¿Á∑Œ ¿œ¿∏≈≤¥Ÿ. ";
#else
        description += "raises an inert skeleton to a state of unlife. ";
#endif
        break;

    case SPELL_VAMPIRIC_DRAINING:
#ifdef JP
        description += "ª˝∏Ì√º¿« ª˝∏Ì∑¬¿ª ª©æ—æ∆ Ω√¿¸¿⁄ø°∞‘ ¿¸º€«—¥Ÿ. $"
                       "ª˝∏Ì∑¬¿∫ Ω√¿¸¿⁄¿« ª˝∏Ì∑¬ «—∞Ë ¿ÃªÛ¿ª ª©æ—¿ª ºˆ æ¯¥Ÿ. ";
#else
        description += "steals the life of a living creature and grants it "
            "to the caster. Life will not be drained in excess of "
            "what the caster can capably absorb. ";
#endif
        break;

    case SPELL_SUMMON_WRAITHS:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄∏¶ µΩ¥¬ æµ•µÂ¿« ºº∑¬¿ª º“»Ø«—¥Ÿ. ";
#else
            "calls on the powers of the undead to aid the caster. ";
#endif
        break;

    case SPELL_DETECT_ITEMS:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄¿« ∫Œ±Ÿø° ¡∏¿Á«œ¥¬ ∏µÁ æ∆¿Ã≈€¿ª √£æ∆≥Ω¥Ÿ. ";
#else
            "detects any items lying about the caster's general vicinity. ";
#endif
        break;

    case SPELL_BORGNJORS_REVIVIFICATION:
#ifdef JP
        description += "Ω√¿¸¿⁄øÕ ±◊ ∫Œ«œ¿« ªÛ√≥∏¶ øœ¿¸»˜ ƒ°∑·«œ¡ˆ∏∏ $"
                       "¥Ò∞°∑Œ «««ÿø° ¥Î«— πÊæÓ∑¬¿Ã øµ±∏»˜ æ‡«ÿ¡¯¥Ÿ.  $"
                       "∏∂π˝ø° º˜∑√µ«æÓ ¿÷¿ªºˆ∑œ ¥Ò∞°¥¬ ¡ŸæÓµÁ¥Ÿ. ";
#else
        description += "instantly heals any and all wounds suffered by the "
            "caster with an attendant, but also permanently lessens his or her "
            "resilience to injury -- the severity of which is dependent on "
            "(and inverse to) magical skill. ";
#endif
        break;

    case SPELL_BURN:
#ifdef JP
        description += "∏ÛΩ∫≈Õ∏¶ ≈¬øˆπˆ∏∞¥Ÿ. ";
#else
        description += "burns a creature. ";
#endif
        break;

    case SPELL_FREEZE:
#ifdef JP
        description += "∏ÛΩ∫≈Õ∏¶ æÛ∏∞¥Ÿ. $"
                       "≥√«˜µøπ∞ø°∞‘¥¬ ¿œΩ√¿˚¿Œ Ω≈√º¥ÎªÁ º”µµ ∞®º“∏¶ ∫“∑Øø√ ºˆ ¿÷¥Ÿ. ";
#else
        description += "freezes a creature. This may temporarily slow the "
            "metabolism of a cold-blooded creature. ";
#endif
        break;

    case SPELL_SUMMON_ELEMENTAL:
#ifdef JP
        description +=
            "ø¯º“∞Ëø°º≠ ¡§∑…¿ª º“»Ø«ÿ Ω√¿¸¿⁄¿« æ∆±∫¿Ã µ«∞‘ «—¥Ÿ. $"
            "º“»Øø°¥¬ «ÿ¥Á«œ¥¬ ¡§∑…¿« ø¯º“∞° ¡÷¿ßø° ¥Î∑Æ¿∏∑Œ ¡∏¿Á«ÿæﬂ «—¥Ÿ.  $"
            "¥Î¡ˆøÕ ¥Î±‚¿« ∞ÊøÏø°¥¬ ¥Î∫Œ∫– πÆ¡¶∞° µ«¡ˆ æ ¡ˆ∏∏ $"
            "∫“∞˙ π∞¿« ∞ÊøÏø°¥¬ πÆ¡¶∞° µ… ∞Õ¿Ã¥Ÿ. $"
            "Ω√¿¸¿⁄∞° «ÿ¥Á«œ¥¬ ¡§∑… ∏∂π˝ø° ¿Õº˜«œ¥Ÿ∏È $"
            "º“»Øµ«¥¬ ¡§∑…¿∫ ¥Î√º∑Œ øÏ»£¿˚¿œ ∞Õ¿Ã¥Ÿ. ";
#else
        description += "calls forth "
            "a spirit from the elemental planes to aid the caster. "
            "A large quantity of the desired element must be "
            "available; this is rarely a problem for earth and air, "
            "but may be for fire or water. The elemental will usually "
            "be friendly to casters -- especially those skilled in "
            "the appropriate form of elemental magic.";
#endif
        break;

    case SPELL_OZOCUBUS_REFRIGERATION:
#ifdef JP
        description += "Ω√¿¸¿⁄øÕ Ω√¿¸¿⁄¿« ¡÷¿ßø°º≠ ø≠¿ª »Ìºˆ«—¥Ÿ. $"
                       "≥√±‚ø° ≥ªº∫¿Ã æ¯¥¬ ¡∏¿Á¥¬ «««ÿ∏¶ ¿‘¥¬¥Ÿ. ";
#else
        description += "drains the heat from the caster and her "
            "surroundings, causing harm to all creatures not resistant to "
            "cold. ";
#endif
        break;

    case SPELL_STICKY_FLAME:
#ifdef JP
        description += "æ◊√º∑Œ ¿Ã∑ÁæÓ¡¯ ∫“ µ¢æÓ∏Æ∏¶ ∏∏µÈæÓ≥Ω¥Ÿ. $"
                       "¿Ã µ¢æÓ∏Æ¥¬ ¿˚ø°∞‘ ¥ﬁ∂Û∫ŸæÓ ¿˚¿ª ≈¬øˆπˆ∏∞¥Ÿ. ";
#else
        description += "conjures a sticky glob of liquid fire, which will "
            "adhere to and burn any creature it strikes. ";
#endif
        break;

    case SPELL_SUMMON_ICE_BEAST:
#ifdef JP
        description += "æÛ¿Ω æﬂºˆ∏¶ º“»Ø«ÿ Ω√¿¸¿⁄¿« ∫Œ«œ∑Œ ªÔ¥¬¥Ÿ. ";
#else
        description += "calls forth " "a beast of ice to serve the caster. ";
#endif
        break;

    case SPELL_OZOCUBUS_ARMOUR:
#ifdef JP
        description +=
            "æÛ¿Ω¿« ∫∏»£√˛¿∏∑Œ Ω√¿¸¿⁄∏¶ ∞®Ω—¥Ÿ. $"
            "∞≠µµ¥¬ Ω√¿¸¿⁄¿« æÛ¿Ω ∏∂π˝ º˜∑√µµø° µ˚∂Û ¥ﬁ∂Û¡¯¥Ÿ. $"
            "Ω√¿¸¿⁄øÕ ±◊ ¿Â∫Ò¥¬ ≥√±‚∑Œ∫Œ≈Õ ∫∏»£µ«¡ˆ∏∏,  $"
            "Ω√¿¸¿⁄∞° π´∞≈øÓ ∞©ø ¿ª ¬¯øÎ«œ∞Ì ¿÷¿ª ∂ß¥¬ ¿˚øÎµ«¡ˆ æ ¥¬¥Ÿ. $"
            "Ω√¿¸¿⁄∞° æÛ¿Ω «¸≈¬¿œ ∞ÊøÏ ¿Ã ¡÷πÆ¿« »ø∞˙¥¬ ¡ı∆¯µ»¥Ÿ. ";
#else
        description += "encases the caster's body in a protective layer "
            "of ice, the power of which depends on his or her "
            "skill with Ice magic. The caster and the caster's "
            "equipment are protected from the cold, but the "
            "spell will not function for casters already wearing "
            "heavy armour.  The effects of this spell are boosted "
            "if the caster is in Ice Form. ";
#endif
        break;

    case SPELL_CALL_IMP:
#ifdef JP
        description += "¡ˆø¡¿« ±∏∏€¿∏∑Œ∫Œ≈Õ ¿€¿∫ æ«∏∂∏¶ º“»Ø«—¥Ÿ. ";
#else
        description += "calls forth " "a minor demon from the pits of Hell. ";
#endif
        break;

    case SPELL_REPEL_MISSILES:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ «‚«œø© ≥Øæ∆ø¿¥¬ ∞¯∞›¿Ã ∏Ì¡ﬂ«“ »Æ∑¸¿ª ∞®º“Ω√≈≤¥Ÿ.  $"
                       "¿€¿∫ πÃªÁ¿œ∑˘ ∞¯∞›¿∏∑Œ∫Œ≈Õ $"
                       "∂Û¿Ã∆Æ¥◊ ∫º∆Æ≥™ øÎ¿« ∫Í∑πΩ∫µÓ ∞≠∑¬«— ∞¯∞›ø°µµ ¿˚øÎµ»¥Ÿ.  ";
#else
        description += "reduces the chance of projectile attacks striking "
            "the caster. Even powerful attacks such as "
            "lightning bolts or dragon breath are affected, "
            "although smaller missiles are repelled to a "
            "much greater extent. ";
#endif
        break;

    case SPELL_BERSERKER_RAGE:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ ¿œΩ√¿˚¿Œ ∫–≥ÎªÛ≈¬∑Œ ∏∏µÁ¥Ÿ. ";
#else
        description += "sends the caster into a temporary psychotic rage. ";
#endif
        break;

    case SPELL_DISPEL_UNDEAD:
        description +=
#ifdef JP
            "æµ•µÂø°∞‘ ∏≈øÏ ∞≠«— µ•πÃ¡ˆ∏¶ ¡ÿ¥Ÿ. ";
#else
            "inflicts a great deal of damage on an undead creature. ";
#endif
        break;

        // spell  86 - Guardian
        // spell  87 - Pestilence
        // spell  99 - Thunderbolt
        // spell 100 - Flame of Cleansing
        // spell 101 - Shining Light
        // spell 102 - Summon Daeva
        // spell 103 - Abjuration II

    case SPELL_TWISTED_RESURRECTION:
#ifdef JP
        description += "Ω√¿¸¿⁄∞° ¥Ÿºˆ¿« Ω√√ºø° ª˝∏Ì∑¬¿ª ∫“æÓ ≥÷¿ª ºˆ ¿÷∞‘ «—¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿ª Ω√¿¸«œ±‚ ¿ß«ÿº≠¥¬ ∏Ó ∞≥¿« Ω√√º∏¶ º“∏«—¥Ÿ. $"
                       "Ω√√º¿« ºˆ∞° ∏π¿ª ºˆ∑œ º∫∞¯»Æ∑¸µµ ¡ı∞°«—¥Ÿ.";
#else
        description += "allows its caster to imbue a mass of deceased flesh "
            "with a magical life force. Casting this spell involves "
            "the assembling several corpses together; the greater "
            "the combined mass of flesh available, the greater the "
            "chances of success. ";
#endif
        break;

    case SPELL_REGENERATION:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« »∏∫π∑¬¿ª ¿œ¡§ º¯∞£¿Ã¡ˆ∏∏ ∏≈øÏ ∫¸∏£∞‘ «—¥Ÿ. $"
                       "¥ı∫“æÓ ¿ΩΩƒ¿« º“∏µµ ¡ı∞°«—¥Ÿ.  ";
#else
        description += "dramatically but temporarily increases the caster's "
            "recuperative abilities, while also increasing the rate "
            "of food consumption. ";
#endif
        break;

    case SPELL_BONE_SHARDS:
#ifdef JP
        description += "«ÿ∞Ò ∂«¥¬ ∞Ô√Ê¿« ø‹∞Ò∞›ø°º≠ ≥™ø¬ ∆ƒ∆Ì¿∏∑Œ  $"
                       "ƒ°∏Ì¿˚¿Œ ∆¯«≥¿ª ∏∏µÈæÓ≥Ω¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ ∞≠∑…º˙ ¡÷πÆ¿« º˜∑√∞˙ ∏∂≥™∏¶ ≈©∞‘ « ø‰∑Œ «œ¡ˆ æ ±‚ ∂ßπÆø° $"
                       "≥∑¿∫ ∑π∫ß¿« Ω√¿¸¿⁄µµ »ø∞˙¿˚¿∏∑Œ ªÁøÎ«“ ºˆ ¿÷¥Ÿ. $"
                       "≈©∞Ì π´∞≈øÓ ª¿∏¶ º’ø° µÈ∞Ì ªÁøÎ«“ ºˆ∑œ ¿Ã ∏∂π˝¿« $"
                       "¿ß∑¬¿∫ ¡ı∞°«—¥Ÿ. ";
#else
        description += "uses the bones of a skeleton (or similar materials: "
            "the rigid exoskeleton of an insect, for example) to "
            "dispense a lethal spray of slicing fragments, allowing "
            "its caster to dispense with conjurations in favour of "
            "necromancy alone to provide a low-level yet very "
            "powerful offensive spell. The use of a large and "
            "heavy skeleton (by wielding it) amplifies this spell's "
            "effect. ";
#endif
        break;

    case SPELL_BANISHMENT:
#ifdef JP
        description += "∏ÛΩ∫≈Õ∏¶ æÓ∫ÒΩ∫∑Œ ∫∏≥ªπˆ∏∞¥Ÿ. $"
                       "∏∏æ‡ Ω∫Ω∫∑Œ ±◊ ¿Âº“∏¶ πÊπÆ«œ∞Ì¿⁄ «—¥Ÿ∏È $"
                       "Ω√¿¸¿⁄¥¬ ¿Ã ¡÷πÆ¿ª ¿⁄Ω≈ø°∞‘ ªÁøÎ«œ∏È µ»¥Ÿ. ";
#else
        description += "banishes one creature to the Abyss. Those wishing "
            "to visit that unpleasant place in person may always "
            "banish themselves. ";
#endif
        break;

    case SPELL_CIGOTUVIS_DEGENERATION:
        description +=
#ifdef JP
            "∏ÛΩ∫≈Õ∏¶ ∏∆π⁄∂Ÿ¥¬ ªÏµ¢æÓ∏Æ∑Œ ∫Ø¿ÃΩ√≈≤¥Ÿ. ";
#else
            "mutates one creature into a pulsating mass of flesh. ";
#endif
        break;

    case SPELL_STING:
#ifdef JP
        description += "µ∂¿Ã ¿÷¥¬ ∏∂π˝ ¥Ÿ∆Æ∏¶ ¥¯¡¯¥Ÿ. ";
#else
        description += "throws a magical dart of poison. ";
#endif
        break;

    case SPELL_SUBLIMATION_OF_BLOOD:
#ifdef JP
        description += "ªÏ, «˜æ◊, ±◊ π€¿« √ºæ◊¿ª ∏∂≥™∑Œ ¿¸»Ø«—¥Ÿ. $"
                       "Ω√¿¸¿⁄¥¬ ¿Ã ¡÷πÆ¿ª ∂««— ¿⁄Ω≈¿« ∏ˆ¿ª «‚«ÿ ªÁøÎ«“ ºˆµµ ¿÷¥Ÿ. $"
                       "(¿ß«Ë«œ±‰ «œ¡ˆ∏∏ ¡ÔªÁ«“ ¡§µµ¥¬ æ∆¥œ¥Ÿ.) $"
                       "»§¿∫ ¡◊¿∫¡ˆ æÛ∏∂ µ«¡ˆ æ ¥¬ ªÏµ¢æÓ∏Æ∏¶ $"
                       "º’ø° µÈ∞Ì ªÁøÎ«“ ºˆµµ ¿÷¥Ÿ. ";
#else
        description += "converts flesh, blood, and other bodily fluids "
            "into magical energy. Casters may focus this spell "
            "on their own bodies (which can be dangerous but "
            "never directly lethal) or can wield freshly butchered "
            "flesh in order to draw power into themselves. ";
#endif
        break;

    case SPELL_TUKIMAS_DANCE:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« º’ø° µÈ∏∞ π´±‚∏¶ √„√ﬂ∞‘ «œø© $"
                       "¿˚¿ª ∞¯∞›Ω√≈≤¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ ¡ˆ∆Œ¿ÃøÕ ¿«¡ˆ∞° ¿÷¥¬ æ∆∆º∆—∆Æø°¥¬ "
                       "¿˚øÎµ«¡ˆ æ ¥¬¥Ÿ. ";
#else
        description += "causes a weapon held in the caster's hand to dance "
            "into the air and strike the caster's enemies. It will "
            "not function on magical staves and certain "
            "willful artefacts. ";
#endif
        break;

    case SPELL_HELLFIRE:        // basically, a debug message {dlb}
#ifdef JP
        description += "µΩ∫∆‰¿Ã≈Õ¿« ¡ˆ∆Œ¿Ã∑Œ∏∏ ±‚¥…«ÿæﬂ «—¥Ÿ. $" //∞À≈‰ - ±∏¿Ã - ∞≥πﬂ¿⁄øÎ¿ŒµÌ.
                       "æÓ∂ª∞‘ ¿Ã∞… ¿–¿ª ºˆ ¿÷¡ˆ? (describe.cc)";
#else
        description += "should only be available from Dispater's staff. "
            "So how are you reading this? ";
#endif
        break;

    case SPELL_SUMMON_DEMON:
#ifdef JP
        description += "∆«µ•∏¥œøÚ¿∏∑Œ ≈Î«œ¥¬ πÆ¿ª ø≠æÓ $"
                       "¿œ¡§Ω√∞£ Ω√¿¸¿⁄∏¶ µµøÔ ∫Œ«œ∏¶ º“»Ø«—¥Ÿ.  ";
#else
        description += "opens a gate to the realm of Pandemonium "
            "and draws forth one of its inhabitants "
            "to serve the caster for a time. ";
#endif
        break;

    case SPELL_DEMONIC_HORDE:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« ¿˚∞˙ ΩŒøÏ¥¬ ¿€¿∫ æ«∏∂µÈ¿« π´∏Æ∏¶ º“»Ø«—¥Ÿ. ";
#else
        description += "calls forth "
            "a small swarm of small demons "
            "to do battle with the caster's foes. ";
#endif
        break;

    case SPELL_SUMMON_GREATER_DEMON:
#ifdef JP
        description += "∆«µ•∏¥œøÚ¿« ¿ß¥Î«— æ«∏∂∏¶ º“»Ø«œø© $"
                       "Ω√¿¸¿⁄¿« ∫Œ«œ∑Œ «—¥Ÿ. ¡÷¿««“ ∞Õ¿∫ $"
                       "æ«∏∂ø°∞‘ ∫π¡æ¿ª ∞≠ø‰«œ∞‘ «œ¥¬ ¡÷πÆ¿« »ø∞˙∞° $"
                       "æ«∏∂∞° ¿Ã ºº∞Ëø° ¡∏¿Á«œ¥¬ µøæ»ø°µµ æ‡«ÿ¡˙¡ˆ ∏∏•¥Ÿ¥¬ ∞Õ¿Ã¥Ÿ! ";
#else
        description += "calls forth one of the greater demons of Pandemonium "
            "to serve the caster. Beware, for the spell binding it "
            "to service may not outlast "
            "that which binds it to this world! ";
#endif
        break;

    case SPELL_CORPSE_ROT:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« ¡÷∫Øø° ¿÷¥¬ Ω√√º¿« ∫Œ∆–∏¶ ∞°º”Ω√ƒ— $"
                       "≤˚¬Ô«— µ∂±‚∏¶ ≥ªª’∞‘ «—¥Ÿ. $"
                       "¿Ã µ∂±‚¥¬ ¡÷∫Ø¿« ∏ÛΩ∫≈Õ¿« ª˝∏Ì¿ª ¡ª∏‘¥¬¥Ÿ. ";
#else
        description += "rapidly accelerates the decomposition of any "
            "corpses lying around the caster, emitting in"
            "process a foul miasmic vapour, which eats away "
            "at the life force of any creature it envelops. ";
#endif
        break;

    case SPELL_TUKIMAS_VORPAL_BLADE:
#ifdef JP
        description += "Ω√¿¸¿⁄∞° µÈ∞Ì ¿÷¥¬ ∞À¿ª ¿œΩ√¿˚¿∏∑Œ ≥Øƒ´∑”∞‘ «—¥Ÿ. "
                       "∆Ø∫∞«— ∞≠»≠∏∂π˝¿Ã ∞…∑¡¿÷¥¬ π´±‚ø°¥¬ »ø∞˙∞° æ¯¥Ÿ. ";
#else
        description += "bestows a lethal but temporary sharpness "
            "on a sword held by the caster. It will not affect "
            "weapons otherwise subject to special enchantments. ";
#endif
        break;

    case SPELL_FIRE_BRAND:
#ifdef JP
        description += "Ω√¿¸¿⁄∞° µÈ∞Ì ¿÷¥¬ π´±‚∏¶ ∫“±Ê¿Ã ∞®ΩŒ∞‘ «—¥Ÿ. "
                       "∆Ø∫∞«— ∞≠»≠∏∂π˝¿Ã ∞…∑¡¿÷¥¬ π´±‚ø°¥¬ »ø∞˙∞° æ¯¥Ÿ. ";
#else
        description += "sets a weapon held by the caster ablaze. It will not "
            "affect weapons otherwise subject to special enchantments. ";
#endif
        break;

    case SPELL_FREEZING_AURA:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄∞° µÈ∞Ì ¿÷¥¬ π´±‚∏¶ ¬˜∞°øÓ ≥√±‚∑Œ ∞®Ω—¥Ÿ. "
            "∆Ø∫∞«— ∞≠»≠∏∂π˝¿Ã ∞…∑¡¿÷¥¬ π´±‚ø°¥¬ »ø∞˙∞° æ¯¥Ÿ. ";
#else
            "surrounds a weapon held by the caster with an aura of "
            "freezing cold. It will not affect weapons which are "
            "otherwise subject to special enchantments. ";
#endif
        break;

    case SPELL_LETHAL_INFUSION:
#ifdef JP
        description += "Ω√¿¸¿⁄∞° µÈ∞Ì ¿÷¥¬ π´±‚∏¶ ªÁæ««— ø°≥ ¡ˆ∑Œ ∞®Ω—¥Ÿ. "
                       "∆Ø∫∞«— ∞≠»≠∏∂π˝¿Ã ∞…∑¡¿÷¥¬ π´±‚ø°¥¬ »ø∞˙∞° æ¯¥Ÿ.";
#else
        description += "infuses a weapon held by the caster with unholy "
            "energies. It will not affect weapons which are "
            "otherwise subject to special enchantments. ";
#endif
        break;

    case SPELL_CRUSH:           // a theory of gravity in Crawl? {dlb}
#ifdef JP
        description += "¡ﬂ∑¬∆ƒ∏¶ ¿ÃøÎ«ÿ ∞°±ÓøÓ ∏ÛΩ∫≈Õ∏¶ ¡˛π∂∞µ¥Ÿ. ";
#else
        description += "crushes a nearby creature with waves of "
            "gravitational force. ";
#endif
        break;

    case SPELL_BOLT_OF_IRON:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« ¿˚ø°∞‘ ≈©∞Ì π´∞≈øÓ ±›º” »≠ªÏ¿ª πﬂªÁ«—¥Ÿ. ";
#else
        description += "hurls "
            "a large and heavy metal bolt " "at the caster's foes. ";
#endif
        break;

    case SPELL_STONE_ARROW:
#ifdef JP
        description += "»∏¿¸«œ¥¬ ≥Øƒ´∑ŒøÓ µπ ∞°Ω√∏¶ πﬂªÁ«—¥Ÿ. ";
#else
        description += "hurls "
            "a sharp spine of rock outward from the caster. ";
#endif
        break;

    case SPELL_TOMB_OF_DOROKLOHE:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ µπ∫Æ¿∏∑Œ ∞®Ω—¥Ÿ. $"
                       "¿Ã ∫Æ¿∫ π∞√º∞° ¿÷æÓµµ ±◊ ¿Âº“ø° ∏∏µÈæÓ ¡ˆ¡ˆ∏∏ $"
                       "∏ÛΩ∫≈Õ∞° ¿÷¥¬ ∞˜ø°¥¬ ∏∏µÈæÓ¡ˆ¡ˆ æ ¥¬¥Ÿ. $"
                       "∫¸¡Æ≥™ø√ ºˆ¥‹µµ æ¯¿Ã ¿Ã ¡÷πÆ¿ª Ω√¿¸«œ¥¬ ∞Õ¿∫ πŸ∫∏¡˛¿Ã¥Ÿ. ";
#else
        description += "entombs the caster within four walls of rock. These "
            "walls will destroy most objects in their way, but "
            "their growth is obstructed by the presence of any "
            "creature. Beware - only the unwise cast this spell "
            "without reliable means of escape. ";
#endif
        break;

    case SPELL_STONEMAIL:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ µŒ≤®øÓ µπ¿« ∫Ò¥√∑Œ ∞®Ω—¥Ÿ. $"
                       "∫Ò¥√¿« ≥ª±∏º∫¿∫ ¥Î¡ˆ∏∂π˝¿« º˜∑√µµø° µ˚∏•¥Ÿ.  $"
                       "¿Ã ∫Ò¥√¿∫ ¥Ÿ∏• πÊæÓ±∏øÕ ∞¯¡∏«œ¡ˆ∏∏, ∏≈øÏ π´∞≈øˆº≠ µø¿€¿ª πÊ«ÿ«—¥Ÿ. $"
                       "Ω√¿¸¿⁄∞° µπ «¸≈¬¿œ∂ß¥¬ »ø∞˙∞° ¡ı∆¯µ»¥Ÿ. ";
#else
        description += "covers the caster with chunky scales of stone, "
            "the durability of which depends on his or her "
            "skill with Earth magic. These scales can coexist "
            "with other forms of armour, but are in and of "
            "themselves extremely heavy and cumbersome.  The effects "
            "of this spell are increased if the caster is in Statue Form. ";
#endif
        break;

    case SPELL_SHOCK:
#ifdef JP
        description += "¿¸∞›¿« »≠ªÏ¿ª ¥¯¡¯¥Ÿ. ";
#else
        description += "throws a bolt of electricity. ";
#endif
        break;

    case SPELL_SWIFTNESS:
#ifdef JP
        description += "Ω√¿¸¿⁄¥¬ ∏≈øÏ ∫¸∏£∞‘ ¿Ãµø«“ ºˆ ¿÷¥Ÿ. $"
                       "∫Ò«‡¡ﬂ¿Œ Ω√¿¸¿⁄ø°∞‘µµ »ø∞˙∞° ¿÷¥Ÿ. ";
#else
        description += "imbues its caster with the ability to achieve "
            "great movement speeds.  Flying spellcasters can move even "
            "faster.";
#endif
        break;

    case SPELL_FLY:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄ø°∞‘ ∞¯¡ﬂ¿ª ≥Ø ºˆ ¿÷∞‘ «—¥Ÿ. ";
#else
            "grants to the caster the ability to fly through the air. ";
#endif
        break;

    case SPELL_INSULATION:
#ifdef JP
        description += "¿¸±‚ ºÓ≈©∑Œ∫Œ≈Õ Ω√¿¸¿⁄∏¶ ∫∏»£«—¥Ÿ. ";
#else
        description += "protects the caster from electric shocks. ";
#endif
        break;

    case SPELL_ORB_OF_ELECTROCUTION:
#ifdef JP
        description += "∞≠∑¬«— √Ê∞›¿ª ¿œ¿∏≈∞∏Á ∆¯πﬂ«œ¥¬ $"
                       "¿¸±‚ ø°≥ ¡ˆ¿« ±∏√º∏¶ ∏∏µÈæÓ≥Ω¥Ÿ. ";
#else
        description += "hurls "
            "a crackling orb of electrical energy "
            "which explodes with immense force on impact. ";
#endif
        break;

    case SPELL_DETECT_CREATURES:
#ifdef JP
        description += "Ω√¿¸¿⁄ ¡÷∫Ø¿« ∏ÛΩ∫≈Õ∏¶ ∞®¡ˆ«—¥Ÿ. ";
#else
        description += "allows the caster to detect any creatures "
            "within a certain radius. ";
#endif
        break;

    case SPELL_CURE_POISON_II:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄ √º≥ªø° ¿÷¥¬ µ∂¿ª ¡¶∞≈«—¥Ÿ. ";
#else
            "removes some or all toxins from the caster's system. ";
#endif
        break;

    case SPELL_CONTROL_TELEPORT:
#ifdef JP
        description += "Ω√¿¸¿⁄¥¬ ¡¶æÓ∞° ∞°¥…«— ∞¯∞£¿Ãµø¿ª «“ ºˆ ¿÷¥Ÿ. $"
                       "∞¯∞£¿Ãµø¿ª ¡¶æÓ«œ¥¬ ∞Õ¿∫ ªÁøÎ¿⁄∏¶ ∏∂π˝¿« ø°≥ ¡ˆø° $"
                       "ø¿ø∞Ω√≈≥ ºˆ ¿÷¿∏π«∑Œ ¡÷¿««ÿæﬂ «—¥Ÿ. ";
#else
        description += "allows the caster to control translocations.  Be "
            "wary that controlled teleports will cause the subject to "
            "become contaminated with magical energy. ";
#endif
        break;

    case SPELL_POISON_AMMUNITION:
#ifdef JP
        description += "Ω√¿¸¿⁄∞° µÈ∞Ì ¿÷¥¬ »≠ªÏ∑˘ø° µ∂¿ª πŸ∏•¥Ÿ. ";
#else
        description += "envenoms missile ammunition held by the caster. ";
#endif
        break;

    case SPELL_POISON_WEAPON:
        description +=
#ifdef JP
            "≥Ø ¥ﬁ∏∞ π´±‚ø° ¿œΩ√¿˚¿∏∑Œ µ∂¿ª πŸ∏•¥Ÿ. $"
            "¿ÃπÃ ∏∂π˝¿∏∑Œ ∞≠»≠µ» π´±‚ø°¥¬ »ø∞˙∞° æ¯¥Ÿ. ";
#else
            "temporarily coats any sharp bladed weapon with poison.  Will only "
            "work on weapons without an existing enchantment.";
#endif
        break;

    case SPELL_RESIST_POISON:
#ifdef JP
        description += "¿œ¡§Ω√∞£µøæ» Ω√¿¸¿⁄∏¶ ∏µÁ ¡æ∑˘¿« µ∂¿∏∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. ";
#else
        description += "protects the caster from exposure to all poisons "
            "for a period of time. ";
#endif
        break;

    case SPELL_PROJECTED_NOISE:
#ifdef JP
        description += "Ω√¿¸¿⁄∞° º±≈√«— ¡ˆ¡°¿∏∑Œ∫Œ≈Õ º“¿Ω¿ª πﬂª˝Ω√≈≤¥Ÿ. ";
#else
        description += "produces a noise emanating "
            "from a place of the caster's own choosing. ";
#endif
        break;

    case SPELL_ALTER_SELF:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« ¿∞√ºø° ∫Ø¿Ã∏¶ ¿œ¿∏≈≤¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ Ω√¿¸¿⁄∏¶ æ‡»≠Ω√≈≤¥Ÿ. $"
                       "(æ‡»≠∞° ¡˜¡¢¿˚¿Œ ¡◊¿Ω¿∏∑Œ ¿ÃæÓ¡ˆ¡ˆ¥¬ æ ¥¬¥Ÿ.) $"
                       "¿ÃπÃ Ω…∞¢«œ∞‘ ∫Ø¿Ãµ» Ω√¿¸¿⁄ø°∞‘¥¬ $"
                       "¿Ã ¡÷πÆ¿Ã ≈Î«œ¡ˆ æ ¿ª ºˆµµ ¿÷¥Ÿ. ";
#else
        description += "causes aberrations to form in the caster's body, "
            "leaving the caster in a weakened state "
            "(though it is not fatal in and of itself). "
            "It may fail to affect those who are already "
            "heavily mutated. ";
#endif
        break;

// spell 145 - debugging ray

    case SPELL_RECALL:
#ifdef JP
        description += "¿Ã ¡÷πÆ¿∫ º“»Øº˙ªÁøÕ ∞≠∑…º˙ªÁø°∞‘ ¿ØøÎ«œ¥Ÿ. $"
                       "Ω√¿¸«œ∏È µø∑· π◊ ∫Œ«œ¿Œ ∏µÁ ∏ÛΩ∫≈Õ∏¶ $"
                       "Ω√¿¸¿⁄¿« ø∑¿∏∑Œ ∫“∑Øø¬¥Ÿ. ";
#else
        description += "is greatly prized by summoners and necromancers, "
            "as it allows the caster to recall any friendly "
            "creatures nearby to a position adjacent to the caster. ";
#endif
        break;

    case SPELL_PORTAL:
#ifdef JP
        description += "∑π∫ß∞£ ¿Ãµø¿Ã ∞°¥…«— ∆˜≈ª¿ª ø¨¥Ÿ $"
                       "∫∏≈Î¿« ¥¯¡Øø°º≠∏∏ ¡÷πÆ¿Ã ¿Ø»ø«œ¥Ÿ. $"
                       "∆˜≈ª¿∫ Ω√¿¸¿⁄øÕ ∏ÛΩ∫≈Õ∞° µÂ≥™µÈ ºˆ ¿÷µµ∑œ $"
                       "√Ê∫–«— Ω√∞£µøæ» ¡∏¿Á«—¥Ÿ.  $"
                       "¿Ã ¡÷πÆ¿∏∑Œ ¥¯¡Ø¿« ªÛ«—º±¿Ã≥™ $"
                       "«œ«—º±¿ª ≥—æÓº≠ µÂ≥™µÈ ºˆ¥¬ æ¯¥Ÿ. ";
#else
        description += "creates a gate allowing long-distance travel "
            "in relatively ordinary environments "
            "(i.e., the Dungeon only). The portal lasts "
            "long enough for the caster and nearby creatures "
            "to enter. Casters are never taken past the level "
            "limits of the current area. ";
#endif
        break;

    case SPELL_AGONY:
#ifdef JP
        description += "¥ÎªÛ¿« ª˝∏Ì∑¬¿ª π›¿∏∑Œ ±¥¬¥Ÿ. $"
                       "π∞∑– ¿Ã ¡÷πÆ¿∏∑Œ ¡◊¿Ωø° ¿Ã∏£∞‘ «“ ºˆ¥¬ æ¯¥Ÿ.  ";
#else
        description += "cuts the resilience of a target creature in half, "
            "although it will never cause death directly. ";
#endif
        break;

    case SPELL_SPIDER_FORM:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ µ∂¿Ã ¿÷¥¬ ∞≈πÃ ∫ÒΩ∫π´∏Æ«— ∏ÛΩ∫≈Õ∑Œ ∫Ø«¸Ω√≈≤¥Ÿ. $"
                       "¿Ã «¸≈¬ø°º≠¥¬ ¡÷πÆ Ω√¿¸¿Ã ¡∂±› »˚µÈæÓ¡¯¥Ÿ. $"
                       "∂««— ¿Ã «¸≈¬¥¬ Ω√¿¸¿⁄∏¶ ¿˙¡÷πﬁ¿∫ ¿Â∫ÒøÕ ∂ºæÓ≥ı¿ª∏∏≈≠ ∞≠∑¬«œ¡ˆ æ ¥Ÿ. ";
#else
        description += "temporarily transforms the caster into a venomous, "
            "spider-like creature.  Spellcasting is slightly more difficult "
            "in this form.  This spell is not powerful enough to allow "
            "the caster to slip out of cursed equipment. ";
#endif
        break;

    case SPELL_DISRUPT:
#ifdef JP
        description += "∏ÛΩ∫≈Õ ¡÷∫Ø¿« ∞¯∞£¿ª ø÷∞ÓΩ√ƒ— «««ÿ∏¶ ¿‘»˘¥Ÿ. ";
#else
        description += "disrupts space around another creature, "
            "causing injury.";
#endif
        break;

    case SPELL_DISINTEGRATE:
#ifdef JP
        description += "«— ∞¯∞£ø° ¿÷¥¬ ∏µÁ ∞Õ¿ª ∞›∑ƒ»˜ ∫ŒºˆæÓ∂ﬂ∏∞¥Ÿ. $"
                       "∏ÛΩ∫≈Õø°∞‘ Ω…∞¢«— «««ÿ∏¶ ¿‘»˜¥¬µ•ø°µµ ªÁøÎ∞°¥…«œ¥Ÿ. ";
#else
        description += "violently rends apart anything in a small volume of "
            "space.  Can be used to cause severe damage.";
#endif
        break;

    case SPELL_BLADE_HANDS:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« º’ø°º≠ ±‰ ≥¥ ∏æÁ¿« ƒÆ≥Ø¿ª ∏∏µÈæÓ≥Ω¥Ÿ.  $"
                       "¿Ã ªÛ≈¬ø°º≠¥¬ ¡÷πÆ Ω√¿¸¿Ã ¡∂±› »˚µÈæÓ ¡¯¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ º’∞˙ ¿˙¡÷πﬁ¿∫ π´±‚∏¶ ∂ºæÓ≥ı¿ª ∏∏≈≠ ∞≠«œ¡ˆ æ ¥Ÿ. ";
#else
        description += "causes long, scythe-shaped blades to grow "
            "from the caster's hands.  It makes spellcasting somewhat "
            "difficult.  This spell is not powerful enough to force "
            "a cursed weapon from the caster's hands.";
#endif
        break;

    case SPELL_STATUE_FORM:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ ∏≈øÏ ∆∞∆∞«œ¡ˆ∏∏ ¥¿∏Æ∞‘ øÚ¡˜¿Ã¥¬ $"
                       "µπ ºÆªÛ¿∏∑Œ ∏∏µÁ¥Ÿ. ";
#else
        description += "temporarily transforms the caster into a "
            "slow-moving (but extremely robust) stone statue. ";
#endif
        break;

    case SPELL_ICE_FORM:
#ifdef JP
        description += "¿œΩ√¿˚¿∏∑Œ Ω√¿¸¿⁄¿« ∏ˆ¿ª ¬˜∞°øÓ æÛ¿Ωª˝π∞∑Œ ∏∏µÁ¥Ÿ. ";
#else
        description += "temporarily transforms the caster's body into a "
            "frozen ice-creature. ";
#endif
        break;

    case SPELL_DRAGON_FORM:
#ifdef JP
        description += "¿œΩ√¿˚¿∏∑Œ Ω√¿¸¿⁄¿« ∏ˆ¿ª ∞≈¥Î«œ∞Ì ∫“¿ª ª’¥¬ øÎ¿∏∑Œ ∏∏µÁ¥Ÿ. ";
#else
        description += "temporarily transforms the caster into a "
            "great, fire-breathing dragon. ";
#endif
        break;

    case SPELL_NECROMUTATION:
#ifdef JP
        description += "øÏº± Ω√¿¸¿⁄¿« ∏ˆ¿ª ¿Ωø°≥ ¡ˆøÕ ƒ£º˜«— π›π∞¡˙¿« "
                       "¿Ø√º∑Œ ∏∏µÈ∞Ì, ±◊ ¥Ÿ¿Ω ¡◊¿Ω¿« »˚¿ª √§øÓ¥Ÿ. "
                       "Ω√¿¸¿⁄¥¬ ≥√±‚, µ∂, ∏∂π˝ ±◊∏Æ∞Ì "
                       "¿˚¥Î¿˚¿Œ ¿Ωø°≥ ¡ˆø° ¿˙«◊¿ª æÚ¥¬¥Ÿ. ";
#else
        description += "first transforms the caster into a "
            "semi-corporeal apparition receptive to negative energy, "
            "then infuses that form with the powers of Death. "
            "The caster becomes resistant to "
            "cold, poison, magic and hostile negative energies. ";
#endif
        break;

    case SPELL_DEATH_CHANNEL:
#ifdef JP
        description += "Ω√¿¸¿⁄∞° ¡◊¿Œ ª˝π∞¿ª Ω∫∆Â∆Æ∑≤ »£∑Ø∑Œ ∫Œ»∞Ω√≈≤¥Ÿ. ";
#else
        description += "raises living creatures slain by the caster "
            "into a state of unliving slavery as spectral horrors. ";
#endif
        break;

    case SPELL_SYMBOL_OF_TORMENT:
#ifdef JP
        description += "¡ˆø¡¿« ∞Ì≈Î¿ª ∫“∑ØøÕ Ω√¿¸¿⁄¿« ¡÷∫Øø° ¿÷¥¬ ∏µÁ ª˝∏Ì√ºø°∞‘ $"
                       "∞Ì≈Î¿∏∑Œ ∞°µÊ¬˘ «««ÿ∏¶ ¿‘»˘¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿ª ªÁøÎ«œ¥¬ ∞Õ¿∫ ¿ß«Ë¿ª ºˆπ›«—¥Ÿ. $"
                       "Ω√¿¸¿⁄∞° ∞Ì≈Îø° ∏Èø™¿œ ∞ÊøÏø°¥¬ πﬂµø«œ¡ˆ æ ¿∏∏Á $"
                       "Ω√¿¸¿⁄ø°∞‘µµ «««ÿ∏¶ ¡÷±‚ ∂ßπÆ¿Ã¥Ÿ. $"
                       "±◊ ∞≠∑¬«‘ø°µµ ∫“±∏«œ∞Ì ¿Ã ¡÷πÆ¿∏∑Œ ¡ÔªÁ∏¶ Ω√≈≥ ºˆ¥¬ æ¯¥Ÿ. ";
#else
        description += "calls on the powers of Hell to cause agonising "
            "injury to any living thing in the caster's vicinity. "
            "It carries within itself a degree of danger, "
            "for any brave enough to invoke it, for the Symbol "
            "also affects its caller and indeed will not function "
            "if he or she is immune to its terrible effects. "
            "Despite its ominous power, this spell is never lethal. ";
#endif
        break;

    case SPELL_DEFLECT_MISSILES:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ ∏µÁ πﬂªÁ√º∑Œ∫Œ≈Õ ∫∏»£«—¥Ÿ. $"
                       "¿€¿∫ »≠ªÏø°¥¬ ≈´ »ø∞˙∏¶ πﬂ»÷«œ∏Á $"
                       "∂Û¿Ã∆Æ¥◊ ∫º∆Æ ∞∞¿∫ ∞≠∑¬«— πﬂªÁ√ºø°µµ »ø∞˙∞° ¿÷¥Ÿ. ";
#else
        description += "protects the caster from "
            "any kind of projectile attack, "
            "although particularly powerful attacks "
            "(lightning bolts, etc.) are deflected "
            "to a lesser extent than lighter missiles. ";
#endif
        break;

    case SPELL_ORB_OF_FRAGMENTATION:
#ifdef JP
        description += "∆¯πﬂ«œ∏Èº≠ ƒ°∏Ì¿˚¿Œ ∆ƒ∆Ì¿∏∑Œ ≥™¥µ¥¬ π´∞≈øÓ ±›º”¿« ±∏√º∏¶ ¥¯¡¯¥Ÿ. $"
                       "∆ƒ∆Ì¿∫ ∏ÛΩ∫≈Õ∏¶ ¬ıæÓπˆ∏Æ¡ˆ∏∏, ¥ÎªÛ¿Ã ¡ﬂ∞©¿ª ¿‘æ˙¿ª ∞ÊøÏ¥¬ $"
                       "»ø∞˙¿˚¿Ã¡ˆ ∏¯«œ¥Ÿ. ";
#else
        description += "throws a heavy sphere of metal "
            "which explodes on impact into a rain of "
            "deadly, jagged fragments. "
            "It can rip a creature to shreds, "
            "but proves ineffective against heavily-armoured targets. ";
#endif
        break;

    case SPELL_ICE_BOLT:
#ifdef JP
        description += "∞≈¥Î«— æÛ¿Ω µ¢æÓ∏Æ∏¶ ¥¯¡¯¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ ¡÷∑Œ ≥√±‚ø° ¿˙«◊¿Ã æ¯¥¬ ¿˚ø°∞‘ ¿ØøÎ«œ¡ˆ∏∏. $"
                       "∆ƒ±´∑¬¿« π›¿Ã≥™ ¬˜¡ˆ«œ∞Ì ¿÷¥¬ ¡ﬂ∑Æ∞˙ ≥Øƒ´∑ŒøÚ¿« «««ÿ¥¬ $"
                       "≥√±‚ø° ¿˙«◊¿ª ∞°¡ˆ¥¬ ¿˚¿Ã∂Ûµµ π´Ω√«“ ºˆ æ¯¥Ÿ. ";
#else
        description += "throws forth a chunk of ice. "
            "It is particularly effective against "
            "those creatures not immune to the effects of freezing, "
            "but the half of its destructive potential that comes from "
            "its weight and cutting edges "
            "cannot be ignored by even cold-resistant creatures. ";
#endif
        break;

    case SPELL_ICE_STORM:
#ifdef JP
        description += "æÛ¿Ω∞˙ ¡¯¥´±˙∫Ò, ≥√±‚¿« ∞°Ω∫∑Œ ¿Ã∑ÁæÓ¡¯ ∆¯«≥øÏ∏¶ ª˝º∫«—¥Ÿ. ";
#else
        description += "conjures forth "
            "a raging blizzard of ice, sleet and freezing gasses. ";
#endif
        break;

    case SPELL_ARC:
#ifdef JP
        description += "Ω√¿¸¿⁄ø°∞‘ ±Ÿ¡¢«— π´¿€¿ß¿« ¥ÎªÛø°∞‘ $"
                       "∞≠∑¬«— ¿¸∑˘∑Œ ∞¯∞›«—¥Ÿ. ";
#else
        description += "zaps at random a nearby creature with a powerful "
            "electrical current.";
#endif
        break;

    case SPELL_AIRSTRIKE:       // jet planes in Crawl ??? {dlb}
        description +=
#ifdef JP
            "¥ÎªÛ¿ª µ—∑ØΩŒ¥¬ ¥Î±‚ø° ∞≠∑¬«— »∏ø¿∏Æ∏¶ ∏∏µÈæÓ≥Ω¥Ÿ. ";
#else
            "causes the air around a creature to twist itself into "
            "a whirling vortex of meteorological fury. ";
#endif
        break;

    case SPELL_SHADOW_CREATURES:
#ifdef JP
        description +=
            "æÓ∫ÒΩ∫¿« π∞¡˙∑Œ∫Œ≈Õ ∏ÛΩ∫≈Õ∏¶ ∏∏µÈæÓ ≥Ω¥Ÿ. $"
            "∏∏µÈæÓ≥Ω ¡∏¿Á¥¬ Ω√¿¸¿⁄¿« ±Ÿ√≥ø°º≠ æÓ∂≤ ∏ÛΩ∫≈Õ∑Œ $"
            "Ω«√º»≠µ»¥Ÿ. $"
            "¿Ã ¡÷πÆ¿∫ ∏ÛΩ∫≈Õ¿« ¿Â∫Ò±Ó¡ˆµµ ∏∏µÈæÓ ≥Ω¥Ÿ. $"
            "∏ÛΩ∫≈Õø°∞‘ ª˝º∫µ» ¿Â∫Ò¥¬ ∞≠«— «ˆΩ«º∫¿∏∑Œ ¿Œ«ÿ $"
            "øµ±∏»˜ ¡∏¿Á«—¥Ÿ. ";
#else
        description += "weaves a creature from shadows and threads of "
            "Abyssal matter. The creature thus brought into "
            "existence will recreate some type of creature "
            "found in the caster's immediate vicinity. "
            "The spell even creates appropriate equipment for "
            "the creature, which are given a lasting substance "
            //jmf: if also conjuration:
            //"by the spell's conjuration component. ";
            //jmf: else:
            "by their firm contact with reality. ";
#endif
        break;

        //jmf: new spells
    case SPELL_FLAME_TONGUE:
#ifdef JP
        description += "¿€¿∫ ∫“±Ê¿ª ∏∏µÈæÓ≥Ω¥Ÿ. ";
#else
        description += "creates a short burst of flame.";
#endif
        break;

    case SPELL_PASSWALL:
#ifdef JP
        description += "∏≈øÏ ¬™¿∫ Ω√∞£¿Ã¡ˆ∏∏ Ω√¿¸¿⁄¿« ∏ˆ¿ª πŸ¿ß∏¶ ∂’∞Ì ¡ˆ≥™∞• ºˆ ¿÷∞‘ $"
                       "∫Ø»≠Ω√≈≤¥Ÿ. $"
                       "πŸ¿ß∏¶ ≈Î∞˙«œ¥¬ µµ¡ﬂø° ¡÷πÆ¿Ã º“¡¯µ… ∞°¥…º∫µµ ¿÷∞Ì $"
                       "∏ˆ¿ª ∫Ø»≠Ω√≈∞¥¬ µøæ»ø°¥¬ π´πÊ∫Ò ªÛ≈¬∞° µ«π«∑Œ  $"
                       "¿Ã ¡÷πÆø°¥¬ ¿ß«Ë¿Ã µ˚∏•¥Ÿ. ";
#else
        description += "tunes the caster's body such that it can instantly "
            "pass through solid rock. This can be dangerous, "
            "since it is possible for the spell to expire while "
            "the caster is en route, and it also takes time for the "
            "caster to attune to the rock, during which time they will "
            "be helpless. ";
#endif
        break;

    case SPELL_IGNITE_POISON:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« Ω√æﬂø° ¿÷¥¬ ∏µÁ µ∂¿ª ∫“∑Œ ∫Ø»≠Ω√≈≤¥Ÿ. $"
                       "¿Ã∞Õ¿∫ µ∂¿Ã ¿÷¥¬ ∏ÛΩ∫≈Õ≥™ $"
                       "µ∂¿Ã µÁ π∞æ‡¿ª ∞°¡ˆ∞Ì ¥Ÿ¥œ¥¬ ¿˚ø°∞‘ ¿ØøÎ«œ¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ Ω√¿¸¿⁄ √º≥ª¿« µ∂ø°µµ ¿€øÎ«ÿ æÓ∏∂æÓ∏∂«— ∞Ì≈Î∞˙ «‘≤≤ $"
                       "µ∂¿ª ¥Ÿ ≈¬øˆπˆ∏∞¥Ÿ.  ";
#else
        description += "attempts to convert all poison within the caster's "
            "view into liquid flame. It is very effective against "
            "poisonous creatures or those carrying poison potions. "
            "It is also an amazingly painful way to eliminate "
            "poison from one's own system. ";
#endif
        break;

    case SPELL_STICKS_TO_SNAKES:        // FIXME: description sucks
#ifdef JP
        description += "Ω√¿¸¿⁄∞° µÈ∞Ì ¿÷¥¬ ∏Ò¡¶ æ∆¿Ã≈€¿ª ¿ÃøÎ«œø© "
                       "∞≠∑¬«— º“»Ø¿ª «‡«—¥Ÿ. "
                       "∏∂π˝ªÁ¿« ¡ˆ∆Œ¿Ã ∞∞¿∫ ∏∂π˝¿∏∑Œ ∞≠»≠µ» æ∆¿Ã≈€¿∫ "
                       "øµ«‚¿ª πﬁ¡ˆ æ ¥¬¥Ÿ. ";
#else
        description += "uses wooden items in the caster's grasp as raw "
            "material for a powerful summoning. Note that highly "
            "enchanted items, such as wizard's staves, will not be "
            "affected. ";
#endif
        // "Good examples of sticks include arrows, quarterstaves and clubs.";
        break;

    case SPELL_SUMMON_LARGE_MAMMAL:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ µµøÔ ∞≥∏¶ º“»Ø«—¥Ÿ. ";
#else
        description += "summons a canine to the caster's aid.";
#endif
        break;

    case SPELL_SUMMON_DRAGON:   //jmf: reworking, currently unavailable
#ifdef JP
        description += "∞≠∑¬«— øÎ¿ª º“»Ø«ÿ, Ω√¿¸¿⁄¿« ∏Ì∑…ø° µ˚∏£µµ∑œ ¡ˆπË«—¥Ÿ. $"
                       "¡÷¿««“ ¡°¿∫ º“»Ø∏∏ º∫∞¯«œ∞Ì ¡ˆπË¥¬ Ω«∆–«“ ∞°¥…º∫µµ ¿÷¥Ÿ¥¬ ∞Õ¿Ã¥Ÿ. ";
#else
        description += "summons and binds a powerful dragon to perform the "
            "caster's bidding. Beware, for the summons may succeed "
            "even as the binding fails. ";
#endif
        break;

    case SPELL_TAME_BEASTS:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« ¡÷∫Øø° ¿÷¥¬ µøπ∞¿ª ¡ˆπË«œ∑¡ Ω√µµ«—¥Ÿ. $"
                       "ø¯∑° ±ÊµÈ¿Ã±‚ Ω¨øÓ µøπ∞ø°∞‘ ¿ﬂ ¿€øÎ«—¥Ÿ. ";
#else
        description += "attempts to tame animals in the caster's vicinity. "
            "It works best on animals amenable to domestication. ";
#endif
        break;

    case SPELL_SLEEP:
#ifdef JP
        description += "¥ÎªÛ¿« Ω≈¡¯¥ÎªÁ º”µµ∏¶ ¥ √Á, ¿˙√ºø¬ø° ¿««— $"
                       "µø∏ÈªÛ≈¬∑Œ ∏∏µÁ¥Ÿ. $"
                       "≥√«˜µøπ∞ø°∞‘¥¬ √ﬂ∞° »ø∞˙∞° ¿÷¿ª¡ˆµµ ∏∏•¥Ÿ. ";
#else
        description += "tries to lower its target's metabolic rate, "
            "inducing hypothermic hibernation. It may have side effects "
            "on cold-blooded creatures. ";
#endif
        break;

    case SPELL_MASS_SLEEP:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« Ω√æﬂø° ¿÷¥¬ ∏µÁ ¥ÎªÛ¿« Ω≈¡¯¥ÎªÁº”µµ∏¶ ¿˙«œΩ√ƒ— $"
                       "¿˙√ºø¬ø° ¿««— µø∏ÈªÛ≈¬∏¶ ¿Øπﬂ«—¥Ÿ. $"
                       "≥√«˜µøπ∞ø°∞‘¥¬ √ﬂ∞° »ø∞˙∞° ¿÷¿ª¡ˆµµ ∏∏•¥Ÿ. ";
#else
        description += "tries to lower the metabolic rate of every creature "
            "within the caster's view enough to induce hypothermic hibernation. "
            "It may have side effects on cold-blooded creatures. ";
#endif
        break;

/* ******************************************************************
// not implemented {dlb}:
    case SPELL_DETECT_MAGIC:
      description += "probes one or more items lying nearby for enchantment. "
         "An experienced diviner may glean additional information. ";
      break;
****************************************************************** */

    case SPELL_DETECT_SECRET_DOORS:
#ifdef JP
        description += "∞‘¿∏∏ßπ¿Ã ∏«Ë∞°µÈ¿Ã ªÁ∂˚«œ¥¬ ∞Õ¿∏∑Œ $"
                       "≈Ωªˆø° ∞…∏Æ¥¬ Ω√∞£¿ª ∏≈øÏ ¡Ÿø©¡÷±‚ ∂ßπÆ¿Ã¥Ÿ. ";
#else
        description += "is beloved by lazy dungeoneers everywhere, for it can "
            "greatly reduce time-consuming searches. ";
#endif
        break;

    case SPELL_SEE_INVISIBLE:
#ifdef JP
        description += "Ω√¿¸¿⁄ø°∞‘ ∫∏≈Î¿« Ω√∞¢¿∏∑Œ∫Œ≈Õ º˚∞‹¡¯ ∞Õ¿ª $"
                       "∫º ºˆ ¿÷∞‘ «—¥Ÿ. ";
#else
        description += "enables the caster to perceive things that are "
            "shielded from ordinary sight. ";
#endif
        break;

    case SPELL_FORESCRY:
#ifdef JP
        description += "∏≈øÏ ¥‹Ω√∞£»ƒ¿« πÃ∑°∏¶ Ω√¿¸¿⁄∞° ∫º ºˆ ¿÷∞‘ «—¥Ÿ. $"
                       "¿¸≈ı¿« ∞·∞˙∏¶ æÀ ºˆ ¿÷¿ª∏∏≈≠¿« πÃ∑°¥¬ ∫º ºˆ æ¯¡ˆ∏∏ $"
                       "¿˚¿« ∞¯∞›¿Ã æÓµ∑Œ ≥Øæ∆µÈ ∞Õ¿Œ¡ˆ (π›ªÁΩ≈∞Ê¿Ã «„∂Ù«œ¥¬ «—) $"
                       "¡§µµ¥¬ øπ√¯«“ ºˆ ¿÷∞‘ «ÿ¡ÿ¥Ÿ. ";
#else
        description += "makes the caster aware of the immediate future; "
            "while not far enough to predict the result of a "
            "fight, it does give the caster ample time to get "
            "out of the way of a punch (reflexes allowing). ";
#endif
        break;

    case SPELL_SUMMON_BUTTERFLIES:
        description +=
#ifdef JP
            "«¸«¸ªˆªˆ¿« ≥™∫Òπ´∏Æ∏¶ ∏∏µÈæÓ≥Ω¥Ÿ. ∏≈øÏ æ∆∏ß¥‰¥Ÿ!  ";
#else
            "creates a shower of colourful butterflies. How pretty!";
#endif
        break;

    case SPELL_WARP_BRAND:
#ifdef JP
        description += "¿œΩ√¿˚¿∏∑Œ Ω√¿¸¿⁄¿« π´±‚ø° ±πº“¿˚¿Œ ∞¯∞£ø÷∞Ó¿« »˚¿ª ∫Œø©«—¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ ∏≈øÏ ¿ß«Ë«—µ• $"
                       "±◊ ∞¯∞£¿Ã Ω√¿¸¿⁄ø°∞‘µµ øµ«‚¿ª ¡Ÿ ºˆ ¿÷±‚ ∂ßπÆ¿Ã¥Ÿ. ";
#else
        description += "temporarily binds a localized warp field to the "
            "invoker's weapon. This spell is very dangerous to cast, "
            "as the field is likely to effect the caster as well. ";
#endif
        break;

    case SPELL_SILENCE:
#ifdef JP
        description += "Ω√¿¸¿⁄ ¡÷∫Ø¿« ∏µÁ º“∏Æ∏¶ ¡¶∞≈«—¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ Ω√¿¸¿⁄¿« ¡÷∫Øø°º≠ ∏∂π˝ µŒ∑Á∏∂∏Æ ¿–±‚, ¡÷πÆ Ω√¿¸, $"
                       "±‚µµ≥™ º“∏Æ¡ˆ∏£¥¬ ∞Õ µÓ¿ª ∫“∞°¥…«œ∞‘ «—¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿Ã ¥ÁΩ≈¿« ¡∏¿Á∏¶ º˚±Ê ºˆ¥¬ æ¯¥Ÿ. $"
                       "ø÷≥ƒ«œ∏È ¿Ã ±ÿ¥‹¿˚¿∏∑Œ ∫Œ¿⁄ø¨Ω∫∑ØøÓ ¡∂øÎ«‘¿∫ $"
                       "π∫∞° ∏≈øÏ ¿ÃªÛ«œ¥Ÿ∞Ì ¥¿≥¢∞‘ «œ±‚ ∂ßπÆ¿Ã¥Ÿ. ";
#else
        description += "eliminates all sound near the caster. This makes "
            "reading scrolls, casting spells, praying or yelling "
            "in the caster's vicinity impossible. (Applies to "
            "caster too, of course.)  This spell will not hide your "
            "presence, since its oppressive, unnatural effect "
            "will almost certainly alert any living creature that something "
            "is very wrong. ";
#endif
        break;

    case SPELL_SHATTER:
        description +=
#ifdef JP
            "Ω√¿¸¿⁄¿« ¡÷¿ßø° √Ê∞›º∫¿« ∏∂∑¬¿ª ∆¯πﬂΩ√≈≤¥Ÿ. $"
            "¿Ã ¡÷πÆ¿∫ ¥Î∫Œ∫–¿« ¿˚ø°∞‘ «««ÿ∏¶ ¡÷¡ˆ∏∏ $"
            "∆Ø»˜ µπ¿Ã≥™ ±›º”, ∞·¡§∞˙ ∞∞¿∫ ≈∫∑¬¿Ã æ¯¥¬ π∞√º∑Œ ¿Ã∑ÁæÓ¡¯ $"
            "¿˚ø°∞‘ »ø∞˙∞° ¿÷¥Ÿ. $"
            "∂««— ¿Ã ¡÷πÆ¿∫ ∫Æø°µµ »ø∞˙∞° ¿÷¥Ÿ∞Ì æÀ∑¡¡Æ ¿÷¥Ÿ. ";
#else
            "causes a burst of concussive force around the caster, "
            "which will damage most creatures, although those "
            "composed of stone, metal or crystal, or otherwise "
            "brittle, will particularly suffer. The magic has been "
            "known to adversely affect walls. ";
#endif
        break;

    case SPELL_DISPERSAL:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« πŸ∑Œ ø∑ø° ¿÷¥¬ ∏ÛΩ∫≈Õ∏¶ º¯∞£¿ÃµøΩ√≈≤¥Ÿ. ";
#else
        description += "tries to teleport away any monsters directly beside "
                       "the caster. ";
#endif
        break;

    case SPELL_DISCHARGE:
#ifdef JP
        description += "Ω√¿¸¿⁄ø°∞‘ ±Ÿ¡¢«œ¥¬ ¿˚ø°∞‘ π¯∞≥∏¶ ∂≥æÓ∂ﬂ∏∞¥Ÿ. $"
                       "¿Ã π¯∞≥¥¬ ¡¢¡ˆµ«æÓ ªÁ∂Û¡˙ ∂ß±Ó¡ˆ (Ω√¿¸¿⁄∏¶ ∆˜«‘«ÿº≠) ¿Œ¡¢«— ¥ÎªÛø°∞‘ $"
                       "¿¸µµµ«æÓ ∞£¥Ÿ. ";
#else
        description += "releases electric charges against those "
                       "next to the caster.  These may arc to "
                       "adjacent monsters (or even the caster) before "
                       "they eventually ground out. ";
#endif
        break;

    case SPELL_BEND:
        description +=
#ifdef JP
            "±π¡ˆ¿˚¿Œ ∞¯∞£ø÷∞Ó¿ª ¿ÃøÎ«ÿ ±Ÿ√≥¿« ¿˚ø°∞‘ «««ÿ∏¶ ¡ÿ¥Ÿ. ";
#else
            "applies a localized spatial distortion to the detriment"
            " of some nearby creature. ";
#endif
        break;

    case SPELL_BACKLIGHT:
#ifdef JP
        description += "¥ÎªÛ¿ª µ—∑ØΩŒ¥¬ »÷±§¿ª πﬂª˝Ω√ƒ— $"
                       "±◊ ∏Ω¿¿ª º±∏Ì«œ∞‘ ∫∏¿Ãµµ∑œ «—¥Ÿ. $"
                       "¿Ã ∫˚¿∫ ¥¯¡Ø¿« æÓµŒøÚ¿ª ªÛº‚Ω√≈∞π«∑Œ $"
                       "¡÷πÆø° ∞…∏∞ ¿˚¿ª ¿ﬂ ∏¬√‚ ºˆ ¿÷∞‘ «—¥Ÿ. ";
#else
        description += "causes a halo of glowing light to surround and "
            "effectively outline a creature. This glow offsets "
            "the dark, musty atmosphere of the dungeon, and "
            "thereby makes the affected creature appreciably easier to hit.";
#endif
        break;

    case SPELL_INTOXICATE:
#ifdef JP
        description += "≥˙≥ª¿« π∞¡˙ æ‡∞£¿ª æÀƒ⁄ø√∑Œ ∫Ø»≠Ω√≈≤¥Ÿ. $"
                       "¿Ã∞Õ¿∫ Ω√¿¸¿⁄¿« Ω√æﬂ≥ª¿« ∏µÁ ¡ˆ¿˚ª˝∏Ì√ºø°∞‘ $"
                       "(æ∆∏∂µµ Ω√¿¸¿⁄µµ ∆˜«‘)¿€øÎ«—¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ ∏∂π˝ªÁµÈ¿« ∆ƒ∆ºø°º≠ ∫–¿ß±‚ æ˜¿ª ¿ß«ÿ ¿⁄¡÷ ªÁøÎµ»¥Ÿ. ";
#else
        description += "works by converting a small portion of brain matter "
            "into alcohol. It affects all intelligent humanoids within "
            "the caster's view (presumably including the caster). It "
            "is frequently used as an icebreaker at wizard parties. ";
#endif
        break;

    case SPELL_GLAMOUR: // intended only as Grey Elf ability
#ifdef JP
        description += "ø§«¡¿« ∏∂π˝¿∏∑Œ $"
                       "Ω√∞¢¿ª ∞°¡¯ ¿˚ø°∞‘ Ω√¿¸¿⁄¿« ∏Ω¿¿ª »Ø∞¢¿∏∑Œ ∫∏¿Ã∞‘ «œø© $"
                       "∏ÛΩ∫≈Õ∏¶ ∏≈∑·Ω√≈∞∞Ì »•∂ıªÛ≈¬∑Œ ∏∏µÁ¥Ÿ.  ";
#else
        description += "is an Elvish magic, which draws upon the viewing "
            "creature's credulity and the caster's comeliness "
            "to charm, confuse or render comatose. ";
#endif
        break;

    case SPELL_EVAPORATE:
#ifdef JP
        description += "π∞æ‡¿ª ∞°ø≠«œø©, ¥¯¡ˆ∏È ºˆ¡ı±‚øÕ «‘≤≤ ∆¯πﬂ«œ∞‘ ∏∏µÁ¥Ÿ. $"
                       "¡÷πÆ¿« ∆Øº∫ªÛ π∞æ‡¿∫ ∞°ø≠µ ∞˙ µøΩ√ø° $"
                       "¥¯¡Æ¡Ææﬂ «—¥Ÿ. ";
#else
        description += "heats a potion causing it to explode into a large "
            "cloud when thrown.  The potion must be thrown immediately, "
            "as part of the spell, for this to work. ";
#endif
        break;

    case SPELL_FULSOME_DISTILLATION:
#ifdef JP
        description += "Ω√√ºø°º≠ ∫Œ¡§∞˙ ¿Øµ∂¿« ¡§ºˆ∏¶ ªÃæ∆≥Ω¥Ÿ. $"
                       "∫Œ∆–«— Ω√√ºø°º≠¥¬ ¡§ºˆ∏¶ ¥ı ∏π¿Ã ªÃæ∆ ≥æ ºˆ ¿÷¥Ÿ. $"
                       "æ∆∏∂µµ ¿Ã ∞·∞˙π∞¿ª ∏∂Ω√∞Ì ΩÕæÓ«œ¡¯ æ ¿ª ∞Õ¿Ã¥Ÿ. ";
#else
        description += "extracts the vile and poisonous essences from a "
            "corpse.  A rotten corpse may produce a stronger potion."
            "$$You probably don't want to drink the results. ";
#endif
        break;

/* ******************************************************************
// not implemented {dlb}:
    case SPELL_ERINGYAS_SURPRISING_BOUQUET:
      description += "transmutes any wooden items in the caster's grasp "
                     "into a bouquet of beautiful flowers. ";
      break;
****************************************************************** */

    case SPELL_FRAGMENTATION:
        description +=
#ifdef JP
            "πŸ¿ß≥™ ±◊ π€¿« ¥‹¥‹«— π∞√º∑Œ ¿Ã∑ÁæÓ¡¯ ¥ÎªÛ¿« √º≥ªø°º≠ $"
            "∆¯πﬂ¿ª πﬂª˝Ω√ƒ— ±◊ ¡÷¿ß¿« ∏µÁ ¥ÎªÛø°∞‘ «««ÿ∏¶ ¡ÿ¥Ÿ. ";
#else
            "creates a concussive explosion within a large body of "
            "rock (or other hard material), to the detriment of "
            "any who happen to be standing nearby. ";
#endif
        break;

    case SPELL_AIR_WALK:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« ∏ˆ¿ª Ω«√º∞° æ¯¥¬ ±∏∏ßµ¢æÓ∏Æ∑Œ ∫Ø»≠Ω√≈≤¥Ÿ. $"
                       "Ω«√º∞° æ¯æÓ¡¯ Ω√¿¸¿⁄¥¬ π∞∏Æ¿˚¿Œ ≈∏∞›ø° $"
                       "∞≈¿« ∏Èø™¿Ã µ«¡ˆ∏∏, ∏∂π˝¿˚¿Œ ∫“±Ê∞˙ ≥√±‚ø° «««ÿ∏¶ ¿‘¥¬¥Ÿ. $"
                       "¥Áø¨«œ∞‘µµ ∫ÒΩ«√º¿Œ Ω√¿¸¿⁄¥¬ π∞∞«¿ª ∏∏¡ˆ∞≈≥™ «“ ºˆ æ¯¡ˆ∏∏  $"
                       "±◊∑≥ø°µµ ∫“±∏«œ∞Ì ¡÷πÆ Ω√¿¸¿∫ ∞°¥…«œ¥Ÿ. ";
#else
        description += "transforms the caster's body into an insubstantial "
            "cloud. The caster becomes immaterial and nearly immune "
            "to physical harm, but is vulnerable to magical fire "
            "and ice. While insubstantial the caster is, of course, "
            "unable to interact with physical objects (but may still "
            "cast spells). ";
#endif
        break;

    case SPELL_SANDBLAST:
#ifdef JP
        description += "∞Ìº”¿« ¿‘¿⁄∑Œ ±∏º∫µ» ¬™¿∫ ∆¯«≥¿ª ∏∏µÈæÓ≥Ω¥Ÿ.  $"
                       "Ω√¿¸¿⁄∞° µπ µÓ¿« √À∏≈∏¶ ¿ÃøÎ«œ∏È $"
                       "¥ı ∞≠«— ∆¯«≥¿ª ∏∏µÈæÓ ≥æ ºˆ ¿÷¡ˆ∏∏ $"
                       "¡ˆ∏È¿« ∏∑°∏∏¿∏∑Œµµ Ω√¿¸¿∫ ∞°¥…«œ¥Ÿ. ";
#else
        description += "creates a short blast of high-velocity particles. "
            "It works best when the caster provides some source "
            "(by wielding a stone), but will do what it can with "
            "whatever ambient grit is available. ";
#endif
        break;

    case SPELL_ROTTING:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« ±Ÿ√≥ø° ¿÷¥¬ ∏µÁ ªÏ¿ª ∫Œ∆–Ω√≈≤¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ ªÏæ∆¿÷¥¬ ª˝∏Ì√ºøÕ Ω«√º∏¶ ∞°¡ˆ¥¬ æµ•µÂø°∞‘ $"
                       "»ø∞˙∞° ¿÷¥Ÿ. ";
#else
        description += "causes the flesh of all those near the caster to "
            "rot. It will affect the living and many of the "
            "corporeal undead. ";
#endif
        break;

    case SPELL_SHUGGOTH_SEED:
#ifdef JP
        description += "ªÏæ∆¿÷¥¬ ª˝∏Ì√ºø°∞‘ Ω¥∞ÌΩ∫¿« æÀ¿ª π⁄¥¬¥Ÿ. $"
                       "¿Ã∞Õ¿∫ π´º≠øÓ Ω¥∞ÌΩ∫¿« ¿Øª˝«¸≈¬¿Œ ±‚ª˝√º¿Ã¥Ÿ. $"
                       "Ω¥∞ÌΩ∫¿« æÀ¿∫ º˜¡÷∑Œ∫Œ≈Õ ª˝∏Ì∑¬¿ª »Ìºˆ«œ∏Á ±‚ª˝«—¥Ÿ. $"
                       "øœ¿¸»˜ ¥Ÿ ¿⁄∂Û∏È Ω¥∞ÌΩ∫¿« º∫√Ê¿∫ º˜¡÷¿« ∞°Ωø¿ª ∂’∞Ì  $"
                       "∆¢æÓ ≥™ø√ ∞Õ¿Ã¥Ÿ. ";
#else
        description += "implants a shuggoth seed, the larval parasitic form "
            "of the fearsome shuggoth, in a living host. The "
            "shuggoth seed will draw life from its host and then "
            "hatch, whereupon a fully grown shuggoth will burst "
            "from the unfortunate host's chest. ";
#endif
        break;

    case SPELL_MAXWELLS_SILVER_HAMMER:
#ifdef JP
        description += "¿œΩ√¿˚¿Ã¡ˆ∏∏ Ω√¿¸¿⁄∞° µÁ µ–±‚ø° ƒ°∏Ì¿˚¿Œ ¡ﬂ∑¬¿Â¿ª ∏∏µÁ¥Ÿ. $"
                       "¿ÃπÃ æÓ∂≤ ∏∂π˝¿∏∑Œ ∞≠»≠µ» π´±‚ø°¥¬ »ø∞˙∞° æ¯¥Ÿ. ";
#else
        description += "bestows a lethal but temporary gravitic field "
            "to a crushing implement held by the caster. "
            "It will not affect weapons otherwise subject to "
            "special enchantments. ";
#endif
        break;

    case SPELL_CONDENSATION_SHIELD:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ µ—∑ØΩ— ∞¯±‚∑Œ∫Œ≈Õ æ–√‡µ» ∞¯±‚¿« ø¯π›¿ª ∏∏µÈæÓ≥Ω¥Ÿ. $"
                       "¿Ã∞Õ¿∫ ∫∏≈Î¿« πÊ∆–√≥∑≥ ªÁøÎµ«∏Á, $"
                       "±◊ π–µµ, ¡Ô ∞¯∞›¿ª ∏∑æ∆≥ª¥¬ »˚¿∫ æÛ¿Ω ∏∂π˝ º˜∑√µµø° ¥ﬁ∑¡¿÷¥Ÿ. $"
                       "¿Ã ø¯π›¿∫ Ω√¿¸¿⁄¿« ¿«¡ˆ∑Œ ¡∂¡æµ«±‚ ∂ßπÆø° $"
                       "æÁº’π´±‚∏¶ µÈæÓµµ ∞…∏Æ¿˚∞≈∏Æ¡ˆ æ ¥¬¥Ÿ. ";
#else
        description += "causes a disc of dense vapour to condense out of the "
            "air surrounding the caster. It acts like a normal "
            "shield, but its density (and therefore stopping power) "
            "depends upon the caster's skill with Ice Magic. The "
            "disc is controlled by the caster's mind and thus will "
            "not conflict with the wielding of a two-handed weapon. ";
#endif
        break;

    case SPELL_STONESKIN:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« ««∫Œø° ¥Î¡ˆ∏∂π˝ º˜∑√µµø° µ˚∏£¥¬ ∞≠¿Œ«‘¿ª ¡ÿ¥Ÿ.$"
                       "¿Ã ¡÷πÆ¿∫ ∫Ò±≥¿˚ ¡§ªÛ¿˚¿Œ ««∫Œø°∏∏ ¿˚øÎµ»¥Ÿ. $"
                       "«¸√º∞° ∫Ø«¸µ» ¿⁄≥™ æµ•µÂ¥¬ ªÁøÎ«“ ºˆ æ¯¥Ÿ. $"
                       "Ω√¿¸¿⁄∞° ºÆªÛ¿« «¸≈¬¿œ∂ß ¿Ã ¡÷πÆ¿« »ø∞˙¥¬ $ "
                       "¡ı∆¯µ»¥Ÿ. ";
#else
        description += "hardens the one's skin to a degree determined "
            "by one's skill in Earth Magic. This only works on relatively "
            "normal flesh; it will aid neither the undead nor the bodily "
            "transformed.  The effects of this spell are boosted if the "
            "caster is in Statue Form. ";
#endif
        break;

    case SPELL_SIMULACRUM:
#ifdef JP
        description += "Ω√¿¸¿⁄∞° º’ø° µÁ ªÏµ¢æÓ∏Æ∑Œ∫Œ≈Õ æÛ¿Ω¿∏∑Œ µ» ∫πªÁ√º∏¶ ∏∏µÈæÓ≥Ω¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ ∫“æ»¡§«ÿº≠, æÛ¿Ω¿∫ ±›πÊ ±‚»≠«ÿ ªÁ∂Û¡Æ πˆ∏∞¥Ÿ. $"
                       "±◊∞Õ∫∏¥Ÿ ∏’¿˙ ¿˚ø°∞‘ ∫£ø©º≠ ±˙¡ˆ∞≈≥™ $"
                       "øıµ¢¿Ãø° ≥Ïæ∆πˆ∏Æ¡ˆ æ ¿∏∏È ∏ª¿Ã¥Ÿ. $";
#else
        description += "uses a piece of a flesh in hand to create a replica "
                       "of the original being out of ice. This magic is "
                       "unstable so eventually the replica will sublimate "
                       "into a freezing cloud, if it isn't hacked or melted "
                       "into a small puddle of water first. ";
#endif
        break;

    case SPELL_CONJURE_BALL_LIGHTNING:
#ifdef JP
        description += "¿¸±‚ ±∏√º∏¶ ∏∏µÈæÓ≥Ω¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ ¿ß«Ë«œ¡ˆ æ ¥Ÿ∞Ì «“ ºˆ æ¯¥Ÿ. $"
                       "¿¸±‚ ±∏√º∏¶ ¡∂¡æ«œ¥¬ ∞Õ¿∫ æÓ∑∆±‚ ∂ßπÆ¿Ã¥Ÿ. ";
#else
        description += "allows the conjurer to create ball lightning.  "
                        "Using the spell is not without risk - ball lighting "
                        "can be difficult to control. ";
#endif
        break;

    case SPELL_TWIST:
#ifdef JP
        description += "Ω√¿¸¿⁄¿« Ω√æﬂ≥ªø° ∞°∫≠øÓ ∞¯∞£ø÷∞Ó¿ª ¿œ¿∏ƒ— «««ÿ∏¶ ¿‘»˘¥Ÿ. ";
#else
        description += "causes a slight spatial distortion around a monster "
                       "in line of sight of the caster, causing injury. ";
#endif
        break;

    case SPELL_FAR_STRIKE:
#ifdef JP
        description += "Ω√æﬂ≥ª¿« ¿˚ø°∞‘ Ω√¿¸¿⁄∞° µÁ π´±‚∑Œ ∏÷∏Æ±Ó¡ˆµµ $"
                       "∞¯∞›«“ ºˆ ¿÷∞‘ «—¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿∫ π´±‚¿« º¯ºˆ«— ≈∏∞›∏∏¿ª ¿¸º€«—¥Ÿ. $"
                       "¡Ô, π´±‚¿« √ﬂ∞° ∏∂π˝»ø∞˙¥¬ ¿˚øÎµ«¡ˆ æ ¥¬¥Ÿ. $"
                       "±◊ ¿ß∑¬¿∫ π´±‚∏¶ ¥Ÿ∑Á¥¬ ¥…∑¬¿Ã≥™ »˚, ¿ßƒ° ∫Ø»≠, $"
                       "∏∂π˝ ¥…∑¬ø° ∞≈¿« øµ«‚¿ª πﬁ¡ˆ æ ¥¬¥Ÿ. ";
#else
        description += "allows the caster to transfer the force of a "
                       "weapon strike to any target the caster can see.  "
                       "This spell will only deliver the impact of the blow; "
                       "magical side-effects and enchantments cannot be "
                       "transferred in this way.  The force transferred by "
                       "this spell has little to do with one's skill with "
                       "weapons, and more to do with personal strength, "
                       "translocation skill, and magic ability. ";
#endif
        break;

    case SPELL_SWAP:
#ifdef JP
        description += "Ω√¿¸¿⁄∏¶ ¿Œ¡¢«— ∏ÛΩ∫≈ÕøÕ ¿ßƒ°∏¶ πŸ≤Ÿ∞‘ «—¥Ÿ. ";
#else
        description += "allows the caster to swap positions with an adjacent "
                       "being. ";
#endif
        break;

    case SPELL_APPORTATION:
#ifdef JP
        description += "Ω√¿¸¿⁄∞° ¡∂±› ∂≥æÓ¡¯ æ∆¿Ã≈€ π´¥ı±‚∑Œ∫Œ≈Õ ∏« ¿ß¿« $"
                       "æ∆¿Ã≈€¿ª ≤¯æÓ ø¿∞‘ «“ ºˆ ¿÷¥Ÿ.  $"
                       "π´∞‘∞° π´∞≈øÓ æ∆¿Ã≈€¿∫ º∫∞¯«“ »Æ∑¸¿Ã ¡ŸæÓµÁ¥Ÿ. $"
                       "æÓ∂≤ ¡æ∑˘¿« æ∆¿Ã≈€¿∫ ¿Ã ¡÷πÆ¿ª ªÁøÎ«“ ∂ß ¡÷¿««ÿæﬂ «—¥Ÿ. $"
                       "¿Ã ¡÷πÆ¿ª ±◊∑Ï»≠µ» (»≠ªÏ ∞∞¿∫) æ∆¿Ã≈€ø° ªÁøÎ«œ∞‘ µ«∏È $"
                       "»˚¿« ∫Œ¡∑ø° µ˚∂Û æ∆¿Ã≈€¿« ¿œ∫Œ∞° $"
                       "«„∞¯¿∏∑Œ ªÁ∂Û¡˙ ¿ß«Ë¿Ã ¿÷¥Ÿ. ";
#else
        description += "allows the caster to pull the top item or group of "
                       "similar items from a distant pile to the floor "
                       "near the caster.  The mass of the target item(s) will "
                       "make the task more difficult, with some items too "
                       "massive to ever be moved by this spell.   Using this "
                       "spell on a group of items can be risky;  insufficient "
                       "power will cause some of the items to be lost in the "
                       "infinite void.";
#endif
        break;

    default:
#ifdef JP
        DEBUGSTR("Bad spell"); //∞À≈‰ - ±∏¿Ã - »Ï»Ï...
        description += "»ÆΩ«»˜ ¿Ã∑± ∞« æ¯¥Ÿ. "
                       "µ˚∂Ûº≠ ¿Ã∑± ∏∂π˝ Ω√¿¸¿∫ «ˆ∏Ì«œ¡ˆ ∏¯«œ¥Ÿ. "
#if DEBUG
            "Ç∆Ç¢Ç§Ç©ÅAèCê≥ÇµÇΩÇ‹Ç¶ÇÊÅB ";
#else
            "Please contact Dungeon Tech Support "
            "at /dev/null for details. ";
#endif // DEBUG
#else
        DEBUGSTR("Bad spell");
        description += "apparently does not exist. "
            "Casting it may therefore be unwise. "
#if DEBUG
            "Instead, go fix it. ";
#else
            "Please contact Dungeon Tech Support "
            "at /dev/null for details. ";
#endif // DEBUG
#endif
    }

    print_description(description);

    set_keyin_mode(KEYIN_MODE_MORE);
    if (getch() == 0)
        getch();
    set_keyin_mode(KEYIN_MODE_NONE);

#ifdef DOS_TERM
    puttext(25, 1, 80, 25, buffer);
    window(1, 1, 80, 25);
#endif
}                               // end describe_spell()


//---------------------------------------------------------------
//
// describe_monsters
//
// Contains sketchy descriptions of every monster in the game.
//
//---------------------------------------------------------------
void describe_monsters(int class_described, unsigned char which_mons)
{
    std::string description;

    description.reserve(200);

#ifdef DOS_TERM
    char buffer[3400];

    gettext(25, 1, 80, 25, buffer);
    window(25, 1, 80, 25);
#endif

    clrscr();
    description = std::string( ptr_monam( &(menv[ which_mons ]), DESC_CAP_A ) );
    description += "$$";

    switch (class_described)
    {
        // (missing) case 423 - MONS_ANOTHER_LAVA_THING ??? 15jan2000 {dlb}
        //                      no entry in m_list.h 17jan200 {dlb}
        //          monster has no stats!
        // mv: changed ANOTHER_LAVA_THING to SALAMANDER, added stats and
        //     description
        // (missing) case 250 - MONS_PROGRAM_BUG ??? 16jan2000 {dlb}
    case MONS_KILLER_BEE_LARVA:
#ifdef JP
        description += "¿€∞Ì π´∑¬«— ªÏ¿Œπ˙¿« ¿Ø√Ê¿Ã¥Ÿ. ";
#else
        description += "A small, powerless larva of killer bee.";
#endif
        break;

    case MONS_QUASIT:
#ifdef JP
        description += "±Ê∞Ì øπ∏Æ«— ≤ø∏Æ∞° ¥ﬁ∏∞ ±‚∫–≥™ª€ æ«∏∂¥Ÿ. ";
#else
        description += "A small twisted demon with long sharply pointed tail.";
#endif
        break;

    case MONS_ANGEL:
#ifdef JP
        description += "≥Ø∞≥∞° ¥ﬁ∏∞ √ ¿⁄ø¨¿˚¿Œ æ∆∏ß¥ŸøÚ¿« ¡∏¿Á¥Ÿ.  $"
                       "»≤±›¿∏∑Œ ∫˚≥™¥¬ ±§√§ø° µ—∑ØΩŒø©¿÷¥Ÿ. ";
#else
        description += "A winged holy being of unnatural beauty. "
            "It's surrounded by aura of brilliant golden light. ";
#endif
        break;

    case MONS_HUMAN:
        // These should only be possible from polymorphing or shapeshifting.
#ifdef JP
        description += "¿¸«Ù ∆Ø¬°¿Ã æ¯¥¬ ¿Œπ∞¿Ã¥Ÿ. ø¿»˜∑¡ ¿ÃªÛ«—µ•? ";
#else
        description += "A remarkably nondescript person.  How odd!";
#endif
        break;

    case MONS_GIANT_ANT:
#ifdef JP
        description += "µ∂¿Ã ¿÷¥¬ ¡˝∞‘∞° ¥ﬁ∏∞ ∞À¿∫ ∞≥πÃ¥Ÿ. ≈´ ∞≥ ¡§µµ¿« ≈©±‚¥Ÿ.";
#else
        description += "A black ant with poisonous pincers,"
            " about the size of a large dog.";
#endif
        break;

    case MONS_SOLDIER_ANT:
#ifdef JP
        description += "≈´ ≈Œ∞˙ ªÁæ««— ƒß¿ª ∞°¡¯ ∞≈¥Î«— ∞≥πÃ¥Ÿ. ";
#else
        description += "A giant ant with large mandibles and a vicious sting.";
#endif
        break;

    case MONS_QUEEN_ANT:
#ifdef JP
        description += "µŒ≤®øÓ ≈∞∆æ¡˙∑Œ µ—∑ØΩŒ¿Œ ∫Œ«ÆæÓ ø¿∏• ∞Ô√Ê¿Ã¥Ÿ. $"
                       "¿Ã¡¶ ¥ÁΩ≈¿∫ ∏µÁ ∞≥πÃµÈ¿Ã æÓµº≠ ø‘¥¬¡ˆ æÀ∞‘ µ«æ˙¥Ÿ! ";
#else
        description += "A bloated insect, covered in thick chitinous armour."
            "Now you know where all those ants keep coming from!";
#endif
        break;

    case MONS_ANT_LARVA:
#ifdef JP
        description += "∞≥πÃ ªı≥¢¥Ÿ. ±Õø±¡ˆ æ æ∆? ";
#else
        description += "A baby ant. Isn't it cute?";
#endif
        break;

    case MONS_GIANT_BAT:
#ifdef JP
        description += "≈©∞Ì ∞À¿∫ π⁄¡„¥Ÿ. ";
#else
        description += "A huge black bat.";
#endif
        break;

    case MONS_CENTAUR:
    case MONS_CENTAUR_WARRIOR:
#ifdef JP
        description += "ªÛ√º¥¬ ¿Œ∞£¿Ã∞Ì «œ√º¥¬ ∏ª¿Œ »•º∫¡æ¿Ã¥Ÿ. ";
#else
        description += "A hybrid with the torso of a "
            "human atop the body of a large horse. ";
#endif
        if (class_described == MONS_CENTAUR_WARRIOR)
#ifdef JP
            description += "$¿Ã∞Õ¿∫ ∞≠«œ∞Ì ∞¯∞›¿˚¿∏∑Œ ∫∏¿Œ¥Ÿ. " ;
#else
            description += "It looks strong and aggressive. ";
#endif
        break;

    case MONS_YAKTAUR:
    case MONS_YAKTAUR_CAPTAIN:
#ifdef JP
        description += "ºæ≈∏øÏ∏£øÕ ¥‡æ“¡ˆ∏∏ π›¿∫ æﬂ≈©¥Ÿ. ";
#else
        description += "Like a centaur, but half yak. ";
#endif
        if (class_described == MONS_YAKTAUR_CAPTAIN)
#ifdef JP
            description += "$¿Ã∞Õ¿∫ ∏≈øÏ ∞≠«œ∞Ì ∞¯∞›¿˚¿∏∑Œ ∫∏¿Œ¥Ÿ. ";
#else
            description += "It looks very strong and aggressive. ";
#endif
        break;

    case MONS_RED_DEVIL:
#ifdef JP
        description += "¿Ã ∫”¿∫ªˆ¿« æ«∏∂¥¬ ¿Œ∞£∫∏¥Ÿ ¡∂±› ¿€¿∫ ≈∞∑Œ $"
                       "∏ˆ¿∫ ±Ÿ¿∞¡˙ø° ∞°Ω√øÕ ª‘∑Œ µ§ø©¿÷¥Ÿ. $"
                       "æÓ±˙ø°¥¬ µŒ ∞≥¿« ¿€¿∫ ≥Ø∞≥∞° º⁄æ∆¿÷¥Ÿ. ";
#else
        description += "The Red Devil is slightly shorter than a human, "
            "but muscular and covered in spikes and horns. Two "
            "short wings sprout from its shoulders.";
#endif
        break;

    case MONS_ROTTING_DEVIL:
#ifdef JP
        description += "∫Œ∆–«œ∞Ì ¿÷¥¬ ªÁ∂˜¿« «¸≈¬∏¶ «— ¡∏¿Á¥Ÿ. ";
#else
        description += "A hideous decaying form.";
#endif
        if (you.species == SP_GHOUL)
#ifdef JP
            description += "$∏≈øÏ ∏‘¿Ω¡˜Ω∫∑ØøÓ ≥øªı∞° ≥≠¥Ÿ! ";
#else
            description += "$It smells great!";
#endif
        else if (you.species != SP_MUMMY)
#ifdef JP
            description += "$¿Ã∞Õ¿∫ æ«√Î∏¶ «≥±‰¥Ÿ. ";
#else
            description += "$It stinks.";
#endif
        break;

    case MONS_HAIRY_DEVIL:
#ifdef JP
        description +=
            "∞•ªˆ¿« ≈–∑Œ µ§»˘ ¿€¿∫ ¿Œ∞£«¸¿« æ«∏∂¥Ÿ. $"
            "¡∂Ω…«œµµ∑œ - ∫≠∑Ë¿Ã ¿÷¿ª¡ˆµµ ∏∏•¥Ÿ! ";
#else
        description += "A small humanoid demon covered in brown hair. "
            "Watch out - it may have fleas!";
#endif
        break;

    case MONS_ICE_DEVIL:
#ifdef JP
        description += "π›¬¶∞≈∏Æ¥¬ æÛ¿Ω¿∏∑Œ µ§»˘ ¿Œ∞£ ªÁ¿Ã¡Ó¿« æ«∏∂¥Ÿ. ";
#else
        description += "A man-sized demon covered in glittering ice.";
#endif
        break;

    case MONS_BLUE_DEVIL:
#ifdef JP
        description += "¿ÃªÛ«œ∞Ì ±‚±´«— «™∏•ªˆ¿« ¡∏¿Á¥Ÿ. ¬˜∞°øˆ ∫∏¿Œ¥Ÿ. ";
#else
        description += "A strange and nasty blue thing. It looks cold.";
#endif
        break;

    case MONS_IRON_DEVIL:
#ifdef JP
        description += "π´º≠øÓ ±›º” ««∫Œ∏¶ ∞°¡¯ ¿Œ∞£«¸¿« ¡∏¿Á¥Ÿ  ";
#else
        description += "A hideous humanoid figure with metal skin.";
#endif
        break;

    case MONS_ETTIN:
#ifdef JP
        description +=
            "µŒ ∞≥¿« ∏”∏Æ∞° ¥ﬁ∏∞ ∞≈¥Î«— ¿Œ∞£«¸ ª˝π∞√º¥Ÿ. "
            "¥Î√º∑Œ µŒ ∞≥¿« π´±‚∏¶ µÈ∞Ì ¿÷¥¬ ∞Õ¿Ã ∏Ò∞›µ«¥¬µ• "
            "∏”∏ÆµÈ¿Ã º≠∑Œ ΩŒøÏ¡ˆ æ ±‚ ¿ß«‘¿ŒµÌ «œ¥Ÿ. ";
#else
        description += "A large, two headed humanoid. Most often seen "
            "wielding two weapons, so that the heads will have one less "
            "thing to bicker about.";
#endif
        break;

    case MONS_FUNGUS:
#ifdef JP
        description +=
            "µ¢æÓ∏Æ¡¯ »∏ªˆ ±’∑˘¥Ÿ.  $"
            "æÓµŒøÓ ¡ˆ«œ ¥¯¡Øø°º≠ ¿ﬂ ¿⁄∂ı¥Ÿ. ";
#else
        description += "A lumpy grey fungus, "
            "growing well in the dank underground dungeon.";
#endif
        break;

    case MONS_GOBLIN:
#ifdef JP
        description += "¿€∞Ì ∏¯ª˝±‚∞Ì ¿˚¿«∏¶ ∞°¡¯ ¿Œ∞£«¸ ª˝π∞¿Ã¥Ÿ. ";
#else
        description += "A race of short, ugly and unfriendly humanoids.";
#endif
        break;

    case MONS_HOUND:
#ifdef JP
        description += "π´º≠øÓ ªÁ≥…∞≥¥Ÿ. ";
#else
        description += "A fearsome hunting dog.";
#endif
        break;

    case MONS_HELL_HOUND:
#ifdef JP
        description += "∫“≈∏¥¬ ¥´¿ª «œ∞Ì ¿‘ø°º≠ ø¨±‚∏¶ ª’æÓ≥ª¥¬ ≈©∞Ì ∞À¿∫ ∞≥¥Ÿ.  ";
#else
        description += "A huge black dog, with glowing red eyes and "
            "smoke pouring from its fanged mouth.";
#endif
        break;

    case MONS_WAR_DOG:
#ifdef JP
        description +=
            "ªÏ¿∞¿ª ¿ß«ÿ »∆∑√µ» ªÁ≥™øÓ ∞≥¥Ÿ.$"
            "∏Ò¿∫ ¥‹¥‹«— ∞°Ω√¥ﬁ∏∞ ∞≥∏Ò∞…¿Ã∑Œ ∫∏»£µ«∞Ì ¿÷¥Ÿ. ";
#else
        description += "A vicious dog, trained to kill."
            "Its neck is protected by massive spiked collar.";
#endif
        break;

    case MONS_IMP:
#ifdef JP
        description += "¿€∞Ì ∏¯ª˝±‰ «œ±ﬁ æ«∏∂¥Ÿ. ";
#else
        description += "A small, ugly minor demon.";
#endif
        break;

    case MONS_JACKAL:
#ifdef JP
        description +=
            "¿€¿∫ ∞≠æ∆¡ˆ∏¶ ¥‡¿∫, Ω‚¿∫ ∞Ì±‚∏¶ ∏‘¥¬ ª˝π∞¿Ã¥Ÿ. $"
            "¡ˆ«œ∏¶ πË»∏«œ∏Á ∏‘¿ª ∞Õ¿ª √£æ∆¥Ÿ¥—¥Ÿ. ";
#else
        description += "A small, dog-like scavenger. Packs of these creatures "
            "roam the underworld, searching for carrion to devour.";
#endif
        break;

    case MONS_KILLER_BEE:
#ifdef JP
        description +=
            "π›∫π«ÿº≠ ¬Ó∏¶ ºˆ ¿÷¥¬ ƒß¿ª ∞°¡¯ ∞≈¥Î«— π˙¿Ã¥Ÿ. ";
#else
        description += "A giant bee, bearing a deadly barb which can sting "
            "repeatedly.";
#endif
        break;

    case MONS_QUEEN_BEE:
#ifdef JP
        description +=
            "ªÏ¿Œπ˙∫∏¥Ÿ ¥ı ≈©∞Ì ¿ß«Ë«ÿ∫∏¿Ã¥¬ ø‹∞¸¿ª ∞°¡¯ π˙¿Ã¥Ÿ. $"
            "¡ˆ±› ¿Ã ª˝π∞¿∫ ¥ÁΩ≈¿Ã ±◊¿« π˙¡˝ø°º≠ ≥™∞°±Ê ø¯«œ∞Ì ¿÷¥Ÿ!";
#else
        description += "Even larger and more dangerous-looking than its "
            "offspring, this creature wants you out of its hive. Now!";
#endif
        break;

    case MONS_BUMBLEBEE:
#ifdef JP
        description += "∏≈øÏ ≈©∞Ì ∂◊∂◊«— ≈– ∏π¿∫ π˙¿Ã¥Ÿ.  ";
#else
        description += "A very large and fat hairy bee.";
#endif
        break;

    case MONS_MANTICORE:
#ifdef JP
        description +=
            "∏≈øÏ √ﬂæ««— ¿‚¡æ¿Ã¥Ÿ. ¿Œ∞£¿« ∏”∏ÆøÕ ªÁ¿⁄¿« ∏ˆ, ±◊∏Æ∞Ì $"
            "π⁄¡„¿« ≥Ø∞≥∏¶ ¥‡¿∫ ∞≈¥Î«— ≥Ø∞≥∏¶ ∞°¡ˆ∞Ì ¿÷¥Ÿ. $"
            "≤ø∏Æø£ ∞°Ω√∞° π⁄«Ù¿÷¿∏∏Á, ªÁ≥…∞®¿ª «‚«ÿ πﬂªÁµ»¥Ÿ.  ";
#else
        description += "A hideous cross-breed, bearing the features of a "
            "human and a lion, with great bat-like wings. Its tail "
            "bristles with spikes, which can be loosed at potential prey.";
#endif
        break;

    case MONS_NECROPHAGE:
#ifdef JP
        description += "∞°¿Â ªÁæ««œ∞Ì ∫Œ¡§«— æµ•µÂ∑Œ $"
                       "¿Ã ¡∏¿Á¥¬ Ω‚æÓ∞°¥¬ Ω√√º∑Œ∫Œ≈Õ ∏∏µÈæÓ¡¯¥Ÿ. $"
                       "±◊µÈ¿∫ ø™∫¥∞˙ ∫Œ∆–∏¶ ∆€∂ﬂ∏Æ±‚ ¿ß«ÿ ¡∏¿Á«œ∏Á $"
                       "¥Ÿ∏• ¡∏¿Á¿« Ω‚æÓ∞°¥¬ Ω√√º∑Œ∫Œ≈Õ »˚¿ª æÚ¥¬¥Ÿ. ";
#else
        description += "A vile undead creation of the most unholy necromancy,"
            " these creatures are made from the decaying corpses "
            "of humanoid creatures.  They exist to spread disease "
            "and decay, and gain power from the decaying corpses "
            "of other beings.";
#endif
        break;

    case MONS_GHOUL:
#ifdef JP
        description +=
            "ªÁæ««— ∞≠∑…º˙ ∏∂π˝¿« »˚¿∏∑Œ ∫Œ∆–«— Ω√√º∑Œ∫Œ≈Õ ∏∏µÈæÓ¡¯ $"
            "¿Œ∞£«¸¿« æµ•µÂ¥Ÿ. $"
            "±◊µÈ¿∫ ø™∫¥∞˙ ∫Œ∆–∏¶ ∆€∂ﬂ∏Æ±‚ ¿ß«ÿ ¡∏¿Á«œ∏Á $"
            "∞≠∑…º˙ªÁøÕ ∞∞¿Ã Ω‚æÓ∞°¥¬ Ω√√º∑Œ∫Œ≈Õ »˚¿ª æÚ¥¬¥Ÿ. ";
#else
        description += "An undead humanoid creature created from the decaying "
            "corpse by some unholy means of necromancy. It "
            "exists to spread disease and decay, and gains power"
            "from the decaying corpses same way as necrophage does.";
#endif
        break;

    case MONS_ORC:
#ifdef JP
        description +=
            "ø¿≈©¥¬ ªÁæ««— ¡ˆ«œ ¡æ¡∑¿∏∑Œ, µ≈¡ˆøÕ ¿Œ∞£, ±◊ π€¿« $"
            "∫“ƒË«— ª˝π∞∞˙¿« »•«’π∞¿Ã¥Ÿ. ";
#else
        description += "An ugly subterranean race, orcs combine the"
            " worst features of humans, pigs, and several"
            " other unpleasant creatures.";
#endif
        break;

    case MONS_ORC_KNIGHT:
#ifdef JP
        description +=
            "¡ﬂ∞©¿ª ¿‘¿∫ ø¿≈©¥Ÿ. ∞˙∞≈¿« ¿¸≈ıø°º≠ æÚ¿∫ »‰≈Õ∑Œ µ§ø©¿÷¥Ÿ. $";
#else
        description += "A heavily armoured orc, covered in scars from many "
            "past battles.";
#endif
        break;

    case MONS_ORC_PRIEST:
#ifdef JP
        description +=
            "∞Ì¥Î¿« ¿‹¿Œ«— ø¿≈©¿« Ω≈ø°∞‘ ∫π¡æ«œ¥¬ ø¿≈©¥Ÿ. $"
            "±◊¥¬ ±‚±´«— ±‚µµπÆ¿ª ø‹øÏ∞Ì ¿÷¥Ÿ.  $"
            "±◊ ±‚µµπÆ¿Ã Ω≈ø°∞‘ ¥Í¡ˆ æ ±‚∏¶ πŸ∂Ûæﬂ «“ ∞Õ¿Ã¥Ÿ. ";
#else
        description += "A servant of the ancient and cruel gods of the orcs,"
            " dressed in long robe. he's mumbling some strange prayers. "
            "Hope that they will remain unheard.";
#endif
        break;

    case MONS_ORC_HIGH_PRIEST:
#ifdef JP
        description += "ø¿≈©¿« Ω≈¿« ∞Ì¿ß º∫¡˜¿⁄¥Ÿ. ";
#else
        description += "An exalted servant of the orc god.";
#endif
        break;

    case MONS_ORC_SORCERER:
#ifdef JP
        description += "¡ˆø¡ø°º≠ ø¬ ∏∂π˝¿« »˚¿ª ªÁøÎ«œ¥¬ ø¿≈©¥Ÿ. ";
#else
        description += "An orc who draws magical power from Hell.";
#endif
        break;

    case MONS_ORC_WARLORD:
#ifdef JP
        description += "≈©∞Ì ¿ÃªÛ«œ∞‘ ª˝±‰ ø¿≈©¥Ÿ. ";
#else
        description += "A very large and strong looking orc.";
#endif
        break;

    case MONS_ORC_WARRIOR:
#ifdef JP
        description +=
            "∞©ø ¿ª ¿‘¿∫ ø¿≈©¥Ÿ. ¥Ÿ∏• ª˝π∞¿ª µ—∑Œ ¬…∞≥πˆ∏Æ¥¬ ¿œø° $"
            "≥Î∑√«“ ∞Õ¿Ã¥Ÿ. ";
#else
        description += "An armoured orc, obviously experienced in the ways of "
            "hacking other creatures apart.";
#endif
        break;

    case MONS_ORC_WIZARD:
#ifdef JP
        description +=
            "ø¿≈©µÈ¿« ¥Î∫Œ∫–¿∫ ∏€√ª«œ¡ˆ∏∏, ∂ß∑Œ¥¬ ∏∂π˝¥…∑¬¿ª πﬂ¿¸Ω√≈∞¥¬ $"
            "ø¿≈©µµ ¿÷¥Ÿ. ";
#else
        description += "While orcs are generally quite stupid, occasionally"
            " one develops an aptitude for magic.";
#endif
        break;

    case MONS_PHANTOM:
#ifdef JP
        description += "π›≈ı∏Ì«— ¿Œ∞£ «¸≈¬¿« æµ•µÂ øµ»•¿Ã¥Ÿ. ";
#else
        description += "A transparent man-like undead spirit.";
#endif
        break;

    case MONS_RAT:
#ifdef JP
        description +=
            "∞°»§«— »Ø∞Êø°º≠ ¿⁄∂Û, ≈©∞Ì ≥≠∆¯«œ∞‘ µ» ¡„¥Ÿ. ";
#else
        description += "Rats which have grown large and aggressive in "
            "the pestilential dungeon environment.";
#endif
        break;

    case MONS_GREY_RAT:
#ifdef JP
        description += "≈©∞Ì »∏ªˆ¿Œ ¡„¥Ÿ. ";
#else
        description += "A very large grey rat.";
#endif
        break;

    case MONS_GREEN_RAT:
#ifdef JP
        description +=
            "∏≈øÏ ƒø¥Ÿ∂ı ¡„¥Ÿ. $"
            "¿ÃªÛ«— ≥Ïªˆ¿« ≈–∞˙ ∞°¡◊¿∏∑Œ µ⁄µ§ø©¿÷¥Ÿ. ";
#else
        description += "A very large rat, with hair and skin of a "
            "most peculiar green colour.";
#endif
        break;

    case MONS_ORANGE_RAT:
#ifdef JP
        description +=
            "∞≈¥Î«— ¡„¥Ÿ. ±‚∫– ≥™ª€ »§≈ıº∫¿Ã¿« ∞°¡◊¿∫ ¡÷»≤ªˆ¿Ã∏Á$"
            "ªÁæ««— ø°≥ ¡ˆ∑Œ ∫˚≥™∞Ì ¿÷¥Ÿ. ";
#else
        description += "A huge rat, with weird knobbly orange skin."
            "It glows with unholy energies. ";
#endif
        break;

    case MONS_SCORPION:
#ifdef JP
        description +=
            "≈©∞Ì ∞À¿∫ ¿¸∞•¿Ã¥Ÿ. ∏ˆ¿∫ µŒ≤®øÓ ø‹∞Ò∞›¿∏∑Œ ΩŒø©¿÷∞Ì $"
            "≤ø∏Æø°¥¬ ∏Õµ∂º∫¿« ƒß¿Ã ¥ﬁ∑¡¿÷¥Ÿ. ";
#else
        description += "A giant black scorpion, its body covered in thick"
            " armour plating, and its tail tipped by a nasty "
            "venomous sting.";
#endif
        break;

/* ******************************************************************
// the tunneling worm is no more ...
// not until it can be re-implemented safely {dlb}
    case MONS_TUNNELING_WORM:
    case MONS_WORM_TAIL:
        description += "A gargantuan worm, its huge maw capable of crushing rock into dust with little trouble.";
        break;
****************************************************************** */

    case MONS_BRAIN_WORM:
#ifdef JP
        description += "≤ˆ¿˚∞≈∏Æ¥¬ ¿⁄¡÷ªˆ π˙∑π¥Ÿ. ∏”∏Æ∞° ∞≈¥Î«œ∞‘ ∆ÿ√¢«ÿ ¿÷¥Ÿ.  ";
#else
        description += "A slimy mauve worm with a greatly distended head.";
#endif
        break;

    case MONS_LAVA_WORM:
#ifdef JP
        description += "øÎæœ¿ª «Ïæˆƒ°¥¬ ¿ß«Ë«— π˙∑π¥Ÿ. ";
#else
        description += "A vicious red worm which swims through molten rock.";
#endif
        break;

    case MONS_SPINY_WORM:
#ifdef JP
        description +=
            "≈©∞Ì ∞À¿∫ªˆ¿« π˙∑π¥Ÿ. ∏π¿∫ ∏∂µ∞° ¿÷¥¬ ∏ˆ¿∫ ∞°Ω√∞° º⁄¿∫ ≈∞∆æ¡˙¿« $"
            "ø‹∞Ò∞›¿∏∑Œ ΩŒø©¿÷¥Ÿ.  $"
            "≥Øƒ´∑ŒøÓ ¿Ãª°¿Ã ¿÷¥¬ ¿‘¿∏∑Œ∫Œ≈Õ ªÍº∫¿« µ∂¿Ã ∂≥æÓ¡ˆ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A great black worm, its many-segmented body covered "
            "in spiky plates of chitinous armour. Acidic venom drips "
            "from its toothy maw.";
#endif
        break;

    case MONS_SWAMP_WORM:
#ifdef JP
        description +=
            "≤ˆ¿˚∞≈∏Æ¥¬ ≈´ π˙∑π¥Ÿ. $"
            "¥ı∑ØøÓ ¥À¿ª «Ïæˆƒ°¥¬µ•ø° ∆Ø»≠µ«æÓ ¿÷¥Ÿ. ";
#else
        description += "A large slimy worm, adept at swimming through the "
            "muck of this foul swamp.";
#endif
        break;

    case MONS_WORM:
#ifdef JP
        description += "∫Ò¡§ªÛ¿˚¿∏∑Œ ∞≈¥Î«— ¿Ãª°¿Ã ¥ﬁ∏∞ ∞≈¥Î«— π˙∑π¥Ÿ. ";
#else
        description += "A giant worm, with unusually large teeth.";
#endif
        break;

    case MONS_UGLY_THING:
#ifdef JP //∞À≈‰ - ±∏¿Ã - ¿°? øÏ¿°? øÏøÌ? ¿˚¥Á«—∞…∑Œ...
        description += "√ﬂæ««— ¡∏¿Á¥Ÿ. øÏ¿°. ";
#else
        description += "An ugly thing. Yuck.";
#endif
        break;

    case MONS_VERY_UGLY_THING:
#ifdef JP
        description += "¡§∏ª∑Œ √ﬂæ««— ¡∏¿Á¥Ÿ. øÏ¿°¿°. ";
#else
        description += "A very ugly thing. Double yuck.";
#endif
        break;

    case MONS_FIRE_VORTEX:
#ifdef JP
        description += "º“øÎµπ¿Ãƒ°¥¬ ∫“±Ê¿« ±∏∏ß¿Ã¥Ÿ. ";
#else
        description += "A swirling cloud of flame.";
#endif
        break;

    case MONS_SPATIAL_VORTEX:
#ifdef JP //∞À≈‰ - ±∏¿Ã - ¿Ã∞« ¿ﬂ ∏∏£∞⁄±∫ø‰.
        description += "πÃƒ£µÌ¿Ã º¯∞£¿Ãµø«œ¥¬ ∞¯∞£ ±∏¡∂¿« µ⁄∆≤∏≤¿Ã¥Ÿ.";
#else
        description += "A crazily shifting twist in the fabric of reality.";
#endif
        break;

    case MONS_ABOMINATION_SMALL:
        description +=
#ifdef JP
            "∫∏¥¬∞Õ¡∂¬˜µµ π´º≠øÓ ∏Ω¿¿ª «œ∞Ì¿÷¥Ÿ. ∏∂π˝¿˚¿Œ ∞¯¡§¿∏∑Œ √¢¡∂ ∂«¥¬ º“»Øµ«æ˙¥Ÿ. ";
#else
            "A hideous form, created or summoned by some arcane process.";
#endif
        break;

    case MONS_ABOMINATION_LARGE:
#ifdef JP
        description +=
            "≈©∞Ì π´º≠øÓ ∏Ω¿¿ª «œ∞Ì¿÷¥Ÿ. $"
            "∫Òπ–Ω∫∑± ∞¯¡§¿∏∑Œ √¢¡∂ ∂«¥¬ º“»Øµ«æ˙¥Ÿ. ";
#else
        description += "A huge and hideous form, created or summoned "
            "by some arcane process.";
#endif
        break;

    case MONS_YELLOW_WASP:
#ifdef JP
        description +=
            "∞≈¥Î«— ∏ªπ˙¿Ã¥Ÿ. $"
            "µŒ≤®øÓ ≥Î∂ı ≈∞∆æ¡˙¿« ø‹∞Ò∞›¿∏∑Œ ΩŒø©¿÷¥Ÿ. ";
#else
        description += "A giant wasp covered with thick plates of yellow "
            "chitinous armour.";
#endif
        break;

    case MONS_RED_WASP:
#ifdef JP
        description += "∞≈¥Î«— ∫”¿∫ ∏ªπ˙¿Ã¥Ÿ. ¿ß«Ë«ÿ∫∏¿Ã¥¬ µ∂ƒß∞°Ω√∏¶ Ω¥Ÿ.  ";
#else
        description += "A huge red wasp with a viciously barbed stinger.";
#endif
        break;

    case MONS_ZOMBIE_SMALL:
#ifdef JP
        description += "∞≠∑…º˙∑Œ ¿œ¿∏ƒ—¡¯ Ω√√º¥Ÿ. ";
#else
        description += "A corpse raised to undeath by necromancy. ";
#endif
        break;
    case MONS_ZOMBIE_LARGE:
#ifdef JP
        description += "∞≠∑…º˙∑Œ ¿œ¿∏ƒ—¡¯ ∞≈¥Î«— Ω√√º¥Ÿ. ";
#else
        description += "A large corpse raised to undeath by necromancy. ";
#endif
        break;

    case MONS_SIMULACRUM_LARGE:
    case MONS_SIMULACRUM_SMALL:
#ifdef JP
        description +=
            "∏ÛΩ∫≈Õ¿« æÛ¿Ω ∫π¡¶«∞¿Ã¥Ÿ. $"
            "∞≠∑…º˙∑Œ ¡∂¡æµ«∞Ì ¿÷¥Ÿ.  ";
#else
        description += "An ice replica of a monster, that's animated by "
            "the powers of necromancy. ";
#endif
        break;

    case MONS_CYCLOPS:
#ifdef JP
        description +=
            "ø‹¥´π⁄¿Ã ∞≈¿Œ¿Ã¥Ÿ. $"
            "¥´¿Ã «œ≥™π€ø° æ¯¿Ωø°µµ ∫“±∏«œ∞Ì $"
            "π´º≠øÔ ¡§µµ∑Œ ¡§»Æ«œ∞‘ πŸ¿ß∏¶ ≥ª¥¯¡¯¥Ÿ. ";
#else
        description += "A giant with one eye in the centre of its forehead."
            " Despite their lack of binocular vision, cyclopes "
            "throw boulders with fearsomely accuracy.";
#endif
        break;

    case MONS_DRAGON:
#ifdef JP
        description +=
            "∞≈¥Î«— ∆ƒ√Ê∑˘¥Ÿ. $"
            "µŒ ∞≥¿« ∞≈¥Î«— π⁄¡„≥Ø∞≥∏¶ ¥‡¿∫ ≥Ø∞≥øÕ µŒ≤®øÓ √ ∑œ ∫Ò¥√∑Œ µ§ø©¿÷¥Ÿ.$"
            "º€∞˜¥œ∞° ≥≠ ≈Œ¿∏∑Œ∫Œ≈Õ ø¨±‚∞° ¿œ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A great reptilian beast, covered in thick green "
            "scales and with two huge bat-like wings. Little trails "
            "of smoke spill from its toothy maw.";
#endif
        break;

    case MONS_GOLDEN_DRAGON:
#ifdef JP
        description += "∫˚≥™¥¬ »≤±› ∫Ò¥√∑Œ µ§¿Œ ∞≈¥Î«— øÎ¿Ã¥Ÿ. ";
#else
        description += "A great dragon covered in shining golden scales. ";
#endif
        break;

    case MONS_ICE_DRAGON:
        description +=
#ifdef JP
            "∫∏≈Î øÎ ∞∞¡ˆ∏∏, «œæÈ∞Ì ≥√±‚ø° ΩŒø©¿÷¥Ÿ. ";
#else
            "Like a normal dragon, only white and covered in frost.";
#endif
        break;

    case MONS_IRON_DRAGON:
#ifdef JP
        description += "∏≈øÏ π´∞≈øˆº≠ ≥Ø ºˆ æ¯¥¬ øÎ¿Ã¥Ÿ.  ";
#else
        description += "A very heavy and apparently flightless dragon.";
#endif
        break;

    case MONS_MOTTLED_DRAGON:
#ifdef JP
        description += "æÛ∑Ë¥˙∑Ë«— ∫Ò¥√¿Ã ¿÷¥¬ ¿€¿∫ øÎ¿Ã¥Ÿ. ";
#else
        description += "A small dragon with strangely mottled scales.";
#endif
        break;

    case MONS_QUICKSILVER_DRAGON:
#ifdef JP
        description +=
            "∏≈øÏ ±Ê∞Ì ≤Ÿ∫“≤Ÿ∫“«— øÎ¿Ã¥Ÿ.  $"
            "∏Ò∞˙ ≤ø∏Æø‹ø°¥¬ æ∆π´∞Õµµ æ¯æÓ∫∏¿Œ¥Ÿ. $"
            "««∫Œ¥¬ ≥Ï¿∫ ºˆ¿∫√≥∑≥ ∫˚≥™∏Á, ¡÷µ’¿Ã∑Œ∫Œ≈Õ ∏∂π˝¿˚¿Œ »˚¿Ã ¥¿≤∏¡¯¥Ÿ.  ";
#else
        description += "A long and sinuous dragon, seemingly more neck and "
            "tail than anything else. Its skin shines like molten mercury, "
            "and magical energies arc from its pointed snout.";
#endif
        break;

    case MONS_SHADOW_DRAGON:
#ifdef JP
        description += "∞≈¥Î«— ±◊∏≤¿⁄¥Ÿ. ªÁæ««‘∞˙ ¡◊¿Ω¿« ±‚øÓ¿ª ≥ªª’∞Ì ¿÷¥Ÿ. ";
#else
        description += "A great shadowy shape, radiating evil and death.";
#endif
        break;

    case MONS_SKELETAL_DRAGON:
#ifdef JP
        description +=
            "«¯ø¿Ω∫∑± ∞≈¥Î«— æµ•µÂ¥Ÿ. $"
            "π´ºˆ«— ∫Œ∑Ø¡¯ øÎ¿« ª¿∑Œ ±∏º∫µ«æÓ ¿÷¥Ÿ. ";
#else
        description += "A huge undead abomination, pieced together from "
            "the broken bones of many dragons.";
#endif
        break;

    case MONS_STEAM_DRAGON:
#ifdef JP
        description +=
            "ªÛ¥Î¿˚¿∏∑Œ ¿€¿∫ »∏ªˆ¿« øÎ¿Ã¥Ÿ. $"
            "¡÷µ’¿Ãø°º≠ ¡ı±‚∏¶ ª’æÓ¥Î∞Ì ¿÷¥Ÿ. ";
#else
        description += "A relatively small grey dragon, with steam pouring "
            "from its mouth.";
#endif
        break;

    case MONS_STORM_DRAGON:
#ifdef JP
        description +=
            "≈©∞Ì ∏≈øÏ ∞≠∑¬«— øÎ¿Ã¥Ÿ. $"
            "∞≈¥Î«— ≥Ø∞≥¿« øÚ¡˜¿”ø° µ˚∂Û Ω∫∆ƒ≈©∞° ¿œ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A huge and very powerful dragon. "
            "Sparks crackle along its enormous scaly wings.";
#endif
        break;

    case MONS_SWAMP_DRAGON:
#ifdef JP
        description +=
            "≤ˆ¿˚∞≈∏Æ¥¬ øÎ¿Ã¥Ÿ. ¥À ¡¯»Î¿∏∑Œ µ§ø©¿÷¥Ÿ. $"
            "¡÷µ’¿Ãø°º≠¥¬ µ∂º∫¿« ∞°Ω∫∞° ª’æÓ¡Æ ≥™ø¬¥Ÿ. ";
#else
        description += "A slimy dragon, covered in swamp muck. "
            "Poisonous gasses dribble from its snout.";
#endif
        break;

    case MONS_SERPENT_OF_HELL:
#ifdef JP
        description += "∞≈¥Î«— ∫”¿∫ªˆ¿∏∑Œ ∫˚≥™¥¬ øÎ¿Ã¥Ÿ. ∫“≈∏¥¬ «Ô∆ƒ¿ÃæÓ∏¶ π∞∞Ì ¿÷¥Ÿ. ";
#else
        description += "A huge red glowing dragon, burning with hellfire. ";
#endif
        break;

    case MONS_SWAMP_DRAKE:
#ifdef JP
        description += "¿€∞Ì ≤ˆ¿˚∞≈∏Æ¥¬ øÎ¿Ã¥Ÿ. ¥À ¡¯»Î¿∏∑Œ µ§ø©¿÷¥Ÿ. $";
#else
        description += "A small and slimy dragon, covered in swamp muck. ";
#endif
        if (you.species != SP_MUMMY)
#ifdef JP
            description += "≥øªı∞° ∞Ìæ‡«œ¥Ÿ. ";
#else
            description += "It smells horrible.";
#endif
        break;

    case MONS_FIREDRAKE:
#ifdef JP
        description += "ø¨±‚ ±∏∏ß¿ª ª’¥¬ ¿€¿∫ øÎ¿Ã¥Ÿ. ";
#else
        description += "A small dragon, puffing clouds of smoke.";
#endif
        break;

    case MONS_TWO_HEADED_OGRE:
#ifdef JP
        description +=
            "∏”∏Æ∞° µ— ¥ﬁ∏∞ ∞≈¥Î«— ø¿∞≈¥Ÿ. $"
            "≈´ æÁ¬  º’ ∏µŒø° π´±‚∏¶ ¡Ê ºˆ ¿÷¥Ÿ. ";
#else
        description += "A huge ogre with two heads on top of a "
            "bloated ogre body. It is capable of holding a weapon "
            "in each giant hand.";
#endif
        break;

    case MONS_FIEND:
#ifdef JP
        description +=
            "¡ˆø¡¿« ±∏º∫ø¯ ¡ﬂø°º≠ ∞°¿Â π´º≠øÓ ¡∏¿Á¡ﬂ¿« «œ≥™¥Ÿ. $"
            "«Ô∆ƒ¿ÃæÓø° ΩŒ¿Œ ∞≈¥Î«œ∞Ì ∞≠∑¬«— æ«∏∂¥Ÿ. $"
            "∫Ò¥√¿Ã µ§¿Œ ≈´ ≥Ø∞≥∞° ¿÷¥Ÿ. ";
#else
        description += "One of the most fearsome denizens of any Hell. "
            "A huge and powerful demon wreathed in hellfire,"
            " with great scaly wings.";
#endif
        break;

    case MONS_ICE_FIEND:
#ifdef JP
        description +=
            "¡ˆø¡¿« ±∏º∫ø¯ ¡ﬂø°º≠µµ ∞°¿Â π´º≠øÓ ¡∏¿Á ¡ﬂ «œ≥™¥Ÿ. $"
            "æ∆¿ÃΩ∫ «…µÂ¥¬ ∞≈¥Î«— æÛ¿Ω¿« «¸ªÛ¿∏∑Œ, º≠∏ÆøÕ æÛæÓ∫Ÿ∞‘ «œ¥¬ ∞¯±‚∞° µ—∑ØΩŒ∞Ì ¿÷¥Ÿ. ";
#else
        description += "One of the most terrible denizens of the "
            "many Hells, the Ice Fiend is a huge icy figure, "
            "covered in frost and wreathed in freezing air.";
#endif
        break;

    case MONS_SHADOW_FIEND:
#ifdef JP
        description +=
            "¡ˆø¡¿« ±∏º∫ø¯µÈ ¡ﬂø°º≠ ∞°¿Â π´º≠øÓ ¡∏¿Á ¡ﬂ «œ≥™¥Ÿ.$"
            "¿Ã π´º≠øÓ ¡∏¿Á¥¬ ≤Ÿ∫“∞≈∏Æ¥¬ ∞≈¥Î«— ±◊∏≤¿⁄∑Œ ≥™≈∏≥™¡ˆ∏∏ $"
            "∞°≤˚ ±◊ æ»ø° ∞≈¥Î«— ª‘¥ﬁ∏∞ «ÿ∞Ò¿« ∏Ω¿¿Ã ∏Ò∞›µ«±‚µµ «—¥Ÿ. ";
#else
        description += "One of the most terrible denizens of the many Hells, "
            "this horrible being appears as a great mass of "
            "writhing shadows which occasionally reveal a huge, "
            "horned skeleton.";
#endif
        break;

    case MONS_GIANT_SPORE:
#ifdef JP
        description +=
            "∆¯πﬂ«œ±‚ Ω¨øÓ ∆˜¿⁄∑Œ µ» ∞¯¿Ã¥Ÿ. $"
            "»§≈ıº∫¿Ã¿« ¡Ÿ±‚∑Œ µ§ø©¿÷¥Ÿ. ";
#else
        description += "A volatile floating ball of spores, "
            "covered in knobbly rhizome growths.";
#endif
        break;

    case MONS_HOBGOBLIN:
#ifdef JP
        description += "¥ı ≈©∞Ì ¥ı ∞≠«— ∞Ì∫Ì∏∞¿« ƒ£√¥¿Ã¥Ÿ. ";
#else
        description += "A larger and stronger relatives of goblins.";
#endif
        break;

    case MONS_ICE_BEAST:
        description +=
#ifdef JP
            "π´º≠øÓ ∏ÛΩ∫≈Õ¥Ÿ. ¥´∞˙ æÛ¿Ω ∞·¡§¿∏∑Œ ¿Ã∑ÁæÓ¡Æ ¿÷¥Ÿ. $"
            "∞…¿ª ∂ß∏∂¥Ÿ ¡ˆ∏Èø° ¬˜∞°øÓ π∞ øıµ¢¿Ã∏¶ ∏∏µÈ∞Ì ¿÷¥Ÿ.";
#else
            "A terrible creature, formed of snow and crystalline ice. "
            "Its feet leave puddles of icy water on the floor.";
#endif
        break;

    case MONS_KOBOLD:
#ifdef JP
        description +=
            "¿¸º≥ø° ¿««œ∏È ∞Ì¥Î¿« æ«∏∂Ω≈¿« √¢¡∂π∞¿Ã∂Û∞Ìµµ «—¥Ÿ. $"
            "ƒ⁄∫º∆Æ¥¬ ∞Ì∫Ì∏∞¿ª ¥‡¿∫ ¿€¿∫ ª˝π∞∑Œ ∞≥¿« ∏”∏Æ∏¶ «œ∞Ì ¿÷¥Ÿ.  ";
#else
        description += "Reputedly the creation of an ancient demon-god, "
            "kobolds are small goblin-like creatures with canine heads.";
#endif
        break;

    case MONS_BIG_KOBOLD:
#ifdef JP
        description += "∫∏≈Î∫∏¥Ÿ ≈´ ƒ⁄∫º∆Æ¥Ÿ. ";
#else
        description += "An unusually large kobold.";
#endif
        break;

    case MONS_KOBOLD_DEMONOLOGIST:
#ifdef JP
        description += "æ«∏∂∏¶ º“»Ø«œ∞Ì ∫Œ∏Æ¥¬ ∞Õ¿ª πËøÓ ƒ⁄∫º∆Æ¥Ÿ. ";
#else
        description += "A kobold who has learned to summon and direct demons.";
#endif
        break;

    case MONS_LICH:
        description +=
#ifdef JP
            "∏Æƒ°¥¬ ¡◊¿Ω¿ª ∞≈∫Œ«— ∏∂π˝ªÁ¥Ÿ. ∏Æƒ°¥¬ «ÿ∞Ò¿Ã ∏ª∂Û∫Ÿ¿∫ Ω√√º∏Á $"
            "∞≠∑¬«— ∞≠∑…º˙ ∏∂π˝¿∏∑Œ ∫Ò¡§ªÛ¿˚¿Œ ª˝∏Ì¿ª ¿Ø¡ˆ«œ∞Ì ¿÷¥Ÿ. $"
            "¿Ã æµ•µÂ ∏ÛΩ∫≈Õ¥¬ ∞≠∑¬«— ∏∂π˝ªÁ∏Á $"
            "Ω«∑¬ø° ¥Î«— ∞≠«— ¿⁄Ω≈¿Ã æ¯¥Ÿ∏È «««ÿæﬂ «“ ¡∏¿Á¥Ÿ. ";
#else
            "A wizard who didn't want to die, a Lich is a skeletal,"
            " desiccated corpse kept alive by a mighty exercise of "
            "necromancy.  These undead creatures can wield great "
            "magic and are best avoided by all but the most confident.";
#endif
        break;

    case MONS_ANCIENT_LICH:
#ifdef JP
        description += "ºø ºˆ æ¯¥¬ ººø˘¿ª ∫∏≥ª∏Á ¥ıøÌ ∞≠∑¬«ÿ¡¯ ∏Æƒ°¥Ÿ. ";
#else
        description += "A lich who has grown mighty over countless years. ";
#endif
        break;

    case MONS_MUMMY:
#ifdef JP
        description +=
            "πÊ∫Œ√≥∏Æ∞° µ», ∫ÿ¥Î∑Œ µ—µ—∏ª∏∞ æµ•µÂ¥Ÿ. $"
            "∞Ì¥Î¿« ¿˙¡÷∑Œ øÚ¡˜¿Ã∞‘ µ«æ˙¥Ÿ. $"
            "±◊¿« øµø™¿ª ƒßπ¸«œ¥¬ ¿⁄µÈø°∞‘ ¿Ø«ÿ«— ø¿∂Û∏¶ πÊ√‚«—¥Ÿ. ";
#else
        description += "An undead figure covered in "
            "bandages and embalming fluids, "
            "compelled to walk by an ancient curse. "
            "It radiates a malign aura to those who intrude on its domain. ";
#endif
        break;

    case MONS_GUARDIAN_MUMMY:
#ifdef JP
        description +=
            "∞Ì¥Î¿« ¿¸ªÁ∑Œ, πÊ∫Œ√≥∏Æ∞° µ«æ˙¿∏∏Á $"
            "¿˙¡÷∑Œ ¿Œ«ÿ øµø¯»˜ ¡◊¡ˆ æ ∞‘ µ«æ˙¥Ÿ. ";
#else
        description += "An ancient warrior, embalmed "
            "and cursed to walk in undeath for eternity.";
#endif
        break;

    case MONS_GREATER_MUMMY:
    case MONS_MUMMY_PRIEST:
#ifdef JP
        description += "πÊ∫Œ√≥∏Æ∞° µ» ∞Ì¥Î¿« æµ•µÂ¥Ÿ. $";
#else
        description += "The embalmed and undead corpse of an ancient ";
#endif
        if (class_described == MONS_GREATER_MUMMY)
#ifdef JP
            description += "¿Ã¿¸ø°¥¬ ∞Ì¥Î¿« ±∫¡÷ø¥¥Ÿ. ";
#else
            description += "ruler";
#endif
        else
#ifdef JP
            description += "¿Ã¿¸ø°¥¬ æœ»Ê¿« «œ¿Œ¿Ãæ˙¥Ÿ. ";
#else
            description += "servant of darkness";
#endif
#ifdef JP
        //description += ".";
#else
        description += ".";
#endif
        break;

    case MONS_NAGA:
    case MONS_NAGA_MAGE:
    case MONS_NAGA_WARRIOR:
    case MONS_GUARDIAN_NAGA:
    case MONS_GREATER_NAGA:
        if (you.species == SP_NAGA)
#ifdef JP
            description = "∏≈∑¬¿˚¿Œ";
#else
            description = "An attractive";
#endif
        else
#ifdef JP
            description = "¿ÃªÛ«—";
#else
            description = "A strange";
#endif

#ifdef JP
        description +=
            " »•º∫√º¥Ÿ. $"
            "∞°Ωø¿∏∑Œ∫Œ≈Õ ¿ß¥¬ ¿Œ∞£¿Ã¡ˆ∏∏, ∞«¿Â«— ∏ˆ≈Î¿∫ ∫Ò¥√¿Ã ¿÷¥¬ $"
            "πÏ¿« ∏ˆ¿∏∑Œ ø¨∞·µ»¥Ÿ. ";
#else
        description += " hybrid; human from the chest up,"
            " with a scaly, muscular torso trailing off like "
            " that of a snake.  ";
#endif

        switch (class_described)
        {
        case MONS_GUARDIAN_NAGA:
#ifdef JP
            description +=
                "¿Ã ≥™∞°µÈ¿∫ ¡æ¡æ $"
                "∞≠∑¬«— ∏ÛΩ∫≈Õ¿« ºˆ»£¿⁄∑Œ ¿ÃøÎµ»¥Ÿ. ";
#else
            description += "These nagas are "
                "often used as guardians by powerful creatures.";
#endif
            break;
        case MONS_GREATER_NAGA:
#ifdef JP
            description += "∞≠«ÿ∫∏¿Ã∞Ì, ∞¯∞›¿˚¿∏∑Œ ∫∏¿Œ¥Ÿ. ";
#else
            description += "It looks strong and aggressive.";
#endif
            break;
        case MONS_NAGA_MAGE:
#ifdef JP
            description += "º∂∂‡«— »ƒ±§¿ª ≥≤±‚∏Á øÚ¡˜¿Ã∞Ì ¿÷¥Ÿ. ";
#else
            description += "An eldritch nimbus trails its motions. ";
#endif
            break;
        case MONS_NAGA_WARRIOR:
#ifdef JP
            description += "¿Ã¿¸¿« ∏π¿∫ ¿¸≈ıø°º≠ æÚ¿∫ »‰≈Õ∏¶ ¡ˆ¥œ∞Ì ¿÷¥Ÿ. ";
#else
            description += "It bears scars of many past battles. ";
#endif
            break;
        }
        break;

    case MONS_OGRE:
#ifdef JP
        description +=
            "ø¿≈©øÕ ∞Ì∫Ì∏∞¿« ∏’ ƒ£√¥¿Ã¡ˆ∏∏ ¥ı ≈©∞Ì ¥ı √ﬂæ««œ∏Á $"
            "¥ı ªÏ¬«¿÷¥Ÿ. ";
#else
        description += "A larger, uglier and fatter relative "
            "of orcs and goblins.";
#endif
        break;

    case MONS_OGRE_MAGE:
#ifdef JP
        description += "ø¿øÏ∞≈¿« »Ò±Õ«— ¡æ¿∏∑Œ, ∏∂π˝ªÁøÎø° ¥…º˜«œ¥Ÿ. ";
#else
        description += "A rare breed of ogre, skilled in the use of magic.";
#endif
        break;

    case MONS_PLANT:
#ifdef JP
        description +=
            "¥Î∫Œ∫–¿« Ωƒπ∞¿∫ ¥¯¡Ø¿« ¡¡¡ˆ æ ¿∫ »Ø∞Êø°º≠¥¬ ¿ﬂ ¿⁄∂Û¡ˆ æ ¡ˆ∏∏  $"
            "æÓ∂≤ ¡æ¿∫ «ﬁ∫˚¿Ã æ¯¥¬ ¡ˆ«œø° ¿˚¿¿«ÿ, ¿ﬂ ¿⁄∂Û±‚µµ «—¥Ÿ.  ";
#else
        description += "Few plants can grow in the unpleasant dungeon "
            "environment, but some have managed to adapt and even thrive "
            "underground in the absence of the sun.";
#endif
        break;

    case MONS_OKLOB_PLANT:
#ifdef JP
        description += "»≤ªÍ¿Ã πÊøÔ¡Æ ∂≥æÓ¡ˆ¥¬ ¿ß«Ë«— Ωƒπ∞¿Ã¥Ÿ. ";
#else
        description += "A vicious plant, dripping with vitriol.";
#endif
        break;

    case MONS_RAKSHASA:
    case MONS_RAKSHASA_FAKE:
#ifdef JP
        description +=
            "æ«∏∂¿« ¿œ¡æ¿∏∑Œ »˚∞˙ ¡ˆΩƒ¿ª √ﬂ±∏«ÿ π∞¡˙∞Ëø° ø¬ ¡∏¿ÁµÈ¿Ã¥Ÿ. $"
            "∂Ùª˛ª˛µÈ¿∫ »Øøµ¿ª ¡∂¡æ«œ¥¬ »˚ø° æ∆¡÷ ¿Õº˜«œ¥Ÿ. ";
#else
        description += "A type of demon who comes to the material world in "
            "search of power and knowledge. Rakshasas are experts"
            " in the art of illusion, among other things.";
#endif
        break;

    case MONS_SNAKE:
#ifdef JP
        description += "¥¯¡Øø° ªÁ¥¬ ∆Úπ¸«— πÏ¿Ã¥Ÿ. ";
#else
        description += "The common dungeon snake. ";
#endif
        break;

    case MONS_BLACK_SNAKE:
#ifdef JP
        description += "≈©∞Ì ∞À¿∫ πÏ¿Ã¥Ÿ. ";
#else
        description += "A large black snake. ";
#endif
        break;

    case MONS_BROWN_SNAKE:
#ifdef JP
        description += "≈©∞Ì ∞•ªˆ¿Œ πÏ¿Ã¥Ÿ. ";
#else
        description += "A large brown snake.";
#endif
        break;

    case MONS_GREY_SNAKE:
#ifdef JP
        description += "∏≈øÏ ≈´ »∏ªˆ¿« πÏ¿Ã¥Ÿ. ";
#else
        description += "A very large grey python.";
#endif
        break;

    case MONS_LAVA_SNAKE:
#ifdef JP
        description +=
            "∫“≈∏¥¬µÌ«— ∫”¿∫ªˆ¿« πÏ¿∏∑Œ $"
            "øÎæœø°º≠ ø√∂ÛøÕ ¥ÁΩ≈¿ª π∞∑¡«—¥Ÿ. ";
#else
        description += "A burning red snake which rears up from pools "
            "of lava and tries to bite you.";
#endif
        break;

    case MONS_SMALL_SNAKE:
#ifdef JP
        description += "¥¯¡Øø° ªÁ¥¬ ¿€¿∫ πÏ¿Ã¥Ÿ. ";
#else
        description += "The lesser dungeon snake.";
#endif
        break;

    case MONS_YELLOW_SNAKE:
#ifdef JP
        description += "≈´ »≤ªˆ¿« πÏ¡∑ ∆ƒ√Ê∑˘¥Ÿ.";
#else
        description += "A large yellow tubular reptile.";
#endif
        break;

    case MONS_GIANT_NEWT:
#ifdef JP
        description +=
            "∫∏≈Î µµ∑Ê¥®¿« ∏ÓπË¿« ≈©±‚¡ˆ∏∏ $"
            "±◊∑°µµ æ∆¡˜ ≥Ó∂ˆ¡§µµ∑Œ ≈©¥Ÿ∞Ì¥¬ «“ ºˆ æ¯¥Ÿ. ";
#else
        description += "Several times the size of a normal newt, but still "
            "not really impressive.";
#endif
        break;

    case MONS_GIANT_GECKO:
#ifdef JP
        description +=
            "πﬂ∞°∂Ùø° »Ì∆«¿Ã ¿÷æÓ ∫Æ∞˙ √µ¿Â¿ª ¿⁄¿Ø∑”∞‘ ∏≈¥ﬁ∏Æ¥¬ µµ∏∂πÏ¿Ã¥Ÿ.  $"
            "∫∏≈Î µµ∏∂πÏ∫∏¥Ÿ »Œæ¿ ≈©¥Ÿ. ";
#else
        description += "A lizard with pads on its toes allowing it to cling "
            "to walls and ceilings.  It's much larger than a normal gecko... "
            "perhaps it's something in the water?";
#endif
        break;

    case MONS_GIANT_IGUANA:
    case MONS_GIANT_LIZARD:
#ifdef JP
        description += "øÏµŒµœ ±˙π´¥¬ ≈´ ≈Œ¿ª ∞°¡¯ ∞≈¥Î«— µµ∏∂πÏ¿Ã¥Ÿ. ";
#else
        description += "A huge lizard with great crunching jaws.";
#endif
        break;

    case MONS_GILA_MONSTER:
#ifdef JP
        description +=
            "π‡¿∫ ªˆ¿« ¡Ÿπ´¥ÃøÕ æÛ∑Ë¿Ã ¿÷¥¬ ≈´ µµ∏∂πÏ¿Ã¥Ÿ. ";
#else
        description += "A large lizard with brightly coloured stripes and "
            "splotches.";
#endif
        break;

    case MONS_KOMODO_DRAGON:
#ifdef JP
        description +=
            "∏≈øÏ ∞≈¥Î«— µµ∏∂πÏ¿Ã¥Ÿ. $"
            "≈´ µøπ∞¿ª ∏‘¿Ã∑Œ ªÔ¿ª ¡§µµ¥Ÿ. $"
            "¿Ãª°ø°¥¬ ¿¸¿« ∏‘¿Ã∞° Ω‚¿∫ ∞Ì±‚¡∂∞¢¿Ã µ«æÓ æ«√Î∏¶ «≥±‚∞Ì ¿÷¥Ÿ. ";
#else
        description += "An enormous monitor lizard.  It's more than capable "
            "of preying on large animals.  Bits of fetid and rotting flesh "
            "from its last few meals are stuck in its teeth.";
#endif
        break;

    case MONS_LINDWURM:
#ifdef JP
        description +=
            "∞≠∑¬«— «— Ω÷¿« æ’πﬂ¿Ã ¿÷¥¬ µ∂ªÁøÕ ∞∞¿∫ ∏Ω¿¿« ¿€¿∫ øÎ¿Ã¥Ÿ. $"
            "±◊ µŒ≤®øÓ ∫Ò¥√ø°º≠ ±‚∫– ≥™ª€ ≥Ïªˆ ±§≈√¿Ã ≥≠¥Ÿ. ";
#else
        description += "A small serpentine dragon with a pair of strong "
            "forelimbs.  Its thick scales give off an eerie green glow.";
#endif
        break;

    case MONS_TROLL:
        description +=
#ifdef JP
            "≈©∞Ì, «¯ø¿∞®¿Ã µÂ¥¬ ∏Ω¿¿ª «— ∏ÛΩ∫≈Õ¥Ÿ. µŒ≤Æ∞Ì »§¿Ã ≥≠ $"
            "««∫Œ¥¬ ¥Î∫Œ∫–¿« ªÛ√≥∏¶ ∞≈¿« πŸ∑Œ »∏∫π«œ¥¬ µÌ ∫∏¿Œ¥Ÿ. ";
#else
            "A huge, nasty-looking creature. Its thick and knobbly hide "
            "seems to heal almost instantly from most wounds.";
#endif
        break;

    case MONS_DEEP_TROLL:
#ifdef JP
        description += "µÓ¿Ã ±¡¿∫ ∆Æ∑—¿Ã¥Ÿ. ";
#else
        description += "A stooped troll.";
#endif
        break;

    case MONS_IRON_TROLL:
        description +=
#ifdef JP
            "≥ÏΩº √∂¿« µŒ≤®øÓ ∫Ò¥√∑Œ µ§¿Œ ∞≈¥Î«— ∆Æ∑—¿Ã¥Ÿ. ";
#else
            "A great troll, plated with thick scales of rusty iron.";
#endif
        // you can't see its hide, but think it's thick and kobbly, too :P {dlb}
        //jmf: I thought its skin *was* the rusty iron. If so, ought to change
        //     shatter_monsters in spells4.cc.
        break;

    case MONS_ROCK_TROLL:
        description +=
#ifdef JP
            "æˆ√ª≥™∞‘ ≈©∞Ì ∏≈øÏ «¯ø¿Ω∫∑ØøÓ ∏Ω¿¿ª «— ¿Œ∞£«¸ ª˝π∞√º¥Ÿ. $"
            "±◊ µπµ¢æÓ∏Æ ««∫Œ¥¬ ¥Î∫Œ∫–¿« ªÛ√≥∏¶ ∞≈¿« ¡ÔΩ√ »∏∫π«œ¥¬ µÌ «œ¥Ÿ. ";
#else
            "An enormous and very nasty-looking humanoid creature. Its "
            "rocky hide seems to heal almost instantaneously from most wounds.";
#endif
        break;

    case MONS_UNSEEN_HORROR:
        description +=
#ifdef JP
            "¿Ã ∏ÛΩ∫≈Õ¥¬ ∫∏≈Î ¥´¿∏∑Œ¥¬ ∞≈¿« ∫º ºˆ æ¯¥Ÿ. $"
            "«œ¡ˆ∏∏ ±◊ ∏Ω¿¿ª ∫ª¥Ÿ∏È »ƒ»∏«“ ∞Õ¿Ã¥Ÿ.  ";
#else
            "These creatures are usually unseen by the eyes of most,"
            " and those few who have seen them would rather not have.";
#endif
        break;

    case MONS_VAMPIRE:
#ifdef JP
        description += "∞≠∑¬«— æµ•µÂ¥Ÿ. $";
#else
        description += "A powerful undead.";
#endif
        if (you.is_undead == US_ALIVE)
#ifdef JP
            description += "±◊¥¬ ¥ÁΩ≈¿« ««∏¶ ø¯«œ∞Ì ¿÷¥Ÿ! ";
#else
            description += " It wants to drink your blood! ";
#endif
        break;

    case MONS_VAMPIRE_KNIGHT:
        description +=
#ifdef JP
            "æµ•µÂ∞° µ«º≠µµ ±‚∑Æ¿∫ ø©¿¸«—, ∞≠∑¬«— ¿¸ªÁ¥Ÿ. ";
#else
            "A powerful warrior, with skills undiminished by undeath.";
#endif
        if (you.is_undead == US_ALIVE)
#ifdef JP
            description += "±◊¥¬ ¥ÁΩ≈¿« ««∏¶ ø¯«œ∞Ì ¿÷¥Ÿ! ";
#else
            description += " It wants to drink your blood! ";
#endif
        break;

    case MONS_VAMPIRE_MAGE:
#ifdef JP
        description += "æµ•µÂ∞° µ«æ˙¡ˆ∏∏ ø©¿¸»˜ ∞≠∑¬«— ∏∂π˝ªÁ¥Ÿ. ";
#else
        description += "Undeath has not lessened this powerful mage.";
#endif
        if (you.is_undead == US_ALIVE)
#ifdef JP
            description += "±◊¥¬ ¥ÁΩ≈¿« ««∏¶ ø¯«œ∞Ì ¿÷¥Ÿ! ";
#else
            description += " It wants to drink your blood! ";
#endif
        break;

    case MONS_WRAITH:
#ifdef JP
        description +=
            "¿Ã æµ•µÂ øµ»•¿∫ ∞À¿∫ æ»∞≥ø° ΩŒ¿Œ Ω«√º∞° æ¯¥¬ «ÿ∞Ò ∏Ω¿¿ª «œ∞Ì ¿÷¥Ÿ. $"
            "±◊¿« æÁ¥´¿∫ ∫Œ¡§«— æ«¿«∑Œ π‡∞‘ ∫“≈∏∞Ì ¿÷¥Ÿ.  ";
#else
        description += "This undead spirit appears as a cloud of black mist "
            "surrounding an insubstantial skeletal form. Its eyes "
            "burn bright with unholy malevolence.";
#endif
        break;

    case MONS_FREEZING_WRAITH:
#ifdef JP
        description +=
            "Ω«√º∞° æ¯¥¬ «ÿ∞Ò¿« ∏ˆø° ≥√±‚∏¶ ∞®∞Ì¿÷¥¬ æµ•µÂ¥Ÿ. ";
#else
        description += "A cloud of freezing air surrounding an incorporeal "
            "skeletal form.";
#endif
        break;

    case MONS_SHADOW_WRAITH:
#ifdef JP
        description +=
            "æ»∞≥∏¶ ∞®∞Ì¿÷¥¬ «ÿ∞Ò ±◊∏≤¿⁄¥Ÿ. ∞¯¡ﬂø° ∂∞¿÷¥Ÿ. $"
            "¿Ã æµ•µÂ¥¬ ∞≠»≠µ» Ω√∑¬¿∏∑Œµµ ∞≈¿« ∫∏¿Ã¡ˆ æ ¥¬¥Ÿ. ";
#else
        description += "A mist-wreathed skeletal shadow hanging in mid-air, "
            "this creature is almost invisible even to your enhanced sight. ";
#endif
        // assumes: to read this message, has see invis
        break;

    case MONS_YAK:
#ifdef JP
        description +=
            "∆Úπ¸«— ¥¯¡Ø æﬂ≈©¥Ÿ. ∫πΩ«∫πΩ««— ≈–∑Œ µ§ø© ¿÷¥Ÿ. $"
            "«— Ω÷¿« «Ëæ««— ª‘¿Ã ¥ﬁ∑¡ ¿÷¥Ÿ. ";
#else
        description += "The common dungeon yak, covered in shaggy yak hair "
            "and bearing a nasty pair of yak horns.";
#endif
        break;

    case MONS_DEATH_YAK:
#ifdef JP
        description +=
            "∫∏≈Î æﬂ≈©∫∏¥Ÿ ¥ı ≈©∞Ì ¥ı ∞«¿Â«— ƒ£√¥¿Ã¥Ÿ.  $"
            "¿€∞Ì ∫”¿∫ ¥´¿∫ ∏‘¿Ãø° ¥Î«— ±æ¡÷∏≤¿∏∑Œ π¯∂‡¿Ã∞Ì ¿÷¥Ÿ. ";
#else
        description += "A larger and beefier relative of the common dungeon "
            "yak. Its little red eyes gleam with hunger for living flesh.";
#endif
        break;

    case MONS_WYVERN:
#ifdef JP
        description +=
            "≥Øƒ´∑ŒøÓ µπ±‚∞° ¥ﬁ∏∞ ≤ø∏Æ∞° ¿÷¥¬, øÎ¿ª ¥‡¿∫ ∏ÛΩ∫≈Õ¥Ÿ. $"
            "∫Ò∑œ ¡¯¬• øÎ∫∏¥Ÿ ¿€∞Ì, ¿ßæ–∞®µµ µ⁄∂≥æÓ¡ˆ¡ˆ∏∏ $"
            "π´Ω√«“ ºˆ æ¯¥¬ ¿˚¿Ã¥Ÿ. ";
#else
        description += "A dragon-like creature with long sharply pointed tail."
            " Although smaller and less formidable than true dragons, "
            "wyverns are nonetheless a foe to be reckoned with.";
#endif
        break;

    case MONS_GIANT_EYEBALL:
#ifdef JP
        description += "∞≈¥Î«— ¥´æÀ¿Ã¥Ÿ. ∏ˆ¿ª ∏∂∫ÒΩ√≈∞¥¬ ¿¿Ω√∏¶ «—¥Ÿ.";
#else
        description += "A giant eyeball, with a captivating stare.";
#endif
        break;

    case MONS_GREAT_ORB_OF_EYES:
#ifdef JP
        description += "∂∞¥Ÿ¥œ¥¬ ±∏√º¥Ÿ. «•∏È¿Ã æ«¿«∞° ∞°µÊ¬˘ ¥´¿∏∑Œ µ§ø©¿÷¥Ÿ. ";
#else
        description += "A levitating ball, covered in malignant eyes.";
#endif
        break;

    case MONS_EYE_OF_DEVASTATION:
#ifdef JP
        description +=
            "∞≈¥Î«— ¥´æÀ¿Ã¥Ÿ. $"
            "πÈø≠∫˚¿∏∑Œ ¬˘∂ı«— ø°≥ ¡ˆ¿« ±∏√º∑Œ µ—∑ØΩŒø© ¿÷¥Ÿ. $";
#else
        description += "A huge eyeball, encased in a levitating globe of "
            "incandescent energy. ";
#endif
        break;

    case MONS_SHINING_EYE:
#ifdef JP
        description +=
            "≈©∞Ì ¿ÃªÛ«œ∞‘ ∫Ø«¸µ» ¥´æÀ¿Ã¥Ÿ. ∫˚¿∏∑Œ ∞Ìµøƒ°∞Ì ¿÷¥Ÿ.$"
            "¿Ã ∫Ò»¶¥ıø°∞‘ æ∆∏ß¥ŸøÓ ∞˜¿Ã∂Û∞Ô æ¯¥Ÿ. ";
#else
        description += "A huge and strangely deformed eyeball, "
            "pulsating with light. "
            "Beauty is certainly nowhere to be found " "in this beholder. ";
#endif
        break;

    case MONS_EYE_OF_DRAINING:
        description +=
#ifdef JP
            "¿Ã ∂∞¥Ÿ¥œ¥¬ ∞¯∆˜¿« ¡∏¿Á¥¬ ∏∂π˝ªÁµÈ¿Ã ∆Ø»˜ ¡¯¿˙∏Æ∏¶ ƒ£¥Ÿ. ";
#else
            "These hovering horrors are especially loathed by wizards.";
#endif
        break;

    case MONS_WIGHT:
#ifdef JP
        description += "ªÏ∞Ì ΩÕ¥Ÿ¥¬ ¿«¡ˆ∑Œ æµ•µÂ∞° µ» ∞Ì¥Î¿« ¿¸ªÁ¥Ÿ. ";
#else
        description += "An ancient warrior, kept in a state of undeath "
            "by its will to live.";
#endif
        break;

    case MONS_WOLF_SPIDER:
#ifdef JP
        description +=
            "¿ß«Ë«— ≈Œ¿ª ∞°¡¯ ≈´ ≈–∞≈πÃ¥Ÿ.$"
            "∏‘¿Ã∏¶ √£±‚ ¿ß«ÿ ¥¯¡Ø¿ª «Ï∏«¥Ÿ. ";
#else
        description += "A large hairy spider with vicious mandibles, "
            "roaming the dungeon in search of food.";
#endif
        break;

    case MONS_REDBACK:
#ifdef JP
        description +=
            "∫Œ«ÆæÓ ø¿∏• ∫π∫Œø° ∫”¿∫ π›¡°¿Ã ¿÷¥¬, ªÁ≥™øÓ ∞À¿∫ ∞≈πÃ¥Ÿ. $"
            "ƒ°∏Ì¿˚¿Œ µ∂¿Ã ≈Œ¿∏∑Œ∫Œ≈Õ πÊøÔ¡Æ ∂≥æÓ¡ˆ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A vicious black spider with a splash of red on its "
            "swollen abdomen. Its mandibles drip with lethal poison.";
#endif
        break;

    case MONS_SHADOW:
        description +=
#ifdef JP
            "Ω√æﬂ¿« ±∏ºÆ¿ª ∂∞µπæ∆¥Ÿ¥œ¥¬, ª˝∏Ì æ¯¥¬ ±◊∏≤¿⁄ ¿ßΩ¿¿Ã¥Ÿ ";
#else
            "An wisp of unliving shadow, drifting on the edge of vision.";
#endif
        break;

    case MONS_HUNGRY_GHOST:
#ifdef JP
        description +=
            "±ææÓ ¡◊¿∫ Ω√√ºø°º≠ ≥™ø¬ æµ•µÂ¥Ÿ. "
            "¿Ã æµ•µÂ¥¬ ¥ÁΩ≈µµ ∂»∞∞¿Ã ±æ∞‹ ¡◊¿Ã∑¡ «œ∞Ì ¿÷¥Ÿ! ";
#else
        description += "The undead form of someone who died of starvation,"
            " this creature wants the same thing to happen to you!";
#endif
        break;

    case MONS_BUTTERFLY:
#ifdef JP
        description +=
            "ƒø¥Ÿ∂ı «¸«¸ªˆªˆ¿« ≥™∫Ò¥Ÿ. ≥Ø∞≥∞° æ∆∏ß¥ŸøÓ ∏æÁ¿ª «œ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A large multicoloured butterfly with beautifully "
            "patterned wings.";
#endif
        break;

    case MONS_WANDERING_MUSHROOM:
#ifdef JP
        description += "ƒø¥Ÿ∂˛∞Ì ∫Ò¥Î«— πˆº∏¿Ã¥Ÿ. ";
#else
        description += "A large, fat mushroom.";
#endif
        break;

    case MONS_EFREET:
        description +=
#ifdef JP
            "¿€ø≠«œ¥¬ »≠ø∞ ±∏∏ßø° ΩŒ¿Œ, ≈©∞Ì ±Ÿ¿∞¿Ã ¥∆∏ß«— «¸√º¥Ÿ. $";
#else
            "A huge and muscular figure engulfed in a cloud of searing flame.";
#endif
        break;

    case MONS_GIANT_ORANGE_BRAIN:
#ifdef JP
        description +=
            "∞≈¥Î«— ¡÷∏ß¡¯ ≥˙¥Ÿ. πŸ¥⁄ø°º≠ ¡∂±› ∂∞¿÷¥Ÿ. $"
            "∞°≤˚æø ∏∆π⁄¿Ã ∂Ÿ∞Ì¿÷¥Ÿ. ";
#else
        description += "A huge wrinkled brain, floating just off the floor."
            " Every now and then it seems to pulsate.";
#endif
        break;

    case MONS_GIANT_BEETLE:
#ifdef JP
        description +=
            "æ√æÓ ∫Œºˆ¥¬ »˚¿Ã ∞≠«— æ∆∑°≈Œ¿Ã ¿÷¥¬ ∞≈¥Î«— ∞À¿∫ ≈ı±∏π˙∑π¥Ÿ. $"
            "∏≈øÏ ¥‹¥‹«— ≈∞∆æ¡˙¿« ø‹∞Ò∞›µµ ¿÷¥Ÿ. ";
#else
        description += "A huge black beetle with great crunching mandibles "
            "and very hard chitinous armour.";
#endif
        break;

    case MONS_BORING_BEETLE:
        description +=
#ifdef JP
            "πŸ¿ß∏¶ ∫Œºˆ¥¬ ≈Œ¿Ã ¥ﬁ∏∞, ∞•ªˆ¿« ≈´ ≈ı±∏π˙∑π¥Ÿ.";
#else
            "A large brown beetle with huge, rock-crushing mandibles.";
#endif
        break;

    case MONS_BOULDER_BEETLE:
        description +=
#ifdef JP
            "∂’¿ª ºˆ æ¯¥¬ µπ ∞∞¿∫ ≤Æ¡˙¿Ã ¿÷¥¬ $"
            "∞≈¥Î«— ∞•ªˆ ≈ı±∏π˙∑π¥Ÿ. ";
#else
            "A huge grey beetle with an almost impenetrable rocky carapace.";
#endif
        break;

    case MONS_FLYING_SKULL:
        description +=
#ifdef JP
            "ªÁæ««— ∏∂π˝¿« »˚¿∏∑Œ µø¡ﬂø° ∂∞¿÷¥¬ µŒ∞≥∞Ò¿Ã¥Ÿ.  $"
            "¿ß«Ë«— ¿Ãª°¿Ã ≥™¿÷¥Ÿ. ";
#else
            "Unholy magic keeps a disembodied undead skull hovering "
            "above the floor. It has a nasty set of teeth.";
#endif
        break;

    case MONS_MINOTAUR:
#ifdef JP
        description +=
            "≈©∞Ì ±Ÿ¿∞¡˙¿Œ ¿Œ∞£«¸ ∏ÛΩ∫≈Õ¥Ÿ. ∏”∏Æ¥¬ º“∑Œ µ«æÓ ¿÷¥Ÿ.  $"
            "∞›∏Æµ» πÃ±√¿ª ∫ª∞≈¡ˆ∑Œ ªÔ¥¬¥Ÿ. ";
#else
        description += "A large muscular human with the head of a bull. "
            "It makes its home in secluded labyrinths.";
#endif
        break;

    case MONS_SLIME_CREATURE:
        description +=
#ifdef JP
            "¡°æ◊¡˙¿« ΩΩ∂Û¿”¿Ã¥Ÿ. ¡ˆ∏È¿ª πÃ≤Ù∑Ø¡ˆ∏Á ∞°∞Ì ¿÷¥Ÿ. ";
#else
            "An icky glob of slime, which slithers along the ground.";
#endif
        break;

    case MONS_HELLION:
#ifdef JP
        description += "π´º≠øÓ æ«∏∂¥Ÿ. ∫“≈∏ø¿∏£¥¬ «Ô∆ƒ¿ÃæÓ∑Œ µ§ø©¿÷¥Ÿ. ";
#else
        description += "A frightful demon, covered in roaring hellfire.";
#endif
        break;

    case MONS_TORMENTOR:
#ifdef JP
        description +=
            "¿Ã »‰æ««— æ«∏∂¥¬ ø¬∞Æ ¡æ∑˘¿« πﬂ≈È∞˙ ∞°Ω√, ∞•∞Ì∏Æ∑Œ $"
            "ø¬ ∏ˆ¿Ã µ§ø©¿÷¥Ÿ. ";
#else
        description += "This malign devil is covered in all manner "
            "of claws, spines and cruel hooks.";
#endif
        break;

    case MONS_REAPER:
#ifdef JP
        description += "∞≈¥Î«— ≥¥¿ª ¡„∞Ì ¿÷¥¬ «ÿ∞Ò∞˙ ∞∞¿∫ ∏Ω¿¿« ¡∏¿Á¥Ÿ. ";
#else
        description += "A skeletal form wielding a giant scythe. ";
#endif
        if (you.is_undead == US_ALIVE)
#ifdef JP
            description += "$¥ÁΩ≈¿« øµ»•¿ª ∞°¡Æ∞°±‚ ¿ß«ÿ ø‘¥Ÿ! ";
#else
            description += "It has come for your soul!";
#endif
        break;

    case MONS_SOUL_EATER:
        description +=
#ifdef JP
            "¿Ã ∞≠∑¬«— æ«∏∂¥¬ ¥ÁΩ≈¿ª «‚«ÿ πÃ≤Ù∑Ø¡ˆµÌ ¥Ÿ∞°ø¿¥¬ ±◊∏≤¿⁄ ∞∞¿Ã ∫∏¿Œ¥Ÿ. $"
            "¿Ωø°≥ ¡ˆ¿« ∞≠∑¬«— ø¿∂Û∏¶ ª’∞Ì ¿÷¥Ÿ. ";
#else
            "This greater demon looks like a shadow gliding through "
            "the air towards you. It radiates an intense aura of negative power.";
#endif
        break;

    case MONS_BEAST:
#ifdef JP
        description += "¿Œ∞£∞˙ æﬂºˆ¿« ±´ªÛ«— ±≥¿‚π∞¿Ã¥Ÿ. ";
#else
        description += "A weird and hideous cross between beast and human.";
#endif
        break;

    case MONS_GLOWING_SHAPESHIFTER:
#ifdef JP
        description +=
            "∫ØΩ≈¿« ¡¶æÓ∏¶ «“ ºˆ æ¯∞‘ µ» ∫ØΩ≈¥…∑¬¿⁄¥Ÿ. $"
            "∫ØΩ≈¿Ã ∞Ëº”µ«∞Ì ¿÷¥Ÿ. ";
#else
        description += "A shapeshifter who has lost control over its "
            "transformations, and is constantly changing form.";
#endif
        break;

    case MONS_SHAPESHIFTER:
#ifdef JP
        description +=
            "¿⁄Ω≈¿« ∏ˆ¿ª ∏∂¿Ω¥Î∑Œ πŸ≤‹ ºˆ ¿÷¥¬ ∏ÛΩ∫≈Õ¥Ÿ. $"
            "¿Ã ∏ÛΩ∫≈Õ∞° ∫ª∑°¿« ∏Ω¿¿∏∑Œ ∏Ò∞›µ«¥¬ ¿œ¿∫ ∞≈¿« æ¯¥Ÿ.";
#else
        description += "A weird creature with the power to change its form. "
            "It is very rarely observed alive in its natural state.";
#endif
        break;

    case MONS_GIANT_MITE:
#ifdef JP
        description += "∞≈¥Î«— ∞≈πÃ«¸ ª˝π∞¿Ã¥Ÿ. ¿‘∫Œ∫–ø° µ∂¿Ã ¿÷¥Ÿ. ";
#else
        description += "A large arachnid with vicious poisoned mouth-parts.";
#endif
        break;

    case MONS_GRIFFON:
    case MONS_HIPPOGRIFF:
#ifdef JP
        description += "µ∂ºˆ∏Æ¿« ∏”∏ÆøÕ ≥Ø∞≥∞° ¥ﬁ∏Æ∞Ì, ";
#else
        description += "A large creature with the hindquarters of a ";
#endif
        if (class_described == MONS_HIPPOGRIFF)
#ifdef JP
            description += "∏ª¿«";
#else
            description += "horse";
#endif
        else
#ifdef JP
            description += "ªÁ¿⁄¿«";
#else
            description += "lion";
#endif
#ifdef JP
        description += "∏ˆ¿ª ∞°¡¯ ∞≈¥Î«— ∏ÛΩ∫≈Õ¥Ÿ. ";
#else
        description += " and the wings, head, and talons of a great eagle. ";
#endif
        break;

    case MONS_HYDRA:
        description +=
#ifdef JP
            "øÎ¿« ∏’ ƒ£√¥¿Œ ∞≈¥Î«— ∆ƒ√Ê∑˘¥Ÿ. $"
            "∏”∏Æ∞° æ∆¡÷ ∏π¿∏∏Á, ¥ı ¥√æÓ≥Ø ºˆµµ ¿÷¥Ÿ! ";
#else
            "A great reptilian beast, distantly related to the dragon."
            " It has many heads, and the potential to grow many more!";
#endif
        break;

    case MONS_SKELETON_SMALL:   //MONS_SMALL_SKELETON:
    case MONS_SKELETON_LARGE:   //MONS_LARGE_SKELETON:
        description +=
#ifdef JP
            "∞≠∑…º˙¿« »˚¿∏∑Œ µ«ªÏæ∆≥≠ «ÿ∞Ò¿Ã¥Ÿ. ";
#else
            "A skeleton compelled to unlife by the exercise of necromancy.";
#endif
        break;

    case MONS_SKELETAL_WARRIOR:
#ifdef JP
        description +=
            "ªÁæ««œ∞Ì ¡ﬂπ´¿Âµ» ¿Œ∞£«¸ «ÿ∞Ò¿Ã¥Ÿ. $"
            "∫“∞ÊΩ∫∑± »˚ø° ¿««ÿ øÚ¡˜¿Ã∞Ì ¿÷¥Ÿ. ";
#else
        description += "The vicious and heavily armed skeleton of a humanoid "
            "creature, animated by unholy power.";
#endif
        break;

    case MONS_HELL_KNIGHT:
#ifdef JP
        description +=
            "¡ˆø¡¿« ºº∑¬∞˙ ∞Ëæ‡¿ª «— ¡ﬂπ´¿Â«— ¿¸ªÁ¥Ÿ. ";
#else
        description += "A heavily armoured warrior, in league with the powers"
            " of Hell.";
#endif
        break;

    case MONS_WIZARD:
#ifdef JP
        description +=
            "ø©∑Ø∞°¡ˆ ∫Òº˙ø° ¥…≈Î«—, ¿Ãªˆ¿˚¿Œ ¿Œπ∞¿Ã¥Ÿ.  ";
#else
        description += "An rather eccentric person, dabbling in all sorts of"
            " arcanities.";
#endif
        break;

    case MONS_NECROMANCER:
        description +=
#ifdef JP
            "∞≠∑…º˙ ∏∂π˝¿ª ¿¸πÆ¿∏∑Œ «œ¥¬ ∏∂π˝ªÁ¥Ÿ. ";
#else
            "A wizard specializing in the practices of necromantic magic.";
#endif
        break;

    case MONS_GNOLL:
        description +=
#ifdef JP
            "∞Ì∫Ì∏∞∞˙ ø¿≈©¿« ∏’ ƒ£√¥¿Ã¥Ÿ. $"
            "¥ı ≈∞∞° ≈©∞Ì, ¥ı ¡¡¿∫ ¿Â∫Ò∏¶ «œ∞Ì ¿÷¥Ÿ.";
#else
            "A taller and better equipt relative of goblins and orcs.";
#endif
        break;

    case MONS_CLAY_GOLEM:
#ifdef JP
        description += "øÚ¡˜¿Ã∞‘ µ» ∞≈¥Î«— »Î ªÛ¿Ã¥Ÿ.  ";
#else
        description += "A huge animated clay statue.";
#endif
        break;

    case MONS_WOOD_GOLEM:
#ifdef JP
        description += "øÚ¡˜¿Ã∞‘ µ» ≥™π´ ªÛ¿Ã¥Ÿ. ";
#else
        description += "An animated wooden statue.";
#endif
        break;

    case MONS_STONE_GOLEM:
#ifdef JP
        description += "øÚ¡˜¿Ã∞‘ µ» ∞≈¥Î«— ºÆªÛ¿Ã¥Ÿ. ";
#else
        description += "A huge animated stone statue.";
#endif
        break;

    case MONS_IRON_GOLEM:
#ifdef JP
        description += "øÚ¡˜¿Ã∞‘ µ» ∞≈¥Î«— √∂¡¶ ªÛ¿Ã¥Ÿ. ";
#else
        description += "A huge animated metal statue.";
#endif
        break;

    case MONS_CRYSTAL_GOLEM:
#ifdef JP
        description += "øÚ¡˜¿Ã∞‘ µ» ∞≈¥Î«— ºˆ¡§ ªÛ¿Ã¥Ÿ. ";
#else
        description += "A huge animated crystal statue.";
#endif
        break;

    case MONS_TOENAIL_GOLEM:
#ifdef JP
        description +=
            "øÚ¡˜¿Ã∞‘ µ» ∞≈¥Î«— ªÛ¿Ã¥Ÿ. $"
            "ø¿∑Œ¡ˆ º’≈È±¿Ã∑Œ ¿⁄∏• πﬂ≈È∏∏¿∏∑Œ ¿Ã∑ÁæÓ¡Æ ¿÷¥Ÿ.  $"
            "ªÛ¥Á»˜ Ω√∞£¿Ã ≥≤æ∆µµ¥¬ ªÁ∂˜¿Ã ∏∏µÁ ∞Õ ∞∞¥Ÿ. ";
#else
        description += "A huge animated statue made entirely from toenail "
            "clippings. Some people just have too much time on their hands.";
#endif
        break;

    case MONS_ELECTRIC_GOLEM:
#ifdef JP
        description += "øÚ¡˜¿Ã∞‘ µ» ∞≈¥Î«— ªÛ¿Ã¥Ÿ. $"
                       "øœ¿¸»˜ ¿¸±‚¿« »˚¿∏∑Œ∏∏ µ«æÓ¿÷¥Ÿ. ";
#else
        description += "An animated figure made completely of electricity. ";
#endif
        break;

    case MONS_EARTH_ELEMENTAL:
#ifdef JP
        description +=
            "¥Î¡ˆ ¡§∑…∞Ë∑Œ∫Œ≈Õ º“»Øµ» ¡§∑…¿Ã¥Ÿ. $"
            "»Î∞˙ πŸ¿ß µ¢æÓ∏Æ¿« «¸≈¬∑Œ ¡∏¿Á«œ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A spirit drawn from the elemental plane of earth, "
            "which exists in this world by inhabiting a lump of earth and rocks.";
#endif
        break;

    case MONS_FIRE_ELEMENTAL:
#ifdef JP
        description +=
            "∫“ ¡§∑…∞Ë∑Œ∫Œ≈Õ º“»Øµ» ¡§∑…¿Ã¥Ÿ. $"
            "∫˚≥™¥¬ ∏Õ∑ƒ«— ∫“±‚µ’¿« «¸≈¬∑Œ ¡∏¿Á«œ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A spirit drawn from the elemental plane of fire, "
            "which exists in this world as a brilliant column of raging flames.";
#endif
        break;

    case MONS_AIR_ELEMENTAL:
#ifdef JP
        description +=
            "∞¯±‚ ¡§∑…∞Ë∑Œ∫Œ≈Õ º“»Øµ» ¡§∑…¿Ã¥Ÿ. $"
            "»æÓ¡¸∞˙ ∏¿”¿ª ∞Ëº”«œ¥¬ ∞¯±‚ »∏ø¿∏Æ¿« «¸≈¬∑Œ ¡∏¿Á«œ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A spirit drawn from the elemental plane of air. "
            "It exists in this world as a swirling vortex of air, "
            "often dissipating and reforming.";
#endif
        break;

    case MONS_WATER_ELEMENTAL:
#ifdef JP
        description +=
            "π∞ ¡§∑…∞Ë∑Œ∫Œ≈Õ º“»Øµ» ¡§∑…¿Ã¥Ÿ. $"
            "π∞∑Œ ¿Ã∑ÁæÓ¡¯ ∏ˆ¿∏∑Œ ¡∏¿Á«œ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A spirit drawn from the elemental plane of water. "
            "It exists on this world as part of a body of water.";
#endif
        break;

    case MONS_SPECTRAL_WARRIOR: // spectre
#ifdef JP
        description += "π´Ω√π´Ω√«— ≥Ïªˆ¿« π›≈ı∏Ì æµ•µÂ Ω∫««∏¥¿Ã¥Ÿ. ";
#else
        description += "A hideous translucent green undead spirit.";
#endif
        break;

    case MONS_CURSE_TOE:
#ifdef JP
        description +=
            "∂≥æÓ¡Æ ≥™∞£ πﬂ∞°∂Ù¿Ã¥Ÿ. "
            "∞¯¡ﬂ¿ª ∂∞¥Ÿ¥œ∏Á ¿Ωø°≥ ¡ˆ∏¶ πÊ√‚«œ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A disembodied toe, hanging in the air and"
            " radiating an intense field of negative energy.";
#endif
        break;

    case MONS_PULSATING_LUMP:
#ifdef JP
        description += "±∏ø™¡˙≥™¥¬, ∏ˆ∫Œ∏≤ƒ°¥¬ ∞Ì±Íµ¢¿Ã¥Ÿ. ";
#else
        description += "A revolting glob of writhing flesh.";
#endif
        break;

    case MONS_OOZE:
#ifdef JP
        description += "ø™∞‹øÓ ∞•ªˆ ¬Ó≤®±‚¿Ã¥Ÿ. ";
#else
        description += "A disgusting glob of grey sludge.";
#endif
        break;

    case MONS_BROWN_OOZE:
#ifdef JP
        description +=
            "¡°º∫¿Ã ¿÷¥¬ æ◊√º¥Ÿ. $"
            "πŸ¥⁄¿ª »Â∏£∏Á, ∫ŒΩƒΩ√≈≥ π∞¡˙¿ª √£∞Ì ¿÷¥Ÿ. ";
#else
        description += "A viscous liquid, flowing along the floor "
            "in search of organic matter to corrode. ";
#endif
        break;

    case MONS_DEATH_OOZE:
#ifdef JP
        description += "æ«√Î∏¶ «≥±‚¥¬ ∫Œ∆–«— ∞Ì±‚µ¢æÓ∏Æ¥Ÿ. ";
#else
        description += "A putrid mass of decaying flesh. ";
#endif
        break;

    case MONS_GIANT_AMOEBA:
#ifdef JP
        description += "∏∆¿Ã ∂Ÿ¥¬ ø¯«¸¡˙ µ¢æÓ∏Æ¥Ÿ. ";
#else
        description += "A pulsating lump of protoplasm. ";
#endif
        break;

    case MONS_JELLY:
#ifdef JP
        description +=
            "∏∆π⁄¿Ã ∂Ÿ¥¬ ªÍº∫¿« ø¯«¸¡˙ µ¢æÓ∏Æ¥Ÿ. $"
            "∞≈¿« ∏µÁ∞Õ¿ª º“»≠«œ∏Á, ±◊ ∂ß∏∂¥Ÿ ¡∂±›æø ¡ıΩƒ«—¥Ÿ. ";
#else
        description += "A pulsating mass of acidic protoplasm. It can and "
            "will eat almost anything, and grows a little each time...";
#endif
        break;

    case MONS_AZURE_JELLY:
#ifdef JP
        description += "π‡¿∫ «™∏•ªˆ¿ª «— ¬˜∞°øÓ ºº∆˜¡˙ µ¢æÓ∏Æ¥Ÿ. ";
#else
        description += "A frosty blob of bright blue cytoplasm. ";
#endif
        break;

    case MONS_ACID_BLOB:
        description +=
#ifdef JP
            "ƒ°∏Ì¿˚¿Œ ªÍ¿ª ∂≥æÓ∂ﬂ∏Æ¥¬, √¢πÈ«— ≥Ïªˆ¿« ªÏµ¢æÓ∏Æ¥Ÿ.";
#else
            "A lump of sickly green flesh, dripping with lethal acid.";
#endif
        break;

    case MONS_JELLYFISH:
#ifdef JP
        description +=
             "∏∆π⁄¿Ã ∂Ÿ¥¬ ≈ı∏Ì«— ªÏµ¢æÓ∏Æ¥Ÿ. $"
             "ºˆ ∏π¿∫ √Àºˆ∑Œ ¥ÁΩ≈¿ª ¬Ó∏£∑¡∞Ì ºˆ∏È πÿø°º≠ ±‚¥Ÿ∏Æ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A pulsating glob of transparent flesh, waiting just "
            "below the surface to sting you with its many tentacles.";
#endif
        break;

    case MONS_ROYAL_JELLY:
#ifdef JP
        description += "æ∆¡÷ »£»≠∑ŒøÓ ±›∫˚¿ª «— ¡©∂Û∆æ µ¢æÓ∏Æ¥Ÿ.  ";
#else
        description += "A particularly rich and golden gelatinous thing. ";
#endif
        break;

    case MONS_FIRE_GIANT:
#ifdef JP
        description += "∫˚≥™¥¬ ∏”∏Æƒ´∂Ù¿ª «— ∞≈¿Œ¿Ã¥Ÿ. ";
#else
        description += "A huge ruddy humanoid with bright hair. ";
#endif
        break;

    case MONS_FROST_GIANT:
#ifdef JP
        description += "æÛ¿Ω ∏”∏Æƒ´∂Ù¿Ã ¿÷¥¬ «™∏•ªˆ ∞≈¿Œ¿Ã¥Ÿ. ";
#else
        description += "A huge blue humanoid with hoarfrost hair.";
#endif
        break;

    case MONS_HILL_GIANT:
        description +=
#ifdef JP
            "∞≈¿Œ¡∑ ¡ﬂø°º≠¥¬ ≈∞∞° ¿€¿∫ ∆Ì¿Ã¡ˆ∏∏, ¿Ã ∞≈¿Œ¿« ≈©±‚¥¬ √Ê∫–»˜ ¿ß«Ë«œ¥Ÿ.  ";
#else
            "Although one of the smaller giant varieties, this hill giant is still big enough to be dangerous.";
#endif
        break;

    case MONS_STONE_GIANT:
        description +=
#ifdef JP
            "µπ∞˙ ∞∞¿Ã ¥‹¥‹«— ∞•ªˆ ««∫Œ¿« ∞≈¥Î«— ¿Œ∞£«¸ ª˝π∞¿Ã¥Ÿ. $"
            "πŸ¿ß∏¶ ø©∑Ø∞≥ µÈ∞Ì ¿÷¥Ÿ. - ƒ≥ƒ°∫º¿Ã∂Ûµµ «œ∑¡∞Ì «œ¥¬∞«∞°? ";
#else
            "A gigantic humanoid with grey skin almost as hard as rock. "
            "It carries several boulders - are you up for a game of 'catch'?";
#endif
        break;

    case MONS_TITAN:
#ifdef JP
        description +=
            "¿Ã π¯∞≥∑Œ π¶ªÁµ«¥¬ ∞≈¿Œ¿∫  $"
            "∞≈¿ŒµÈ¡ﬂø°º≠µµ ∆Ø»˜ ≈©∞Ì ∞≠«œ¥Ÿ. ";
#else
        description += "This lightning-limned humanoid is unusually large "
            "and powerful, even among giants.";
#endif
        break;

    case MONS_FLAYED_GHOST:
#ifdef JP
        description +=
            "π´Ω√π´Ω√«— æµ•µÂ ∏ÛΩ∫≈Õ¥Ÿ. $"
            "æﬂ¿© ∏ˆø° ∞•±‚∞•±‚ ¬ı±‰ ««∫Œ∞° ∏≈¥ﬁ∑¡ ¿÷¥Ÿ. ";
#else
        description += "A hideous undead creature, with torn skin hanging "
            "from an emaciated body.";
#endif
        break;

    case MONS_INSUBSTANTIAL_WISP:
#ifdef JP
        description += "∫Œ¿Ø«œ¥¬ øØ¿∫ ∞°Ω∫ µ¢æÓ∏Æ¥Ÿ. ";
#else
        description += "A thin wisp of floating gas.";
#endif
        break;

    case MONS_VAPOUR:
#ifdef JP
        description += "∫∏≈Î¿∫ ∫∏¿Ã¡ˆ æ ¥¬, ¿ÃªÛ«— «¸≈¬¿« ¡ı±‚ ±∏∏ß¿Ã¥Ÿ. ";
#else
        description += "A normally invisible cloud of weird-looking vapour.";
#endif
        break;

    case MONS_DANCING_WEAPON:
#ifdef JP
        description += "∞¯¡ﬂø°º≠ √„√ﬂ¥¬ π´±‚¥Ÿ. ";
#else
        description += "A weapon dancing in the air. ";
#endif
        break;

    case MONS_ELEPHANT_SLUG:
#ifdef JP
        description += "¡÷∏ß¿Ã ¡¢»˘ ««∫Œ∞° ¿÷¥¬ ∞≈¥Î«— ∞•ªˆ ¥ﬁ∆ÿ¿Ã¥Ÿ. ";
#else
        description += "A huge grey slug with folds of wrinkled skin. ";
#endif
        break;

    case MONS_GIANT_SLUG:
#ifdef JP
        description += "≈©∞Ì ø™∞‹øÓ ¥ﬁ∆ÿ¿Ã¥Ÿ. ";
#else
        description += "A huge and disgusting gastropod. ";
#endif
        break;

    case MONS_GIANT_SNAIL:
        description +=
#ifdef JP
            "π‡¿∫ ≥Ïªˆ¿« ≤Æµ•±‚∞° ¿÷¥¬ ≈©∞Ì ø™∞‹øÓ ¥ﬁ∆ÿ¿Ã¥Ÿ. ";
#else
            "A huge and disgusting gastropod with light green shell. ";
#endif
        break;

    case MONS_SHEEP:
#ifdef JP
        description += "øÏµ–«œ∞Ì ≈– ∏π¿∫ µøπ∞¿Ã¥Ÿ. ¥´ø° ªÏ¿«∞° ∞°µÊ«œ¥Ÿ. ";
#else
        description += "A stupid woolly animal, with murder in its eyes. ";
#endif
        break;

    case MONS_HOG:
#ifdef JP
        description += "≈©∞Ì ªÏ¬, ±◊∏Æ∞Ì ∏≈øÏ ∏¯ª˝±‰ µ≈¡ˆ¥Ÿ. ";
#else
        description += "A large, fat and very ugly pig. ";
#endif
        break;

    case MONS_HELL_HOG:
#ifdef JP
        description +=
            "¡ˆø¡¿« ±∏µ¢¿Ãø°º≠ ªÁ¿∞µ» "
            "≈©∞Ì ªÏ¬, ±◊∏Æ∞Ì ∏≈øÏ ∏¯ª˝±‰ µ≈¡ˆ¥Ÿ. ";
#else
        description += "A large, fat and very ugly pig, suckled "
            "in the pits of Hell. ";
#endif
        break;

    case MONS_GIANT_MOSQUITO:
#ifdef JP
        description += "∞≈¥Î«œ∞‘ ∫Œ«¨µÌ«— ∏±‚¥Ÿ. ∫¥ø° ∞…∏∞ ∞Õ√≥∑≥ ∫∏¿Œ¥Ÿ. ";
#else
        description += "A huge, bloated mosquito. It looks diseased.";
#endif
        break;

    case MONS_GIANT_CENTIPEDE:
#ifdef JP
        description += "¥Ÿ∏Æ∞° ∏π¿Ã ¥ﬁ∑¡¿÷¥Ÿ. ";
#else
        description += "It has a lot of legs.";
#endif
        break;

    case MONS_GIANT_BLOWFLY:
#ifdef JP
        description += "∞≈¥Î«œ∞Ì ±Õ¬˙¿∫ ∆ƒ∏Æ¥Ÿ. ";
#else
        description += "A huge and irritating fly.";
#endif
        break;

    case MONS_GIANT_FROG:
        description +=
#ifdef JP
            "∏∏æ‡ ¿Ã ∞≥±∏∏Æ∞° ¿€¿∫ ∞Ô√Ê∏∏¿ª ∏‘∞Ì ªÏæ“¥Ÿ∏È $"
            "¿Ã∑∏∞‘±Ó¡ˆ ƒø¡˙ ºˆ¥¬ æ¯æ˙¿ª ∞Õ¿Ã¥Ÿ. ";
#else
            "It probably didn't get this big by eating little insects.";
#endif
        break;

    case MONS_GIANT_BROWN_FROG:
#ifdef JP
        description +=
            "∏≈øÏ ≈©∞Ì ªÁæ««— ø‹∞¸¿ª «— ¿∞Ωƒ ∞≥±∏∏Æ¥Ÿ. $"
            "¿Ã ∞≥±∏∏Æ¿« »§¿Ã ≥≠ ∞•ªˆ ««∫Œ¥¬ ¡÷∫Ø¿« ∞≈ƒ£ πŸ¿ß¿Œ ∞Õ√≥∑≥ øœ¿¸»˜ ¿ß¿Âµ»¥Ÿ. ";
#else
        description += "A very large and vicious-looking carnivorous frog. "
            "Its knobbly brown skin blends in with the rough rock of your surroundings.";
#endif
        break;

    case MONS_SPINY_FROG:
        description +=
#ifdef JP
            "±◊ ªÁ√Ã«¸¡¶∫∏¥Ÿ ¡∂±› ¿€¿∫ ∞≥±∏∏Æ¥Ÿ. $"
            "¿Ã ª‘≈ıº∫¿Ã¿« ∞≥±∏∏Æ¥¬ ≈©¡ˆ æ ¿Ω¿ª $"
            "ø¬ ∏ˆø° µ§¿Œ ∞•∞Ì∏ÆøÕ ≥Øƒ´∑ŒøÓ µπ±‚∑Œ ∫∏√Ê«œ∞Ì ¿÷¥Ÿ.  ";
#else
            "Although slightly smaller than its cousin, the giant brown"
            " frog, the spiny frog makes up for lack of size by being"
            " covered in wickedly barbed spines and spurs.";
#endif
        break;

    case MONS_BLINK_FROG:
        description +=
#ifdef JP
            "¿ÃªÛ«œ∞‘ ª˝±‰ ∞≥±∏∏Æ¥Ÿ. Ω«√ºøÕ ∫ÒΩ«√º ªÁ¿Ã∏¶ ø‘¥Ÿ∞¨¥Ÿ «œ∞Ì¿÷¥Ÿ. ";
#else
            "A weird-looking frog, constantly blinking in and out of reality.";
#endif
        break;

    case MONS_GIANT_COCKROACH:
#ifdef JP
        description += "∞≈¥Î«— ∞•ªˆ πŸƒ˚π˙∑π¥Ÿ. ";
#else
        description += "A large brown cockroach.";
#endif
        break;

    case MONS_PIT_FIEND:
#ifdef JP
        description += "πœ¿ª ºˆ æ¯¿ª ¡§µµ∑Œ ∞≠¿Œ«— ««∫Œ¿« ≥Ø∞≥¥ﬁ∏∞ æ«∏∂¥Ÿ. ";
#else
        description += "A huge winged fiend with incredibly tough skin.";
#endif
        break;

    case MONS_GARGOYLE:
#ifdef JP
        description += "ª˝∏Ì¿ª ∞°¡¯ π´Ω√π´Ω√«— ºÆªÛ¿Ã¥Ÿ. ";
#else
        description += "A hideous stone statue come to life.";
#endif
        break;

    case MONS_METAL_GARGOYLE:
#ifdef JP
        description += "ª˝∏Ì¿ª ∞°¡¯ π´Ω√π´Ω√«— √∂ ªÛ¿Ã¥Ÿ.";
#else
        description += "A hideous metal statue come to life.";
#endif
        break;

    case MONS_MOLTEN_GARGOYLE:
#ifdef JP
        description += "ª˝∏Ì¿ª ∞°¡¯ π´Ω√π´Ω√«— øÎæœ ªÛ¿Ã¥Ÿ. ";
#else
        description += "A hideous melting stone statue come to life.";
#endif
        break;

    case MONS_ELF:
    case MONS_DEEP_ELF_SOLDIER:
    case MONS_DEEP_ELF_FIGHTER:
    case MONS_DEEP_ELF_KNIGHT:
    case MONS_DEEP_ELF_MAGE:
    case MONS_DEEP_ELF_SUMMONER:
    case MONS_DEEP_ELF_CONJURER:
    case MONS_DEEP_ELF_PRIEST:
    case MONS_DEEP_ELF_HIGH_PRIEST:
    case MONS_DEEP_ELF_DEMONOLOGIST:
    case MONS_DEEP_ELF_ANNIHILATOR:
    case MONS_DEEP_ELF_SORCERER:
    case MONS_DEEP_ELF_DEATH_MAGE:
        description +=
#ifdef JP
            "¿Ã »≤∑Æ«— µø±ºø° ªÏ∞Ì ¿÷¥¬ ø§«¡ ¡ﬂ¿« «œ≥™¥Ÿ. $";
#else
            "One of the race of elves which inhabits this dreary cave.$";
#endif
        switch (class_described)
        {

        case MONS_DEEP_ELF_SOLDIER:
#ifdef JP
            description += "¿Ã ø§«¡¥¬ ¿œπ›¿˚¿Œ ¿¸ªÁ¥Ÿ. ";
#else
            description += "This one is just common soldier.";
#endif
            break;

        case MONS_DEEP_ELF_FIGHTER:
#ifdef JP
            description += "¿Ã ¿¸ªÁ¥¬ ∏Ó ∞≥¿« ∏∂π˝¿ª πËøˆµŒæ˙¥Ÿ. ";
#else
            description += "This soldier has learned some magic.";
#endif
            break;

        case MONS_DEEP_ELF_KNIGHT:
#ifdef JP
            description += "¿Ã ø§«¡¥¬ ∞˙∞≈ø° ¿‘¿∫ ªÛ√≥¿⁄±π¿Ã ¿÷¥Ÿ.  ";
#else
            description += "This one bears the scars of battles past.";
#endif
            break;

        case MONS_DEEP_ELF_MAGE:
#ifdef JP
            description += "¿Ã ø§«¡¿« ±‰ º’∞°∂Ù ªÁ¿Ã∑Œ ∏∂≥™∞° πÊ¿¸«œ∞Ì ¿÷¥Ÿ. ";
#else
            description += "Mana crackles between this one's long fingers.";
#endif
            break;

        case MONS_DEEP_ELF_SUMMONER:
        case MONS_DEEP_ELF_CONJURER:
#ifdef JP
            description += "¿Ã ø§«¡¥¬ ∞Ì¥Î¿« ";
#else
            description += "This one is a mage specialized in the ancient art ";
#endif
            if (class_described == MONS_DEEP_ELF_SUMMONER)
#ifdef JP
                description += "º“»Ø ∏∂π˝ø° ∆Ø»≠µ» ∏∂π˝ªÁ¥Ÿ. ";
#else
                description += "of summoning servants";
#endif
            else
#ifdef JP
                description += "∆ƒ±´¿« »˚ø° ∆Ø»≠µ» ∏∂π˝ªÁ¥Ÿ. ";
#else
                description += "of hurling energies";
#endif
#ifdef JP
            //description += " of destruction.";
#else
            description += " of destruction.";
#endif
            break;

        case MONS_DEEP_ELF_PRIEST:
#ifdef JP
            description += "¿Ã ø§«¡¥¬ ¥Ÿ≈©ø§«¡¿« Ω≈¿ª º∂±‚∞Ì ¿÷¥Ÿ. ";
#else
            description += "This one is a servant of the deep elves' god.";
#endif
            break;

        case MONS_DEEP_ELF_HIGH_PRIEST:
            description +=
#ifdef JP
                "¿Ã ø§«¡¥¬ ¥Ÿ≈©ø§«¡¿« Ω≈¿ª º∂±‚¥¬ ∞Ì¿ß º∫¡˜¿⁄¥Ÿ. ";
#else
                "This one is an exalted servant of the deep elves' god.";
#endif
            break;

        case MONS_DEEP_ELF_DEMONOLOGIST:
            description +=
#ifdef JP
                "¿Ã ∏∂π˝ªÁ¥¬ æ«∏∂«–ø° ∆Ø»≠µ∆¥Ÿ. ∆Ø»˜ ¡÷∏Ò«ÿæﬂ «“ ∞Õ¿∫, $"
                "¿Â±‚∞£ø° ∞…√ƒ √ ¿⁄ø¨¿˚¿Œ æ«∏∂¿« ºº∑¬∞˙ ¡¢√À«ÿø‘¥Ÿ¥¬ ∞Õ¿Ã¥Ÿ. ";
#else
                "This mage specialized in demonology, and is marked heavily "
                "from long years in contact with unnatural demonic forces.";
#endif
            break;

        case MONS_DEEP_ELF_ANNIHILATOR:
#ifdef JP
            description +=
                "¿Ã ø§«¡ ∏∂π˝ªÁ¥¬ ∆ƒ±´ ∏∂π˝¿ª ¡¡æ∆«œ∏Á, ∂««— ∏≈øÏ ¥…º˜«œ¥Ÿ.";
#else
            description += "This one likes destructive magics more than most, "
                "and is better at them.";
#endif
            break;

        case MONS_DEEP_ELF_SORCERER:
#ifdef JP
            description += "¿Ã ∞≠∑¬«— ∏∂π˝ªÁ¥¬ ¡ˆø¡¿∏∑Œ∫Œ≈Õ »˚¿ª ≤¯æÓ≥ª∞Ì ¿÷¥Ÿ. ";
#else
            description += "This mighty spellcaster draws power from Hell.";
#endif
            break;

        case MONS_DEEP_ELF_DEATH_MAGE:
#ifdef JP
            description += "∞≠«— ¿Ωø°≥ ¡ˆ¿« ø¿∂Û∞° ±◊∏¶ ∞®ΩŒ∞Ì ¿÷¥Ÿ.";
#else
            description += "A strong negative aura surrounds this one.";
#endif
            break;

        case MONS_ELF:
            // These are only possible from polymorphing or shapeshifting.
#ifdef JP
            description += "¿Ã ø§«¡¥¬ ∆Úπ¸«— ø‹∞¸¿ª «œ∞Ì ¿÷¥Ÿ.";
#else
            description += "This one is remarkably plain looking.";
#endif
            break;
        }
        break;

    case MONS_WHITE_IMP:
#ifdef JP
        description += "¿€∞Ì ¿Â≥≠¿ª ¡¡æ∆«œ¥¬ «œ±ﬁ æ«∏∂¥Ÿ. ";
#else
        description += "A small and mischievous minor demon. ";
#endif
        break;

    case MONS_LEMURE:
#ifdef JP
        description += "»ÒπÃ«œ∞‘ ªÁ∂˜«¸√º∏¶ «œ∞Ì ¿÷¥¬ ∫Œ∆–«— «œæ· ªÏ µ¢æÓ∏Æ¥Ÿ. ";
#else
        description += "A vaguely humanoid blob of putrid white flesh. ";
#endif
        break;

    case MONS_UFETUBUS:
#ifdef JP
        description += "¬∞¡ˆ¥¬ º“∏Æ∑Œ ΩØªıæ¯¿Ã ¡ˆ≤¨¿Ã¥¬ «œ±ﬁ æ«∏∂¥Ÿ. ";
#else
        description += "A chattering and shrieking minor demon. ";
#endif
        break;

    case MONS_MANES:
#ifdef JP
        description += "√ﬂæ««œ∞Ì ±‚∫–≥™ª€ ¿€¿∫ «œ±ﬁ æ«∏∂¥Ÿ. ";
#else
        description += "An ugly, twisted little minor demon. ";
#endif
        break;

    case MONS_MIDGE:
#ifdef JP
        description += "≥Øæ∆¥Ÿ¥œ¥¬ ¿€¿∫ æ«∏∂¥Ÿ. ";
#else
        description += "A small flying demon. ";
#endif
        break;

    case MONS_NEQOXEC:
#ifdef JP
        description += "¿ÃªÛ«— «¸≈¬∏¶ «— æ«∏∂¥Ÿ. ";
#else
        description += "A weirdly shaped demon. ";
#endif
        break;

    case MONS_ORANGE_DEMON:
#ifdef JP
        description += "¿ß«Ë«— µ∂ƒß¿ª ∞°¡¯ π‡¿∫ ø¿∑ª¡ˆªˆ¿« æ«∏∂¥Ÿ. ";
#else
        description += "A bright orange demon with a venomous stinger. ";
#endif
        break;

    case MONS_HELLWING:
        description +=
#ifdef JP
            "π´º≠øÓ «ÿ∞Ò æ«∏∂¥Ÿ. ∞Ì¥Î¿« ∏ª∂Û∫Ÿ¿∫ ≥Ø∞≥∞° ¥ﬁ∑¡¿÷¥Ÿ. ";
#else
            "A hideous skeletal demon, with wings of ancient withered skin. ";
#endif
        break;

    case MONS_SMOKE_DEMON:
#ifdef JP
        description += "∞¯¡ﬂø° ∂· ø¨±‚±∏∏ß µ¢æÓ∏Æ¥Ÿ. ";
#else
        description += "A writhing cloud of smoke hanging in the air. ";
#endif
        break;

    case MONS_YNOXINUL:
#ifdef JP
        description += "π‡¿∫ ±›º”º∫¿« ∫Ò¥√¿Ã ≥≠ æ«∏∂¥Ÿ. ";
#else
        description += "A demon with shiny metallic scales. ";
#endif
        break;

    case MONS_EXECUTIONER:
#ifdef JP
        description += "π´º∑µµ∑œ ∞≠∑¬«— æ«∏∂¥Ÿ. ";
#else
        description += "A horribly powerful demon. ";
#endif
        break;

    case MONS_GREEN_DEATH:
        description +=
#ifdef JP
            "∫Œ«¨ ∏ˆ¿ª «— æ«∏∂¥Ÿ. $"
            "¿¸Ω≈¿Ã ¡¯π∞∞˙ ¡æ±‚∑Œ µ§ø©, ƒ°∏Ì¿˚¿Œ µ∂±∏∏ß¿ª ≥ªª’∞Ì ¿÷¥Ÿ. ";
#else
            "A bloated form covered in oozing sores and exhaling clouds of lethal poison. ";
#endif
        break;

    case MONS_BLUE_DEATH:
#ifdef JP
        description += "∞≠¥Î«— «™∏• æ«∏∂¥Ÿ. ";
#else
        description += "A blue greater demon. ";
#endif
        break;

    case MONS_BALRUG:
        description +=
#ifdef JP
            "∞≈¥Î«œ∞Ì ∏≈øÏ ∞≠∑¬«— æ«∏∂¥Ÿ, ∫“±Ê∞˙ ±◊∏≤¿⁄ø° ΩŒø©¿÷¥Ÿ. ";
#else
            "A huge and very powerful demon, wreathed in fire and shadows. ";
#endif
        break;

    case MONS_CACODEMON:
#ifdef JP
        description += "π´Ω√π´Ω√«œ∞‘ √ﬂæ««— ∫–≥Î¿« æ«∏∂¥Ÿ. $"
                       "¿¸º≥¿˚¿Œ »˚¿ª ∞°¡ˆ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A hideously ugly demon of rage and legendary power. ";
#endif
        break;

    case MONS_DEMONIC_CRAWLER:
#ifdef JP
        description +=
            "∫Œ«¨ ∏ˆ¿ª ¥Ÿºˆ¿« ¬™¿∫ ¥Ÿ∏Æ∑Œ ¡ˆ≈ «œ∏Á $"
            "≥°∫Œ∫–ø°¥¬ √ﬂæ««— ∏”∏Æ∞° ∫ŸæÓ¿÷¥Ÿ.  ";
#else
        description += "A long and bloated body, supported by "
            "dozens of short legs and topped with an evil-looking head. ";
#endif
        break;

    case MONS_SUN_DEMON:
        description +=
#ifdef JP
            "∂≥æÓ¡ˆ¥¬ ∫∞¿« ∫–≥ÎøÕ ∫˚¿∏∑Œ ∫˚≥™¥¬ æ«∏∂¿˚¿Œ ∏Ω¿¿ª «— ¡∏¿Á¥Ÿ. ";
#else
            "A demonic figure shining with the light and fury of a fallen star.";
#endif
        break;

    case MONS_SHADOW_IMP:
#ifdef JP
        description += "¿€¿∫ «œ±ﬁ ±◊∏≤¿⁄ æ«∏∂¥Ÿ. ";
#else
        description += "A small and shadowy minor demon.";
#endif
        break;

    case MONS_SHADOW_DEMON:
#ifdef JP
        description +=
            "æÀºˆæ¯¥¬ æ«∏∂¿˚ ¡∏¿Á¥Ÿ. $"
            "«◊ªÛ ¿⁄Ω≈¿« ø©∑Ø ±◊∏≤¿⁄ æ»ø°º≠ »Â∏¥«œ∞‘ ∫∏¿Ã∞Ì ¿÷¥Ÿ. ";
#else
        description += "A mysterious demonic figure,"
            " constantly blurring into multiple shadows of itself.";
#endif
        break;

    case MONS_LOROCYPROCA:
#ifdef JP
        description +=
            "≈∞∞° ≈©∞Ì æﬂ¿© ¡∏¿Á¥Ÿ. $"
            "ªÏæ∆¿÷¥¬ µÌ«— ±‰ ∑Œ∫Í∏¶ ¥√æÓ∂ﬂ∏Æ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A tall and gaunt figure, "
            "draped in long robes which flow as if alive.";
#endif
        break;

    case MONS_GERYON:
        description +=
#ifdef JP
            "¡ˆø¡¿« ∞¸πÆ¿ª ¡ˆ≈∞∞Ì ¿÷¥¬ ∞≈¥Î«œ∞Ì ∏≈≤ˆ«— æ∆≈©æ«∏∂¥Ÿ. ";
#else
            "A huge and slithery arch-demon, guarding the gates of Hell. ";
#endif
        break;

    case MONS_DISPATER:
#ifdef JP
        description += "√∂¿« µµΩ√ µΩ∫¿« ¡ˆπË¿⁄¥Ÿ. ";
#else
        description += "The lord of the Iron City of Dis. ";
#endif
        break;

    case MONS_ASMODEUS:
        description +=
#ifdef JP
            "¡ˆø¡¿« Ω…√˛ø° ∞≈¡÷«œ¥¬ æ∆≈©æ«∏∂ ¡ﬂ «œ≥™¥Ÿ. ";
#else
            "One of the arch-demons who dwell in the depths of Hell. ";
#endif
        break;

    case MONS_ANTAEUS:
#ifdef JP
        description += "ƒ⁄≈∞≈ıΩ∫¿« Ω…√˛ø° ªÁ¥¬ ∞≠¥Î«— ≈∏¿Ã≈∫¿Ã¥Ÿ. ";
#else
        description += "A great titan who lives in the depths of Cocytus. ";
#endif
        break;

    case MONS_ERESHKIGAL:
        description +=
#ifdef JP
            "≈∏∆Æ≈∏∑ÁΩ∫¿« ¡◊¿Ω¿« ¡ˆø¡¿ª ¥ŸΩ∫∏Æ¥¬ π´º≠øÓ æ∆≈© «…µÂ¥Ÿ. ";
#else
            "A fearsome arch-fiend who rules the deathly netherworld of Tartarus. ";
#endif
        break;

    case MONS_VAULT_GUARD:
#ifdef JP
        description += "¡ﬂ¿Â∞©∞˙ ¡ﬂ¿Â∫Ò∑Œ π´¿Â«—, ∫∏π∞√¢∞Ì∏¶ ¡ˆ≈∞¥¬ ªÁ∂˜¿Ã¥Ÿ. ";
#else
        description += "A heavily armed and armoured guardian of the Vaults. ";
#endif
        break;

    case MONS_CURSE_SKULL:
        description +=
#ifdef JP
            "∞¯¡ﬂø° ∂∞º≠ √µ√µ»˜ »∏¿¸«œ¥¬ ∞À¿∫ µŒ∞≥∞Ò¿Ã¥Ÿ. $"
            "»Ê»≠µ» «•∏Èø° ªı∞‹¡¯ Ω≈∫Ò«— ±‚»£µÈ¿Ã $"
            "∞≈¿« ∏µÁ ¡æ∑˘¿« ∞¯∞›ø° ≥ªº∫¿Ã∂Û¥¬ ∞Õ¿ª æÀ∑¡¡ÿ¥Ÿ. ";
#else
            "A charred skull floating in the air and rotating slowly. "
            "Mystic symbols carved into its blackened surface indicate "
            "its resistance to almost any form of attack. ";
#endif
        break;

    case MONS_ORB_GUARDIAN:
        description +=
#ifdef JP
            "∞≈¥Î«œ∞Ì, ∫˚≥™¥¬ ∫∏∂Ûªˆ ∏ÛΩ∫≈Õ¥Ÿ. $"
            "ø¿∫Í∞° ¿⁄Ω≈¿ª ¡ˆ≈∞∞‘ «œ±‚ ¿ß«œø© √¢¡∂«— ∞Õ¿Ã¥Ÿ. ";
#else
            "A huge and glowing purple creature, created by the Orb to "
            "defend itself. ";
#endif
        break;

    case MONS_DAEVA:
        description +=
#ifdef JP
            "ª˛¿Ã¥◊ ø¯¿« Ω≈º∫«— ªÁµµ¥Ÿ. $"
            "»≤±›∫˚¿∏∑Œ ∫˚≥™¥¬ ±≤»÷ø° µ—∑ØΩŒ¿Œ ≥Ø∞≥µÙ∏∞ ¿Œ∞£«¸≈¬∑Œ ¡∏¿Á«œ∞Ì ¿÷¥Ÿ.";
#else
            "A divine agent of the Shining One. It manifests as a winged "
            "figure obscured by an aura of brilliant golden light. ";
#endif
        break;

    case MONS_SPECTRAL_THING:
#ifdef JP
        description += "π´º≠øÓ, ∫˚≥™¥¬ ¿Ø∑…¿Ã¥Ÿ. ";
#else
        description += "A hideous glowing apparition.";
#endif
        break;

    case MONS_TENTACLED_MONSTROSITY:
        description +=
#ifdef JP
            "√Àºˆ¿« ¡˝«’√º¥Ÿ. æ«√Î≥™¥¬ ¡°æ◊¡˙∑Œ µ§ø©¿÷¥Ÿ. ";
#else
            "A writhing mass of tentacles, all covered in putrid mucous.";
#endif
        break;

    case MONS_SPHINX:
        description +=
#ifdef JP
            "¿Œ∞£¿« ∏”∏Æ, ªÁ¿⁄¿« ∏ˆ≈Î, ∞≈¥Î«— ªı¿« ≥Ø∞≥∏¶ ∞°¡¯ $"
            "ƒø¥Ÿ∂ı ∏ÛΩ∫≈Õ¥Ÿ. ";
#else
            "A large creature with a human head, the body of a lion, and "
            "the wings of a huge bird.";
#endif
        break;

    case MONS_ROTTING_HULK:
#ifdef JP
        description += "±∏øÔ∞˙ ∫ÒΩ¡«—, ∫Ò∆≤∞≈∏Æ∏Á ∞»¥¬ æµ•µÂ¥Ÿ. ";
#else
        description += "A shambling undead, related to the ghoul.";
#endif
        break;

    case MONS_KILLER_KLOWN:
#ifdef JP
        description +=
            "»∞±‚øÕ øÙ¿Ω¿Ã ∞°µÊ«— ¿Áπ’¥¬ ∏Ω¿¿ª «— ¡∏¿Á¥Ÿ. $"
            "¥ÁΩ≈¿ª ∏∏≥™º≠ ∏≈øÏ ±‚ªµ«œ¥¬ ∞Õ ∞∞¥Ÿ... $"
            "«œ¡ˆ∏∏ ±◊ ø‹∏ø°º≠ »ÒπÃ«— æ«¿«∞° ¥¿≤∏¡ˆ¥¬ ∞Õ¿∫ ø÷¿œ±Ó? $"
            "∫”¿∫ »≠¿Â¿Ã≥™ ±◊ ∏Ω¿∏∏¿∏∑Œ¥¬ ¡Ò∞Ã∞‘ ¥¿≤∏¡ˆ¡ˆ æ ¥¬ ∞…±Ó? $"
            "«‘≤≤ ≥Óæ∆∫∏∏È π›µÂΩ√ ¥ÁΩ≈ø°∞‘µµ $"
            "¡Ò∞≈øÚ¿Ã∂ı ∞Õ¿ª æÀ∞‘ «ÿ¡Ÿ ∞Õ¿Ã¥Ÿ. ";
#else
        description += "A comical figure full of life and laughter.  It"
            " looks very happy to see you... but is there a slightly malicious"
            " cast to its features?  Is that red facepaint or something"
            " altogether less pleasant?  Join in the fun, and maybe you'll"
            " find out!";
#endif
        break;

    case MONS_MOTH_OF_WRATH:
#ifdef JP
        description += "∞≈¥Î«— ≥™πÊ¿Ã¥Ÿ. ±◊ ≈–¿« æÁ ∏∏≈≠¿Ã≥™ ªÁ≥≥¥Ÿ. ";
#else
        description += "A huge moth, as violent as it is hairy.";
#endif
        break;

    case MONS_DEATH_COB:
#ifdef JP
        description += "π´º≠øÓ ø¡ºˆºˆ æµ•µÂ¥Ÿ. ";
#else
        description += "A dreadful undead cob of corn.";
#endif
        break;

    case MONS_BOGGART:
        description +=
#ifdef JP
            "±‚∫–≥™ª€ ¿€¿∫ ∏ˆ¡˝¿« ∞Ì∫Ì∏∞ ø‰¡§¿Ã¥Ÿ. ±◊¿« ∏∂π˝ ¿Â≥≠¿ª ¡∂Ω…«œ∂Û! ";
#else
            "A twisted little sprite-goblin. Beware of its magical tricks!";
#endif
        break;

    case MONS_LAVA_FISH:
#ifdef JP
        description += "øÎæœø°º≠ ªÁ¥¬ π∞∞Ì±‚¥Ÿ. ";
#else
        description += "A fish which lives in lava.";
#endif
        break;

    case MONS_BIG_FISH:
#ifdef JP
        description += "∫Ò¡§ªÛ¿˚¿Œ ≈©±‚¿« π∞∞Ì±‚¥Ÿ. ";
#else
        description += "A fish of unusual size.";
#endif
        break;

    case MONS_GIANT_GOLDFISH:
        description +=
#ifdef JP
            "¥ÁΩ≈¿« ±›∫ÿæÓø°∞‘ ∏‘¿Ã∏¶ ≥ π´ ∏π¿Ã ¡÷∏È, ¿Ã∑± ±´π∞¿Ã µ»¥Ÿ! ";
#else
            "This is what happens when you give your pet goldfish too much food!";
#endif
        break;

    case MONS_ELECTRICAL_EEL:
        description +=
#ifdef JP
            "¿€∞Ì ≤ˆ¿˚≤ˆ¿˚«— πÏ¿ÂæÓ¥Ÿ. πÊ¿¸¿∏∑Œ ¿Œ«— ∆ƒø≠¿Ω¿Ã ≥™∞Ì¿÷¥Ÿ. ";
#else
            "A small and slimy eel, crackling with electrical discharge.";
#endif
        break;

    case MONS_PLAYER_GHOST:
#ifdef JP
        description += ghost_description();
#else
        description += "The apparition of ";
        description += ghost_description();
        description += ".$";
#endif
        break;

    case MONS_PANDEMONIUM_DEMON:
        description += describe_demon();
        break;

    // mimics -- I'm not considering these descriptions a bug. -- bwr
    case MONS_GOLD_MIMIC:
        description +=
#ifdef JP
            "∫∏¿Ã±‚ø°¥¬ π´«ÿ«— ±›»≠¥ıπÃ∑Œ ∫∏¿Ã¡ˆ∏∏, $"
            "∏Õµ∂¿ª ∞°¡¯ ∫ØΩ≈±´π∞¿Ã ¿ß¿Â«— ∞Õ¿Ã¥Ÿ. ";
#else
            "An apparently harmless pile of gold coins hides a nasty "
            "venomous shapechanging predator.";
#endif
        break;

    case MONS_WEAPON_MIMIC:
        description +=
#ifdef JP
            "πˆ∑¡¡¯ π´±‚ ∞∞¡ˆ∏∏, Ω«¿∫ ¿ß«Ë«— º“«¸ ¡¸Ω¬¿Ã $"
            "¿ß¿Â«— ∞Õ¿Ã¥Ÿ. ";
#else
            "An apparently abandoned weapon, actually a vicious little "
            "beast in disguise.";
#endif
        break;

    case MONS_ARMOUR_MIMIC:
        description +=
#ifdef JP
            "¿ﬂ ∏∏µÈæÓ¡¯ ∞©ø ¿Ã πˆ∑¡¡Æ ¿÷¥¬ ∞Õ ∞∞¡ˆ∏∏, Ω«¿∫ $"
            "¿ß«Ë«— º“«¸ ¡¸Ω¬¿Ã ¿ß¿Â«— ∞Õ¿Ã¥Ÿ. ";
#else
            "An apparently abandoned suit of finely-made armour, actually "
            "a vicious little beast in disguise.";
#endif
        break;

    case MONS_SCROLL_MIMIC:
        description +=
#ifdef JP
            "∫Òº˙¿« ∑È¿∏∑Œ µ§¿Œ ∞Ì¥Î¿« æÁ««¡ˆ¥Ÿ. $"
            "¡ˆ±› ªÏ¬¶ øÚ¡˜¿Ã¡ˆ æ æ“≥™? ";
#else
            "An ancient parchment covered in arcane runes. Did it just twitch?";
#endif
        break;

    case MONS_POTION_MIMIC:
#ifdef JP
        description += "∏¿¿÷æÓ ∫∏¿Ã¥¬ ∏∂π˝ ¿Ω∑·¥Ÿ. ∞°º≠ ∏∂º≈∫∏¿⁄! ";
#else
        description += "A delicious looking magical drink. Go on, pick it up!";
#endif
        break;

    case MONS_BALL_LIGHTNING:
#ifdef JP
        description +=
                      "¿⁄ø¨¿« πÃΩ∫≈◊∏Æ¿Œ ¿Ã ¿¸±‚ ±∏√º¿« ∏Ω¿¿∫ $"
                      "±◊ ¡÷¿ß∏¶ ∆®±‚∏Á øÚ¡˜¿Ã¥¬µ• $"
                      "∫∏≈Î¿« π¯∞≥øÕ ∏π¿∫ ¬˜¿Ã∞° ¿÷¥Ÿ. ";
#else
        description += "An oddity of nature, ball lightning bounces around "
                      "behaving almost, but not quite, entirely unlike "
                      "regular lightning. ";
#endif
        break;

    case MONS_ORB_OF_FIRE:
#ifdef JP
        description += "ø¯Ω√¿« ∫“±Ê∑Œ ¿Ã∑ÁæÓ¡¯ ±∏√º¥Ÿ. $"
                       "¿ŒªÛ¿˚¿Œ ∫“≤…¿ª ∏∏µÈæÓ ≥æ ºˆ ¿÷¥Ÿ.. ";
#else
        description += "A globe of raw primordial fire, capable of "
                       "impressive pyrotechnics.";
#endif
        break;

    // the quokka is no more ... {dlb}
    // the quokka is back, without cyberware -- bwr
    case MONS_QUOKKA:
#ifdef JP
        description += "¿€¿∫ ¿Ø¥Î∑˘¥Ÿ. ¡„¥¬ æ∆¥œ¥Ÿ. ";
#else
        description += "A small marsupial.  Don't call it a rat.";
#endif
        break;

    // uniques
    case MONS_MNOLEG:           // was: Nemelex Xobeh - and wrong! {dlb}
#ifdef JP
        description +=
            "±‚π¶«œ∞‘ ∫˚¿ª ≥ª¥¬ ¡∏¿Á¥Ÿ. $"
            "∆«µ•∏¥œøÚ¿« µ⁄∆≤∏∞ ∞¯±‚æ»ø°º≠ √„√ﬂ∞Ì ¿÷¥Ÿ. ";
#else
        description += "A weirdly glowing figure, "
            "dancing through the twisted air of Pandemonium. ";
#endif
        break;

    case MONS_LOM_LOBON:        // was: Sif Muna - and wrong! {dlb}
#ifdef JP
        description +=
            "¿ÃªÛ«œ∏Æ ∏∏≈≠ ∆Úø¬«— ∞Ì¥Î¿« æ«∏∂¥Ÿ. $"
            "¿Ã∏∂¿« ¡ﬂæ”ø°º≠ ∫˚≥™¥¬ ∞≈¥Î«— ¥´¿Ã $"
            "¥ÁΩ≈¿ª ¬˜∞©∞‘ ¿¿Ω√«œ∞Ì ¿÷¥Ÿ. ";
#else
        description += "An ancient and strangely serene demon. "
            "It regards you coldly from "
            "the huge glowing eye in the centre of its forehead. ";
#endif
        break;

    case MONS_CEREBOV:          // was: Okawaru - and wrong! {dlb}
#ifdef JP
        description +=
            "»‰∆¯«— æ«∏∂¥Ÿ. $"
            "ºº∑π∫∏∫Í¿∫ ∫˚≥™¥¬ »≤±› ∞©ø ¿ª ¿‘∞Ì "
            "∞≈¥Î«— µ⁄∆≤∏∞ ∞À¿ª µÁ ∞≈¿Œ¿« ∏Ω¿¿∏∑Œ ≥™≈∏≥≠¥Ÿ. ";
#else
        description += "A violent and wrathful demon, "
            "Cerebov appears as a giant human "
            "covered in shining golden armour "
            "and wielding a huge twisted sword. ";
#endif
        break;

    case MONS_GLOORX_VLOQ:      // was: Kikubaaqudgha - and wrong! {dlb}
#ifdef JP
        description += "±Ì¿∫ æÓµ“¿ª ¿‘¿∫ ±◊∏≤¿⁄ ∞∞¿∫ ¡∏¿Á¥Ÿ. ";
#else
        description += "A shadowy figure clothed in profound darkness. ";
#endif
        break;

    case MONS_TERENCE:
#ifdef JP
        description += "æ««— ¿Œ∞£ ¿¸ªÁ¥Ÿ. ";
#else
        description += "An evil human fighter.";
#endif
        break;

    case MONS_JESSICA:
#ifdef JP
        description += "æ««— ∞ﬂΩ¿ ∏∂≥‡¥Ÿ. ";
#else
        description += "An evil apprentice sorceress.";
#endif
        break;

    case MONS_SIGMUND:
#ifdef JP
        description +=
               "ªÁæ««œ∞Ì ¡§∑¬¿˚¿Œ ≥Î¿Œ¿Ã¥Ÿ. $"
               "±◊¿« ¥´¿∫ ±§±‚∑Œ ∫˚≥™∞Ì ¿÷¥Ÿ. $"
               "¡ˆ±◊πÆ∆Æ¥¬ ¿ß«Ë«ÿ∫∏¿Ã¥¬ ≥¥¿ª µÈ∞Ì ¿÷¥Ÿ. ";
#else
        description += "An evil and spry old human, whose eyes "
               "twinkle with madness.  Sigmund wields a nasty looking scythe.";
#endif
        break;

    case MONS_EDMUND:
#ifdef JP
        description += "∞°∫±∞‘ π´¿Â«— ¿¸ªÁ¥Ÿ. ";
#else
        description += "A lightly armoured warrior.";
#endif
        break;

    case MONS_PSYCHE:
#ifdef JP
        description += "±›πﬂ¿« ø©¿⁄ ∏∂π˝ªÁ¥Ÿ. ";
#else
        description += "A fair-haired magess.";
#endif
        break;

    case MONS_DONALD:
#ifdef JP
        description += "ø¿∫Í∏¶ √£∞Ì ¿÷¥¬, ¥ÁΩ≈∞˙ ∞∞¿∫ ∏«Ë∞°¥Ÿ. ";
#else
        description += "An adventurer like you, trying to find the Orb.";
#endif
        break;

    case MONS_MICHAEL:
#ifdef JP
        description += "∞≠∑¬«— ∏∂π˝ªÁ¥Ÿ. ±‰ ∑Œ∫Í∏¶ ¿‘∞Ì ¿÷¥Ÿ. ";
#else
        description += "A powerful spellcaster, dressed in a long robe.";
#endif
        break;

    case MONS_JOSEPH:
#ifdef JP
        description += "øÎ∫¥√≥∑≥ ∫∏¿Œ¥Ÿ. ";
#else
        description += "Looks like a mercenary.";
#endif
        break;

    case MONS_ERICA:
#ifdef JP
        description += "æ∆∏ß¥ŸøÓ ø©¿⁄ ¡÷º˙ªÁ¥Ÿ.  ";
#else
        description += "A comely spellweaver.";
#endif
        break;

    case MONS_JOSEPHINE:
#ifdef JP
        description += "µÂ∑Á¿ÃµÂ¿« ø ¿ª ¿‘¿∫, √ﬂæ««œ∞‘ ¥ƒ¿∫ ¿Œπ∞¿Ã¥Ÿ. ";
#else
        description += "An ugly elderly figure, dressed in Druidic clothes.";
#endif
        break;

    case MONS_HAROLD:
#ifdef JP
        description += "æ««— ¿Œ∞£ «ˆªÛ±› ªÁ≥…≤€¿Ã¥Ÿ. ";
#else
        description += "An evil human bounty hunter.";
#endif
        break;

    case MONS_NORBERT:
#ifdef JP
        description += "º˜∑√µ» ¿¸ªÁ¥Ÿ. ";
#else
        description += "A skilled warrior.";
#endif
        break;

    case MONS_JOZEF:
#ifdef JP
        description += "≈∞∞° ≈´ «ˆªÛ±› ªÁ≥…≤€¿Ã¥Ÿ.";
#else
        description += "A tall bounty hunter.";
#endif
        break;

    case MONS_AGNES:
#ifdef JP
        description += "∫Ò¬Ω∏∂∏• ¿¸ªÁ¥Ÿ. ";
#else
        description += "A lanky warrior.";
#endif
        break;

    case MONS_MAUD:
#ifdef JP
        description += "ªÁæ««— ¿¸ªÁ¥Ÿ, ¿ÃªÛ«œ¡ˆ∏∏ º≥ƒ°∑˘√≥∑≥ ∫∏¿Œ¥Ÿ. ";
#else
        description += "An evil warrior who looks inexplicably like a rodent.";
#endif
        break;

    case MONS_LOUISE:
#ifdef JP
        description += "∫∏≈Î∞˙¥¬ ¥ﬁ∏Æ ¡ﬂ∞©¿∏∑Œ π´¿Â«— ∏∂π˝ªÁ¥Ÿ. ";
#else
        description += "An unusually heavily armoured spellcaster.";
#endif
        break;

    case MONS_FRANCIS:
#ifdef JP
        description += "¡÷∏ß¡¯ ∏∂π˝ªÁ¥Ÿ. ";
#else
        description += "A wizened spellcaster.";
#endif
        break;

    case MONS_FRANCES:
#ifdef JP
        description += "√º∞›¿Ã ¡¡¿∫ ¿¸ªÁ¥Ÿ, æÛ±ºø° ±Ì¿∫ »‰≈Õ∞° ≥™ ¿÷¥Ÿ. ";
#else
        description += "A stout warrior, bearing a deep facial scar.";
#endif
        break;

    case MONS_RUPERT:
#ifdef JP
        description += "ªÁæ««— ±§¿¸ªÁ¥Ÿ. ";
#else
        description += "An evil berserker.";
#endif
        break;

    case MONS_WAYNE:
#ifdef JP
        description += "∂◊∂◊«œ∞Ì æ««— µÂøˆ«¡∑Œ, πŸ∫∏ ∞∞æ∆ ∫∏¿Ã¥¬ ∏¿⁄∏¶ æ≤∞Ì¿÷¥Ÿ. ";
#else
        description += "A fat, evil dwarf in a stupid looking hat.";
#endif
        break;

    case MONS_DUANE:
#ifdef JP
        description += "∫Ò¡§ªÛ¿˚¿∏∑Œ ≈´ ±Õ∏¶ ∞°¡¯ ªÁæ««— øÎ∫¥¿Ã¥Ÿ. ";
#else
        description += "An evil mercenary with unusually large ears.";
#endif
        break;

    case MONS_NORRIS:
#ifdef JP
        description += "∞•ªˆ¿∏∑Œ ±◊¿ª∏∞, ªÁæ« ±◊ ¿⁄√º¿« º≠∆€¥Ÿ. ";
#else
        description += "A tan, fit and thoroughly evil surfer.";
#endif
        break;

    case MONS_ADOLF:
#ifdef JP
        description += "πÃ≤ˆ«— ¿¸ªÁ ∏∂π˝ªÁ¥Ÿ. $"
                       "∫“«‡«œ∞‘µµ æÛ±ºø° ≈–¿Ã ≥™¿÷¥Ÿ. ";
#else
        description += "A svelte fighter-mage with unfortunate facial hair.";
#endif
        break;

    case MONS_MARGERY:
#ifdef JP
        description += "¿Øø¨«— ∏∂π˝ªÁ¥Ÿ. ";
#else
        description += "A lithe spellcaster.";
#endif
        break;

    case MONS_IJYB:
#ifdef JP
        description += "¿€∞Ì µ⁄∆≤∏∞ ∞Ì∫Ì∏∞¿Ã¥Ÿ. æÓ∂≤ «™∏•ªˆ ≥’∏∂∏¶ ¿‘∞Ì ¿÷¥Ÿ. ";
#else
        description += "A small and twisted goblin, wearing some ugly blue rags.";
#endif
        break;

    case MONS_BLORK_THE_ORC:
#ifdef JP
        description += "æˆ√ª≥™∞‘ ªÏ¬ √ﬂæ««— ø¿≈©¥Ÿ. ";
#else
        description += "A particularly fat and ugly orc.";
#endif
        break;

    case MONS_EROLCHA:
#ifdef JP
        description += "¡ˆ±ÿ»˜ ±≥»∞«— ø¿øÏ∞≈ ∏∂≥‡¥Ÿ. ";
#else
        description += "An especially cunning ogre magess.";
#endif
        break;

    case MONS_URUG:
#ifdef JP
        description += "∫º«∞æ¯∞Ì";
#else
        description += "A rude";
#endif
        if (you.species != SP_MUMMY)
#ifdef JP
            description += ", ≥øªı≥™¥¬";
#else
            description += ", smelly";
#endif
#ifdef JP
        description += " ø¿≈©¥Ÿ. ";
#else
        description += " orc.";
#endif
        break;

    case MONS_SNORG:
#ifdef JP
        description += " ≈–≥≠ ∆Æ∑—¿Ã¥Ÿ. ";
#else
        description += "A hairy troll.";
#endif
        break;

    case MONS_XTAHUA:
#ifdef JP
        description += "∞Ì¥Î¿« ∞≠∑¬«— øÎ¿Ã¥Ÿ. ";
#else
        description += "An ancient and mighty dragon.";
#endif
        break;

    case MONS_BORIS:
        description +=
#ifdef JP
            "∞Ì¥Î¿« ∏Æƒ°¥Ÿ. $"
            "±◊ ¡÷∫Ø ∞¯±‚∞° ªÁæ««— ø°≥ ¡ˆ∑Œ ∆ƒø≠¿Ω¿ª ≥ª∞Ì ¿÷¥Ÿ. ";
#else
            "An ancient lich. The air around his shrouded form crackles with evil energy. ";
#endif
        break;

    case MONS_SHUGGOTH:
#ifdef JP
        description +=
            "±‰ ∏”∏ÆøÕ ∞°Ω√∞° ≥≠ ≤ø∏Æ, ±◊∏Æ∞Ì ªÁæ««ÿ∫∏¿Ã¥¬ ø©º∏∞°∂Ù¿« πﬂ≈È¿Ã ¿÷¥¬ $"
            "ªÁæ««— ∏ÛΩ∫≈Õ¥Ÿ. $"
            "±◊¿« ∞Ê¿Ã¿˚¿Œ »˚¿∫ ±◊¿« ¬˜ø¯ø°º≠ √ﬂπÊµ«æ˙æÓµµ ø©¿¸«œ¥Ÿ. ";
#else
        description += "A vile creature with an elongated head, spiked tail "
            "and wicked six-fingered claws. Its awesome strength is matched by "
            "its umbrage at being transported to this backwater dimension. ";
#endif
        break;

    case MONS_WOLF:
#ifdef JP
        description += "≈©∞Ì ∞≠∑¬«— ∞≥∞˙ µøπ∞¿Ã¥Ÿ. ";
#else
        description += "A large and strong grey canine.";
#endif
        break;

    case MONS_WARG:
#ifdef JP
        description +=
            "∏≈øÏ ≈©∞Ì æ««ÿ∫∏¿Ã¥¬ ¥¡¥Î¥Ÿ. $"
            "∫∏≈Î ø¿≈© π´∏Æø°º≠ √£¿ª ºˆ ¿÷¥Ÿ. ";
#else
        description += "A particularly large and evil looking wolf, usually "
            "found in the company of orcs.";
#endif
        break;

    case MONS_BEAR:
#ifdef JP
        description += "∆Úπ¸«— ¥¯¡Ø ∞ı¿Ã¥Ÿ. ";
#else
        description += "The common dungeon bear.";
#endif
        break;

    case MONS_GRIZZLY_BEAR:
#ifdef JP
        description += "≈©∞Ì ªÁ≥™øÓ »∏ªˆ ≈–¿« ∞ı¿Ã¥Ÿ. ";
#else
        description += "A large, nasty bear with grey fur.";
#endif
        break;

    case MONS_POLAR_BEAR:
#ifdef JP
        description +=
            "µ¢ƒ°∞° ≈©∞Ì ∏≈øÏ ∞≠«— ∞ı¿Ã¥Ÿ. $"
            "π›¬¶∞≈∏Æ¥¬ «œæ· ≈–∞°¡◊¿∏∑Œ µ§ø©¿÷¥Ÿ. ";
#else
        description += "A large and very strong bear covered in glistening "
            "white fur. ";
#endif
        break;

    case MONS_BLACK_BEAR:
#ifdef JP
        description += "¿€∞Ì ∞À¿∫ ∞ı¿Ã¥Ÿ. ";
#else
        description += "A small black bear.";
#endif
        break;

    case MONS_SALAMANDER:   // mv: was ANOTHER_LAVA_THING
#ifdef JP
        description +=
            "π›¿∫ ¿Œ∞£¿Ã∞Ì π›¿∫ πÏ¿Œ ª˝π∞√º¥Ÿ. $"
            "µŒ≤®øÓ ∫”¿∫ªˆ¿« ∫Ò¥√∞˙ ∞°Ω√∑Œ µ§ø© ¿÷¥Ÿ. ";
#else
        description += "A strange half-human half-snake creature "
            "covered in thick red scales and thorns.";
#endif
        break;

    case MONS_PROGRAM_BUG:
    default:
#ifdef JP
        description +=
            "∏∏æ‡ ¿Ã ∏ÛΩ∫≈Õ∞° '«¡∑Œ±◊∑• πˆ±◊'∂Û∏È $"
            "¿œ¥‹ ∞‘¿”¿ª ¿˙¿Â«œ∞Ì ¥ŸΩ√ ∫“∑Øø¿¥¬ ∞Õ¿ª √ﬂ√µ«—¥Ÿ.  $"
            "«¡∑Œ±◊∑• πˆ±◊ ∏ÛΩ∫≈Õ∏¶ ∫∏∞Ì«œµÁ¡ˆ $"
            "æ∆¥œ∏È ±◊≥… ≥ˆµŒ∞Ì ¥¯¡Ø¿ª µπæ∆¥Ÿ¥œ∞‘ «œµÁ¡ˆ, ¡¡¿ª¥Î∑Œ «œ∂Û.  ";
#else
        description += "If this monster is a \"program bug\", then it's "
            "recommended that you save your game and reload.  Please report "
            "monsters who masquerade as program bugs or run around the "
            "dungeon without a proper description to the authorities.";
#endif
        break;
        // onocentaur - donkey
    }

#if DEBUG_DIAGNOSTICS

    if (mons_flag( menv[ which_mons ].type, M_SPELLCASTER ))
    {
        int hspell_pass[6] = { MS_NO_SPELL, MS_NO_SPELL, MS_NO_SPELL,
                               MS_NO_SPELL, MS_NO_SPELL, MS_NO_SPELL };

        int msecc = ((class_described == MONS_HELLION)    ? MST_BURNING_DEVIL :
                     (class_described == MONS_PANDEMONIUM_DEMON) ? MST_GHOST
                                                 : menv[ which_mons ].number);

        mons_spell_list(msecc, hspell_pass);

        bool found_spell = false;

        for (int i = 0; i < 6; i++)
        {
            if (hspell_pass[i] != MS_NO_SPELL)
            {
                if (!found_spell)
                {
#ifdef JP
                    description += "$Monster Spells:$";
#else
                    description += "$Monster Spells:$";
#endif
                    found_spell = true;
                }

#ifdef JP
                snprintf( info, INFO_SIZE, "    %d: %s$", i,
#else
                snprintf( info, INFO_SIZE, "    %d: %s$", i,
#endif
                         mons_spell_name( hspell_pass[i] ) );

                description += info;
            }
        }
    }

    bool has_item = false;
    for (int i = 0; i < NUM_MONSTER_SLOTS; i++)
    {
        if (menv[ which_mons ].inv[i] != NON_ITEM)
        {
            if (!has_item)
            {
#ifdef JP
                description += "$Monster Inventory:$";
#else
                description += "$Monster Inventory:$";
#endif
                has_item = true;
            }

            char buff[ ITEMNAME_SIZE ];

            item_def item = mitm[ menv[which_mons].inv[i] ];
            set_ident_flags( item, ISFLAG_IDENT_MASK );

            item_name( item, DESC_NOCAP_A, buff );
#ifdef JP
            snprintf( info, INFO_SIZE, "    %d: %s$", i, buff );
#else
            snprintf( info, INFO_SIZE, "    %d: %s$", i, buff );
#endif
            description += info;
        }
    }

#endif

    print_description(description);

    set_keyin_mode(KEYIN_MODE_MORE);
    if (getch() == 0)
        getch();
    set_keyin_mode(KEYIN_MODE_NONE);

#ifdef DOS_TERM
    puttext(25, 1, 80, 25, buffer);
    window(1, 1, 80, 25);
#endif
}                               // end describe_monsters

//---------------------------------------------------------------
//
// ghost_description
//
// Describes the current ghost's previous owner. The caller must
// prepend "The apparition of" or whatever and append any trailing
// punctuation that's wanted.
//
//---------------------------------------------------------------
std::string ghost_description(bool concise)
{
    char tmp_buff[ INFO_SIZE ];

    // We're fudging stats so that unarmed combat gets based off
    // of the ghost's species, not the player's stats... exact
    // stats are required anyways, all that matters is whether
    // dex >= str. -- bwr
    const int dex = 10;
    int str;
    switch (ghost.values[GVAL_SPECIES])
    {
      case SP_HILL_DWARF:
      case SP_MOUNTAIN_DWARF:
      case SP_TROLL:
      case SP_OGRE:
      case SP_OGRE_MAGE:
      case SP_MINOTAUR:
      case SP_HILL_ORC:
      case SP_CENTAUR:
      case SP_NAGA:
      case SP_MUMMY:
      case SP_GHOUL:
      str = 15;
      break;

      case SP_HUMAN:
      case SP_DEMIGOD:
      case SP_DEMONSPAWN:
      str = 10;
      break;

      default:
      str = 5;
      break;
    }

#ifdef JP  //∞À≈‰ - ±∏¿Ã - ¿ß¬ ¿∫ æÀæ∆º≠ «ÿ¡÷ººø‰.
            snprintf( tmp_buff, sizeof(tmp_buff),
                  (concise ? "%s %s (%s%s-%s)" :
                  "%s %s : %s%s %s $") ,
                  skill_title( ghost.values[GVAL_BEST_SKILL],
                               ghost.values[GVAL_SKILL_LEVEL],
                               ghost.values[GVAL_SPECIES],
                               str, dex, GOD_NO_GOD ),
                  ghost.name,

                  (ghost.values[GVAL_EXP_LEVEL] <  4) ? "æ‡«—" :
                  (ghost.values[GVAL_EXP_LEVEL] <  7) ? "∆Úπ¸«—" :
                  (ghost.values[GVAL_EXP_LEVEL] < 11) ? "∞Ê«Ë¿Ã Ω◊¿Œ" :
                  (ghost.values[GVAL_EXP_LEVEL] < 16) ? "»˚ ¿÷¥¬" :
                  (ghost.values[GVAL_EXP_LEVEL] < 22) ? "≈πø˘«—" :
                  (ghost.values[GVAL_EXP_LEVEL] < 26) ? "¿ß¥Î«—" :
                  (ghost.values[GVAL_EXP_LEVEL] < 27) ? "π´º≠øÓ »˚¿«"
                                                      : "¿¸º≥¿˚¿Œ",

            ( concise? get_species_abbrev(ghost.values[GVAL_SPECIES]) :
                species_name( ghost.values[GVAL_SPECIES],
                    ghost.values[GVAL_EXP_LEVEL] ) ),

            ( concise? get_class_abbrev(ghost.values[GVAL_CLASS]) :
                get_class_name( ghost.values[GVAL_CLASS] ) ) );

#else
    snprintf( tmp_buff, sizeof(tmp_buff),
            "%s the %s, a%s %s %s",
            ghost.name,

            skill_title( ghost.values[GVAL_BEST_SKILL],
                ghost.values[GVAL_SKILL_LEVEL],
                ghost.values[GVAL_SPECIES],
                str, dex, GOD_NO_GOD ),

            (ghost.values[GVAL_EXP_LEVEL] <  4) ? " weakling" :
            (ghost.values[GVAL_EXP_LEVEL] <  7) ? "n average" :
            (ghost.values[GVAL_EXP_LEVEL] < 11) ? "n experienced" :
            (ghost.values[GVAL_EXP_LEVEL] < 16) ? " powerful" :
            (ghost.values[GVAL_EXP_LEVEL] < 22) ? " mighty" :
            (ghost.values[GVAL_EXP_LEVEL] < 26) ? " great" :
            (ghost.values[GVAL_EXP_LEVEL] < 27) ? "n awesomely powerful"
            : " legendary",

            ( concise? get_species_abbrev(ghost.values[GVAL_SPECIES]) :
                species_name( ghost.values[GVAL_SPECIES],
                    ghost.values[GVAL_EXP_LEVEL] ) ),

            ( concise? get_class_abbrev(ghost.values[GVAL_CLASS]) :
                get_class_name( ghost.values[GVAL_CLASS] ) ) );
#endif
    return std::string(tmp_buff);
}

static void print_god_abil_desc( int abil )
{
    const ability_def &abil_info = get_ability_def( abil );

#ifdef JP
    const std::string cost = "(" + make_cost_description( abil_info ) + ")";
#else
    const std::string cost = "(" + make_cost_description( abil_info ) + ")";
#endif

    // Produce a 79 character string with cost right justified:
    std::string str( abil_info.name );
#ifdef JP
    str += std::string( 77 - str.length() - cost.length(), ' ' ) + cost + EOL;
#else
    str += std::string( 79 - str.length() - cost.length(), ' ' ) + cost + EOL;
#endif
    cprintf( str.c_str() );
}


//---------------------------------------------------------------
//
// describe_god
//
// Describes all gods. Accessible through altars (by praying), or
// by the ^ key if player is a worshipper.
//
//---------------------------------------------------------------

void describe_god( int which_god, bool give_title )
{

    const char *description; // mv: tmp string used for printing description
    int         colour;      // mv: colour used for some messages

#ifdef DOS_TERM
    char buffer[4000];
    gettext( 1, 1, 80, 25, buffer );
    window( 1, 1, 80, 25 );
#endif

    clrscr();

    if (give_title)
    {
        textcolor( WHITE );
#ifdef JP
        cprintf( "                                    Ω≈æ”" EOL );
#else
        cprintf( "                                  Religion" EOL );
#endif
        textcolor( LIGHTGREY );
    }

    if (which_god == GOD_NO_GOD) //mv:no god -> say it and go away
    {
#ifdef JP
        cprintf( EOL "¥ÁΩ≈¿∫ Ω≈æ”¿ª ∞°¡ˆ∞Ì ¿÷¡ˆ æ ¥Ÿ." );
#else
        cprintf( EOL "You are not religious." );
#endif
        goto end_god_info;
    }

    colour = god_colour(which_god);

    //mv: print god's name and title - if you can think up better titles
    //I have nothing against
    textcolor(colour);
    cprintf (god_name(which_god,true)); //print long god's name
    cprintf (EOL EOL);

    //mv: print god's description
    textcolor (LIGHTGRAY);

    switch (which_god)
    {
    case GOD_ZIN:
#ifdef JP
        description = "¡¯¿∫ ∞Ì¥Î∑Œ∫Œ≈Õ º˛πËµ«æÓø¬ Ω≈¿Ã∏Á," EOL
                      "¡˙º≠∏¶ »Æ∏≥«œ∞Ì, æœ»Ê∞˙ »•µ∑¿« ºº∑¬¿ª ∆ƒ±´«œ¥¬ ¿œø° »˚¿ª ΩÒæ∆ ø‘¥Ÿ." EOL
                      "∞°ƒ°∏¶ ¿Œ¡§πﬁ¿∫ Ω≈µµ¥¬ ªÁæ««— ¡∏¿ÁøÕ ¥Îƒ°«œ±‚ ¿ß«— ø©∑Ø∞°¡ˆ ¿ØøÎ«— »˚¿ª" EOL
                      "æÚ¿ª ºˆ ¿÷¥Ÿ." EOL
                      "¥‹ ∞≠∑…º˙ ∏∂π˝ π◊ ±◊ ø‹¿« ∫Œ¡§«— ∏∂º˙¿˚ «‡µø¿∫ ªÔ∞°ƒ° æ ¿∏∏È æ» µ»¥Ÿ." EOL
                      "¡¯¿∫ ∞°ƒ° ¿÷¥¬ π∞∞«¿« π¯¡¶, ∫Øƒ°æ ∞Ì ¡ˆº”µ«¥¬ ±‰ Ω≈æ”¿ª ≥Ù∞‘ ∆Ú∞°«—¥Ÿ";
#else
        description = "Zin is an ancient and revered God, dedicated to the establishment of order" EOL
                      "and the destruction of the forces of chaos and night. Valued worshippers " EOL
                      "can gain a variety of powers useful in the fight against the evil, but must" EOL
                      "abstain from the use of necromancy and other forms of unholy magic." EOL
                      "Zin appreciates long-standing faith as well as sacrifices of valued objects." EOL;
#endif
        break;

    case GOD_SHINING_ONE:
#ifdef JP
        description = "ª˛¿Ã¥◊ ø¯¿∫ ∞≠¥Î«— º∫¿¸¿« Ω≈¿Ã∏Á, æ«∞˙¿« ΩŒøÚø° ¿÷æÓº≠ ¡¯∞˙ µø∏Õ ∞¸∞Ëø° ¿÷¥Ÿ. " EOL
                      "Ω≈µµø°∞‘¥¬ √µ∞Ë¿« ∫–≥Î∏¶ Ω≈º”«œ∞‘ ªÛ¥Îø°∞‘ ≥ª∏Æ¥¬ ¥…∑¬¿Ã ¡÷æÓ¡ˆ¡ˆ∏∏" EOL
                      "∞·ƒ⁄ ≥™ª€ º∫¡˙¿« ∏∂º˙¿ª ªÁøÎ«œ∏È æ» µ«∏Á, ∂« ∏Ìøπ∏¶ ∞°¡ˆ∞Ì ΩŒøÏ¡ˆ æ ¿∏∏È æ» µ»¥Ÿ." EOL
                      "ª˛¿Ã¥◊ ø¯¿∫ ªÁæ««— ¡∏¿Á∏¶ π⁄∏Í«œ¥¬ ∞Õ∞˙" EOL
                      "∫Øƒ°æ ∞Ì ¡ˆº”µ«¥¬ ±‰ Ω≈æ”¿ª ≥Ù∞‘ ∆Ú∞°«—¥Ÿ.";
#else
        description = "The Shining One is a powerful crusading deity, allied with Zin in the fight" EOL
                      "against evil. Followers may be granted with the ability to summarily dispense" EOL
                      "the wrath of heaven, but must never use any form of evil magic and should" EOL
                      "fight honourably. The Shining One appreciates long-standing persistence in " EOL
                      "the endless crusade, as well as the dedicated destruction of unholy creatures.";
#endif
        break;

    case GOD_KIKUBAAQUDGHA:
#ifdef JP
        description = "≈∞ƒÌπŸƒÌµÂ«œ¥¬ π´º≠øÓ æ«∏∂¿« Ω≈¿Ã∏Á," EOL
                      "¡◊¿Ω¿« »˚¿ª ≈Ω±∏«œ¥¬ ªÁ∂˜µÈ¿Ã º˛πË«—¥Ÿ." EOL
                      "Ω≈µµ¥¬ æµ•µÂø° ¥Î«— ∆Ø∫∞«— »˚¿ª æÚ¿∏∏Á" EOL
                      "∆Ø»˜ √—æ÷πﬁ¥¬ Ω≈µµ¥¬ ¿˚¿ª ¡◊¿Ã±‚ ¿ß«œø© ∞≠∑¬«— æ«∏∂∏¶ ∫“∑Ø≥æ ºˆ ¿÷¥Ÿ." EOL
                      "≈∞ƒÌπŸƒÌµÂ«œ¥¬ æ¡¶≥™ ªÏæ∆¿÷¥¬ ¡∏¿Á∏¶ ªÏ¿∞«œ±‚∏¶ ø¯«œ∞Ì ¿÷¥Ÿ." EOL
                      "±◊∑Ø≥™ ¡¶¥‹ø°º≠ π¯¡¶«œ¡ˆ æ ¥¬ «—, Ω√√ºø°¥¬ ∫∞∑Œ ∞¸Ω…¿Ã æ¯¥Ÿ.";
#else
        description = "Kikubaaqudgha is a terrible Demon-God, served by those who seek knowledge of" EOL
                      "the powers of death. Followers gain special powers over the undead, and " EOL
                      "especially favoured servants can call on mighty demons to slay their foes." EOL
                      "Kikubaaqudgha requires the deaths of living creatures as often as possible," EOL
                      "but is not interested in the offering of corpses except at an appropriate" EOL
                      "altar.";
#endif
        break;

    case GOD_YREDELEMNUL:
#ifdef JP
        description = "¿Ã∑πµ®∑Ω¥≠¿∫ ∞≠∑…º˙ ∏∂π˝¿ª πËøÏ¡ˆ æ ¿∏∏Èº≠ ¡◊¿Ω∞˙ æµ•µÂø° ¥Î«— ¡ˆπË∑¬¿ª" EOL
                      "ø¯«œ¥¬ ªÁ∂˜µÈø° ¿««ÿ º˛πËµ«∞Ì ¿÷¥Ÿ." EOL
                      "Ω≈µµ¥¬ ≥Îøπ»≠«— æµ•µÂ¿« ±∫ºº∏¶ ∫“∑Ø ¿œ¿∏≈≥ ºˆ∞° ¿÷¿∏∏Á" EOL
                      "±◊ π€ø°µµ ∏π¿∫(∫“ƒË«— ∞ÕµÈ¿Ã ¥Î∫Œ∫–¿Ã¡ˆ∏∏) ¿ØøÎ«— »˚¿ª »πµÊ«—¥Ÿ." EOL
                      "¿Ã∑πµ®∑ª¥≠¿∫ ªÏª˝¿ª ±‚ªµ«œ¡ˆ∏∏, Ω√√ºø° ¥Î«ÿº≠¥¬ ¿⁄±‚ø°∞‘ πŸƒ°¥¬ ∞Õ∫∏¥Ÿµµ" EOL
                      "æµ•µÂ∑Œº≠ ¿ÃøÎ«œ¥¬ ∞Õ¿ª ¥ı ¡¡æ∆«—¥Ÿ.";
#else
        description = "Yredelemnul is worshipped by those who seek powers over death and the undead" EOL
                      "without having to learn to use necromancy. Followers can raise legions of " EOL
                      "servile undead and gain a number of other useful (if unpleasant) powers." EOL
                      "Yredelemnul appreciates killing, but prefers corpses to be put to use rather" EOL
                      "than sacrificed.";
#endif
        break;

    case GOD_XOM:
#ifdef JP
        description = "¡ª¿∫ ±§±‚øÕ ∫Ø¥ˆ¿« »•µ∑¿« Ω≈¿Ã¥Ÿ. " EOL
                      "±◊¥¬ º˛πË¿⁄∞° æ∆¥—, ªı∑ŒøÓ ¿Â≥≠∞®¿ª ø¯«œ∞Ì ¿÷¥Ÿ." EOL
                      "∏π¿∫ ªÁ∂˜¿Ã »«∏¢«— ∆˜ªÛ¿Ã≥™ ∞≠«— »˚¿ª πﬁ¥¬ ∞Õ¿ª πŸ∂Û∏Á ¡ª¿ª º˛πË «œ¡ˆ∏∏," EOL
                      "¡ª¿∫ Ω≈¿⁄∏¶ ø¿∑Œ¡ˆ ∫Ø¥ˆΩ∫∑¥∞‘ √Î±ﬁ«—¥Ÿ.";
#else
        description = "Xom is a wild and unpredictable God of chaos, who seeks not worshippers but" EOL
                      "playthings to toy with. Many choose to follow Xom in the hope of receiving" EOL
                      "fabulous rewards and mighty powers, but Xom is nothing if not capricious. ";
#endif
        break;

    case GOD_VEHUMET:
#ifdef JP
        description = "∫£»ƒ∏‰¿∫ ∆ƒ±´¿˚¿Œ ∏∂π˝¿« »˚¿ª ∞¸¿Â«œ¥¬ Ω≈¿Ã¥Ÿ." EOL
                      "Ω≈µµ¥¬ ∫Òº˙¿ª ±∏ªÁ«œ¥¬ ¥…∑¬¿ª ∞≠»≠Ω√≈∞¥¬ ø©∑Ø∞°¡ˆ »˚¿ª æÚ¥¬¥Ÿ." EOL
                      "±◊∏Æ∞Ì ∞°¿Â ≈´ √—æ÷∏¶ πﬁ¿∫ ªÁ∂˜¿∫, ∫£»ƒ∏‰¿« ¿Âº≠ø° ±‚∑œµ» " EOL
                      "π´º≠øÓ ¡÷πÆ¿ª ¿ÃøÎ«“ ¿⁄∞›¿Ã ¡÷æÓ¡¯¥Ÿ." EOL
                      "∫£»ƒ∏‰ø°∞‘¿« «ÂΩ≈¿∫ «“ ºˆ ¿÷¥¬ «— «–ªÏ∞˙ ∆ƒ±´∏¶ ¿œ¿∏≈∞¥¬ ∞Õ¿∏∑Œ" EOL
                      "¡ı∏Ì«“ ºˆ ¿÷¥Ÿ.";
#else
        description = "Vehumet is a God of the destructive powers of magic. Followers gain various" EOL
                      "useful powers to enhance their command of the hermetic arts, and the most" EOL
                      "favoured stand to gain access to some of the fearsome spells in Vehumet's" EOL
                      "library. One's devotion to Vehumet can be proved by the causing of as much" EOL
                      "carnage and destruction as possible.";
#endif
        break;

    case GOD_OKAWARU:
#ifdef JP
        description = "ø¿ƒ´øÕ∑Á¥¬ »£¿¸¿˚¿Ã∞Ì ∞≠∑¬«— ¿¸≈ı¿« Ω≈¿Ã¥Ÿ." EOL
                      "Ω≈µµ¥¬ ¿¸≈ıø°º≠ ¿ØøÎ«— ºˆ∏π¿∫ »˚∞˙ ∆˜ªÛ¿« π∞∞«¿ª æÚ¥¬¥Ÿ." EOL
                      "±◊∑Ø≥™ Ω≈¿⁄¥¬ ¿¸≈ıøÕ Ω√√ºøÕ ∞°ƒ° ¿÷¥¬ π∞∞«¿« «Â≥≥¿∏∑Œ" EOL
                      "«◊ªÛ Ω≈æ”¿ª ¡ı∏Ì«œ¡ˆ æ ¿∏∏È æ» µ»¥Ÿ.";
#else
        description = "Okawaru is a dangerous and powerful God of battle. Followers can gain a " EOL
                      "number of powers useful in combat as well as various rewards, but must " EOL
                      "constantly prove themselves through battle and the sacrifice of corpses" EOL
                      "and valuable items.";
#endif
        break;

    case GOD_MAKHLEB:
#ifdef JP
        description = "∆ƒ±´¿⁄ ∏∂≈©∑π∫Í¥¬ »•µ∑∞˙ ∆¯∑¬ø° ¿««— ¡◊¿Ω¿ª ∞¸¿Â«œ¥¬ π´º≠øÓ Ω≈¿Ã¥Ÿ." EOL
                      "Ω≈¿⁄¥¬ «◊ªÛ ¿Ø«˜¿ª πŸ√ƒº≠ ∏∂≈©∑π∫Í∏¶ ∏∏¡∑Ω√≈∞¡ˆ æ ¿∏∏È æ» µ«¡ˆ∏∏" EOL
                      "¡◊¿Ω∞˙ ∆ƒ±´¿« ø©∑Ø∞°¡ˆ »˚¿ª æÚ¿ª ºˆ∞° ¿÷¥Ÿ." EOL
                      "¿Ã ∆ƒ±´Ω≈¿∫, Ω√√ºøÕ ∞°ƒ° ¿÷¥¬ π∞∞«¿« «Â≥≥¿ª ≥Ù∞‘ ∆Ú∞°«—¥Ÿ.";
#else
        description = "Makhleb the Destroyer is a fearsome God of chaos and violent death. Followers," EOL
                      "who must constantly appease Makhleb with blood, stand to gain various powers " EOL
                      "of death and destruction. The Destroyer appreciates sacrifices of corpses and" EOL
                      "valuable items.";
#endif
        break;

    case GOD_SIF_MUNA:
#ifdef JP
        description = "Ω√«¡-π´≥™¥¬ ¡÷∑Œ ∏ÌªÛ¿˚¿Ã¡ˆ∏∏, ∞≠∑¬«— Ω≈¿Ã∏Á," EOL
                      "∏∂π˝¿« ¡ˆΩƒ¿ª ≈Ω±∏«œ¥¬ ªÁ∂˜µÈ¿Ã º˛πË«œ∞Ì ¿÷¥Ÿ" EOL
                      "Ω√«¡-π´≥™¥¬ ∞°ƒ° ¿÷¥¬ π∞∞«¿« ∫¿«Â∞˙ ¿⁄¡÷ ¡÷πÆ¿« »˚¿ª ªÁøÎ«œ¥¬ ∞Õ¿ª ¡¡æ∆«—¥Ÿ.";
#else
        description = "Sif Muna is a contemplative but powerful deity, served by those who seek" EOL
                      "magical knowledge. Sif Muna appreciates sacrifices of valuable items, and" EOL
                      "the casting of spells as often as possible.";
#endif
        break;

    case GOD_TROG:
#ifdef JP
        description = "∆Æ∑Œ±◊¥¬ ∫–≥ÎøÕ ∆¯∑¬¿ª ∏√¥¬ ∞Ì¥Î¿« Ω≈¿Ã¥Ÿ." EOL
                      "Ω≈µµ¥¬ ∆Æ∑Œ±◊¿« ¿Ã∏ß¿∏∑Œ ªÏ¿∞«œ∞Ì, Ω√√º∏¶ ¡¶π∞∑Œ πŸ√ƒæﬂ «—¥Ÿ." EOL
                      "¥ÎΩ≈ø° ¿¸≈ıø° ¿÷æÓº≠¿« ¥…∑¬∞˙ ∂ß∂ß∑Œ ∆˜ªÛ¿« π∞∞«¿ª æÚ¥¬¥Ÿ." EOL
                      "∆Æ∑Œ±◊¥¬ ∏∂π˝ªÁ∏¶ πÃøˆ«œ∞Ì ¿÷¿∏π«∑Œ, Ω≈¿⁄¥¬ ∏∂π˝¿« ªÁøÎ¿Ã ±›¡ˆµ»¥Ÿ.";
#else
        description = "Trog is an ancient God of anger and violence. Followers are expected to kill" EOL
                      "in Trog's name and sacrifice the dead, and in return gain power in battle and" EOL
                      "occasional rewards. Trog hates wizards, and followers are forbidden the use" EOL
                      "of spell magic. ";
#endif
        break;

    case GOD_NEMELEX_XOBEH:
#ifdef JP
        description = "≥◊∏·∑∫Ω∫¥¬ ∫“∞°ªÁ¿««œ∞Ì ∫Ø¥ˆΩ∫∑ØøÓ ∆Æ∏ØΩ∫≈Õ¿« Ω≈¿Ã¥Ÿ." EOL
                      "±◊ »˚¿∫ ≥◊∏ﬁ∑∫Ω∫∞° æ«∏∂¿« ««∏¶ ¿ÃøÎ«ÿ ±◊∏∞ ∏∂π˝¿« ƒ´µÂ π≠¿Ω¿ª ≈Î«ÿ" EOL
                      "∫“∑Ø ≥æ ºˆ∞° ¿÷¥Ÿ." EOL
                      "Ω≈µµ∞° ∆Ø∫∞«— º±π∞¿ª πﬁæ“¿ª ∞ÊøÏ, ±◊∞Õ¿ª ∞°¥…«—«— ¿ﬂ ªÁøÎ«ÿæﬂ∏∏ «—¥Ÿ." EOL
                      "≥◊∏·∑∫Ω∫¥¬ æÓ∂∞«— ¡æ∑˘¿« ∞¯π∞µµ ±‚≤®¿Ã πﬁ∞Ì ºˆøÎ«—¥Ÿ.";
#else
        description = "Nemelex is a strange and unpredictable trickster God, whose powers can be" EOL
                      "invoked through the magical packs of cards which Nemelex paints in the ichor" EOL
                      "of demons. Followers receive occasional gifts, and should use these gifts as" EOL
                      "as much as possible. Offerings of any type of item are also appreciated.";
#endif
        break;

    case GOD_ELYVILON:
#ifdef JP
        description = "ƒ°∑·¿⁄ ø°∏Æ∫Ò∑–¿∫ ∆Ø»˜ ƒ°∑·¿⁄µÈø°∞‘ ¿««ÿ º˛πËµ»¥Ÿ." EOL
                      "Ω≈µµ¥¬ ¿ÂΩ√∞£¿« º˛πËøÕ «ÂΩ≈¿ª ≈Î«ÿ ƒ°∑·¿« »˚¿ª ∫Œø©πﬁ¥¬¥Ÿ." EOL
                      "ø§∏Æ∫Ù∑–¿∫ ∆Ú»≠¡÷¿«¿« ±≥¿«∏¶ ≥ª∞…∞Ì¥¬ ¿÷¡ˆ∏∏" EOL
                      "ªÁæ««— »˚∞˙¿« º∫¿¸ø° ¿”«œ¥¬ ªÁ∂˜ø° ¥Î«ÿº≠¥¬ ¿¸≈ı∏¶ ¿˚±ÿ «„øÎ «—¥Ÿ." EOL
                      "ø§∏Æ∫Ù∑–¿∫ π´±‚¿« ∫¿≥≥¿ª ≥Ù∞‘ ∆Ú∞°«—¥Ÿ.";
#else
        description = "Elyvilon the Healer is worshipped by the healers (among others), who gain" EOL
                      "their healing powers by long worship and devotion. Although Elyvilon prefers" EOL
                      "a creed of pacifism, those who crusade against evil are not excluded. Elyvilon" EOL
                      "appreciates the offering of weapons. ";
#endif
        break;

    default:
#ifdef JP
        description = "«¡∑Œ±◊∑• πˆ±◊¿« Ω≈¿∫ ¿ÃªÛ«œ∞Ì ¿ß«Ë«— Ω≈¿∏∑Œ" EOL
                      "±◊¿« ¡∏¿Á¥¬ ∏∂∂•»˜ ∞≥πﬂ¿⁄ø°∞‘ ∫∏∞Ìµ«æÓæﬂ «—¥Ÿ.";
#else
        description = "God of Program Bugs is a weird and dangerous God and his presence should" EOL
                      "be reported to dev-team.";
#endif
    }

    cprintf(description);
    //end of printing description

    // title only shown for our own god
    if (you.religion == which_god)
    {
        //mv: print title based on piety
#ifdef JP
        cprintf( EOL EOL "ƒ™»£  - " );
#else
        cprintf( EOL EOL "Title - " );
#endif
        textcolor(colour);

        // mv: if your piety is high enough you get title
        // based on your god
        if (you.piety > 160)
        {
#ifdef JP
            cprintf((which_god == GOD_SHINING_ONE) ? "¡˙º≠¿« √®««æ" :
                    (which_god == GOD_ZIN) ? "º∫Ω∫∑ØøÓ ¿¸ªÁ" :
                    (which_god == GOD_ELYVILON) ? "∫˚¿« √®««æ" :
                    (which_god == GOD_OKAWARU) ? "ºˆ√µ¿« ΩŒøÚ¿« ¡ˆπË¿⁄" :
                    (which_god == GOD_YREDELEMNUL) ? "øµø¯«— ¡◊¿Ω¿« ¡ˆπË¿⁄" :
                    (which_god == GOD_KIKUBAAQUDGHA) ? "æœ»Ê¿« øµ¡÷" :
                    (which_god == GOD_MAKHLEB) ? "»•µ∑¿« ¥Î∫Ø¿⁄" :
                    (which_god == GOD_VEHUMET) ? "∆ƒ±´¿« ø’" :
                    (which_god == GOD_TROG) ? "¿ß¥Î«— ªÏ¿∞¿⁄" :
                    (which_god == GOD_NEMELEX_XOBEH) ? "¿ß¥Î«— ∆Æ∏ØΩ∫≈Õ" :
                    (which_god == GOD_SIF_MUNA) ? "∫Òº˙¿« ¡ˆπË¿⁄" :
                    (which_god == GOD_XOM) ? "≈◊µ ∫£æÓ" :
                        "πˆ±◊¿« ø’ ∫∏±‚"); // Xom and no god is handled before
#else
            cprintf((which_god == GOD_SHINING_ONE) ? "Champion of Law" :
                    (which_god == GOD_ZIN) ? "Divine Warrior" :
                    (which_god == GOD_ELYVILON) ? "Champion of Light" :
                    (which_god == GOD_OKAWARU) ? "Master of Thousand Battles" :
                    (which_god == GOD_YREDELEMNUL) ? "Master of Eternal Death" :
                    (which_god == GOD_KIKUBAAQUDGHA) ? "Lord of Darkness" :
                    (which_god == GOD_MAKHLEB) ? "Champion of Chaos" :
                    (which_god == GOD_VEHUMET) ? "Lord of Destruction" :
                    (which_god == GOD_TROG) ? "Great Slayer" :
                    (which_god == GOD_NEMELEX_XOBEH) ? "Great Trickster" :
                    (which_god == GOD_SIF_MUNA) ? "Master of Arcane" :
                    (which_god == GOD_XOM) ? "Teddy Bear" :
                        "Bogy the Lord of the Bugs"); // Xom and no god is handled before
#endif
        }
        else
        {
            //mv: most titles are still universal - if any one wants to
            //he might write specific titles for all gods or rewrite current
            //ones (I know they are not perfect)
            //btw. titles are divided according to piety levels on which you get
            //new abilities.In the main it means - new ability = new title
            switch (which_god)
            {
            case GOD_ZIN:
            case GOD_SHINING_ONE:
            case GOD_KIKUBAAQUDGHA:
            case GOD_YREDELEMNUL:
            case GOD_VEHUMET:
            case GOD_OKAWARU:
            case GOD_MAKHLEB:
            case GOD_SIF_MUNA:
            //mv: what about
            //sinner, believer, apprentice, disciple, adept, scholar, oracle
            case GOD_TROG:
            case GOD_NEMELEX_XOBEH:
            case GOD_ELYVILON:
#ifdef JP
                cprintf ( (you.piety >= 120) ? "¥ÎªÁ¡¶" :
                          (you.piety >= 100) ? "¿Â∑Œ" :
                          (you.piety >=  75) ? "ªÁ¡¶" :
                          (you.piety >=  50) ? "∫Œ¡¶" :
                          (you.piety >=  30) ? "ºˆ∑√¿⁄" :
                          (you.piety >    5) ? "Ω≈¿⁄"
                                             : "¡À¿Œ" );
#else
                cprintf ( (you.piety >= 120) ? "High Priest" :
                          (you.piety >= 100) ? "Elder" :
                          (you.piety >=  75) ? "Priest" :
                          (you.piety >=  50) ? "Deacon" :
                          (you.piety >=  30) ? "Novice" :
                          (you.piety >    5) ? "Believer"
                                             : "Sinner" );
#endif
                break;

            case GOD_XOM:
#ifdef JP
                cprintf( (you.experience_level >= 20) ? "¡ª¿Ã ¡¡æ∆«œ¥¬ ¿Â≥≠∞®"
                                                      : "¿Â≥≠∞®" );
#else
                cprintf( (you.experience_level >= 20) ? "Xom's favourite toy"
                                                      : "Toy" );
#endif
            break;

            default:
#ifdef JP
                cprintf ("πˆ±◊");
#else
                cprintf ("Bug");
#endif
            }
        }
    }
    // end of print title

    // mv: now let's print favor as Brent suggested
    // I know these messages aren't perfect so if you can
    // think up something better, do it

    textcolor(LIGHTGRAY);
#ifdef JP
    cprintf(EOL EOL "»£¿«  - ");
#else
    cprintf(EOL EOL "Favour - ");
#endif
    textcolor(colour);

    //mv: player is praying at altar without appropriate religion
    //it means player isn't checking his own religion and so we only
    //display favour and will go out
    if (you.religion != which_god)
    {
        textcolor (colour);
        snprintf( info, INFO_SIZE,
#ifdef JP //∞À≈‰ - ±∏¿Ã - ø™Ω√...
                 (you.penance[which_god] >= 50) ? "%s¿« ∫–≥Î∞° ≥ ø°∞‘ «‚«œ∞Ì ¿÷¥Ÿ!" :
                 (you.penance[which_god] >= 20) ? "%s¿∫(¥¬) ≥ ø°∞‘ »≠∞° ≥≠ ªÛ≈¬¥Ÿ." :
                 (you.penance[which_god] >=  5) ? "%s¿∫(¥¬) ≥ ¿« ¡À∏¶ ¿ﬂ æÀ∞Ì ¿÷¥Ÿ." :
                 (you.penance[which_god] >   0) ? "%s¿∫(¥¬) ≥ ¿« ¡À∏¶ øÎº≠«“ ¡ÿ∫Ò∞° µ«æÓ¿÷¥Ÿ." :
                 (you.worshipped[which_god])    ? "%s¿∫(¥¬) ¥ÁΩ≈∞˙ ªÛπ›µ» ¬ ¿Ã¥Ÿ."
                                                : "%s¿∫(¥¬) ¥ÁΩ≈∞˙ ¡ﬂ∏≥¿Œ ¬ ¿Ã¥Ÿ.",
                 god_name(which_god) );
#else
                 (you.penance[which_god] >= 50) ? "%s's wrath is upon you!" :
                 (you.penance[which_god] >= 20) ? "%s is annoyed with you." :
                 (you.penance[which_god] >=  5) ? "%s well remembers your sins." :
                 (you.penance[which_god] >   0) ? "%s is ready to forgive your sins." :
                 (you.worshipped[which_god])    ? "%s is ambivalent towards you."
                                                : "%s is neutral towards you.",
                 god_name(which_god) );
#endif

        cprintf(info);
    }
    else
    {
        if (player_under_penance()) //mv: penance check
        {
#ifdef JP
            cprintf( (you.penance[which_god] >= 50) ? "Ω≈¿Ã ¥ÁΩ≈ø°∞‘ ∫–≥Î«œ∞Ì ¿÷¥Ÿ!" :
                     (you.penance[which_god] >= 20) ? "¥ÁΩ≈¿∫ Ω≈¿« ±Ê∞˙ Ω…∞¢«œ∞‘ æÓ±ﬂ≥µ¥Ÿ. ¬¸»∏«œ∂Û! " :
                     (you.penance[which_god] >= 5 ) ? "¥ÁΩ≈¿∫ ¡ˆ±› ¬¸»∏¡ﬂ¿Ã¥Ÿ."
                                                    : "¥ÁΩ≈¿∫ ¡∂±› ¥ı ¿⁄¡¶«œ∞Ì ¿÷¿Ω¿ª ≥™≈∏≥ªæﬂ «—¥Ÿ." );
#else
            cprintf( (you.penance[which_god] >= 50) ? "Godly wrath is upon you!" :
                     (you.penance[which_god] >= 20) ? "You've transgressed heavily! Be penitent!" :
                     (you.penance[which_god] >= 5 ) ? "You are under penance."
                                                    : "You should show more discipline." );
#endif

        }
        else
        {
            if (which_god == GOD_XOM)
#ifdef JP
                cprintf("¥ÁΩ≈¿∫ π´Ω√¥Á«œ∞Ì ¿÷¥Ÿ.");
#else
                cprintf("You are ignored.");
#endif
            else
            {
                snprintf( info, INFO_SIZE,

#ifdef JP  //∞À≈‰ - ±∏¿Ã
                         (you.piety > 130) ? "¥ÁΩ≈¿∫ %s¿« »«∏¢«— »≠Ω≈¿Ã¥Ÿ.":
                         (you.piety > 100) ? "%s¿« ¥´ø° ¥ÁΩ≈¿∫ ∫˚≥™¥¬ ∫∞¿Ã¥Ÿ." :
                         (you.piety >  70) ? "%s¿« ¥´ø° ¥ÁΩ≈¿∫ ∂∞ø¿∏£¥¬ ∫∞¿Ã¥Ÿ." :
                         (you.piety >  40) ? "%s¿∫(¥¬) ¥ÁΩ≈¿ª »Ì¡∑«œ∞‘ ª˝∞¢«—¥Ÿ." :
                         (you.piety >  20) ? "¥ÁΩ≈¿∫ %sø°∞‘ æÓ¥¿¡§µµ ¿Œ¡§πﬁ∞Ì ¿÷¥Ÿ." :
                         (you.piety >   5) ? "%s¿∫(¥¬) ¥ÁΩ≈∞˙ æÓ¡§¬ƒ«— ªÛ≈¬¥Ÿ."
                                           : "¥ÁΩ≈¿∫ ∞¸Ω… π€¿Ã¥Ÿ.",
#else
                         (you.piety > 130) ? "A prized avatar of %s.":
                         (you.piety > 100) ? "A shining star in the eyes of %s." :
                         (you.piety >  70) ? "A rising star in the eyes of %s." :
                         (you.piety >  40) ? "%s is most pleased with you." :
                         (you.piety >  20) ? "%s has noted your presence." :
                         (you.piety >   5) ? "%s is noncommittal."
                                           : "You are beneath notice.",
#endif

                         god_name(which_god)
                       );

                cprintf(info);
            }
        }
        //end of favour

        //mv: following code shows abilities given from god (if any)


        textcolor(LIGHTGRAY);
#ifdef JP
        cprintf(EOL EOL "πﬁ¿∫ ¥…∑¬µÈ :                                                     (∫ÒøÎ)" EOL);
#else
        cprintf(EOL EOL "Granted powers :                                                         (Cost)" EOL);
#endif
        textcolor(colour);


        // mv: these gods protects you during your prayer (not mentioning XOM)
        // chance for doing so is (random2(you.piety) >= 30)
        // Note that it's not depending on penance.
        // Btw. I'm not sure how to explain such divine protection
        // because god isn't really protecting player - he only sometimes
        // saves his life (probably it shouldn't be displayed at all).
        // What about this ?
        if ((which_god == GOD_ZIN
                || which_god == GOD_SHINING_ONE
                || which_god == GOD_ELYVILON
                || which_god == GOD_OKAWARU
                || which_god == GOD_YREDELEMNUL)
            && you.piety >= 30)
        {
            snprintf( info, INFO_SIZE,
#ifdef JP //∞À≈‰ - ±∏¿Ã - ∏∂¬˘∞°¡ˆ∑Œ
                      "%s¿∫(¥¬) ¥ÁΩ≈¿Ã ±‚µµ«œ¥¬ µøæ» ¥ÁΩ≈¿ª %s √ƒ¥Ÿ∫ª¥Ÿ." EOL,
                      god_name(which_god),
                      (you.piety >= 150) ? "¡÷¿« ±Ì∞‘":   // > 4/5
                      (you.piety >=  90) ? "¡æ¡æ": // > 2/3
                                           "∂ß∂ß∑Œ"    // less than 2:3
#else
                      "%s %s watches over you during prayer." EOL,
                      god_name(which_god),
                      (you.piety >= 150) ? "carefully":   // > 4/5
                      (you.piety >=  90) ? "often" :      // > 2/3
                                           "sometimes"    // less than 2:3
#endif
                    );

            cprintf(info);
        }

        // mv: No abilities (except divine protection)
        // under penance (fix me if I'm wrong)
        if (player_under_penance())
        {
#ifdef JP
            cprintf( "æ¯¿Ω." EOL );
#else
            cprintf( "None." EOL );
#endif
        }
        else
        {
            switch (which_god) //mv: finaly let's print abilities
            {
            case GOD_ZIN:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_ZIN_REPEL_UNDEAD );
                else
#ifdef JP
                    cprintf( "æ¯¿Ω." EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_ZIN_HEALING );

                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_ZIN_PESTILENCE );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_ZIN_HOLY_WORD );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_ZIN_SUMMON_GUARDIAN );
                break;

            case GOD_SHINING_ONE:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_TSO_REPEL_UNDEAD );
                else
#ifdef JP
                    cprintf( "æ¯¿Ω." EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_TSO_SMITING );

                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_TSO_ANNIHILATE_UNDEAD );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_TSO_THUNDERBOLT );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_TSO_SUMMON_DAEVA );
                break;

            case GOD_KIKUBAAQUDGHA:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_KIKU_RECALL_UNDEAD_SLAVES );
                else
#ifdef JP
                    cprintf( "æ¯¿Ω." EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
#ifdef JP
                    cprintf("¥ÁΩ≈¿∫ ¡◊¿Ω∏∂π˝¿« ∫Œ¿€øÎ¿∏∑Œ∫Œ≈Õ ∫∏»£µ«∞Ì ¿÷¥Ÿ." EOL);

#else
                    cprintf("You are protected from some of the side-effects of death magic." EOL);

#endif
                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_KIKU_ENSLAVE_UNDEAD );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_KIKU_INVOKE_DEATH );
                break;

            case GOD_YREDELEMNUL:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_YRED_ANIMATE_CORPSE );
                else
#ifdef JP
                    cprintf( "æ¯¿Ω." EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_YRED_RECALL_UNDEAD );

                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_YRED_ANIMATE_DEAD );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_YRED_DRAIN_LIFE );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_YRED_CONTROL_UNDEAD );
                break;


            case GOD_VEHUMET:
                if (you.piety >= 30)
                {
#ifdef JP
                    cprintf( "∫£»ƒ∏‰¿« ¿Ã∏ß¿∏∑Œ ¡˜¡¢ ªÏª˝¿ª «œ∞≈≥™ ∫Œ«œ∏¶ Ω√ƒ—º≠ ¡◊¿Ã∏È " EOL
                             "                                       ∏∂≥™∏¶ πﬁ¿ª ºˆ ¿÷¥Ÿ. " EOL );
#else
                    cprintf( "You can gain power from the those you kill " EOL
                             "   in Vehumet's name, or those slain by your servants." EOL );
#endif
                }
                else
#ifdef JP
                    cprintf( "æ¯¿Ω." EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
#ifdef JP
                    cprintf( "±‚µµ¡ﬂø°¥¬ ∫£»ƒ∏‰¿Ã ∆ƒ±´¿˚¿Œ ∏∂π˝¿ª µµøÕ¡ÿ¥Ÿ." EOL );
#else
                    cprintf( "Vehumet assists with destructive magics during prayer." EOL );
#endif

                if (you.piety >= 75)
#ifdef JP
                    cprintf( "±‚µµ¡ﬂø°¥¬ º“»Øµ» ≈©∏Æ√ƒø° ¥Î«œø© πÊæÓ∑¬¿Ã ª˝±‰¥Ÿ." EOL );
#else
                    cprintf( "During prayer you have some protection from summoned creatures." EOL );
#endif

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_VEHUMET_CHANNEL_ENERGY );
                break;


            case GOD_OKAWARU:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_OKAWARU_MIGHT );
                else
#ifdef JP
                    cprintf( "æ¯¿Ω." EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_OKAWARU_HEALING );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_OKAWARU_HASTE );
                break;

            case GOD_MAKHLEB:
                if (you.piety >= 30)
                {
#ifdef JP
                    cprintf( "∏∂≈©∑π∫Í¿« ¿Ã∏ß¿∏∑Œ ªÏª˝¿ª «œ∏È ±◊ ¡◊¿Ω¿∏∑Œ∫Œ≈Õ ∏∂≥™∏¶ πﬁ¿ª ºˆ ¿÷¥Ÿ." EOL );
#else
                    cprintf( "You can gain power from the deaths " EOL
                             "   of those you kill in Makhleb's name." EOL );
#endif
                }
                else
#ifdef JP
                    cprintf( "æ¯¿Ω." EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_MAKHLEB_MINOR_DESTRUCTION );

                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_MAKHLEB_LESSER_SERVANT_OF_MAKHLEB );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_MAKHLEB_MAJOR_DESTRUCTION );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_MAKHLEB_GREATER_SERVANT_OF_MAKHLEB );
                break;

            case GOD_SIF_MUNA:
                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_SIF_MUNA_FORGET_SPELL );
                else
#ifdef JP
                    cprintf( "æ¯¿Ω." EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 100)
#ifdef JP
                    cprintf( "¡÷πÆ Ω√¿¸¿« ∏Ó∏Ó ∫Œ∞°»ø∞˙∑Œ∫Œ≈Õ ∫∏»£πﬁ∞Ì ¿÷¥Ÿ." EOL );
#else
                    cprintf( "You are protected from some side-effects of spellcasting." EOL );
#endif
                break;

            case GOD_TROG:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_TROG_BERSERK );
                else
#ifdef JP
                    cprintf( "æ¯¿Ω." EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_TROG_MIGHT );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_TROG_HASTE_SELF );
                break;

            case GOD_ELYVILON:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_ELYVILON_LESSER_HEALING );
                else
#ifdef JP
                    cprintf( "æ¯¿Ω." EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_ELYVILON_PURIFICATION );

                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_ELYVILON_HEALING );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_ELYVILON_RESTORATION );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_ELYVILON_GREATER_HEALING );
                break;

            default:   //mv: default is Xom, Nemelex and all bugs.
#ifdef JP
                cprintf( "æ¯¿Ω." EOL );
#else
                cprintf( "None." EOL );
#endif
            } //end of printing abilities
        }
    }


end_god_info: //end of everything (life, world, universe etc.)

    set_keyin_mode(KEYIN_MODE_MORE);
    getch(); // wait until keypressed
    set_keyin_mode(KEYIN_MODE_NONE);

#ifdef DOS_TERM //mv: if DOS_TERM is defined than buffer is returned to screen
                //if not redraw_screen() is called everytime when this function is
                //called
    puttext(1, 1, 80, 25, buffer);
    window(1, 1, 80, 25);
#endif
}          //mv: That's all folks.
