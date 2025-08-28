/*
 *  File:       monspeak.cc
 *  Summary:    Functions to handle speaking monsters
 *
 *  Change History (most recent first):
 *
 *      <1>    01/09/00        BWR     Created
 */

#include "AppHdr.h"
#include "monspeak.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "beam.h"
#include "debug.h"
#include "fight.h"
#include "insult.h"
#include "itemname.h"
#include "misc.h"
#include "monplace.h"
#include "monstuff.h"
#include "mon-util.h"
#include "mstuff2.h"
#include "player.h"
#include "spells2.h"
#include "spells4.h"
#include "stuff.h"
#include "view.h"

// returns true if something is said
bool mons_speaks(struct monsters *monster)
{
    int temp_rand;              // probability determination

    // This function is a little bit of a problem for the message channels
    // since some of the messages it generates are "fake" warning to
    // scare the player.  In order to accomidate this intent, we're
    // falsely categorizing various things in the function as spells and
    // danger warning... everything else just goes into the talk channel -- bwr
    int msg_type = MSGCH_TALK;

    const char *m_name = ptr_monam(monster, DESC_CAP_THE);
    strcpy(info, m_name);

    if (mons_has_ench(monster, ENCH_INVIS))
        return false;
    // invisible monster tries to remain unnoticed

    //mv: if it's also invisible, program never gets here
    if (silenced(monster->x, monster->y))
    {
        if (!one_chance_in(3))
            return false;       // while silenced, don't bother so often

        if (mons_has_ench(monster, ENCH_CONFUSION))
        {
            temp_rand = random2(10);
#ifdef JP
            strcat(info, (temp_rand <  4) ? "이(가) 갑자기 엉뚱한 몸짓을 한다." :
                         (temp_rand == 4) ? "은(는) 혼란스러워 하는 것 같다." :
                         (temp_rand == 5) ? "이(가) 사악하게 웃는다." :
                         (temp_rand == 6) ? "이(가) 행복하게 미소를 짓는다." :
                         (temp_rand == 7) ? "이(가) 엉엉 운다."
                             : "이(가) 무언가를 말하나 당신은 아무것도 듣지 못한다.");
#else
            strcat(info, (temp_rand <  4) ? " gestures wildly." :
                         (temp_rand == 4) ? " looks confused." :
                         (temp_rand == 5) ? " grins evilly." :
                         (temp_rand == 6) ? " smiles happily." :
                         (temp_rand == 7) ? " cries."
                             : " says something but you don't hear anything.");
#endif
        }
        else if (monster->behaviour == BEH_FLEE)
        {
            temp_rand = random2(10);
            strcat(info,
#ifdef JP
                     (temp_rand <  3) ? "이(가) 수상쩍게 주위를 두리번 거린다." :
                     (temp_rand == 3) ? "이(가) 소리지르듯이 입을 연다." :
                     (temp_rand == 4) ? "이(가) 주위를 둘러본다." :
                     (temp_rand == 5) ? "은(는) 무언가 주저하는듯 보인다." :
                     (temp_rand == 6) ? "은(는) 자신에게 닥친 상황에 대해 생각을 정리하고 있다."
                                      : "은(는) 무엇인가 말하기 시작할 것 같은 모습이다.");
#else
                     (temp_rand <  3) ? " glances furtively about." :
                     (temp_rand == 3) ? " opens its mouth, as if shouting." :
                     (temp_rand == 4) ? " looks around." :
                     (temp_rand == 5) ? " appears indecisive." :
                     (temp_rand == 6) ? " ponders the situation."
                                      : " seems to says something.");
#endif
        }
        // disregard charmed critters.. they're not too expressive
        else if (monster->attitude == ATT_FRIENDLY)
        {
            temp_rand = random2(10);
#ifdef JP
            strcat(info, (temp_rand <  3) ? "이(가) 당신에게 엄지를 세워보인다." :
                         (temp_rand == 3) ? "이(가) 당신을 바라본다." :
                         (temp_rand == 4) ? "이(가) 당신에게 손을 흔든다." :
                         (temp_rand == 5) ? "이(가) 즐거운 듯이 미소를 짓는다.":
                         (temp_rand == 6) ? "이(가) 당신에게 윙크를 한다."
                             : "이(가) 무엇인가 말했지만, 당신은 듣지 못한다.");
#else
            strcat(info, (temp_rand <  3) ? " gives you a thumbs up." :
                         (temp_rand == 3) ? " looks at you." :
                         (temp_rand == 4) ? " waves at you." :
                         (temp_rand == 5) ? " smiles happily.":
                         (temp_rand == 6) ? " winks at you."
                             : " says something you can't hear.");
#endif
        }
        else
        {
            temp_rand = random2(10);
#ifdef JP
            strcat(info, (temp_rand <  3) ? "이(가) 무엇인가 몸짓을 취한다." :
                         (temp_rand == 3) ? "이(가) 추잡한 짓을 한다." :
                         (temp_rand == 4) ? "이(가) 씨익 웃는다." :
                         (temp_rand == 5) ? "은(는) 화가 난 듯이 보인다." :
                         (temp_rand == 6) ? "은(는) 무언가를 듣고 있는 듯이 보인다."
                             : "이(가) 무엇인가를 말하지만, 당신은 아무것도 듣지 못한다.");
#else
            strcat(info, (temp_rand <  3) ? " gestures." :
                         (temp_rand == 3) ? " gestures obscenely." :
                         (temp_rand == 4) ? " grins." :
                         (temp_rand == 5) ? " looks angry." :
                         (temp_rand == 6) ? " seems to be listening."
                             : " says something but you don't hear anything.");
#endif
        }                       //end switch silenced monster's behaviour

        mpr(info, MSGCH_TALK);
        return true;
    }                           // end silenced monster

    // charmed monsters aren't too expressive
    if (mons_has_ench(monster, ENCH_CHARM))
        return false;

    if (mons_has_ench(monster, ENCH_CONFUSION))
    {
        if (mons_holiness( monster->type ) == MH_DEMONIC
            && monster->type != MONS_IMP)
        {
            return (false);
        }

        if (mons_friendly(monster))
        {
            switch (random2(18))        // speaks for friendly confused monsters
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 도움을 청했다.");
#else
                strcat(info, " prays for help.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 절규한다. \"도와줘!\"");
#else
                strcat(info, " screams, \"Help!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 외친다. \"자신을 억누를 수가 없어!\"");
#else
                strcat(info, " shouts, \"I'm losing control!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 외친다. \"도대체 무슨일이 일어나고 있는거야?\"");
#else
                strcat(info, " shouts, \"What's happening?\"");
#endif
                break;
            case 4:
            case 5:
#ifdef JP
                strcat(info, "은(는) 과장된 몸짓을 하고 있다.");
#else
                strcat(info, " gestures wildly.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 울고 있다.");
#else
                strcat(info, " cries.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 외친다. \"오예~!\"");
#else
                strcat(info, " shouts, \"Yeah!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "이(가) 노래를 부른다.");
#else
                strcat(info, " sings.");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "이(가) 미친듯이 웃는다.");
#else
                strcat(info, " laughs crazily.");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 상황에 대해 생각을 정리하고 있다.");
#else
                strcat(info, " ponders the situation.");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "이(가) 미치광이처럼 싱글거린다.");
#else
                strcat(info, " grins madly.");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "은(는) 매우 혼란스러워 보인다.");
#else
                strcat(info, " looks very confused.");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "이(가) 무엇인가 중얼거린다.");
#else
                strcat(info, " mumbles something.");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "이(가) 미친 사람처럼 킥킥대며 웃는다.");
#else
                strcat(info, " giggles crazily.");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "이(가) 절규한다. \"");
#else
                strcat(info, " screams, \"");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "! 도와줘!\"");
#else
                strcat(info, "! Help!\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "이(가) 절규한다. \"");
#else
                strcat(info, " screams, \"");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "! 무슨 일이 일어나고 있는거야?\"");
#else
                strcat(info, "! What's going on?\"");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "이(는) 말한다. \"");
#else
                strcat(info, " says, \"");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, ", 나는 그저 약간 혼란스러울 뿐이야.\"");
#else
                strcat(info, ", I'm little bit confused.\"");
#endif
                break;
            }
        }
        else
        {
            switch (random2(23))  // speaks for unfriendly confused monsters
            {
            case 0:
#ifdef JP
                strcat(info, "이(가) 외친다. \"이걸 나에게서 빼앗아 보시지!\"");
#else
                strcat(info, " yells, \"Get them off of me!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "이(가) 절규한다. \"반드시 너를 죽일테다!\"");
#else
                strcat(info, " screams, \"I will kill you anyway!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "이(가) 외친다. \"무슨 일이 일어나고 있는거야?\"");
#else
                strcat(info, " shouts, \"What's happening?\"");
#endif
                break;
            case 3:
            case 4:
            case 5:
#ifdef JP
                strcat(info, "이(가) 터무니없는 몸짓을 한다.");
#else
                strcat(info, " gestures wildly.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "이(가) 운다.");
#else
                strcat(info, " cries.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "이(가) 외친다. \"안돼!\"");
#else
                strcat(info, " shouts, \"NO!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "이(가) 외친다. \"그래!\"");
#else
                strcat(info, " shouts, \"YES!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "이(가) 미친 것처럼 웃는다.");
#else
                strcat(info, " laughs crazily.");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 상황에 대해 생각을 정리하고 있다.");
#else
                strcat(info, " ponders the situation.");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "이(가) 미치광이처럼 싱글거린다.");
#else
                strcat(info, " grins madly.");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "은(는) 매우 혼란스러워 보인다.");
#else
                strcat(info, " looks very confused.");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "이(가) 무엇인가 중얼거린다.");
#else
                strcat(info, " mumbles something.");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "이(가) 말한다. \"나는 그저 약간 혼란스러울 뿐이야.\"");
#else
                strcat(info, " says, \"I'm little bit confused.\"");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "은(는) 물었다. \"내가 어디에 있는거지?\"");
#else
                strcat(info, " asks, \"Where am I?\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "은(는) 떨고있다.");
#else
                strcat(info, " shakes.");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "은(는) 물었다. \"넌 누구냐?\"");
#else
                strcat(info, " asks, \"Who are you?\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "은(는) 물었다. \"이런 젠장, 우리 뭐하는거야? 음, 아아 알겠다...\"");
#else
                strcat(info, " asks, \"What the hell are we doing here? Mmm, I see...\"");
#endif
                break;
            case 19:
#ifdef JP
                strcat(info, "은(는) 울부짖었다. \"내 머리! 내 머리!!\"");
#else
                strcat(info, " cries, \"My head! MY HEAD!!!\"");
#endif
                break;
            case 20:
#ifdef JP
                strcat(info, "은(는) 말했다. \"세상이 빙글빙글 도네, 왜 이러지?\"");
#else
                strcat(info, " says, \"Why is everything spinning?\"");
#endif
                break;
            case 21:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"안돼! 이런 잡음은 참을 수 없어!\"");
#else
                strcat(info, " screams, \"NO! I can't bear up that noise!\"");
#endif
                break;
            case 22:
#ifdef JP
                strcat(info, "은(는) 눈을 감싸려 하고있다.");
#else
                strcat(info, " is trying to cover his eyes.");
#endif
                break;
            }
        }

    }
    else if (monster->behaviour == BEH_FLEE)
    {
        if (mons_holiness( monster->type ) == MH_DEMONIC
            && monster->type != MONS_IMP)
        {
            return (false);
        }

        if (mons_friendly(monster))
        {
            switch (random2(11))
            {
            case 0:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"기다려줘!\"", m_name,
                        coinflip() ? "외쳤다" : "아우성쳤다");
                strcat(info, "[");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"WAIT FOR ME!\"", m_name,
                        coinflip() ? "shouts" : "yells");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "], 도와주지 않을래?\"");
#else
                strcat(info, ", could you help me?\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"도와줘!\"");
