/*
 *  File:       randart.cc
 *  Summary:    Random and unrandom artifact functions.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *   <8>     19 Jun 99   GDL    added IBMCPP support
 *   <7>     14/12/99    LRH    random2 -> random5
 *   <6>     11/06/99    cdl    random4 -> random2
 *
 *   <1>     -/--/--     LRH    Created
 */

#include "AppHdr.h"
#include "randart.h"

#include <string.h>
#include <stdio.h>

#include "externs.h"
#include "itemname.h"
#include "stuff.h"
#include "wpn-misc.h"

/*
   The initial generation of a randart is very simple - it occurs
   in dungeon.cc and consists of giving it a few random things - plus & plus2
   mainly.
*/
const char *rand_wpn_names[] = {
#ifdef JP
    "Ç÷·ùÀÇ ",
    "Á×À½ÀÇ ",
    "Âü»ìÀÇ ",
    "°íÅëÀÇ ",
    "¹Î»çÀÇ ",
    "Á×À½°ú °íÅëÀÇ ",
    "¹«ÇÑÇÑ ¾÷¹úÀÇ ",
    "¿µ¿øÇÑ Åë°íÀÇ ",
    "±Ç´ÉÀÇ ",
    "ºÐ³ëÀÇ ",
#else
    " of Blood",
    " of Death",
    " of Bloody Death",
    " of Pain",
    " of Painful Death",
    " of Pain & Death",
    " of Infinite Pain",
    " of Eternal Torment",
    " of Power",
    " of Wrath",
#endif
/* 10: */
#ifdef JP
    "ÆÄ¸êÀÇ ",
    "¿¬¾àÇÑ ÀÚºñÀÇ ",
    "¹¬½Ã·ÏÀÇ ",
    "¾î¸´±¤´ëÀÇ ",
    "¹ÝÁöÀÇ ",
    "¹Ùº¸ÀÇ ",
    "½ÅµéÀÇ ",
    "Á¦±¹ÀÇ ",
    "ÆÄ±«ÀÇ ",
    "¼±¾ÇÀÇ Á¾°áÀüÀÇ ",
#else
    " of Doom",
    " of Tender Mercy",
    " of the Apocalypse",
    " of the Jester",
    " of the Ring",
    " of the Fool",
    " of the Gods",
    " of the Imperium",
    " of Destruction",
    " of Armageddon",
#endif
/* 20: */
#ifdef JP
    "¹«ÀÚºñÇÑ Á¤ÀÇÀÇ ",
    "Á¤ÀÇÀÇ ºÐ³ëÀÇ ",
    "¿©·ÂÀÇ ",
    "º¸¼®ÀÇ ",
    "¸¶Å©·¹ºêÀÇ ",
    "Æ®·Î±×ÀÇ ",
    "¡‹ÀÇ ",
    "°í´ëÀÇ ",
    "¸¶³ªÀÇ ",
    "³×¸Þ·º½ºÀÇ ",
#else
    " of Cruel Justice",
    " of Righteous Anger",
    " of Might",
    " of the Orb",
    " of Makhleb",
    " of Trog",
    " of Xom",
    " of the Ancients",
    " of Mana",
    " of Nemelex Xobeh",
#endif
/* 30: */
#ifdef JP
    "¸¶µµ»çÀÇ ",
    "´ë¸¶µµ»çÀÇ ",
    "¿ÕÀÇ ",
    "¿©¿ÕÀÇ ",
    "Ãµ±¸ÀÇ ",
    "¿øÈ¯ÀÇ ",
    "Ç÷Á·ÀÇ ",
    "ÅõÀïÀÇ ",
    "ÀüÅõÀÇ ",
    "¸í¿¹ÀÇ ",
#else
    " of the Magi",
    " of the Archmagi",
    " of the King",
    " of the Queen",
    " of the Spheres",
    " of Circularity",
    " of Linearity",
    " of Conflict",
    " of Battle",
    " of Honour",
#endif
/* 40: */
#ifdef JP
    "³ªºñÀÇ ",
    "¸»¹úÀÇ ",
    "°³±¸¸®ÀÇ ",
    "Á·Á¦ºñÀÇ ",
    "¾ß¸¸ÀÎÀÇ ",
    "Áã¸ç´À¸®ÀÇ ",
    "¿øÁËÀÇ ",
    "º¹¼öÀÇ ",
    "Ã³ÇüÀÇ ",
    "Á¶Á¤ÀÇ ",
#else
    " of the Butterfly",
    " of the Wasp",
    " of the Frog",
    " of the Weasel",
    " of the Troglodytes",
    " of the Pill-Bug",
    " of Sin",
    " of Vengeance",
    " of Execution",
    " of Arbitration",
#endif
/* 50: */
#ifdef JP
    "Å½±¸ÀÚÀÇ ",
    "Áø½ÇÀÇ ",
    "°ÅÁþÀÇ ",
    "°¡ÁöÀÇ ",
    "¼ø¹«ÀÇ ",
    "±âÈ¸ÀÇ ",
    "ÁÖÀúÀÇ ",
    "Áö¿ÁÀÇ ºÐ³ëÀÇ ",
    "¾ðµ¥µåÀÇ ",
    "È¥µ·ÀÇ ",
#else
    " of the Seeker",
    " of Truth",
    " of Lies",
    " of the Eggplant",
    " of the Turnip",
    " of Chance",
    " of Curses",
    " of Hell's Wrath",
    " of the Undead",
    " of Chaos",
#endif
/* 60: */
#ifdef JP
    "Áú¼­ÀÇ ",
    "»ý¸íÀÇ ",
    "±¸¼¼°èÀÇ ",
    "½Å¼¼°èÀÇ ",
    "Áß½É±¹ÀÇ ",
    "¹èÈ¸ÀÇ ",
    "ºÒÄèÀÇ ",
    "ºñÅºÀÇ ",
    "ÀÜÀÎÇÑ º¸º¹ÀÇ ",
    "Á¤º¹ÀÇ ",
#else
    " of Law",
    " of Life",
    " of the Old World",
    " of the New World",
    " of the Middle World",
    " of Crawl",
    " of Unpleasantness",
    " of Discomfort",
    " of Brutal Revenge",
    " of Triumph",
#endif
/* 70: */
#ifdef JP
    "ÇØºÎÀÇ ",
    "Àý´ÜÀÇ ",
    "ÀüÀ²ÀÇ ",
    "°øÆ÷ÀÇ ",
    "±àÁöÀÇ ",
    "È­»êÀÇ ",
    "ÇÇ¿¡ ±¾ÁÖ¸° ",
    "¾ç´ÜÀÇ ",
    "¿µ¿øÇÑ Á¶È­ÀÇ ",
    "ÆòÈ­ÀÇ ",
#else
    " of Evisceration",
    " of Dismemberment",
    " of Terror",
    " of Fear",
    " of Pride",
    " of the Volcano",
    " of Blood-Lust",
    " of Division",
    " of Eternal Harmony",
    " of Peace",
#endif
/* 80: */
#ifdef JP
    "ºü¸¥ Á×À½ÀÇ ",
    "Áï»çÀÇ ",
    "È°ÇÌÀÇ ",
    "¾îÀÇ ",
    "·Îºê½ºÅÍÀÇ ",
    "³ª»çÁ¶°³ÀÇ ",
    "Æë±ÏÀÇ ",
    "¹Ù´Ù¿À¸®ÀÇ ",
    "¹ö¼¸ÀÇ ",
    "µ¶¹ö¼¸ÀÇ ",
#else
    " of Quick Death",
    " of Instant Death",
    " of Misery",
    " of the Whale",
    " of the Lobster",
    " of the Whelk",
    " of the Penguin",
    " of the Puffin",
    " of the Mushroom",
    " of the Toadstool",
#endif
/* 90: */
#ifdef JP
    "¹é¼ºÀÇ ",
    "¸»ºÒ¹ö¼¸ÀÇ ",
    "Æ÷ÀÚÀÇ ",
    "ÃÖ¼±ÀÇ ",
    "ÆÄ·¹Åä ÃÖÀûÀÇ ",
    "°¡Àå Å« Çàº¹ÀÇ ",
    "¹«Áú¼­ÀÇ ",
    "¿À·¡µÈ ¾Ç·ÉÀÇ ",
    "Çõ¸íÀÇ ",
    "ÀÎ¹ÎÀÇ ",
#else
    " of the Little People",
    " of the Puffball",
    " of Spores",
    " of Optimality",
    " of Pareto-Optimality",
    " of Greatest Utility",
    " of Anarcho-Capitalism",
    " of Ancient Evil",
    " of the Revolution",
    " of the People",
#endif
/* 100: */
#ifdef JP
    "¿¤ÇÁµéÀÇ ",
    "µå¿öÇÁµéÀÇ ",
    "¿ÀÅ©µéÀÇ ",
    "ÀÎ°£µéÀÇ ",
    "ÁøÈëÀÇ ",
    "½Å¼ºÇÑ ¿ëÀÇ ",
    "Æ®·ÑµéÀÇ ",
    "¿À¿ì°ÅµéÀÇ  ",
    "°øÁ¤ÇÑ ÀçºÐ¹èÀÇ ",
    "ºÎÀÚÀÇ ",
#else
    " of the Elves",
    " of the Dwarves",
    " of the Orcs",
    " of the Humans",
    " of Sludge",
    " of the Naga",
    " of the Trolls",
    " of the Ogres",
    " of Equitable Redistribution",
    " of Wealth",
#endif
/* 110: */
#ifdef JP
    "ºóÅÐÅÍ¸®ÀÇ ",
    "Àç¹èºÐÀÇ ",
    "µ¡¾ø´Â ÆòÈ­ÀÇ ",
    "°­È­ÀÇ ",
    "¹Ì¸ðÀÇ ",
    "¹Î´ÞÆØÀÌÀÇ ",
    "´ÞÆØÀÌÀÇ ",
    "³ª»çÁ¶°³ÀÇ ",
    "Ã¼¹úÀÇ ",
    "±ØÇüÀÇ ",
#else
    " of Poverty",
    " of Reapportionment",
    " of Fragile Peace",
    " of Reinforcement",
    " of Beauty",
    " of the Slug",
    " of the Snail",
    " of the Gastropod",
    " of Corporal Punishment",
    " of Capital Punishment",
#endif
/* 120: */
#ifdef JP
    "Áü½ÂÀÇ ",
    "±¤¼±ÀÇ ",
    "¾ÏÈæÀÇ ",
    "ÇÞºµÀÇ ",
    "³· µ¿¾ÈÀÇ ",
    "¾ß¾ÏÀÇ ",
    "¹ã µ¿¾ÈÀÇ ",
    "¶¥°Å¹ÌÀÇ ",
    "È²È¥ÀÇ ",
    "»õº®ºûÀÇ ",
#else
    " of the Beast",
    " of Light",
    " of Darkness",
    " of Day",
    " of the Day",
    " of Night",
    " of the Night",
    " of Twilight",
    " of the Twilight",
    " of Dawn",
#endif
/* 130: */
#ifdef JP
    "»õº®ÀÇ ",
    "ÅÂ¾çÀÇ ",
    "´ÞÀÇ ",
    "¸Õ ¼¼°èÀÇ ",
    "º¸ÀÌÁö ¾Ê´Â ¿µ¿ªÀÇ ",
    "ÆÇµ¥¸ð´Ï¿òÀÇ ",
    "¾îºñ½ºÀÇ ",
    "ÀÎ°úÀÇ ",
    "±³µµ¼ÒÀÇ ",
    "½ÊÀÚ±ºÀÇ ",
#else
    " of the Dawn",
    " of the Sun",
    " of the Moon",
    " of Distant Worlds",
    " of the Unseen Realm",
    " of Pandemonium",
    " of the Abyss",
    " of the Nexus",
    " of the Gulag",
    " of the Crusades",
#endif
/* 140: */
#ifdef JP
    "À°¹ÚÀÇ ",
    "ÆÄ±«ÀÇ ",
    "À§±âÀÇ ",
    "¿µ¿øÇÑ Àü»çÀÇ ",
    "¿µ¿øÇÑ ½Î¿òÀÇ ",
    "»ç¾ÇÀÇ ",
    "³­Å¸ÀÇ ",
    "³ó·çÀÇ ",
    "Áúº´ÀÇ ",
    "Àç¾ÓÀÇ ",
#else
    " of Proximity",
    " of Wounding",
    " of Peril",
    " of the Eternal Warrior",
    " of the Eternal War",
    " of Evil",
    " of Pounding",
    " of Oozing Pus",
    " of Pestilence",
    " of Plague",
#endif
/* 150: */
#ifdef JP
    "ºÎÁ¤ÀÇ ",
    "±¸¼¼ÁÖÀÇ ",
    "°¨¿°ÀÇ ",
    "¹æ¾îÀÇ ",
    "º¸È£ÀÇ ",
    "°ø°Ý¿¡ ÀÇÇÑ ¹æ¾îÀÇ ",
    "°ø¸®ÀÇ ",
    "Á¶¸®ÀÇ ",
    "ºÎÁ¶¸®ÀÇ ",
    "¿ë±âÀÇ ",
#else
    " of Negation",
    " of the Saviour",
    " of Infection",
    " of Defence",
    " of Protection",
    " of Defence by Offence",
    " of Expedience",
    " of Reason",
    " of Unreason",
    " of the Heart",
#endif
/* 160: */
#ifdef JP
    "°ø¼¼ÀÇ ",
    "»ïÀÙÀÇ ",
    "¸¹Àº ÀÙÀÇ ",
    "°Ü¿ïÀÇ ",
    "¿©¸§ÀÇ ",
    "°¡À»ÀÇ ",
    "º½ÀÇ ",
    "ÇÑ ¿©¸§ÀÇ ",
    "ÇÑ °Ü¿ïÀÇ ",
    "¹à¾ÆÁöÁö ¾ÊÀº ¹ãÀÇ ",
#else
    " of Offence",
    " of the Leaf",
    " of Leaves",
    " of Winter",
    " of Summer",
    " of Autumn",
    " of Spring",
    " of Midsummer",
    " of Midwinter",
    " of Eternal Night",
#endif
/* 170: */
#ifdef JP
    "¾Æºñ±ÔÈ¯ÀÇ ",
    "²ÞÆ²°Å¸®´Â °ÍÀÇ ",
    "±â¾î°¡´Â °ÍÀÇ",
    "¹°Ã¼ XÀÇ ",
    "¡¸»ç°Ç¡¹ÀÇ ",
    "¹Ù´å¼ÓÀÇ ",
    "»ï¸²ÀÇ ",
    "³ª¹«µéÀÇ ",
    "´ëÁöÀÇ ",
    "»ï¶ó¸¸»óÀÇ ",
#else
    " of Shrieking Terror",
    " of the Lurker",
    " of the Crawling Thing",
    " of the Thing",
    "\"Thing\"",
    " of the Sea",
    " of the Forest",
    " of the Trees",
    " of Earth",
    " of the World",
#endif
/* 180: */
#ifdef JP
    "¼ºÂùÀÇ ",
    "¹°°ÅÇ°ÀÇ ",
    "¾Æ¸Þ¹ÙÀÇ ",
    "±âÇüÀÇ ",
    "ÁË¾ÇÀÇ ",
    "¼ø¼öÇÔÀÇ ",
    "Ãâ¼¼ÀÇ ",
    "Ãß¶ôÀÇ ",
    "À½¾ÇÀÇ ",
    "±¤¸íÀÇ ",
#else
    " of Bread",
    " of Yeast",
    " of the Amoeba",
    " of Deformation",
    " of Guilt",
    " of Innocence",
    " of Ascent",
    " of Descent",
    " of Music",
    " of Brilliance",
#endif
/* 190: */
#ifdef JP
    "Çø¿ÀÀÇ ",
    "Çâ¿¬ÀÇ ",
    "ÅÂ¾çºûÀÇ ",
    "º°ºûÀÇ ",
    "º°µéÀÇ ",
    "¸ÕÁöÀÇ ",
    "Ãµ±¹ÀÇ ",
    "ÇÏ´ÃÀÇ ",
    "Àí´õ¹ÌÀÇ ",
    "Á¡ÂøÀÇ ",
#else
    " of Disgust",
    " of Feasting",
    " of Sunlight",
    " of Starshine",
    " of the Stars",
    " of Dust",
    " of the Clouds",
    " of the Sky",
    " of Ash",
    " of Slime",
#endif
/* 200: */
#ifdef JP
    "¸íÄèÀÇ ",
    "²÷ÀÓ¾ø´Â °æ°èÀÇ ",
    "°áÀÇÀÇ ",
    "³ª¹æÀÇ ",
    "»ýÁöÀÇ ",
    "°ßÀÎÀÇ ",
    "ÆòÇüÀÇ ",
    "±ÕÇüÀÇ ",
    "ºÒ±ÕÇüÀÇ ",
    "Á¶È­ÀÇ ",
#else
    " of Clarity",
    " of Eternal Vigilance",
    " of Purpose",
    " of the Moth",
    " of the Goat",
    " of Fortitude",
    " of Equivalence",
    " of Balance",
    " of Unbalance",
    " of Harmony",
#endif
/* 210: */
#ifdef JP
    "ºÎÁ¶È­ÀÇ ",
    "È­¿° Áö¿ÁÀÇ ",
    "Á¾¸»Á¡ÀÇ ",
    "µî±ÍÀÇ ",
    "Àú¶ôÀÇ ",
    "°ø±ÞÀÇ ",
    "¼ö¿äÀÇ ",
    "GDPÀÇ ",
    "ºÎ´çÇÑ ÀÌµæÀÇ ",
    "ºÒ¹ý À¯Ä¡ÀÇ ",
#else
    " of Disharmony",
    " of the Inferno",
    " of the Omega Point",
    " of Inflation",
    " of Deflation",
    " of Supply",
    " of Demand",
    " of Gross Domestic Product",
    " of Unjust Enrichment",
    " of Detinue",
#endif
/* 220: */
#ifdef JP
    "ÀüÈ¯ÀÇ ",
    "¾ÈÅæÀÇ ",
    "Ä¢¼­ÀÇ ",
    "ÁÂÀýÀÇ ",
    "Ä§ÇØÀÇ ",
    "À§¹ÝÀÇ ",
    "Á¾°áÀÇ ",
    "±ÙÀýÀÇ ",
    "¼ÓÁËÀÇ ",
    "¹«ÁÖ¹°ÀÇ ",
#else
    " of Conversion",
    " of Anton Piller",
    " of Mandamus",
    " of Frustration",
    " of Breach",
    " of Fundamental Breach",
    " of Termination",
    " of Extermination",
    " of Satisfaction",
    " of Res Nullius",
#endif
/* 230: */
#ifdef JP
    "ºÀÅäÀÇ ",
    "¹«ÁÖÁö Á¡À¯ÀÇ ",
    "¸Æ¶ôÀÇ ",
    "±ÔÄ¢ÀÇ ",
    "ÀÚÀ¯ º¸À¯ÀÇ ",
    "ºÒ¹ý ÀÛÀ§ÀÇ ",
    "ºÎÀÛÀ§ÀÇ ",
    "°ú½ÇÀÇ ",
    "Çü¹úÀÇ ",
    "¿À¸íÀÇ ",
#else
    " of Fee Simple",
    " of Terra Nullius",
    " of Context",
    " of Prescription",
    " of Freehold",
    " of Tortfeasance",
    " of Omission",
    " of Negligence",
    " of Pains",
    " of Attainder",
#endif
/* 240: */
#ifdef JP
    "È°¹ßÀÇ ",
    "ÈÞ¸éÀÇ ",
    "ÆóÁöÀÇ ",
    "¹æÄ¡ÀÇ ",
    "»ç¸·ÀÇ ",
    "¿ø½Ã¸²ÀÇ ",
    "¹ÌÄ¡±¤ÀÌÀÇ ",
    "ºÒ¾ÈÀÇ ",
    "Æí½ÉÀÇ ",
    "ÇâÀÀÀÇ ",
#else
    " of Action",
    " of Inaction",
    " of Truncation",
    " of Defenestration",
    " of Desertification",
    " of the Wilderness",
    " of Psychosis",
    " of Neurosis",
    " of Fixation",
    " of the Open Hand",
#endif
/* 250: */
#ifdef JP
    "ÀÌ»¡ÀÇ ",
    "¼º½ÇÀÇ ",
    "ºÎ½ÇÇÑ ",
    "½ÅµéÀÇ °­Á¦ÀÇ ",
    "º¸ÀÌÁö ¾Ê´Â ¼ÕÀÇ ",
    "ÀÚ±â °áÁ¤ÀÇ ",
    "ÀÚÀ¯ÀÇ ",
    "¿¹¼ÓÀÇ ",
    "µ¶ÀçÀÇ ",
    "±ä¹ÚÀÇ ",
#else
    " of the Tooth",
    " of Honesty",
    " of Dishonesty",
    " of Divine Compulsion",
    " of the Invisible Hand",
    " of Freedom",
    " of Liberty",
    " of Servitude",
    " of Domination",
    " of Tension",
#endif
/* 260: */
#ifdef JP
    "ÀÏ½Å±³ÀÇ ",
    "ºÒ°æÀÇ ",
    "¾Ë ¼ö ¾øÀ½ÀÇ ",
    "½ÇÁ¸ÀÇ ",
    "¼±ÇÔÀÇ ",
    "»ó´ëÀÇ ",
    "Àý´ëÀÇ ",
    "»çÁËÀÇ ",
    "±Ý¿åÀÇ ",
    "Áõ¿ÀÀÇ ",
#else
    " of Monotheism",
    " of Atheism",
    " of Agnosticism",
    " of Existentialism",
    " of the Good",
    " of Relativism",
    " of Absolutism",
    " of Absolution",
    " of Abstinence",
    " of Abomination",
#endif
/* 270: */
#ifdef JP
    "ÀýÁ¦ÀÇ ",
    "¿ïÇ÷ÀÇ ",
    "ºÒ°¡»çÀÇÀÇ ",
    "Å¹À½ÀÇ ",
    "¹Ú¸íÀÇ ",
    "±¤ÈÖÀÇ ",
    "ºÎµµ´öÀÇ ",
    "¹«µµ´öÀÇ ",
    "Á¤¹ÐÇÑ ¼ö¼úÀÇ ",
    "Á¤Åë¼ºÀÇ ",
#else
    " of Mutilation",
    " of Stasis",
    " of Wonder",
    " of Dullness",
    " of Dim Light",
    " of the Shining Light",
    " of Immorality",
    " of Amorality",
    " of Precise Incision",
    " of Orthodoxy",
#endif
/* 280: */
#ifdef JP
    "½ÅÀÇÀÇ ",
    "ÇãÀ§ÀÇ ",
    "Á¡ÀïÀÌÀÇ ",
    "Ç³¼ö»çÀÇ ",
    "¿¹¾ðÀÚÀÇ ",
    "°­Å¸ÀÇ ",
    "Çõ½ÅÀÇ ",
    "À¯È²ÀÇ ",
    "¾ËÀÇ ",
    "ÃµÃ¼ÀÇ ",
#else
    " of Faith",
    " of Untruth",
    " of the Augurer",
    " of the Water Diviner",
    " of the Soothsayer",
    " of Punishment",
    " of Amelioration",
    " of Sulphur",
    " of the Egg",
    " of the Globe",
#endif
/* 290: */
#ifdef JP
    "ÃÊÀÇ ",
    "ÃÐ´ëÀÇ ",
    "ÈíÇ÷±ÍÀÇ ",
    "¿ÀÅ© Á·ÀÇ ",
    "È£ºñÆ® Á·ÀÇ ",
    "¼¼»óÀÇ ³¡ÀÇ ",
    "Çª¸¥ ÇÏ´ÃÀÇ ",
    "»¡°£ ÇÏ´ÃÀÇ ",
    "³ë¶õ ÇÏ´ÃÀÇ ",
    "º¸¶ùºû ÇÏ´ÃÀÇ ",
#else
    " of the Candle",
    " of the Candelabrum",
    " of the Vampires",
    " of the Orcs",
    " of the Halflings",
    " of World's End",
    " of Blue Skies",
    " of Red Skies",
    " of Orange Skies",
    " of Purple Skies",
#endif
/* 300: */
#ifdef JP
    "¼ÒÀ½ÀÇ ",
    "ÀÌ¼ºÀÇ ",
    "°Å¹ÌÀÇ ",
    "Ä¥¼ºÀå¾îÀÇ ",
    "¿øÃÊÀÇ ",
    "Á¾¸»ÀÇ ",
    "´ÜÀýÀÇ ",
    "Ãß¹æÀÇ ",
    "¾ÖµµÀÇ ",
    "Á×À½ÀÇ ¾Æ°¡¸®ÀÇ ",
#else
    " of Articulation",
    " of the Mind",
    " of the Spider",
    " of the Lamprey",
    " of the Beginning",
    " of the End",
    " of Severance",
    " of Sequestration",
    " of Mourning",
    " of Death's Door",
#endif
/* 310: */
#ifdef JP
    "¿­¼èÀÇ ",
    "ÁöÁøÀÇ ",
    "½ÇÆÐÀÇ ",
    "¼º°øÀÇ ",
    "°­¹ÚÀÇ ",
    "¸ð±âÀÇ ",
    "µî¿¡ÀÇ ",
    "½¬ÆÄ¸®ÀÇ ",
    "¹Ù´Ù °ÅºÏÀÌÀÇ ",
    "À°Áö °ÅºÏÀÌÀÇ ",
#else
    " of the Key",
    " of Earthquakes",
    " of Failure",
    " of Success",
    " of Intimidation",
    " of the Mosquito",
    " of the Gnat",
    " of the Blowfly",
    " of the Turtle",
    " of the Tortoise",
#endif
/* 320: */
#ifdef JP
    "È²ÃµÀÇ ",
    "¹¦ºñÀÇ ",
    "º¹Á¾ÀÇ ",
    "Áö¹èÀÇ ",
    "Àü·ÉÀÇ ",
    "°áÁ¤ÀÇ ",
    "Áß·ÂÀÇ ",
    "ºÎ·ÂÀÇ ",
    "ÁøÃ¢ÀÇ ",
    "ºÒÀÇÀÇ ½À°ÝÀÇ ",
#else
    " of the Pit",
    " of the Grave",
    " of Submission",
    " of Dominance",
    " of the Messenger",
    " of Crystal",
    " of Gravity",
    " of Levity",
    " of the Slorg",
    " of Surprise",
#endif
/* 330: */
#ifdef JP
    "¹Ì·ÎÀÇ ",
    "¹Ì±ÃÀÇ ",
    "½ÅÀû ÁßÀçÀÇ ",
    "¼øÈ¯ÀÇ ",
    "¹æÀûµ¹±âÀÇ ",
    "Àü°¥ÀÇ ",
    "¾Ç¸¶ Á·ÀÇ ",
    "ÃµÀçÀÇ ",
    "Ç÷¼®ÀÇ ",
    "±×·ÐÅçÀÇ ",
#else
    " of the Maze",
    " of the Labyrinth",
    " of Divine Intervention",
    " of Rotation",
    " of the Spinneret",
    " of the Scorpion",
    " of Demonkind",
    " of the Genius",
    " of Bloodstone",
    " of Grontol",
#endif
/* 340: */
#ifdef JP
    "¡¸±×¸² Åõ½º¡¹",
    "¡¸À§µµ ¸ÞÀÌÄ¿¡¹",
    "¡¸À§µµ¿ö ¸ÞÀÌÄ¿¡¹",
    "¡¸»ý¸íÀÇ ÆÄ¸ê¡¹",
    "¡¸½ÃÁß²Û¡¹",
    "¡¸Ãß¹æÀÚ¡¹",
    "¡¸°í¹®ÇÏ´Â »ç¶÷¡¹",
    "¡¸ºñ¹Ð º´±â¡¹",
    "¡¸±³¼öÇü¡¹",
    "¡¸°­³¶Äá¡¹",
#else
    " \"Grim Tooth\"",
    " \"Widowmaker\"",
    " \"Widowermaker\"",
    " \"Lifebane\"",
    " \"Conservator\"",
    " \"Banisher\"",
    " \"Tormentor\"",
    " \"Secret Weapon\"",
    " \"String\"",
    " \"Stringbean\"",
#endif
/* 350: */
#ifdef JP
    "¡¸ºê·Îºê¡¹",
    "¡¸±×·Îºê·ç½º¡¹",
    "¡¸ÇæÅ©¡¹",
    "¡¸ÂÞ±ÛÂÞ±Û ÇÒ¸Á±¸¡¹",
    "¡¸¼º¿ù¾ß¡¹",
    "¡¸°ÅÀÎÀÇ ÀÌ¾¥½Ã°³¡¹",
    "¡¸¿ìÀ¯ºÎ´Ü¡¹",
    "¡¸¾ÆÃ·²Û¡¹",
    "¡¸ÀÏ´ç¹é¡¹",
    "¡¸ºÒÆò ºÒ¸¸¡¹",
#else
    " \"Blob\"",
    " \"Globulus\"",
    " \"Hulk\"",
    " \"Raisin\"",
    " \"Starlight\"",
    " \"Giant's Toothpick\"",
    " \"Pendulum\"",
    " \"Backscratcher\"",
    " \"Brush\"",
    " \"Murmur\"",
#endif
/* 360: */
#ifdef JP
    "¡¸¼®°ü¡¹",
    "¡¸Á¶È­¡¹",
    "¡¸¿ëÀÇ Çô¡¹",
    "¡¸Á¶Á¤ÀÚ¡¹",
    "¡¸±×·¥¡¹",
    "¡¸±×·Ò¡¹",
    "¡¸±×¸²¡¹",
    "¡¸±×·ë¡¹",
    "¡¸µÚÀû°Å¸®´Â ÀÚ¡¹",
    "¡¸¿À¹É·¿¡¹",
#else
    " \"Sarcophage\"",
    " \"Concordance\"",
    " \"Dragon's Tongue\"",
    " \"Arbiter\"",
    " \"Gram\"",
    " \"Grom\"",
    " \"Grim\"",
    " \"Grum\"",
    " \"Rummage\"",
    " \"Omelette\"",
#endif
/* 370: */
#ifdef JP
    "¡¸¹Ì¼÷ÇÑ ÀÚ¡¹",
    "¡¸°¡Áö¡¹",
    "¡¸Z¡¹",
    "¡¸X¡¹",
    "¡¸Q¡¹",
    "¡¸OX¡¹",
    "¡¸ÀÓÁ¾ ½ÃÀÇ Ãµ½Ä¡¹",
    "¡¸°íÀÚÁúÀïÀÌ¡¹",
    "¡¸¸ÅÃá³à¡¹",
    "¡¸³ú¹°¡¹",
#else
    " \"Egg\"",
    " \"Aubergine\"",
    " \"Z\"",
    " \"X\"",
    " \"Q\"",
    " \"Ox\"",
    " \"Death Rattle\"",
    " \"Tattletale\"",
    " \"Fish\"",
    " \"Bung\"",
#endif
/* 380: */
#ifdef JP
    "¡¸¾Æ¸£Ä«³ª¡¹",
    "¡¸Á×À½ÀÇ ÁøÈë ¸¸µÎ¡¹",
    "¡¸¿µÈ¥ÀÇ À±È¸¡¹",
    "¡¸ÃÖÈÄÀÇ ÅëÃ¸¡¹",
    "¡¸Áö··ÀÌ¡¹",
    "¡¸¹ö·¯Áö¡¹",
    "¡¸¹ö·¯ÁöÀÇ ¿ª½À¡¹",
    "¡¸Á»ÀÇ ÀºÃÑ¡¹",
    "¡¸¿Ïº®ÇÑ Á¤´ä¡¹",
    "¡¸·¹À§±â¡¹",
#else
    " \"Arcanum\"",
    " \"Mud Pie of Death\"",
    " \"Transmigrator\"",
    " \"Ultimatum\"",
    " \"Earthworm\"",
    " \"Worm\"",
    " \"Worm's Wrath\"",
    " \"Xom's Favour\"",
    " \"Bingo\"",
    " \"Leviticus\"",
#endif
// Not yet possible...
/* 390: */
#ifdef JP
    "¡¸Äè¶ô »ìÀÎÀÚÀÇ¡¹",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#else
    " of Joyful Slaughter",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#endif

/* 390: */
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",

/* 340: */
#ifdef JP
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#else
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#endif

/* 200: */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

const char *rand_armour_names[] = {
/* 0: */
#ifdef JP
    "Â÷ÆóÀÇ ",
    "ÀºÃÑÀÇ ",
    "ºÒÄ§ÅõÀÇ ",
    "È¸ÀüÀÇ ",
    "»ý¸íÀÇ ",
    "¹æ¾îÀÇ ",
    "³ó´ãÀÇ ",
    "²÷ÀÓ¾ø´Â °æ°èÀÇ ",
    "ÈñÇÐÀÇ ",
    "È¯ÈñÀÇ ",
#else
    " of Shielding",
    " of Grace",
    " of Impermeability",
    " of the Onion",
    " of Life",
    " of Defence",
    " of Nonsense",
    " of Eternal Vigilance",
    " of Fun",
    " of Joy",
#endif
/* 10: */
#ifdef JP
    "Á×À½ÀÇ ¾Æ°¡¸®ÀÇ ",
    "¹®ÀÇ ",
    "°æÈ£ÀÇ ",
    "¿ÏÀü¼ºÀÇ ",
    "ÀüÃ¼ÀûÀÎ Á¶È­ÀÇ ",
    "Á¶È­ÀÇ ",
    "ºÒ°¡ÃËÃµ¹ÎÀÇ ",
    "Ãµ¹ÎÀÇ ",
    "ºñÃµÀÇ ",
    "¿ÀÅ¹ÀÇ ",
#else
    " of Death's Door",
    " of the Gate",
    " of Watchfulness",
    " of Integrity",
    " of Bodily Harmony",
    " of Harmony",
    " of the Untouchables",
    " of Grot",
    " of Grottiness",
    " of Filth",
#endif
/* 20: */
#ifdef JP
    "ÀÌ»óÇÔÀÇ ",
    "ÀÌ»óÇÑ ÈûÀÇ ",
    "ÈûÀÇ ",
    "ºê¶óµå °øÀÇ ",
    "¿µÈ¥ÀÇ ¿­¸ÅÀÇ ",
    "¹«ÀûÀÇ ",
    "¼û¹Ù²ÀÁúÀÇ ",
    "°ÌÀïÀÌÀÇ ",
    "±¸¼¼ÁÖÀÇ ",
    "À¯¿¬ÀÇ ",
#else
    " of Wonder",
    " of Wondrous Power",
    " of Power",
    " of Vlad",
    " of the Eternal Fruit",
    " of Invincibility",
    " of Hide-and-Seek",
    " of the Mouse",
    " of the Saviour",
    " of Plasticity",
#endif
/* 30: */
#ifdef JP
    "´ë¸Ó¸®ÀÇ ",
    "ÀüÀ²ÀÇ ",
    "¾Æ¸£Ä«³ªÀÇ ",
    "Á×À½¿¡ÀÇ Ç×ÀüÀÇ ",
    "¹«Åë°¢ÀÇ ",
    "¼öÈ£ÀÚÀÇ ",
    "½Å¼ººÒ°¡Ä§ÀÇ ",
    "¹Ù´Ù °ÅºÏÀÌÀÇ ",
    "À°Áö °ÅºÏÀÌÀÇ ",
    "¾Æ¸£¸¶µô·ÎÀÇ ",
#else
    " of Baldness",
    " of Terror",
    " of the Arcane",
    " of Resist Death",
    " of Anaesthesia",
    " of the Guardian",
    " of Inviolability",
    " of the Tortoise",
    " of the Turtle",
    " of the Armadillo",
#endif
/* 40: */
#ifdef JP
    "¹Ù´ÃµÎ´õÁöÀÇ ",
    "¹«ÀåÇÑ ÀÚÀÇ ",
    "ÃÊÀÚ¿¬ÀÇ ",
    "°ÝÁ¤ÀÇ ",
    "È£¿îÀÇ ",
    "¼ÕÇØÀÇ ",
    "º¸ÇèÀÇ ",
    "¹è»óÀÇ ",
    "Á¦¾àÀÇ ",
    "¹èÃ´ÀÇ ",
#else
    " of the Echidna",
    " of the Armoured One",
    " of Weirdness",
    " of Pathos",
    " of Serendipity",
    " of Loss",
    " of Hedging",
    " of Indemnity",
    " of Limitation",
    " of Exclusion",
#endif
/* 50: */
#ifdef JP
    "¹Ý¹ß·ÂÀÇ ",
    "¸»ÇØÁöÁö ¾ÊÀº ºñ¹ÐÀÇ ",
    "´ëÁöÀÇ ",
    "»êºñµÑ±âÀÇ ",
    "À¯ÇÑÇÑ Ã¥ÀÓÀÇ ",
    "Ã¥¹«ÀÇ ",
    "ÇÏÀõÀÇ ",
    "¿µ±¤ÀÇ ",
    "À¯ÁöÀÇ ",
    "º¸Á¸ÀÇ ",
#else
    " of Repulsion",
    " of Untold Secrets",
    " of the Earth",
    " of the Turtledove",
    " of Limited Liability",
    " of Responsibility",
    " of Hadjma",
    " of Glory",
    " of Preservation",
    " of Conservation",
#endif
/* 60: */
#ifdef JP
    "º¸È£¿Í ±¸¼ÓÀÇ ",
    "¹¬ºñÀÇ ",
    "ÁË¾ÇÀÇ ",
    "¹Ùº¸ÀÇ ",
    "¾ß¸¸ÀÇ ",
    "°­ÀÎÀÇ ",
    "°ø°£ÀÇ ",
    "Áø°øÀÇ ",
    "°¡¾ÐÀÇ ",
    "°¨¾ÐÀÇ ",
#else
    " of Protective Custody",
    " of the Clam",
    " of the Barnacle",
    " of the Lobster",
    " of Hairiness",
    " of Supple Strength",
    " of Space",
    " of the Vacuum",
    " of Compression",
    " of Decompression",
#endif

/* 70: */
#ifdef JP
    "¼ö¼¼¹ÌÀÇ ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#else
    " of the Loofah",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#endif
// Sarcophagus
};

#ifdef JP
    //50
    const char *rand_pre_names[] = {
    "ºÓÀº»ö ",      //»öÃ¤
    "´ÙÈ«»öÀÇ ",
    "Çª¸¥»ö ",
    "°¨Ã»»öÀÇ ",
    "³ì»öÀÇ ",
    "ÇÏ¾á ",
    "¹é¾Ç»öÀÇ ",
    "°ËÀº ",
    "Ä¥ÈæÀÇ ",
    "È¸»öÀÇ ",
    "¼öÁ¤ÀÇ ",    //ÀçÁú
    "ºñÃëÀÇ ",
    "È£¹ÚÀÇ ",
    "±Ý°­ÀÇ ",
    "°­Ã¶ÀÇ ",
    "ÀºÀÇ ",
    "È²±ÝÀÇ ",
    "ÇÇ·Î ¹°µé¿©Áø ",  //»óÅÂ¡¤¼ºÁú
    "¿À·¡µÈ ",
    "¾îµÎ¿î ",
    "»ç³ª¿î ",
    "³ÃÈ¤ÇÑ ",
    "ÀÜÀÎÇÑ ",
    "Á¶¿ëÇÑ ",
    "Àý¸ÁÀÇ ",
    "°í³úÀÇ ",
    "¹æÈ²ÇÏ´Â ",
    "±â´Ù¸®´Â ",
    "Àáµç ",
    "¹ÝÂ¦ÀÌ´Â ",
    "ºûÀÇ ",      //¼Ó¼º¡¤Àå¼Ò
    "º°ÀÇ ",
    "¸í°èÀÇ ",
    "È²ÃµÀÇ ",
    "Áö¿ÁÀÇ ",
    "È¥µ·ÀÇ ",
    "¾ÏÈæÀÇ ",
    "Ãµ°èÀÇ ",
    "¹¬½Ã·ÏÀÇ ",
    "¶¥°Å¹ÌÀÇ ",
    "¿î¸íÀÇ ",
    "ºÀÀÎÀÇ ",
    "ÀÌ°èÀÇ ",
    "¿¹¾ðÀÇ ",
    "Èä¿îÀÇ ",
    "µ¿¹æÀÇ ",
    "¼­¹æÀÇ ",
    "Á×À½ÀÇ ",
    "»ç¸ÁÀÇ ",
    "ºÒ»çÀÇ ",
    };

    //50
    const char *rand_suf_names[] = {
    "´«µ¿ÀÚ",    //À°Ã¼ÀÇ ÀÏºÎ µî
    "ÇÑÂÊ ´«",
    "¼Û°÷´Ï",
    "ÅÎ",
    "¼ÕÅé",
    "½ÉÀå",
    "³¯°³",
    "¾çÂÊ ³¯°³",
    "°Å¹Ì",  //»ý¹° µî
    "¹ì",
    "¸Å",
    "¾ß»ý ¸Å",
    "»ç³É°³",
    "ÀÌ¸®",
    "»çÀÚ",
    "È£¶ûÀÌ",
    "Ç¥¹ü",
    "¿Õ",    //¿ªÇÒ µî
    "±Í°øÀÚ",
    "¸ÍÁÖ",
    "ÀÍ»ì²Û",
    "¼ºÀÚ",
    "ÁËÀÎ",
    "°æºñ¿ø",
    "°æÈ£¿ø",
    "ÀÌ¾ß±â²Û",
    "Â¡Á¶",    //¹°°Ç µî
    "¹®Àå",
    "ÈÖÀå",
    "°¢ÀÎ",
    "ºñ¹®",
    "À¯¹°",
    "¼è»ç½½",
    "Æ÷È¿",
    "¼±À²",
    "Âù°¡",
    "°³¼±°¡",
    "Áúº´",
    "¼ºÂù",
    "¿µ·Â",  //ÀÇ»ç³ª ÀÇ½Ä µî
    "±âµµ",
    "¼­¿ø",
    "Ãàº¹",
    "ÁÖÀú",
    "ÈäÀå",
    "Ã³¹ú",
    "½ÉÆÇ",
    "°ø°í",
    "ºñÀû",
    "¼±¼­",
    };
#endif


// Remember: disallow unrandart creation in abyss/pan

/*
   The following unrandart bits were taken from $pellbinder's mon-util code
   (see mon-util.h & mon-util.cc) and modified (LRH). They're in randart.cc and
   not randart.h because they're only used in this code module.
*/

#if defined(MAC) || defined(__IBMCPP__) || defined(__BCPLUSPLUS__)
#define PACKED
#else
#define PACKED __attribute__ ((packed))
#endif

//int unranddatasize;

#ifdef __IBMCPP__
#pragma pack(push)
#pragma pack(1)
#endif

struct unrandart_entry
{
    const char *name;        // true name of unrandart (max 31 chars)
    const char *unid_name;   // un-id'd name of unrandart (max 31 chars)

    int ura_cl;        // class of ura
    int ura_ty;        // type of ura
    int ura_pl;        // plus of ura
    int ura_pl2;       // plus2 of ura
    int ura_col;       // colour of ura
    short prpty[RA_PROPERTIES];

    // special description added to 'v' command output (max 31 chars)
    const char *spec_descrip1;
    // special description added to 'v' command output (max 31 chars)
    const char *spec_descrip2;
    // special description added to 'v' command output (max 31 chars)
    const char *spec_descrip3;
};

#ifdef __IBMCPP__
#pragma pack(pop)
#endif

static struct unrandart_entry unranddata[] = {
#include "unrand.h"
};

char *art_n;
static FixedVector < char, NO_UNRANDARTS > unrandart_exist;

static int random5( int randmax );
static struct unrandart_entry *seekunrandart( const item_def &item );

static int random5( int randmax )
{
    if (randmax <= 0)
        return (0);

    //return rand() % randmax;
    return ((int) rand() / (RAND_MAX / randmax + 1));
    // must use random (not rand) for the predictable-results-from-known
    //  -srandom-seeds thing to work.
}

void set_unrandart_exist(int whun, char is_exist)
{
    unrandart_exist[whun] = is_exist;
}

char does_unrandart_exist(int whun)
{
    return (unrandart_exist[whun]);
}

// returns true is item is a pure randart or an unrandart
bool is_random_artefact( const item_def &item )
{
    return (item.flags & ISFLAG_ARTEFACT_MASK);
}

// returns true if item in an unrandart
bool is_unrandom_artefact( const item_def &item )
{
    return (item.flags & ISFLAG_UNRANDART);
}

// returns true if item is one of the origional fixed artefacts
bool is_fixed_artefact( const item_def &item )
{
    if (!is_random_artefact( item )
        && item.base_type == OBJ_WEAPONS
        && item.special >= SPWPN_SINGING_SWORD)
    {
        return (true);
    }

    return (false);
}

int get_unique_item_status( int base_type, int art )
{
    // Note: for weapons "art" is in item.special,
    //       for orbs it's the sub_type.
    if (base_type == OBJ_WEAPONS)
    {
        if (art >= SPWPN_SINGING_SWORD && art <= SPWPN_SWORD_OF_ZONGULDROK)
            return (you.unique_items[ art - SPWPN_SINGING_SWORD ]);
        else if (art >= SPWPN_SWORD_OF_POWER && art <= SPWPN_STAFF_OF_WUCAD_MU)
            return (you.unique_items[ art - SPWPN_SWORD_OF_POWER + 24 ]);
    }
    else if (base_type == OBJ_ORBS)
    {
        if (art >= 4 && art <= 19)
            return (you.unique_items[ art + 3 ]);

    }

    return (UNIQ_NOT_EXISTS);
}

void set_unique_item_status( int base_type, int art, int status )
{
    // Note: for weapons "art" is in item.special,
    //       for orbs it's the sub_type.
    if (base_type == OBJ_WEAPONS)
    {
        if (art >= SPWPN_SINGING_SWORD && art <= SPWPN_SWORD_OF_ZONGULDROK)
            you.unique_items[ art - SPWPN_SINGING_SWORD ] = status;
        else if (art >= SPWPN_SWORD_OF_POWER && art <= SPWPN_STAFF_OF_WUCAD_MU)
            you.unique_items[ art - SPWPN_SWORD_OF_POWER + 24 ] = status;
    }
    else if (base_type == OBJ_ORBS)
    {
        if (art >= 4 && art <= 19)
            you.unique_items[ art + 3 ] = status;

    }
}

static long calc_seed( const item_def &item )
{
    return (item.special & RANDART_SEED_MASK);
}

void randart_wpn_properties( const item_def &item,
                             FixedVector< char, RA_PROPERTIES > &proprt )
{
    ASSERT( is_random_artefact( item ) );

    const int aclass = item.base_type;
    const int atype  = item.sub_type;

    int i = 0;
    int power_level = 0;

    if (is_unrandom_artefact( item ))
    {
        struct unrandart_entry *unrand = seekunrandart( item );

        for (i = 0; i < RA_PROPERTIES; i++)
            proprt[i] = unrand->prpty[i];

        return;
    }

    // long seed = aclass * adam + atype * (aplus % 100) + aplus2 * 100;
    long seed = calc_seed( item );
    long randstore = rand();
    srand( seed );

    if (aclass == OBJ_ARMOUR)
        power_level = item.plus / 2 + 2;
    else if (aclass == OBJ_JEWELLERY)
        power_level = 1 + random5(3) + random5(2);
    else // OBJ_WEAPON
        power_level = item.plus / 3 + item.plus2 / 3;

    if (power_level < 0)
        power_level = 0;

    for (i = 0; i < RA_PROPERTIES; i++)
        proprt[i] = 0;

    if (aclass == OBJ_WEAPONS)  /* Only weapons get brands, of course */
    {
        proprt[RAP_BRAND] = SPWPN_FLAMING + random5(15);        /* brand */

        if (random5(6) == 0)
            proprt[RAP_BRAND] = SPWPN_FLAMING + random5(2);

        if (random5(6) == 0)
            proprt[RAP_BRAND] = SPWPN_ORC_SLAYING + random5(4);

        if (random5(6) == 0)
            proprt[RAP_BRAND] = SPWPN_VORPAL;

        if (proprt[RAP_BRAND] == SPWPN_FLAME
            || proprt[RAP_BRAND] == SPWPN_FROST)
        {
            proprt[RAP_BRAND] = 0;      /* missile wpns */
        }

        if (proprt[RAP_BRAND] == SPWPN_PROTECTION)
            proprt[RAP_BRAND] = 0;      /* no protection */

        if (proprt[RAP_BRAND] == SPWPN_DISRUPTION
            && !(atype == WPN_MACE || atype == WPN_GREAT_MACE
                || atype == WPN_HAMMER))
        {
            proprt[RAP_BRAND] = SPWPN_NORMAL;
        }

        // is this happens, things might get broken -- bwr
        if (proprt[RAP_BRAND] == SPWPN_SPEED && atype == WPN_QUICK_BLADE)
            proprt[RAP_BRAND] = SPWPN_NORMAL;

        if (launches_things(atype))
        {
            proprt[RAP_BRAND] = SPWPN_NORMAL;

            if (random5(3) == 0)
            {
                int tmp = random5(20);

                proprt[RAP_BRAND] = (tmp >= 18) ? SPWPN_SPEED :
                                    (tmp >= 14) ? SPWPN_PROTECTION :
                                    (tmp >= 10) ? SPWPN_VENOM
                                                : SPWPN_FLAME + (tmp % 2);
            }
        }


        if (is_demonic(atype))
        {
            switch (random5(9))
            {
            case 0:
                proprt[RAP_BRAND] = SPWPN_DRAINING;
                break;
            case 1:
                proprt[RAP_BRAND] = SPWPN_FLAMING;
                break;
            case 2:
                proprt[RAP_BRAND] = SPWPN_FREEZING;
                break;
            case 3:
                proprt[RAP_BRAND] = SPWPN_ELECTROCUTION;
                break;
            case 4:
                proprt[RAP_BRAND] = SPWPN_VAMPIRICISM;
                break;
            case 5:
                proprt[RAP_BRAND] = SPWPN_PAIN;
                break;
            case 6:
                proprt[RAP_BRAND] = SPWPN_VENOM;
                break;
            default:
                power_level -= 2;
            }
            power_level += 2;
        }
        else if (random5(3) == 0)
            proprt[RAP_BRAND] = SPWPN_NORMAL;
        else
            power_level++;
    }

    if (random5(5) == 0)
        goto skip_mods;

    /* AC mod - not for armours or rings of protection */
    if (random5(4 + power_level) == 0
        && aclass != OBJ_ARMOUR
        && (aclass != OBJ_JEWELLERY || atype != RING_PROTECTION))
    {
        proprt[RAP_AC] = 1 + random5(3) + random5(3) + random5(3);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_AC] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

    /* ev mod - not for rings of evasion */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_EVASION))
    {
        proprt[RAP_EVASION] = 1 + random5(3) + random5(3) + random5(3);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_EVASION] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

    /* str mod - not for rings of strength */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_STRENGTH))
    {
        proprt[RAP_STRENGTH] = 1 + random5(3) + random5(2);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_STRENGTH] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

    /* int mod - not for rings of intelligence */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_INTELLIGENCE))
    {
        proprt[RAP_INTELLIGENCE] = 1 + random5(3) + random5(2);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_INTELLIGENCE] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

    /* dex mod - not for rings of dexterity */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_DEXTERITY))
    {
        proprt[RAP_DEXTERITY] = 1 + random5(3) + random5(2);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_DEXTERITY] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

  skip_mods:
    if (random5(15) < power_level
        || aclass == OBJ_WEAPONS
        || (aclass == OBJ_JEWELLERY && atype == RING_SLAYING))
    {
        goto skip_combat;
    }

    /* Weapons and rings of slaying can't get these */
    if (random5(4 + power_level) == 0)  /* to-hit */
    {
        proprt[RAP_ACCURACY] = 1 + random5(3) + random5(2);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_ACCURACY] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

    if (random5(4 + power_level) == 0)  /* to-dam */
    {
        proprt[RAP_DAMAGE] = 1 + random5(3) + random5(2);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_DAMAGE] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

  skip_combat:
    if (random5(12) < power_level)
        goto finished_powers;

