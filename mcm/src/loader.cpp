#include "mcm/inc/loader.h"
#include <fstream>
#include <iostream>

namespace mcm {

bool LogLoader::load(const std::string& filepath, maple_combat_calculator::shared::CombatLog& combat_log) {
    std::ifstream input(filepath, std::ios::binary);
    if (!input) {
        std::cerr << "Error: Cannot open file " << filepath << std::endl;
        return false;
    }

    if (!combat_log.ParseFromIstream(&input)) {
        std::cerr << "Error: Failed to parse CombatLog from file." << std::endl;
        return false;
    }

    return true;
}

}