#else
                strcat(info, " screams, \"Help!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"날 보호해!\"");
#else
                strcat(info, " shouts, \"Cover me!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"");
#else
                strcat(info, " screams, \"");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "! 도와줘!\"");
#else
                strcat(info, "! Help me!\"");
#endif
                break;
            case 4:
            case 5:
            case 6:
#ifdef JP
                strcat(info, "은(는) 어디엔가 몸을 숨기려했다.");
#else
                strcat(info, " tries to hide somewhere.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 도움을 요청했다.");
#else
                strcat(info, " prays for help.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 간절히 원하는 눈빛으로 당신을 보았다.");
#else
                strcat(info, " looks at you beseechingly.");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"나를 보호하라\"");
#else
                strcat(info, " shouts, \"Protect me!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 울부짖었다. \"친구를 버리지 마!\"");
#else
                strcat(info, " cries, \"Don't forget your friends!\"");
#endif
                break;
            }
        }
        else
        {
            switch (random2(20))    // speaks for unfriendly fleeing monsters
            {
            case 0:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"도와줘!\"", m_name,
                coinflip()? "아우성쳤다" : "울부짖었다");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Help!\"", m_name, coinflip()? "yells" : "wails");
#endif
                break;
            case 1:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"도와줘!\"", m_name,
                        coinflip() ? "울먹였다" : "부르짖었다"); break;
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Help!\"", m_name,
                        coinflip() ? "cries" : "screams"); break;
#endif
            case 2:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"사이좋게 지내는게 어때?\"",
                        m_name, coinflip() ? "비굴하게 말했다" : "애걸했다");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Why can't we all just get along?\"",
                        m_name, coinflip() ? "begs" : "pleads");
#endif
                break;
            case 3:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) 도망치려고 %s 멀리 갔다.", m_name,
                        coinflip() ? "상당히" : "아주");
#else
                snprintf( info, INFO_SIZE, "%s %s trips in trying to escape.", m_name,
                        coinflip() ? "nearly" : "almost");
#endif
                break;
            case 4:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"빌어먹을 운!\"", m_name,
                        coinflip() ? "중얼거렸다" : "투덜거렸다");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Of all the rotten luck!\"", m_name,
                        coinflip() ? "mutters" : "mumbles");
#endif
                break;
            case 5:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"어머나! 어머나!", m_name,
                        coinflip() ? "신음했다" : "탄식했다");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Oh dear! Oh dear!\"", m_name,
                        coinflip() ? "moans" : "wails");
#endif
            case 6:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"젠장, 실패했다!\"", m_name,
                        coinflip() ? "중얼거렸다" : "투덜거렸다");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Damn and blast!\"", m_name,
                        coinflip() ? "mutters" : "mumbles");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 도움을 기도했다.");
#else
                strcat(info, " prays for help.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"안돼! 더이성 그런일은 못해!\"");
#else
                strcat(info, " shouts, \"No! I'll never do that again!\"");
#endif
                break;
            case 9:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s", m_name,
                        coinflip() ? "애걸했다." : "울먹였다. \"제게 자비를!\"");
#else
                snprintf( info, INFO_SIZE, "%s %s", m_name,
                        coinflip() ? "begs for mercy." : "cries, \"Mercy!\"");
#endif
                break;
            case 10:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"%s\"", m_name,
                        coinflip() ? "울며 아우성쳤다" : "울부짖었다",
                        coinflip() ? "엄마아~" : "아빠~");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"%s!\"", m_name,
                        coinflip() ? "blubbers" : "cries",
                        coinflip() ? "Mommeee" : "Daddeee");
#endif
                break;
            case 11:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"죽이지는 말아주세요!\"", m_name,
                        coinflip() ? "애걸했다" : "호소했다");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Please don't kill me!\"", m_name,
                        coinflip() ? "begs" : "pleads");
#endif
                break;
            case 12:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"날 아프게 하지마세요!\"", m_name,
                        coinflip() ? "애걸했다" : "호소했다");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Please don't hurt me!\"", m_name,
                        coinflip() ? "begs" : "pleads");
#endif
                break;
            case 13:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) %s. \"제발, 나에겐 딸린 자식이 많아...\"",
                        m_name, coinflip() ? "애걸했다" : "호소했다");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Please, I have a lot of children...\"",
                        m_name, coinflip() ? "begs" : "pleads");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "은(는) 잃은 용기를 되찾으려고 발버둥치고 있다.");
#else
                strcat(info, " tries to recover lost courage.");
#endif
                break;
            case 15:
            case 16:
            case 17:
#ifdef JP
                strcat(info, "은(는) 포기했다.");
#else
                strcat(info, " gives up.");
#endif
                break;
            case 19:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) 완전히 %s 모습이다.", m_name,
                        coinflip() ? "공포에 질린" : "매우 약해진");
#else
                snprintf( info, INFO_SIZE, "%s looks really %s.", m_name,
                        coinflip() ? "scared stiff" : "rattled");
#endif
                break;
            }
        }
    }
    else if (mons_friendly(monster))
    {
        if (mons_holiness( monster->type ) == MH_DEMONIC
            && monster->type != MONS_IMP)
        {
            return (false);
        }

        // friendly imps are too common so they speak very very rarely
        if ((monster->type == MONS_IMP) && (random2(10)))
            return (false);

        switch (random2(18))
        {
        case 0:
#ifdef JP
            strcat(info, "은(는) 아우성쳤다. \"뛰어! 내가 지켜 주겠어!\"");
#else
            strcat(info, " yells, \"Run! I'll cover you!\"");
#endif
            break;
        case 1:
#ifdef JP
            strcat(info, "은(는) 외쳤다. \"죽어라, 괴물!\"");
#else
            strcat(info, " shouts, \"Die, monster!\"");
#endif
            break;
        case 2:
#ifdef JP
            strcat(info, "은(는) 말했다. \"친구가 있는건 좋군.\"");
#else
            strcat(info, " says, \"It's nice to have friends.\"");
#endif
            break;

        case 3:
#ifdef JP
            strcat(info, "은(는) 당신을 응시하고 있다.");
#else
            strcat(info, " looks at you.");
#endif
            break;
        case 4:
#ifdef JP
            strcat(info, "은(는) 당신에게 미소지었다.");
#else
            strcat(info, " smiles at you.");
#endif
            break;
        case 5:
#ifdef JP
            strcat(info, "은(는) 말했다. \"");
#else
            strcat(info, " says, \"");
#endif
            strcat(info, you.your_name);
#ifdef JP
            strcat(info, ", 너는 내 최고의 친구야.\"");
#else
            strcat(info, ", you are my only friend.\"");
#endif
            break;
        case 6:
#ifdef JP
            strcat(info, "은(는) 말했다. \"");
#else
            strcat(info, " says, \"");
#endif
            strcat(info, you.your_name);
#ifdef JP
            strcat(info, ", 난 네가 좋아.\"");
#else
            strcat(info, ", I like you.\"");
#endif
            break;

        case 7:
#ifdef JP
            strcat(info, "은(는) 당신에게 손을 흔들었다.");
#else
            strcat(info, " waves at you.");
#endif
            break;
        case 8:
#ifdef JP
            strcat(info, "은(는) 말했다. \"조심해!\"");
#else
            strcat(info, " says, \"Be careful!\"");
#endif
            break;
        case 9:
#ifdef JP
            strcat(info, "은(는) 말했다. \"걱정마, 내가 곁에 있을께.\"");
#else
            strcat(info, " says, \"Don't worry. I'm here with you.\"");
#endif
            break;
        case 10:
#ifdef JP
            strcat(info, "은(는) 행복한 미소를 띄우고있다.");
#else
            strcat(info, " smiles happily.");
#endif
            break;
        case 11:
#ifdef JP
            strcat(info, "은(는) 외쳤다. \"자비를 바라지마라! 전부 죽이겠다!\"");
#else
            strcat(info, " shouts, \"No mercy! Kill them all!");
#endif
            break;
        case 12:
#ifdef JP
            strcat(info, "은(는) 당신에게 윙크했다.");
#else
            strcat(info, " winks at you.");
#endif
            break;
        case 13:
#ifdef JP
            strcat(info, "은(는) 말했다. \"나와 너. 멋진걸.\"");
#else
            strcat(info, " says, \"Me and you. It sounds cool.\"");
#endif
            break;
        case 14:
#ifdef JP
            strcat(info, "은(는) 말했다. \"난 널 떠나지 않을꺼야.\"");
#else
            strcat(info, " says, \"I'll never leave you.\"");
#endif
            break;
        case 15:
#ifdef JP
            strcat(info, "은(는) 말했다. \"너를 위해서라면 죽을 수 도 있어.\"");
#else
            strcat(info, " says, \"I would die for you.\"");
#endif
            break;
        case 16:
#ifdef JP
            strcat(info, "은(는) 외쳤다. \"괴물을 조심해!\"");
#else
            strcat(info, " shouts, \"Beware of monsters!\"");
#endif
            break;
        case 17:
#ifdef JP
            strcat(info, "은(는) 우호적인 모습이다.");
#else
            strcat(info, " looks friendly.");
#endif
            break;
        }
    }
    else
    {
        switch (monster->type)
        {
        case MONS_TERENCE:  // fighter who likes to kill
            switch (random2(15))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"널 죽여주겠다!\"");
#else
                strcat(info, " screams, \"I'm going to kill you! \"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"이제 넌 죽었다.\"");
#else
                strcat(info, " shouts, \"Now you die.\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 말했다. \"명복을 빈다.\"");
#else
                strcat(info, " says, \"Rest in peace.\"");
#endif
                break;
            case 3:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) 외쳤다. \"%s!!!\"",
                  m_name, coinflip() ? "공격" : "죽어라");
#else
                snprintf( info, INFO_SIZE, "%s shouts, \"%s!!!\"",
                  m_name, coinflip() ? "ATTACK" : "DIE");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 말했다. \"어때, 즐거워?\"");
#else
                strcat(info, " says, \"How do you enjoy it?\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"죽을 준비나 해둬!\"");
#else
                strcat(info, " shouts, \"Get ready for death!\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 말했다. \"넌 기억의 뒷편으로 사라질 것야.\"");
#else
                strcat(info, " says, \"You are history.\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 말했다. \"빨리 죽고싶어? 아니면 천천히?\"");
#else
                strcat(info, " says, \"Do you want it fast or slow?.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 말했다. \"유언장은 써놓았나? 필요할텐데...\"");
#else
                strcat(info, " says, \"Did you write a testament? You should...\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 말했다. \"안녕이라 말할 시간이로군...\"");
#else
                strcat(info, " says, \"Time to say good-bye...\"");
#endif
                break;
            case 10:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) 말했다. \"%s 방어를 하려하지마.\"",
                        m_name, coinflip() ? "헛된" : "무의미한");
#else
                snprintf( info, INFO_SIZE, "%s says, \"Don't try to defend, it's %s.\"",
                        m_name, coinflip() ? "pointless" : "senseless");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "은(는) 이빨을 드러냈다.");
#else
                strcat(info, " bares his teeth.");
#endif
                break;
            case 12:
#ifdef JP
                snprintf( info, INFO_SIZE, "%s은(는) 말했다. \"전문가의 %s을 보여주겠어.\"",
                        m_name, coinflip() ? "기술" : "테크닉");
#else
                snprintf( info, INFO_SIZE, "%s says, \"I'll show you few %s.\"",
                        m_name, coinflip() ? "tricks" : "ploys.");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"난 너의 피를 원해.\"");
#else
                strcat(info, " screams, \"I want your blood.\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "은(는) 경멸의 눈빛으로 당신을 쳐다보았다.");
#else
                strcat(info, " looks scornfully at you.");
#endif
                break;
            }
            break;          // end Terence

        case MONS_EDMUND:   // mercenaries guarding dungeon
        case MONS_LOUISE:   //Louise,Franches궼룛맜뼹
        case MONS_FRANCES:
        case MONS_DUANE:
        case MONS_ADOLF:
            switch (random2(17))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"지금 바로 널 죽이겠다!\"");
