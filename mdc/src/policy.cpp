#include "mdc/inc/policy.h"

namespace mdc {

PriorityPolicy::PriorityPolicy(const std::vector<std::string>& skill_priority_list)
    : priority_list_(skill_priority_list) {}

std::unique_ptr<maple_combat_calculator::shared::Action> PriorityPolicy::get_next_action(const Entity& entity) {
    for (const auto& skill_name : priority_list_) {
        if (entity.is_skill_available(skill_name)) {
            auto action = std::make_unique<maple_combat_calculator::shared::Action>();
            action->set_name(skill_name);
            action->set_method("use"); // 기본 행동은 'use'
            return action;
        }
    }
    return nullptr; // 사용할 스킬 없음
}

}
