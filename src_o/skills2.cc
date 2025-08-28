/*
 *  File:       skills2.cc
 *  Summary:    More skill related functions.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *     01aug2000 jmf             RESTORED TITLES TO THEIR FORMER GLORY! MUA-HA!
 * <4> 22Jul2000 GDL             added warning for low throwing skill
 *                               Changed a few titles.
 * <3> 5/20/99   BWR             Changed Trapper titles, avoided
 *                               overflow on the weapon skill
 *                               column.
 * <2> -/--/--   WL              Extensive mods from Wladimir van der Laan.
 * <1> -/--/--   LRH             Created
 */

#include "AppHdr.h"
#include "skills2.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"
#include "fight.h"
#include "player.h"
#include "randart.h"
#include "religion.h"
#include "stuff.h"
#include "wpn-misc.h"
#include "view.h"

/* jmf: some references for words I used below:
 Peltast http://www.geocities.com/Athens/Aegean/9659/shields_main.htm
 Phalangite http://www.users.cts.com/funtv/j/jjartist/EpiroteScenario1.htm
 Yeoman: http://snt.student.utwente.nl/campus/sagi/artikel/longbow/longbow.html
*/

// Note:  Even though %s could be used with most of these, remember that
// the character's race will be listed on the next line.  Its only really
// intended for cases where things might be really awkward without it. -- bwr

const char *skills[50][6] = {
#ifdef JP
    {"전투", "정찰병", "보병", "숙련병", "전사", "역전의 용사"},      // 0
    {"단검", "찔러내는 자", "잘라내는 자", "단검술사", "단검의 광대", "칼날의 묘술사"},
    {"장검", "베어내는 자", "검사", "검의 숙달자", "검술가", "검무의 참격자"},
    {NULL},                     //  3- was: great swords {dlb}
    {"도끼", "나무꾼", "갈라내는 자", "참격자", "일도양단자", "선혈의 출몰자"},
    {"둔기", "후려치는 자", "쓰러트리는 자", "강타자", "분쇄자", "두개골 파쇄자"},   // 5
    {"창봉류 무기", "창지기", "%s의 창병", "창격병", "돌격병", "팔랑크스병"},
    {"봉술", "휘두르는 자", "부숴내는 자", "봉술사", "격파자", "일타의 격쇄자"},
#else
    {"Fighting", "Skirmisher", "Grunt", "Veteran", "Warrior", "Slayer"},      // 0
    {"Short Blades", "Stabber", "Cutter", "Knifefighter", "Eviscerator", "Blademaster"},
    {"Long Blades", "Slasher", "Slicer", "Fencer", "Swordfighter", "Swordmaster"},
    {NULL},                     //  3- was: great swords {dlb}
    {"Axes", "Chopper", "Cleaver", "Hacker", "Severer", "Axe Maniac"},
    {"Maces & Flails", "Basher", "Cudgeler", "Shatterer", "Bludgeoner", "Skullcrusher"},   // 5
    {"Polearms", "Spear-Bearer", "Pike-%s", "Phalangite", "Lancer", "Halberdier"},
    {"Staves", "Twirler", "Cruncher", "Smasher", "Stickfighter", "Skullbreaker"},
#endif

#ifdef JP
    {"투석기", "야만인", "선회시키는 자", "투석자", "%s의 투석기", "%s의 다윗"},
    {"활", "활잡이", "사수", "궁술사", "%s의 로빈 훗", "%s의 로빈 훗"},
    {"석궁", "사수", "궁술사", "사격의 명수", "%s의 화살폭격자", "%s의 화살폭격자"},     // 10
    {"다트", "다트 던지는 자", "다트 날리는 자", "다트에 숙련된 자", "%s 챔피언", "다트 세계 챔피언"},
    {"투척", "던지는 자", "날리는 자", "저격의 명수", "매의 눈", "예리한 저격수"},
#else
    {"Slings", "Vandal", "Slinger", "Whirler", "Crazy %s", "Very Crazy %s"},
    {"Bows", "Shooter", "Yeoman", "Archer", "Merry %s", "Merry %s"},
    {"Crossbows", "Shooter", "Sharpshooter", "Archer", "%s Ballista", "%s Ballista"},     // 10
    {"Darts", "Dart Thrower", "Hurler", "Hurler, First Class", "%s Darts Champion", "Universal Darts Champion"},
    {"Throwing", "Chucker", "Thrower", "Deadly Accurate", "Hawkeye", "Sniper"},
#endif

#ifdef JP
    {"투구와 갑옷", "가려지는 자", "지켜지는 자", "육지 거북이", "철벽 방어", "난공불락의 성"},
    {"회피", "조심스러운 자", "재빠른 자", "날파리", "경쾌한 묘기꾼", "신풍의 몸놀림"},
    {"은밀", "숨는 자", "좀도둑", "존재감 없는 녀석", "기척 없는 질주자", "투명 인간"},
    {"암습", "불량배", "강도", "불의의 습격자", "살인청부업자", "일순간의 배후자"},
    {"방패", "가로막는 자", "방패지기", "%s의 방호벽", "중장갑 보병", "최전선 요격자"},
    {"함정과 문", "찾아내는 자", "탐색자", "기술사", "건축설계자", "미궁의 주인"},
#else
    {"Armour", "Covered", "Protected", "Tortoise", "Impregnable", "Invulnerable"},
    {"Dodging", "Ducker", "Dodger", "Nimble", "Spry", "Acrobat"},
    {"Stealth", "Footpad", "Sneak", "Covert", "Unseen", "Imperceptible"},
    {"Stabbing", "Miscreant", "Blackguard", "Backstabber", "Cutthroat", "Politician"},
    {"Shields", "Shield-Bearer", "Blocker", "%s Barricade", "Peltast", "Hoplite"},
    {"Traps & Doors", "Disarmer", "Trapper", "Architect", "Engineer", "Dungeon Master"},
#endif

    // STR based fighters, for DEX/martial arts titles see below
#ifdef JP
    {"맨손 격투", "건달", "싸움꾼", "폭한", "격투가", "난투의 화신" },
#else
    {"Unarmed Combat", "Ruffian", "Grappler", "Brawler", "Wrestler", "Boxer" },
#endif

    {NULL},                     // 20- empty
    {NULL},                     // 21- empty
    {NULL},                     // 22- empty
    {NULL},                     // 23- empty
    {NULL},                     // 24- empty

#ifdef JP
    {"주문 시전", "외우는 자", "시전자", "정예 마술사", "통달의 마도사", "극의의 대현자"},     // 25
    {"요술", "해치는 자", "요술사", "파괴자", "괴멸자", "참멸의 주동자"},
    {"주술", "마력의 주입자", "주술의 취급자", "주술사", "현혹의 대가", "만물의 주박자"},
    {"소환술", "부르는 자", "소환사", "소집하는 자", "악마술사", "지옥을 부르는 자"},
    {"강령술 마법", "무덤을 더럽히는 자", "시체 사용자", "소생을 이루는 자", "죽음의 마술사", "%s의 사신"},
    {"전이술", "도약자", "순간이동자", "전이능력자", "공간의 사용자", "차원의 초월자"},       // 30
    {"변이술", "변화하는 자", "변신능력자", "변천하는 자", "연금술사", "변이의 지배자"},
    {"예견술", "손금쟁이", "점쟁이", "예언자", "미래를 보는 자", "신탁자"},
#else
    {"Spellcasting", "Magician", "Thaumaturge", "Eclecticist", "Sorcerer", "Archmage"},     // 25
    {"Conjurations", "Ruinous", "Conjurer", "Destroyer", "Devastator", "Annihilator"},
    {"Enchantments", "Charm-Maker", "Infuser", "Bewitcher", "Enchanter", "Spellbinder"},
    {"Summonings", "Caller", "Summoner", "Convoker", "Demonologist", "Hellbinder"},
    {"Necromancy", "Grave Robber", "Reanimator", "Necromancer", "Thanatomancer", "%s of Death"},
    {"Translocations", "Jumper", "Blinker", "Shifter", "Portalist", "Plane Walker"},       // 30
    {"Transmigration", "Changer", "Transmogrifier", "Transformer", "Alchemist", "Transmuter"},
    {"Divinations", "Seer", "Soothsayer", "Diviner", "Augur", "Oracle"},
#endif

#ifdef JP
    {"불의 마술", "불장난하는 자", "방화자", "화염의 마술사", "멸화의 마도사", "겁화를 짊어진 자"},
    {"얼음의 마술", "저체온증자", "서리의 마술사", "얼음의 마법사", "냉기의 마도사", "빙하의 창조자"},
    {"바람의 마술", "바람을 불러오는 자", "구름의 마술사", "대기의 마력자", "창공의 마도사", "폭풍우의 눈"}, // 35
    {"땅의 마술", "채굴자", "흙점쟁이", "반석의 마술사", "금속의 마도사", "대지를 뒤흔드는 자"},
    {"독의 마술", "피를 더럽히는 자", "독극물의 연구자", "독침술사", "병독의 해방자", "맹독의 유린자"},
#else
    {"Fire Magic", "Firebug", "Arsonist", "Scorcher", "Pyromancer", "Infernalist"},
    {"Ice Magic", "Chiller", "Frost Mage", "Ice Mage", "Cryomancer", "Englaciator"},
    {"Air Magic", "Wind Mage", "Cloud Mage", "Air Mage", "Sky Mage", "Storm Mage"}, // 35
    {"Earth Magic", "Digger", "Geomancer", "Earth Mage", "Metallomancer", "Petrodigitator"},
    {"Poison Magic", "Stinger", "Tainter", "Polluter", "Poisoner", "Envenomancer"},
#endif

#ifdef JP
    {"기도", "신도", "감천의 신봉자", "지상의 대리자", "기적의 집행자", "삼위일체자"},  // 38
    {"발동", "영물의 습득자", "골동품상", "고고학자", "고대의 발동자", "영험한 추대자"}, // 39
#else
    {"Invocations", "Believer", "Servant", "Worldly Agent", "Theurge", "Avatar"},  // 38
    {"Evocations", "Charlatan", "Prestidigitator", "Fetichist", "Evocator", "Talismancer"}, // 39
#endif

/*NOTE: If more skills are added, must change ranges in level_change() in player.cc */
/*{"",             "", "", "", ""}, */

    {NULL},                     // 40- empty
    {NULL},                     // 41- empty
    {NULL},                     // 42- empty
    {NULL},                     // 43- empty
    {NULL},                     // 44- empty
    {NULL},                     // 45- empty
    {NULL},                     // 46- empty
    {NULL},                     // 47- empty
    {NULL},                     // 48- empty
    {NULL}                      // 49- empty  {end of array}
};

