#ifndef MCC_LIBRARY_H
#define MCC_LIBRARY_H

#include "internal/mcc_stat.pb.h"

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

/**
 * 매핑된 스탯 정보를 담는 구조체
 */
struct MappedStats {
    double mainStat;
    double subStat;
    double attackOrMagic;
};

/**
 * Stat과 StatType을 기반으로 주스탯, 부스탯, 공격력/마력을 매핑합니다.
 */
MappedStats mapStatType(
    const maple_combat_calculator::shared::MCCStat& stat,
    int mainStatType
);

/**
 * 공식 API 기반의 Stat과 무기 및 기본 제공(innate) 수치를 입력으로 받아 전투력을 계산합니다.
 */
int calculateCombatPower(
    const maple_combat_calculator::shared::MCCStat& stat,
    int mainStatType, // CharacterInfo.StatType enum 값
    int weaponBaseAtt,
    int weaponSfAtt,
    int convertedWeaponAtt,
    double innateCritDmg,
    double innateBossDmg,
    double innateDmg,
    double innateFinalDmg
);

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
