#pragma once

#include "internal/combat_log.pb.h"
#include <string>

namespace mcm {

class LogLoader {
public:
    static bool load(const std::string& filepath, maple_combat_calculator::shared::CombatLog& combat_log);
};

}
