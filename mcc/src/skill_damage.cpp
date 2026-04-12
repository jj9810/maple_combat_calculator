//
// Created by ubuntu on 25. 8. 10..
//

#include "../inc/skill_damage.h"
#include "../inc/combat_power.h"
#include <cmath>

long long calcSkillDamage(
    double skillDamage,
    const maple_combat_calculator::shared::MCCStat& stat,
    int mainStatType,
    double mastery,
    double mobDefense,
    double mobElemRes,
    double weaponConst,
    double levelAdjust,
    double forceAdjust
) {
    // StatType 로직을 사용하여 필요한 스탯을 매핑합니다.
    MappedStats mapped = mapStatType(stat, mainStatType);

    // 공식 API 기반의 MCCStat은 이미 최종 합산된 값을 제공하므로,
    // 기존의 복잡한 개별 파라미터 대신 필요한 필드를 직접 추출하여 계산합니다.
    return calcSkillDamageRaw(
        skillDamage,
        mapped.mainStat,
        mapped.subStat,
        mapped.attackOrMagic,
        mastery,
        stat.damage() + stat.boss_damage(), // 데미지 + 보공 합산
        stat.final_damage(),
        stat.critical_chance(),
        stat.critical_damage(),
        stat.ignore_defense(),
        stat.elemental_resistance_ignore(), // stat 내부의 속성 내성 무시 사용
        mobDefense,
        mobElemRes,                         // 매개변수로 받은 몬스터 속성 저항 사용
        weaponConst,
        levelAdjust,
        forceAdjust
    );
}

inline long long calcAverageSkillDamage(
    double damageBaseVal,
    double mastery,
    double critDamagePercent,
    bool isCrit
    )
{
    // mastery는 % 단위이므로 0.01을 곱해 비율로 변환
    return std::floor(damageBaseVal * ((mastery * 0.01 + 1.0) / 2)
                * (isCrit ? (CRIT_RATIO_MEAN + critDamagePercent * 0.01) : 1.0)
                );
}

/**
 * 맥뎀 보정 계산 함수
 * @param averageDamage 평균 데미지
 * @param maxDamageVal (크리티컬 데미지 증폭 미적용 기준) 최대 데미지 값
 * @param critDamagePercent 크리티컬 데미지 (%)
 * @param mastery 숙련도 (%)
 * @return 보정된 데미지
 */
long long applyMaxDamageCorrection(
    long long averageDamage,
    double maxDamageVal,
    double critDamagePercent,
    double mastery,
    bool isCrit
) {
    constexpr double D = DEFAULT_MAX_DAMAGE; // 맥뎀 기준값

    double minCrit = isCrit ? CRIT_RATIO_MIN + critDamagePercent * 0.01 : 1.0;
    double maxCrit = isCrit ? CRIT_RATIO_MAX + critDamagePercent * 0.01 : 1.0;

    double minDamageVal = maxDamageVal * mastery * 0.01;

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
               defenseRatio * elementalRatio *
               levelAdjust * forceAdjust;

    long long averageDamageCrit = calcAverageSkillDamage(maxDamageVal, mastery, critDamagePercent, true);
    long long averageDamageNonCrit = calcAverageSkillDamage(maxDamageVal, mastery, 0, false);

#if MAX_DAMAGE_CAP
    long long correctedAverageDamageCrit = applyMaxDamageCorrection(averageDamageCrit, maxDamageVal, critDamagePercent, mastery, true);
    long long correctedAverageDamageNonCrit = applyMaxDamageCorrection(averageDamageNonCrit, maxDamageVal, 0, mastery, false);
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
)
{
    return calcSkillDamageRaw(
        skillDamage,
        mainStat,
        subStat,
        attack,
        mastery,
        damagePercent,
        finalDamagePercent,
        100.0,              // critRate: 100%
        critDamagePercent,
        ignoreDefense,
        elementalAdjust,
        mobDefense,
        MOB_ELEM_RES,       // mobElemRes: 상수 값 사용
        weaponConst,
        levelAdjust,
        forceAdjust
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
        100.0,              // mastery: 도트딜은 숙련도 100%
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