/* res_fire */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY
            || (atype != RING_PROTECTION_FROM_FIRE
                && atype != RING_FIRE
                && atype != RING_ICE))
        && (aclass != OBJ_ARMOUR
            || (atype != ARM_DRAGON_ARMOUR
                && atype != ARM_ICE_DRAGON_ARMOUR
                && atype != ARM_GOLD_DRAGON_ARMOUR)))
    {
        proprt[RAP_FIRE] = 1;
        if (random5(5) == 0)
            proprt[RAP_FIRE]++;
        power_level++;
    }

    /* res_cold */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY
            || (atype != RING_PROTECTION_FROM_COLD
                && atype != RING_FIRE
                && atype != RING_ICE))
        && (aclass != OBJ_ARMOUR
            || (atype != ARM_DRAGON_ARMOUR
                && atype != ARM_ICE_DRAGON_ARMOUR
                && atype != ARM_GOLD_DRAGON_ARMOUR)))
    {
        proprt[RAP_COLD] = 1;
        if (random5(5) == 0)
            proprt[RAP_COLD]++;
        power_level++;
    }

    if (random5(12) < power_level || power_level > 7)
        goto finished_powers;

    /* res_elec */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_ARMOUR || atype != ARM_STORM_DRAGON_ARMOUR))
    {
        proprt[RAP_ELECTRICITY] = 1;
        power_level++;
    }

    /* res_poison */
    if (random5(5 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_POISON_RESISTANCE)
        && (aclass != OBJ_ARMOUR
            || atype != ARM_GOLD_DRAGON_ARMOUR
            || atype != ARM_SWAMP_DRAGON_ARMOUR))
    {
        proprt[RAP_POISON] = 1;
        power_level++;
    }

    /* prot_life - no necromantic brands on weapons allowed */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_TELEPORTATION)
        && proprt[RAP_BRAND] != SPWPN_DRAINING
        && proprt[RAP_BRAND] != SPWPN_VAMPIRICISM
        && proprt[RAP_BRAND] != SPWPN_PAIN)
    {
        proprt[RAP_NEGATIVE_ENERGY] = 1;
        power_level++;
    }

    /* res magic */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_PROTECTION_FROM_MAGIC))
    {
        proprt[RAP_MAGIC] = 20 + random5(40);
        power_level++;
    }

    /* see_invis */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_SEE_INVISIBLE))
    {
        proprt[RAP_EYESIGHT] = 1;
        power_level++;
    }

    if (random5(12) < power_level || power_level > 10)
        goto finished_powers;

    /* turn invis */
    if (random5(10) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_INVISIBILITY))
    {
        proprt[RAP_INVISIBLE] = 1;
        power_level++;
    }

    /* levitate */
    if (random5(10) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_LEVITATION))
    {
        proprt[RAP_LEVITATE] = 1;
        power_level++;
    }

    if (random5(10) == 0)       /* blink */
    {
        proprt[RAP_BLINK] = 1;
        power_level++;
    }

    /* teleport */
    if (random5(10) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_TELEPORTATION))
    {
        proprt[RAP_CAN_TELEPORT] = 1;
        power_level++;
    }

    /* go berserk */
    if (random5(10) == 0 && (aclass != OBJ_JEWELLERY || atype != AMU_RAGE))
    {
        proprt[RAP_BERSERK] = 1;
        power_level++;
    }

    if (random5(10) == 0)       /* sense surr */
    {
        proprt[RAP_MAPPING] = 1;
        power_level++;
    }


  finished_powers:
    /* Armours get less powers, and are also less likely to be
       cursed that wpns */
    if (aclass == OBJ_ARMOUR)
        power_level -= 4;

    if (random5(17) >= power_level || power_level < 2)
        goto finished_curses;

    switch (random5(9))
    {
    case 0:                     /* makes noise */
        if (aclass != OBJ_WEAPONS)
            break;
        proprt[RAP_NOISES] = 1 + random5(4);
        break;
    case 1:                     /* no magic */
        proprt[RAP_PREVENT_SPELLCASTING] = 1;
        break;
    case 2:                     /* random teleport */
        if (aclass != OBJ_WEAPONS)
            break;
        proprt[RAP_CAUSE_TELEPORTATION] = 5 + random5(15);
        break;
    case 3:   /* no teleport - doesn't affect some instantaneous teleports */
        if (aclass == OBJ_JEWELLERY && atype == RING_TELEPORTATION)
            break;              /* already is a ring of tport */
        if (aclass == OBJ_JEWELLERY && atype == RING_TELEPORT_CONTROL)
            break;              /* already is a ring of tport ctrl */
        proprt[RAP_BLINK] = 0;
        proprt[RAP_CAN_TELEPORT] = 0;
        proprt[RAP_PREVENT_TELEPORTATION] = 1;
        break;
    case 4:                     /* berserk on attack */
        if (aclass != OBJ_WEAPONS)
            break;
        proprt[RAP_ANGRY] = 1 + random5(8);
        break;
    case 5:                     /* susceptible to fire */
        if (aclass == OBJ_JEWELLERY
            && (atype == RING_PROTECTION_FROM_FIRE || atype == RING_FIRE
                || atype == RING_ICE))
            break;              /* already does this or something */
        if (aclass == OBJ_ARMOUR
            && (atype == ARM_DRAGON_ARMOUR || atype == ARM_ICE_DRAGON_ARMOUR
                || atype == ARM_GOLD_DRAGON_ARMOUR))
            break;
        proprt[RAP_FIRE] = -1;
        break;
    case 6:                     /* susceptible to cold */
        if (aclass == OBJ_JEWELLERY
            && (atype == RING_PROTECTION_FROM_COLD || atype == RING_FIRE
                || atype == RING_ICE))
            break;              /* already does this or something */
        if (aclass == OBJ_ARMOUR
            && (atype == ARM_DRAGON_ARMOUR || atype == ARM_ICE_DRAGON_ARMOUR
                || atype == ARM_GOLD_DRAGON_ARMOUR))
            break;
        proprt[RAP_COLD] = -1;
        break;
    case 7:                     /* speed metabolism */
        if (aclass == OBJ_JEWELLERY && atype == RING_HUNGER)
            break;              /* already is a ring of hunger */
        if (aclass == OBJ_JEWELLERY && atype == RING_SUSTENANCE)
            break;              /* already is a ring of sustenance */
        proprt[RAP_METABOLISM] = 1 + random5(3);
        break;
    case 8:   /* emits mutagenic radiation - increases magic_contamination */
        /* property is chance (1 in ...) of increasing magic_contamination */
        proprt[RAP_MUTAGENIC] = 2 + random5(4);
        break;
    }

