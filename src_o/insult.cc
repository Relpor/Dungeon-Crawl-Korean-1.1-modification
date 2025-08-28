// insult generator
// Josh Fishman (c) 2001, All Rights Reserved
// This file is released under the GNU GPL, but special permission is granted
// to link with Linley Henzel's Dungeon Crawl (or Crawl) without change to
// Crawl's license.
//
// The goal of this stuff is catachronistic feel.

#include "AppHdr.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "externs.h"
#include "insult.h"
#include "mon-util.h"
#include "stuff.h"

static const char* insults1(void);
static const char* insults2(void);
static const char* insults3(void);
static const char* run_away(void);
static const char* give_up(void);
static const char* meal(void);
static const char* whilst_thou_can(void);
static const char* important_body_part(void);
static const char* important_spiritual_part(void);

static void init_cap(char *);

void init_cap(char * str)
{
    if (str != NULL)
        str[0] = toupper( str[0] );
}

void imp_taunt( struct monsters *mons )
{
    char buff[80];
    const char *mon_name = ptr_monam( mons, DESC_CAP_THE );

    snprintf( buff, sizeof(buff), 
#ifdef JP 
              "%s, %s 녀석아!", 
#else
              "%s, thou %s!", 
#endif
              random2(7) ? run_away() : give_up(),
              generic_insult() );

    init_cap( buff );

#ifdef JP 
    // XXX: Not pretty, but stops truncation...
    if (strlen( mon_name ) + 11 + strlen( buff ) >= 76)
    {
        snprintf( info, INFO_SIZE, "%s은(는) 외쳤다 : ", mon_name );
        mpr( info, MSGCH_TALK );
        snprintf( info, INFO_SIZE, "\"%s\"", buff );
        mpr( info, MSGCH_TALK );
#else
    // XXX: Not pretty, but stops truncation...
    if (strlen( mon_name ) + 11 + strlen( buff ) >= 79)
    {
        snprintf( info, INFO_SIZE, "%s shouts:", mon_name );
        mpr( info, MSGCH_TALK );
        mpr( buff, MSGCH_TALK );
#endif
    }
    else
    {
#ifdef JP 
        snprintf( info, INFO_SIZE, "%s은(는) 외쳤다. \"%s\"", mon_name, buff );
#else
        snprintf( info, INFO_SIZE, "%s shouts, \"%s\"", mon_name, buff );
#endif
        mpr( info, MSGCH_TALK );
    }
}

void demon_taunt( struct monsters *mons )
{
    static const char * sound_list[] = 
    {
#ifdef JP 
        "말했다",         // actually S_SILENT
        "외쳤다", 
        "고함쳤다", 
        "소리쳤다" 
        "아우성쳤다", 
        "쨰지는 소리를 질렀다", 
        "짖었다", 
        "날카롭게 외쳤다", 
        "중얼거렸다", 
        "신음소리를 내질렀다", 
        "처량한 소리로 말했다", 
        "쉰 소리로 말했다", 
        "궁시렁 거렸다", 
#else
        "says",         // actually S_SILENT
        "shouts", 
        "barks", 
        "shouts", 
        "roars", 
        "screams", 
        "bellows", 
        "screeches", 
        "buzzes", 
        "moans", 
        "whines", 
        "croaks", 
        "growls", 
#endif
    };
  
    char buff[80];
    const char *mon_name = ptr_monam( mons, DESC_CAP_THE );
    const char *voice = sound_list[ mons_shouts(mons->type) ];

    if (coinflip())
    {
        snprintf( buff, sizeof(buff), 
#ifdef JP   // 테스트시 확인바람 (감자) 
                 "%s, %s 녀석아!",
#else
                 "%s, thou %s!",
#endif
                 random2(3) ? give_up() : run_away(),
                 generic_insult() );
    }
    else
    {
        switch( random2( 4 ) ) 
        {
        case 0:
            snprintf( buff, sizeof(buff), 
#ifdef JP 
                      "너의 %s은(는) 내 %s으로 만들겠다!",
#else
                      "Thy %s shall be my %s!",
#endif
                      random2(4) ? important_body_part() 
                                 : important_spiritual_part(), meal() );
            break;
        case 1:
            snprintf( buff, sizeof(buff), 
#ifdef JP 
                      "%s, 먹음직스러운 %s(이)로군!", give_up(), meal() );
#else
                      "%s, thou tasty %s!", give_up(), meal() );
#endif
            break;
        case 2:
            snprintf( buff, sizeof(buff), 
#ifdef JP 
                      "%s, %s!", run_away(), whilst_thou_can() );
#else
                      "%s %s!", run_away(), whilst_thou_can() );
#endif
            break;
        case 3:
            snprintf( buff, sizeof(buff), 
#ifdef JP 
                      "너의 %s을(를) %s%s!",
                      random2(4) ? important_body_part() 
                                 : important_spiritual_part() ,
                      coinflip() ? "진수성찬으로 알고" : "게걸스럽게",
                      coinflip() ? "먹어주겠다" : "먹어치우겠다" );
#else
                      "I %s %s thy %s!",
                      coinflip() ? "will" : "shall",
                      coinflip() ? "feast upon" : "devour",
                      random2(4) ? important_body_part() 
                                 : important_spiritual_part() );
#endif
            break;
        default:
#ifdef JP 
            snprintf( buff, sizeof(buff), "이놈, %s 녀석아!", generic_insult() );
#else
            snprintf( buff, sizeof(buff), "Thou %s!", generic_insult() );
#endif
            break;
        }
    }

    init_cap( buff );

