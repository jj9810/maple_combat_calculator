#pragma once

#include "shared/proto/internal/combat_log.pb.h"

namespace mdc {

class Logger {
public:
    Logger(const maple_combat_calculator::shared::CombatLog& initial_log);

    void record(double clock,
                const maple_combat_calculator::shared::Action& action,
                const std::vector<maple_combat_calculator::shared::Event>& events);

    const maple_combat_calculator::shared::CombatLog& get_log() const;

private:
    maple_combat_calculator::shared::CombatLog combat_log_;
};

}