#else
                strcat(info, " screams, \"I'm going to kill you! Now!\"");
#endif
                break;
            case 1:
                strcat(info,
#ifdef JP
                       "은(는) 외쳤다. \"즉시 되돌려라. 아니면 죽이겠다!\"");
#else
                       " shouts, \"Return immediately or I'll kill you!\"");
#endif
                break;
            case 2:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"너의 여행도 여기서 끝이다.\"");
#else
                       " says, \"Now you've reached the end of your journey!\"");
#endif
                break;
            case 3:
                strcat(info,
#ifdef JP
                       "은(는) 외쳤다. \"단한번의 실수라도 한다면 널 죽이겠다!\"");
#else
                       " screams, \"One false step and I'll kill you!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 말했다. \"여기서 발견한걸 모두 버려야만 집으로 돌아갈 수 있다.\"");
#else
                strcat(info, " says, \"Drop everything you've found here and return home.\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"결코 오브를 가지고 나갈 수 없다.\"");
#else
                strcat(info, " shouts, \"You will never get the Orb.\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 매우 적의에 찬 모습이다.");
#else
                strcat(info, " looks very unfriendly.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 매우 냉철한 모습이다.");
#else
                strcat(info, " looks very cold.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"이것으로 파티는 끝이다!\"");
#else
                strcat(info, " shouts, \"It's the end of the party!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 말했다. \"훔친 물건을 모두 되돌려놓아라!\"");
#else
                strcat(info, " says, \"Return every stolen item!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 말했다. \"이곳은 불법침입이 인정되지 않는다.\"");
#else
                strcat(info, " says, \"No trespassing is allowed here.\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "은(는) 사악한 미소를 띄웠다.");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"너는 반드시 응징될 것이다!\"");
#else
                strcat(info, " screams, \"You must be punished!\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "은(는) 말했다. \"원한이 있어서 그러는 건 아니야...\"");
#else
                strcat(info, " says, \"It's nothing personal...\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "은(는) 말했다. \"죽어있는 모험가가 착한 모험가지.\"");
#else
                strcat(info, " says, \"A dead adventurer is good adventurer.\"");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "은(는) 말했다. \"여기에 온 것이 너의 마지막 실수이지.\"");
#else
                strcat(info, " says, \"Coming here was your last mistake.\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"침입자다!\"");
#else
                strcat(info, " shouts, \"Intruder!\"");
#endif
                break;
            }
            break;          // end Edmund & Co

        case MONS_JOSEPH:
            switch (random2(16))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 행복한 미소를 띄웠다.");
#else
                strcat(info, " smiles happily.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 말했다. \"널 만나서 기뻐. 그리고 널 죽일 수 있는 것도 기뻐.\"");
#else
                strcat(info, " says, \"I'm happy to see you. And I'll be happy to kill you.\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 말했다. \"이 순간을 얼마나 애타게 기다렸던가.\"");
#else
                strcat(info, " says, \"I've waited for this moment for such a long time.\"");
#endif
                break;
            case 3:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"너에게 원한이 있는 건 아니지만 널 죽여야겠어.\"");
#else
                       " says, \"It's nothing personal but I have kill you.\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 말했다. \"미안하지만 넌 결코 오브를 가질 수 없어.\"");
#else
                strcat(info, " says, \"You will never get the Orb, sorry.\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"난 싸움을 사랑해! 난 살생을 사랑해!\"");
#else
                strcat(info, " shouts, \"I love to fight! I love killing!\"");
#endif
                break;
            case 10:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"침입자를 죽이기위해 난 여기에 있지. 이 일이 정말 좋아.\"");
#else
                       " says, \"I'm here to kill trespassers. I like my job.\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "은(는) 사악한 미소를 띄우려 노렸했다.");
#else
                strcat(info, " tries to grin evilly.");
#endif
                break;
            case 12:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"넌 벌받아야해!.. 널 벌주고 싶어!\"");
#else
                       " says, \"You must be punished! Or... I want to punish you!\"");
#endif
                break;
            case 13:
                strcat(info,
#ifdef JP
                       "은(는) 한탄했다. \"경비를 서는 것은 지루해...\"");
#else
                       " sighs, \"Being guard is usually so boring...\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"기다렸던 전투로군!\"");
#else
                strcat(info, " shouts, \"At last some action!\"");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"와우!\"");
#else
                strcat(info, " shouts, \"Wow!\"");
#endif
                break;
            }
            break;          // end Joseph

        case MONS_ORC_HIGH_PRIEST:  // priest, servants of dark ancient god
        case MONS_DEEP_ELF_HIGH_PRIEST:
            switch (random2(9))
            {
            case 0:
            case 1:
#ifdef JP
                strcat(info, "은(는) 기도했다.");
#else
                strcat(info, " prays.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 2:
#ifdef JP
                strcat(info, "은(는) 무언가 기묘한 기도의 말을 중얼거렸다.");
#else
                strcat(info, " mumbles some strange prayers.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 3:
                strcat(info,
#ifdef JP
                       "은(는) 외쳤다. \"너와 같은 이단자는 죽어야만해.\"");
#else
                       " shouts, \"You are a heretic and must be destroyed.\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 말했다. \"모든 죄인은 죽어야만 한다.\"");
#else
                strcat(info, " says, \"All sinners must die.\"");
#endif
                break;

            case 5:
#ifdef JP
                strcat(info, "은(는) 흥분한 모습니다.");
#else
                strcat(info, " looks excited.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 말했다. \"넌 훌륭한 재물이 될 수 있어.\"");
#else
                strcat(info, " says, \"You will make a fine sacrifice.\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 기도를 노래하기 시작했다.");
#else
                strcat(info, " starts to sing a prayer.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"넌 천벌을 받아야해.\"");
#else
                strcat(info, " shouts, \"You must be punished.\"");
#endif
                break;
            }
            break;          // end priests

        case MONS_ORC_SORCERER:   // hateful wizards, using strange powers
        case MONS_DEEP_ELF_SORCERER:
        case MONS_WIZARD:
            switch (random2(19))
            {
            case 0:
            case 1:
            case 2:
#ifdef JP
                strcat(info, "은(는) 몹시 거친 몸짓을 했다.");
#else
                strcat(info, " wildly gestures.");
#endif
                mpr( info, MSGCH_MONSTER_SPELL );
                if (coinflip())
                    canned_msg( MSG_NOTHING_HAPPENS );
                else
                    canned_msg( MSG_YOU_RESIST );
                return (true);

            case 3:
            case 4:
            case 5:
#ifdef JP
                strcat(info, "은(는) 무엇인가 기묘한 말로 중얼거렸다.");
#else
                strcat(info, " mumbles some strange words.");
#endif
                mpr( info, MSGCH_MONSTER_SPELL );
                if (coinflip())
                    canned_msg( MSG_NOTHING_HAPPENS );
                else
                    canned_msg( MSG_YOU_RESIST );
                return (true);

            case 6:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"넌 나의 힘을 당할 수 없어!\"");
#else
                strcat(info, " shouts, \"You can't withstand my power!\"");
#endif
                break;

            case 7:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"너는 죽은거나 다름없어.\"");
#else
                strcat(info, " shouts, \"You are history.\"");
#endif
                break;

            case 8:
#ifdef JP
                simple_monster_message( monster, "은(는) 주문을 외우기 시작했다.",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "은(는) 잠깐 투명하게 되었다.");
#else
                strcat(info, " becomes transparent for a moment.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 9:
#ifdef JP
                strcat(info, "은(는) 당신을 향해 기묘한 힘을 투사했다.");
#else
                strcat(info, " throws some strange powder towards you.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 10:
#ifdef JP
                simple_monster_message( monster, "은(는) 주문을 시전했다.",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "은(는) 잠깐 밝게 빛났다.");
#else
                strcat(info, " glows brightly for a moment.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 11:
#ifdef JP /* ?? */
                strcat(info, "은(는) 말했다. \"아르가탁스 네트라노크 데르텍스\"");
#else
                strcat(info, " says, \"argatax netranoch dertex\"");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 12:
#ifdef JP /* ?? */
                strcat(info, "은(는) 말했다. \"두그르우 누테우 베르그\"");
#else
                strcat(info, " says, \"dogrw nutew berg\"");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 13:
#ifdef JP /* ?? */
                strcat(info, "은(는) 외쳤다. \"엔트람 모스 데그 우라그\"");
#else
                strcat(info, " shouts, \"Entram moth deg ulag!\"");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 14:
#ifdef JP
                strcat(info, "은(는) 주문을 외웠다.");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);

                strcpy(info, m_name);
#ifdef JP
                strcat(info, "은(는) 잠깐 커졌다.");
#else
                strcat(info, " becomes larger for a moment.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 15:
#ifdef JP
                strcat(info, "은(는) 주문을 외웠다.");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);

                strcpy(info, m_name);
#ifdef JP
                strcat(info, "의 손가락 끝이 빛났다.");
#else
                strcat(info, "'s fingertips starts to glow.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 16:
#ifdef JP
                strcat(info, "의 눈이 빛나기 시작했다.");
#else
                strcat(info, "'s eyes starts to glow.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 17:
#ifdef JP
                strcat(info, "은(는) 당신을 응시해서 마비를 시도했다.");
#else
                strcat(info, " tries to paralyze you with his gaze.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 18:
#ifdef JP
                strcat(info, "은(는) 주문을 외웠다.");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
                canned_msg( MSG_YOU_RESIST );
                return (true);
            }
            break;          // end wizards

        case MONS_JESSICA:  // sorceress disturbed by player
            switch (random2(10))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 사악한 미소를 지었다.");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 말했다. \"나를 진심으로 화나게 했군.\"");
#else
                strcat(info, " says, \"I'm really upset.\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"나는 너 같은 녀석은 싫어.\"");
#else
                strcat(info, " shouts, \"I don't like beings like you.\"");
#endif
                break;
            case 3:
                strcat(info,
#ifdef JP
                       "은(는) 외쳤다. \"나를 그만 귀찮게 해. 안그러면 죽여버릴테다!\"");
#else
                       " shouts, \"Stop bothering me, or I'll kill you!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 싸늘하게 말했다. \"네 부하로 있기가 싫어.\"");
#else
                strcat(info, " very coldly says, \"I hate your company.\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 이상한 말을 중얼거리고 있다.");
#else
                strcat(info, " mumbles something strange.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 매우 화난 것 같다.");
#else
                strcat(info, " looks very angry.");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                        "은(는) 외쳤다. \"너는 날 귀찮게 하는군. 널 죽여야겠다.\"");
#else
                        " shouts, \"You're disturbing me.  I'll have kill you.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"넌 매우 불쾌한 놈이다!\"");
#else
                strcat(info, " screams, \"You are a ghastly nuisance!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 과장된 몸짓을 했다.");
#else
                strcat(info, " gestures wildly.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;
            }
            break;          // end Jessica

        case MONS_SIGMUND:  // mad old wizard
            switch (random2(19))
            {
            case 0:
            case 1:
            case 2:
#ifdef JP
                strcat(info, "은(는) 미친듯이 웃었다.");
#else
                strcat(info, " laughs crazily.");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 말했다. \"걱정마, 빠르게 죽여줄테니.\"");
#else
                strcat(info, " says, \"Don't worry, I'll kill you fast.\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 이빨을 득득 갈았다.");
#else
                strcat(info, " grinds his teeth.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 물었다. \"날 좋아해?\"");
#else
                strcat(info, " asks, \"Do you like me?\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 절규했다. \"죽어라, 괴물!\"");
#else
                strcat(info, " screams, \"Die, monster!\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 말했다. \"곧바로 모든 걸 잊게 될꺼야.\"");