const char *martial_arts_titles[6] =
#ifdef JP
    {"맨손 격투", "문하생", "유단자", "무술 사범", "유파의 당주", "권법의 달인"};
#else
    {"Unarmed Combat", "Martial Artist", "Black Belt", "Sensei", "Master", "Grand Master"};
#endif


/* Note that this (humans have 100 for all skills) is assumed in the
   level_change function in player.cc, if CLASSES is def'd

   3.10: but it never is, and CLASSES is probably broken now. Anyway,
   the Spellcasting skill (25) is actually about 130% of what is shown here.
 */
const int spec_skills[ NUM_SPECIES ][40] = {
    {                           // SP_HUMAN (1)
     100,                       // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     100,                       // SK_SLINGS
     100,                       // SK_BOWS
     100,                       // SK_CROSSBOWS
     100,                       // SK_DARTS
     100,                       // SK_THROWING
     100,                       // SK_ARMOUR
     100,                       // SK_DODGING
     100,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     100,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_ELF (2)
     120,                       // SK_FIGHTING
     80,                        // SK_SHORT_BLADES
     80,                        // SK_LONG_SWORDS
     110,                       // SK_UNUSED_1
     120,                       // SK_AXES
     130,                       // SK_MACES_FLAILS
     130,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     60,                        // SK_BOWS
     100,                       // SK_CROSSBOWS
     90,                        // SK_DARTS
     80,                        // SK_THROWING
     120,                       // SK_ARMOUR
     80,                        // SK_DODGING
     80,                        // SK_STEALTH
     100,                       // SK_STABBING
     120,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     110,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     80,                        // SK_SPELLCASTING
     105,                       // SK_CONJURATIONS
     70,                        // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     120,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     70,                        // SK_AIR_MAGIC
     130,                       // SK_EARTH_MAGIC
     110,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     80,                        // SK_EVOCATIONS
     },

    {                           // SP_HIGH_ELF (3)
     100,                       // SK_FIGHTING
     70,                        // SK_SHORT_BLADES
     70,                        // SK_LONG_SWORDS
     115,                       // SK_UNUSED_1
     130,                       // SK_AXES
     150,                       // SK_MACES_FLAILS
     150,                       // SK_POLEARMS
     100,                       // SK_STAVES
     140,                       // SK_SLINGS
     60,                        // SK_BOWS
     100,                       // SK_CROSSBOWS
     90,                        // SK_DARTS
     80,                        // SK_THROWING
     110,                       // SK_ARMOUR
     90,                        // SK_DODGING
     90,                        // SK_STEALTH
     110,                       // SK_STABBING
     110,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     130,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     70,                        // SK_SPELLCASTING
     90,                        // SK_CONJURATIONS
     70,                        // SK_ENCHANTMENTS
     110,                       // SK_SUMMONINGS
     130,                       // SK_NECROMANCY
     90,                        // SK_TRANSLOCATIONS
     90,                        // SK_TRANSMIGRATION
     110,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     70,                        // SK_AIR_MAGIC
     130,                       // SK_EARTH_MAGIC
     130,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_GREY_ELF (4)
     140,                       // SK_FIGHTING
     90,                        // SK_SHORT_BLADES
     95,                        // SK_LONG_SWORDS
     120,                       // SK_UNUSED_1
     140,                       // SK_AXES
     160,                       // SK_MACES_FLAILS
     160,                       // SK_POLEARMS
     100,                       // SK_STAVES
     130,                       // SK_SLINGS
     70,                        // SK_BOWS
     100,                       // SK_CROSSBOWS
     90,                        // SK_DARTS
     80,                        // SK_THROWING
     140,                       // SK_ARMOUR
     75,                        // SK_DODGING
     70,                        // SK_STEALTH
     100,                       // SK_STABBING
     140,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     130,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     60,                        // SK_SPELLCASTING
     90,                        // SK_CONJURATIONS
     50,                        // SK_ENCHANTMENTS
     90,                        // SK_SUMMONINGS
     130,                       // SK_NECROMANCY
     80,                        // SK_TRANSLOCATIONS
     80,                        // SK_TRANSMIGRATION
     80,                        // SK_DIVINATIONS
     90,                        // SK_FIRE_MAGIC
     90,                        // SK_ICE_MAGIC
     60,                        // SK_AIR_MAGIC
     150,                       // SK_EARTH_MAGIC
     110,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     90,                        // SK_EVOCATIONS
     },

    {                           // SP_DEEP_ELF (5)
     150,                       // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     105,                       // SK_LONG_SWORDS
     120,                       // SK_UNUSED_1
     150,                       // SK_AXES
     165,                       // SK_MACES_FLAILS
     165,                       // SK_POLEARMS
     100,                       // SK_STAVES
     135,                       // SK_SLINGS
     74,                        // SK_BOWS
     75,                        // SK_CROSSBOWS
     75,                        // SK_DARTS
     80,                        // SK_THROWING
     140,                       // SK_ARMOUR
     70,                        // SK_DODGING
     65,                        // SK_STEALTH
     80,                        // SK_STABBING
     140,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     130,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     55,                        // SK_SPELLCASTING
     80,                        // SK_CONJURATIONS
     50,                        // SK_ENCHANTMENTS
     80,                        // SK_SUMMONINGS
     70,                        // SK_NECROMANCY
     75,                        // SK_TRANSLOCATIONS
     75,                        // SK_TRANSMIGRATION
     75,                        // SK_DIVINATIONS
     90,                        // SK_FIRE_MAGIC
     90,                        // SK_ICE_MAGIC
     80,                        // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     80,                        // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     90,                        // SK_EVOCATIONS
     },

    {                           // SP_SLUDGE_ELF (6)
     80,                        // SK_FIGHTING
     110,                       // SK_SHORT_BLADES
     110,                       // SK_LONG_SWORDS
     110,                       // SK_UNUSED_1
     130,                       // SK_AXES
     140,                       // SK_MACES_FLAILS
     140,                       // SK_POLEARMS
     100,                       // SK_STAVES
     100,                       // SK_SLINGS
     100,                       // SK_BOWS
     100,                       // SK_CROSSBOWS
     100,                       // SK_DARTS
     70,                        // SK_THROWING
     140,                       // SK_ARMOUR
     70,                        // SK_DODGING
     75,                        // SK_STEALTH
     100,                       // SK_STABBING
     130,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     80,                        // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     70,                        // SK_SPELLCASTING
     130,                       // SK_CONJURATIONS
     130,                       // SK_ENCHANTMENTS
     90,                        // SK_SUMMONINGS
     90,                        // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     60,                        // SK_TRANSMIGRATION
     130,                       // SK_DIVINATIONS
     80,                        // SK_FIRE_MAGIC
     80,                        // SK_ICE_MAGIC
     80,                        // SK_AIR_MAGIC
     80,                        // SK_EARTH_MAGIC
     80,                        // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     110,                       // SK_EVOCATIONS
     },

    {                           // SP_HILL_DWARF (7)
     70,                        // SK_FIGHTING
     80,                        // SK_SHORT_BLADES
     80,                        // SK_LONG_SWORDS
     90,                        // SK_UNUSED_1
     60,                        // SK_AXES
     70,                        // SK_MACES_FLAILS
     110,                       // SK_POLEARMS
     130,                       // SK_STAVES
     130,                       // SK_SLINGS
     150,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     70,                        // SK_ARMOUR
     120,                       // SK_DODGING
     150,                       // SK_STEALTH
     140,                       // SK_STABBING
     70,                        // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     160,                       // SK_SPELLCASTING
     120,                       // SK_CONJURATIONS
     150,                       // SK_ENCHANTMENTS
     150,                       // SK_SUMMONINGS
     160,                       // SK_NECROMANCY
     150,                       // SK_TRANSLOCATIONS
     120,                       // SK_TRANSMIGRATION
     130,                       // SK_DIVINATIONS
     80,                        // SK_FIRE_MAGIC
     120,                       // SK_ICE_MAGIC
     150,                       // SK_AIR_MAGIC
     70,                        // SK_EARTH_MAGIC
     130,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     60,                        // SK_EVOCATIONS
     },

    {                           // SP_MOUNTAIN_DWARF (8)
     70,                        // SK_FIGHTING
     90,                        // SK_SHORT_BLADES
     90,                        // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     70,                        // SK_AXES
     70,                        // SK_MACES_FLAILS
     110,                       // SK_POLEARMS
     120,                       // SK_STAVES
     125,                       // SK_SLINGS
     140,                       // SK_BOWS
     100,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     115,                       // SK_THROWING
     60,                        // SK_ARMOUR
     110,                       // SK_DODGING
     140,                       // SK_STEALTH
     130,                       // SK_STABBING
     70,                        // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     140,                       // SK_SPELLCASTING
     115,                       // SK_CONJURATIONS
     135,                       // SK_ENCHANTMENTS
     150,                       // SK_SUMMONINGS
     160,                       // SK_NECROMANCY
     150,                       // SK_TRANSLOCATIONS
     120,                       // SK_TRANSMIGRATION
     130,                       // SK_DIVINATIONS
     70,                        // SK_FIRE_MAGIC
     130,                       // SK_ICE_MAGIC
     150,                       // SK_AIR_MAGIC
     70,                        // SK_EARTH_MAGIC
     130,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     70,                        // SK_EVOCATIONS
     },

    {                           // SP_HALFLING (9)
     120,                       // SK_FIGHTING
     60,                        // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     130,                       // SK_UNUSED_1
     120,                       // SK_AXES
     150,                       // SK_MACES_FLAILS
     160,                       // SK_POLEARMS
     130,                       // SK_STAVES
     50,                        // SK_SLINGS
     70,                        // SK_BOWS
     90,                        // SK_CROSSBOWS
     50,                        // SK_DARTS
     60,                        // SK_THROWING
     150,                       // SK_ARMOUR
     70,                        // SK_DODGING
     60,                        // SK_STEALTH
     70,                        // SK_STABBING
     130,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     140,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     130,                       // SK_SPELLCASTING
     130,                       // SK_CONJURATIONS
     100,                       // SK_ENCHANTMENTS
     120,                       // SK_SUMMONINGS
     150,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     150,                       // SK_TRANSMIGRATION
     140,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     90,                        // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     120,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     90,                        // SK_EVOCATIONS
     },

    {                           // SP_HILL_ORC (10)
     70,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     80,                        // SK_LONG_SWORDS
     70,                        // SK_UNUSED_1
     70,                        // SK_AXES
     80,                        // SK_MACES_FLAILS
     80,                        // SK_POLEARMS
     110,                       // SK_STAVES
     130,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     130,                       // SK_DARTS
     130,                       // SK_THROWING
     90,                        // SK_ARMOUR
     140,                       // SK_DODGING
     150,                       // SK_STEALTH
     100,                       // SK_STABBING
     80,                        // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     90,                        // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     150,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     120,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     150,                       // SK_TRANSLOCATIONS
     160,                       // SK_TRANSMIGRATION
     160,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     150,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     110,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_KOBOLD (11)
     80,                        // SK_FIGHTING
     60,                        // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     120,                       // SK_UNUSED_1
     110,                       // SK_AXES
     140,                       // SK_MACES_FLAILS
     150,                       // SK_POLEARMS
     110,                       // SK_STAVES
     70,                        // SK_SLINGS
     80,                        // SK_BOWS
     90,                        // SK_CROSSBOWS
     50,                        // SK_DARTS
     60,                        // SK_THROWING
     140,                       // SK_ARMOUR
     70,                        // SK_DODGING
     60,                        // SK_STEALTH
     70,                        // SK_STABBING
     130,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     110,                       // SK_SPELLCASTING
     110,                       // SK_CONJURATIONS
     110,                       // SK_ENCHANTMENTS
     105,                       // SK_SUMMONINGS
     105,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     110,                       // SK_TRANSMIGRATION
     130,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     80,                        // SK_EVOCATIONS
     },

    {                           // SP_MUMMY (12)
     100,                       // SK_FIGHTING
     140,                       // SK_SHORT_BLADES
     140,                       // SK_LONG_SWORDS
     140,                       // SK_UNUSED_1
     140,                       // SK_AXES
     140,                       // SK_MACES_FLAILS
     140,                       // SK_POLEARMS
     140,                       // SK_STAVES
     140,                       // SK_SLINGS
     140,                       // SK_BOWS
     140,                       // SK_CROSSBOWS
     140,                       // SK_DARTS
     140,                       // SK_THROWING
     140,                       // SK_ARMOUR
     140,                       // SK_DODGING
     140,                       // SK_STEALTH
     140,                       // SK_STABBING
     140,                       // SK_SHIELDS
     140,                       // SK_TRAPS_DOORS
     140,                       // SK_UNARMED_COMBAT
     140,                       // undefined
     140,                       // undefined
     140,                       // undefined
     140,                       // undefined
     140,                       // undefined
     100,                       // SK_SPELLCASTING
     140,                       // SK_CONJURATIONS
     140,                       // SK_ENCHANTMENTS
     140,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     140,                       // SK_TRANSLOCATIONS
     140,                       // SK_TRANSMIGRATION
     140,                       // SK_DIVINATIONS
     140,                       // SK_FIRE_MAGIC
     140,                       // SK_ICE_MAGIC
     140,                       // SK_AIR_MAGIC
     140,                       // SK_EARTH_MAGIC
     140,                       // SK_POISON_MAGIC
     140,                       // SK_INVOCATIONS
     140,                       // SK_EVOCATIONS
     },

    {                           // SP_NAGA (13)
     100,                       // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     120,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     150,                       // SK_ARMOUR
     150,                       // SK_DODGING
     40,                        // SK_STEALTH
     100,                       // SK_STABBING
     140,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     100,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     60,                        // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_GNOME (14)
     100,                       // SK_FIGHTING
     75,                        // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     130,                       // SK_UNUSED_1
     100,                       // SK_AXES
     130,                       // SK_MACES_FLAILS
     140,                       // SK_POLEARMS
     130,                       // SK_STAVES
     80,                        // SK_SLINGS
     100,                       // SK_BOWS
     90,                        // SK_CROSSBOWS
     60,                        // SK_DARTS
     100,                       // SK_THROWING
     150,                       // SK_ARMOUR
     70,                        // SK_DODGING
     70,                        // SK_STEALTH
     80,                        // SK_STABBING
     120,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     110,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     120,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     100,                       // SK_ENCHANTMENTS
     110,                       // SK_SUMMONINGS
     130,                       // SK_NECROMANCY
     130,                       // SK_TRANSLOCATIONS
     120,                       // SK_TRANSMIGRATION
     120,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     170,                       // SK_AIR_MAGIC
     60,                        // SK_EARTH_MAGIC
     130,                       // SK_POISON_MAGIC
     120,                       // SK_INVOCATIONS
     60,                        // SK_EVOCATIONS
     },

    {                           // SP_OGRE (15)
     100,                       // SK_FIGHTING
     140,                       // SK_SHORT_BLADES
     120,                       // SK_LONG_SWORDS
     110,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     110,                       // SK_POLEARMS
     120,                       // SK_STAVES
     150,                       // SK_SLINGS
     150,                       // SK_BOWS
     180,                       // SK_CROSSBOWS
     150,                       // SK_DARTS
     100,                       // SK_THROWING
     140,                       // SK_ARMOUR
     150,                       // SK_DODGING
     200,                       // SK_STEALTH
     150,                       // SK_STABBING
     110,                       // SK_SHIELDS
     200,                       // SK_TRAPS_DOORS
     130,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     220,                       // SK_SPELLCASTING
     180,                       // SK_CONJURATIONS
     220,                       // SK_ENCHANTMENTS
     200,                       // SK_SUMMONINGS
     150,                       // SK_NECROMANCY
     200,                       // SK_TRANSLOCATIONS
     200,                       // SK_TRANSMIGRATION
     200,                       // SK_DIVINATIONS
     150,                       // SK_FIRE_MAGIC
     150,                       // SK_ICE_MAGIC
     200,                       // SK_AIR_MAGIC
     120,                       // SK_EARTH_MAGIC
     150,                       // SK_POISON_MAGIC
     130,                       // SK_INVOCATIONS
     170,                       // SK_EVOCATIONS
     },

    {                           // SP_TROLL (16)
     140,                       // SK_FIGHTING
     150,                       // SK_SHORT_BLADES
     150,                       // SK_LONG_SWORDS
     150,                       // SK_UNUSED_1
     150,                       // SK_AXES
     130,                       // SK_MACES_FLAILS
     150,                       // SK_POLEARMS
     150,                       // SK_STAVES
     180,                       // SK_SLINGS
     180,                       // SK_BOWS
     180,                       // SK_CROSSBOWS
     180,                       // SK_DARTS
     130,                       // SK_THROWING
     150,                       // SK_ARMOUR
     130,                       // SK_DODGING
     250,                       // SK_STEALTH
     150,                       // SK_STABBING
     150,                       // SK_SHIELDS
     200,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     200,                       // SK_SPELLCASTING
     160,                       // SK_CONJURATIONS
     200,                       // SK_ENCHANTMENTS
     160,                       // SK_SUMMONINGS
     150,                       // SK_NECROMANCY
     160,                       // SK_TRANSLOCATIONS
     160,                       // SK_TRANSMIGRATION
     200,                       // SK_DIVINATIONS
     160,                       // SK_FIRE_MAGIC
     160,                       // SK_ICE_MAGIC
     200,                       // SK_AIR_MAGIC
     120,                       // SK_EARTH_MAGIC
     160,                       // SK_POISON_MAGIC
     150,                       // SK_INVOCATIONS
     180,                       // SK_EVOCATIONS
     },

    {                           // SP_OGRE_MAGE (17)
     100,                       // SK_FIGHTING
     110,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     150,                       // SK_SLINGS
     150,                       // SK_BOWS
     150,                       // SK_CROSSBOWS
     150,                       // SK_DARTS
     150,                       // SK_THROWING
     170,                       // SK_ARMOUR
     130,                       // SK_DODGING
     100,                       // SK_STEALTH
     130,                       // SK_STABBING
     150,                       // SK_SHIELDS
     150,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     70,                        // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     80,                        // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                        // SK_EVOCATIONS
     },

    {                           // SP_RED_DRACONIAN (18)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     70,                        // SK_FIRE_MAGIC
     150,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_WHITE_DRACONIAN (19)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     150,                       // SK_FIRE_MAGIC
     70,                        // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_GREEN_DRACONIAN (20)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     70,                        // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_YELLOW_DRACONIAN (21)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_GREY_DRACONIAN (22)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_BLACK_DRACONIAN (23)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     70,                        // SK_AIR_MAGIC
     150,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_PURPLE_DRACONIAN (24)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     70,                        // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     90,                        // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     90,                        // SK_EVOCATIONS
     },

    {                           // SP_MOTTLED_DRACONIAN (25)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     80,                        // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_PALE_DRACONIAN (26)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     90,                        // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     90,                        // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     90,                        // SK_EVOCATIONS
     },

    {                           // SP_UNK0_DRACONAIN (27)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_UNK1_DRACONIAN (28)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_UNK2_DRACONIAN (29)
     90,                        // SK_FIGHTING
     100,                       // SK_SHORT_BLADES
     100,                       // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     100,                       // SK_AXES
     100,                       // SK_MACES_FLAILS
     100,                       // SK_POLEARMS
     100,                       // SK_STAVES
     120,                       // SK_SLINGS
     120,                       // SK_BOWS
     120,                       // SK_CROSSBOWS
     120,                       // SK_DARTS
     120,                       // SK_THROWING
     200,                       // SK_ARMOUR
     120,                       // SK_DODGING
     120,                       // SK_STEALTH
     100,                       // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     120,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     100,                       // SK_TRANSLOCATIONS
     100,                       // SK_TRANSMIGRATION
     100,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     100,                       // SK_ICE_MAGIC
     100,                       // SK_AIR_MAGIC
     100,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_CENTAUR (30)
     100,                       // SK_FIGHTING
     120,                       // SK_SHORT_BLADES
     110,                       // SK_LONG_SWORDS
     110,                       // SK_UNUSED_1
     110,                       // SK_AXES
     110,                       // SK_MACES_FLAILS
     110,                       // SK_POLEARMS
     110,                       // SK_STAVES
     75,                        // SK_SLINGS
     60,                        // SK_BOWS
     85,                        // SK_CROSSBOWS
     80,                        // SK_DARTS
     60,                        // SK_THROWING
     180,                       // SK_ARMOUR
     170,                       // SK_DODGING
     200,                       // SK_STEALTH
     170,                       // SK_STABBING
     180,                       // SK_SHIELDS
     150,                       // SK_TRAPS_DOORS
     100,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     140,                       // SK_SPELLCASTING
     120,                       // SK_CONJURATIONS
     110,                       // SK_ENCHANTMENTS
     120,                       // SK_SUMMONINGS
     120,                       // SK_NECROMANCY
     120,                       // SK_TRANSLOCATIONS
     120,                       // SK_TRANSMIGRATION
     130,                       // SK_DIVINATIONS
     120,                       // SK_FIRE_MAGIC
     120,                       // SK_ICE_MAGIC
     120,                       // SK_AIR_MAGIC
     120,                       // SK_EARTH_MAGIC
     130,                       // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     130,                       // SK_EVOCATIONS
     },

    {                           // SP_DEMIGOD (31)
     110,                       // SK_FIGHTING
     110,                       // SK_SHORT_BLADES
     110,                       // SK_LONG_SWORDS
     110,                       // SK_UNUSED_1
     110,                       // SK_AXES
     110,                       // SK_MACES_FLAILS
     110,                       // SK_POLEARMS
     110,                       // SK_STAVES
     110,                       // SK_SLINGS
     110,                       // SK_BOWS
     110,                       // SK_CROSSBOWS
     110,                       // SK_DARTS
     110,                       // SK_THROWING
     110,                       // SK_ARMOUR
     110,                       // SK_DODGING
     110,                       // SK_STEALTH
     110,                       // SK_STABBING
     110,                       // SK_SHIELDS
     110,                       // SK_TRAPS_DOORS
     110,                       // SK_UNARMED_COMBAT
     110,                       // undefined
     110,                       // undefined
     110,                       // undefined
     110,                       // undefined
     110,                       // undefined
     110,                       // SK_SPELLCASTING
     110,                       // SK_CONJURATIONS
     110,                       // SK_ENCHANTMENTS
     110,                       // SK_SUMMONINGS
     110,                       // SK_NECROMANCY
     110,                       // SK_TRANSLOCATIONS
     110,                       // SK_TRANSMIGRATION
     110,                       // SK_DIVINATIONS
     110,                       // SK_FIRE_MAGIC
     110,                       // SK_ICE_MAGIC
     110,                       // SK_AIR_MAGIC
     110,                       // SK_EARTH_MAGIC
     110,                       // SK_POISON_MAGIC
     110,                       // SK_INVOCATIONS
     110,                       // SK_EVOCATIONS
     },

    {                           // SP_SPRIGGAN (32)
     150,                       // SK_FIGHTING
     90,                        // SK_SHORT_BLADES
     140,                       // SK_LONG_SWORDS
     160,                       // SK_UNUSED_1
     150,                       // SK_AXES
     160,                       // SK_MACES_FLAILS
     180,                       // SK_POLEARMS
     150,                       // SK_STAVES
     70,                        // SK_SLINGS
     70,                        // SK_BOWS
     100,                       // SK_CROSSBOWS
     70,                        // SK_DARTS
     90,                        // SK_THROWING
     170,                       // SK_ARMOUR
     50,                        // SK_DODGING
     50,                        // SK_STEALTH
     50,                        // SK_STABBING
     180,                       // SK_SHIELDS
     60,                        // SK_TRAPS_DOORS
     130,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     60,                        // SK_SPELLCASTING
     160,                       // SK_CONJURATIONS
     50,                        // SK_ENCHANTMENTS
     150,                       // SK_SUMMONINGS
     120,                       // SK_NECROMANCY
     50,                        // SK_TRANSLOCATIONS
     60,                        // SK_TRANSMIGRATION
     70,                        // SK_DIVINATIONS
     140,                       // SK_FIRE_MAGIC
     140,                       // SK_ICE_MAGIC
     120,                       // SK_AIR_MAGIC
     120,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     130,                       // SK_INVOCATIONS
     70,                        // SK_EVOCATIONS
     },

    {                           // SP_MINOTAUR (33)
     70,                        // SK_FIGHTING
     70,                        // SK_SHORT_BLADES
     70,                        // SK_LONG_SWORDS
     70,                        // SK_UNUSED_1
     70,                        // SK_AXES
     70,                        // SK_MACES_FLAILS
     70,                        // SK_POLEARMS
     70,                        // SK_STAVES
     90,                        // SK_SLINGS
     90,                        // SK_BOWS
     90,                        // SK_CROSSBOWS
     90,                        // SK_DARTS
     90,                        // SK_THROWING
     80,                        // SK_ARMOUR
     80,                        // SK_DODGING
     130,                       // SK_STEALTH
     100,                       // SK_STABBING
     80,                        // SK_SHIELDS
     120,                       // SK_TRAPS_DOORS
     80,                        // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     180,                       // SK_SPELLCASTING
     170,                       // SK_CONJURATIONS
     170,                       // SK_ENCHANTMENTS
     170,                       // SK_SUMMONINGS
     170,                       // SK_NECROMANCY
     170,                       // SK_TRANSLOCATIONS
     170,                       // SK_TRANSMIGRATION
     170,                       // SK_DIVINATIONS
     170,                       // SK_FIRE_MAGIC
     170,                       // SK_ICE_MAGIC
     170,                       // SK_AIR_MAGIC
     170,                       // SK_EARTH_MAGIC
     170,                       // SK_POISON_MAGIC
     130,                       // SK_INVOCATIONS
     170,                       // SK_EVOCATIONS
     },

    {                           // SP_DEMONSPAN (34)
     100,                       // SK_FIGHTING
     110,                       // SK_SHORT_BLADES
     110,                       // SK_LONG_SWORDS
     110,                       // SK_UNUSED_1
     110,                       // SK_AXES
     110,                       // SK_MACES_FLAILS
     110,                       // SK_POLEARMS
     110,                       // SK_STAVES
     110,                       // SK_SLINGS
     110,                       // SK_BOWS
     110,                       // SK_CROSSBOWS
     110,                       // SK_DARTS
     110,                       // SK_THROWING
     110,                       // SK_ARMOUR
     110,                       // SK_DODGING
     110,                       // SK_STEALTH
     110,                       // SK_STABBING
     110,                       // SK_SHIELDS
     110,                       // SK_TRAPS_DOORS
     110,                       // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     100,                       // SK_CONJURATIONS
     110,                       // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     90,                        // SK_NECROMANCY
     110,                       // SK_TRANSLOCATIONS
     110,                       // SK_TRANSMIGRATION
     110,                       // SK_DIVINATIONS
     100,                       // SK_FIRE_MAGIC
     110,                       // SK_ICE_MAGIC
     110,                       // SK_AIR_MAGIC
     110,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     80,                        // SK_INVOCATIONS
     110,                       // SK_EVOCATIONS
     },

    {                           // SP_GHOUL (35)
     80,                        // SK_FIGHTING
     110,                       // SK_SHORT_BLADES
     110,                       // SK_LONG_SWORDS
     110,                       // SK_UNUSED_1
     110,                       // SK_AXES
     110,                       // SK_MACES_FLAILS
     110,                       // SK_POLEARMS
     110,                       // SK_STAVES
     130,                       // SK_SLINGS
     130,                       // SK_BOWS
     130,                       // SK_CROSSBOWS
     130,                       // SK_DARTS
     130,                       // SK_THROWING
     110,                       // SK_ARMOUR
     110,                       // SK_DODGING
     80,                        // SK_STEALTH
     100,                       // SK_STABBING
     110,                       // SK_SHIELDS
     120,                       // SK_TRAPS_DOORS
     80,                        // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     120,                       // SK_SPELLCASTING
     130,                       // SK_CONJURATIONS
     130,                       // SK_ENCHANTMENTS
     120,                       // SK_SUMMONINGS
     100,                       // SK_NECROMANCY
     120,                       // SK_TRANSLOCATIONS
     120,                       // SK_TRANSMIGRATION
     120,                       // SK_DIVINATIONS
     150,                       // SK_FIRE_MAGIC
     90,                        // SK_ICE_MAGIC
     150,                       // SK_AIR_MAGIC
     90,                        // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     110,                       // SK_INVOCATIONS
     130,                       // SK_EVOCATIONS
     },

    {                           // SP_KENKU (36)
     100,                       // SK_FIGHTING
     75,                        // SK_SHORT_BLADES
     75,                        // SK_LONG_SWORDS
     75,                        // SK_UNUSED_1
     75,                        // SK_AXES
     75,                        // SK_MACES_FLAILS
     75,                        // SK_POLEARMS
     75,                        // SK_STAVES
     100,                       // SK_SLINGS
     80,                        // SK_BOWS
     80,                        // SK_CROSSBOWS
     90,                        // SK_DARTS
     90,                        // SK_THROWING
     90,                        // SK_ARMOUR
     90,                        // SK_DODGING
     100,                       // SK_STEALTH
     80,                        // SK_STABBING
     100,                       // SK_SHIELDS
     100,                       // SK_TRAPS_DOORS
     80,                        // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     60,                        // SK_CONJURATIONS
     160,                       // SK_ENCHANTMENTS
     70,                        // SK_SUMMONINGS
     80,                        // SK_NECROMANCY
     150,                       // SK_TRANSLOCATIONS
     150,                       // SK_TRANSMIGRATION
     180,                       // SK_DIVINATIONS
     90,                        // SK_FIRE_MAGIC
     120,                       // SK_ICE_MAGIC
     90,                        // SK_AIR_MAGIC
     120,                       // SK_EARTH_MAGIC
     100,                       // SK_POISON_MAGIC
     160,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },

    {                           // SP_MERFOLK (37)
     80,                        // SK_FIGHTING
     70,                        // SK_SHORT_BLADES
     90,                        // SK_LONG_SWORDS
     100,                       // SK_UNUSED_1
     140,                       // SK_AXES
     150,                       // SK_MACES_FLAILS
     50,                        // SK_POLEARMS
     130,                       // SK_STAVES
     150,                       // SK_SLINGS
     140,                       // SK_BOWS
     140,                       // SK_CROSSBOWS
     100,                       // SK_DARTS
     100,                       // SK_THROWING
     160,                       // SK_ARMOUR
     60,                        // SK_DODGING
     90,                        // SK_STEALTH
     70,                        // SK_STABBING
     100,                       // SK_SHIELDS
     120,                       // SK_TRAPS_DOORS
     90,                        // SK_UNARMED_COMBAT
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // undefined
     100,                       // SK_SPELLCASTING
     140,                       // SK_CONJURATIONS
     90,                        // SK_ENCHANTMENTS
     100,                       // SK_SUMMONINGS
     150,                       // SK_NECROMANCY
     140,                       // SK_TRANSLOCATIONS
     60,                        // SK_TRANSMIGRATION
     80,                        // SK_DIVINATIONS
     160,                       // SK_FIRE_MAGIC
     80,                        // SK_ICE_MAGIC
     150,                       // SK_AIR_MAGIC
     150,                       // SK_EARTH_MAGIC
     80,                        // SK_POISON_MAGIC
     100,                       // SK_INVOCATIONS
     100,                       // SK_EVOCATIONS
     },



/* ******************************************************

// base draconian
    {
         90,               // SK_FIGHTING
        100,               // SK_SHORT_BLADES
        100,               // SK_LONG_SWORDS
        100,               // SK_UNUSED_1
        100,               // SK_AXES
        100,               // SK_MACES_FLAILS
        100,               // SK_POLEARMS
        100,               // SK_STAVES
        120,               // SK_SLINGS
        120,               // SK_BOWS
        120,               // SK_CROSSBOWS
        120,               // SK_DARTS
        120,               // SK_THROWING
        200,               // SK_ARMOUR
        120,               // SK_DODGING
        120,               // SK_STEALTH
        100,               // SK_STABBING
        100,               // SK_SHIELDS
        100,               // SK_TRAPS_DOORS
        100,               // SK_UNARMED_COMBAT
        100,               // undefined
        100,               // undefined
        100,               // undefined
        100,               // undefined
        100,               // undefined
        100,               // SK_SPELLCASTING
        100,               // SK_CONJURATIONS
        120,               // SK_ENCHANTMENTS
        100,               // SK_SUMMONINGS
        100,               // SK_NECROMANCY
        100,               // SK_TRANSLOCATIONS
        100,               // SK_TRANSMIGRATION
        100,               // SK_DIVINATIONS
        100,               // SK_FIRE_MAGIC
        100,               // SK_ICE_MAGIC
        100,               // SK_AIR_MAGIC
        100,               // SK_EARTH_MAGIC
        100,               // SK_POISON_MAGIC
        100,               // SK_INVOCATIONS
        100,               // SK_EVOCATIONS
    },

****************************************************** */

};




