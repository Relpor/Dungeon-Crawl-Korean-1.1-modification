/*
 *  File:       mutation.cc
 *  Summary:    Functions for handling player mutations.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <5>      7/29/00        JDJ             Made give_cosmetic_mutation static
 *      <4>      9/25/99        CDL             linuxlib -> liblinux
 *      <3>      9/21/99        LRH             Added many new scales
 *      <2>      5/20/99        BWR             Fixed it so demonspwan should
 *                                              always get a mutation, 3 level
 *                                              perma_mutations now work.
 *      <1>      -/--/--        LRH             Created
 */

#include "AppHdr.h"
#include "mutation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DOS
#include <conio.h>
#endif

//#ifdef LINUX
//#include "liblinux.h"
//#endif

#include "externs.h"

#include "defines.h"
#include "effects.h"
#include "macro.h"
#include "ouch.h"
#include "player.h"
#include "skills2.h"
#include "stuff.h"
#include "transfor.h"
#include "view.h"

int how_mutated(void);
char body_covered(void);
bool perma_mutate(int which_mut, char how_much);

const char *mutation_descrip[][3] = {
#ifdef JP 
    {"당신은 강인한 피부를 가지고 있다.(AC +1)", "당신은 매우 강인한 피부를 가지고 있다.(AC +2)",
     "당신은 엄청나게 강인한 피부를 가지고 있다.(AC +3)"},
#else
    {"You have tough skin (AC +1).", "You have very tough skin (AC +2).",
     "You have extremely tough skin (AC +3)."},
#endif

#ifdef JP 
    {"당신의 근육은 강하다.(STR +", "", ""},
    {"당신의 정신은 날카롭다.(INT +", "", ""},
    {"당신은 민첩하다.(DEX +", "", ""},
#else
    {"Your muscles are strong (Str +", "", ""},
    {"Your mind is acute (Int +", "", ""},
    {"You are agile (Dex +", "", ""},
#endif

#ifdef JP 
    {"당신은 녹색의 비늘로 부분적으로 덮여 있다.(AC +1)",
     "당신은 녹색의 비늘로 대부분 덮여 있다.(AC +3)",
     "당신이 녹색의 비늘로 완전하게 덮여 있다.(AC +5)"},
#else
    {"You are partially covered in green scales (AC + 1).",
     "You are mostly covered in green scales (AC + 3).",
     "You are covered in green scales (AC + 5)."},
#endif

#ifdef JP 
    {"당신은 두께가 있는 검은 비늘로 부분적으로 덮여 있다.(AC +3, DEX -1)",
     "당신은 두께가 있는 검은 비늘로 대부분 덮여 있다.(AC +6, DEX -2)",
     "당신은 두께가 있는 검은 비늘로 완전하게 덮여 있다.(AC +9, DEX -3)"},
#else
    {"You are partially covered in thick black scales (AC + 3, dex - 1).",
     "You are mostly covered in thick black scales (AC + 6, dex - 2).",
     "You are completely covered in thick black scales (AC + 9, dex - 3)."},
#endif

#ifdef JP 
    {"당신은 유연한 회색의 비늘로 부분적으로 덮여 있다.(AC +1)",
     "당신은 유연한 회색의 비늘로 대부분 덮여 있다.(AC +2)",
     "당신은 유연한 회색의 비늘로 완전하게 덮여 있다.(AC +3)"},
#else
    {"You are partially covered in supple grey scales (AC + 1).",
     "You are mostly covered in supple grey scales (AC + 2).",
     "You are completely covered in supple grey scales (AC + 3)."},
#endif

#ifdef JP 
    {"당신은 뼈의 장갑으로 지켜지고 있다.(AC +2, DEX -1)",
     "당신은 뼈의 장갑으로 지켜지고 있다.(AC +3, DEX -2)",
     "당신은 뼈의 장갑으로 지켜지고 있다.(AC +4, DEX -3)"},
#else
    {"You are protected by plates of bone (AC + 2, dex - 1).",
     "You are protected by plates of bone (AC + 3, dex - 2).",
     "You are protected by plates of bone (AC + 4, dex - 3)."},
#endif

#ifdef JP 
    {"당신은 완만한 반발 필드에 덮여 있다.(EV +1)",
     "당신은 반발 필드에 덮여 있다.(EV +3)",
     "당신은 강력한 반발 필드에 덮여 있다.(EV +5, 미사일류 방어)"},
    {"당신의 육체는 독에 면역이 있다.", "당신의 육체는 독에 면역이 있다.",
     "당신의 육체는 독에 면역이 있다."},
#else
    {"You are surrounded by a mild repulsion field (ev + 1).",
     "You are surrounded by a moderate repulsion field (ev + 3).",
     "You are surrounded by a strong repulsion field (ev + 5; repel missiles)."},
    {"Your system is immune to poisons.", "Your system is immune to poisons.",
     "Your system is immune to poisons."},
#endif
// 10

#ifdef JP 
    {"당신의 소화기계통은 고기의 소화에 특화되어 있다.",
     "당신의 소화기계통은 고기의 소화에 특화되어 있다.",
     "당신은 본질적으로 육식성이다."},
#else
    {"Your digestive system is specialised to digest meat.",
     "Your digestive system is specialised to digest meat.",
     "You are primarily a carnivore."},
#endif

#ifdef JP 
    {"당신은 고기를 잘 소화할 수 없다.", "당신은 고기를 잘 소화할 수 없다.",
     "당신은 본질적으로 초식성이다."},
#else
    {"You digest meat inefficiently.", "You digest meat inefficiently.",
     "You are primarily a herbivore."},
#endif

#ifdef JP 
    {"당신의 피부는 열에 내성이 있다.", "당신의 피부는 열에 상당한 내성이 있다.",
     "당신의 피부는 열의 영향에 대해 거의 면역이다."},
#else
    {"Your flesh is heat resistant.", "Your flesh is very heat resistant.",
     "Your flesh is almost immune to the effects of heat."},
#endif

#ifdef JP 
    {"당신의 피부는 냉기에 내성이 있다.", "당신의 피부는 냉기에 상당한 내성이 있다.",
     "당신의 피부는 냉기의 영향에 대해 거의 면역이다."},
#else
    {"Your flesh is cold resistant.", "Your flesh is very cold resistant.",
     "Your flesh is almost immune to the effects of cold."},
#endif

#ifdef JP 
    {"당신은 전기 쇼크에 면역이 있다.", "당신은 전기 쇼크에 면역이 있다.",
     "당신은 전기 쇼크에 면역이 있다."},
#else
    {"You are immune to electric shocks.", "You are immune to electric shocks.",
     "You are immune to electric shocks."},
#endif

#ifdef JP 
    {"당신의 자연치유의 속도는 현저하게 빠르다.",
     "당신은 매우 빠르게 치유 된다.",
     "당신은 재생 능력이 있다."},
#else
    {"Your natural rate of healing is unusually fast.",
     "You heal very quickly.",
     "You regenerate."},
#endif

#ifdef JP 
    {"당신의 신진대사는 빠르다.", "당신의 신진대사는 매우 빠르다.",
     "당신의 신진대사는 터무니없이 빠르다."},
#else
    {"You have a fast metabolism.", "You have a very fast metabolism.",
     "Your metabolism is lightning-fast."},
#endif

#ifdef JP 
    {"당신의 신진대사는 늦다.", "당신의 신진대사는 늦다.",
     "당신은 식료품를 먹을 필요가 거의 없다."},
#else
    {"You have a slow metabolism.", "You have a slow metabolism.",
     "You need consume almost no food."},
#endif

#ifdef JP 
    {"당신은 약하다.(STR -", "", ""},
    {"당신은 어리석다.(INT -", "", ""},
#else
    {"You are weak (Str -", "", ""},
    {"You are dopey (Int -", "", ""},
#endif
// 20
#ifdef JP 
    {"당신은 서투르다.(DEX -", "", ""},
#else
    {"You are clumsy (Dex -", "", ""},
#endif

#ifdef JP 
    {"당신은 공간전위를 제어할 수가 있다.", "당신은 공간전위를 제어할 수가 있다.",
     "당신은 공간전위를 제어할 수가 있다."},
#else
    {"You can control translocations.", "You can control translocations.",
     "You can control translocations."},
#endif

#ifdef JP 
    {"당신의 근처에서 이따금 공간이 비뚤어진다.",
     "당신의 근처에서 가끔 공간이 비뚤어진다.",
     "당신의 근처에서 빈번하게 공간이 비뚤어진다."},
#else
    {"Space occasionally distorts in your vicinity.",
     "Space sometimes distorts in your vicinity.",
     "Space frequently distorts in your vicinity."},
#endif

#ifdef JP 
    {"당신은 마법에 대하여 저항력이 있다.", "당신은 마법에 대해 높은 저항력이 있다.",
     "당신은 마법에 대하여 최상의 저항력이 있다."},
#else
    {"You are resistant to magic.", "You are highly resistant to magic.",
     "You are extremely resistant to the effects of magic."},
#endif

#ifdef JP 
    {"당신은 지면을 빠르게 움직인다.", "당신은 지면을 매우 빠르게 움직인다.",
     "당신은 지면을 엄청난 속도로 빠르게 움직인다."},
#else
    {"You cover the ground quickly.", "You cover the ground very quickly.",
     "You cover the ground extremely quickly."},
#endif

#ifdef JP 
    {"당신은 초자연적인 날카로운 시력을 가지고 있다.",
     "당신은 초자연적인 날카로운 시력을 가지고 있다.",
     "당신은 초자연적인 날카로운 시력을 가지고 있다."},
#else
    {"You have supernaturally acute eyesight.",
     "You have supernaturally acute eyesight.",
     "You have supernaturally acute eyesight."},
#endif

#ifdef JP 
    {"기형적인 몸 때문에 갑옷이 맞지 않는다.",
     "심각한 기형적인 몸 때문에 갑옷이 맞지 않는다.",
     "극심한 기형적인 몸 때문에 갑옷이 맞지 않는다."},
#else
    {"Armour fits poorly on your deformed body.",
     "Armour fits poorly on your badly deformed body.",
     "Armour fits poorly on your hideously deformed body."},
#endif

#ifdef JP 
    {"당신은 공간이동 능력이 있다.", "당신은 능숙한 공간이동 능력이 있다.",
     "당신은 순간이동 능력이 있다."},
#else
    {"You can teleport at will.", "You are good at teleporting at will.",
     "You can teleport instantly at will."},
#endif

#ifdef JP 
    {"당신은 독을 뱉을 수 있다.", "당신은 독을 뱉을 수 있다.", "당신은 독을 뱉을 수 있다."},
#else
    {"You can spit poison.", "You can spit poison.", "You can spit poison."},
#endif

#ifdef JP 
    {"당신은 좁은 범위의 주변을 감지할 수 있다.",
     "당신은 주변을 감지할 수 있다.",
     "당신은 광범위한 주변을 감지할 수 있다."},
#else
    {"You can sense your immediate surroundings.",
     "You can sense your surroundings.",
     "You can sense a large area of your surroundings."},
#endif

// 30

#ifdef JP 
    {"당신은 불꽃 브레스를 뿜을 수 있다.", "당신은 화염 브레스를 뿜을 수 있다.",
     "당신은 강렬한 화염 브레스를 뿜을 수 있다."},
#else
    {"You can breathe flames.", "You can breathe fire.",
     "You can breathe blasts of fire."},
#endif

#ifdef JP 
    {"당신은 짦은 거리의 순간이동이 가능하다.",
     "당신은 짦은 거리의 순간이동이 가능하다.",
     "당신은 짦은 거리의 순간이동이 가능하다."},
#else
    {"You can translocate small distances instantaneously.",
     "You can translocate small distances instantaneously.",
     "You can translocate small distances instantaneously."},
#endif

#ifdef JP 
    {"당신의 머리에는 한 쌍의 조그마한 뿔이 있다.",
     "당신의 머리에는 한 쌍의 뿔이 있다.",
     "당신의 머리에는 한 쌍의 커다란 뿔이 있다."},
#else
    {"You have a pair of small horns on your head.",
     "You have a pair of horns on your head.",
     "You have a pair of large horns on your head."},
#endif

#ifdef JP 
    {"당신의 근육은 강하다(STR +1) 그러나 뻣뻣하다(DEX -1)",
     "당신의 근육은 매우 강하다(STR +2) 그러나 뻣뻣하다(DEX -2)",
     "당신의 근육은 엄청나게 강하다(STR +3) 그러나 뻣뻣하다(DEX -3)"},
#else
    {"Your muscles are strong (Str +1), but stiff (Dex -1).",
     "Your muscles are very strong (Str +2), but stiff (Dex -2).",
     "Your muscles are extremely strong (Str +3), but stiff (Dex -3)."},
#endif

#ifdef JP 
    {"당신의 근육은 유연하다(DEX +1) 그러나 약하다(STR -1)",
     "당신의 근육은 매우 유연하다(DEX +2) 그러나 약하다(STR -2)",
     "당신의 근육은 엄청나게 유연하다(DEX +3) 그러나 약하다(STR -3)"},
#else
    {"Your muscles are flexible (Dex +1), but weak (Str -1).",
     "Your muscles are very flexible (Dex +2), but weak (Str -2).",
     "Your muscles are extremely flexible (Dex +3), but weak (Str -3)."},
#endif

#ifdef JP 
    {"당신은 매우 가끔 자신이 있는 장소를 잊어버린다.",
     "당신은 가끔 자신이 있는 장소를 잊어버린다.",
     "당신은 자주 자신이 있는 장소를 잊어버린다."},
#else
    {"You occasionally forget where you are.",
     "You sometimes forget where you are.",
     "You frequently forget where you are."},
#endif

#ifdef JP 
     {"당신은 매우 명석한 정신을 지녔다.",
      "당신은 부자연스러울 정도로 명석한 정신을 지녔다.",
      "당신은 초자연적인 명석한 정신을 지녔다."},
#else
    {"You possess an exceptional clarity of mind.",
     "You possess an unnatural clarity of mind.",
     "You possess a supernatural clarity of mind."},
#endif

#ifdef JP 
    {"당신은 전투중에 이성을 잃는 경향이 있다.",
     "당신은 전투중에 자주 이성을 잃는다.",
     "당신은 본능을 억제할 수 없다."},
#else
    {"You tend to lose your temper in combat.",
     "You often lose your temper in combat.",
     "You have an uncontrollable temper."},
#endif

#ifdef JP 
    {"당신의 육체는 서서히 능력이 저하되어간다.", "당신의 육체는 능력이 저하되어 간다.",
     "당신의 육체는 급속하게 능력이 저하되어간다."},
#else
    {"Your body is slowly deteriorating.", "Your body is deteriorating.",
     "Your body is rapidly deteriorating."},
#endif

#ifdef JP 
    {"당신의 시야는 약간 흐릿하다.", "당신의 시야는 매우 흐릿하다.",
     "당신의 시야는 엄청나게 흐릿하다."},
#else
    {"Your vision is a little blurry.", "Your vision is quite blurry.",
     "Your vision is extremely blurry."},
#endif
// 40

#ifdef JP 
    {"당신은 돌연변이의 진행에 다소의 저항력이 있다.",
     "당신은 돌연변이의 진행과 제거 모두 약간의 저항력이 있다.",
     "당신의 돌연변이는 완료되었고 앞으로 돌연변이는 없다."},
#else
    {"You are somewhat resistant to further mutation.",
     "You are somewhat resistant to both further mutation and mutation removal.",
     "Your current mutations are irrevocably fixed, and you can mutate no more."},
#endif

#ifdef JP 
    {"당신은 허약하다(-10% HP).", "당신은 매우 허약하다(-20% HP).",
     "당신은 엄청나게 허약하다(-30% HP)."},
#else
    {"You are frail (-10 percent hp).", "You are very frail (-20 percent hp).",
     "You are extremely frail (-30 percent hp)."},
#endif

#ifdef JP 
    {"당신은 건강하다(+10% HP).",
     "당신은 매우 건강하다(+20% HP).",
     "당신은 엄청나게 건강하다(+30% HP)."},
#else
    {"You are robust (+10 percent hp).",
     "You are very robust (+20 percent hp).",
     "You are extremely robust (+30 percent hp)."},
#endif

#ifdef JP 
    {"당신은 사악한 고통에 면역성이 있다.", "", ""},
#else
    {"You are immune to unholy pain and torment.", "", ""},
#endif

#ifdef JP 
    {"당신은 음에너지에 저항력이 있다.",
     "당신은 음에너지에 꽤 저항력이 있다.",
     "당신은 음에너지에 완전 면역상태 이다."},
#else
    {"You resist negative energy.",
     "You are quite resistant to negative energy.",
     "You are immune to negative energy."},
#endif

    /* Use player_has_spell() to avoid duplication */
#ifdef JP 
    {"당신은 하급 악마를 원군으로 소환할 수 있다.", "", ""},
    {"당신은 악마를 원군으로 소환할 수 있다.", "", ""},
    {"당신은 헬파이어를 일으킬 수 있다.", "", ""},
    {"당신은 지옥의 고통을 불러 일으킬 수 있다.", "", ""},
    {"당신은 시체를 일으켜 부릴 수 있다.", "", ""},
#else
    {"You can summon minor demons to your aid.", "", ""},
    {"You can summon demons to your aid.", "", ""},
    {"You can hurl blasts of hellfire.", "", ""},
    {"You can call on the torments of Hell.", "", ""},
    {"You can raise the dead to walk for you.", "", ""},
#endif
// 50
#ifdef JP 
    {"당신은 악마를 조종할 수 있다.", "", ""},
    {"당신은 판데모니움에 들어갈 수 있다.(나올 수 있을지는 모르지만)", "", ""},
    {"당신은 죽음과 파괴로 부터 힘을 얻을 수 있다.", "", ""},
#else
    {"You can control demons.", "", ""},
    {"You can travel to (but not from) Pandemonium at will.", "", ""},
    {"You can draw strength from death and destruction.", "", ""},
#endif

    /* Not worshippers of Vehumet */
#ifdef JP 
    {"당신은 지옥으로부터 마법 에너지를 꺼내 채울 수 있다.", "", ""},
#else
    {"You can channel magical energy from Hell.", "", ""},
#endif

#ifdef JP 
    {"당신은 맨손 격투를 하면서 생명력을 흡수 할 수 있다.", "", ""},
#else
    {"You can drain life in unarmed combat.", "", ""},
#endif

    /* Not conjurers/worshippers of Makhleb */
#ifdef JP 
    {"당신은 게헨나의 불길을 뿌릴 수 있다.", "", ""},
#else
    {"You can throw forth the flames of Gehenna.", "", ""},
#endif

#ifdef JP 
    {"당신은 코큐투스의 냉기를 뿌릴 수 있다.", "", ""},
#else
    {"You can throw forth the frost of Cocytus.", "", ""},
#endif

#ifdef JP 
    {"당신은 타르타루스의 힘을 발동시켜 살아있는 적을 타격할 수 있다.", "", ""},
    {"당신의 손톱은 날카롭다.", "당신의 손톱은 매우 날카롭다.",
     "당신의 손톱은 갈퀴이다."},
#else
    {"You can invoke the powers of Tartarus to smite your living foes.", "", ""},
    {"You have sharp fingernails.", "Your fingernails are very sharp.",
     "You have claws for hands."},
#endif

#ifdef JP 
    {"당신은 발에 발굽이 있다.", "", ""},
#else
    {"You have hooves in place of feet.", "", ""},
#endif
    // 60 - leave some space for more demonic powers...
#ifdef JP 
    {"당신은 독구름을 뿜을 수 있다.", "", ""},
#else
    {"You can exhale a cloud of poison.", "", ""},
#endif

#ifdef JP 
    {"당신의 꼬리의 끝에는 독이 있는 가시가 나 있다.",
     "당신의 꼬리의 끝에는 독이 있는 날카로운 가시가 나 있다.",
     "당신의 꼬리의 끝에는 독이 있는 흉악한 가시가 나 있다."}, //jmf: nagas & dracos
#else
    {"Your tail ends in a poisonous barb.",
     "Your tail ends in a sharp poisonous barb.",
     "Your tail ends in a wicked poisonous barb."}, //jmf: nagas & dracos
#endif

#ifdef JP 
    {"당신의 날개는 크고 강력하다.", "", ""},       //jmf: dracos only
#else
    {"Your wings are large and strong.", "", ""},       //jmf: dracos only
#endif

    //jmf: these next two are for evil gods to mark their followers; good gods
    //     will never accept a 'marked' worhsipper

#ifdef JP 
    {"당신의 양손에는 푸른 인장이 새겨져 있다.",
     "당신의 손으로부터 팔에는 몇개인가 푸른 인장이 새겨져 있다.",
     "당신은 손으로부터 어깨에 걸쳐, 복잡하고 신비적인 푸른 인장으로 덮여 있다."},
#else
    {"There is a blue sigil on each of your hands.",
     "There are several blue sigils on your hands and arms.",
     "Your hands, arms and shoulders are covered in intricate, arcane blue writing."},
#endif

#ifdef JP 
    {"당신의 가슴에는 초록의 인장이 새겨져 있다.",
     "당신의 가슴으로부터 배에는 몇개인가 초록의 인장이 새겨져 있다.",
     "당신은 목으로부터 배에 걸쳐, 복잡하고 신비적인 초록의 인장으로 덮여 있다."},
#else
    {"There is a green sigil on your chest.",
     "There are several green sigils on your chest and abdomen.",
     "Your chest, abdomen and neck are covered in intricate, arcane green writing."},
#endif

    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    // 70

#ifdef JP 
    {"당신은 빨간색의 비늘로 일부분이 덮여있다.(AC +1)",
     "당신은 빨간색의 비늘로 대부분 덮여있다.(AC +2)",
     "당신은 빨간색의 비늘로 완전하게 덮여있다.(AC +4)"},
#else
    {"You are partially covered in red scales (AC + 1).",
     "You are mostly covered in red scales (AC + 2).",
     "You are covered in red scales (AC + 4)."},
#endif

#ifdef JP 
    {"당신은 부드러운 진줏빛의 비늘로 일부분이 덮여있다.(AC +1)",
     "당신은 부드러운 진줏빛의 비늘로 대부분 덮여있다.(AC +3)",
     "당신은 부드러운 진줏빛의 비늘로 완전하게 덮여있다.(AC +5)"},
#else
    {"You are partially covered in smooth nacreous scales (AC + 1).",
     "You are mostly covered in smooth nacreous scales (AC + 3).",
     "You are completely covered in smooth nacreous scales (AC + 5)."},
#endif

#ifdef JP 
    {"당신은 돌기 진 회색의 비늘로 일부분이 덮여있다.(AC +2, DEX -1)",
     "당신은 돌기 진 회색의 비늘로 대부분 덮여있다.(AC +4, DEX -1)",
     "당신은 돌기 진 회색의 비늘로 완전하게 덮여있다.(AC +6, DEX -2)"},
#else
    {"You are partially covered in ridged grey scales (AC + 2, dex - 1).",
     "You are mostly covered in ridged grey scales (AC + 4, dex - 1).",
     "You are completely covered in ridged grey scales (AC + 6, dex - 2)."},
#endif

#ifdef JP 
    {"당신은 금속성의 비늘로 일부분이 덮여있다.(AC +3, DEX -2)",
     "당신은 금속성의 비늘로 대부분 덮여있다.(AC +7, DEX -3)",
     "당신은 금속성의 비늘로 완전하게 덮여있다.(AC +10, DEX -4)"},
#else
    {"You are partially covered in metallic scales (AC + 3, dex - 2).",
     "You are mostly covered in metallic scales (AC + 7, dex - 3).",
     "You are completely covered in metallic scales (AC + 10, dex - 4)."},
#endif

#ifdef JP 
    {"당신은 검은 비늘로 일부분이 덮여있다.(AC +1)",
     "당신은 검은 비늘로 대부분 덮여있다.(AC +3)",
     "당신은 검은 비늘로 완전하게 덮여있다.(AC +5)"},
#else
    {"You are partially covered in black scales (AC + 1).",
     "You are mostly covered in black scales (AC + 3).",
     "You are completely covered in black scales (AC + 5)."},
#endif

#ifdef JP 
    {"당신은 흰 비늘로 일부분이 덮여있다.(AC +1)",
     "당신은 흰 비늘로 대부분 덮여있다.(AC +3)",
     "당신은 흰 비늘로 완전하게 덮여있다.(AC +5)"},
#else
    {"You are partially covered in white scales (AC + 1).",
     "You are mostly covered in white scales (AC + 3).",
     "You are completely covered in white scales (AC + 5)."},
#endif

#ifdef JP 
    {"당신은 황색의 비늘로 일부분이 덮여있다.(AC +2)",
     "당신은 황색의 비늘로 대부분 덮여있다.(AC +4, DEX -1)",
     "당신은 황색의 비늘로 완전하게 덮여있다.(AC +6, DEX -2)"},
#else
    {"You are partially covered in yellow scales (AC + 2).",
     "You are mostly covered in yellow scales (AC + 4, dex - 1).",
     "You are completely covered in yellow scales (AC + 6, dex - 2)."},
#endif

#ifdef JP 
    {"당신은 갈색의 비늘로 일부분이 덮여있다.(AC +2)",
     "당신은 갈색의 비늘로 대부분 덮여있다.(AC +4)",
     "당신은 갈색의 비늘로 완전하게 덮여있다.(AC +5)"},
#else
    {"You are partially covered in brown scales (AC + 2).",
     "You are mostly covered in brown scales (AC + 4).",
     "You are completely covered in brown scales (AC + 5)."},
#endif

#ifdef JP 
    {"당신은 푸른 비늘로 일부분이 덮여있다.(AC +1)",
     "당신은 푸른 비늘로 대부분 덮여있다.(AC +2)",
     "당신은 푸른 비늘로 완전하게 덮여있다.(AC +3)"},
#else
    {"You are partially covered in blue scales (AC + 1).",
     "You are mostly covered in blue scales (AC + 2).",
     "You are completely covered in blue scales (AC + 3)."},
#endif

#ifdef JP 
    {"당신은 보라색의 비늘로 일부분이 덮여있다.(AC +2)",
     "당신은 보라색의 비늘로 대부분 덮여있다.(AC +4)",
     "당신은 보라색의 비늘로 완전하게 덮여있다.(AC +6)"},
#else
    {"You are partially covered in purple scales (AC + 2).",
     "You are mostly covered in purple scales (AC + 4).",
     "You are completely covered in purple scales (AC + 6)."},
#endif

// 80

#ifdef JP 
    {"당신은 얼룩진 비늘로 일부분이 덮여있다.(AC +1)",
     "당신은 얼룩진 비늘로 대부분 덮여있다.(AC +2)",
     "당신은 얼룩진 비늘로 완전하게 덮여있다.(AC +3)"},
#else
    {"You are partially covered in speckled scales (AC + 1).",
     "You are mostly covered in speckled scales (AC + 2).",
     "You are covered in speckled scales (AC + 3)."},
#endif

#ifdef JP 
    {"당신은 오렌지색의 비늘로 일부분이 덮여있다.(AC +1)",
     "당신은 오렌지색의 비늘로 대부분 덮여있다.(AC +3)",
     "당신은 오렌지색의 비늘로 완전하게 덮여있다.(AC +4)"},
#else
    {"You are partially covered in orange scales (AC + 1).",
     "You are mostly covered in orange scales (AC + 3).",
     "You are completely covered in orange scales (AC + 4)."},
#endif

#ifdef JP 
    {"당신은 남색의 비늘로 일부분이 덮여있다.(AC +2)",
     "당신은 남색의 비늘로 대부분 덮여있다.(AC +3)",
     "당신은 남색의 비늘로 완전하게 덮여있다.(AC +5)"},
#else
    {"You are partially covered in indigo scales (AC + 2).",
     "You are mostly covered in indigo scales (AC + 3).",
     "You are completely covered in indigo scales (AC + 5)."},
#endif

#ifdef JP 
    {"당신은 혹이 솟은 붉은 비늘로 일부분이 덮여있다.(AC +2)",
     "당신은 혹이 솟은 붉은 비늘로 대부분 덮여있다.(AC +5, DEX -1)",
     "당신은 혹이 솟은 붉은 비늘로 완전하게 덮여있다.(AC +7, DEX -2)"},
#else
    {"You are partially covered in knobbly red scales (AC + 2).",
     "You are mostly covered in knobbly red scales (AC + 5, dex - 1).",
     "You are completely covered in knobbly red scales (AC + 7, dex - 2)."},
#endif

#ifdef JP 
    {"당신은 반짝이는 비늘로 일부분이 덮여있다.(AC +1)",
     "당신은 반짝이는 비늘로 대부분 덮여있다.(AC +2)",
     "당신은 반짝이는 비늘로 완전하게 덮여있다.(AC +3)"},
#else
    {"You are partially covered in iridescent scales (AC + 1).",
     "You are mostly covered in iridescent scales (AC + 2).",
     "You are completely covered in iridescent scales (AC + 3)."},
#endif

#ifdef JP 
    {"당신은 문양이 세겨진 비늘로 일부분이 덮여있다.(AC +1)",
     "당신은 문양이 세겨진 비늘로 대부분 덮여있다.(AC +2)",
     "당신은 문양이 세겨진 비늘로 완전하게 덮여있다.(AC +3)"},
#else
    {"You are partially covered in patterned scales (AC + 1).",
     "You are mostly covered in patterned scales (AC + 2).",
     "You are completely covered in patterned scales (AC + 3)."},
#endif
};