#ifdef JP 
    // XXX: Not pretty, but stops truncation...
    if (strlen(mon_name) + strlen(voice) + strlen(buff) + 5 >= 76)
    {
        snprintf( info, INFO_SIZE, "%s은(는) %s:", mon_name, voice );
#else
    // XXX: Not pretty, but stops truncation...
    if (strlen(mon_name) + strlen(voice) + strlen(buff) + 5 >= 79)
    {
        snprintf( info, INFO_SIZE, "%s %s:", mon_name, voice );
#endif
        mpr( info, MSGCH_TALK );

        mpr( buff, MSGCH_TALK );
    }
    else
    {
#ifdef JP 
        snprintf( info, INFO_SIZE, "%s은(는) %s, \"%s\"", mon_name, voice, buff );
#else
        snprintf( info, INFO_SIZE, "%s %s, \"%s\"", mon_name, voice, buff );
#endif
        mpr( info, MSGCH_TALK );
    }
}

const char * generic_insult(void)
{
    static char buffer[80]; //FIXME: use string objects or whatnot

    strcpy(buffer, insults1());
    strcat(buffer, "");
    strcat(buffer, insults2());
    strcat(buffer, "");
    strcat(buffer, insults3());

    return (buffer);
}

static const char * important_body_part(void)
{
    static const char * part_list[] = {
#ifdef JP 
        "머리",
        "뇌",
        "심장",
        "내장",
        "눈",
        "폐",
        "간",
        "숨통",
        "목",
        "두개골",
        "등뼈",
#else
        "head",
        "brain",
        "heart",
        "viscera",
        "eyes",
        "lungs",
        "liver",
        "throat",
        "neck",
        "skull",
        "spine",
#endif
    };
  
    return (part_list[random2(sizeof(part_list) / sizeof(char *))]);
}

static const char * important_spiritual_part(void)
{
    static const char * part_list[] = {
#ifdef JP 
        "혼",
        "영혼",
        "마음의 빛",
        "희망",
        "신앙",
        "기개",
        "용기",
        "마음",
        "정신력",
        "근성",
        "생명력",
#else
        "soul",
        "spirit",
        "inner light",
        "hope",
        "faith",
        "will",
        "heart",
        "mind",
        "sanity",
        "fortitude",
        "life force",
#endif
    };

    return (part_list[random2(sizeof(part_list) / sizeof(char *))]);
}

static const char * meal(void)
{
    static const char * meal_list[] = {
#ifdef JP 
        //다응어가 쓰이는 것은 다른말로 치환
        "밥",
        "아침밥",
        "점심밥",
        "새참",
        "저녁밥",
        "맛있는 음식",
        "간식",
        "식량",
        "경양식",
        "다과",
        "도시락",
        "야식",
        "반찬",
        "간식",
        "한입",
#else
        "meal",
        "breakfast",
        "lunch",
        "dinner",
        "supper",
        "repast",
        "snack",
        "victuals",
        "refection",
        "junket",
        "luncheon",
        "snackling",
        "curdle",
        "snacklet",
        "mouthful",
#endif
    };

    return (meal_list[random2(sizeof(meal_list) / sizeof(char *))]);
}

