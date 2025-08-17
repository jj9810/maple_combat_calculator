//
// Created by ubuntu on 25. 8. 10..
//

#ifndef MCC_SKILL_DAMAGE_H
#define MCC_SKILL_DAMAGE_H

constexpr long long DEFAULT_MAX_DAMAGE = 700000000000;

#define MAX_DAMAGE_CAP true

constexpr double CRIT_RATIO_MIN = 1.2;
constexpr double CRIT_RATIO_MEAN = 1.35;
constexpr double CRIT_RATIO_MAX = 1.5;

constexpr double MOB_ELEM_RES = 0.5;

inline long long calcAverageSkillDamage(
    double damageBaseVal,
    double expert,
    double critDamagePercent,
    double critRate
    );

long long applyMaxDamageCorrection(
    long long averageDamage,
    double maxDamageVal,
    double critDamagePercent,
    double critRate,
    double expert
);

long long calcSkillDamage(
    double skillDamage,
    double mainStat,
    double subStat,
    double weaponConst,
    double attack,
    double damagePercent,
    double finalDamagePercent,
    double ignoreDefense,
    double mobDefense,
    double critDamagePercent,
    double elementalAdjust,
    double expert
);

long long calcDotDamage(
    double skillDamage,
    double mainStat,
    double subStat,
    double weaponConst,
    double attack,
    double damagePercent,
    double finalDamagePercent,
    double ignoreDefense,
    double mobDefense,
    double critDamagePercent,
    double elementalAdjust,
    double expert
);

// TODO: calcSkillDamageRaw: 모든 옵션을 입력받음
long long calcSkillDamageRaw();

#endif //MCC_SKILL_DAMAGE_H