/*
   If giving a mutation which must succeed (eg demonspawn), must add exception
   to the "resist mutation" mutation thing.
 */

const char *gain_mutation[][3] = {
#ifdef JP 
    {"당신의 피부는 강인하게 되었다.", "당신의 피부는 강인하게 되었다.", "당신의 피부는 강인하게 되었다."},
#else
    {"Your skin toughens.", "Your skin toughens.", "Your skin toughens."},
#endif

    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}
    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}
    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}

#ifdef JP 
    {"녹색의 비늘이 당신의 몸의 일부에 자라났다.",
     "녹색의 비늘이 당신의 몸을 더욱 가렸다.",
     "녹색의 비늘이 당신의 몸을 완전하게 가렸다."},
#else
    {"Green scales grow over part of your body.",
     "Green scales spread over more of your body.",
     "Green scales cover you completely."},
#endif

#ifdef JP 
    {"두께가 있는 검은 비늘이 당신의 몸의 일부에 자라났다.",
     "두께가 있는 검은 비늘이 당신의 몸을 더욱 가렸다.",
     "두께가 있는 검은 비늘이 당신의 몸을 완전하게 가렸다."},
#else
    {"Thick black scales grow over part of your body.",
     "Thick black scales spread over more of your body.",
     "Thick black scales cover you completely."},