static const char * run_away(void)
{
    static const char * run_away_list[] = {
#ifdef JP
        //의미가 연결될 것 같은 형태로 의역
        "항복하라",
        "없어져라",
        "떠나라",
        "도망치는게 좋을 것이다",
        "물러나라",
        "날아서 도망쳐라",
        "바보 같은 놈, 꺼져라",
        "무례한 놈, 꺼져라",
        "다시는 돌아오지 마라",
        "죽어버려라",
        "네이놈 썩 꺼져라",
        "네이놈 썩 떠나라",
        "꽁무니를 빼고 도망치는게 좋을 것이다",
        "떠나라",
        "네놈이 온 곳으로 돌아가라",
        "빨리 사라져라!",
        "꺼져라",
        "죽어라",
        "기어서 내눈앞에서 사라져라",
        "기어서 돌아가라",
        "기어서 꺼져라",
        "기어서 돌아가라",
        "냉큼 되돌아가라",
        "냉큼 사라지지 못할까",
        "냉큼 없어져라",
        "당황해 도망치는 것이 좋을 것이다",
        "살금살금 도망치는 것이 좋을 것이다",
#else
        "give up",
        "quit",
        "run away",
        "escape",
        "flee",
        "fly",
        "take thy face hence",
        "remove thy stench",
        "go and return not",
        "get thee hence",
        "back with thee",
        "away with thee",
        "turn tail",
        "leave",
        "return whence thou came",
        "begone",
        "get thee gone",
        "get thee hence",
        "slither away",
        "slither home",
        "slither hence",
        "crawl home",
        "scamper home",
        "scamper hence",
        "scamper away",
        "bolt",
        "decamp",
#endif
    };

    return (run_away_list[random2(sizeof(run_away_list) / sizeof(char *))]);
}

static const char * give_up(void)
{
    static const char * give_up_list[] = {
#ifdef JP 
        "단념해라",
        "항복해라",
        "멈추어라",
        "항복해라",
        "무릎 꿇어라",
        "자비를 청해라",
        "절망해라",
        "복종해라",
        "굴복 해라",
        "무서워 해라",
        "네놈의 잘못을 인정해라",
        "네놈의 좌절을 인정해라",
        "네놈의 운명을 인정해라",
        "항복해",
        "복종해",
        "네놈의 실패를 알고 수용해라",
        "네놈의 좌절을 알고 수용해라",
        "네놈의 운명을 알고 수용해라",
        "저항을 그만두어라",
        "떨어라",
        "희망을 버려라",
        "패배를 곰씹어라",
        "떨며 꿀어라",
        "그 몸을 버려라",
        "희망을 버려라",
        "네놈의 진혼가를 들어라",
        "네놈의 송가를 들어라",
        "패배를 인정해라",
        "괴롭게 발버둥 쳐라",
#else
        "give up",
        "give in",
        "quit",
        "surrender",
        "kneel",
        "beg for mercy",
        "despair",
        "submit",
        "succumb",
        "quail",
        "embrace thy failure",
        "embrace thy fall",
        "embrace thy doom",
        "embrace thy dedition",
        "embrace submission",
        "accept thy failure",
        "accept thy fall",
        "accept thy doom",
        "capitulate",
        "tremble",
        "relinquish hope",
        "taste defeat",
        "despond",
        "disclaim thyself",
        "abandon hope",
        "face thy requiem",
        "face thy fugue",
        "admit defeat",
        "flounder",
#endif
    };

    return (give_up_list[random2(sizeof(give_up_list) / sizeof(char *))]);
}

static const char * whilst_thou_can(void)
{
    static const char * threat_list[] = {
#ifdef JP   // 테스트시 확인바람 (감자) 
        "네놈이 그럴 수 있는 동안",
        "네놈이 할 수 있는 동안",
        "네놈이 그렇게 할 수 있는 동안",
        "만약 네놈에게 지혜의",
        "네놈의 운이 계속되는 동안",
        "악운이 너를 차장내기 전에",
        "죽음이 너를 찾아내기 전에",
        "네놈이 만족스러운 동안",
        "네놈에게 생명이 있는 동안", //jmf: hmm. screen vs. this for undead?
#else
        "whilst thou can",
        "whilst thou may",
        "whilst thou are able",
        "if wit thou hast",
        "whilst thy luck holds",
        "before doom catcheth thee",
        "lest death find thee",
        "whilst thou art whole",
        "whilst life thou hast", //jmf: hmm. screen vs. this for undead?
#endif
    };

    return (threat_list[random2(sizeof(threat_list) / sizeof(char *))]);
}

