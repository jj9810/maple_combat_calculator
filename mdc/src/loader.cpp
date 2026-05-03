#include "mdc/inc/loader.h"
#include "mdc/inc/skill_component.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>

namespace mdc {

// 도우미 함수: StatInfo 추가
void add_stat_info(maple_combat_calculator::shared::BattlePracticeCharacterInfo& info, const std::string& name, const std::string& value) {
    auto* stat_info = info.mutable_stat_object()->mutable_basic_stat_object()->add_final_stat();
    stat_info->set_stat_name(name);
    stat_info->set_stat_value(value);
}

maple_combat_calculator::shared::BattlePracticeCharacterInfo DataLoader::load_character_info(const std::string& filepath) {
    maple_combat_calculator::shared::BattlePracticeCharacterInfo char_info;
    YAML::Node config = YAML::LoadFile(filepath);

    if (config["character"]) {
        auto character = config["character"];
        auto* basic = char_info.mutable_basic_object();
        if (character["class"]) basic->set_character_class(character["class"].as<std::string>());
        if (character["level"]) basic->set_character_level(character["level"].as<int64_t>());
        
        if (character["weapon_constant"]) {
            add_stat_info(char_info, "무기상수", std::to_string(character["weapon_constant"].as<double>()));
        }
    }
    return char_info;
}

maple_combat_calculator::shared::BattlePracticeCharacterInfo DataLoader::load_character_preset(const std::string& filepath) {
    maple_combat_calculator::shared::BattlePracticeCharacterInfo char_info;
    YAML::Node config = YAML::LoadFile(filepath);

    if (config["character"] && config["character"]["stats"]) {
        auto stats = config["character"]["stats"];
        
        if (stats["str"]) add_stat_info(char_info, "STR", std::to_string(stats["str"].as<double>()));
        if (stats["dex"]) add_stat_info(char_info, "DEX", std::to_string(stats["dex"].as<double>()));
        if (stats["damage"]) add_stat_info(char_info, "데미지", std::to_string(stats["damage"].as<double>()));
        if (stats["boss_damage"]) add_stat_info(char_info, "보스 데미지", std::to_string(stats["boss_damage"].as<double>()));
        if (stats["final_damage"]) add_stat_info(char_info, "최종 데미지", std::to_string(stats["final_damage"].as<double>()));
        if (stats["critical_damage"]) add_stat_info(char_info, "크리티컬 데미지", std::to_string(stats["critical_damage"].as<double>()));

        if (stats["ignore_defense"]) {
            for (const auto& val : stats["ignore_defense"]) {
                add_stat_info(char_info, "방어율 무시", std::to_string(val.as<double>()));
            }
        }
    }
    return char_info;
}

maple_combat_calculator::shared::MonsterInfo DataLoader::load_monster_preset(const std::string& filepath) {
    maple_combat_calculator::shared::MonsterInfo monster_info;
    YAML::Node config = YAML::LoadFile(filepath);

    if (config["monster"]) {
        auto monster = config["monster"];
        if (monster["level"]) monster_info.set_level(monster["level"].as<int>());
        if (monster["defense_rate"]) monster_info.set_defense_rate(monster["defense_rate"].as<double>());
        if (monster["elemental_resistance"]) monster_info.set_elemental_resistance(monster["elemental_resistance"].as<double>());
    }
    return monster_info;
}

SkillData parse_skill_node(const YAML::Node& node) {
    SkillData data;
    data.name = node["name"].as<std::string>();
    data.display_name = node["display_name"] ? node["display_name"].as<std::string>() : data.name;
    data.damage = node["damage"] ? node["damage"].as<double>() : 0.0;
    data.hit = node["hit"] ? node["hit"].as<int>() : 0;
    data.cooldown = node["cooldown"] ? node["cooldown"].as<double>() : 0.0;
    data.delay = node["delay"] ? node["delay"].as<double>() : 0.0;

    if (node["growth"]) {
        auto growth = node["growth"];
        if (growth["damage"]) {
            data.growth.damage.slope = growth["damage"]["slope"].as<double>();
            data.growth.damage.intercept = growth["damage"]["intercept"].as<double>();
        }
        if (growth["hit"]) data.growth.hit = growth["hit"].as<int>();
        
        if (growth["special"]) {
            for (auto const& spec : growth["special"]) {
                int lv = spec.first.as<int>();
                if (spec.second["bonus_ignore_defense"]) 
                    data.growth.bonus_ignore_defense[lv] = spec.second["bonus_ignore_defense"].as<double>();
                if (spec.second["bonus_boss_damage"]) 
                    data.growth.bonus_boss_damage[lv] = spec.second["bonus_boss_damage"].as<double>();
            }
        }
    }

    if (node["extra_attacks"]) {
        for (const auto& extra : node["extra_attacks"]) {
            data.extra_attacks.push_back({
                extra["name"].as<std::string>(),
                extra["damage"].as<double>(),
                extra["hit"].as<int>(),
                extra["count"] ? extra["count"].as<int>() : 1
            });
        }
    }
    return data;
}

std::vector<std::shared_ptr<Component>> DataLoader::load_skills(const std::string& filepath) {
    std::vector<std::shared_ptr<Component>> components;
    YAML::Node config = YAML::LoadFile(filepath);

    if (!config["skills"]) return components;

    for (const auto& node : config["skills"]) {
        SkillData base_data = parse_skill_node(node);
        std::string type = node["type"] ? node["type"].as<std::string>() : "attack";

        std::shared_ptr<Component> base_comp;
        if (type == "buff") {
            base_comp = std::make_shared<BuffSkillComponent>(base_data);
        } else {
            base_comp = std::make_shared<AttackSkillComponent>(base_data);
        }

        // HEXA 업그레이드 처리
        if (node["hexa_upgrade"]) {
            SkillData hexa_data = parse_skill_node(node["hexa_upgrade"]);
            // HEXA 버전의 타입은 기본적으로 베이스와 동일하다고 가정
            std::shared_ptr<Component> hexa_comp;
            if (type == "buff") {
                hexa_comp = std::make_shared<BuffSkillComponent>(hexa_data);
            } else {
                hexa_comp = std::make_shared<AttackSkillComponent>(hexa_data);
            }
            components.push_back(std::make_shared<HexaSkillComponent>(base_comp, hexa_comp));
        } else {
            components.push_back(base_comp);
        }
    }

    return components;
}

std::unique_ptr<Policy> DataLoader::load_policy(const std::string& filepath) {
    YAML::Node config = YAML::LoadFile(filepath);
    std::vector<std::string> priority_list;

    if (config["policy"] && config["policy"]["priority"]) {
        for (const auto& skill_name : config["policy"]["priority"]) {
            priority_list.push_back(skill_name.as<std::string>());
        }
    }

    return std::make_unique<PriorityPolicy>(priority_list);
}

}