/*
   26 - +to-hit (no wpns)
   27 - +to-dam (no wpns)
 */

finished_curses:
    if (random5(10) == 0
        && (aclass != OBJ_ARMOUR
            || atype != ARM_CLOAK
            || !cmp_equip_race( item, ISFLAG_ELVEN ))
        && (aclass != OBJ_ARMOUR
            || atype != ARM_BOOTS
            || !cmp_equip_race( item, ISFLAG_ELVEN )
        && get_armour_ego_type( item ) != SPARM_STEALTH))
    {
        power_level++;
        proprt[RAP_STEALTH] = 10 + random5(70);

        if (random5(4) == 0)
        {
            proprt[RAP_STEALTH] = -proprt[RAP_STEALTH] - random5(20);
            power_level--;
        }
    }

    if ((power_level < 2 && random5(5) == 0) || random5(30) == 0)
        proprt[RAP_CURSED] = 1;

    srand(randstore);

}

int randart_wpn_property( const item_def &item, char prop )
{
    FixedVector< char, RA_PROPERTIES > proprt;

    randart_wpn_properties( item, proprt );

    return (proprt[prop]);
}

const char *randart_name( const item_def &item )
{
    ASSERT( item.base_type == OBJ_WEAPONS );

    if (is_unrandom_artefact( item ))
    {
        struct unrandart_entry *unrand = seekunrandart( item );

        return (item_ident(item, ISFLAG_KNOW_TYPE) ? unrand->name
                                                   : unrand->unid_name);
    }

    free(art_n);
    art_n = (char *) malloc(sizeof(char) * 80);

    if (art_n == NULL)
#ifdef JP
        return ("Malloc ½ÇÆÐ ¿¡·¯ÀÔ´Ï´Ù.");
#else
        return ("Malloc Failed Error");
#endif

    strcpy(art_n, "");

    // long seed = aclass + adam * (aplus % 100) + atype * aplus2;
    long seed = calc_seed( item );
    long randstore = rand();
    srand( seed );

    if (item_not_ident( item, ISFLAG_KNOW_TYPE ))
    {
        switch (random5(21))
        {
#ifdef JP
        case  0: strcat(art_n, "¹à°Ô ¹ÝÂ¦ÀÌ´Â "); break;
        case  1: strcat(art_n, "·éÀÌ »õ°ÜÁø "); break;
        case  2: strcat(art_n, "¿¬±â¸¦ ÇÇ¿ì´Â "); break;
        case  3: strcat(art_n, "ÇÇ¿¡ ¹°µç "); break;
        case  4: strcat(art_n, "¿Ö°îµÈ "); break;
        case  5: strcat(art_n, "¹ÝÂ¦¹ÝÂ¦ ºû³ª´Â "); break;
        case  6: strcat(art_n, "µÚÆ²¸° "); break;
        case  7: strcat(art_n, "¼öÁ¤ÀÇ "); break;
        case  8: strcat(art_n, "º¸¼®À¸·Î ²Ù¸çÁø "); break;
        case  9: strcat(art_n, "Åõ¸íÇÑ "); break;
        case 10: strcat(art_n, "Àå½ÄµÈ "); break;
        case 11: strcat(art_n, "±¸¸ÛÅõ¼ºÀÌÀÇ "); break;
        case 12: strcat(art_n, "¹Ì²ö¹Ì²öÇÑ "); break;
        case 13: strcat(art_n, "´Û¾Æ³»¾îÁø "); break;
        case 14: strcat(art_n, "È£È­·Î¿î "); break;
        case 15: strcat(art_n, "Çã¼úÇÑ "); break;
        case 16: strcat(art_n, "³°¾Æ ºû¹Ù·£ "); break;
        case 17: strcat(art_n, "°í¸§ÀÌ ³ª´Â "); break;
        case 18: strcat(art_n, "¾àÇÏ°Ô ¹ÝÂ¦ÀÌ´Â "); break;
        case 19: strcat(art_n, "Áõ±â¸¦ ÇÇ¿ì´Â "); break;
        case 20: strcat(art_n, "ºû³ª´Â "); break;
#else
        case  0: strcat(art_n, "brightly glowing "); break;
        case  1: strcat(art_n, "runed "); break;
        case  2: strcat(art_n, "smoking "); break;
        case  3: strcat(art_n, "bloodstained "); break;
        case  4: strcat(art_n, "twisted "); break;
        case  5: strcat(art_n, "shimmering "); break;
        case  6: strcat(art_n, "warped "); break;
        case  7: strcat(art_n, "crystal "); break;
        case  8: strcat(art_n, "jewelled "); break;
        case  9: strcat(art_n, "transparent "); break;
        case 10: strcat(art_n, "encrusted "); break;
        case 11: strcat(art_n, "pitted "); break;
        case 12: strcat(art_n, "slimy "); break;
        case 13: strcat(art_n, "polished "); break;
        case 14: strcat(art_n, "fine "); break;
        case 15: strcat(art_n, "crude "); break;
        case 16: strcat(art_n, "ancient "); break;
        case 17: strcat(art_n, "ichor-stained "); break;
        case 18: strcat(art_n, "faintly glowing "); break;
        case 19: strcat(art_n, "steaming "); break;
        case 20: strcat(art_n, "shiny "); break;
#endif
        }

        char st_p3[ITEMNAME_SIZE];

        standard_name_weap( item.sub_type, st_p3 );
        strcat(art_n, st_p3);
        srand(randstore);
        return (art_n);
    }

    char st_p[ITEMNAME_SIZE];
#ifdef JP
    //if (random5(5) >= 2)
    if (random5(2) == 0)
    {
        standard_name_weap( item.sub_type, st_p );
        strcpy(art_n, "" );

//ÀÏº»¾îÆÇ¿¡¼­´Â ¾ÆÆ¼ÆÑÆ® ¼ö½Ä»çÀÇ ¾î¼øÀ» Á¶ÀÛÇØ
        int rand_name_no;
        rand_name_no = random5(390);
//340¹ø±îÁö´Â ¡¸¡Û¡ÛÀÇ °Ë¡¹±× ÀÌÈÄ´Â ¡¸°Ë¡º¡Û¡Û¡»¡¹À¸·Î ÇÑ´Ù.
        if ( rand_name_no < 340 )
        {
        strcat(art_n, rand_wpn_names[rand_name_no]);
        strcat(art_n, st_p);
        }
        else
        {
        strcat(art_n, st_p);
        strcat(art_n, rand_wpn_names[rand_name_no]);
        }
    }
#else
    if (random5(2) == 0)
    {
        standard_name_weap( item.sub_type, st_p );
        strcat(art_n, st_p);
        strcat(art_n, rand_wpn_names[random5(390)]);
    }
#endif
    else
    {
        char st_p2[ITEMNAME_SIZE];
        strcpy(st_p2, "" );
#ifdef JP
        strcpy( st_p, rand_pre_names[ random5(50) ] );
        strcat( st_p, rand_suf_names[ random5(50) ] );
#else
        make_name(random5(250), random5(250), random5(250), 3, st_p);
#endif
        standard_name_weap( item.sub_type, st_p2 );
        strcpy(art_n, "" );

        if (random5(3) == 5) //(random5(3) == 0)
        {
#ifdef JP
            strcat(art_n, "¡º");
            strcat(art_n, st_p);
            strcat(art_n, "¡»");
            strcat(art_n, "ÀÇ ");
            strcat(art_n, st_p2);
#else
            strcat(art_n, " of ");
            strcat(art_n, st_p);
#endif

        }
        else
        {
            strcat(art_n, st_p2);
#ifdef JP
            strcat(art_n, "¡º");
#else
            strcat(art_n, " \"");
#endif
            strcat(art_n, st_p);
#ifdef JP
            strcat(art_n, "¡»");
#else
            strcat(art_n, "\"");
#endif
        }
    }

    srand(randstore);

    return (art_n);
}