static const char * insults1(void)
{
    static const char * insults1_list[] = {
#ifdef JP 
//번역할 수 있는 것은 번역한다.그렇지 않은 것은 그런 것 같은 말을 찾고 쓴다.
        "서투르고 ",             // artless
        "눈치없고 ",               // baffled
        "비열하고 ",               // bawdy
        "끈적거리고 ",           // beslubbering
        "무익하고 ",           // bootless
        "거만하고 ",               // bumbling
        "위선적이고 ",           // canting
        "버릇없고 ",             // churlish
        "응석부리고 ",           // cockered
        "멍청하고 ",             // clouted
        "무기력하고 ",         // craven
        "천하고 ",               // currish
        "안색이 나쁜고 ",             // dankish
        "무시당하고 ",         // dissembling
        "지루하고 ",               // droning
        "무책임하고 ",             // ducking
        "변태스럽고 ",               // errant
        "비굴하고 ",               // fawning
        "무기력하고 ",             // feckless
        "겁쟁이고 ",             // feeble
        "거짓말쟁이고 ",             // fobbing
        "질투만 하고 ",             // foppish
        "성격이 비뚤어지고 ",       // froward
        "얄팍하고 ",           // frothy
        "끈질기고 ",             // fulsome
        "들뜨고 ",         // gleeking
        "호색하고 ",               // goatish
        "느슨해지고 ",             // gorbellied
        "때 찌들고 ",             // grime-gilt
        "무섭고 ",           // horrid
        "밉고 ",             // hateful
        "주제넘게 참견하고 ",         // impertinent
        "병옮기고 ",           // infectious
        "거슬리고 ",             // jarring
        "고집만 앞서고 ",           // loggerheaded
        "굼뱅이고 ",             // lumpish
        "말더듬이고 ",             // mammering
        "망쳐지고 ",           // mangled
        "울먹거리고 ",         // mewling
        "불쾌하고 ",           // odious
        "얄입고 ",             // paunchy
        "겁장이고 ",               // pribbling
        "화나고 ",             // puking
        "하잘것 없고 ",             // puny
        "움츠리고 ",         // qualling
        "떨고 ",         // quaking
        "냄새나고 ",         // rank
        "뚜쟁이고 ",           // pandering
        "본성을 숨기고 ",           // pecksniffian
        "깃털 뽑히고 ",     // plume-plucked
        "속에 든게 없고 ",             // pottle-deep
        "매독걸리고 ",           // pox-marked
        "주정꾼이고 ",           // reeling-ripe
        "교양없고 ",             // rough-hewn
        "질투나고 ",             // simpering
        "말랑말랑하고 ",           // spongy
        "퉁명스럽고 ",           // surly
        "비틀비틀거리고 ",           // tottering
        "뒤틀리고 ",           // twisted
        "아첨하고 ",           // unctious
        "미치고 ",           // unhinged
        "목줄이 풀리고 ",         // unmuzzled
        "허세부리고 ",           // vain
        "입이 더럽고 ",           // venomed
        "악랄하고 ",           // villainous
        "비뚤어지고 ",         // warped
        "멋대로이고 ",           // wayward
        "빈약하고 ",               // weedy
        "쓸모없고 ",             // worthless
        "경박하고 ",             // yeasty
#else
        "artless",
        "baffled",
        "bawdy",
        "beslubbering",
        "bootless",
        "bumbling",
        "canting",
        "churlish",
        "cockered",
        "clouted",
        "craven",
        "currish",
        "dankish",
        "dissembling",
        "droning",
        "ducking",
        "errant",
        "fawning",
        "feckless",
        "feeble",
        "fobbing",
        "foppish",
        "froward",
        "frothy",
        "fulsome",
        "gleeking",
        "goatish",
        "gorbellied",
        "grime-gilt",
        "horrid",
        "hateful",
        "impertinent",
        "infectious",
        "jarring",
        "loggerheaded",
        "lumpish",
        "mammering",
        "mangled",
        "mewling",
        "odious",
        "paunchy",
        "pribbling",
        "puking",
        "puny",
        "qualling",
        "quaking",
        "rank",
        "pandering",
        "pecksniffian",
        "plume-plucked", 
        "pottle-deep",
        "pox-marked",
        "reeling-ripe",
        "rough-hewn",
        "simpering",
        "spongy",
        "surly",
        "tottering",
        "twisted",
        "unctious", 
        "unhinged",
        "unmuzzled",
        "vain",
        "venomed",
        "villainous",
        "warped",
        "wayward",
        "weedy",
        "worthless",
        "yeasty",
#endif
    };

    return (insults1_list[random2(sizeof(insults1_list) / sizeof(char*))]);
}