#else
                strcat(info, " says, \"You will soon forget everything.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"넌 절대... 절대로 못해!\"");
#else
                strcat(info, " screams, \"You will never... NEVER!\"");
#endif
                break;

            case 9:
#ifdef JP
                simple_monster_message( monster, "은(는) 주문을 외웠다.",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "의 눈이 붉게 빛나기 시작했다.");
#else
                strcat(info, "'s eyes starts to glow with a red light. ");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 10:
#ifdef JP
                strcat(info, "은(는) 말했다. \"내 눈을 쳐다봐.\"");
#else
                strcat(info, " says, \"Look in to my eyes.\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "은(는) 말했다. \"내가 너의 사신이다.\"");
#else
                strcat(info, " says, \"I'm your fate.\"");
#endif
                break;

            case 12:
#ifdef JP
                simple_monster_message( monster, "은(는) 주문을 외웠다.",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "은(는) 갑자기 창백한 빛에 쌓였다.");
#else
                strcat(info, " is suddenly surrounded by pale blue light.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 13:
#ifdef JP
                strcat(info, "은(는) 당신을 물어뜯으려고했다.");
#else
                strcat(info, " tries to bite you.");
#endif
                break;

            case 14:
#ifdef JP
                simple_monster_message( monster, "은(는) 주문을 외웠다.",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "은(는) 갑자기 희미한 초록 빛에 싸였다.");
#else
                strcat(info, " is suddenly surrounded by pale green light.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 15:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"나는 죽음의 천사다!\"");
#else
                strcat(info, " screams, \"I am the angel of Death!\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"오직 죽음만이 너를 자유롭게 할 수 있다!\"");
#else
                strcat(info, " screams, \"Only death can liberate you!\"");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "은(는) 속삭였다. \"이제 곧 사후세계를 보여줄께...\"");
#else
                strcat(info, " whispers, \"You'll know eternity soon...\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"쓸데없는 저항은 하지마!\"");
#else
                strcat(info, " screams, \"Don't try to resist!\"");
#endif
                break;
            }
            break;          // end Sigmund

        case MONS_IMP:      // small demon
        case MONS_WHITE_IMP:
        case MONS_SHADOW_IMP:
            if (one_chance_in(3))
            {
                imp_taunt( monster );
                return (true);
            }
            else
            {
                switch (random2(11))
                {
                case 0:
#ifdef JP
                    strcat(info, "은(는) 미친듯이 웃었다.");
#else
                    strcat(info, " laughs crazily.");
#endif
                    break;
                case 1:
#ifdef JP
                    strcat(info, "은(는) 사악한 미소를 지었다.");
#else
                    strcat(info, " grins evilly.");
#endif
                    break;
                case 2:
#ifdef JP
                    strcat(info, "은(는) 당신에게 연기를 조금씩 뿜었다.");
#else
                    strcat(info, " breathes a bit of smoke at you.");
#endif
                    break;
                case 3:
#ifdef JP
                    strcat(info, "은(는) 꼬리로 격렬하게 내리쳤다.");
#else
                    strcat(info, " lashes with his tail.");
#endif
                    break;
                case 4:
#ifdef JP
                    strcat(info, "은(는) 이빨을 부딪쳤다.");
#else
                    strcat(info, " grinds his teeth.");
#endif
                    break;
                case 5:
#ifdef JP
                    strcat(info, "은(는) 침을 뱉었다.");
#else
                    strcat(info, " sputters.");
#endif
                    break;
                case 6:
#ifdef JP
                    strcat(info, "은(는) 당신을 향해 증기를 조금씩 뿜었다.");
#else
                    strcat(info, " breathes some steam toward you.");
#endif
                    break;
                case 7:
#ifdef JP
                    strcat(info, "은(는) 당신에게 침을 뱉었다.");
#else
                    strcat(info, " spits at you.");
#endif
                    break;
                case 8:
#ifdef JP
                    strcat(info, "은(는) 순간적으로 사라졌다.");
#else
                    strcat(info, " disappears for a moment.");
#endif
                    break;
                case 9:
#ifdef JP
                    strcat(info, "은(는) 파리떼를 소환했다.");
#else
                    strcat(info, " summons a swarm of flies.");
#endif
                    break;
                case 10:
#ifdef JP
                    strcat(info, "은(는) 근처의 투구벌레를 잡아먹었다.");
#else
                    strcat(info, " picks up some beetle and eats it.");
#endif
                    break;
                }
            }
            break;          // end imp

        case MONS_TORMENTOR:        // cruel devil
            if (one_chance_in(10))
            {
                demon_taunt( monster );
                return (true);
            }
            else
            {
                switch (random2(18))
                {
                case 0:
#ifdef JP
                    strcat(info, "은(는) 미친듯이 웃었다.");
#else
                    strcat(info, " laughs crazily.");
#endif
                    break;
                case 1:
#ifdef JP
                    strcat(info, "은(는) 사악한 미소를 지었다.");
#else
                    strcat(info, " grins evilly.");
#endif
                    break;
                case 2:
#ifdef JP
                    strcat(info, "은(는) 말했다. \"난 너의 모든 악몽을 현실로 만들어 주겠다.\"");
#else
                    strcat(info, " says, \"I am all your nightmares come true.\"");
#endif
                    break;
                case 3:
#ifdef JP
                    strcat(info, "은(는) 말했다. \"고통이란게 무언지 보여주겠어.\"");
#else
                    strcat(info, " says, \"I will show you what pain is.\"");
#endif
                    break;
                case 4:
#ifdef JP
                    strcat(info, "은(는) 외쳤다. \"너를 조각조각 찢어주겠어.\"");
#else
                    strcat(info, " shouts, \"I'll tear you apart.\"");
#endif
                    break;
                case 5:
                    strcat(info,
#ifdef JP
                           "은(는) 말했다. \"내 고문을 받으면 오히려 죽음을 바라게 되지.\"");
#else
                           " says, \"You will wish to die when I get to you.\"");
#endif
                    break;
                case 6:
#ifdef JP
                    strcat(info, "은(는) 말했다. \"너의 피에 빠져죽게 만들어주마.\"");
#else
                    strcat(info, " says, \"I will drown you in your own blood.\"");
#endif
                    break;
                case 7:
                    strcat(info,
#ifdef JP
                           "은(는) 부르짖었다. \"넌 비참하게 죽을 것이다.\"");
#else
                           " screams, \"You will die horribly!\"");
#endif
                    break;
                case 8:
#ifdef JP
                    strcat(info, "은(는) 말했다. \"네 간을 먹을 것이다.\"");
#else
                    strcat(info, " says, \"I will eat your liver.\"");
#endif
                    break;
                case 9:
#ifdef JP
                    strcat(info, "은(는) 미친 것처럼 싱글거리고 있다.");
#else
                    strcat(info, " grins madly.");
#endif
                    break;
                case 10:
#ifdef JP
                    strcat(info, "은(는) 외쳤다. \"내가 보내는 천 개의 바늘 고통을 준비하게!\"");
#else
                    strcat(info, " shouts, \"Prepare for my thousand needles of pain!\"");
#endif
                    break;
                case 11:
                    strcat(info,
#ifdef JP
                           "은(는) 말했다. \"난 널 죽일 천 가지 방법을 알고있지.\"");
#else
                           " says, \"I know thousand and one way to kill you.\"");
#endif
                    break;
                case 12:
                    strcat(info,
#ifdef JP
                           "은(는) 말했다. \"너에게 나의 고문실을 보여주마!\"");
#else
                           " says, \"I'll show you my torture chamber!\"");
#endif
                    break;
                case 13:
                case 14:
                    strcat(info,
#ifdef JP
                           "은(는) 말했다. \"너의 뼈 하나 하나를 부숴주마.\"");
#else
                           " says, \"I'll crush your bones, one by one.\"");
#endif
                    break;
                case 15:
#ifdef JP
                    strcat(info, "은(는) 말했다. \"난 너의 운명을 알지. 그건 고통 뿐이야.\"");
#else
                    strcat(info, " says, \"I know your fate. It's pain.\"");
#endif
                    break;
                case 16:
#ifdef JP
                    strcat(info, "은(는) 말했다. \"각오해! 고통이 기다리고 있으니.\"");
#else
                    strcat(info, " says, \"Get ready! Throes await you.\"");
#endif
                    break;
                case 17:
#ifdef JP
                    strcat(info, "은(는) 악의를 뿜고 있다.");
#else
                    strcat(info, " grins malevolently.");
#endif
                    break;
                }
            }
            break;          // end tormentor

        case MONS_PANDEMONIUM_DEMON:    // named demons
        case MONS_GERYON:
        case MONS_ASMODEUS:
        case MONS_DISPATER:
        case MONS_ANTAEUS:
        case MONS_ERESHKIGAL:
        case MONS_MNOLEG:
        case MONS_LOM_LOBON:
        case MONS_CEREBOV:
        case MONS_GLOORX_VLOQ:
            demon_taunt( monster );
            return (true);

        case MONS_PLAYER_GHOST:     // ghost of unsuccesful player
            switch (random2(24))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 미친 것처럼 웃고 있다.");
#else
                strcat(info, " laughs crazily.");
#endif
                break;

            case 1:
#ifdef JP
                strcat(info, "은(는) 사악한 미소를 지었다.");
#else
                strcat(info, " grins evilly.");
#endif
                break;

            case 2:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"넌 결코 오브를 가질 수 없어!\"");
#else
                strcat(info, " shouts, \"You will never get the ORB!\"");
#endif
                break;

            case 3: // mv: ghosts are usually wailing, aren't ?
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
#ifdef JP
                strcat(info, "은(는) 울부짖었다.");
#else
                strcat(info, " wails.");
#endif
                break;

            case 12:
#ifdef JP
                strcat(info, "은(는) 당신을 가만히 응시했다.");
#else
                strcat(info, " stares at you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신은 한기를 느꼈다.", MSGCH_WARN);
#else
                mpr("You feel cold.", MSGCH_WARN);
#endif
                return (true);

            case 13:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"너도 이제 곧 나의 동료가 될 것이다!\"");
#else
                strcat(info, " screams, \"You will join me soon!\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "은(는) 한탄했다. \"죽어버려, 그리고 잠들어, 그걸로 끝이야.\"");
#else
                strcat(info, " wails, \"To die, to sleep, no more.\"");
#endif
                break;      //Hamlet 됇궕궳궖궫귞뫮뽷귩뱰궫귡
            case 15:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"내가 실패한 것처럼 너도 실패할 것이다.\"");
#else
                       " screams, \"You must not succeed where I failed.\"");
#endif
                break;
            case 16:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"누구든 오브를 원하는자는 죽여버리겠다.\"");
#else
                       " screams, \"I'll kill anyone who wants the ORB.\"");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "은(는) 속삭였다. \"죽음의 공허함의 일부가 되어라!\"");
#else
                strcat(info, " whispers, \"Meet emptiness of death!\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "은(는) 속삭였다. \"죽음이야말로 해방이다.\"");
#else
                strcat(info, " whispers, \"Death is liberation.\"");
#endif
                break;
            case 19:
                strcat(info,
#ifdef JP
                       "은(는) 속삭였다. \"영원의 고요함이 널 기다리고 있다.\"");
#else
                       " whispers, \"Everlasting silence awaits you.\"");
#endif
                break;
            case 20:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"막지마. 너에겐 기회가 없어!\"");
#else
                       " screams, \"Don't try to defend. You have no chance!\"");
#endif
                break;
            case 21:
                strcat(info,
#ifdef JP
                       "은(는) 속삭였다. \"죽음은 아프지 않아. 삶이 고통인 것이지.\"");
#else
                       " whispers, \"Death doesn't hurt. What you feel is life.\"");
#endif
                break;
            case 22:
#ifdef JP
                strcat(info, "은(는) 속삭였다. \"오브 같은 건 없어.\"");