#endif

#ifdef JP 
    {"유연한 회색의 비늘이 당신의 몸의 일부에 자라났다.",
     "유연한 회색의 비늘이 당신의 몸을 더욱 가렸다.",
     "유연한 회색의 비늘이 당신의 몸을 완전하게 가렸다."},
#else
    {"Supple grey scales grow over part of your body.",
     "Supple grey scales spread over more of your body.",
     "Supple grey scales cover you completely."},
#endif

#ifdef JP 
    {"당신의 몸에 뼈의 장갑이 자라났다.",
     "당신의 뼈의 장갑이 더욱 성장했다.",
     "당신의 뼈의 장갑이 더욱 성장했다."},
#else
    {"You grow protective plates of bone.",
     "You grow more protective plates of bone.",
     "You grow more protective plates of bone."},
#endif

#ifdef JP 
    {"당신은 반발력을 방출하기 시작했다.",
     "당신의 반발력이 더욱 강해졌다.",
     "당신의 반발력이 더욱 강해졌다."},
#else
    {"You begin to radiate repulsive energy.",
     "Your repulsive radiation grows stronger.",
     "Your repulsive radiation grows stronger."},
#endif

#ifdef JP 
    {"당신은 보다 건강하게 된 것 같다.", "당신은 보다 건강하게 된 것 같다.",  "당신은 보다 건강하게 된 것 같다."},
#else
    {"You feel healthy.", "You feel healthy.",  "You feel healthy."},
#endif
// 10
#ifdef JP 
    {"당신은 고기에 굶주렸다.", "당신은 고기에 굶주렸다.", "당신은 고기에 굶주렸다."},
#else
    {"You hunger for flesh.", "You hunger for flesh.", "You hunger for flesh."},
#endif

#ifdef JP 
    {"당신은 야채에 굶주렸다.", "당신은 야채에 굶주렸다.",
     "당신은 야채에 굶주렸다."},
#else
    {"You hunger for vegetation.", "You hunger for vegetation.",
     "You hunger for vegetation."},
#endif

#ifdef JP 
    {"당신은 갑자기 쌀쌀함을 느꼈다.", "당신은 갑자기 쌀쌀함을 느꼈다.",
     "당신은 갑자기 쌀쌀함을 느꼈다."},
#else
    {"You feel a sudden chill.", "You feel a sudden chill.",
     "You feel a sudden chill."},
#endif

#ifdef JP 
    {"당신은 갑자기 더위를 느꼈다.", "당신은 갑자기 더위를 느꼈다.",
     "당신은 갑자기 더위를 느꼈다."},
#else
    {"You feel hot for a moment.", "You feel hot for a moment.",
     "You feel hot for a moment."},
#endif

#ifdef JP 
    {"당신은 부전도체가 된 것 같다.", "당신은 부전도체가 된 것 같다.", "당신은 부전도체가 된 것 같다."},
#else
    {"You feel insulated.", "You feel insulated.", "You feel insulated."},
#endif

#ifdef JP 
    {"당신은 보다 빠르게 상처가 치유되게 되었다.",
     "당신은 보다 빠르게 상처가 치유되게 되었다.",
     "당신은 재생 능력을 몸에 익혔다."},
#else
    {"You begin to heal more quickly.",
     "You begin to heal more quickly.",
     "You begin to regenerate."},
#endif

#ifdef JP 
    {"당신은 조금 허기를 느꼈다.", "당신은 조금 허기를 느꼈다.",
     "당신은 조금 허기를 느꼈다."},
#else
    {"You feel a little hungry.", "You feel a little hungry.",
     "You feel a little hungry."},
#endif

#ifdef JP 
    {"당신의 신진대사는 느려졌다.", "당신의 신진대사는 느려졌다.",
     "당신의 신진대사는 느려졌다."},
#else
    {"Your metabolism slows.", "Your metabolism slows.",
     "Your metabolism slows."},
#endif

#ifdef JP 
    {"당신은 약해진 것 같다.", "당신은 약해진 것 같다.", "당신은 약해진 것 같다."},
#else
    {"You feel weaker.", "You feel weaker.", "You feel weaker."},
#endif

#ifdef JP 
    {"당신은 지능이 떨어짐을 느꼈다.", "당신은 지능이 떨어짐을 느꼈다.",
     "당신은 지능이 떨어짐을 느꼈다."},
#else
    {"You feel less intelligent.", "You feel less intelligent",
     "You feel less intelligent"},
#endif
// 20
#ifdef JP 
    {"당신은 서투르게 된 것 같다.", "당신은 서투르게 된 것 같다.",
     "당신은 서투르게 된 것 같다."},
#else
    {"You feel clumsy.", "You feel clumsy.",
     "You feel clumsy."},
#endif

#ifdef JP 
    {"당신은 제어의 힘을 느꼈다.", "당신은 제어의 힘을 느꼈다.",
     "당신은 제어의 힘을 느꼈다."},
#else
    {"You feel controlled.", "You feel controlled.",
     "You feel controlled."},
#endif

#ifdef JP 
    {"당신은 기묘한 불분명함을 느꼈다.",
     "당신은 더욱 기묘한 불분명함을 느꼈다.",
     "당신은 더욱 기묘한 불분명함을 느꼈다."},
#else
    {"You feel weirdly uncertain.",
     "You feel even more weirdly uncertain.",
     "You feel even more weirdly uncertain."},
#endif

#ifdef JP 
    {"당신은 마법의 저항력을 얻었다.",
     "당신은 더욱 더 강한 마법의 저항력을 얻었다.",
     "당신은 마법의 효과에 거의 영향을 받지 않게 되었다."},
#else
    {"You feel resistant to magic.",
     "You feel more resistant to magic.",
     "You feel almost impervious to the effects of magic."},
#endif

#ifdef JP 
    {"당신은 재빠르게 되었다.", "당신은 재빠르게 되었다.", "당신은 재빠르게 되었다."},
#else
    {"You feel quick.", "You feel quick.", "You feel quick."},
#endif

#ifdef JP 
    {"당신의 시력은 날카로워졌다.", "당신의 시력은 날카로워졌다.", "당신의 시력은 날카로워졌다."},
#else
    {"Your vision sharpens.", "Your vision sharpens.", "Your vision sharpens."},
#endif

#ifdef JP 
    {"당신의 몸은 비비 꼬여 기형이 되었다.", "당신의 몸은 비비 꼬여 기형이 되었다.",
     "당신의 몸은 비비 꼬여 기형이 되었다."},
#else
    {"Your body twists and deforms.", "Your body twists and deforms.",
     "Your body twists and deforms."},
#endif

#ifdef JP 
    {"당신은 튀어오름을 느꼈다.", "당신은 튀어오름을 느꼈다.", "당신은 튀어오름을 느꼈다."},
#else
    {"You feel jumpy.", "You feel more jumpy.", "You feel even more jumpy."},
#endif

#ifdef JP 
    {"잠깐 당신의 입 안에서 불쾌한 맛이 났다.",
     "잠깐 당신의 입 안에서 불쾌한 맛이 났다.",
     "잠깐 당신의 입 안에서 불쾌한 맛이 났다."},
#else
    {"There is a nasty taste in your mouth for a moment.",
     "There is a nasty taste in your mouth for a moment.",
     "There is a nasty taste in your mouth for a moment."},
#endif

#ifdef JP 
    {"당신은 주위의 상황을 알 수 있게 되었다.",
     "당신은 주위의 상황을 더욱 잘 알게 되었다.",
     "당신은 주위의 상황을 보다 한층 더 잘 알게 되었다."},
#else
    {"You feel aware of your surroundings.",
     "You feel more aware of your surroundings.",
     "You feel even more aware of your surroundings."},
#endif
// 30

#ifdef JP 
    {"당신은 목구멍이 뜨거워졌다.", "당신은 목구멍이 뜨거워졌다.",
     "당신은 목구멍이 뜨거워졌다."},
#else
    {"Your throat feels hot.", "Your throat feels hot.",
     "Your throat feels hot."},
#endif

#ifdef JP 
    {"당신은 조금 튀어오름을 느꼈다.", "당신은 한층 더 튀어오름을 느꼈다.",
     "당신은 보다 한층 더 튀어오름을 느꼈다."},
#else
    {"You feel a little jumpy.", "You feel more jumpy.",
     "You feel even more jumpy."},
#endif

#ifdef JP 
    {"당신의 머리에 한 쌍의 뿔이 나 왔다!",
     "당신의 뿔이 다소 성장했다.",
     "당신의 뿔이 다소 성장했다."},
#else
    {"A pair of horns grows on your head!",
     "The horns on your head grow some more.",
     "The horns on your head grow some more."},
#endif

#ifdef JP 
    {"당신은 근육통을 느꼈다.", "당신은 근육통을 느꼈다.",
     "당신은 근육통을 느꼈다."},
#else
    {"Your muscles feel sore.", "Your muscles feel sore.",
     "Your muscles feel sore."},
#endif

#ifdef JP 
    {"당신의 근육은 유연하게 되었다.", "당신의 근육은 유연하게 되었다.",
     "당신의 근육은 유연하게 되었다."},
#else
    {"Your muscles feel loose.", "Your muscles feel loose.",
     "Your muscles feel loose."},
#endif

#ifdef JP 
    {"당신은 가벼운 방향감각 상실을 느꼈다.", "당신은 가벼운 방향감각 상실을 느꼈다.",
     "당신은 어디에 있는 것일까?"},
#else
    {"You feel a little disoriented.", "You feel a little disoriented.",
     "Where the Hells are you?"},
#endif

#ifdef JP 
    {"당신은 사고가 명료해졌다.", "당신은 사고가 명료해졌다.",
     "당신은 사고가 명료해졌다."},
#else
    {"Your thoughts seem clearer.", "Your thoughts seem clearer.",
     "Your thoughts seem clearer."},
#endif

#ifdef JP 
    {"당신은 조금 화가 났다.", "당신은 화가 났다.",
     "당신은 모두에 대해서 매우 분노를 느꼈다!"},
#else
    {"You feel a little pissed off.", "You feel angry.",
     "You feel extremely angry at everything!"},
#endif

#ifdef JP 
    {"당신은 몸이 소모되는 것을 느꼈다.", "당신은 몸이 소모되는 것을 느꼈다.",
     "당신은 몸이 너덜너덜 떨어져 나가는 것을 느꼈다."},
#else
    {"You feel yourself wasting away.", "You feel yourself wasting away.",
     "You feel your body start to fall apart."},
#endif

#ifdef JP 
    {"당신의 시야는 희미해졌다.", "당신의 시야는 희미해졌다.", "당신의 시야는 희미해졌다."},
#else
    {"Your vision blurs.", "Your vision blurs.", "Your vision blurs."},
#endif
// 40

#ifdef JP 
    {"당신은 유전자적으로 안정되었다.", "당신은 유전자적으로 안정되었다.",
     "당신은 유전자적으로 변화하지 않게 되었다."},
#else
    {"You feel genetically stable.", "You feel genetically stable.",
     "You feel genetically immutable."},
#endif

#ifdef JP 
    {"당신은 허약하게 되었다.", "당신은 허약하게 되었다.",  "당신은 허약하게 되었다."},
    {"당신은 강건하게 되었다.", "당신은 강건하게 되었다.", "당신은 강건하게 되었다."},
    {"당신은 기묘하게 무감각이 되었다.", "", ""},
    {"당신은 음에너지 내성을 얻었다.", "당신은 음에너지 내성을 얻었다.", "당신은 음에너지 내성을 얻었다."},
    {"어떤 소리가 당신에게 호소해 왔다.", "", ""},
    {"도움이 그렇게 멀지 않은 곳에 있다!", "", ""},
    {"당신은 불과 유황의 냄새를 냄새 맡았다.", "", ""},
    {"당신은 공포를 일으키는 힘을 불러일으킬 수가 있게 되었다.", "", ""},
    {"당신은 죽은자에게 친근감을 느끼게 되었다.", "", ""},
#else
    {"You feel frail.", "You feel frail.",  "You feel frail."},
    {"You feel robust.", "You feel robust.", "You feel robust."},
    {"You feel a strange anaesthesia.", "", ""},
    {"You feel negative.", "You feel negative.", "You feel negative."},
    {"A thousand chattering voices call out to you.", "", ""},
    {"Help is not far away!", "", ""},
    {"You smell fire and brimstone.", "", ""},
    {"You feel a terrifying power at your call.", "", ""},
    {"You feel an affinity for the dead.", "", ""},
#endif
// 50
#ifdef JP 
    {"당신은 악마 류에 친근감을 느끼게 되었다.", "", ""},
    {"당신은 누군가를 기괴하고 무서운 장소에 불러올 수 있고 있다.", "", ""},
    {"당신은 굶어 죽고 있다.", "", ""},
    {"당신은 마법의 에너지가 흘러드는 것을 느꼈다.", "", ""},
    {"당신의 피부는 기묘하고 불쾌한 감각이 느껴졌다.", "", ""},
    {"당신은 게헨나의 불의 냄새를 맡았다.", "", ""},
    {"당신은 코큐투스의 얼음의 냉기에 의해 영혼이 얼어붙는걸 느꼈다.", "", ""},
    {"당신의 주위 세계를 그림자가 앞질러갔다.", "", ""},
