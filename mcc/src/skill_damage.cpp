//
// Created by ubuntu on 25. 8. 10..
//

#include "../inc/skill_damage.h"
#include "../inc/combat_power.h"
#include <cmath>
#include <boost/multiprecision/cpp_bin_float.hpp>

using namespace boost::multiprecision;
typedef cpp_bin_float_quad high_float;

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

    // 매개변수로 받은 몬스터 속성 저항 사용 (없으면 기본값 적용)
    double finalMobElemRes = (mobElemRes > 0) ? mobElemRes : MOB_ELEM_RES;

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
        stat.ignore_defense(), // MCM에서 이미 합산된 값을 사용
        stat.elemental_resistance_ignore(), // stat 내부의 속성 내성 무시 사용
        mobDefense,
        finalMobElemRes,
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
    high_float D = DEFAULT_MAX_DAMAGE; // 맥뎀 기준값 (고정밀도)

    high_float minCrit = isCrit ? high_float(CRIT_RATIO_MIN) + high_float(critDamagePercent) * 0.01 : high_float(1.0);
    high_float maxCrit = isCrit ? high_float(CRIT_RATIO_MAX) + high_float(critDamagePercent) * 0.01 : high_float(1.0);

    high_float x1 = high_float(maxDamageVal) * (high_float(mastery) * 0.01);
    high_float x2 = high_float(maxDamageVal);
    high_float y1 = minCrit;
    high_float y2 = maxCrit;

    // 이론상 최대치가 맥뎀 미만인 경우
    if (x2 * y2 <= D) {
        return averageDamage;
    }

    // 모든 데미지가 맥뎀 이상인 경우
    if (x1 * y1 >= D) {
        return static_cast<long long>(D);
    }

    // x1 == x2 이고 y1 == y2 인 경우는 위에서 이미 처리됨 (x2*y2 <= D 또는 x1*y1 >= D)
    
    // y1 == y2 인 경우 (예: Non-Crit) - 1차원 적분
    if (y1 == y2) {
        high_float x_p = D / y1;
        high_float result = ( (y1 / 2.0) * (x_p * x_p - x1 * x1) + D * (x2 - x_p) ) / (x2 - x1);
        return static_cast<long long>(result);
    }

    // x1 == x2 인 경우 (예: 숙련도 100%) - 1차원 적분
    if (x1 == x2) {
        high_float y_p = D / x1;
        high_float result = ( (x1 / 2.0) * (y_p * y_p - y1 * y1) + D * (y2 - y_p) ) / (y2 - y1);
        return static_cast<long long>(result);
    }

    high_float normalizer = (x2 - x1) * (y2 - y1);
    high_float basis = (x1 + x2) / 2.0 * (y1 + y2) / 2.0;

    // Case별 맥뎀 보정 (E[min(D, xy)] 계산)
    // Reference: https://github.com/oleneyl/maplestory_dpm_calc/blob/master/dpmModule/kernel/core.py
    
    if (D > x1 * y2 && D > x2 * y1) {
        high_float x_p = D / y2;
        // Case 1
        high_float excess = D * y2 * (x2 - x_p) - (y2 * y2 / 4.0) * (x2 * x2 - x_p * x_p) - (D * D / 2.0) * log(x2 / x_p);
        high_float result = basis + excess / normalizer;
        return static_cast<long long>(result);
    } 
    else if (D <= x1 * y2 && D > x2 * y1) {
        // Case 2
        high_float x_p = x1;
        high_float excess = D * y2 * (x2 - x_p) - (y2 * y2 / 4.0) * (x2 * x2 - x_p * x_p) - (D * D / 2.0) * log(x2 / x_p);
        high_float result = basis + excess / normalizer;
        return static_cast<long long>(result);
    }
    else if (D > x1 * y2 && D <= x2 * y1) {
        // Case 3: Swap x and y to reuse Case 2 logic
        high_float y_p_c3 = D / x2;
        high_float excess = D * x2 * (y2 - y_p_c3) - (x2 * x2 / 4.0) * (y2 * y2 - y_p_c3 * y_p_c3) - (D * D / 2.0) * log(y2 / y_p_c3);
        high_float result = basis + excess / normalizer;
        return static_cast<long long>(result);
    }
    else if (D > x1 * y1) {
        // Case 4
        high_float x_p = D / y1;
        high_float excess = D * y1 * (x_p - x1) - (y1 * y1 / 4.0) * (x_p * x_p - x1 * x1) - (D * D / 2.0) * log(x_p / x1);
        high_float result = D + excess / normalizer;
        return static_cast<long long>(result);
    }

    return static_cast<long long>(D);
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
        MOB_ELEM_RES,       // 기본값 사용
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
        weaponConst,
        1.0,                // TODO: verify levelAdjust for DOT
        1.0                 // TODO: verify forceAdjust for DOT
    );
}
