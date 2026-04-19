#pragma once

#include "component.h"
#include "entity.h"
#include "logger.h"
#include "policy.h"
#include <vector>
#include <memory>

namespace mdc {

class Engine {
public:
    Engine(const maple_combat_calculator::shared::CombatLog& initial_log);

    // 시뮬레이션을 실행합니다.
    void run(double duration);

    // 컴포넌트를 추가합니다.
    void add_component(std::shared_ptr<Component> component);

    // 행동 정책을 설정합니다.
    void set_policy(std::unique_ptr<Policy> policy);

    // 최종 로그를 반환합니다.
    const maple_combat_calculator::shared::CombatLog& get_combat_log() const;

private:
    void elapse(double time_delta);
    void execute_action(const maple_combat_calculator::shared::Action& action);

    double clock_ = 0.0;
    Entity entity_;
    Logger logger_;
    std::unique_ptr<Policy> policy_;
    std::vector<std::shared_ptr<Component>> components_;
};

}