#else
    {"You feel an affinity for all demonkind.", "", ""},
    {"You feel something pulling you to a strange and terrible place.", "", ""},
    {"You feel hungry for death.", "", ""},
    {"You feel a flux of magical energy.", "", ""},
    {"Your skin tingles in a strangely unpleasant way.", "", ""},
    {"You smell the fires of Gehenna.", "", ""},
    {"You feel the icy cold of Cocytus chill your soul.", "", ""},
    {"A shadow passes over the world around you.", "", ""},
#endif

#ifdef JP 
    {"당신의 손톱은 성장해 갔다.", "당신의 손톱은 날카로워졌다.",
     "당신의 손은 구조가 변화했다."},
#else
    {"Your fingernails lengthen.", "Your fingernails sharpen.",
     "Your hands twist into claws."},
#endif

#ifdef JP 
    {"당신의 다리는 굽으로 갈라지고 쭈그러 들었다.", "", ""},
#else
    {"Your feet shrivel into cloven hooves.", "", ""},
#endif
    // 60

#ifdef JP 
    {"당신의 입속에 불쾌한 맛이 났다.", "당신의 입속에 매우 불쾌한 맛이 났다.",
     "당신의 입속에 엄청나게 불쾌한 맛이 났다."},
#else
    {"You taste something nasty.", "You taste something very nasty.",
     "You taste something extremely nasty."},
#endif

#ifdef JP 
    {"당신의 꼬리의 끝에 독이 있는 가시가 났다.",
     "당신의 꼬리의 가시는 날카로워졌다.",
     "당신의 꼬리의 가시는 매우 날카로워졌다."},
#else
    {"A poisonous barb forms on the end of your tail.",
     "The barb on your tail looks sharper.",
     "The barb on your tail looks very sharp."},
#endif

#ifdef JP 
    {"당신의 날개는 크고 강력하게 성장했다.", "", ""},
#else
    {"Your wings grow larger and stronger.", "", ""},
#endif

#ifdef JP 
    {"당신은 손이 근지려워졌다.", "당신은 손으로부터 팔에 걸쳐 근지려워졌다.",
     "당신은 손으로부터 어깨에 걸쳐 근지려워졌다."},
#else
    {"Your hands itch.", "Your hands and forearms itch.",
     "Your arms, hands and shoulders itch."},
#endif

#ifdef JP 
    {"당신은 가슴이 근지려워졌다.", "당신은 가슴으로부터 배에 걸쳐 근지려워졌다.",
     "당신은 목으로부터 배에 걸쳐 근지려워졌다."},
#else
    {"Your chest itches.", "Your chest and abdomen itch.",
     "Your chest, abdomen and neck itch."},
#endif

    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    // 70

#ifdef JP 
    {"적색의 비늘이 당신의 몸의 일부에 자라났다.",
     "적색의 비늘이 당신의 몸을 더욱 가렸다.",
     "적색의 비늘이 당신의 몸을 완전하게 가렸다."},
    {"부드러운 진줏빛의 비늘이 당신의 몸의 일부에 자라났다.",
     "부드러운 진줏빛의 비늘이 당신의 몸을 더욱 가렸다.",
     "부드러운 진줏빛의 비늘이 당신의 몸을 완전하게 가렸다."},
    {"돌기 진 회색의 비늘이 당신의 몸의 일부에 자라났다.",
     "돌기 진 회색의 비늘이 당신의 몸을 더욱 가렸다.",
     "돌기 진 회색의 비늘이 당신의 몸을 완전하게 가렸다."},
    {"금속성의 비늘이 당신의 몸의 일부에 자라났다.",
     "금속성의 비늘이 당신의 몸을 더욱 가렸다.",
     "금속성의 비늘이 당신의 몸을 완전하게 가렸다."},
    {"검은 비늘이 당신의 몸의 일부에 자라났다.",
     "검은 비늘이 당신의 몸을 더욱 가렸다.",
     "검은 비늘이 당신의 몸을 완전하게 가렸다."},
    {"흰 비늘이 당신의 몸의 일부에 자라났다.",
     "흰 비늘이 당신의 몸을 더욱 가렸다.",
     "흰 비늘이 당신의 몸을 완전하게 가렸다."},
    {"황색의 비늘이 당신의 몸의 일부에 자라났다.",
     "황색의 비늘이 당신의 몸을 더욱 가렸다.",
     "황색의 비늘이 당신의 몸을 완전하게 가렸다."},
    {"갈색의 비늘이 당신의 몸의 일부에 자라났다.",
     "갈색의 비늘이 당신의 몸을 더욱 가렸다.",
     "갈색의 비늘이 당신의 몸을 완전하게 가렸다."},
    {"푸른 비늘이 당신의 몸의 일부에 자라났다.",
     "푸른 비늘이 당신의 몸을 더욱 가렸다.",
     "푸른 비늘이 당신의 몸을 완전하게 가렸다."},
    {"보라색의 비늘이 당신의 몸의 일부에 자라났다.",
     "보라색의 비늘이 당신의 몸을 더욱 가렸다.",
     "보라색의 비늘이 당신의 몸을 완전하게 가렸다."},
#else
    {"Red scales grow over part of your body.",
     "Red scales spread over more of your body.",
     "Red scales cover you completely."},
    {"Smooth nacreous scales grow over part of your body.",
     "Smooth nacreous scales spread over more of your body.",
     "Smooth nacreous scales cover you completely."},
    {"Ridged grey scales grow over part of your body.",
     "Ridged grey scales spread over more of your body.",
     "Ridged grey scales cover you completely."},
    {"Metallic scales grow over part of your body.",
     "Metallic scales spread over more of your body.",
     "Metallic scales cover you completely."},
    {"Black scales grow over part of your body.",
     "Black scales spread over more of your body.",
     "Black scales cover you completely."},
    {"White scales grow over part of your body.",
     "White scales spread over more of your body.",
     "White scales cover you completely."},
    {"Yellow scales grow over part of your body.",
     "Yellow scales spread over more of your body.",
     "Yellow scales cover you completely."},
    {"Brown scales grow over part of your body.",
     "Brown scales spread over more of your body.",
     "Brown scales cover you completely."},
    {"Blue scales grow over part of your body.",
     "Blue scales spread over more of your body.",
     "Blue scales cover you completely."},
    {"Purple scales grow over part of your body.",
     "Purple scales spread over more of your body.",
     "Purple scales cover you completely."},
#endif
    // 80

#ifdef JP 
    {"얼룩진 비늘이 당신의 몸의 일부에 자라났다.",
     "얼룩진 비늘이 당신의 몸을 더욱 가렸다.",
     "얼룩진 비늘이 당신의 몸을 완전하게 가렸다."},
    {"오렌지색의 비늘이 당신의 몸의 일부에 자라났다.",
     "오렌지색의 비늘이 당신의 몸을 더욱 가렸다.",
     "오렌지색의 비늘이 당신의 몸을 완전하게 가렸다."},
    {"남색의 비늘이 당신의 몸의 일부에 자라났다.",
     "남색의 비늘이 당신의 몸을 더욱 가렸다.",
     "남색의 비늘이 당신의 몸을 완전하게 가렸다."},
    {"혹이 솟은 붉은 비늘이 당신의 몸의 일부에 자라났다.",
     "혹이 솟은 붉은 비늘이 당신의 몸을 더욱 가렸다.",
     "혹이 솟은 붉은 비늘이 당신의 몸을 완전하게 가렸다."},
    {"반짝이는 비늘이 당신의 몸의 일부에 자라났다.",
     "반짝이는 비늘이 당신의 몸을 더욱 가렸다.",
     "반짝이는 비늘이 당신의 몸을 완전하게 가렸다."},
    {"문양이 세겨진 비늘이 당신의 몸의 일부에 자라났다.",
     "문양이 세겨진 비늘이 당신의 몸을 더욱 가렸다.",
     "문양이 세겨진 비늘이 당신의 몸을 완전하게 가렸다."},
#else
    {"Speckled scales grow over part of your body.",
     "Speckled scales spread over more of your body.",
     "Speckled scales cover you completely."},
    {"Orange scales grow over part of your body.",
     "Orange scales spread over more of your body.",
     "Orange scales cover you completely."},
    {"Indigo scales grow over part of your body.",
     "Indigo scales spread over more of your body.",
     "Indigo scales cover you completely."},
    {"Knobbly red scales grow over part of your body.",
     "Knobbly red scales spread over more of your body.",
     "Knobbly red scales cover you completely."},
    {"Iridescent scales grow over part of your body.",
     "Iridescent scales spread over more of your body.",
     "Iridescent scales cover you completely."},
    {"Patterned scales grow over part of your body.",
     "Patterned scales spread over more of your body.",
     "Patterned scales cover you completely."},
#endif
};

const char *lose_mutation[][3] = {

#ifdef JP 
    {"당신의 피부는 부드러워졌다.", "당신의 피부는 부드러워졌다.",
     "당신의 피부는 부드러워졌다."},
#else
    {"Your skin feels delicate.", "Your skin feels delicate.",
     "Your skin feels delicate."},
#endif

#ifdef JP 
    {"당신은 허약해졌다.", "당신은 허약해졌다.", "당신은 허약해졌다."},
#else
    {"You feel weaker.", "You feel weaker.", "You feel weaker."},
#endif

#ifdef JP 
    {"당신은 지능이 떨어졌다.", "당신은 지능이 떨어졌다.",
     "당신은 지능이 떨어졌다."},
#else
    {"You feel less intelligent.", "You feel less intelligent",
     "You feel less intelligent"},
#endif

#ifdef JP 
    {"당신은 서툴러졌다.", "당신은 서툴러졌다.", "당신은 서툴러졌다."},
#else
    {"You feel clumsy.", "You feel clumsy.", "You feel clumsy."},
#endif

#ifdef JP 
    {"당신의 녹색의 비늘이 사라져버렸다.",
     "당신의 녹색의 비늘이 다소 감소했다.",
     "당신의 녹색의 비늘이 다소 감소했다."},
#else
    {"Your green scales disappear.",
     "Your green scales recede somewhat.",
     "Your green scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 검은 비늘이 사라져버렸다.", "당신의 검은 비늘이 다소 감소했다.",
     "당신의 검은 비늘이 다소 감소했다."},
#else
    {"Your black scales disappear.", "Your black scales recede somewhat.",
     "Your black scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 회색의 비늘이 사라져버렸다.", "당신의 회색의 비늘이 다소 감소했다.",
     "당신의 회색의 비늘이 다소 감소했다."},
#else
    {"Your grey scales disappear.", "Your grey scales recede somewhat.",
     "Your grey scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 뼈의 장갑은 수축하기 시작했다.", "당신의 뼈의 장갑은 수축했다.",
     "당신의 뼈의 장갑은 수축했다."},
#else
    {"Your bony plates shrink away.", "Your bony plates shrink.",
     "Your bony plates shrink."},
#endif

#ifdef JP 
    {"당신이 매력적이게 된 것을 느겼다.", "당신이 매력적이게 된 것을 느겼다.", "당신이 매력적이게 된 것을 느겼다."},
#else
    {"You feel attractive.", "You feel attractive.", "You feel attractive."},
#endif

#ifdef JP 
    {"당신은 조금 건강이 약화된 것을 느꼈다.", "당신은 조금 건강이 약화된 것을 느꼈다.",
     "당신은 조금 건강이 약화된 것을 느꼈다."},
#else
    {"You feel a little less healthy.", "You feel a little less healthy.",
     "You feel a little less healthy."},
#endif

#ifdef JP 
    {"당신은 보다 밸런스 좋은 다이어트가 가능해 졌다.",
     "당신은 보다 밸런스 좋은 다이어트가 가능해 졌다.",
     "당신은 보다 밸런스 좋은 다이어트가 가능해 졌다."},
#else
    {"You feel able to eat a more balanced diet.",
     "You feel able to eat a more balanced diet.",
     "You feel able to eat a more balanced diet."},
#endif

#ifdef JP 
    {"당신은 보다 밸런스 좋은 다이어트가 가능해 졌다.",
     "당신은 보다 밸런스 좋은 다이어트가 가능해 졌다.",
     "당신은 보다 밸런스 좋은 다이어트가 가능해 졌다."},
#else
    {"You feel able to eat a more balanced diet.",
     "You feel able to eat a more balanced diet.",
     "You feel able to eat a more balanced diet."},
#endif

#ifdef JP 
    {"당신은 갑자기 더위를 느꼈다.", "당신은 갑자기 더위를 느꼈다.",
     "당신은 갑자기 더위를 느꼈다."},
#else
    {"You feel hot for a moment.", "You feel hot for a moment.",
     "You feel hot for a moment."},
#endif

#ifdef JP 
    {"당신은 갑자기 한기를 느꼈다.", "당신은 갑자기 한기를 느꼈다.",
     "당신은 갑자기 한기를 느꼈다."},
#else
    {"You feel a sudden chill.", "You feel a sudden chill.",
     "You feel a sudden chill."},
#endif

#ifdef JP 
    {"당신은 전도체가 되는걸 느꼈다.", "당신은 전도체가 되는걸 느꼈다.", "당신은 전도체가 되는걸 느꼈다."},
#else
    {"You feel conductive.", "You feel conductive.", "You feel conductive."},
#endif

#ifdef JP 
    {"당신의 치유 속도는 느려졌다.", "당신의 치유 속도는 느려졌다.",
     "당신의 치유 속도는 느려졌다."},
#else
    {"Your rate of healing slows.", "Your rate of healing slows.",
     "Your rate of healing slows."},
#endif

#ifdef JP 
    {"당신의 신진대사는 느려졌다.", "당신의 신진대사는 느려졌다.",
     "당신의 신진대사는 느려졌다."},
#else
    {"Your metabolism slows.", "Your metabolism slows.",
     "Your metabolism slows."},
#endif

#ifdef JP 
    {"당신은 조금 허기를 느꼈다.", "당신은 조금 허기를 느꼈다.",
     "당신은 조금 허기를 느꼈다."},
#else
    {"You feel a little hungry.", "You feel a little hungry.",
     "You feel a little hungry."},
#endif

    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}
    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}
// 20
    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}

