#include "combat_power.h"
#include <cmath>
#include <algorithm>

MappedStats mapStatType(
    const maple_combat_calculator::shared::MCCStat& stat,
    int mainStatType
) {
    MappedStats mapped = {0, 0, 0};

    switch (mainStatType) {
        case 1: // STR
            mapped.mainStat = stat.str();
            mapped.subStat = stat.dex();
            mapped.attackOrMagic = stat.attack_power();
            break;
        case 2: // DEX
            mapped.mainStat = stat.dex();
            mapped.subStat = stat.str();
            mapped.attackOrMagic = stat.attack_power();
            break;
        case 3: // INT
            mapped.mainStat = stat.int_();
            mapped.subStat = stat.luk();
            mapped.attackOrMagic = stat.magic_power();
            break;
        case 4: // LUK
            mapped.mainStat = stat.luk();
            mapped.subStat = stat.dex();
            mapped.attackOrMagic = stat.attack_power();
            break;
        case 5: // LUK (Dual Blade 등)
            mapped.mainStat = stat.luk();
            mapped.subStat = stat.str() + stat.dex();
            mapped.attackOrMagic = stat.attack_power();
            break;
        case 6: // ALL (Xenon)
            mapped.mainStat = stat.str() + stat.dex() + stat.luk();
            mapped.subStat = 0;
            mapped.attackOrMagic = stat.attack_power();
            break;
        case 7: // HP (Demon Avenger)
            mapped.mainStat = stat.hp() / 3.5; // HP 캐릭 보정 계수
            mapped.subStat = stat.str();
            mapped.attackOrMagic = stat.attack_power();
            break;
        default:
            // 알 수 없는 타입의 경우 가장 높은 수치를 사용하도록 예외 처리
            mapped.mainStat = std::max({stat.str(), stat.dex(), stat.int_(), stat.luk()});
            mapped.attackOrMagic = std::max(stat.attack_power(), stat.magic_power());
    }

    return mapped;
}

int calculateCombatPower(
    const maple_combat_calculator::shared::MCCStat& stat,
    int mainStatType,
    int weaponBaseAtt,
    int weaponSfAtt,
    int convertedWeaponAtt,
    double innateCritDmg,
    double innateBossDmg,
    double innateDmg,
    double innateFinalDmg
) {
    // StatType 로직을 별개 함수로 분리하여 사용합니다.
    MappedStats mapped = mapStatType(stat, mainStatType);

    // 무기 및 기본 제공(innate) 정보를 모두 외부에서 받아 기존 함수로 전달합니다.
    return calculateCombatPower(
        static_cast<int>(mapped.mainStat),
        static_cast<int>(mapped.subStat),
        static_cast<int>(mapped.attackOrMagic),
        weaponBaseAtt,
        weaponSfAtt,
        convertedWeaponAtt,
        0, // attPercent (이미 합산됨)
        stat.critical_damage(),
        innateCritDmg,
        stat.boss_damage(),
        innateBossDmg,
        stat.damage(),
        innateDmg,
        stat.final_damage(),
        innateFinalDmg
    );
}

int calculateCombatPower(
    int mainStat,
    int subStat,
    int flatAtt,
    int weaponBaseAtt,
    int weaponSfAtt,
    int convertedWeaponAtt,
    double attPercent,
    double critDmg,
    double innateCritDmg,
    double bossDmg,
    double innateBossDmg,
    double dmg,
    double innateDmg,
    double finalDmg,
    double innateFinalDmg
    )
{
    // totalFlatAtt 계산 로직: 
    // flatAtt(주인공격력/마력)에 무기 관련 보정치를 계산하여 합산합니다.
    double totalFlatAtt = flatAtt +
        std::floor((static_cast<double>(convertedWeaponAtt) / weaponBaseAtt - 1.0)
                   * (weaponBaseAtt + weaponSfAtt));

    double attMul = toMultiplier(attPercent);

    return static_cast<int>(std::floor(
        BASE_RATIO * (mainStat * 4 + subStat) *
        std::floor(totalFlatAtt * attMul) *
        toMultiplier(critDmg - innateCritDmg + BASE_CRIT_CONSTANT) *
        toMultiplier(bossDmg - innateBossDmg + dmg - innateDmg) *
        (toMultiplier(finalDmg) / innateFinalDmg)
    ));
}

inline double toMultiplier(const double percent) {
    return 1.0 + percent * 0.01;
}