#else
                strcat(info, " whispers, \"The ORB doesn't exist.\"");
#endif
                break;
            case 23:
#ifdef JP
                strcat(info, "은(는) 한탄했다. \"넌 이제 죽을 수 밖에 없어.\"");
#else
                strcat(info, " wails, \"Death is your only future.\"");
#endif
                break;
            }
            break;          // end players ghost

        case MONS_PSYCHE:   // insane girl
            switch (random2(20))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 행복한 미소를 지었다.");
#else
                strcat(info, " smiles happily.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 미친듯이 킥킥 웃었다.");
#else
                strcat(info, " giggles crazily.");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 울고있다.");
#else
                strcat(info, " cries.");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 잠시동안 당신을 응시했다.");
#else
                strcat(info, " stares at you for a moment.");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 노래했다.");
#else
                strcat(info, " sings.");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"부탁인데, 좀 더 빨리 죽어줄 수 있니?\"");
#else
                       " says, \"Please, could you die a little faster?\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"난 나쁜 여자야. 하지만 나도 어쩔 수 없어.\"");
#else
                       " says, \"I'm bad girl. But I can't do anything about it.\"");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"넌 이지역의 안전을 위협하고 있다.\"");
#else
                       " screams, \"YOU ARE VIOLATING AREA SECURITY!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 울부짖었다. \"나는 피도 폭력도 싫단말야.\"");
#else
                strcat(info, " cries, \"I hate blood and violence.\"");
#endif
                break;
            case 9:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"평화! 꽃! 자유! 그리고 죽어버린 모험가들!\"");
#else
                       " screams, \"Peace! Flowers! Freedom! Dead adventurers!\"");
#endif
                break;
            case 10:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"난 너무 외로워. 시체만이 나의 친구들 이었지.\"");
#else
                       " says, \"I'm so lonely. Only corpses are my friends.\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "은(는) 울부짖었다. \"당신이 나의 애완동물을 죽였다.\"");
#else
                strcat(info, " cries, \"You've killed my pet.\"");
#endif
                break;
            case 12:
                strcat(info,
#ifdef JP
                       "은(는) 울부짖었다. \"나의 오브 콜랙션을 훔치려고 하는건가?\"");
#else
                       " cries, \"You want to steal my orb collection?!\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "은(는) 괴상한 노래를 불렀다.");
#else
                strcat(info, " sings some strange song.");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "은(는) 눈물을 철철 흘렸다.");
#else
                strcat(info, " bursts in tears.");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "은(는) 엄지손가락을 빨고 있다.");
#else
                strcat(info, " sucks her thumb.");
#endif
                break;
            case 16:
                strcat(info,
#ifdef JP
                       "은(는) 속삭였다. \"안아줘, 나를 짜릿하게 해줘, 키스해줘, 날 죽여줘.\"");
#else
                       " whispers, \"Hold me, thrill me, kiss me, kill me.\"");
#endif
                break;      //(c) U2 ?
            case 17:
#ifdef JP
                strcat(info, "은(는) 말했다. \"당신을 죽여서 집으로 가져갈께.\"");
#else
                strcat(info, " says, \"I'll kill you and take you home.\"");
#endif
                break;
            case 18:
                strcat(info,
#ifdef JP
                       "은(는) 외쳤다. \"그래, 나는 미쳤을꺼야, 그렇지만 뭐 어때?\"");
#else
                       " shouts, \"Well, maybe I'm nutty, but who cares?\"");
#endif
                break;
            case 19:
                strcat(info,
#ifdef JP
                       "은(는) 외쳤다. \"난 어쨌든 당신이 슬퍼하길 바래.\"");
#else
                       " shouts, \"I hope that you are sorry for that.\"");
#endif
                break;
            }
            break;          // end Psyche

        case MONS_DONALD:   // adventurers hating competition
        case MONS_WAYNE:
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"집으로 돌아가!\"");
#else
                strcat(info, " screams, \"Return home!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"오브는 나의 것이다!\"");
#else
                strcat(info, " screams, \"The Orb is mine!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"너의 보물 전부를 내놔라!\"");
#else
                strcat(info, " screams, \"Give me all your treasure!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"결코 오브를 가져갈 수 없다!\"");
#else
                strcat(info, " screams, \"You will never get the Orb!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"내가 여기 제일 먼저 왔다!\"");
#else
                strcat(info, " screams, \"I was here first!\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 눈쌀을 찌뿌렸다.");
#else
                strcat(info, " frowns.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 매우 분개한 모습이다.");
#else
                strcat(info, " looks very upset.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"도망치든지 죽든지!\"");
#else
                strcat(info, " screams, \"Get away or die!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어라!\"");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"여길 지나가려면 날 먼저 쓰러뜨려야 해!\"");
#else
                strcat(info, " screams, \"First you have to pass me!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"난 니가 싫어!\"");
#else
                strcat(info, " screams, \"I hate you!\"");
#endif
                break;
            }
            break;          // end Donald

        case MONS_MICHAEL:  // spellcaster who wanted to be alone
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 매우 화가난 것 같다.");
#else
                strcat(info, " looks very angry.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 얼굴을 찡그렸다.");
#else
                strcat(info, " frowns.");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"난 혼자있고 싶어!\"");
#else
                strcat(info, " screams, \"I want to be alone!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 말했다. \"넌 정말 우울하게 생겼군.\"");
#else
                strcat(info, " says, \"You are really nuisance.\"");
#endif
                break;
            case 4:
                strcat(info,
#ifdef JP
                       "은(는) 절규했다. \"난 혼자 있고 싶어. 그러니까 넌...\"");
#else
                       " screams, \"I wanted to be alone. And you...\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"꺼져! 아니면 죽는게 나을꺼야!\"");
#else
                strcat(info, " screams, \"Get away! Or better yet, die!\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 기묘한 단어들을 중얼거렸다.");
#else
                strcat(info, " mumbles some strange words.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 7:
#ifdef JP
                strcat(info, "은(는) 당신을 가리켰다.");
#else
                strcat(info, " points at you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
                canned_msg(MSG_YOU_RESIST);
                return (true);

            case 8:
#ifdef JP
                strcat(info, "은(는) 분노에 떨고있다.");
#else
                strcat(info, " shakes with wrath.");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 물약을 마셨다.");
#else
                strcat(info, " drinks a potion.");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 과장된 몸짓을 했다.");
#else
                strcat(info, " gestures wildly.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;
            }
            break;          // end Michael

        case MONS_ERICA:    // wild tempered adventuress
            switch (random2(12))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어라!\"");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"빠른 걸 원해? 느린 걸 원해?\"");
#else
                strcat(info, " screams, \"Do you want it fast or slow?\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 화가 난 모습이다.");
#else
                strcat(info, " looks angry.");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 물약을 마셨다.");
#else
                strcat(info, " drinks a potion.");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 말했다. \"난 너보다 훨씬 뛰어나.\"");
#else
                strcat(info, " says, \"I'm so much better than you.\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"나의 죽이는 방법은 빠르고 완벽하지.\"");
#else
                       " says, \"Fast and perfect. Such is my way of killing.\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"서둘러! 사신이 기다리고 있어!\"");
#else
                strcat(info, " screams, \"Hurry! Death awaits!\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 몹시 거칠게 웃었다.");
#else
                strcat(info, " laughs wildly.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"그것이 어디에 있는지 절대 말할 수 없다!\"");
#else
                strcat(info, " screams, \"I'll never tell where it is!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"넌 결코 그것을 가질 수 없다!\"");
#else
                strcat(info, " screams, \"You'll never get it!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"여기까지 오는건 자살행위지!\"");
#else
                strcat(info, " screams, \"Coming here was suicide!\"");
#endif
                break;
            case 11:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"난 싸움이 좋아. 그러나 살생은 더 좋아하지.\"");
#else
                       " says, \"I love to fight,  but killing is better.\"");
#endif
                break;
            }
            break;          // end Erica

        case MONS_JOSEPHINE:        // ugly old witch looking for somone to kill
            switch (random2(13))
            {
            case 0:
            case 1:
            case 2:
#ifdef JP
                strcat(info, "은(는) 사악한 미소를 띄웠다.");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 3:
            case 4:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"널 죽이고 말겠다!\"");
#else
                strcat(info, " screams, \"I will kill you!\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 이를 갈았다.");
#else
                strcat(info, " grinds her teeth.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 악의를 내뿜고 있다.");
#else
                strcat(info, " grins malevolently.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 미친듯이 웃었다.");
#else
                strcat(info, " laughs insanely.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어라!\"");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 9:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"널 위해 소중한 무언가를 준비했지!\"");
#else
                       " screams, \"I have something special for you!\"");
#endif
                break;
            case 10:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"너의 머리로 내 집을 장식살꺼야!\"");
#else
                       " screams, \"I'll use your head as decoration in my hut!\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "은(는) 말했다. \"너의 가죽을 깔개로 만들겠다.\"");
#else
                strcat(info, " says, \"I'll make a rug of your skin.\"");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "은(는) 말했다. \"목을 베어 버리는 건 어때?\"");
#else
                strcat(info, " says, \"How about some decapitation?\"");
#endif
                break;
            }
            break;          // end Josephine

        case MONS_HAROLD:  // middle aged man, hired to kill you. He is in a hurry.
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 초조한 모습니다.");
#else
                strcat(info, " looks nervous.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"서둘러!\"");
#else
                strcat(info, " screams, \"Hurry up!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"더 빨리 죽여줄까?\"");
#else
                strcat(info, " screams, \"Could you die faster?\"");
#endif
                break;
            case 3:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"가만히 있어. 죽여줄테니까.\"");
#else
                       " says, \"Stand still. I'm trying to kill you.\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어라!\"");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 말했다. \"빨리 죽기를 바란다!\"");
#else
                strcat(info, " says, \"I hope you die soon!\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"몇 대면 죽겠군.\"");
#else
                       " says, \"Only few hits and it's over.\".");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 말했다. \"내가 서두르고 있다는걸 알아라.\"");
#else
                strcat(info, " says, \"You know, I'm in a hurry.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"곧바로 숨통을 끊어주겠다.\"");
#else
                strcat(info, " screams, \"I'll finish you soon!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"질질 끌지마라.\"");
#else
                strcat(info, " screams, \"Don't delay it.\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "은(는) 말했다. \"이유는 없다. 그냥 하는거야. 널 죽일뿐이야.\"" );
#else
                strcat(info, " says, \"Mine is not to reason why.  Mine's to do, yours to die.\"" );
#endif
            }
            break;          // end Harold

        // skilled warrior looking for some fame. More deads = more fame
        case MONS_NORBERT:
            switch (random2(13))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 행복한 미소를 띄웠다.");
#else
                strcat(info, " smiles happily.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어라, 몬스터!\"");
#else
                strcat(info, " screams, \"Die, monster!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"나는 영웅이다!\"");
#else
                strcat(info, " screams, \"I'm a hero!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"그래! 또다른 기록이군!");
#else
                strcat(info, " shouts, \"YES! Another notch!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 말했다. \"안됬지만 너의 머리는 추한 트로피가 될 것이다.\"");
#else
                strcat(info, " says, \"A pity your head will make such an ugly trophy.\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"기도해라. 넌 곧 죽을꺼니까!\"");
#else
                       " screams, \"Pray, because you'll die soon!\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                      "은(는) 물었다. \"유언장은 썼어? 필요할텐데.\"");
#else
                      " asks \"Did you write a will? You should.\".");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"난 너 같은 추악한 몬스터를 죽이는게 좋아.\"");
#else
                       " says, \"I love killing ugly monsters like you.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"피와 살육!\"");
#else
                strcat(info, " screams, \"Blood and destruction!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 말했다. \"알다시피 내 손에 죽는 것은 명예로운 것이다.\"");
#else
                strcat(info, " says, \"You know, it's honour to die by my hand.\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"죽을때가 되었군!\"");