const char *randart_armour_name( const item_def &item )
{
    ASSERT( item.base_type == OBJ_ARMOUR );

    if (is_unrandom_artefact( item ))
    {
        struct unrandart_entry *unrand = seekunrandart( item );

        return (item_ident(item, ISFLAG_KNOW_TYPE) ? unrand->name
                                                   : unrand->unid_name);
    }

    free(art_n);
    art_n = (char *) malloc(sizeof(char) * 80);

    if (art_n == NULL)
    {
#ifdef JP
        return ("Malloc ½ÇÆÐ ¿¡·¯ÀÔ´Ï´Ù.");
#else
        return ("Malloc Failed Error");
#endif
    }

    strcpy(art_n, "");

    // long seed = aclass + adam * (aplus % 100) + atype * aplus2;
    long seed = calc_seed( item );
    long randstore = rand();
    srand( seed );

    if (item_not_ident( item, ISFLAG_KNOW_TYPE ))
    {
        switch (random5(21))
        {
#ifdef JP
        case  0: strcat(art_n, "¹à°Ô ¹ÝÂ¦ÀÌ´Â "); break;
        case  1: strcat(art_n, "·éÀÌ »õ°ÜÁø "); break;
        case  2: strcat(art_n, "¿¬±â¸¦ ÇÇ¿ì´Â "); break;
        case  3: strcat(art_n, "ÇÇ¿¡ ¹°µç "); break;
        case  4: strcat(art_n, "¿Ö°îµÈ "); break;
        case  5: strcat(art_n, "¹ÝÂ¦¹ÝÂ¦ ºû³ª´Â "); break;
        case  6: strcat(art_n, "µÚÆ²¸° "); break;
        case  7: strcat(art_n, "·éÀÌ ¹ÚÈù "); break;
        case  8: strcat(art_n, "º¸¼®À¸·Î ²Ù¸çÁø "); break;
        case  9: strcat(art_n, "Åõ¸íÇÑ "); break;
        case 10: strcat(art_n, "Àå½ÄµÈ "); break;
        case 11: strcat(art_n, "±¸¸ÛÅõ¼ºÀÌÀÇ "); break;
        case 12: strcat(art_n, "¹Ì²ö¹Ì²öÇÑ "); break;
        case 13: strcat(art_n, "´Û¾Æ³»¾îÁø "); break;
        case 14: strcat(art_n, "È£È­·Î¿î "); break;
        case 15: strcat(art_n, "Çã¼úÇÑ "); break;
        case 16: strcat(art_n, "³°¾Æ ºû¹Ù·£ "); break;
        case 17: strcat(art_n, "°í¸§ÀÌ ³ª´Â "); break;
        case 18: strcat(art_n, "¾àÇÏ°Ô ¹ÝÂ¦ÀÌ´Â "); break;
        case 19: strcat(art_n, "Áõ±â¸¦ ÇÇ¿ì´Â "); break;
        case 20: strcat(art_n, "ºû³ª´Â "); break;
#else
        case  0: strcat(art_n, "brightly glowing "); break;
        case  1: strcat(art_n, "runed "); break;
        case  2: strcat(art_n, "smoking "); break;
        case  3: strcat(art_n, "bloodstained "); break;
        case  4: strcat(art_n, "twisted "); break;
        case  5: strcat(art_n, "shimmering "); break;
        case  6: strcat(art_n, "warped "); break;
        case  7: strcat(art_n, "heavily runed "); break;
        case  8: strcat(art_n, "jeweled "); break;
        case  9: strcat(art_n, "transparent "); break;
        case 10: strcat(art_n, "encrusted "); break;
        case 11: strcat(art_n, "pitted "); break;
        case 12: strcat(art_n, "slimy "); break;
        case 13: strcat(art_n, "polished "); break;
        case 14: strcat(art_n, "fine "); break;
        case 15: strcat(art_n, "crude "); break;
        case 16: strcat(art_n, "ancient "); break;
        case 17: strcat(art_n, "ichor-stained "); break;
        case 18: strcat(art_n, "faintly glowing "); break;
        case 19: strcat(art_n, "steaming "); break;
        case 20: strcat(art_n, "shiny "); break;
#endif
        }
        char st_p3[ITEMNAME_SIZE];

        standard_name_armour(item, st_p3);
        strcat(art_n, st_p3);
        srand(randstore);
        return (art_n);
    }

    char st_p[ITEMNAME_SIZE];

    if (random5(2) == 0)
    {
        standard_name_armour(item, st_p);
#ifdef JP
//ÀÏº»¾îÆÇ¿¡¼­´Â ¾ÆÆ¼ÆÑÆ® ¼ö½Ä»çÀÇ ¾î¼øÀ» ¹Ù²ã ³Ö´Â´Ù
        strcpy(art_n, "" );
        strcat(art_n, rand_armour_names[random5(71)]);
        strcat(art_n, st_p );
#else
        strcpy(art_n, st_p);
        strcat(art_n, rand_armour_names[random5(71)]);
#endif
    }
    else
    {
        char st_p2[ITEMNAME_SIZE];
        strcpy(st_p2, "" );
#ifdef JP
        strcpy( st_p, rand_pre_names[ random5(50) ] );
        strcat( st_p, rand_suf_names[ random5(50) ] );
#else
        make_name(random5(250), random5(250), random5(250), 3, st_p);
#endif
        standard_name_armour(item, st_p2);
        strcpy(art_n, "" );

        if (random5(3) == 5) //(random5(3) == 0)
        {
#ifdef JP
            strcat(art_n, "¡º");
            strcat(art_n, st_p);
            strcat(art_n, "¡»");
            strcat(art_n, "ÀÇ ");
            strcat(art_n, st_p2);
#else
            strcat(art_n, " of ");
            strcat(art_n, st_p);
#endif

        }
        else
        {
            strcat(art_n, st_p2);
#ifdef JP
            strcat(art_n, "¡º");
#else
            strcat(art_n, " \"");
#endif
            strcat(art_n, st_p);
#ifdef JP
            strcat(art_n, "¡»");
#else
            strcat(art_n, "\"");
#endif
        }
    }

    srand(randstore);

    return (art_n);
}

