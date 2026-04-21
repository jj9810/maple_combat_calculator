#pragma once

#include "internal/mcc_stat.pb.h"
#include "internal/combat_log.pb.h"
#include "nexon/battle_practice_character_info.pb.h"
#include <map>
#include <string>

namespace mcm {

// 시뮬레이션 중 고정값과 백분율을 분리하여 관리하기 위한 내부 구조체
struct InternalStat {
    double str_fixed = 0, str_percent = 0;
    double dex_fixed = 0, dex_percent = 0;
    double int_fixed = 0, int_percent = 0;
    double luk_fixed = 0, luk_percent = 0;
    double hp_fixed = 0, hp_percent = 0;
    double mp_fixed = 0, mp_percent = 0;
    double att_fixed = 0, att_percent = 0;
    double mag_fixed = 0, mag_percent = 0;

    double damage = 0;
    double boss_damage = 0;
    double final_damage = 0; // 곱연산
    double ignore_defense = 0; // 곱연산
    double crit_chance = 0;
    double crit_damage = 0;

    void add(const InternalStat& other);
};

class SimulationContext {
public:
    SimulationContext(const maple_combat_calculator::shared::CharacterBaseStat& base,
                      const maple_combat_calculator::shared::CharacterInfo& char_info,
                      const maple_combat_calculator::shared::MonsterInfo& mob_info);

    // 버프 관리 (내부 구조체를 사용하여 합산)
    void apply_buff(const std::string& name, const InternalStat& stat);
    void remove_buff(const std::string& name);

    // 현재 시점의 모든 보정치가 계산 완료된 최종 MCCStat 반환
    maple_combat_calculator::shared::MCCStat get_current_total_stat() const;

    const maple_combat_calculator::shared::CharacterInfo& get_char_info() const { return char_info_; }
    const maple_combat_calculator::shared::MonsterInfo& get_mob_info() const { return mob_info_; }

private:
    maple_combat_calculator::shared::CharacterBaseStat base_stat_;
    maple_combat_calculator::shared::CharacterInfo char_info_;
    maple_combat_calculator::shared::MonsterInfo mob_info_;

    std::map<std::string, InternalStat> active_buffs_;
    
    // 최종 스탯 계산용 도우미
    InternalStat calculate_total_internal() const;
};



}
