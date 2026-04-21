#include "mdc/inc/logger.h"

namespace mdc {

Logger::Logger(const maple_combat_calculator::shared::CombatLog& initial_log)
    : combat_log_(initial_log) {}

void Logger::record(double clock,
                    const maple_combat_calculator::shared::Action& action,
                    const std::vector<maple_combat_calculator::shared::Event>& events) {

    auto* op_log = combat_log_.add_operation_logs();
    auto* play_log = op_log->add_play_logs();

    play_log->set_clock(clock);
    *play_log->mutable_action() = action;

    for (const auto& event : events) {
        *play_log->add_events() = event;
    }
}

const maple_combat_calculator::shared::CombatLog& Logger::get_log() const {
    return combat_log_;
}

}
