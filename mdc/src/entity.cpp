#include "mdc/inc/entity.h"
#include <algorithm>

namespace mdc {

Entity::Entity(const maple_combat_calculator::shared::CharacterBaseStat& base_stat)
    : current_stat_(base_stat.stat()) {}

const maple_combat_calculator::shared::Stat& Entity::get_current_stat() const {
    return current_stat_;
}

void Entity::set_cooldown(const std::string& skill_name, double cooldown) {
    cooldowns_[skill_name] = cooldown;
}

double Entity::get_cooldown(const std::string& skill_name) const {
    auto it = cooldowns_.find(skill_name);
    if (it != cooldowns_.end()) {
        return it->second;
    }
    return 0.0;
}

bool Entity::is_skill_available(const std::string& skill_name) const {
    return get_cooldown(skill_name) <= 0;
}

void Entity::elapse(double time_delta) {
    for (auto& pair : cooldowns_) {
        pair.second = std::max(0.0, pair.second - time_delta);
    }
}

}