#ifdef JP 
    {"당신은 무작위 효과를 느꼈다.", "당신은 제어의 약화를 느꼈다.", "당신은 제어의 약해짐을 느꼈다."},
    {"당신은 안정성을 느꼈다.", "당신은 안정성을 느꼈다.", "당신은 안정성을 느꼈다."},
#else
    {"You feel random.", "You feel uncontrolled.", "You feel uncontrolled."},
    {"You feel stable.", "You feel stable.", "You feel stable."},
#endif

#ifdef JP 
    {"당신은 마법에의 저항력이 약화된 것을 느꼈다.", "당신은 마법에의 저항력이 약화된 것을 느꼈다.",
     "당신은 다시 마법의 충격에 영향을 받게 되었다."},
#else
    {"You feel less resistant to magic.", "You feel less resistant to magic.",
     "You feel vulnerable to magic again."},
#endif

#ifdef JP 
    {"당신은 움직임이 완만하게 되었다.", "당신은 움직임이 완만하게 되었다.", "당신은 움직임이 완만하게 되었다."},
#else
    {"You feel sluggish.", "You feel sluggish.", "You feel sluggish."},
#endif

#ifdef JP 
    {"당신의 시력은 둔해졌다.", "당신의 시력은 둔해졌다.",
     "당신의 시력은 둔해졌다."},
#else
    {"Your vision seems duller.", "Your vision seems duller.",
     "Your vision seems duller."},
#endif

#ifdef JP 
    {"당신의 육체는 한층 더 정상으로 보인다.",
     "당신의 육체는 약간씩 정상으로 보인다.",
     "당신의 육체는 약간씩 정상으로 보인다."},
#else
    {"Your body's shape seems more normal.",
     "Your body's shape seems slightly more normal.",
     "Your body's shape seems slightly more normal."},
#endif

#ifdef JP 
    {"당신은 정체를 느꼈다.", "당신의 튀어오름은 감소했다.", "당신의 튀어오름은 감소했다."},
#else
    {"You feel static.", "You feel less jumpy.", "You feel less jumpy."},
#endif

#ifdef JP 
    {"당신은 목의 안쪽이 상하는 것을 느꼈다.",
     "당신은 목의 안쪽이 상하는 것을 느꼈다.", "당신은 목의 안쪽이 상하는 것을 느꼈다."},
#else
    {"You feel an ache in your throat.",
     "You feel an ache in your throat.", "You feel an ache in your throat."},
#endif

#ifdef JP 
    {"당신은 조금 분별을 잃었다.", "당신은 조금 분별을 잃었다.",
     "당신은 조금 분별을 잃었다."},
#else
    {"You feel slightly disorientated.", "You feel slightly disorientated.",
     "You feel slightly disorientated."},
#endif
// 30

#ifdef JP 
    {"당신의 목의 안쪽은 차가와졌다.",
     "당신의 목의 안쪽은 차가와졌다.",
     "당신의 목의 안쪽은 차가와졌다."},
#else
    {"A chill runs up and down your throat.",
     "A chill runs up and down your throat.",
     "A chill runs up and down your throat."},
#endif

#ifdef JP 
    {"당신은 튀어오름이 약간 줄어들었다.", "당신은 튀어오름이 줄었다.",
     "당신은 튀어오름이 줄었다."},
#else
    {"You feel a little less jumpy.", "You feel less jumpy.",
     "You feel less jumpy."},
#endif

#ifdef JP 
    {"당신의 머리에 있는 뿔이 수축해 사라졌다.",
     "당신의 머리에 있는 뿔이 조금 작아졌다.",
     "당신의 머리에 있는 뿔이 조금 작아졌다."},
#else
    {"The horns on your head shrink away.",
     "The horns on your head shrink a bit.",
     "The horns on your head shrink a bit."},
#endif

#ifdef JP 
    {"당신의 근육은 부드러워진 것 같다.", "당신의 근육은 부드러워진 것 같다.",
     "당신의 근육은 부드러워진 것 같다."},
#else
    {"Your muscles feel loose.", "Your muscles feel loose.",
     "Your muscles feel loose."},
#endif

#ifdef JP 
    {"당신은 근육통을 느꼈다.", "당신은 근육통을 느꼈다.",
     "당신은 근육통을 느꼈다."},
#else
    {"Your muscles feel sore.", "Your muscles feel sore.",
     "Your muscles feel sore."},
#endif

#ifdef JP 
    {"당신의 분별이 약간 좋아졌다.", "당신의 분별이 약간 좋아졌다.",
     "당신의 분별이 약간 좋아졌다."},
#else
    {"You feel less disoriented.", "You feel less disoriented.",
     "You feel less disoriented."},
#endif

#ifdef JP 
    {"당신의 사고는 혼란스러워졌다.", "당신의 사고는 사고는 혼란스러워졌다.",
     "당신의 사고는 혼란스러워졌다."},
#else
    {"Your thinking seems confused.", "Your thinking seems confused.",
     "Your thinking seems confused."},
#endif

#ifdef JP 
    {"당신은 조금 온화하게 되었다.", "당신은 조금 덜 화내게 되었다.",
     "당신은 조금 덜 화내게 되었다."},
#else
    {"You feel a little more calm.", "You feel a little less angry.",
     "You feel a little less angry."},
#endif

#ifdef JP 
    {"당신은 건강한 몸이 되었다.", "당신은 조금 건강한 몸에 가까워졌다.",
     "당신은 조금 건강한 몸에 가까워졌다."},
#else
    {"You feel healthier.", "You feel a little healthier.",
     "You feel a little healthier."},
#endif

#ifdef JP 
    {"당신의 시력은 날카로워졌다.", "당신의 시력은 조금 날카로워졌다.",
     "당신의 시력은 조금 날카로워졌다."},
#else
    {"Your vision sharpens.", "Your vision sharpens a little.",
     "Your vision sharpens a little."},
#endif
// 40

#ifdef JP 
    {"당신은 유전자적으로 불안정하게 되었다.", "당신은 유전자적으로 불안정하게 되었다.",
     "당신은 유전자적으로 불안정하게 되었다."},
#else
    {"You feel genetically unstable.", "You feel genetically unstable.",
     "You feel genetically unstable."},
#endif

#ifdef JP 
    {"당신은 튼튼하게 되었다.", "당신은 튼튼하게 되었다.", "당신은 튼튼하게 되었다."},
    {"당신은 취약하게 되었다.", "당신은 취약하게 되었다.", "당신은 취약하게 되었다."},
#else
    {"You feel robust.", "You feel robust.", "You feel robust."},
    {"You feel frail.", "You feel frail.", "You feel frail."},
#endif

/* Some demonic powers (which can't be lost) start here... */
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
// 50
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},

#ifdef JP 
    {"당신의 손톱은 보통 크기로 줄어들어 버렸다.",
     "당신의 손톱은 날카로움을 잃었다.", "당신의 손 구조는 손가락으로 변화했다."},
#else
    {"Your fingernails shrink to normal size.",
     "Your fingernails look duller.", "Your hands feel fleshier."},
#endif

#ifdef JP 
    {"당신의 발굽이 보통 다리로 변화했다!", "", ""},
#else
    {"Your hooves expand and flesh out into feet!", "", ""},
#endif
    // 60
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
// 70

#ifdef JP 
    {"당신의 적색의 비늘이 사라져 버렸다.", "당신의 적색의 비늘의 일부분이 떨어져나갔다.",
     "당신의 적색의 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your red scales disappear.", "Your red scales recede somewhat.",
     "Your red scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 부드러운 진줏빛 비늘이 사라져 버렸다.",
     "당신의 부드러운 진줏빛 비늘의 일부분이 떨어져나갔다.",
     "당신의 부드러운 진줏빛 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your smooth nacreous scales disappear.",
     "Your smooth nacreous scales recede somewhat.",
     "Your smooth nacreous scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 돌기 진 회색의 비늘이 사라져 버렸다.",
     "당신의 돌기 진 회색의 비늘의 일부분이 떨어져나갔다.",
     "당신의 돌기 진 회색의 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your ridged grey scales disappear.",
     "Your ridged grey scales recede somewhat.",
     "Your ridged grey scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 금속성의 비늘이 사라져 버렸다.",
     "당신의 금속성의 비늘의 일부분이 떨어져나갔다.",
     "당신의 금속성의 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your metallic scales disappear.",
     "Your metallic scales recede somewhat.",
     "Your metallic scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 검은 비늘이 사라져 버렸다.",
     "당신의 비늘의 일부분이 떨어져나갔다.",
     "당신의 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your black scales disappear.", "Your black scales recede somewhat.",
     "Your black scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 흰 비늘이 사라져 버렸다.",
     "당신의 흰 비늘의 일부분이 떨어져나갔다.",
     "당신의 흰 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your white scales disappear.", "Your white scales recede somewhat.",
     "Your white scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 황색의 비늘이 사라져 버렸다.",
     "당신의 황색의 비늘의 일부분이 떨어져나갔다.",
     "당신의 황색의 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your yellow scales disappear.", "Your yellow scales recede somewhat.",
     "Your yellow scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 갈색의 비늘이 사라져 버렸다.",
     "당신의 갈색의 비늘의 일부분이 떨어져나갔다.",
     "당신의 갈색의 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your brown scales disappear.", "Your brown scales recede somewhat.",
     "Your brown scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 푸른 비늘이 사라져 버렸다.",
     "당신의 푸른 비늘의 일부분이 떨어져나갔다.",
     "당신의 푸른 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your blue scales disappear.", "Your blue scales recede somewhat.",
     "Your blue scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 보라색의 비늘이 사라져 버렸다.",
     "당신의 보라색의 비늘의 일부분이 떨어져나갔다.",
     "당신의 보라색의 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your purple scales disappear.", "Your purple scales recede somewhat.",
     "Your purple scales recede somewhat."},
#endif
// 80

#ifdef JP 
    {"당신의 얼룩진 비늘이 사라져 버렸다.",
     "당신의 얼룩진 비늘의 일부분이 떨어져나갔다.",
     "당신의 얼룩진 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your speckled scales disappear.",
     "Your speckled scales recede somewhat.",
     "Your speckled scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 오렌지색의 비늘이 사라져 버렸다.",
     "당신의 오렌지색의 비늘의 일부분이 떨어져나갔다.",
     "당신의 오렌지색의 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your orange scales disappear.", "Your orange scales recede somewhat.",
     "Your orange scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 남색의 비늘이 사라져 버렸다.",
     "당신의 남색의 비늘의 일부분이 떨어져나갔다.",
     "당신의 남색의 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your indigo scales disappear.", "Your indigo scales recede somewhat.",
     "Your indigo scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 혹이 솟은 붉은 비늘이 사라져 버렸다.",
     "당신의 혹이 솟은 붉은 비늘의 일부분이 떨어져나갔다.",
     "당신의 혹이 솟은 붉은 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your knobbly red scales disappear.",
     "Your knobbly red scales recede somewhat.",
     "Your knobbly red scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 반짝이는 비늘이 사라져 버렸다.",
     "당신의 반짝이는 비늘의 일부분이 떨어져나갔다.",
     "당신의 반짝이는 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your iridescent scales disappear.",
     "Your iridescent scales recede somewhat.",
     "Your iridescent scales recede somewhat."},
#endif

#ifdef JP 
    {"당신의 문양이 세겨진 비늘이 사라져 버렸다.",
     "당신의 문양이 세겨진 비늘의 일부분이 떨어져나갔다.",
     "당신의 문양이 세겨진 비늘의 일부분이 떨어져나갔다."},
#else
    {"Your patterned scales disappear.",
     "Your patterned scales recede somewhat.",
     "Your patterned scales recede somewhat."},
#endif
};

/*
   Chance out of 10 that mutation will be given/removed randomly. 0 means never.
 */
const char mutation_rarity[] = {
    10,                         // tough skin
    8,                          // str
    8,                          // int
    8,                          // dex
    2,                          // gr scales
    1,                          // bl scales
    2,                          // grey scales
    1,                          // bone
    1,                          // repuls field
    4,                          // res poison
// 10
    5,                          // carn
    5,                          // herb
    4,                          // res fire
    4,                          // res cold
    2,                          // res elec
    3,                          // regen
    10,                         // fast meta
    7,                          // slow meta
    10,                         // abil loss
    10,                         // ""
// 20
    10,                         // ""
    2,                          // tele control
    3,                          // teleport
    5,                          // res magic
    1,                          // run
    2,                          // see invis
    8,                          // deformation
    2,                          // teleport at will
    8,                          // spit poison
    3,                          // sense surr
// 30
    4,                          // breathe fire
    3,                          // blink
    7,                          // horns
    10,                         // strong/stiff muscles
    10,                         // weak/loose muscles
    6,                          // forgetfulness
    6,                          // clarity (as the amulet)
    7,                          // berserk/temper
    10,                         // deterioration
    10,                         // blurred vision
// 40
    4,                          // resist mutation
    10,                         // frail
    5,                          // robust
/* Some demonic powers start here: */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
// 50
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,                          //jmf: claws
    1,                          //jmf: hooves
// 60
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
// 70
    2,                          // red scales
    1,                          // nac scales
    2,                          // r-grey scales
    1,                          // metal scales
    2,                          // black scales
    2,                          // wh scales
    2,                          // yel scales
    2,                          // brown scales
    2,                          // blue scales
    2,                          // purple scales
// 80
    2,                          // speckled scales
    2,                          // orange scales
    2,                          // indigo scales
    1,                          // kn red scales
    1,                          // irid scales
    1,                          // pattern scales
    0,                          //
    0,                          //
    0,                          //
    0                           //
};

