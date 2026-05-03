#include "mdc/inc/engine.h"

namespace mdc {

Engine::Engine(const maple_combat_calculator::shared::CombatLog& initial_log)
    : entity_(initial_log.base_stat()), logger_(initial_log) {}

void Engine::run(double duration) {
    const double time_step = 0.01; // 10ms 단위로 시뮬레이션

    while (clock_ < duration) {
        // 1. 시간이 경과함
        elapse(time_step);

        // 2. 정책에 따라 행동 결정
        auto action = policy_->get_next_action(entity_);

        // 3. 행동 실행
        if (action) {
            execute_action(*action);
        }

        clock_ += time_step;
    }
}

void Engine::add_component(std::shared_ptr<Component> component) {
    components_.push_back(component);
}

void Engine::set_policy(std::unique_ptr<Policy> policy) {
    policy_ = std::move(policy);
}

const maple_combat_calculator::shared::CombatLog& Engine::get_combat_log() const {
    return logger_.get_log();
}

void Engine::elapse(double time_delta) {
    entity_.elapse(time_delta);
    for (auto& component : components_) {
        auto events = component->elapse(time_delta, entity_);
        if (!events.empty()) {
            // 시간이 경과하며 발생한 이벤트 기록 (예: 도트 데미지)
            maple_combat_calculator::shared::Action empty_action;
            empty_action.set_name("Elapse");
            logger_.record(clock_, empty_action, events);
        }
    }
}

void Engine::execute_action(const maple_combat_calculator::shared::Action& action) {
    for (auto& component : components_) {
        if (component->get_name() == action.name()) {
            auto events = component->process_action(action, entity_);
            if (!events.empty()) {
                logger_.record(clock_, action, events);
                break; // 하나의 컴포넌트만 Action을 처리한다고 가정
            }
        }
    }
}

}