#else
                strcat(info, " shouts, \"Your time has come!\"");
#endif
                break;
            case 11:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"유감이지만 너에겐 기회가 없어.\"");
#else
                       " says, \"I'm sorry but you don't have a chance.\"");
#endif
                break;
            case 12:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"또다른 몬스터의 죽음이라... 오늘은 횡재했군!\"");
#else
                       " says, \"Another dead monster... It must be my lucky day!\"");
#endif
                break;
            }
            break;          // end Norbert

        case MONS_JOZEF:    // bounty hunter
            switch (random2(14))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 배불러 보였다.");
#else
                strcat(info, " looks satisfied.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어라!\"");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"간신히 널 찾아냈군!\"");
#else
                strcat(info, " screams, \"At last I found you!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"네 머리를 잘라서 현상금 500골드를 받아야겠다!\"");
#else
                strcat(info, " shouts, \"I'll get 500 for your head!\"");
#endif
                break;
            case 4:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"넌 아무리봐도 현상금 만큼 값어치있어 보이지 않는데.\"");
#else
                       " says, \"You don't look worth for that money.\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"너에게 원한은 없다. 다만 돈 때문이지!\"");
#else
                       " says, \"It's nothing personal. I'm paid for it!\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "은(는) 물었다. \"유언장은 썼는가? 필요할꺼야.\"");
#else
                       " asks \"Did you write a testament? You should.\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 말했다. \"넌 [");
#else
                strcat(info, " says, \"You are ");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "]이다. 그렇지?\"");
#else
                strcat(info, ", aren't you?.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 말했다. \"넌 [");
#else
                strcat(info, " says, \"I suppose that you are ");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "]일 것이다. 아니면 미안하지만.\"");
#else
                strcat(info, ". Sorry, if I'm wrong.\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 말했다. \"[");
#else
                strcat(info, " says, \"One dead ");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "]를 죽인자에게 살인청부의 댓가로 금화 500냥을 주겠다.");
#else
                strcat(info, ", 500 gold pieces. It's in my contract.\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"넌 이제 죽을 시간이다!\"");
#else
                strcat(info, " shouts, \"Your time has come!\"");
#endif
                break;
            case 11:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. 나의 일은 가끔은 매우 재미있다. 가끔은...\"");
#else
                       " says, \"My job is sometimes very exciting. Sometimes...\"");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "은(는) 말했다. \"내가 생각하기에 난 돈을 받을 가치가 있어.\"");
#else
                strcat(info, " says, \"I think I deserve my money.\"");
#endif
                break;
            case 13:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"죽어랏! 난 오늘 처리해야할 청부살인이 더 있단 말야.\"");
#else
                       " screams, \"Die! I've got more contracts today.\"");
#endif
                break;
            }
            break;          // end Jozef

        case MONS_AGNES:    // she is trying to get money and treasure
            switch (random2(10))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"가진 돈을 모두 내놓아라!\"");
#else
                strcat(info, " screams, \"Give me all your money!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"이 모든 보물은 모두 내 것이야!\"");
#else
                strcat(info, " screams, \"All treasure is mine!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"넌 결코 나의 돈을 가져가지 못한다!\"");
#else
                strcat(info, " screams, \"You'll never get my money!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 사악한 미소를 지었다.");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 4:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"가진 것 모두를 내어놓고 꺼져라!\"");
#else
                       " screams, \"Give me everything and get away!\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"새로운 로브가 필요해. 너의 돈으로 그걸 사고말 것이다.\"");
#else
                       " says, \"I need new robe. I'll buy it from your money.\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"너의 반지, 너의 목걸이, 그리고... 모든 것을 원한다!\"");
#else
                       " screams, \"I want your rings! And amulets! And... EVERYTHING!\"");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"난 너 같이 더러운 모험가를 싫어한다.\"");
#else
                       " screams, \"I hate dirty adventurers like you.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 말했다. \"어떻게하면 너처럼 추레하게 옷을 입을 수 있지?\"");
#else
                strcat(info, " says, \"How can you wear that ugly dress?\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어라, 짐승!\"");
#else
                strcat(info, " screams, \"Die, beast!\"");
#endif
                break;
            }
            break;          // end Agnes

        case MONS_MAUD:     // warrior princess looking for sword "Entarex"
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"항복인가 죽음인가!\"");
#else
                strcat(info, " screams, \"Submit or die!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"나에게 [엔타렉스]를 다오!\"");
#else
                strcat(info, " screams, \"Give me \"Entarex\"!\"");
#endif
                break;
            case 2:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"[엔타렉스]를 건내주면 살려서 보내주겠다!\"");
#else
                       " screams, \"If you give me \"Entarex\", I'll let you live!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 눈살을 찌푸렸다.");
#else
                strcat(info, " frowns.");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 마음이 동요한 것처럼 보인다.");
#else
                strcat(info, " looks upset.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"넌 나의 힘에 맞설 수 없다!\"");
#else
                strcat(info, " screams, \"You can't face my power!\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"나에게 칼을 다오! 지금 즉시!\"");
#else
                       " screams, \"Give me that sword! Immediately!\"");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. 너의 생명인가 아니면 [엔타렉스]인가! 고르거라.\"");
#else
                       " screams, \"Your life or \"Entarex\"! You must choose.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은 부르짖었다. \"난 그것을 원해!\"");
#else
                strcat(info, " screams, \"I want it!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어라, 이 도둑놈!\"");
#else
                strcat(info, " screams, \"Die, you thief!\"");
#endif
                break;
            case 10:
                // needed at least one in here to tie to the amnesia
                // scroll reference -- bwr
#ifdef JP
                strcat(info, "은(는) 물었다. \"네가 죽을때 넌 날 생각할 것인가?\"");
#else
                strcat(info, " asks \"Will you think of me as you die?\"");
#endif
                break;
            }
            break;          // end Maud

        // wizard looking for bodyparts as spell components
        case MONS_FRANCIS:
            switch (random2(15))
            {
            case 0:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"눈이 좋군. 내가 좀 써야겠어.\"");
#else
                       " says, \"You've nice eyes. I could use them.\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 말했다. \"미안하지만 난 너의 머리통이 필요해.\"");
#else
                strcat(info, " says, \"Excuse me, but I need your head.\"");
#endif
                    break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 말했다. \"난 오직 너의 내장의 일부가 필요해!\"");
#else
                strcat(info, " says, \"I only need a few of your organs!\"");
#endif
                    break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 그 상황에 대해 심사숙고 중이다.");
#else
                strcat(info, " ponders the situation.");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 수술칼을 찾고있다.");
#else
                strcat(info, " looks for scalpel.");
#endif
                break;

            case 5:
#ifdef JP
                simple_monster_message( monster, "은(는) 주문을 외웠다",
#else
                simple_monster_message( monster, " casts a spell",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "의 손에서 부드러운 빛이 났다.");
#else
                strcat(info, "'s hands started to glow with soft light.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 6:
#ifdef JP
                strcat(info, "은(는) 말했다. \"조금도 아프지않아.\"");
#else
                strcat(info, " says, \"This won't hurt a bit.\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 무언가를 당신에게 던졌다.");
#else
                strcat(info, " throws something at you.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 말했다. \"나의 실험에 니가 필요해!\"");
#else
                strcat(info, " says, \"I want you in my laboratory!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 말했다. \"조금만 해부를 해보는건 어때?\"");
#else
                strcat(info, " says, \"What about little dissection?\"");
#endif
                break;
            case 10:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"널위해 특별한 걸 준비했지.\"");
#else
                       " says, \"I have something special for you.\"");
#endif
                    break;
            case 11:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"꼼짝마랏! 난 너의 귀를 잘라낼 것이다!\"");
#else
                       " screams, \"Don't move! I want to cut your ear!\"");
#endif
                break;
            case 12:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"심장을 도려내는 건 어때? 그거 너한테 필요한거야?\"");
#else
                       " says, \"What about your heart? Do you need it?\"");
#endif
                break;
            case 13:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"시체는 중요한 천연자원이지. 알고있었어?\"");
#else
                       " says, \"Did you know that corpses are an important natural resource?\"");
#endif
                break;
            case 14:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"걱정하지마, 난 필요한 부분만 잘라가니까.\"");
#else
                       " says, \"Don't worry, I'll only take what I need.\"");
#endif
                break;
            }
            break;          // end Francis

        case MONS_RUPERT:   // crazy adventurer
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 말했다. \"넌 괴물이지? 그렇지?\"");
#else
                strcat(info, " says, \"You are a monster, aren't you?\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어라 괴물!\"");
#else
                strcat(info, " screams, \"Die, monster!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"나에게 성배를 다오!\"");
#else
                strcat(info, " screams, \"Give me Holy Grail!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"빨강색! 파랑은 아니야!\"");
#else
                strcat(info, " screams, \"Red!  No, blue!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 혼란스러워 보였다.");
#else
                strcat(info, " looks confused.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 흥분한 것 같다.");
#else
                strcat(info, " looks excited.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"나는 위대한 최강의 영웅이다!\"");
#else
                strcat(info, " shouts, \"I'm great and powerful hero!\"");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"각오해라! 널 죽여버리겠다!\"");
#else
                       " screams, \"Get ready! I'll kill you! Or something like it...\"");
#endif
                break;
            case 8:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"엄마는 내게 항상 몰살시켜야 한다고 말슴하셨지.\"");
#else
                       " says, \"My Mom always said, kill them all.\"");
#endif
                break;
            case 9:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"살인마, 니가 이 사랑스런 괴물들을 죽였지!\"");
#else
                       " screams, \"You killed all those lovely monsters, you murderer!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"만세!\"");
#else
                strcat(info, " screams, \"Hurray!\"");
#endif
                break;
            }
            break;          // end Rupert

        case MONS_NORRIS:   // enlighten but crazy man
            switch (random2(24))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 노래했다. \"하레 라마, 하레 크리쉬나!\"");
#else
                strcat(info, " sings \"Hare Rama, Hare Krishna!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 당신을 보고 미소지었다.");
#else
                strcat(info, " smiles at you.");
#endif
                break;
            case 2:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"죽은 후에 너는 내적 평화를 찾을 것이다.\"");
#else
                       " says, \"After death you'll find inner peace.\"");
#endif
                break;
            case 3:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"살아있는 것이 바로 괴로움이지. 내가 널 도와주마.\"");
#else
                       " says, \"Life is just suffering. I'll help you.\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 평화의 오라로 덮혔다.");
#else
                strcat(info, " is surrounded with aura of peace.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 매우 평온해 보인다.");
#else
                strcat(info, " looks very balanced.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 말했다. \"저항하지마라. 난 널 돕는거야.\"");
#else
                strcat(info, " says, \"Don't resist. I'll do it for you.\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"이제 열반에 들어가랏!\"");
#else
                strcat(info, " screams, \"Enter NIRVANA! Now!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 말했다. \"죽음이야말로 진정한 해탈이지!\"");
#else
                strcat(info, " says, \"Death is just a liberation!\"");
#endif
                break;
            case 9:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"죽음으로 자유를 느껴봐. 굉장한 것이지.\"");
#else
                       " says, \"Feel free to die. It's great thing.\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 말했다. \"옴마니반메훔\"");
#else
                strcat(info, " says, \"OHM MANI PADME HUM!\"");
#endif
                break;

            case 11:
#ifdef JP
                strcat(info, "은(는) 진언을 중얼 거렸다.");
#else
                strcat(info, " mumbles some mantras.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 12:
#ifdef JP
                strcat(info, "은(는) 말했다. \"호흡은 깊숙하게 하라.\"");
#else
                strcat(info, " says, \"Breath deeply.\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"사랑! 영원한 사랑!\"");
#else
                strcat(info, " screams, \"Love! Eternal love!\"");
#endif
                break;
            case 14:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"평화! 나가 너를 영원한 평화로 인도하겠다!\"");
#else
                       " screams, \"Peace! I bring you eternal peace!\"");