void display_mutations(void)
{
    int i;
    int j = 0;
#ifdef JP  
    const char *mut_title = "선천적, 불가사의한 & 돌연변이 능력";
#else
    const char *mut_title = "Innate abilities, Weirdness & Mutations";
#endif
    const int num_lines = get_number_of_lines(); 

#ifdef DOS_TERM
    char buffer[4800];

    window(1, 1, 80, 25);
    gettext(1, 1, 80, 25, buffer);
#endif

    clrscr();
    textcolor(WHITE);

    // center title
    i = 40 - strlen(mut_title) / 2;
    if (i<1) i=1;
    gotoxy(i, 1);
    cprintf(mut_title);
    gotoxy(1,3);
    textcolor(LIGHTBLUE);  //textcolor for inborn abilities and weirdness

    switch (you.species)   //mv: following code shows innate abilities - if any
    {
    case SP_MERFOLK:
#ifdef JP 
        cprintf("당신은 물속에서는 본래의 모습으로 돌아온다." EOL);
#else
        cprintf("You revert to your normal form in water." EOL);
#endif
        j++;
        break;

    case SP_NAGA:
        // breathe poison replaces spit poison:
        if (!you.mutation[MUT_BREATHE_POISON])
#ifdef JP 
            cprintf("당신은 독을 뱉을 수 있다." EOL);
#else
            cprintf("You can spit poison." EOL);
#endif
        else
#ifdef JP 
            cprintf("당신은 독구름을 뿜어낼 수 있다." EOL);
#else
            cprintf("You can exhale a cloud of poison." EOL);
#endif

#ifdef JP 
        cprintf("당신은 독에 대하여 완전한 내성을 가지고 있다." EOL);
        cprintf("당신은 투명체를 볼 수 있다." EOL);
#else
        cprintf("Your system is immune to poisons." EOL);
        cprintf("You can see invisible." EOL);
#endif
        j += 3;
        break;

    case SP_GNOME:
#ifdef JP 
        cprintf("당신은 주변을 감지할 수 있다." EOL);
#else
        cprintf("You can sense your surroundings." EOL);
#endif
        j++;
        break;

    case SP_TROLL:
#ifdef JP 
        cprintf("당신의 몸은 빠른속도로 상처를 재생하여 복구한다." EOL);
#else
        cprintf("Your body regenerates quickly." EOL);
#endif
        j++;
        break;

    case SP_GHOUL:
#ifdef JP 
        cprintf("당신의 몸은 썩어 문드러진다." EOL);
        cprintf("당신은 육식성이다." EOL);
#else
        cprintf("Your body is rotting away." EOL);
        cprintf("You are carnivorous." EOL);
#endif
        j += 2;
        break;

    case SP_KOBOLD:
#ifdef JP 
        cprintf("당신은 육식성이다." EOL);
#else
        cprintf("You are carnivorous." EOL);
#endif
        j++;
        break;

    case SP_GREY_ELF:
        if (you.experience_level > 4)
        {
#ifdef JP 
            cprintf("당신은 매우 매력적이다." EOL);
#else
            cprintf("You are very charming." EOL);
#endif
            j++;
        }
        break;

    case SP_KENKU:
        if (you.experience_level > 4)
        {
#ifdef JP 
            cprintf("당신은 ");
            cprintf((you.experience_level > 14) ? "오랬동안 날 수 있다." EOL : "날 수 있다."
                    EOL);
#else
            cprintf("You can fly");
            cprintf((you.experience_level > 14) ? " continuously." EOL : "."
                    EOL);
#endif
            j++;
        }
        break;

    case SP_MUMMY:
#ifdef JP 
        cprintf("당신은 ");
        cprintf((you.experience_level > 25) ? "죽음의 힘으로 부여받은 매우 강한 손길이 있다." :
                ((you.experience_level > 12) ? "죽음의 힘으로 부여받은 강한 손길이 있다." : "죽음의 힘으로 부여받은 손길이 있다."));
        cprintf("" EOL);
#else
        cprintf("You are");
        cprintf((you.experience_level > 25) ? " very strongly" :
                ((you.experience_level > 12) ? " strongly" : ""));
        cprintf(" in touch with the powers of death." EOL);
#endif
        j++;

        if (you.experience_level >= 12)
        {
#ifdef JP 
            cprintf("당신은 마법 에너지를 불어넣어 육신을 회복할 수 있다." EOL);
#else
            cprintf("You can restore your body by infusing magical energy." EOL);
#endif
            j++;
        }
        break;

    case SP_GREEN_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("당신은 독에 대하여 저항력이 있다." EOL);
            cprintf("당신은 독 브레스를 뿜을 수 있다." EOL);
#else
            cprintf("You are resistant to poison." EOL);
            cprintf("You can breathe poison." EOL);
#endif
            j += 2;
        }
        break;

    case SP_RED_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("당신은 화염 브레스를 뿜을 수 있다." EOL);
#else
            cprintf("You can breathe fire." EOL);
#endif
            j++;
        }
        if (you.experience_level > 17)
        {
#ifdef JP 
            cprintf("당신은 불에 대하여 저항력이 있다." EOL);
#else
            cprintf("You are resistant to fire." EOL);
#endif
            j++;
        }
        break;

    case SP_WHITE_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("당신은 냉기 브레스를 뿜을 수 있다." EOL);
#else
            cprintf("You can breathe frost." EOL);
#endif
            j++;
        }
        if (you.experience_level > 17)
        {
#ifdef JP 
            cprintf("당신은 추위에 대하여 저항력이 있다." EOL);
#else
            cprintf("You are resistant to cold." EOL);
#endif
            j++;
        }
        break;

    case SP_BLACK_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("당신은 번개의 브레스를 뿜을 수 있다." EOL);
#else
            cprintf("You can breathe lightning." EOL);
#endif
            j++;
        }
        if (you.experience_level > 17)
        {
#ifdef JP 
            cprintf("당신은 번개에 대하여 저항력이 있다." EOL);
#else
            cprintf("You are resistant to lightning." EOL);
#endif
            j++;
        }
        break;

    case SP_GOLDEN_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("당신은 강한 산을 뱉을 수 있다." EOL);
#else
            cprintf("You can spit acid." EOL);
#endif
            j++;
        }
        break;

    case SP_PURPLE_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("당신은 마력의 브레스를 뿜을 수 있다." EOL);
#else
            cprintf("You can breathe power." EOL);
#endif
            j++;
        }
        break;

    case SP_MOTTLED_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("당신은 끈적이는 화염의 브레스를 뿜을 수 있다." EOL);
#else
            cprintf("You can breathe sticky flames." EOL);
#endif
            j++;
        }
        break;

    case SP_PALE_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("당신은 수증기 브레스를 뿜을 수 있다." EOL);
#else
            cprintf("You can breathe steam." EOL);
#endif
            j++;
        }
        break;
    }                           //end switch - innate abilities

    textcolor(LIGHTGREY);

    for (i = 0; i < 100; i++)
    {
        if (you.mutation[i] != 0)
        {
            // this is already handled above:
            if (you.species == SP_NAGA && i == MUT_BREATHE_POISON)
                continue;

            j++;
            textcolor(LIGHTGREY);

            if (j > num_lines - 4)
            {
                gotoxy( 1, num_lines - 1 );
#ifdef JP 
                cprintf("-다음장-");
#else
                cprintf("-more-");
#endif

                if (getch() == 0)
                    getch();

                clrscr();

                // center title
                int x = 40 - strlen(mut_title) / 2;
                if (x < 1) 
                    x = 1;

                gotoxy(x, 1);
                textcolor(WHITE);
                cprintf(mut_title);
                textcolor(LIGHTGREY);
                gotoxy(1,3);
                j = 1;
            }

            /* mutation is actually a demonic power */
            if (you.demon_pow[i] != 0)
                textcolor(RED);

            /* same as above, but power is enhanced by mutation */
            if (you.demon_pow[i] != 0 && you.demon_pow[i] < you.mutation[i])
                textcolor(LIGHTRED);

            cprintf( mutation_name( i ) );
            cprintf(EOL);
        }
    }

    if (j == 0)
#ifdef JP 
        cprintf( "당신은 변이 상태가 아니다." EOL );
#else
        cprintf( "You are not a mutant." EOL );
#endif

#ifndef USE_MULTIWIN // skip getch
    if (getch() == 0)
        getch();
#endif

#ifdef DOS_TERM
    puttext(1, 1, 80, 25, buffer);
#endif

    //cprintf("xxxxxxxxxxxxx");
    //last_requested = 0;

    return;
}                               // end display_mutations()

