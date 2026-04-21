#pragma once

#include "context.h"
#include "aggregator.h"
#include "internal/combat_log.pb.h"
#include <string>
#include <memory>

namespace mcm {

class MCM {
public:
    bool load_log(const std::string& filepath);
    void run();
    void print_report();

private:
    void process_operation(const maple_combat_calculator::shared::OperationLog& op_log);
    void process_playlog(const maple_combat_calculator::shared::PlayLog& play_log);
    void process_event(const maple_combat_calculator::shared::Event& event, double clock);

    // 이벤트 핸들러
    void handle_damage_event(const maple_combat_calculator::shared::Event& event, double clock);
    void handle_buff_event(const maple_combat_calculator::shared::Event& event);
    void handle_dot_event(const maple_combat_calculator::shared::Event& event, double clock);

    std::unique_ptr<SimulationContext> context_;
    DamageAggregator aggregator_;
    maple_combat_calculator::shared::CombatLog combat_log_;
};

}