#endif
                break;
            case 15:
                strcat(info,
#ifdef JP
                       "은(는) 탄식했다. \"계발은 일종의 책임이다.\"");
#else
                       " sighs \"Enlightenment is such responsibility.\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "은(는) 평정심을 찾은 것 같다.");
#else
                strcat(info, " looks relaxed.");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"너의 영혼을 자유롭게 하라! 죽어랏!\"");
#else
                strcat(info, " screams, \"Free your soul! Die!\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"번뇌를 날려버려라!\"");
#else
                strcat(info, " screams, \"Blow your mind!\"");
#endif
                break;
            case 19:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"오브는 단지 전설일 뿐이다. 잊도록 해라.\"");
#else
                       " says, \"The Orb is only a myth. Forget about it.\"");
#endif
                break;
            case 20:
#ifdef JP
                strcat(info, "은(는) 말했다. \"세상 모든 것은 환상이다.\"");
#else
                strcat(info, " says, \"It's all maya.\"");
#endif
                break;
            case 21:
#ifdef JP
                strcat(info, "은(는) 말했다. \"해탈하라!\"");
#else
                strcat(info, " says, \"Drop out!\"");
#endif
                break;
            case 22:
                strcat(info,
#ifdef JP
                       "은(는) 노래했다. \"이제는 평화를, 이제는 자유를, 이제는 평화를, 이제는 자유를!\"");
#else
                       " sings, \"Peace now, freedom now! Peace now, freedom now!\"");
#endif
                break;
            case 23:
#ifdef JP
                strcat(info, "은(는) 말했다. \"이것이 전투명상 이라 불리는 것이다.\"");
#else
                strcat(info, " says, \"This is called Combat Meditation.\"");
#endif
                break;
            }
            break;          // end Norris

        case MONS_MARGERY:  // powerful sorceress, guarding the ORB
            switch (random2(22))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 말했다. \"당신은 죽었습니다.\"");
#else
                strcat(info, " says, \"You are dead.\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 매우 자아만족으로 가득찬 모습이다.");
#else
                strcat(info, " looks very self-confident.");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"당신은 처벌받아야 한다!\"");
#else
                strcat(info, " screams, \"You must be punished!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"넌 나의 힘에 맞겨룰 수 없다!\"");
#else
                strcat(info, " screams, \"You can't withstand my power!\"");
#endif
                break;

            case 4:
#ifdef JP
                simple_monster_message( monster, "은(는) 주문을 외웠다.",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "은(는) 마력의 오라에 쌓였다.");
#else
                strcat(info, " is surrounded with aura of power.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 5:
#ifdef JP
                strcat(info, "의 눈은 붉은 빛이 나기 시작했다.");
#else
                strcat(info, "'s eyes starts to glow with a red light.");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "의 눈은 초록 빛이 나기 시작했다.");
#else
                       "'s eyes starts to glow with a green light.");
#endif
                    break;
            case 7:
#ifdef JP
                strcat(info, "의 눈은 파란 빛이 나기 시작했다.");
#else
                strcat(info, "'s eyes starts to glow with a blue light.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"모든 침입자를 죽여라!\"");
#else
                strcat(info, " screams, \"All trespassers must die!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 말했다. \"죽어랏!\"");
#else
                strcat(info, " says, \"Die!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"넌 날를 통과시켜 줘야만 한다!\"");
#else
                strcat(info, " screams, \"You'll have to get past me!\"");
#endif
                break;

            case 11:
#ifdef JP
                simple_monster_message( monster, "은(는) 주문을 외웠다.",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "은(는) 잠시 투명해졌다.");
#else
                strcat(info, " becomes transparent for a moment.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 12:
#ifdef JP
                strcat(info, "은(는) 손짓했다.");
#else
                strcat(info, " gestures.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 13:
#ifdef JP
                simple_monster_message( monster, "은(는) 주문을 외웠다.",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "의 손이 빛나기 시작했다.");
#else
                strcat(info, "'s hands start to glow.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 14:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"에르기찬테그 레즈타하!\"");
#else
                strcat(info, " screams, \"Ergichanteg reztahaw!\"");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신은 기분이 매우 나빠졌다.", MSGCH_WARN);
#else
                mpr("You feel really bad.", MSGCH_WARN);
#endif
                return (true);

            case 15:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"당신은 운명은 끝났습니다.\"");
#else
                strcat(info, " screams, \"You are doomed!\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"널 도와줄 것은 없다.\"");
#else
                strcat(info, " screams, \"Nothing can help you.\"");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"사신, 나의 또다른 이름이지!\"");
#else
                strcat(info, " screams, \"Death is my middle name!\"");
#endif
                break;

            case 18:
#ifdef JP
                strcat(info, "은(는) 손짓했다.");
#else
                strcat(info, " gestures.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신은 절망을 느겼다.", MSGCH_WARN);
#else
                mpr("You feel doomed.", MSGCH_WARN);
#endif
                return (true);

            case 19:
#ifdef JP
                strcat(info, "은(는) 손짓했다.");
#else
                strcat(info, " gestures.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신은 약해진 것 같다.", MSGCH_WARN);
#else
                mpr("You feel weakened.", MSGCH_WARN);
#endif
                return (true);

            case 20:
#ifdef JP
                strcat(info, "은(는) 당신을 향해 보라색 가루를 던졌다.");
#else
                strcat(info, " throws some purple powder towards you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신은 저주에 걸린 것 같다.", MSGCH_WARN);
#else
                mpr("You feel cursed.", MSGCH_WARN);
#endif
                return (true);

            case 21:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"오브는 옛날 이야기야, 하지만 난 널 죽여버리겠다!\"");
#else
                       " screams, \"The ORB is only a tale, but I will kill you anyway!");
#endif
                break;
            }
            break;          // end Margery

        case MONS_IJYB:     // twisted goblin
            switch (random2(14))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어랏!\"");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"나 널 죽이겠다!\"");
#else
                strcat(info, " screams, \"Me kill you!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"나 너보다 강해!\"");
#else
                strcat(info, " screams, \"Me stronger than you!\"");
#endif
                break;
            case 3:
            case 4:
#ifdef JP
                strcat(info, "은(는) 사악한 미소를 띄웠다.");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"그것은 전부 내것이야!\"");
#else
                strcat(info, " screams, \"It's all mine!\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"꺼져!\"");
#else
                strcat(info, " screams, \"Get away!\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"이 층은 내것이다! 모두 내것이다!\"");
#else
                strcat(info, " screams, \"Level is mine! All mine!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"너의 머리를 자르겠다!\"");
#else
                strcat(info, " screams, \"I cut your head off!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"난 너의 뼈 위에서 춤추겠다!\"");
#else
                strcat(info, " screams, \"I dance on your bones!\"");
#endif
                    break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"나 몹시 흥분!\"");
#else
                strcat(info, " screams, \"Me very upset!\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"너 더러워! 아주 더러워!\"");
#else
                strcat(info, " screams, \"You nasty! Big nasty!\"");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"안돼! 안돼! 안돼! 안돼!\"");
#else
                strcat(info, " screams, \"No! No, no, no, no!\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"난 널 좋아하지 않아!\"");
#else
                strcat(info, " screams, \"I no like you!\"");
#endif
                break;
            }
            break;          // end IJYB

        case MONS_BLORK_THE_ORC:    // unfriendly orc
            switch (random2(21))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"너 같은 건 싫어!\"");
#else
                strcat(info, " screams, \"I don't like you!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"널 죽여버리겠다!\"");
#else
                strcat(info, " screams, \"I'm going to kill you!\"");
#endif
                break;
            case 2:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"난 너보다 훨씬 강하다!\"");
#else
                       " screams, \"I'm much stronger than you!\"");
#endif
                break;
            case 3:
            case 4:
#ifdef JP
                strcat(info, "은(는) 사악한 미소를 띄웠다.");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 찡그렸다.");
#else
                strcat(info, " frowns.");
#endif
                break;
            case 6:
            case 7:
            case 8:
            case 9:
#ifdef JP
                strcat(info, "은(는) 화가난 것 같다.");
#else
                strcat(info, " looks angry.");
#endif
                break;
            case 10:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"너의 뇌를 먹어버릴 것이다! 그리고 도로 토해내겠어!");
#else
                       " screams, \"I'll eat your brain! And then I'll vomit it back up!\"");
#endif
                break;
            case 11:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"넌 내가 본 것중에 제일 못생긴 피조물이군!\"");
#else
                       " screams, \"You are the ugliest creature I've ever seen!\"");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"너의 머리를 잘라 내겠다!\"");
#else
                strcat(info, " screams, \"I'll cut your head off!\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"네 다리를 부숴버리겠다!\"");
#else
                strcat(info, " screams, \"I'll break your legs!\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"네 팔을 부러뜨리겠다!\"");
#else
                strcat(info, " screams, \"I'll break your arms!\"");
#endif
                break;
            case 15:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"너의 갈비뼈 하나 하나를 모두 부숴주지!\"");
#else
                       " screams, \"I'll crush all your ribs! One by one!\"");
#endif
                break;
            case 16:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"너의 껍질로 망토를 만들어 입겠다!\"");
#else
                       " screams, \"I'll make a cloak from your skin!\"");
#endif
                    break;
            case 17:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"너의 내장으로 내 집을 장식하겠다!\"");
#else
                       " screams, \"I'll decorate my home with your organs!\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"죽어랏!\"");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 19:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"너의 피로 던젼을 뒤덮겠다!\"");
#else
                       " screams, \"I'll cover the dungeon with your blood!\"");
#endif
                break;
            case 20:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"조만간 너의 피를 마셔버리겠다!\"");
#else
                strcat(info, " screams, \"I'll drink your blood! Soon!\"");
#endif
                break;
            }
            break;          // end Blork

        case MONS_EROLCHA:  // ugly ogre
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 사악한 미소를 띄우려고 노력했다.");
#else
                strcat(info, " tries to grin evilly.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"먹겠다!\"");
#else
                strcat(info, " screams, \"Eat!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"서랏! 에롤차가 널 때린다!\"");
#else
                strcat(info, " screams, \"Stand! Erolcha hit you!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"피!\"");
#else
                strcat(info, " screams, \"Blood!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"에롤차는 널 죽인다!\"");
#else
                strcat(info, " screams, \"Erolcha kill you!\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "은(는) 부르짖었다. \"에롤차는 너의 머리를 부숴버린다.\"");
#else
                       " screams, \"Erolcha crush your head!\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 사납게 울부짖었다.");
#else
                strcat(info, " roars.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 그르릉 울리고 있다.");
#else
                strcat(info, " growls.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"점심 식사!\"");
#else
                strcat(info, " screams, \"Lunch!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"에롤차는 널 죽일 수 있어 행복해!\"");
#else
                strcat(info, " screams, \"Erolcha happy to kill you!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 부르짖었다. \"에롤차는 화났다!\"");
#else
                strcat(info, " screams, \"Erolcha angry!\"");
#endif
                break;
            }
            break;          // end Erolcha

        case MONS_URUG:     // orc hired to kill you
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 사악한 미소를 지었다.");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 소리쳤다. \"죽어랏!\"");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 소리쳤다. \"당장 널 죽이고야 말겠다!\"");
#else
                strcat(info, " screams, \"I'm going to kill you! Now!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 소리쳤다. \"피와 살육을!\"");
#else
                strcat(info, " screams, \"Blood and destruction!\"");
#endif
                break;
            case 4:
                strcat(info,
#ifdef JP
                       "은(는) 비웃었다. \"죄가 없다고? 널 죽여버리겠다.\"");
#else
                       " sneers, \"Innocent? I'll kill you anyway.\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "은(는) 소리쳤다. \"네 머리값으로 30실버를 받아내겠다!\"");
#else
                       " screams, \"I'll get 30 silver pieces for your head!\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 표호했다.");