static const char * insults2(void)
{
    static const char * insults2_list[] = {
#ifdef JP 
//번역할 수 있는 것은 번역한다.그렇지 않은 것은 그런 것 같은 말을 찾고 쓴다.
        "신변잡기스런 ",         // base-court
        "새를 쫓는 ",             // bat-fowling
        "아둔한 ",               // beef-witted
        "치매의 ",               // beetle-headed
        "성격이 급한 ",             // boil-brained
        "포악한 ",           // clapper-clawed
        "지능이 낮은 ",               // clay-brained
        "난잡한 ",             // common-kissing
        "나쁜쪽으로 머리가 좋은 ",       // crook-pated
        "우울증이 있는 ",               // dismal-dreaming
        "천한 출신의 ",         // ditch-delivered
        "난시의 ",               // dizzy-eyed
        "냉혈한 ",             // doghearted
        "불쾌한 ",         // dread-bolted
        "짜증나는 ",           // earth-vexing
        "엘프피부의 ",         // elf-skinned
        "뚱뚱한 ",             // fat-kidneyed
        "끈적한 ",             // fen-sucked
        "입술을 늘어뜰인 ",             // flap-mouthed
        "좀스러운 ",           // fly-bitten
        "타락한 ",             // folly-fallen
        "바보스러운 ",           // fool-born
        "이중 턱인 ",           // full-gorged
        "요절복통인 ",           // guts-griping
        "얼굴이 반쪽인 ",           // half-faced
        "속단을 잘하는 ",           // hasty-witted
        "비천한 ",               // hedge-born
        "미움받는 ",         // hell-hated
        "게으른 ",       // idle-headed
        "잘못 태어난 ",             // ill-breeding
        "교육을 못 받은 ",           // ill-nurtured
        "추한 ",           // kobold-kissing
        "정신착란의 ",           // knotty-pated
        "절뚝거리는 ",             // limp-willed
        "소심한 ",             // milk-livered
        "미친 ",             // moon-mazed
        "익살스런 ",               // motley-minded
        "잘 우는 ",             // onion-eyed
        "이단의 ",           // miscreant
        "껄렁한 ",             // roguish
        "곰팡이투성이의 ",         // moldwarp
        "발정한 ",             // ruttish
        "입이 싼 ",           // mumble-news
        "건방진 ",           // saucy
        "날치기의 ",         // nut-hook
        "우울한 ",             // spleeny
        "애송이의 ",             // pigeon-egg
        "버릇없이 자란 ",             // rude-growing
        "더러운 ",               // rump-fed
        "딱딱한 ",             // shard-borne
        "도둑의 ",             // sheep-biting
        "못생긴 ",             // sow-suckled
        "가축의 ",               // spur-galled
        "띠룩띠룩 살이찐 ",             // swag-bellied
        "느릿느릿한 ",               // tardy-gaited
        "근질거리는 ",             // tickle-brained
        "추한 ",         // toad-spotted
        "신랄한 ",           // toenail-biting
        "돼지코의 ",               // unchin-snouted
        "구두쇠의 ",               // weather-bitten
        "해충의 ",               // weevil-witted
#else
        "base-court",
        "bat-fowling",
        "beef-witted",
        "beetle-headed",
        "boil-brained",
        "clapper-clawed",
        "clay-brained",
        "common-kissing",
        "crook-pated",
        "dismal-dreaming",
        "ditch-delivered",
        "dizzy-eyed",
        "doghearted",
        "dread-bolted",
        "earth-vexing",
        "elf-skinned",
        "fat-kidneyed",
        "fen-sucked",
        "flap-mouthed",
        "fly-bitten",
        "folly-fallen",
        "fool-born",
        "full-gorged",
        "guts-griping",
        "half-faced",
        "hasty-witted",
        "hedge-born",
        "hell-hated",
        "idle-headed",
        "ill-breeding",
        "ill-nurtured",
        "kobold-kissing",
        "knotty-pated",
        "limp-willed",
        "milk-livered",
        "moon-mazed",
        "motley-minded",
        "onion-eyed",
        "miscreant",
        "roguish",
        "moldwarp",
        "ruttish",
        "mumble-news",
        "saucy",
        "nut-hook",
        "spleeny",
        "pigeon-egg",
        "rude-growing",
        "rump-fed",
        "shard-borne",
        "sheep-biting",
        "sow-suckled",
        "spur-galled",
        "swag-bellied",
        "tardy-gaited",
        "tickle-brained",
        "toad-spotted",
        "toenail-biting",
        "unchin-snouted",
        "weather-bitten",
        "weevil-witted",
#endif
    };

    return (insults2_list[random2(sizeof(insults2_list) / sizeof(char*))]);
}

