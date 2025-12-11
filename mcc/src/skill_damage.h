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

/**
 * 스킬의 최종 데미지를 계산하는 가장 기본적인 함수
 */
long long calcSkillDamageRaw(
    // 스킬 기본 정보
    double skillDamage,
    // 캐릭터 스탯
    double mainStat,
    double subStat,
    double attack,
    double expert,
    // 데미지 관련 스탯
    double damagePercent,
    double finalDamagePercent,
    // 크리티컬 관련 스탯
    double critRate,
    double critDamagePercent,
    // 몬스터 상호작용
    double ignoreDefense,
    double elementalAdjust,
    double mobDefense,
    double mobElemRes,
    // 기타
    double weaponConst
);

/**
 * 일반적인 스킬 데미지를 계산하는 편의 함수 (크확 100%, 기본 몬스터 속성 저항 적용)
 */
long long calcSkillDamage(
    // 스킬 기본 정보
    double skillDamage,
    // 캐릭터 스탯
    double mainStat,
    double subStat,
    double attack,
    double expert,
    // 데미지 관련 스탯
    double damagePercent,
    double finalDamagePercent,
    // 크리티컬 관련 스탯
    double critDamagePercent,
    // 몬스터 상호작용
    double ignoreDefense,
    double elementalAdjust,
    double mobDefense,
    // 기타
    double weaponConst
);

/**
 * 도트 데미지를 계산하는 함수
 */
long long calcDotDamage(
    // 스킬 기본 정보
    double skillDamage,
    // 캐릭터 스탯
    double mainStat,
    double subStat,
    double attack,
    // 데미지 관련 스탯
    double damagePercent,
    double finalDamagePercent,
    // 몬스터 상호작용
    double elementalAdjust,
    double mobDefense,
    double mobElemRes,
    // 기타
    double weaponConst
);

/**
 * 맥뎀 보정 계산 함수
 */
long long applyMaxDamageCorrection(
    long long averageDamage,
    double maxDamageVal,
    double critDamagePercent,
    double expert,
    bool isCrit
);

#endif //MCC_SKILL_DAMAGE_H