const char *randart_ring_name( const item_def &item )
{
    ASSERT( item.base_type == OBJ_JEWELLERY );

    int temp_rand = 0;          // probability determination {dlb}

    if (is_unrandom_artefact( item ))
    {
        struct unrandart_entry *unrand = seekunrandart( item );

        return (item_ident(item, ISFLAG_KNOW_TYPE) ? unrand->name
                                                   : unrand->unid_name);
    }

    char st_p[ITEMNAME_SIZE];
#ifdef JP
    char chors[ITEMNAME_SIZE];
#endif
    free(art_n);
    art_n = (char *) malloc(sizeof(char) * 80);

    if (art_n == NULL)
#ifdef JP
        return ("Malloc ½ÇÆÐ ¿¡·¯ÀÔ´Ï´Ù.");
#else
        return ("Malloc Failed Error");
#endif

    strcpy(art_n, "");

    // long seed = aclass + adam * (aplus % 100) + atype * aplus2;
    long seed = calc_seed( item );
    long randstore = rand();
    srand( seed );

    if (item_not_ident( item, ISFLAG_KNOW_TYPE ))
    {
        temp_rand = random5(21);

#ifdef JP
        strcat(art_n,  (temp_rand == 0)  ? "¹à°Ô ºû³ª´Â " :
                       (temp_rand == 1)  ? "·éÀÌ »õ°ÜÁø " :
                       (temp_rand == 2)  ? "¿¬±â¸¦ ÇÇ¿ì´Â " :
                       (temp_rand == 3)  ? "·çºñÀÇ " :
                       (temp_rand == 4)  ? "¿Ö°îµÈ " :
                       (temp_rand == 5)  ? "¹ÝÂ¦¹ÝÂ¦ ºû³ª´Â " :
                       (temp_rand == 6)  ? "µÚÆ²¸° " :
                       (temp_rand == 7)  ? "¼öÁ¤ÀÇ " :
                       (temp_rand == 8)  ? "´ÙÀÌ¾Æ¸óµåÀÇ " :
                       (temp_rand == 9)  ? "Åõ¸íÇÑ " :
                       (temp_rand == 10) ? "Àå½ÄµÈ " :
                       (temp_rand == 11) ? "±¸¸ÛÅõ¼ºÀÌÀÇ " :
                       (temp_rand == 12) ? "¹Ì²ö¹Ì²öÇÑ " :
                       (temp_rand == 13) ? "´Û¾Æ³»¾îÁø " :
                       (temp_rand == 14) ? "È£È­·Î¿î " :
                       (temp_rand == 15) ? "Çã¼úÇÑ " :
                       (temp_rand == 16) ? "³°¾Æ ºû¹Ù·£ " :
                       (temp_rand == 17) ? "¿¡¸Þ¶öµåÀÇ " :
                       (temp_rand == 18) ? "¾àÇÏ°Ô ¹ÝÂ¦ÀÌ´Â " :
                       (temp_rand == 19) ? "Áõ±â¸¦ ÇÇ¿ì´Â "
                                         : "ºû³ª´Â ");
#else
        strcat(art_n,  (temp_rand == 0)  ? "brightly glowing" :
                       (temp_rand == 1)  ? "runed" :
                       (temp_rand == 2)  ? "smoking" :
                       (temp_rand == 3)  ? "ruby" :
                       (temp_rand == 4)  ? "twisted" :
                       (temp_rand == 5)  ? "shimmering" :
                       (temp_rand == 6)  ? "warped" :
                       (temp_rand == 7)  ? "crystal" :
                       (temp_rand == 8)  ? "diamond" :
                       (temp_rand == 9)  ? "transparent" :
                       (temp_rand == 10) ? "encrusted" :
                       (temp_rand == 11) ? "pitted" :
                       (temp_rand == 12) ? "slimy" :
                       (temp_rand == 13) ? "polished" :
                       (temp_rand == 14) ? "fine" :
                       (temp_rand == 15) ? "crude" :
                       (temp_rand == 16) ? "ancient" :
                       (temp_rand == 17) ? "emerald" :
                       (temp_rand == 18) ? "faintly glowing" :
                       (temp_rand == 19) ? "steaming"
                                         : "shiny");
#endif

#ifdef JP
        strcat(art_n, (item.sub_type < AMU_RAGE) ? "¹ÝÁö" : "ºÎÀû");
#else
        strcat(art_n, " ");
        strcat(art_n, (item.sub_type < AMU_RAGE) ? "ring" : "amulet");
#endif
        srand(randstore);
        return (art_n);
    }

    if (random5(5) == 0)
    {
        strcpy(art_n, "" );
#ifdef JP
        strcpy(st_p, (item.sub_type < AMU_RAGE) ? "¹ÝÁö" : "ºÎÀû");
#else
        strcpy(st_p, (item.sub_type < AMU_RAGE) ? "ring" : "amulet");
#endif
#ifdef JP
//ÀÏº»¾îÆÇ¿¡¼­´Â ¾ÆÆ¼ÆÑÆ® ¼ö½Ä»çÀÇ ¾î¼øÀ» ¹Ù²ã ³Ö´Â´Ù
        strcpy(art_n, rand_armour_names[random5(71)]);
        strcat(art_n, st_p );
#else
        strcpy(art_n, st_p );
        strcat(art_n, rand_armour_names[random5(71)]);
#endif
    }
//‚±‚±‚©‚çŽô•¶•—‚Ìƒ‰ƒ“ƒ_ƒ€–¼
    else
    {
        char st_p2[ITEMNAME_SIZE];
        strcpy(st_p2, "" );
#ifdef JP
        strcpy( st_p, rand_pre_names[ random5(50) ] );
        strcat( st_p, rand_suf_names[ random5(50) ] );
        strcpy( st_p2, (item.sub_type < AMU_RAGE) ? "¹ÝÁö" : "ºÎÀû");
#else
        make_name(random5(250), random5(250), random5(250), 3, st_p);
        strcat(st_p2, (item.sub_type < AMU_RAGE) ? "ring" : "amulet");
#endif
        strcpy(art_n, "" );

        if (random5(3) == 5) //(random5(3) == 0)
        {
#ifdef JP
            strcat(art_n, "¡º");
            strcat(art_n, st_p);
            strcat(art_n, "¡»");
            strcat(art_n, "ÀÇ ");
            strcat(art_n, st_p2);
#else
            strcat(art_n, " of ");
            strcat(art_n, st_p);
#endif
        }
        else
        {
            strcat(art_n, st_p2);
#ifdef JP
            strcat(art_n, "¡º");
#else
            strcat(art_n, " \"");
#endif
            strcat(art_n, st_p);
#ifdef JP
            strcat(art_n, "¡»");
#else
            strcat(art_n, "\"");
#endif
        }
    }
    srand(randstore);
    return (art_n);
}
// end randart_ring_name()

