#include "mcm/inc/connector.h"
#include "mcc/inc/skill_damage.h"
#include "mcc/inc/combat_power.h"
#include <algorithm>
#include <cmath>

namespace mcm {

long long MCMConnector::calculate_damage(
    const SimulationContext& context,
    const maple_combat_calculator::shared::Event& event
) {
    auto total_stat = context.get_current_total_stat();
    const auto& char_info = context.get_char_info();
    const auto& mob_info = context.get_mob_info();

    double skill_damage_percent = event.damage();
    double mastery = total_stat.mastery();
    if (mastery <= 0) mastery = 0.95; 

    int char_level = char_info.level();
    int mob_level = mob_info.level();

    // 몬스터 속성 반감
    double mob_elem_res = mob_info.elemental_resistance();
    if (mob_elem_res <= 0 && mob_info.is_boss()) mob_elem_res = MOB_ELEM_RES;

    return calcSkillDamage(
        skill_damage_percent,
        total_stat,
        char_info.main_stat_type(),
        mastery,
        mob_info.defense_rate(),
        mob_elem_res,
        char_info.weapon_constant(),
        get_level_adjust(char_level, mob_level),
        get_force_adjust(context)
    );
}

long long MCMConnector::calculate_dot_damage(
    const SimulationContext& context,
    const maple_combat_calculator::shared::Event& event
) {
    auto total_stat = context.get_current_total_stat();
    const auto& char_info = context.get_char_info();
    const auto& mob_info = context.get_mob_info();

    // MCC의 MappedStats를 사용하여 주/부스탯/공격력 추출
    MappedStats mapped = mapStatType(total_stat, char_info.main_stat_type());

    // 몬스터 속성 반감
    double mob_elem_res = mob_info.elemental_resistance();
    if (mob_elem_res <= 0 && mob_info.is_boss()) mob_elem_res = MOB_ELEM_RES;

    return calcDotDamage(
        event.damage(),
        mapped.mainStat,
        mapped.subStat,
        mapped.attackOrMagic,
        total_stat.damage() + total_stat.boss_damage(),
        total_stat.final_damage(),
        total_stat.elemental_resistance_ignore(),
        mob_info.defense_rate(),
        mob_elem_res,
        char_info.weapon_constant()
    );
}

InternalStat MCMConnector::parse_stat_from_payload(
    const google::protobuf::Struct& payload
) {
    InternalStat stat;
    const auto& fields = payload.fields();

    auto get_val = [&](const std::string& key) -> double {
        auto it = fields.find(key);
        if (it != fields.end() && it->second.kind_case() == google::protobuf::Value::kNumberValue) {
            return it->second.number_value();
        }
        return 0.0;
    };

    stat.str_fixed = get_val("str_fixed");
    stat.str_percent = get_val("str_percent");
    stat.dex_fixed = get_val("dex_fixed");
    stat.dex_percent = get_val("dex_percent");
    stat.int_fixed = get_val("int_fixed");
    stat.int_percent = get_val("int_percent");
    stat.luk_fixed = get_val("luk_fixed");
    stat.luk_percent = get_val("luk_percent");
    stat.hp_fixed = get_val("hp_fixed");
    stat.hp_percent = get_val("hp_percent");
    stat.mp_fixed = get_val("mp_fixed");
    stat.mp_percent = get_val("mp_percent");

    stat.att_fixed = get_val("attack_power_fixed");
    stat.att_percent = get_val("attack_power_percent");
    stat.mag_fixed = get_val("magic_power_fixed");
    stat.mag_percent = get_val("magic_power_percent");

    stat.damage = get_val("damage");
    stat.boss_damage = get_val("boss_damage");
    stat.final_damage = get_val("final_damage");
    stat.ignore_defense = get_val("ignore_defense");
    stat.crit_chance = get_val("critical_chance");
    stat.crit_damage = get_val("critical_damage");

    return stat;
}

double MCMConnector::get_level_adjust(int char_level, int mob_level) {
    int level_diff = char_level - mob_level;
    if (level_diff >= 5) return 1.1;
    if (level_diff >= 0) return 1.0 + (level_diff * 0.02);
    
    double adjust = 1.0 + (level_diff * 0.05); 
    return std::max(0.1, adjust);
}

double MCMConnector::get_force_adjust(const SimulationContext& context) {
    const auto& mob_info = context.get_mob_info();
    const auto& total_stat = context.get_current_total_stat();

    // 1. 어센틱포스(AUT) 지역인 경우
    if (mob_info.required_authenticforce() > 0) {
        int my_aut = total_stat.authenticforce();
        int req_aut = mob_info.required_authenticforce();
        
        if (my_aut >= req_aut) return 1.0 + (std::min(my_aut - req_aut, 50) / 10 * 0.05);
        
        double ratio = static_cast<double>(my_aut) / req_aut;
        if (ratio >= 0.9) return 0.95;
        if (ratio >= 0.8) return 0.90;
        if (ratio >= 0.7) return 0.85;
        if (ratio >= 0.6) return 0.75;
        if (ratio >= 0.5) return 0.60;
        if (ratio >= 0.4) return 0.50;
        if (ratio >= 0.3) return 0.40;
        if (ratio >= 0.2) return 0.25;
        if (ratio >= 0.1) return 0.10;
        return 0.05;
    }

    // 2. 아케인포스(ARC) 지역인 경우
    if (mob_info.required_arcaneforce() > 0) {
        double ratio = static_cast<double>(total_stat.arcaneforce()) / mob_info.required_arcaneforce();
        
        if (ratio >= 1.5) return 1.5;
        if (ratio >= 1.3) return 1.3;
        if (ratio >= 1.1) return 1.1;
        if (ratio >= 1.0) return 1.0;
        if (ratio >= 0.7) return 0.8;
        if (ratio >= 0.5) return 0.6;
        if (ratio >= 0.3) return 0.3;
        return 0.1;
    }

    return 1.0;
}

}
