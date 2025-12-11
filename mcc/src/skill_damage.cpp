//
// Created by ubuntu on 25. 8. 10..
//

#include "skill_damage.h"
#include <cmath>


inline long long calcAverageSkillDamage(
    double damageBaseVal,
    double expert,
    double critDamagePercent,
    bool isCrit
    )
{
    // expert는 % 단위이므로 0.01을 곱해 비율로 변환
    return std::floor(damageBaseVal * ((expert * 0.01 + 1.0) / 2)
                * (isCrit ? (CRIT_RATIO_MEAN + critDamagePercent * 0.01) : 1.0)
                );
}

/**
 * 맥뎀 보정 계산 함수
 * @param averageDamage 평균 데미지
 * @param maxDamageVal (크리티컬 데미지 증폭 미적용 기준) 최대 데미지 값
 * @param critDamagePercent 크리티컬 데미지 (%)
 * @param expert 숙련도 (%)
 * @return 보정된 데미지
 */
long long applyMaxDamageCorrection(
    long long averageDamage,
    double maxDamageVal,
    double critDamagePercent,
    double expert,
    bool isCrit
) {
    constexpr double D = DEFAULT_MAX_DAMAGE; // 맥뎀 기준값

    double minCrit = isCrit ? CRIT_RATIO_MIN + critDamagePercent * 0.01 : 1.0;
    double maxCrit = isCrit ? CRIT_RATIO_MAX + critDamagePercent * 0.01 : 1.0;

    double minDamageVal = maxDamageVal * expert * 0.01;

    // normalize
    double maxRatio = maxDamageVal / D;
    double minRatio = minDamageVal / D;

    // 이론상 최대치가 맥뎀 미만인 경우
    if (maxRatio * maxCrit < 1.0) {
        return averageDamage;
    }

    double normalizer = (maxRatio - minRatio) * (maxCrit - minCrit);

    if (minRatio * maxCrit < 1.0 && maxRatio * minCrit < 1.0) {
        double pRatio = 1.0 / maxCrit;
        double excess = maxCrit * (maxCrit - pRatio)
            - (maxCrit * maxCrit / 4.0) * (maxRatio * maxRatio - pRatio * pRatio)
            - 0.5 * log(maxRatio / pRatio);

        return averageDamage + std::floor(excess / normalizer * D * D);
    }
    if (minRatio * maxCrit < 1.0 && maxRatio * minCrit >= 1.0) {
        double pRatio = minRatio;
        double excess = maxCrit * (maxCrit - pRatio)
            - (maxCrit * maxCrit / 4.0) * (maxRatio * maxRatio - pRatio * pRatio)
            - 0.5 * log(maxRatio / pRatio);

        return averageDamage + std::floor(excess / normalizer * D * D);
    }
    if (minRatio * maxCrit >= 1.0 && maxRatio * minCrit < 1.0) {
        double pRatio = minCrit;
        double excess = maxRatio * (maxRatio - pRatio)
            - (maxRatio * maxRatio / 4.0) * (maxCrit * maxCrit - pRatio * pRatio)
            - 0.5 * log(maxCrit / pRatio);

        return averageDamage + std::floor(excess / normalizer * D * D);
    }
    if (minRatio * minCrit < 1.0) {
        double pRatio = 1.0 / minCrit;
        double excess = minCrit * (pRatio - minRatio)
            - (minCrit * minCrit / 4.0) * (pRatio * pRatio - minRatio * minRatio)
            - 0.5 * log(pRatio / minRatio);

        return DEFAULT_MAX_DAMAGE + std::floor(excess / normalizer * D * D);
    }

    // 모든 데미지가 맥뎀일 경우
    return DEFAULT_MAX_DAMAGE;
}

/**
 * 스킬의 최종 데미지를 계산하는 핵심 함수
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
)
{
    // (주스텟*4 + 부스텟) / 100
    double statRatio = (mainStat * 4 + subStat) * 0.01;

    double damageRatio = damagePercent * 0.01 + 1.0;
    double finalDamageRatio = finalDamagePercent * 0.01 + 1.0;

    double defenseRatio = std::max(1.0 - mobDefense * 0.01 * (1.0 - ignoreDefense * 0.01), 0.0);

    // {1 - 속성내성% * (1 - 내속성무시%)}
    double elementalRatio = 1.0 - mobElemRes * (1.0 - elementalAdjust * 0.01);

    double maxDamageVal = skillDamage * statRatio * weaponConst * attack *
               damageRatio * finalDamageRatio *
               defenseRatio * elementalRatio;

    long long averageDamageCrit = calcAverageSkillDamage(maxDamageVal, expert, critDamagePercent, true);
    long long averageDamageNonCrit = calcAverageSkillDamage(maxDamageVal, expert, 0, false);

#if MAX_DAMAGE_CAP
    long long correctedAverageDamageCrit = applyMaxDamageCorrection(averageDamageCrit, maxDamageVal, critDamagePercent, expert, true);
    long long correctedAverageDamageNonCrit = applyMaxDamageCorrection(averageDamageNonCrit, maxDamageVal, 0, expert, false);
    long long averageDamageFinal = static_cast<long long>((critRate * 0.01 * correctedAverageDamageCrit) + ((100.0 - critRate) * 0.01 * correctedAverageDamageNonCrit));
    return averageDamageFinal;
#else
    long long averageDamage = static_cast<long long>((critRate * 0.01 * averageDamageCrit) + ((100.0 - critRate) * 0.01 * averageDamageNonCrit));
    return averageDamage;
#endif
}

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
)
{
    return calcSkillDamageRaw(
        skillDamage,
        mainStat,
        subStat,
        attack,
        expert,
        damagePercent,
        finalDamagePercent,
        100.0,              // critRate: 100%
        critDamagePercent,
        ignoreDefense,
        elementalAdjust,
        mobDefense,
        MOB_ELEM_RES,       // mobElemRes: 상수 값 사용
        weaponConst
    );
}

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
)
{
    return calcSkillDamageRaw(
        skillDamage,
        mainStat,
        subStat,
        attack,
        100.0,              // expert: 도트딜은 숙련도 100%
        damagePercent,
        finalDamagePercent,
        0.0,                // critRate: 크리티컬 미적용
        0.0,                // critDamagePercent: 크리티컬 데미지 미적용
        0.0,                // ignoreDefense: 방무 미적용
        elementalAdjust,
        mobDefense,
        mobElemRes,
        weaponConst
    );
}