static const char * insults3(void)
{
    static const char * insults3_list[] = {
#ifdef JP 
//번역할 수 있는 것은 번역한다.그렇지 않은 것은 그런 것 같은 말을 찾고 쓴다
        "성숙한",               // apple-john
        "말괄량이",                 // baggage
        "소매치기",             // bandersnitch
        "편집광",                 // barnacle
        "거지",                 // beggar
        "허풍선이",               // bladder
        "수퇘지",               // boar-pig
        "버릇없는",               // bounder
        "도깨비",                 // bugbear
        "부랑자",                 // bum-bailey
        "사기꾼",                 // canker-blossom
        "시끄러운",             // clack-dish
        "조용한",             // clam
        "얼간이",             // clotpole
        "멋쟁이",               // coxcomb
        "음흉한",                   // codpiece
        "죽음의 징표 같은",             // death-token
        "덜떨어진",             // dewberry
        "얼간이",                 // dingleberry
        "들뜬",             // flap-bat
        "음탕한 여자 같은",                   // flax-wench
        "경박한",               // flirt-gill
        "아첨꾼",         // foot-licker
        "비관주의",               // fustilarian
        "응석쟁이",             // giglet
        "똘마니",                   // gnoll-tail
        "송사리 같은",                   // gudgeon
        "부랑아",                 // guttersnipe
        "말라깽이",             // haggard
        "욕심쟁이",             // harpy
        "고집쟁이",                   // hedge-pig
        "짐승 같은",               // horn-beast
        "좀도둑",                 // hugger-mugger
        "쑥맥",               // joithead
        "비열한",                 // lewdster
        "촌뜨기",                 // lout
        "비열한",                   // maggot-pie
        "해로운",                   // malt-worm
        "마멧 같은",               // mammet
        "빈약한",               // measle
        "거지",                 // mendicant
        "하찮은",               // minnow reeky
        "완고한",               // mule
        "똥거름 장수",                   // nightsoil
        "이름도 없는",                 // nobody
        "쓰레기",                   // nothing
        "천치",                 // pigeon-egg
        "돼지사료 같은",                 // pignut
        "곰보",             // pimple
        "사마귀가 있는",               // pustule
        "어중간한",               // puttock
        "호박 같은",               // pumpion
        "쥐약 같은",                   // ratsbane
        "시체청소부",             // scavenger
        "애송이",                   // scut
        "노예",               // serf
        "얼간이",                 // simpleton
        "식구",               // skainsmate
        "슬라임 몰드",         // slime mold
        "소매치기",                 // snaffler
        "뱀 허물 같은",           // snake-molt
        "매춘부",                   // strumpet
        "사기꾼",                   // surfacer
        "방랑자",                 // tinkerer
        "꼬마",               // tiddler
        "장난꾸러기",                 // urchin
        "악당",                   // varlet
        "하인",                   // vassal
        "사기꾼",               // vulture
        "쓸모없는",             // wastrel
        "아첨꾼",               // wagtail
        "희끄무리한",               // whey-face
        "징그러운",           // wormtrail
        "야크 똥 같은",               // yak-dropping
        "좀비 먹이 같은",           // zombie-fodder
#else
        "apple-john",
        "baggage",
        "bandersnitch",
        "barnacle",
        "beggar",
        "bladder",
        "boar-pig",
        "bounder",
        "bugbear",
        "bum-bailey",
        "canker-blossom",
        "clack-dish",
        "clam",
        "clotpole",
        "coxcomb",
        "codpiece",
        "death-token",
        "dewberry",
        "dingleberry",
        "flap-bat",
        "flax-wench",
        "flirt-gill",
        "foot-licker",
        "fustilarian",
        "giglet",
        "gnoll-tail",
        "gudgeon",
        "guttersnipe",
        "haggard",
        "harpy",
        "hedge-pig",
        "horn-beast",
        "hugger-mugger",
        "joithead",
        "lewdster",
        "lout",
        "maggot-pie",
        "malt-worm",
        "mammet",
        "measle",
        "mendicant",
        "minnow reeky",
        "mule",
        "nightsoil",
        "nobody",
        "nothing",
        "pigeon-egg",
        "pignut",
        "pimple",
        "pustule",
        "puttock",
        "pumpion",
        "ratsbane",
        "scavenger",
        "scut",
        "serf",
        "simpleton",
        "skainsmate",
        "slime mold",
        "snaffler",
        "snake-molt",
        "strumpet",
        "surfacer",
        "tinkerer",
        "tiddler",
        "urchin",
        "varlet",
        "vassal",
        "vulture",
        "wastrel",
        "wagtail",
        "whey-face",
        "wormtrail",
        "yak-dropping",
        "zombie-fodder",
#endif
    };

    return (insults3_list[random2(sizeof(insults3_list) / sizeof(char*))]);
}

