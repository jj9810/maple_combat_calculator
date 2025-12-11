#include "combat_power.h"
#include <cmath>

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
