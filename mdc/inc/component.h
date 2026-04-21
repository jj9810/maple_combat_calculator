#pragma once

#include "shared/proto/internal/combat_log.pb.h"
#include <vector>
#include <string>
#include <memory>

namespace mdc {

// 전방 선언
class Entity;

// Component는 특정 스킬이나 행동의 로직을 담당합니다.
// Simaple의 Component와 유사하게, 상태(Entity)와 상호작용하여 이벤트를 생성합니다.
class Component {
public:
    virtual ~Component() = default;

    virtual std::string get_name() const = 0;

    // 주어진 Action을 처리하여 Event 목록을 반환합니다.
    // 처리할 수 없는 Action이면 빈 목록을 반환합니다.
    virtual std::vector<maple_combat_calculator::shared::Event> process_action(
        const maple_combat_calculator::shared::Action& action,
        Entity& entity
    ) = 0;

    // 시간이 경과했을 때 발생하는 이벤트(예: 쿨타임 감소, 도트 데미지)를 처리합니다.
    virtual std::vector<maple_combat_calculator::shared::Event> elapse(
        double time_delta,
        Entity& entity
    ) = 0;
};

}