/* *************************************************************

// these were unimplemented "level titles" for two classes {dlb}

JOB_PRIEST
   "Preacher";
   "Priest";
   "Evangelist";
   "Pontifex";

JOB_PALADIN:
   "Holy Warrior";
   "Holy Crusader";
   "Paladin";
   "Scourge of Evil";

************************************************************* */

void show_skills(void)
{
    int i;
    int x;
    char lcount;

    const int num_lines = get_number_of_lines();

#ifdef DOS_TERM
    window(1, 1, 80, 25);
    char buffer[4600];

    gettext(1, 1, 80, 25, buffer);
#endif

    clrscr();

  reprint_stuff:
    lcount = 'a';

    gotoxy(1, 1);
    textcolor(LIGHTGREY);

#if DEBUG_DIAGNOSTICS
#ifdef JP
    cprintf( "You have %d points of unallocated experience (cost lvl %d; total %d)." EOL EOL,
#else
    cprintf( "You have %d points of unallocated experience (cost lvl %d; total %d)." EOL EOL,
#endif
             you.exp_available, you.skill_cost_level, you.total_skill_points );
#else
#ifdef JP
    cprintf("당신은 %d 점의 미분배 경험치를 가지고 있다." EOL EOL, you.exp_available );
#else
    cprintf(" You have %d points of unallocated experience." EOL EOL, you.exp_available );
#endif
#endif

    char scrln = 3, scrcol = 1;

    // Don't want the help line to appear too far down a big window.
    int bottom_line = ((num_lines > 30) ? 30 : num_lines);

    for (x = 0; x < NUM_SKILLS; x++)
    {
        /* spells in second column */
        if ((x == SK_SPELLCASTING && scrcol != 40) || scrln > bottom_line - 3)
        {
            scrln = 3;
            scrcol = 40;
        }

        gotoxy(scrcol, scrln);

#if DEBUG_DIAGNOSTICS
        // In diagnostic mode we show skills at 0, but only real skills
        if (x != SK_UNUSED_1 && (x <= SK_UNARMED_COMBAT || x >= SK_SPELLCASTING))
#else
        if (you.skills[x] > 0)
#endif
        {
            if (you.practise_skill[x] == 0 || you.skills[x] == 0)
                textcolor(DARKGREY);
            else
                textcolor(LIGHTGREY);

            if (you.skills[x] == 27)
                textcolor(YELLOW);

#if DEBUG_DIAGNOSTICS
            if (you.skills[x] == 0)
                putch(' ');
            else
            {
                putch(lcount);
                if (lcount == 'z')
                    lcount = 'A';
                else
                    lcount++;
            }
#else
            putch(lcount);
            if (lcount == 'z')
                lcount = 'A';
            else
                lcount++;
#endif

#ifdef JP
            cprintf( " %c %-14s 기술 %2d", \
                        (you.skills[x] == 0) ? ' ' : \
                        (you.practise_skill[x] == 0) ? '-' : \
                        '+', skills[x][0], you.skills[x] );
#else
            cprintf( " %c %-14s Skill %2d", (you.skills[x] == 0)         ? ' ' : (you.practise_skill[x] == 0) ? '-' : '+', skills[x][0], you.skills[x] );
#endif

            textcolor(BLUE);

#if DEBUG_DIAGNOSTICS
            cprintf( " %5d", you.skill_points[x] );
#endif

            if (you.skills[x] < 27)
            {
                const int needed = skill_exp_needed(you.skills[x] + 2);
                const int prev_needed = skill_exp_needed(you.skills[x] + 1);
                const int spec_abil = species_skills(x, you.species);

                cprintf( " (%d)",
                     (((needed * spec_abil) / 100 - you.skill_points[x]) * 10) /
                           (((needed - prev_needed) * spec_abil) / 100) );
            }

            scrln++;
        }

        /* Extra CR between classes of weapons and such things */
        if (x == SK_STAVES || x == SK_THROWING || x == SK_TRAPS_DOORS
            || x == SK_UNARMED_COMBAT || x == SK_POISON_MAGIC)
        {
            scrln++;
        }
    }

    // if any more skills added, must adapt letters to go into caps
    gotoxy(1, bottom_line);
    textcolor(LIGHTGREY);
#ifdef JP
    cprintf("기술에 대응하는 글자를 누르면, 기술을 배워나갈지 아닐지의 방침을 전환합니다.");
#else
    cprintf("Press the letter of a skill to choose whether you want to practise it.");
#endif

    char get_thing;

    get_thing = getch();

    if (get_thing == 0)
        getch();
    else
    {
        if ((get_thing >= 'a' && get_thing <= 'z')
            || (get_thing >= 'A' && get_thing <= 'Z'))
        {
            lcount = 'a';       // toggle skill practise

            for (i = 0; i < 50; i++)
            {
                if (you.skills[i] == 0)
                    continue;

                if (get_thing == lcount)
                {
                    you.practise_skill[i] = (you.practise_skill[i]) ? 0 : 1;
                    break;
                }

                if (lcount == 'z')
                    lcount = 'A';
                else
                    lcount++;
            }

            goto reprint_stuff;
        }
    }

#ifdef DOS_TERM
    puttext(1, 1, 80, 25, buffer);
#endif
    return;
}


const char *skill_name(unsigned char which_skill)
{
    return (skills[which_skill][0]);
}                               // end skill_name()


const char *skill_title( unsigned char best_skill, unsigned char skill_lev,
                         int species, int str, int dex, int god )
{
    unsigned char skill_rank;
    const char *tempstr = NULL;

    static char title_buff[80];

    // paranoia
    if (best_skill == SK_UNUSED_1
        || (best_skill > SK_UNARMED_COMBAT && best_skill < SK_SPELLCASTING)
        || best_skill >= NUM_SKILLS)
    {
#ifdef JP
        return ("모험자");
#else
        return ("Adventurer");
#endif
    }

    if (species == -1)
        species = you.species;

    if (str == -1)
        str = you.strength;

    if (dex == -1)
        dex = you.dex;

    if (god == -1)
        god = you.religion;

    // translate skill level into skill ranking {dlb}:
    // increment rank by one to "skip" skill name in array {dlb}:
    skill_rank = ((skill_lev <= 7)  ? 1 :
                  (skill_lev <= 14) ? 2 :
                  (skill_lev <= 20) ? 3 :
                  (skill_lev <= 26) ? 4
                   /* level 27 */   : 5);

    if (best_skill < NUM_SKILLS)
    {
        // Note that ghosts default to (dex == str) and god == no_god, due
        // to a current lack of that information... the god case is probably
        // suitable for most cases (TSO/Zin/Ely at the very least). -- bwr
        switch (best_skill)
        {
        case SK_UNARMED_COMBAT:
            tempstr = (dex >= str) ? martial_arts_titles[skill_rank]
                                   : skills[best_skill][skill_rank];

            break;

        case SK_INVOCATIONS:
            if (god == GOD_NO_GOD)
#ifdef JP
                tempstr = "무신앙자";
#else
                tempstr = "Godless";
#endif
            else
                tempstr = skills[best_skill][skill_rank];
            break;

        default:
            tempstr = skills[best_skill][skill_rank];
            break;
        }
    }

    const char *const ptr = strchr( tempstr, '%' );
    const bool species_found = (ptr != NULL);

    if (species_found)
    {
        // need species name
        snprintf( title_buff, sizeof(title_buff), tempstr,
                  species_name(species, 0, true,
                                (ptr == tempstr && best_skill != SK_NECROMANCY)) );
                  // The above code only capitalises start-of-string racenames
        tempstr = title_buff;
    }

    return ((tempstr == NULL) ? "Invalid Title" : tempstr);
}                               // end skill_title()

const char *player_title( void )
{
    const unsigned char best = best_skill( SK_FIGHTING, (NUM_SKILLS - 1), 99 );

    return (skill_title( best, you.skills[ best ] ));
}                               // end player_title()

unsigned char best_skill( unsigned char min_skill, unsigned char max_skill,
                          unsigned char excl_skill )
{
    unsigned char ret = SK_FIGHTING;
    unsigned int best_skill_level = 0;
    unsigned int best_position = 1000;

    for (int i = min_skill; i <= max_skill; i++)    // careful!!!
    {
        if (i == excl_skill
            || i == SK_UNUSED_1
            || (i > SK_UNARMED_COMBAT && i < SK_SPELLCASTING))
        {
            continue;
        }

        if (you.skills[i] > best_skill_level)
        {
            ret = i;
            best_skill_level = you.skills[i];
            best_position = you.skill_order[i];

        }
        else if (you.skills[i] == best_skill_level
                && you.skill_order[i] < best_position)
        {
            ret = i;
            best_position = you.skill_order[i];
        }
    }

    return (ret);
}                               // end best_skill()

// Calculate the skill_order array from scratch.
//
// The skill order array is used for breaking ties in best_skill.
// This is done by ranking each skill by the order in which it
// has attained its current level (the values are the number of
// skills at or above that level when the current skill reached it).
//
// In this way, the skill which has been at a level for the longest
// is judged to be the best skill (thus, nicknames are sticky)...
// other skills will have to attain the next level higher to be
// considered a better skill (thus, the first skill to reach level 27
// becomes the characters final nickname).
//
// As for other uses of best_skill:  this method is still appropriate
// in that there is no additional advantage anywhere else in the game
// for partial skill levels.  Besides, its probably best if the player
// isn't able to micromanage at that level.  -- bwr
void init_skill_order( void )
{
    for (int i = SK_FIGHTING; i < NUM_SKILLS; i++)
    {
        if (i == SK_UNUSED_1
            || (i > SK_UNARMED_COMBAT && i < SK_SPELLCASTING))
        {
            you.skill_order[i] = MAX_SKILL_ORDER;
            continue;
        }

        const int i_diff = species_skills( i, you.species );
        const unsigned int i_points = (you.skill_points[i] * 100) / i_diff;

        you.skill_order[i] = 0;

        for (int j = SK_FIGHTING; j < NUM_SKILLS; j++)
        {
            if (i == j
                || j == SK_UNUSED_1
                || (j > SK_UNARMED_COMBAT && j < SK_SPELLCASTING))
            {
                continue;
            }

            const int j_diff = species_skills( j, you.species );
            const unsigned int j_points = (you.skill_points[j] * 100) / j_diff;

            if (you.skills[j] == you.skills[i]
                && (j_points > i_points
                    || (j_points == i_points && j > i)))
            {
                you.skill_order[i]++;
            }
        }
    }
}

int calc_hp(void)
{
    int hitp;

    hitp = (you.base_hp - 5000) + (you.base_hp2 - 5000);
    hitp += (you.experience_level * you.skills[SK_FIGHTING]) / 5;

    // being berserk makes you resistant to damage. I don't know why.
    if (you.berserker)
    {
        hitp *= 15;
        hitp /= 10;
    }

    // some transformations give you extra hp
    switch (you.attribute[ATTR_TRANSFORMATION])
    {
    case TRAN_STATUE:
        hitp *= 15;
        hitp /= 10;
        break;
    case TRAN_ICE_BEAST:
        hitp *= 12;
        hitp /= 10;
        break;
    case TRAN_DRAGON:
        hitp *= 16;
        hitp /= 10;
        break;
    }

    // frail and robust mutations
    hitp *= (10 + you.mutation[MUT_ROBUST] - you.mutation[MUT_FRAIL]);
    hitp /= 10;

    you.hp_max = hitp;

    deflate_hp( you.hp_max, false );

    return (hitp);
}                               // end calc_hp()


int calc_mp(void)
{
    int enp;

    // base_magic_points2 accounts for species and magic potions
    enp = (you.base_magic_points2 - 5000);

    int spell_extra = (you.experience_level * you.skills[SK_SPELLCASTING]) / 4;
    int invoc_extra = (you.experience_level * you.skills[SK_INVOCATIONS]) / 6;
    int evoc_extra  = (you.experience_level * you.skills[SK_EVOCATIONS]) / 6;

    if (spell_extra > invoc_extra && spell_extra > evoc_extra)
        enp += spell_extra;
    else if (invoc_extra > evoc_extra)
        enp += invoc_extra;
    else
        enp += evoc_extra;

    you.max_magic_points = stepdown_value( enp, 9, 18, 45, 100 );

    // this is our "rotted" base (applied after scaling):
    you.max_magic_points += (you.base_magic_points - 5000);

    // Yes, we really do want this duplication... this is so the stepdown
    // doesn't truncate before we apply the rotted base.  We're doing this
    // the nice way. -- bwr
    if (you.max_magic_points > 50)
        you.max_magic_points = 50;

    // now applied after scaling so that power items are more useful -- bwr
    you.max_magic_points += player_magical_power();

    if (you.max_magic_points > 50)
        you.max_magic_points = 50 + ((you.max_magic_points - 50) / 2);

    if (you.max_magic_points < 0)
        you.max_magic_points = 0;

    if (you.magic_points > you.max_magic_points)
        you.magic_points = you.max_magic_points;

    you.redraw_magic_points = 1;

    return (you.max_magic_points);
}                               // end calc_mp()


unsigned int skill_exp_needed(int lev)
{
    lev--;
    switch (lev)
    {
    case 0:
        return 0;               // old:   0
    case 1:
        return 200;             // old:  20
    case 2:
        return 300;             // old:  30
    case 3:
        return 500;             // old:  50
    case 4:
        return 750;             // old:  75
    case 5:
        return 1050;            // old: 105
    case 6:
        return 1350;            // old: 145
    case 7:
        return 1700;            // old: 200
    case 8:
        return 2100;            // old: 275
    case 9:
        return 2550;            // old: 355
    case 10:
        return 3150;            // old: 440
    case 11:
        return 3750;            // old: 560
    case 12:
        return 4400;            // old: 680
    case 13:
        return 5250;            // old: 850
    default:
        return 6200 + 1800 * (lev - 14);
        // old: 1100 + 300 * (lev - 14)
        // older: 1200 * (lev - 11) + ((lev - 11) * (lev - 11));// * (lev - 11))
    }

    return 0;
}


int species_skills(char skill, char species)
{
    // Spellcasting is more expensive, invocations and evocations are cheaper
    if (skill == SK_SPELLCASTING)
        return (spec_skills[species - 1][skill] * 130) / 100;
    else if (skill == SK_INVOCATIONS || skill == SK_EVOCATIONS)
        return (spec_skills[species - 1][skill] * 75) / 100;
    else
        return (spec_skills[species - 1][skill]);
}                               // end species_skills()

// new: inform player if they need more throwing skill (GDL)
void wield_warning(bool newWeapon)
{
    // hold weapon name
    char wepstr[ITEMNAME_SIZE];
    char wepstr2[ITEMNAME_SIZE];

    // early out - no weapon
    if (you.equip[EQ_WEAPON] == -1)
         return;

    if (newWeapon)
#ifdef JP
        strcpy(wepstr, "이 ");
#else
        strcpy(wepstr, "this ");
#endif
    else
#ifdef JP
        strcpy(wepstr, "당신의 ");
#else
        strcpy(wepstr, "your ");
#endif

    int wepType  = you.inv[you.equip[EQ_WEAPON]].sub_type;

    // early out - don't warn for non-weapons
    if (you.inv[you.equip[EQ_WEAPON]].base_type != OBJ_WEAPONS)
        return;

    // put the standard wep name in.
    standard_name_weap(wepType, wepstr2);
    strcat(wepstr, wepstr2);

    // only warn about str/dex for non-launcher weapons
    if (!launches_things(wepType))
    {
#ifdef USE_NEW_COMBAT_STATS
        const int stat_bonus = effective_stat_bonus();

        if (stat_bonus <= -4)
        {
            if (you.strength < you.dex)
            {
                if (you.strength < 11)
#ifdef JP
                    snprintf( info, INFO_SIZE, "당신은 %s을(를) 다루는 것이 %s 곤란하다.",
                        wepstr, (you.strength < 7)?"":"다소");
#else
                    snprintf( info, INFO_SIZE, "You have %strouble swinging %s.",
                        (you.strength < 7)?"":"a little ", wepstr);
#endif
                else
#ifdef JP
                    snprintf( info, INFO_SIZE, "당신에게 좀 더 힘이 있다면 "
                        "%s을(를) 더 능숙하게 다룰 수 있을 것이다.", wepstr);
#else
                    snprintf( info, INFO_SIZE, "You'd be more effective with "
                        "%s if you were stronger.", wepstr);
#endif
            }
            else
            {
                if (you.dex < 11)
                {
#ifdef JP
                    snprintf( info, INFO_SIZE, "손에 들고 있는 %s은(는) %s 다루기 힘들다.",
                              wepstr, (you.dex < 7) ? "상당히" : "다소" );
#else
                    snprintf( info, INFO_SIZE, "Wielding %s is %s awkward.",
                              wepstr, (you.dex < 7) ? "fairly" : "a little" );
#endif
                }
                else
                {
#ifdef JP
                    snprintf( info, INFO_SIZE, "당신에게 좀 더 민첩함이 있다면 "
                        "%s을(를) 더 능숙하게 다룰 수 있을 것이다.", wepstr );
#else
                    snprintf( info, INFO_SIZE, "You'd be more effective with "
                        "%s if you were nimbler.", wepstr );
#endif
                }
            }

            mpr( info, MSGCH_WARN );
        }
#endif
        return;
    }

    // must be a launcher
    int effSkill = you.skills[SK_THROWING] * 2 + 1;
    int shoot_skill = 0;

    switch (wepType)
    {
        case WPN_SLING:
            shoot_skill = you.skills[SK_SLINGS];
            break;
        case WPN_BOW:
            shoot_skill = you.skills[SK_BOWS];
            break;
        case WPN_CROSSBOW:
        case WPN_HAND_CROSSBOW:
            shoot_skill = you.skills[SK_CROSSBOWS];
            break;
        case WPN_BLOWGUN:
            shoot_skill = you.skills[SK_DARTS];
            break;
        default:
            shoot_skill = 0;
            break;
    }

    if (shoot_skill > effSkill)
    {
#ifdef JP
        strcpy( info, "당신은 투척 기술이 너무 낮기 때문에, ");
        mpr( info, MSGCH_WARN );
        strcpy( info, wepstr );
        strcat( info, "을(를) 능숙하게 다룰 수 없다." );
#else
        strcpy( info, "Your low throwing skill limits the effectiveness of ");
        strcat( info, wepstr );
#endif
        mpr( info, MSGCH_WARN );
    }
}