#else
                strcat(info, " roars.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 피에 굶주려 울부짖었다.");
#else
                strcat(info, " howls with blood-lust.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 소리쳤다. \"넌 이미 죽은 목숨이다.\"");
#else
                strcat(info, " screams, \"You are already dead.\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 말했다. \"너는 [");
#else
                strcat(info, " says, \"Maybe you aren't ");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "]이(가) 아닐지도... 그런건 문제가 안된다.\"");
#else
                strcat(info, ". It doesn't matter.\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 소리쳤다. \"난 피보기를 좋아한다!\"");
#else
                strcat(info, " screams, \"I love blood!\"");
#endif
                break;
            }
            break;          // end Urug

        case MONS_SNORG:    // troll
            switch (random2(16))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 이빨을 드러냈다.");
#else
                strcat(info, " grins.");
#endif
                break;
            case 1:
            case 2:
            case 3:
#ifdef JP
                strcat(info, "은(는) 끔찍한 냄새를 풍긴다.");
#else
                strcat(info, " smells terrible.");
#endif
                break;
            case 4:
            case 5:
            case 6:
#ifdef JP
                strcat(info, "은(는) 몹시 배고파 보인다.");
#else
                strcat(info, " looks very hungry.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 소리쳤다. \"간식이로군!\"");
#else
                strcat(info, " screams, \"Snack!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 울부짖었다.");
#else
                strcat(info, " roars.");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 말했다. \"밥이 왔다!\"");
#else
                strcat(info, " says, \"Food!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 소리쳤다. \"스노그는 배고파!\"");
#else
                strcat(info, " screams, \"Snorg hungry!\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "은(는) 외쳤다. \"스노그는 매우 몹시 배고파!\"");
#else
                strcat(info, " screams, \"Snorg very, very hungry!\"");
#endif
            case 12:
#ifdef JP
                strcat(info, "은(는) 말했다. \"스노그는 널 먹는다.\"");
#else
                strcat(info, " says, \"Snorg eat you.\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "은(는) 말했다. \"너, 먹어도 되는 거지?\"");
#else
                strcat(info, " says, \"You food?\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "은(는) 말했다. \"냠냠 맛있다.\"");
#else
                strcat(info, " says, \"Yum, yum.\"");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "은(는) 트림을 했다.");
#else
                strcat(info, " burps.");
#endif
                break;
            }
            break;          // end Snorg

        case MONS_XTAHUA:   // ancient dragon
            switch (random2(13))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 울부짖었다. \"죽어랏, 하잘 것 없는 녀석!\"");
#else
                strcat(info, " roars, \"DIE,  PUNY ONE!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 신음했다. \"넌 이제 지긋지긋해.\"");
#else
                strcat(info, " growls, \"YOU BORE ME SO.\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 불평했다. \"넌 겨우 간식거리밖에 안되.\"");
#else
                strcat(info, " rumbles, \"YOU'RE BARELY A SNACK.\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 울부짖었다. \"난 귀찮은건 딱 질색이야!\"");
#else
                strcat(info, " roars, \"I HATE BEING BOTHERED!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 울부짖었다. \"난 니가 맛있기를 바란다!\"");
#else
                strcat(info, " roars, \"I HOPE YOU'RE TASTY!\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 울부짖었다. \"흥! 잔인한 모험자.\"");
#else
                strcat(info, " roars, \"BAH!  BLOODY ADVENTURERS.\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 울부짖었다. \"나를 화나게 만드는군!\"");
#else
                strcat(info, " roars, \"FACE MY WRATH!\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 당신을 노려보았다.");
#else
                strcat(info, " glares at you.");
#endif
                break;
            case 8:
                strcat(info,
#ifdef JP
                       "은(는) 울부짖었다. \"여기에 온 것 자체가 너의 운명이 다했다는 것이다!\"");
#else
                       " roars, \"COMING HERE WAS YOUR LAST MISTAKE!\"");
#endif
                break;
            case 9:
                strcat(info,
#ifdef JP
                       "은(는) 울부짖었다. \"난 수백명의 모험가를 죽여왔다!\"");
#else
                       " roars, \"I'VE KILLED HUNDREDS OF ADVENTURERS!\"");
#endif
                break;
            case 10:
            case 11:
            case 12:
#ifdef JP
                strcat(info, "은(는) 무시무시하게 울부짖었다.");
#else
                strcat(info, " roars horribly.");
#endif
                mpr(info, MSGCH_TALK);
#ifdef JP
                mpr("당신은 무서워졌다.", MSGCH_WARN);
#else
                mpr("You are afraid.", MSGCH_WARN);
#endif
                return (true);
            }
            break;          // end Xtahua

        case MONS_BORIS:    // ancient lich
            switch (random2(24))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 말했다. \"난 널 초대한적이 없어.\"");
#else
                strcat(info, " says, \"I didn't invite you.\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 말했다. \"넌 나의 힘을 상상하지도 못한다.\"");
#else
                strcat(info, " says, \"You can't imagine my power.\"");
#endif
                break;
            case 2:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"오브? 넌 오브를 원하는 건가? 결코 가질 수 없는 것일텐데.\"");
#else
                       " says, \"Orb? You want the Orb? You'll never get it.\"");
#endif
                break;

            case 3:
#ifdef JP
                strcat(info, "은(는) 말했다. \"이 세상, 육체, 그리고 악마.\"");
#else
                strcat(info, " says, \"The world, the flesh, and the devil.\"");
#endif
                break;

            case 4:
#ifdef JP
                strcat(info, "은(는) 손을 내저었다.");
#else
                strcat(info, " gestures.");
#endif
                break;

            case 5:
#ifdef JP
                strcat(info, "은(는) 당신을 응시했다.");
#else
                strcat(info, " stares at you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신은 생명이 흡수당한 것을 느꼈다.", MSGCH_WARN);
#else
                mpr("You feel drained.", MSGCH_WARN);
#endif
                return (true);

            case 6:
#ifdef JP
                strcat(info, "은(는) 당신을 응시했다.");
#else
                strcat(info, " stares at you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신은 약해진 것 같다.", MSGCH_WARN);
#else
                mpr("You feel weakened.", MSGCH_WARN);
#endif
                return (true);

            case 7:
#ifdef JP
                strcat(info, "은(는) 당신을 응시했다.");
#else
                strcat(info, " stares at you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신은 난처함을 느겼다.", MSGCH_WARN);
#else
                mpr("You feel troubled.", MSGCH_WARN);
#endif
                return (true);

            case 8:
#ifdef JP
                strcat(info, "은(는) 말했다. \"마법. 넌 그것에 대해 아는게 없어.\"");
#else
                strcat(info, " says \"Magic. You know nothing about it.\"");
#endif
                break;

            case 9:
#ifdef JP
                strcat(info, "은(는) 말했다. \"나의 힘은 무한하다.\"");
#else
                strcat(info, " says, \"My power is unlimited.\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "은(는) 말했다. \"넌 날 죽일 수 없어. 난 불사신 이니까.\"");
#else
                strcat(info, " says, \"You can't kill me. I'm immortal.\"");
#endif
                break;

            case 11:
#ifdef JP
                strcat(info, "은(는) 마법 주문을 외웠다.");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신의 장비가 갑자기 무거워진 것 같다.", MSGCH_WARN);
#else
                mpr("Your equipment suddenly seems to weigh more.", MSGCH_WARN);
#endif
                return (true);

            case 12:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"나는 영생의 비밀을 알고있다. 너도 아는 건가?\"");
#else
                       " says, \"I know the secret of eternal life.  Do you?\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "은(는) 말했다. \"다시 돌아오겠다.\"");
#else
                strcat(info, " says, \"I'll be back.\"");
#endif
                break;

            case 14:
#ifdef JP
                strcat(info, "은(는) 주문을 외웠다.");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
                canned_msg( MSG_YOU_RESIST );
                return (true);

            case 15:
#ifdef JP
                strcat(info, "은(는) 주문을 외웠다.");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신은 갑자기 희미한 초록빛에 둘러쌓였다.", MSGCH_WARN);
#else
                mpr("Suddenly you are surrounded with pale green light.", MSGCH_WARN);
#endif
                return (true);

            case 16:
#ifdef JP
                strcat(info, "은(는) 주문을 외웠다.");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("당신은 끔찍한 두통에 빠졌다.", MSGCH_WARN);
#else
                mpr("You have terrible head-ache.", MSGCH_WARN);
#endif
                return (true);

            case 17:
                strcat(info,
#ifdef JP
                       "은(는) 말했다. \"난 너의 미래를 알지. 너의 미래는 바로 죽음이다.\"");
#else
                       " says, \"I know your future. Your future is death.\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "은(는) 말했다. \"영원한 삶을 바라는 자가 누군지 알겠는가? 바로 나다.\"");
#else
                strcat(info, " says, \"Who wants to live forever?  Me.\"");
#endif
                break;
            case 19:
#ifdef JP
                strcat(info, "은(는) 소리내어 웃었다.");
#else
                strcat(info, " laughs.");
#endif
                break;
            case 20:
#ifdef JP
                strcat(info, "은(는) 말했다. \"나의 노예 군사에 합류하라.\"");
#else
                strcat(info, " says, \"Join the legion of my servants.\"");
#endif
                break;
            case 21:
#ifdef JP
                strcat(info, "은(는) 말했다. \"너에겐 오직 한가지 길 뿐이다. 바로 죽음이지.\"");
#else
                strcat(info, " says, \"There's only one solution for you. To die.\"");
#endif
                break;
            case 22:
#ifdef JP
                strcat(info, "은(는) 말했다. \"넌 결코 이길 수 없다.\"");
#else
                strcat(info, " says, \"You can never win.\"");
#endif
                break;
            case 23:
#ifdef JP
                simple_monster_message( monster, "은(는) 주문을 외웠다.",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat( info, "은(는) 빨라졌다." );
#else
                strcat( info, " speeds up." );
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;
            }
            break;          // end BORIS


        case MONS_DEATH_COB:
            if (one_chance_in(2000))
            {
#ifdef JP
                mpr("데스 콥은 썰렁한 농담만 했다.", MSGCH_TALK);
#else
                mpr("The death cob makes a corny joke.", MSGCH_TALK);
#endif
                return (true);
            }
            return (false);

        case MONS_KILLER_KLOWN:     // Killer Klown - guess!
            switch (random2(10))
            {
            case 0:
#ifdef JP
                strcat(info, "은(는) 미친듯이 킥킥거리며 웃었다.");
#else
                strcat(info, " giggles crazily.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "은(는) 유쾌하게 웃었다.");
#else
                strcat(info, " laughs merrily.");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "은(는) 당신을 유인했다.");
#else
                strcat(info, " beckons to you.");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "은(는) 재주를 넘었다.");
#else
                strcat(info, " does a flip.");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "은(는) 공중 회전을 했다.");
#else
                strcat(info, " does a somersault.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "은(는) 당신에게 미소 지었다.");
#else
                strcat(info, " smiles at you.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "은(는) 멋적은듯이 이빨을 드러내고 웃었다.");
#else
                strcat(info, " grins with merry abandon.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "은(는) 피에 굶주린듯이 울었다!");
#else
                strcat(info, " howls with blood-lust!");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "은(는) 혀를 쑥 내밀었다.");
#else
                strcat(info, " pokes out its tongue.");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "은(는) 말했다. \"이리와 나와 놀자!\"");
#else
                strcat(info, " says, \"Come and play with me!\"");
#endif
                break;
            }
            break;          // end Killer Klown

        default:
            strcat(info,
#ifdef JP
                   "은(는) 말했다. \"내가 무슨말을 해야할지 모르겠다. 이건 버그야.\"");
#else
                   " says, \"I don't know what to say. It's a bug.\"");
#endif
            break;
        }                   // end monster->type - monster type switch
    }                       // end default

    mpr(info, msg_type);
    return true;
}                               // end mons_speaks = end of routine
