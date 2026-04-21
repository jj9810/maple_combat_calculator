#include "mcm/inc/mcm.h"
#include "mcm/inc/loader.h"
#include "mcm/inc/connector.h"
#include "mcc/inc/skill_damage.h"

#include <iostream>

namespace mcm {

bool MCM::load_log(const std::string& filepath) {
    return LogLoader::load(filepath, combat_log_);
}

void MCM::run() {
    if (combat_log_.operation_logs_size() == 0) {
        std::cout << "Log is empty." << std::endl;
        return;
    }

    // Context 초기화
    context_ = std::make_unique<SimulationContext>(
        combat_log_.base_stat(),
        combat_log_.character_info(),
        combat_log_.monster_info()
    );

    // 전투 시간 설정
    double start_time = combat_log_.operation_logs(0).play_logs(0).clock();
    const auto& last_op = combat_log_.operation_logs(combat_log_.operation_logs_size() - 1);
    const auto& last_play = last_op.play_logs(last_op.play_logs_size() - 1);
    double end_time = last_play.clock();
    aggregator_.set_combat_time(start_time, end_time);

    // 모든 로그 순회
    for (const auto& op_log : combat_log_.operation_logs()) {
        process_operation(op_log);
    }
}

void MCM::print_report() {
    aggregator_.print_report();
}

void MCM::process_operation(const maple_combat_calculator::shared::OperationLog& op_log) {
    for (const auto& play_log : op_log.play_logs()) {
        process_playlog(play_log);
    }
}

void MCM::process_playlog(const maple_combat_calculator::shared::PlayLog& play_log) {
    for (const auto& event : play_log.events()) {
        process_event(event, play_log.clock());
    }
}

void MCM::process_event(const maple_combat_calculator::shared::Event& event, double clock) {
    switch (event.method()) {
        case maple_combat_calculator::shared::Event::DAMAGE:
            handle_damage_event(event, clock);
            break;
        case maple_combat_calculator::shared::Event::BUFF:
            handle_buff_event(event);
            break;
        case maple_combat_calculator::shared::Event::DOT:
            handle_dot_event(event, clock);
            break;
        case maple_combat_calculator::shared::Event::OTHER:
        default:
            // 무시
            break;
    }
}

void MCM::handle_damage_event(const maple_combat_calculator::shared::Event& event, double clock) {
    // 1. Connector를 통해 MCC 연산 수행
    long long single_line_damage = MCMConnector::calculate_damage(*context_, event);

    // 2. 결과 집계
    aggregator_.record_damage(clock, event.name(), single_line_damage, event.hit());
}

void MCM::handle_buff_event(const maple_combat_calculator::shared::Event& event) {
    if (event.hit() > 0) {
        // 버프 적용
        auto parsed_stat = MCMConnector::parse_stat_from_payload(event.payload());
        context_->apply_buff(event.name(), parsed_stat);
    } else {
        // 버프 해제
        context_->remove_buff(event.name());
    }
}

void MCM::handle_dot_event(const maple_combat_calculator::shared::Event& event, double clock) {
    // 1. Connector를 통해 MCC DOT 연산 수행
    long long dot_damage = MCMConnector::calculate_dot_damage(*context_, event);

    // 2. 결과 집계 (DOT은 보통 단일 히트로 처리)
    aggregator_.record_damage(clock, event.name(), dot_damage, 1);
}

}