static struct unrandart_entry *seekunrandart( const item_def &item )
{
    int x = 0;

    while (x < NO_UNRANDARTS)
    {
        if (unranddata[x].ura_cl == item.base_type
            && unranddata[x].ura_ty == item.sub_type
            && unranddata[x].ura_pl == item.plus
            && unranddata[x].ura_pl2 == item.plus2)
        {
            return (&unranddata[x]);
        }

        x++;
    }

    return (&unranddata[0]);  // Dummy object
}                               // end seekunrandart()
#if 1
int find_unrandart_index2(const item_def &item)
{
    static int cache_head = -1;
    static int cache_next[NO_UNRANDARTS];

    int x, prev;

    /* Init Cache */
    if (cache_head == -1)
    {
        cache_head = 0;
        for (x = 0; x <NO_UNRANDARTS-1; x++)
        {
            cache_next[x] = x + 1;
        }
        cache_next[NO_UNRANDARTS-1] = -1;
    }

    /* Search Cache */
    x = cache_head;
    prev = -1;

    while(x != -1)
    {
        int next = cache_next[x];

        if (unranddata[x].ura_cl == item.base_type
            && unranddata[x].ura_ty == item.sub_type
            && unranddata[x].ura_pl == item.plus
            && unranddata[x].ura_pl2 == item.plus2)
        {
            if (x != cache_head)
            {
                cache_next[x] = cache_head;
                cache_head = x;
                cache_next[prev] = next;
            }
            return (x);
        }
        prev = x;
        x = next;
    }

    return (-1);
}

