#include "mcm/inc/context.h"
#include <cmath>
#include <algorithm>

namespace mcm {

void InternalStat::add(const InternalStat& other) {
    str_fixed += other.str_fixed; str_percent += other.str_percent;
    dex_fixed += other.dex_fixed; dex_percent += other.dex_percent;
    int_fixed += other.int_fixed; int_percent += other.int_percent;
    luk_fixed += other.luk_fixed; luk_percent += other.luk_percent;
    hp_fixed += other.hp_fixed; hp_percent += other.hp_percent;
    mp_fixed += other.mp_fixed; mp_percent += other.mp_percent;
    att_fixed += other.att_fixed; att_percent += other.att_percent;
    mag_fixed += other.mag_fixed; mag_percent += other.mag_percent;

    damage += other.damage;
    boss_damage += other.boss_damage;
    crit_chance += other.crit_chance;
    crit_damage += other.crit_damage;

    // 곱연산 필드 처리 (최종 데미지, 방무)
    final_damage = 100.0 * ((1.0 + final_damage * 0.01) * (1.0 + other.final_damage * 0.01) - 1.0);
    ignore_defense = 100.0 * (1.0 - (1.0 - ignore_defense * 0.01) * (1.0 - other.ignore_defense * 0.01));
}

SimulationContext::SimulationContext(const maple_combat_calculator::shared::CharacterBaseStat& base,
                                     const maple_combat_calculator::shared::CharacterInfo& char_info,
                                     const maple_combat_calculator::shared::MonsterInfo& mob_info)
    : base_stat_(base), char_info_(char_info), mob_info_(mob_info) {}

void SimulationContext::apply_buff(const std::string& name, const InternalStat& stat) {
    active_buffs_[name] = stat;
}

void SimulationContext::remove_buff(const std::string& name) {
    active_buffs_.erase(name);
}

InternalStat SimulationContext::calculate_total_internal() const {
    InternalStat total;
    
    // 1. 기본 스탯 로드 (base_stat_.stat()에서 초기값 추출)
    const auto& base = base_stat_.stat();
    total.str_fixed = base.str();
    total.dex_fixed = base.dex();
    total.int_fixed = base.int_();
    total.luk_fixed = base.luk();
    total.hp_fixed = base.hp();
    total.mp_fixed = base.mp();
    total.att_fixed = base.attack_power();
    total.mag_fixed = base.magic_power();
    total.damage = base.damage();
    total.boss_damage = base.boss_damage();
    total.final_damage = base.final_damage();
    total.ignore_defense = base.ignore_defense();
    total.crit_chance = base.critical_chance();
    total.crit_damage = base.critical_damage();

    // 2. 활성화된 모든 버프 합산
    for (const auto& [name, stat] : active_buffs_) {
        total.add(stat);
    }
    
    return total;
}

maple_combat_calculator::shared::MCCStat SimulationContext::get_current_total_stat() const {
    InternalStat internal = calculate_total_internal();
    maple_combat_calculator::shared::MCCStat result;

    // 메이플스토리 공식에 따른 최종 수치 계산
    // 최종 스탯 = 고정값 * (1 + 백분율 / 100)
    // 참고: 공식 API의 기본 응답은 이미 보정된 값이지만, 시뮬레이션 중 추가되는 버프를 처리하기 위함
    result.set_str(std::floor(internal.str_fixed * (1.0 + internal.str_percent * 0.01)));
    result.set_dex(std::floor(internal.dex_fixed * (1.0 + internal.dex_percent * 0.01)));
    result.set_int_(std::floor(internal.int_fixed * (1.0 + internal.int_percent * 0.01)));
    result.set_luk(std::floor(internal.luk_fixed * (1.0 + internal.luk_percent * 0.01)));
    result.set_hp(std::floor(internal.hp_fixed * (1.0 + internal.hp_percent * 0.01)));
    result.set_mp(std::floor(internal.mp_fixed * (1.0 + internal.mp_percent * 0.01)));

    result.set_attack_power(std::floor(internal.att_fixed * (1.0 + internal.att_percent * 0.01)));
    result.set_magic_power(std::floor(internal.mag_fixed * (1.0 + internal.mag_percent * 0.01)));

    result.set_damage(internal.damage);
    result.set_boss_damage(internal.boss_damage);
    result.set_final_damage(internal.final_damage);
    result.set_ignore_defense(internal.ignore_defense);
    result.set_critical_chance(internal.crit_chance);
    result.set_critical_damage(internal.crit_damage);

    // 기타 필드 복사
    const auto& base = base_stat_.stat();
    result.set_mastery(base.mastery());
    result.set_buff_duration(base.buff_duration());
    result.set_elemental_resistance_ignore(base.elemental_resistance_ignore());
    result.set_arcaneforce(base.arcaneforce());
    result.set_authenticforce(base.authenticforce());
    result.set_starforce(base.starforce());

    return result;
}

}
