#pragma once

#include "shared/proto/internal/combat_log.pb.h"
#include "entity.h"
#include <vector>
#include <string>

namespace mdc {

// Policy는 다음에 수행할 Action을 결정합니다.
class Policy {
public:
    virtual ~Policy() = default;

    // 현재 Entity 상태를 보고 다음에 수행할 Action을 결정합니다.
    // 수행할 Action이 없으면 nullptr을 반환합니다.
    virtual std::unique_ptr<maple_combat_calculator::shared::Action> get_next_action(const Entity& entity) = 0;
};


// 간단한 우선순위 기반 정책
class PriorityPolicy : public Policy {
public:
    PriorityPolicy(const std::vector<std::string>& skill_priority_list);

    std::unique_ptr<maple_combat_calculator::shared::Action> get_next_action(const Entity& entity) override;

private:
    std::vector<std::string> priority_list_;
};

}
