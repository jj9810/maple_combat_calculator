//
// Created by ubuntu on 25. 8. 10..
//

#ifndef MCC_SKILL_DAMAGE_H
#define MCC_SKILL_DAMAGE_H

#include "internal/mcc_stat.pb.h"

constexpr long long DEFAULT_MAX_DAMAGE = 700000000000;

#define MAX_DAMAGE_CAP true

constexpr double CRIT_RATIO_MIN = 1.2;
constexpr double CRIT_RATIO_MEAN = 1.35;
constexpr double CRIT_RATIO_MAX = 1.5;

constexpr double MOB_ELEM_RES = 0.5;

/**
 * 공식 API 기반의 Stat을 사용하여 스킬 데미지를 계산합니다.
 */
long long calcSkillDamage(
    double skillDamage,
    const maple_combat_calculator::shared::MCCStat& stat,
    int mainStatType, // CharacterInfo.StatType enum 값
    double mastery,
    double mobDefense,
    double mobElemRes,
    double weaponConst,
    double levelAdjust,
    double forceAdjust
);

/**
 * 스킬의 최종 데미지를 계산하는 가장 기본적인 함수
 * Reference:
 *  https://www.inven.co.kr/board/maple/2299/5679951
 *  https://github.com/oleneyl/maplestory_dpm_calc/blob/e7d05a772e5e4935c3c27c56864b5c1f2a380137/dpmModule/kernel/core.py
 * @return expected damage in integer format
 */
long long calcSkillDamageRaw(
    // 스킬 기본 정보
    double skillDamage,
    // 캐릭터 스탯
    double mainStat,
    double subStat,
    double attack,
    double mastery,
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
    double weaponConst,
    double levelAdjust,
    double forceAdjust
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
    double mastery,
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
    double weaponConst,
    double levelAdjust,
    double forceAdjust
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
    double mastery,
    bool isCrit
);

#endif //MCC_SKILL_DAMAGE_H