int find_unrandart_index(int item_number)
{
  return find_unrandart_index2(mitm[item_number]);
}
#else

int find_unrandart_index(int item_number)
{
    int x;

    for(x=0; x < NO_UNRANDARTS; x++)
    {
        if (unranddata[x].ura_cl == mitm[item_number].base_type
            && unranddata[x].ura_ty == mitm[item_number].sub_type
            && unranddata[x].ura_pl == mitm[item_number].plus
            && unranddata[x].ura_pl2 == mitm[item_number].plus2)
        {
            return (x);
        }
    }

    return (-1);
}
#endif

int find_okay_unrandart(unsigned char aclass, unsigned char atype)
{
    int x, count;
    int ret = -1;

    for (x = 0, count = 0; x < NO_UNRANDARTS; x++)
    {
        if (unranddata[x].ura_cl == aclass
            && does_unrandart_exist(x) == 0
            && (atype == OBJ_RANDOM || unranddata[x].ura_ty == atype))
        {
            count++;

            if (random5(count) == 0)
                ret = x;
        }
    }

    return (ret);
}                               // end find_okay_unrandart()

// which == 0 (default) gives random fixed artefact.
// Returns true if successful.
bool make_item_fixed_artefact( item_def &item, bool in_abyss, int which )
{
    bool  force = true;  // we force any one asked for specifically

    if (!which)
    {
        // using old behaviour... try only once. -- bwr
        force = false;

        which = SPWPN_SINGING_SWORD + random2(12);
        if (which >= SPWPN_SWORD_OF_CEREBOV)
            which += 3; // skip over Cerebov's, Dispater's, and Asmodeus' weapons
    }

    int status = get_unique_item_status( OBJ_WEAPONS, which );

    if ((status == UNIQ_EXISTS
            || (in_abyss && status == UNIQ_NOT_EXISTS)
            || (!in_abyss && status == UNIQ_LOST_IN_ABYSS))
        && !force)
    {
        return (false);
    }

    switch (which)
    {
    case SPWPN_SINGING_SWORD:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_LONG_SWORD;
        item.plus  = 7;
        item.plus2 = 6;
        break;

    case SPWPN_WRATH_OF_TROG:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_BATTLEAXE;
        item.plus  = 3;
        item.plus2 = 11;
        break;

    case SPWPN_SCYTHE_OF_CURSES:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_SCYTHE;
        item.plus  = 13;
        item.plus2 = 13;
        break;

    case SPWPN_MACE_OF_VARIABILITY:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_MACE;
        item.plus  = random2(16) - 4;
        item.plus2 = random2(16) - 4;
        break;

    case SPWPN_GLAIVE_OF_PRUNE:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_GLAIVE;
        item.plus  = 0;
        item.plus2 = 12;
        break;

    case SPWPN_SCEPTRE_OF_TORMENT:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_MACE;
        item.plus  = 7;
        item.plus2 = 6;
        break;

    case SPWPN_SWORD_OF_ZONGULDROK:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_LONG_SWORD;
        item.plus  = 9;
        item.plus2 = 9;
        break;

    case SPWPN_SWORD_OF_POWER:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_GREAT_SWORD;
        item.plus  = 0; // set on wield
        item.plus2 = 0; // set on wield
        break;

    case SPWPN_KNIFE_OF_ACCURACY:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_DAGGER;
        item.plus  = 27;
        item.plus2 = -1;
        break;

    case SPWPN_STAFF_OF_OLGREB:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 0; // set on wield
        item.plus2 = 0; // set on wield
        break;

    case SPWPN_VAMPIRES_TOOTH:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_DAGGER;
        item.plus  = 3;
        item.plus2 = 4;
        break;

    case SPWPN_STAFF_OF_WUCAD_MU:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 0; // set on wield
        item.plus2 = 0; // set on wield
        break;

    case SPWPN_SWORD_OF_CEREBOV:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_GREAT_SWORD;
        item.plus  = 6;
        item.plus2 = 6;
        item.colour = YELLOW;
        do_curse_item( item );
        break;

    case SPWPN_STAFF_OF_DISPATER:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 4;
        item.plus2 = 4;
        item.colour = YELLOW;
        break;

    case SPWPN_SCEPTRE_OF_ASMODEUS:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 7;
        item.plus2 = 7;
        item.colour = RED;
        break;

    default:
#ifdef JP
        DEBUGSTR( "ºñÇÕ¹ýÀûÀ¸·Î °³Á¶µÈ ¾ÆÆ¼ÆÑÆ®¸¦ Á¦Á¶ÇÕ´Ï´Ù!" );
#else
        DEBUGSTR( "Trying to create illegal fixed artefact!" );
#endif
        return (false);
    }

    // If we get here, we've made the artefact
    item.special = which;
    item.quantity = 1;

    // Items originally generated in the abyss and not found will be
    // shifted to "lost in abyss", and will only be found there. -- bwr
    set_unique_item_status( OBJ_WEAPONS, which, UNIQ_EXISTS );

    return (true);
}

bool make_item_randart( item_def &item )
{
    if (item.base_type != OBJ_WEAPONS
        && item.base_type != OBJ_ARMOUR
        && item.base_type != OBJ_JEWELLERY)
    {
        return (false);
    }

    item.flags |= ISFLAG_RANDART;
    item.special = (random() & RANDART_SEED_MASK);

    return (true);
}

// void make_item_unrandart( int x, int ura_item )
bool make_item_unrandart( item_def &item, int unrand_index )
{
    item.base_type = unranddata[unrand_index].ura_cl;
    item.sub_type  = unranddata[unrand_index].ura_ty;
    item.plus      = unranddata[unrand_index].ura_pl;
    item.plus2     = unranddata[unrand_index].ura_pl2;
    item.colour    = unranddata[unrand_index].ura_col;

    item.flags |= ISFLAG_UNRANDART;
    item.special = unranddata[ unrand_index ].prpty[ RAP_BRAND ];

    if (unranddata[ unrand_index ].prpty[ RAP_CURSED ])
        do_curse_item( item );

    set_unrandart_exist( unrand_index, 1 );

    return (true);
}                               // end make_item_unrandart()

const char *unrandart_descrip( char which_descrip, const item_def &item )
{
/* Eventually it would be great to have randomly generated descriptions for
   randarts. */
    struct unrandart_entry *unrand = seekunrandart( item );

    return ((which_descrip == 0) ? unrand->spec_descrip1 :
            (which_descrip == 1) ? unrand->spec_descrip2 :
#ifdef JP
            (which_descrip == 2) ? unrand->spec_descrip3 : "¹ÌÆÇ¸í");
#else
            (which_descrip == 2) ? unrand->spec_descrip3 : "Unknown.");
#endif

}                               // end unrandart_descrip()

