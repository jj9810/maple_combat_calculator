#pragma once

#include "internal/combat_log.pb.h"
#include <string>

namespace mcm {

class LogLoader {
public:
    static bool load(const std::string& filepath, maple_combat_calculator::shared::CombatLog& combat_log);

    /**
     * @brief 넥슨 API JSON 파일들을 읽어 CombatLog로 변환합니다.
     */
    static bool load_nexon_json(
        const std::string& character_json_path,
        const std::string& timeline_json_path,
        maple_combat_calculator::shared::CombatLog& combat_log
    );
};

}