// currently unused:
#if 0
const char * racial_insult(void)
{
    static const char * food3[] = {
        "snackling",
        "crunchlet",
        "half-meal",
        "supper-setting",
        "snacklet",
        "noshlet",
        "morsel",
        "mug-up",
        "bite-bait",
        "crunch-chow",
        "snack-pap",
        "grub",
    };

    static const char * elf1[] = {
        "weakly",
        "sickly",
        "frail",
        "delicate",
        "fragile",
        "brittle",
        "tender",
        "mooning",
        "painted",
        "lily-hearted",
        "dandy",
        "featherweight",
        "flimsy",
        "rootless",
        "spindly",
        "puny",
        "shaky",
        "prissy",
    };

    static const char * halfling3[] = {
        "half-pint",
        "footstool",
        "munchkin",
        "side-stool",
        "pudgelet",
        "groundling",
        "burrow-snipe",
        "hole-bolter",
        "low-roller",
        "runt",
        "peewee",
        "mimicus",
        "manikin",
        "hop-o-thumb",
        "knee-biter",
        "burrow-botch",
        "hole-pimple",
        "hovel-pustule",
    };

    static const char * spriggan3[] = {
        "rat-rider",
        "mouthfull",
        "quarter-pint",
        "nissette",
        "fizzle-flop",
        "spell-botch",
        "feeblet",
        "weakling",
        "pinchbeck-pixie",
        "ankle-biter",
        "bootstain",
        "nano-nebbish",
        "sopling",
        "shrunken violet",
        "sissy-prig",
        "pussyfoot",
        "creepsneak",
    };

    static const char * dwarf2[] = {
        "dirt-grubbing",
        "grit-sucking",
        "muck-plodding",
        "stone-broke",
        "pelf-dandling",
        "fault-botching",
        "gravel-groveling",
        "boodle-bothering",
        "cabbage-coddling",
        "rhino-raveling",
        "thigh-biting",
        "dirt-delving",
    };

    static const char * kenku2[] = {
        "hollow-boned",
        "feather-brained",
        "beak-witted",
        "hen-pecked",
        "lightweight",
        "frail-limbed",
        "bird-brained",
        "featherweight",
        "pigeon-toed",
        "crow-beaked",
        "magpie-eyed",
        "mallardish",
    };

    static const char * minotaur3[] = {
        "bull-brain",
        "cud-chewer",
        "calf-wit",
        "bovine",
        //"mooer", // of Venice
        "cow",
        "cattle",
        "meatloaf",
        "veal",
        "meatball",
        "rump-roast",
        "briscut",
        "cretin",
        "walking sirloin",
    };

    switch (you.species) 
    {
    default:
    break;
    }
}
#endif