void standard_name_weap(unsigned char item_typ, char glorg[ITEMNAME_SIZE])
{
#ifdef JP
//ÀåºñÀÇ ÀÌ¸§Àº 8±ÛÀÚ ÀÌ³»·Î ÇØ Áà. Æ®¶óÀÌ´øÆ®¿Í ¾Ç¸¶ÀÇ Æ®¶óÀÌ´øÆ®ÀÇ Á¤ÇÕ¼ºÀº ÀÎ³».
    strcpy(glorg,  (item_typ == WPN_CLUB) ? "°ïºÀ" :
                   (item_typ == WPN_MACE) ? "¸ÞÀÌ½º" :
                   (item_typ == WPN_FLAIL) ? "ÇÁ·¹ÀÏ" :
                   (item_typ == WPN_KNIFE) ? "³ªÀÌÇÁ" :
                   (item_typ == WPN_DAGGER) ? "´Üµµ" :
                   (item_typ == WPN_MORNINGSTAR) ? "¸ð´×½ºÅ¸" :
                   (item_typ == WPN_SHORT_SWORD) ? "´Ü°Ë" :
                   (item_typ == WPN_LONG_SWORD) ? "Àå°Ë" :
                   (item_typ == WPN_GREAT_SWORD) ? "´ë°Ë" :
                   (item_typ == WPN_SCIMITAR) ? "½Ã¹ÌÅÍ" :
                   (item_typ == WPN_HAND_AXE) ? "¼Õ µµ³¢" :
                   (item_typ == WPN_BATTLEAXE) ? "ÀüÅõ µµ³¢" :
                   (item_typ == WPN_SPEAR) ? "Ã¢" :
                   (item_typ == WPN_TRIDENT) ? "»ïÁöÃ¢" :
                   (item_typ == WPN_HALBERD) ? "µµ³¢Ã¢" :
                   (item_typ == WPN_SLING) ? "Åõ¼®±â" :
                   (item_typ == WPN_BOW) ? "È°" :
                   (item_typ == WPN_BLOWGUN) ? "ºí·Î¿ì °Ç" :
                   (item_typ == WPN_CROSSBOW) ? "¼®±Ã" :
                   (item_typ == WPN_HAND_CROSSBOW) ? "¼ÒÇü ¼®±Ã" :
                   (item_typ == WPN_GLAIVE) ? "±Û·¹ÀÌºê" :
                   (item_typ == WPN_QUARTERSTAFF) ? "À°Ã´ºÀ" :
                   (item_typ == WPN_SCYTHE) ? "Å« ³´" :
                   (item_typ == WPN_EVENINGSTAR) ? "ÀÌºê´×½ºÅ¸" :
                   (item_typ == WPN_QUICK_BLADE) ? "Äü ºí·¹ÀÌµå" :
                   (item_typ == WPN_KATANA) ? "Ä«Å¸³ª" :
                   (item_typ == WPN_EXECUTIONERS_AXE) ? "ÁýÇàÀÎÀÇ µµ³¢" :
                   (item_typ == WPN_DOUBLE_SWORD) ? "ÀÌÁß°Ë" :
                   (item_typ == WPN_TRIPLE_SWORD) ? "»ïÁß°Ë" :
                   (item_typ == WPN_HAMMER) ? "¸ÁÄ¡" :
                   (item_typ == WPN_ANCUS) ? "¾ÓÄí½º" :
                   (item_typ == WPN_WHIP) ? "Ã¤Âï" :
                   (item_typ == WPN_SABRE) ? "»þº§" :
                   (item_typ == WPN_DEMON_BLADE) ? "¾Ç¸¶ÀÇ Ä®³¯" :
                   (item_typ == WPN_DEMON_WHIP) ? "¾Ç¸¶ÀÇ Ã¤Âï" :
                   (item_typ == WPN_DEMON_TRIDENT) ? "¾Ç¸¶ÀÇ »ïÁöÃ¢" :
                   (item_typ == WPN_BROAD_AXE) ? "´ëÇü µµ³¢" :
                   (item_typ == WPN_WAR_AXE) ? "±º¿ë µµ³¢" :
                   (item_typ == WPN_SPIKED_FLAIL) ? "¸ø¹ÚÈù ÇÁ·¹ÀÏ" :
                   (item_typ == WPN_GREAT_MACE) ? "´ëÇü ¸ÞÀÌ½º" :
                   (item_typ == WPN_GREAT_FLAIL) ? "´ëÇü ÇÁ·¹ÀÏ" :
                   (item_typ == WPN_FALCHION) ? "ÆÈÄ¡¿Â" :

           (item_typ == WPN_GIANT_CLUB)
                           ? (SysEnv.board_with_nail ? "°ïºÀ"
                                                     : "°Å´ë °ïºÀ") :

           (item_typ == WPN_GIANT_SPIKED_CLUB)
                           ? (SysEnv.board_with_nail ? "°ïºÀ"
                                                     : "¸ø¹ÚÈù °Å´ë °ïºÀ")

                                   : "unknown weapon");
#else
    strcpy(glorg,  (item_typ == WPN_CLUB) ? "club" :
                   (item_typ == WPN_MACE) ? "mace" :
                   (item_typ == WPN_FLAIL) ? "flail" :
                   (item_typ == WPN_KNIFE) ? "knife" :
                   (item_typ == WPN_DAGGER) ? "dagger" :
                   (item_typ == WPN_MORNINGSTAR) ? "morningstar" :
                   (item_typ == WPN_SHORT_SWORD) ? "short sword" :
                   (item_typ == WPN_LONG_SWORD) ? "long sword" :
                   (item_typ == WPN_GREAT_SWORD) ? "great sword" :
                   (item_typ == WPN_SCIMITAR) ? "scimitar" :
                   (item_typ == WPN_HAND_AXE) ? "hand axe" :
                   (item_typ == WPN_BATTLEAXE) ? "battleaxe" :
                   (item_typ == WPN_SPEAR) ? "spear" :
                   (item_typ == WPN_TRIDENT) ? "trident" :
                   (item_typ == WPN_HALBERD) ? "halberd" :
                   (item_typ == WPN_SLING) ? "sling" :
                   (item_typ == WPN_BOW) ? "bow" :
                   (item_typ == WPN_BLOWGUN) ? "blowgun" :
                   (item_typ == WPN_CROSSBOW) ? "crossbow" :
                   (item_typ == WPN_HAND_CROSSBOW) ? "hand crossbow" :
                   (item_typ == WPN_GLAIVE) ? "glaive" :
                   (item_typ == WPN_QUARTERSTAFF) ? "quarterstaff" :
                   (item_typ == WPN_SCYTHE) ? "scythe" :
                   (item_typ == WPN_EVENINGSTAR) ? "eveningstar" :
                   (item_typ == WPN_QUICK_BLADE) ? "quick blade" :
                   (item_typ == WPN_KATANA) ? "katana" :
                   (item_typ == WPN_EXECUTIONERS_AXE) ? "executioner's axe" :
                   (item_typ == WPN_DOUBLE_SWORD) ? "double sword" :
                   (item_typ == WPN_TRIPLE_SWORD) ? "triple sword" :
                   (item_typ == WPN_HAMMER) ? "hammer" :
                   (item_typ == WPN_ANCUS) ? "ancus" :
                   (item_typ == WPN_WHIP) ? "whip" :
                   (item_typ == WPN_SABRE) ? "sabre" :
                   (item_typ == WPN_DEMON_BLADE) ? "demon blade" :
                   (item_typ == WPN_DEMON_WHIP) ? "demon whip" :
                   (item_typ == WPN_DEMON_TRIDENT) ? "demon trident" :
                   (item_typ == WPN_BROAD_AXE) ? "broad axe" :
                   (item_typ == WPN_WAR_AXE) ? "war axe" :
                   (item_typ == WPN_SPIKED_FLAIL) ? "spiked flail" :
                   (item_typ == WPN_GREAT_MACE) ? "great mace" :
                   (item_typ == WPN_GREAT_FLAIL) ? "great flail" :
                   (item_typ == WPN_FALCHION) ? "falchion" :

           (item_typ == WPN_GIANT_CLUB)
                           ? (SysEnv.board_with_nail ? "two-by-four"
                                                     : "giant club") :

           (item_typ == WPN_GIANT_SPIKED_CLUB)
                           ? (SysEnv.board_with_nail ? "board with nail"
                                                     : "giant spiked club")

                                   : "unknown weapon");
#endif
}                               // end standard_name_weap()

void standard_name_armour( const item_def &item, char glorg[ITEMNAME_SIZE] )
{
    short helm_type;

    glorg[0] = '\0';

    switch (item.sub_type)
    {
    case ARM_ROBE:
#ifdef JP
        strcat(glorg, "·Îºê");
#else
        strcat(glorg, "robe");
#endif
        break;

    case ARM_LEATHER_ARMOUR:
#ifdef JP
        strcat(glorg, "°¡Á× °©¿Ê");
#else
        strcat(glorg, "leather armour");
#endif
        break;

    case ARM_RING_MAIL:
#ifdef JP
        strcat(glorg, "°í¸® °©¿Ê");
#else
        strcat(glorg, "ring mail");
#endif
        break;

    case ARM_SCALE_MAIL:
#ifdef JP
        strcat(glorg, "ºñ´Ã °©¿Ê");
#else
        strcat(glorg, "scale mail");
#endif
        break;

    case ARM_CHAIN_MAIL:
#ifdef JP
        strcat(glorg, "»ç½½ °©¿Ê");
#else
        strcat(glorg, "chain mail");
#endif
        break;

    case ARM_SPLINT_MAIL:
#ifdef JP
        strcat(glorg, "¹Ì´Ã °©¿Ê");
#else
        strcat(glorg, "splint mail");
#endif
        break;

    case ARM_BANDED_MAIL:
#ifdef JP
        strcat(glorg, "¶ì °©¿Ê");
#else
        strcat(glorg, "banded mail");
#endif
        break;

    case ARM_PLATE_MAIL:
#ifdef JP
        strcat(glorg, "ÆÇ±Ý °©¿Ê");
#else
        strcat(glorg, "plate mail");
#endif
        break;

    case ARM_SHIELD:
#ifdef JP
        strcat(glorg, "¹æÆÐ");
#else
        strcat(glorg, "shield");
#endif
        break;

    case ARM_CLOAK:
#ifdef JP
        strcat(glorg, "¸ÁÅä");
#else
        strcat(glorg, "cloak");
#endif
        break;

    case ARM_HELMET:
        if (cmp_helmet_type( item, THELM_HELM )
                    || cmp_helmet_type( item, THELM_HELMET ))
        {
            short dhelm = get_helmet_desc( item );

            if (dhelm != THELM_DESC_PLAIN)
            {
#ifdef JP
                strcat( glorg,
                        (dhelm == THELM_DESC_WINGED)   ? "³¯°³ ´Þ¸° " :
                        (dhelm == THELM_DESC_HORNED)   ? "»Ô ´Þ¸° " :
                        (dhelm == THELM_DESC_CRESTED)  ? "²À´ë±â Àå½ÄµÈ " :
                        (dhelm == THELM_DESC_PLUMED)   ? "±êÅÐ Àå½ÄµÈ " :
                        (dhelm == THELM_DESC_SPIKED)   ? "°¡½Ã ´Þ¸° " :
                        (dhelm == THELM_DESC_VISORED)  ? "Ã¬ ´Þ¸° " :
                        (dhelm == THELM_DESC_JEWELLED) ? "º¸¼®À¸·Î Àå½ÄµÈ "
                                                       : "¹ú·¹Åõ¼ºÀÌÀÇ " );
#else
                strcat( glorg,
                        (dhelm == THELM_DESC_WINGED)   ? "winged " :
                        (dhelm == THELM_DESC_HORNED)   ? "horned " :
                        (dhelm == THELM_DESC_CRESTED)  ? "crested " :
                        (dhelm == THELM_DESC_PLUMED)   ? "plumed " :
                        (dhelm == THELM_DESC_SPIKED)   ? "spiked " :
                        (dhelm == THELM_DESC_VISORED)  ? "visored " :
                        (dhelm == THELM_DESC_JEWELLED) ? "jeweled "
                                                       : "buggy " );
#endif
            }
        }

        helm_type = get_helmet_type( item );
        if (helm_type == THELM_HELM)
#ifdef JP
            strcat(glorg, "Åõ±¸");
#else
            strcat(glorg, "helm");
#endif
        else if (helm_type == THELM_CAP)
#ifdef JP
            strcat(glorg, "¸ðÀÚ");
#else
            strcat(glorg, "cap");
#endif
        else if (helm_type == THELM_WIZARD_HAT)
#ifdef JP
            strcat(glorg, "¸¶¹ý»çÀÇ ¸ðÀÚ");
#else
            strcat(glorg, "wizard's hat");
#endif
        else
#ifdef JP
            strcat(glorg, "Ã¶¸ð");
#else
            strcat(glorg, "helmet");
#endif
        break;

    case ARM_GLOVES:
#ifdef JP
        strcat(glorg, "Àå°©");
#else
        strcat(glorg, "gloves");
#endif
        break;

    case ARM_BOOTS:
        if (item.plus2 == TBOOT_NAGA_BARDING)
#ifdef JP
            strcat(glorg, "³ª°¡ ²¿¸®°©¿Ê");
#else
            strcat(glorg, "naga barding");
#endif
        else if (item.plus2 == TBOOT_CENTAUR_BARDING)
#ifdef JP
            strcat(glorg, "¼¾Å¸¿ì¸£ ¸¶°©");
#else
            strcat(glorg, "centaur barding");
#endif
        else
#ifdef JP
            strcat(glorg, "ÀåÈ­");
#else
            strcat(glorg, "boots");
#endif
        break;

    case ARM_BUCKLER:
#ifdef JP
        strcat(glorg, "ÀÛÀº ¿øÇü ¹æÆÐ");
#else
        strcat(glorg, "buckler");
#endif
        break;

    case ARM_LARGE_SHIELD:
#ifdef JP
        strcat(glorg, "Å« ¹æÆÐ");
#else
        strcat(glorg, "large shield");
#endif
        break;

    case ARM_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "¿ëÀÇ °¡Á×");
#else
        strcat(glorg, "dragon hide");
#endif
        break;

    case ARM_TROLL_HIDE:
#ifdef JP
        strcat(glorg, "Æ®·ÑÀÇ °¡Á×");
#else
        strcat(glorg, "troll hide");
#endif
        break;

    case ARM_CRYSTAL_PLATE_MAIL:
#ifdef JP
        strcat(glorg, "¼öÁ¤ ÆÇ±Ý °©¿Ê");
#else
        strcat(glorg, "crystal plate mail");
#endif
        break;

    case ARM_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "¿ëÀÇ °©¿Ê");
#else
        strcat(glorg, "dragon armour");
#endif
        break;

    case ARM_TROLL_LEATHER_ARMOUR:
#ifdef JP
        strcat(glorg, "Æ®·Ñ °¡Á× °©¿Ê");
#else
        strcat(glorg, "troll leather armour");
#endif
        break;

    case ARM_ICE_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "¾óÀ½ ¿ëÀÇ °¡Á×");
#else
        strcat(glorg, "ice dragon hide");
#endif
        break;

    case ARM_ICE_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "¾óÀ½ ¿ëÀÇ °©¿Ê");
#else
        strcat(glorg, "ice dragon armour");
#endif
        break;

    case ARM_STEAM_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "Áõ±â ¿ëÀÇ °¡Á×");
#else
        strcat(glorg, "steam dragon hide");
#endif
        break;

    case ARM_STEAM_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "Áõ±â ¿ëÀÇ °©¿Ê");
#else
        strcat(glorg, "steam dragon armour");
#endif
        break;

    case ARM_MOTTLED_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "¾ó·è¹«´Ì ¿ëÀÇ °¡Á×");
#else
        strcat(glorg, "mottled dragon hide");
#endif
        break;

    case ARM_MOTTLED_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "¾ó·è¹«´Ì ¿ëÀÇ °©¿Ê");
#else
        strcat(glorg, "mottled dragon armour");
#endif
        break;

    case ARM_STORM_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "ÆøÇ³ ¿ëÀÇ °¡Á×");
#else
        strcat(glorg, "storm dragon hide");
#endif
        break;

    case ARM_STORM_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "ÆøÇ³ ¿ëÀÇ °©¿Ê");
#else
        strcat(glorg, "storm dragon armour");
#endif
        break;

    case ARM_GOLD_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "È²±Ý ¿ëÀÇ °¡Á×");
#else
        strcat(glorg, "gold dragon hide");
#endif
        break;

    case ARM_GOLD_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "È²±Ý ¿ëÀÇ °©¿Ê");
#else
        strcat(glorg, "gold dragon armour");
#endif
        break;

    case ARM_ANIMAL_SKIN:
#ifdef JP
        strcat(glorg, "Áü½Â °¡Á×");
#else
        strcat(glorg, "animal skin");
#endif
        break;

    case ARM_SWAMP_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "´ËÁö ¿ëÀÇ °¡Á×");
#else
        strcat(glorg, "swamp dragon hide");
#endif
        break;

    case ARM_SWAMP_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "´ËÁö ¿ëÀÇ °©¿Ê");
#else
        strcat(glorg, "swamp dragon armour");
#endif
        break;
    }
}                               // end standard_name_armour()
