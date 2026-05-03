#include <emscripten/bind.h>
#include "mcm/inc/mcm.h"
#include "mcm/inc/loader.h"
#include <string>
#include <sstream>
#include <iomanip>

using namespace emscripten;

namespace {

/**
 * @brief 시뮬레이션 결과를 JSON 문자열로 변환합니다.
 */
std::string report_to_json(const mcm::MCM& combat_manager) {
    const auto& aggregator = combat_manager.get_aggregator();
    const auto& records = aggregator.get_skill_records();

    std::stringstream ss;
    ss << "{";
    ss << "\"total_damage\": " << aggregator.get_total_damage() << ",";
    ss << "\"dps\": " << static_cast<long long>(aggregator.get_dps()) << ",";
    ss << "\"skills\": [";

    bool first = true;
    for (const auto& [name, record] : records) {
        if (!first) ss << ",";
        ss << "{";
        ss << "\"name\": \"" << name << "\",";
        ss << "\"total_damage\": " << record.total_damage << ",";
        ss << "\"use_count\": " << record.use_count << ",";
        ss << "\"hit_count\": " << record.hit_count;
        ss << "}";
        first = false;
    }

    ss << "]}";
    return ss.str();
}

} // namespace

/**
 * @brief 자바스크립트에서 호출할 메인 시뮬레이션 함수입니다.
 */
std::string calculate_battle_report(std::string char_json, std::string timeline_json) {
    mcm::MCM combat_manager;
    maple_combat_calculator::shared::CombatLog log;

    // 1. JSON을 Protobuf로 변환 및 로드
    if (!mcm::LogLoader::load_nexon_json_from_string(char_json, timeline_json, log)) {
        return "{\"error\": \"Failed to parse input JSON\"}";
    }

    // 2. 시뮬레이션 실행
    combat_manager.load_log_direct(log);
    combat_manager.run();

    // 3. 결과 리포트를 JSON으로 반환
    return report_to_json(combat_manager);
}

/**
 * @brief Emscripten 바인딩 정의
 */
EMSCRIPTEN_BINDINGS(mcm_wasm) {
    function("calculateBattleReport", &calculate_battle_report);
}
