#ifndef MCC_LIBRARY_H
#define MCC_LIBRARY_H

/***
 *
 * https://maplestorywiki.net/w/Combat_Power
 *
 * https://python-fiddle.com/saved/Hv6sN7miNlXfTijSoTO6
 *
 */
#pragma once

constexpr double BASE_RATIO = 0.01;

constexpr int BASE_CRIT_CONSTANT = 35;

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
    );

inline double toMultiplier(double percent);

#endif // MCC_LIBRARY_H
