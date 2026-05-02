#include "mcm/inc/loader.h"
#include "nexon/skill_timeline.pb.h"
#include "nexon/battle_practice_character_info.pb.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <vector>

namespace mcm {

struct SkillInfo {
    std::string name;
    std::string type;
    double damage = 0;
    int hit = 0;
    double duration = 0;
    std::map<std::string, double> effects;
    
    struct Action {
        double damage;
        int hit;
        int repeat = 1;
        double interval = 0;
        double delay = 0;
    };
    std::vector<Action> actions;
};

bool LogLoader::load(const std::string& filepath, maple_combat_calculator::shared::CombatLog& combat_log) {
    std::ifstream input(filepath, std::ios::binary);
    if (!input) {
        std::cerr << "Error: Cannot open file " << filepath << std::endl;
        return false;
    }

    if (!combat_log.ParseFromIstream(&input)) {
        std::cerr << "Error: Failed to parse CombatLog from file." << std::endl;
        return false;
    }

    return true;
}

bool LogLoader::load_nexon_json(
    const std::string& character_json_path,
    const std::string& timeline_json_path,
    maple_combat_calculator::shared::CombatLog& combat_log
) {
    auto read_json = [](const std::string& path) -> std::string {
        std::ifstream ifs(path);
        if (!ifs.is_open()) return "";
        std::stringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    };

    std::string char_json = read_json(character_json_path);
    std::string timeline_json = read_json(timeline_json_path);

    return load_nexon_json_from_string(char_json, timeline_json, combat_log);
}

bool LogLoader::load_nexon_json_from_string(
    const std::string& char_json,
    const std::string& timeline_json,
    maple_combat_calculator::shared::CombatLog& combat_log
) {
    if (char_json.empty() || timeline_json.empty()) {
        return false;
    }

    maple_combat_calculator::shared::BattlePracticeCharacterInfo char_info;
    maple_combat_calculator::shared::SkillTimeline timeline;

    if (!json_to_proto(char_json, char_info)) return false;
    if (!json_to_proto(timeline_json, timeline)) return false;

    // 1. 스킬 데이터베이스 로드 (Adele 고정 - 추후 확장)
    std::map<std::string, SkillInfo> skill_db;
    try {
        YAML::Node skill_root = YAML::LoadFile("shared/data/classes/adele_skills.yaml");
        for (auto s_node : skill_root["skills"]) {
            SkillInfo si;
            si.name = s_node["name"].as<std::string>();
            si.type = s_node["type"].as<std::string>();
            if (s_node["duration"]) si.duration = s_node["duration"].as<double>();
            
            if (s_node["actions"]) {
                for (auto a_node : s_node["actions"]) {
                    SkillInfo::Action a;
                    a.damage = a_node["damage"].as<double>(0);
                    a.hit = a_node["hit"].as<int>(0);
                    a.repeat = a_node["repeat"].as<int>(1);
                    a.interval = a_node["interval"].as<double>(0);
                    a.delay = a_node["delay"].as<double>(0);
                    si.actions.push_back(a);
                }
            }
            if (si.actions.empty()) {
                // 단일 액션 스킬 처리 (damage, hit 필드만 있는 경우 대비)
                if (s_node["damage"] && s_node["hit"]) {
                    SkillInfo::Action a;
                    a.damage = s_node["damage"].as<double>();
                    a.hit = s_node["hit"].as<int>();
                    si.actions.push_back(a);
                }
            }

            if (s_node["effects"]) {
                for (auto it = s_node["effects"].begin(); it != s_node["effects"].end(); ++it) {
                    si.effects[it->first.as<std::string>()] = it->second.as<double>();
                }
            }

            std::string d_name = s_node["display_name"].as<std::string>();
            skill_db[d_name] = si;

            if (s_node["hexa_upgrade"]) {
                std::string h_name = s_node["hexa_upgrade"]["display_name"].as<std::string>();
                // HEXA 스킬은 일단 베이스와 동일하게 (데미지 보정은 추후)
                skill_db[h_name] = si;
                // HEXA 전용 데미지 설정이 있다면 덮어쓰기
                if (s_node["hexa_upgrade"]["growth"]) {
                    // slope/intercept 기반 계산은 생략하고 일단 intercept 사용
                    if (s_node["hexa_upgrade"]["growth"]["intercept"]) {
                        double base_dmg = s_node["hexa_upgrade"]["growth"]["intercept"].as<double>();
                        if (!skill_db[h_name].actions.empty()) {
                             skill_db[h_name].actions[0].damage = base_dmg;
                        }
                    }
                }
            }
        }
    } catch (...) {
        std::cerr << "Warning: Failed to load skill database." << std::endl;
    }

    // 2. CombatLog 기본 정보 설정
    *combat_log.mutable_raw_character_info() = char_info;
    auto* mcc_char_info = combat_log.mutable_character_info();
    mcc_char_info->set_level(char_info.basic_object().character_level());
    mcc_char_info->set_main_stat_type(1); // Adele STR
    mcc_char_info->set_weapon_constant(1.3);

    auto* mcc_stat = combat_log.mutable_base_stat()->mutable_stat();
    for (const auto& stat : char_info.stat_object().basic_stat_object().final_stat()) {
        const std::string& name = stat.stat_name();
        try {
            double val = std::stod(stat.stat_value());
            if (name == "STR") mcc_stat->set_str(val);
            else if (name == "DEX") mcc_stat->set_dex(val);
            else if (name == "INT") mcc_stat->set_int_(val);
            else if (name == "LUK") mcc_stat->set_luk(val);
            else if (name == "HP") mcc_stat->set_hp(val);
            else if (name == "공격력") mcc_stat->set_attack_power(val);
            else if (name == "데미지") mcc_stat->set_damage(val);
            else if (name == "보스 몬스터 데미지") mcc_stat->set_boss_damage(val);
            else if (name == "최종 데미지") mcc_stat->set_final_damage(val);
            else if (name == "방어율 무시") mcc_stat->set_ignore_defense(val);
            else if (name == "크리티컬 확률") mcc_stat->set_critical_chance(val);
            else if (name == "크리티컬 데미지") mcc_stat->set_critical_damage(val);
            else if (name == "무기 숙련도") mcc_stat->set_mastery(val);
        } catch (...) {}
    }

    auto* mcc_mob_info = combat_log.mutable_monster_info();
    try {
        YAML::Node mob_root = YAML::LoadFile("shared/data/mobs/default_boss_spec.yaml");
        auto mob_node = mob_root["monster"];
        mcc_mob_info->set_name(mob_node["name"].as<std::string>("Standard Boss"));
        mcc_mob_info->set_level(mob_node["level"].as<int>(285));
        mcc_mob_info->set_defense_rate(mob_node["defense_rate"].as<double>(300.0));
        mcc_mob_info->set_elemental_resistance(mob_node["elemental_resistance"].as<double>(0.5));
        mcc_mob_info->set_is_boss(mob_node["is_boss"].as<bool>(true));

        if (mob_node["requirements"]) {
            std::string f_type = mob_node["requirements"]["force_type"].as<std::string>("");
            int f_val = mob_node["requirements"]["force_value"].as<int>(0);
            if (f_type == "ARCANE") {
                mcc_mob_info->set_required_arcaneforce(f_val);
            } else if (f_type == "AUTHENTIC") {
                mcc_mob_info->set_required_authenticforce(f_val);
            }
        }
    } catch (...) {
        std::cerr << "Warning: Failed to load default monster info. Using fallbacks." << std::endl;
        mcc_mob_info->set_name("Practice Dummy");
        mcc_mob_info->set_level(260);
        mcc_mob_info->set_defense_rate(300.0);
        mcc_mob_info->set_elemental_resistance(0.5);
        mcc_mob_info->set_is_boss(true);
    }

    // 3. 타임라인 이벤트 생성 및 정렬
    struct InternalEvent {
        double clock;
        maple_combat_calculator::shared::Event ev;
    };
    std::vector<InternalEvent> all_events;

    for (const auto& entry : timeline.skill_timeline()) {
        double start_time = entry.elapse_time() / 1000.0;
        auto it = skill_db.find(entry.skill_name());
        
        if (it != skill_db.end()) {
            const auto& si = it->second;
            if (si.type == "attack" || si.type == "origin" || si.type == "summon") {
                for (const auto& action : si.actions) {
                    for (int r = 0; r < action.repeat; ++r) {
                        double t = start_time + action.delay + (r * action.interval);
                        InternalEvent ie;
                        ie.clock = t;
                        ie.ev.set_method(maple_combat_calculator::shared::Event::DAMAGE);
                        ie.ev.set_name(entry.skill_name());
                        ie.ev.set_damage(action.damage);
                        ie.ev.set_hit(action.hit);
                        all_events.push_back(ie);
                    }
                }
            }
            if (si.type == "buff") {
                InternalEvent on;
                on.clock = start_time;
                on.ev.set_method(maple_combat_calculator::shared::Event::BUFF);
                on.ev.set_name(entry.skill_name());
                on.ev.set_hit(1);
                auto* payload = on.ev.mutable_payload();
                for (const auto& [stat, val] : si.effects) {
                    std::string key = stat;
                    if (stat == "final_damage") key = "final_damage";
                    else if (stat == "damage") key = "damage";
                    else if (stat == "attack_power") key = "attack_power_fixed";
                    else if (stat == "ignore_defense") key = "ignore_defense";
                    (*payload->mutable_fields())[key].set_number_value(val);
                }
                all_events.push_back(on);

                if (si.duration > 0) {
                    InternalEvent off;
                    off.clock = start_time + si.duration;
                    off.ev.set_method(maple_combat_calculator::shared::Event::BUFF);
                    off.ev.set_name(entry.skill_name());
                    off.ev.set_hit(0);
                    all_events.push_back(off);
                }
            }
        } else {
            InternalEvent ie;
            ie.clock = start_time;
            ie.ev.set_method(maple_combat_calculator::shared::Event::DAMAGE);
            ie.ev.set_name(entry.skill_name());
            ie.ev.set_damage(100.0);
            ie.ev.set_hit(1);
            all_events.push_back(ie);
        }
    }
    std::cout << "Generated " << all_events.size() << " events." << std::endl;

    // 시간 순 정렬
    std::sort(all_events.begin(), all_events.end(), [](const auto& a, const auto& b) {
        return a.clock < b.clock;
    });

    // OperationLog에 밀어넣기
    auto* op_log = combat_log.add_operation_logs();
    std::map<double, maple_combat_calculator::shared::PlayLog*> time_map;
    for (const auto& ie : all_events) {
        if (time_map.find(ie.clock) == time_map.end()) {
            auto* play = op_log->add_play_logs();
            play->set_clock(ie.clock);
            time_map[ie.clock] = play;
        }
        *time_map[ie.clock]->add_events() = ie.ev;
    }

    return true;
}

}