bool mutate(int which_mutation, bool failMsg)
{
    char mutat = which_mutation;
    bool force_mutation = false;        // is mutation forced?
    int  i;

    if (which_mutation >= 1000) // must give mutation without failure
    {
        force_mutation = true;
        mutat -= 1000;
        which_mutation -= 1000;
    }

    // Undead bodies don't mutate, they fall apart. -- bwr
    if (you.is_undead) 
    {
        if (force_mutation 
            || (!wearing_amulet(AMU_RESIST_MUTATION) && coinflip()))
        {
#ifdef JP 
            mpr( "당신의 육체는 부패했다!" );
#else
            mpr( "Your body decomposes!" );
#endif

            if (coinflip())
                lose_stat( STAT_RANDOM, 1 );
            else
            {
                ouch( 3, 0, KILLED_BY_ROTTING );
                rot_hp( roll_dice( 1, 3 ) );
            }

            return (true);
        }

        if (failMsg)
#ifdef JP 
            mpr("잠시동안 기묘한 감각이 되었다.");
#else
            mpr("You feel odd for a moment.");
#endif

        return (false);
    }

    if (wearing_amulet(AMU_RESIST_MUTATION)
        && !force_mutation && !one_chance_in(10))
    {
        if (failMsg)
#ifdef JP 
            mpr("잠시동안 기묘한 감각이 되었다.");
#else
            mpr("You feel odd for a moment.");
#endif

        return (false);
    }

    if (you.mutation[MUT_MUTATION_RESISTANCE]
        && !force_mutation
        && (you.mutation[MUT_MUTATION_RESISTANCE] == 3 || !one_chance_in(3)))
    {
        if (failMsg)
#ifdef JP 
            mpr("잠시동안 기묘한 감각이 되었다.");
#else
            mpr("You feel odd for a moment.");
#endif

        return (false);
    }

    if (which_mutation == 100 && random2(15) < how_mutated())
    {
        if (!force_mutation && !one_chance_in(3))
            return (false);
        else
            return (delete_mutation(100));
    }

    if (which_mutation == 100)
    {
        do
        {
            mutat = random2(NUM_MUTATIONS);

            if (one_chance_in(1000))
                return false;
        }
        while ((you.mutation[mutat] >= 3
                && (mutat != MUT_STRONG && mutat != MUT_CLEVER
                    && mutat != MUT_AGILE) && (mutat != MUT_WEAK
                                               && mutat != MUT_DOPEY
                                               && mutat != MUT_CLUMSY))
               || you.mutation[mutat] > 13
               || random2(10) >= mutation_rarity[mutat] + you.demon_pow[mutat]);
    }

    if (you.mutation[mutat] >= 3
        && (mutat != MUT_STRONG && mutat != MUT_CLEVER && mutat != MUT_AGILE)
        && (mutat != MUT_WEAK && mutat != MUT_DOPEY && mutat != MUT_CLUMSY))
    {
        return false;
    }

    if (you.mutation[mutat] > 13 && !force_mutation)
        return false;

    // These can be forced by demonspawn
    if ((mutat == MUT_TOUGH_SKIN
         || (mutat >= MUT_GREEN_SCALES && mutat <= MUT_BONEY_PLATES)
         || (mutat >= MUT_RED_SCALES && mutat <= MUT_PATTERNED_SCALES))
        && body_covered() >= 3 && !force_mutation)
    {
        return false;
    }

    if (mutat == MUT_HORNS && you.species == SP_MINOTAUR)
        return false;

    // nagas have see invis and res poison and can spit poison
    if (you.species == SP_NAGA)
    {
        if (mutat == MUT_ACUTE_VISION || mutat == MUT_POISON_RESISTANCE)
            return false;

        // gdl: spit poison 'upgrades' to breathe poison.  Why not..
        if (mutat == MUT_SPIT_POISON)
        {
            if (coinflip())
                return false;
            {
                mutat = MUT_BREATHE_POISON;

                // breathe poison replaces spit poison (so it takes the slot)
                for (i = 0; i < 52; i++)
                {
                    if (you.ability_letter_table[i] == ABIL_SPIT_POISON)
                        you.ability_letter_table[i] = ABIL_BREATHE_POISON;
                }
            }
        }
    }

    // gnomes can already sense surroundings
    if (you.species == SP_GNOME && mutat == MUT_MAPPING)
        return false;

    // spriggans already run at max speed (centaurs can get a bit faster)
    if (you.species == SP_SPRIGGAN && mutat == MUT_FAST)
        return false;

    // this might have issues if we allowed it -- bwr
    if (you.species == SP_KOBOLD 
        && (mutat == MUT_CARNIVOROUS || mutat == MUT_HERBIVOROUS))
    {
        return (false);
    }

    // This one can be forced by demonspawn
    if (mutat == MUT_REGENERATION
        && you.mutation[MUT_SLOW_METABOLISM] > 0 && !force_mutation)
    {
        return false;           /* if you have a slow metabolism, no regen */
    }

    if (mutat == MUT_SLOW_METABOLISM && you.mutation[MUT_REGENERATION] > 0)
        return false;           /* if you have a slow metabolism, no regen */

    // This one can be forced by demonspawn
    if (mutat == MUT_ACUTE_VISION
        && you.mutation[MUT_BLURRY_VISION] > 0 && !force_mutation)
    {
        return false;
    }

    if (mutat == MUT_BLURRY_VISION && you.mutation[MUT_ACUTE_VISION] > 0)
        return false;           /* blurred vision/see invis */

    //jmf: added some checks for new mutations
    if (mutat == MUT_STINGER
        && !(you.species == SP_NAGA || player_genus(GENPC_DRACONIAN)))
    {
        return false;
    }

    // putting boots on after they are forced off. -- bwr
    if (mutat == MUT_HOOVES
        && (you.species == SP_NAGA || you.species == SP_CENTAUR
            || you.species == SP_KENKU || player_genus(GENPC_DRACONIAN)))
    {
        return false;
    }

    if (mutat == MUT_BIG_WINGS && !player_genus(GENPC_DRACONIAN))
        return false;

    //jmf: added some checks for new mutations
#ifdef JP 
    mpr("돌연변이 상태가 되었다.", MSGCH_MUTATION);
#else
    mpr("You mutate.", MSGCH_MUTATION);
#endif

    // find where these things are actually changed
    // -- do not globally force redraw {dlb}
    you.redraw_hit_points = 1;
    you.redraw_magic_points = 1;
    you.redraw_armour_class = 1;
    you.redraw_evasion = 1;
    you.redraw_experience = 1;
    you.redraw_gold = 1;
    //you.redraw_hunger = 1;

    switch (mutat)
    {
    case MUT_STRONG:
        if (you.mutation[MUT_WEAK] > 0)
        {
            delete_mutation(MUT_WEAK);
            return true;
        }
        // replaces earlier, redundant code - 12mar2000 {dlb}
        modify_stat(STAT_STRENGTH, 1, false);
        break;

    case MUT_CLEVER:
        if (you.mutation[MUT_DOPEY] > 0)
        {
            delete_mutation(MUT_DOPEY);
            return true;
        }
        // replaces earlier, redundant code - 12mar2000 {dlb}
        modify_stat(STAT_INTELLIGENCE, 1, false);
        break;

    case MUT_AGILE:
        if (you.mutation[MUT_CLUMSY] > 0)
        {
            delete_mutation(MUT_CLUMSY);
            return true;
        }
        // replaces earlier, redundant code - 12mar2000 {dlb}
        modify_stat(STAT_DEXTERITY, 1, false);
        break;

    case MUT_WEAK:
        if (you.mutation[MUT_STRONG] > 0)
        {
            delete_mutation(MUT_STRONG);
            return true;
        }
        modify_stat(STAT_STRENGTH, -1, true);
        mpr(gain_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_DOPEY:
        if (you.mutation[MUT_CLEVER] > 0)
        {
            delete_mutation(MUT_CLEVER);
            return true;
        }
        modify_stat(STAT_INTELLIGENCE, -1, true);
        mpr(gain_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_CLUMSY:
        if (you.mutation[MUT_AGILE] > 0)
        {
            delete_mutation(MUT_AGILE);
            return true;
        }
        modify_stat(STAT_DEXTERITY, -1, true);
        mpr(gain_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_REGENERATION:
        if (you.mutation[MUT_SLOW_METABOLISM] > 0)
        {
            // Should only get here from demonspawn, where our innate
            // ability will clear away the counter-mutation.
            while (delete_mutation(MUT_SLOW_METABOLISM))
                ;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_ACUTE_VISION:
        if (you.mutation[MUT_BLURRY_VISION] > 0)
        {
            // Should only get here from demonspawn, where our inate
            // ability will clear away the counter-mutation.
            while (delete_mutation(MUT_BLURRY_VISION))
                ;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_CARNIVOROUS:
        if (you.mutation[MUT_HERBIVOROUS] > 0)
        {
            delete_mutation(MUT_HERBIVOROUS);
            return true;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_HERBIVOROUS:
        if (you.mutation[MUT_CARNIVOROUS] > 0)
        {
            delete_mutation(MUT_CARNIVOROUS);
            return true;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_SHOCK_RESISTANCE:
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_FAST_METABOLISM:
        if (you.mutation[MUT_SLOW_METABOLISM] > 0)
        {
            delete_mutation(MUT_SLOW_METABOLISM);
            return true;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_SLOW_METABOLISM:
        if (you.mutation[MUT_FAST_METABOLISM] > 0)
        {
            delete_mutation(MUT_FAST_METABOLISM);
            return true;
        }
        //if (you.mutation[mutat] == 0 || you.mutation[mutat] == 2)
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_TELEPORT_CONTROL:
        you.attribute[ATTR_CONTROL_TELEPORT]++;
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;


    case MUT_HOOVES:            //jmf: like horns
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        if (you.equip[EQ_BOOTS] != -1)
        {
            FixedVector < char, 8 > removed;

            for (int i = EQ_WEAPON; i < EQ_RIGHT_RING; i++)
            {
                removed[i] = 0;
            }

            removed[EQ_BOOTS] = 1;
            remove_equipment(removed);
        }
        break;

    case MUT_CLAWS:
        mpr( gain_mutation[ mutat ][ you.mutation[mutat] ], MSGCH_MUTATION );

        // gloves aren't prevented until level three
        if (you.mutation[ mutat ] >= 3 && you.equip[ EQ_GLOVES ] != -1)
        {
            FixedVector < char, 8 > removed;

            for (int i = EQ_WEAPON; i < EQ_RIGHT_RING; i++)
            {
                removed[i] = 0;
            }

            removed[ EQ_GLOVES ] = 1;
            remove_equipment( removed );
        }
        break;

    case MUT_HORNS:             // horns force your helmet off
        {
            mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);

            if (you.equip[EQ_HELMET] != -1
                && you.inv[you.equip[EQ_HELMET]].plus2 > 1)
            {
                break;          // horns don't push caps/wizard hats off
            }

            FixedVector < char, 8 > removed;

            for (int i = EQ_WEAPON; i < EQ_RIGHT_RING; i++)
            {
                removed[i] = 0;
            }

            removed[EQ_HELMET] = 1;
            remove_equipment(removed);
        }
        break;

    case MUT_STRONG_STIFF:
        if (you.mutation[MUT_FLEXIBLE_WEAK] > 0)
        {
            delete_mutation(MUT_FLEXIBLE_WEAK);
            return true;
        }
        modify_stat(STAT_STRENGTH, 1, true);
        modify_stat(STAT_DEXTERITY, -1, true);
        mpr(gain_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_FLEXIBLE_WEAK:
        if (you.mutation[MUT_STRONG_STIFF] > 0)
        {
            delete_mutation(MUT_STRONG_STIFF);
            return true;
        }
        modify_stat(STAT_STRENGTH, -1, true);
        modify_stat(STAT_DEXTERITY, 1, true);
        mpr(gain_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_FRAIL:
        if (you.mutation[MUT_ROBUST] > 0)
        {
            delete_mutation(MUT_ROBUST);
            return true;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        you.mutation[mutat]++;
        calc_hp();
        return true;

    case MUT_ROBUST:
        if (you.mutation[MUT_FRAIL] > 0)
        {
            delete_mutation(MUT_FRAIL);
            return true;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        you.mutation[mutat]++;
        calc_hp();
        return true;

    case MUT_BLACK_SCALES:
    case MUT_BONEY_PLATES:
        modify_stat(STAT_DEXTERITY, -1, true);
        // deliberate fall-through
    default:
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_GREY2_SCALES:
        if (you.mutation[mutat] != 1)
            modify_stat(STAT_DEXTERITY, -1, true);

        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_METALLIC_SCALES:
        if (you.mutation[mutat] == 0)
            modify_stat(STAT_DEXTERITY, -2, true);
        else
            modify_stat(STAT_DEXTERITY, -1, true);

        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_RED2_SCALES:
    case MUT_YELLOW_SCALES:
        if (you.mutation[mutat] != 0)
            modify_stat(STAT_DEXTERITY, -1, true);

        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;
    }

    you.mutation[mutat]++;

    /* remember, some mutations don't get this far (eg frail) */
    return true;
}                               // end mutation()

int how_mutated(void)
{
    int j = 0;

    for (int i = 0; i < 100; i++)
    {
        if (you.mutation[i] && you.demon_pow[i] < you.mutation[i])
        {
            // these allow for 14 levels:
            if (i == MUT_STRONG || i == MUT_CLEVER || i == MUT_AGILE
                || i == MUT_WEAK || i == MUT_DOPEY || i == MUT_CLUMSY)
            {
                j += (you.mutation[i] / 5 + 1);
            }
            else 
            {
                j += you.mutation[i];
            }
        }
    }

#if DEBUG_DIAGNOSTICS
#ifdef JP 
    snprintf( info, INFO_SIZE, "levels: %d", j );
#else
    snprintf( info, INFO_SIZE, "levels: %d", j );
#endif
    mpr( info, MSGCH_DIAGNOSTICS );
#endif

    return (j);
}                               // end how_mutated()

bool delete_mutation(char which_mutation)
{
    char mutat = which_mutation;
    int i;

    if (you.mutation[MUT_MUTATION_RESISTANCE] > 1
        && (you.mutation[MUT_MUTATION_RESISTANCE] == 3 || coinflip()))
    {
#ifdef JP 
        mpr("잠시동안 뭔가 이상한 감각을 느꼈다.");
#else
        mpr("You feel rather odd for a moment.");
#endif
        return false;
    }

    if (which_mutation == 100)
    {
        do
        {
            mutat = random2(NUM_MUTATIONS);
            if (one_chance_in(1000))
                return false;
        }
        while ((you.mutation[mutat] == 0
                   && (mutat != MUT_STRONG && mutat != MUT_CLEVER && mutat != MUT_AGILE) 
                   && (mutat != MUT_WEAK && mutat != MUT_DOPEY && mutat != MUT_CLUMSY))
               || random2(10) >= mutation_rarity[mutat]
               || you.demon_pow[mutat] >= you.mutation[mutat]);
    }

    if (you.mutation[mutat] == 0)
        return false;

    if (you.demon_pow[mutat] >= you.mutation[mutat])
        return false;

#ifdef JP 
    mpr("돌연변이 상태가 되었다.", MSGCH_MUTATION);
#else
    mpr("You mutate.", MSGCH_MUTATION);
#endif

    switch (mutat)
    {
    case MUT_STRONG:
        modify_stat(STAT_STRENGTH, -1, true);
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_CLEVER:
        modify_stat(STAT_INTELLIGENCE, -1, true);
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_AGILE:
        modify_stat(STAT_DEXTERITY, -1, true);
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_WEAK:
        modify_stat(STAT_STRENGTH, 1, false);
        break;

    case MUT_DOPEY:
        modify_stat(STAT_INTELLIGENCE, 1, false);
        break;

    case MUT_CLUMSY:
        // replaces earlier, redundant code - 12mar2000 {dlb}
        modify_stat(STAT_DEXTERITY, 1, false);
        break;

    case MUT_SHOCK_RESISTANCE:
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_FAST_METABOLISM:
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_SLOW_METABOLISM:
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_TELEPORT_CONTROL:
        you.attribute[ATTR_CONTROL_TELEPORT]--;
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_STRONG_STIFF:
        modify_stat(STAT_STRENGTH, -1, true);
        modify_stat(STAT_DEXTERITY, 1, true);
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_FLEXIBLE_WEAK:
        modify_stat(STAT_STRENGTH, 1, true);
        modify_stat(STAT_DEXTERITY, -1, true);
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_FRAIL:
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        if (you.mutation[mutat] > 0)
            you.mutation[mutat]--;
        calc_hp();
        return true;

    case MUT_ROBUST:
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        if (you.mutation[mutat] > 0)
            you.mutation[mutat]--;
        calc_hp();
        return true;

    case MUT_BLACK_SCALES:
    case MUT_BONEY_PLATES:
        modify_stat(STAT_DEXTERITY, 1, true);

    default:
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_GREY2_SCALES:
        if (you.mutation[mutat] != 2)
            modify_stat(STAT_DEXTERITY, 1, true);
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_METALLIC_SCALES:
        if (you.mutation[mutat] == 1)
            modify_stat(STAT_DEXTERITY, 2, true);
        else
            modify_stat(STAT_DEXTERITY, 1, true);

        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_RED2_SCALES:
    case MUT_YELLOW_SCALES:
        if (you.mutation[mutat] != 1)
            modify_stat(STAT_DEXTERITY, 1, true);

        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_BREATHE_POISON:
        // can't be removed yet, but still covered:
        if (you.species == SP_NAGA)
        {
            // natural ability to spit poison retakes the slot
            for (i = 0; i < 52; i++)
            {
                if (you.ability_letter_table[i] == ABIL_BREATHE_POISON)
                    you.ability_letter_table[i] = ABIL_SPIT_POISON;
            }
        }
        break;
    }

    // find where these things are actually altered
    /// -- do not globally force redraw {dlb}
    you.redraw_hit_points = 1;
    you.redraw_magic_points = 1;
    you.redraw_armour_class = 1;
    you.redraw_evasion = 1;
    you.redraw_experience = 1;
    you.redraw_gold = 1;
    //you.redraw_hunger = 1;

    if (you.mutation[mutat] > 0)
        you.mutation[mutat]--;

    return true;
}                               // end delete_mutation()

char body_covered(void)
{
    /* checks how much of your body is covered by scales etc */
    char covered = 0;

    if (you.species == SP_NAGA)
        covered++;

    if (player_genus(GENPC_DRACONIAN))
        return 3;

    covered += you.mutation[MUT_TOUGH_SKIN];
    covered += you.mutation[MUT_GREEN_SCALES];
    covered += you.mutation[MUT_BLACK_SCALES];
    covered += you.mutation[MUT_GREY_SCALES];
    covered += you.mutation[MUT_BONEY_PLATES];
    covered += you.mutation[MUT_RED_SCALES];
    covered += you.mutation[MUT_NACREOUS_SCALES];
    covered += you.mutation[MUT_GREY2_SCALES];
    covered += you.mutation[MUT_METALLIC_SCALES];
    covered += you.mutation[MUT_BLACK2_SCALES];
    covered += you.mutation[MUT_WHITE_SCALES];
    covered += you.mutation[MUT_YELLOW_SCALES];
    covered += you.mutation[MUT_BROWN_SCALES];
    covered += you.mutation[MUT_BLUE_SCALES];
    covered += you.mutation[MUT_PURPLE_SCALES];
    covered += you.mutation[MUT_SPECKLED_SCALES];
    covered += you.mutation[MUT_ORANGE_SCALES];
    covered += you.mutation[MUT_INDIGO_SCALES];
    covered += you.mutation[MUT_RED2_SCALES];
    covered += you.mutation[MUT_IRIDESCENT_SCALES];
    covered += you.mutation[MUT_PATTERNED_SCALES];

    return covered;
}

const char *mutation_name( char which_mutat, int level )
{
    static char mut_string[INFO_SIZE];

    // level == -1 means default action of current level
    if (level == -1)
        level = you.mutation[ which_mutat ];

    if (which_mutat == MUT_STRONG || which_mutat == MUT_CLEVER
        || which_mutat == MUT_AGILE || which_mutat == MUT_WEAK
        || which_mutat == MUT_DOPEY || which_mutat == MUT_CLUMSY)
    {
#ifdef JP 
        snprintf( mut_string, sizeof( mut_string ), "%s%d)", 
#else
        snprintf( mut_string, sizeof( mut_string ), "%s%d).", 
#endif
                  mutation_descrip[ which_mutat ][0], level );

        return (mut_string);
    }

    // Some mutations only have one "level", and it's better
    // to show the first level description than a blank description.
    if (mutation_descrip[ which_mutat ][ level - 1 ][0] == '\0')
        return (mutation_descrip[ which_mutat ][ 0 ]);
    else 
        return (mutation_descrip[ which_mutat ][ level - 1 ]);
}                               // end mutation_name()

/* Use an attribute counter for how many demonic mutations a dspawn has */
void demonspawn(void)
{
    int whichm = -1;
    char howm = 1;
    int counter = 0;

    const int scale_levels = body_covered();

    you.attribute[ATTR_NUM_DEMONIC_POWERS]++;

#ifdef JP 
    mpr("당신의 악마적인 피가 나타나기 시작한다...", MSGCH_INTRINSIC_GAIN);
#else
    mpr("Your demonic ancestry asserts itself...", MSGCH_INTRINSIC_GAIN);
#endif

    // Merged the demonspawn lists into a single loop.  Now a high level
    // character can potentially get mutations from the low level list if 
    // its having trouble with the high level list.
    do
    {
        if (you.experience_level >= 10)
        {
            if (you.skills[SK_CONJURATIONS] < 5)
            {                       // good conjurers don't get bolt of draining
                whichm = MUT_SMITE;
                howm = 1;
            }

            if (you.skills[SK_CONJURATIONS] < 10 && one_chance_in(4))
            {                       // good conjurers don't get hellfire
                whichm = MUT_HURL_HELLFIRE;
                howm = 1;
            }

            if (you.skills[SK_SUMMONINGS] < 5 && one_chance_in(3))
            {                       // good summoners don't get summon demon
                whichm = MUT_SUMMON_DEMONS;
                howm = 1;
            }

            if (one_chance_in(8))
            {
                whichm = MUT_MAGIC_RESISTANCE;
                howm = (coinflip() ? 2 : 3);
            }

            if (one_chance_in(12))
            {
                whichm = MUT_FAST;
                howm = 1;
            }

            if (one_chance_in(7))
            {
                whichm = MUT_TELEPORT_AT_WILL;
                howm = 2;
            }

            if (one_chance_in(10))
            {
                whichm = MUT_REGENERATION;
                howm = (coinflip() ? 2 : 3);
            }

            if (one_chance_in(12))
            {
                whichm = MUT_SHOCK_RESISTANCE;
                howm = 1;
            }

            if (!you.mutation[MUT_CALL_TORMENT] && one_chance_in(15))
            {
                whichm = MUT_TORMENT_RESISTANCE;
                howm = 1;
            }

            if (one_chance_in(12))
            {
                whichm = MUT_NEGATIVE_ENERGY_RESISTANCE;
                howm = 1 + random2(3);
            }

            if (!you.mutation[MUT_TORMENT_RESISTANCE] && one_chance_in(20))
            {
                whichm = MUT_CALL_TORMENT;
                howm = 1;
            }

            if (you.skills[SK_SUMMONINGS] < 5 && you.skills[SK_NECROMANCY] < 5
                && one_chance_in(12))
            {
                whichm = MUT_CONTROL_DEMONS;
                howm = 1;
            }

            if (you.skills[SK_TRANSLOCATIONS] < 5 && one_chance_in(15))
            {
                whichm = MUT_PANDEMONIUM;
                howm = 1;
            }

            if (you.religion != GOD_VEHUMET && one_chance_in(11))
            {
                whichm = MUT_DEATH_STRENGTH;
                howm = 1;
            }

            if (you.religion != GOD_VEHUMET && one_chance_in(11))
            {
                whichm = MUT_CHANNEL_HELL;
                howm = 1;
            }

            if (you.skills[SK_SUMMONINGS] < 3 && you.skills[SK_NECROMANCY] < 3
                && one_chance_in(10))
            {
                whichm = MUT_RAISE_DEAD;
                howm = 1;
            }

            if (you.skills[SK_UNARMED_COMBAT] > 5 && one_chance_in(14))
            {
                whichm = MUT_DRAIN_LIFE;
                howm = 1;
            }
        }

        // check here so we can see if we need to extent our options:
        if (whichm != -1 && you.mutation[whichm] != 0)
            whichm = -1;

        if (you.experience_level < 10 || (counter > 0 && whichm == -1))
        {
            if ((!you.mutation[MUT_THROW_FROST]         // only one of these
                    && !you.mutation[MUT_THROW_FLAMES]
                    && !you.mutation[MUT_BREATHE_FLAMES])
                && (!you.skills[SK_CONJURATIONS]        // conjurers seldomly
                    || one_chance_in(5))
                && (!you.skills[SK_ICE_MAGIC]           // already ice & fire?
                    || !you.skills[SK_FIRE_MAGIC]))
            {
                // try to give the flavour the character doesn't have:
                if (!you.skills[SK_FIRE_MAGIC])
                    whichm = MUT_THROW_FLAMES;
                else if (!you.skills[SK_ICE_MAGIC])
                    whichm = MUT_THROW_FROST;
                else
                    whichm = (coinflip() ? MUT_THROW_FLAMES : MUT_THROW_FROST);

                howm = 1;
            }

            if (!you.skills[SK_SUMMONINGS] && one_chance_in(3))
            {                           /* summoners don't get summon imp */
                whichm = (you.experience_level < 10) ? MUT_SUMMON_MINOR_DEMONS
                                                     : MUT_SUMMON_DEMONS;
                howm = 1;
            }

            if (one_chance_in(4))
            {
                whichm = MUT_POISON_RESISTANCE;
                howm = 1;
            }

            if (one_chance_in(4))
            {
                whichm = MUT_COLD_RESISTANCE;
                howm = 1;
            }

            if (one_chance_in(4))
            {
                whichm = MUT_HEAT_RESISTANCE;
                howm = 1;
            }

            if (one_chance_in(5))
            {
                whichm = MUT_ACUTE_VISION;
                howm = 1;
            }

            if (!you.skills[SK_POISON_MAGIC] && one_chance_in(7))
            {
                whichm = MUT_SPIT_POISON;
                howm = (you.experience_level < 10) ? 1 : 3;
            }

            if (one_chance_in(10))
            {
                whichm = MUT_MAPPING;
                howm = 3;
            }

            if (one_chance_in(12))
            {
                whichm = MUT_TELEPORT_CONTROL;
                howm = 1;
            }

            if (!you.mutation[MUT_THROW_FROST]         // not with these
                && !you.mutation[MUT_THROW_FLAMES]
                && !you.mutation[MUT_BREATHE_FLAMES]
                && !you.skills[SK_FIRE_MAGIC]          // or with fire already
                && one_chance_in(5))
            {
                whichm = MUT_BREATHE_FLAMES;
                howm = 2;
            }

            if (!you.skills[SK_TRANSLOCATIONS] && one_chance_in(12))
            {
                whichm = (you.experience_level < 10) ? MUT_BLINK
                                                     : MUT_TELEPORT_AT_WILL;
                howm = 2;
            }

            if (scale_levels < 3 && one_chance_in( 1 + scale_levels * 5 ))
            {
                const int bonus = (you.experience_level < 10) ? 0 : 1;
                int levels = 0;

                if (one_chance_in(10))
                {
                    whichm = MUT_TOUGH_SKIN;
                    levels = (coinflip() ? 2 : 3);
                }

                if (one_chance_in(24))
                {
                    whichm = MUT_GREEN_SCALES;
                    levels = (coinflip() ? 2 : 3);
                }

                if (one_chance_in(24))
                {
                    whichm = MUT_BLACK_SCALES;
                    levels = (coinflip() ? 2 : 3);
                }

                if (one_chance_in(24))
                {
                    whichm = MUT_GREY_SCALES;
                    levels = (coinflip() ? 2 : 3);
                }

                if (one_chance_in(12))
                {
                    whichm = MUT_RED_SCALES + random2(16);

                    switch (whichm)
                    {
                    case MUT_RED_SCALES:
                    case MUT_NACREOUS_SCALES:
                    case MUT_BLACK2_SCALES:
                    case MUT_WHITE_SCALES:
                    case MUT_BLUE_SCALES:
                    case MUT_SPECKLED_SCALES:
                    case MUT_ORANGE_SCALES:
                    case MUT_IRIDESCENT_SCALES:
                    case MUT_PATTERNED_SCALES:
                        levels = (coinflip() ? 2 : 3);
                        break;

                    default:
                        levels = (coinflip() ? 1 : 2);
                        break;
                    }
                }

                if (one_chance_in(30))
                {
                    whichm = MUT_BONEY_PLATES;
                    levels = (coinflip() ? 1 : 2);
                }

                if (levels)
                    howm = MINIMUM( 3 - scale_levels, levels + bonus );
            }

            if (one_chance_in(25))
            {
                whichm = MUT_REPULSION_FIELD;
                howm = (coinflip() ? 2 : 3);
            }

            if (one_chance_in( (you.experience_level < 10) ? 5 : 20 ))
            {
                whichm = MUT_HORNS;
                howm = (coinflip() ? 1 : 2);

                if (you.experience_level > 4 || one_chance_in(5))
                    howm++;
            }
        }

        if (whichm != -1 && you.mutation[whichm] != 0)
            whichm = -1;

        counter++;
    }
    while (whichm == -1 && counter < 5000);

    if (whichm == -1 || !perma_mutate( whichm, howm ))
    {
        /* unlikely but remotely possible */
        /* I know this is a cop-out */
        modify_stat(STAT_STRENGTH, 1, true);
        modify_stat(STAT_INTELLIGENCE, 1, true);
        modify_stat(STAT_DEXTERITY, 1, true);
#ifdef JP 
        mpr("컨디션이 매우 좋아졌다.", MSGCH_INTRINSIC_GAIN);
#else
        mpr("You feel much better now.", MSGCH_INTRINSIC_GAIN);
#endif
    }
}                               // end demonspawn()

bool perma_mutate(int which_mut, char how_much)
{
    char levels = 0;

    if (mutate(which_mut + 1000))
        levels++;

    if (how_much >= 2 && mutate(which_mut + 1000))
        levels++;

    if (how_much >= 3 && mutate(which_mut + 1000))
        levels++;

    you.demon_pow[which_mut] = levels;

    return (levels > 0);
}                               // end perma_mutate()

bool give_good_mutation(bool failMsg)
{
    int temp_rand = 0;          // probability determination {dlb}
    int which_good_one = 0;

    temp_rand = random2(25);

    which_good_one = ((temp_rand >= 24) ? MUT_TOUGH_SKIN :
                      (temp_rand == 23) ? MUT_STRONG :
                      (temp_rand == 22) ? MUT_CLEVER :
                      (temp_rand == 21) ? MUT_AGILE :
                      (temp_rand == 20) ? MUT_HEAT_RESISTANCE :
                      (temp_rand == 19) ? MUT_COLD_RESISTANCE :
                      (temp_rand == 18) ? MUT_SHOCK_RESISTANCE :
                      (temp_rand == 17) ? MUT_REGENERATION :
                      (temp_rand == 16) ? MUT_TELEPORT_CONTROL :
                      (temp_rand == 15) ? MUT_MAGIC_RESISTANCE :
                      (temp_rand == 14) ? MUT_FAST :
                      (temp_rand == 13) ? MUT_ACUTE_VISION :
                      (temp_rand == 12) ? MUT_GREEN_SCALES :
                      (temp_rand == 11) ? MUT_BLACK_SCALES :
                      (temp_rand == 10) ? MUT_GREY_SCALES :
                      (temp_rand ==  9) ? MUT_BONEY_PLATES :
                      (temp_rand ==  8) ? MUT_REPULSION_FIELD :
                      (temp_rand ==  7) ? MUT_POISON_RESISTANCE :
                      (temp_rand ==  6) ? MUT_TELEPORT_AT_WILL :
                      (temp_rand ==  5) ? MUT_SPIT_POISON :
                      (temp_rand ==  4) ? MUT_MAPPING :
                      (temp_rand ==  3) ? MUT_BREATHE_FLAMES :
                      (temp_rand ==  2) ? MUT_BLINK :
                      (temp_rand ==  1) ? MUT_CLARITY
                                        : MUT_ROBUST);

    return (mutate(which_good_one, failMsg));
}                               // end give_good_mutation()

bool give_bad_mutation(bool forceMutation, bool failMsg)
{
    int temp_rand = 0;          // probability determination {dlb}
    int which_bad_one = 0;

    temp_rand = random2(12);

    which_bad_one = ((temp_rand >= 11) ? MUT_CARNIVOROUS :
                     (temp_rand == 10) ? MUT_HERBIVOROUS :
                     (temp_rand ==  9) ? MUT_FAST_METABOLISM :
                     (temp_rand ==  8) ? MUT_WEAK :
                     (temp_rand ==  7) ? MUT_DOPEY :
                     (temp_rand ==  6) ? MUT_CLUMSY :
                     (temp_rand ==  5) ? MUT_TELEPORT :
                     (temp_rand ==  4) ? MUT_DEFORMED :
                     (temp_rand ==  3) ? MUT_LOST :
                     (temp_rand ==  2) ? MUT_DETERIORATION :
                     (temp_rand ==  1) ? MUT_BLURRY_VISION
                                       : MUT_FRAIL);

    if (forceMutation)
        which_bad_one += 1000;

    return (mutate(which_bad_one), failMsg);
}                               // end give_bad_mutation()

//jmf: might be useful somewhere (eg Xom or transmigration effect)
bool give_cosmetic_mutation()
{
    int mutation = -1;
    int how_much = 0;
    int counter = 0;

    do
    {
        mutation = MUT_DEFORMED;
        how_much = 1 + random2(3);

        if (one_chance_in(6))
        {
            mutation = MUT_ROBUST;
            how_much = 1 + random2(3);
        }

        if (one_chance_in(6))
        {
            mutation = MUT_FRAIL;
            how_much = 1 + random2(3);
        }

        if (one_chance_in(5))
        {
            mutation = MUT_TOUGH_SKIN;
            how_much = 1 + random2(3);
        }

        if (one_chance_in(4))
        {
            mutation = MUT_CLAWS;
            how_much = 1 + random2(3);
        }

        if (you.species != SP_CENTAUR && you.species != SP_NAGA
            && you.species != SP_KENKU && !player_genus(GENPC_DRACONIAN)
            && one_chance_in(5))
        {
            mutation = MUT_HOOVES;
            how_much = 1;
        }

        if (player_genus(GENPC_DRACONIAN) && one_chance_in(5))
        {
            mutation = MUT_BIG_WINGS;
            how_much = 1;
        }

        if (one_chance_in(5))
        {
            mutation = MUT_CARNIVOROUS;
            how_much = 1 + random2(3);
        }

        if (one_chance_in(6))
        {
            mutation = MUT_HORNS;
            how_much = 1 + random2(3);
        }

        if ((you.species == SP_NAGA || player_genus(GENPC_DRACONIAN))
            && one_chance_in(4))
        {
            mutation = MUT_STINGER;
            how_much = 1 + random2(3);
        }

        if (you.species == SP_NAGA && one_chance_in(6))
        {
            mutation = MUT_BREATHE_POISON;
            how_much = 1;
        }

        if (!(you.species == SP_NAGA || player_genus(GENPC_DRACONIAN))
            && one_chance_in(7))
        {
            mutation = MUT_SPIT_POISON;
            how_much = 1;
        }

        if (!(you.species == SP_NAGA || player_genus(GENPC_DRACONIAN))
            && one_chance_in(8))
        {
            mutation = MUT_BREATHE_FLAMES;
            how_much = 1 + random2(3);
        }

        if (you.mutation[mutation] > 0)
            how_much -= you.mutation[mutation];

        if (how_much < 0)
            how_much = 0;
    }
    while (how_much == 0 && counter++ < 5000);

    if (how_much != 0)
        return mutate(mutation);
    else
        return false;
}                               // end give_cosmetic_mutation()
