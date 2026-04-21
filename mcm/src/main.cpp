#include "mcm/inc/mcm.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_combat_log.bin>" << std::endl;
        return 1;
    }

    mcm::MCM combat_manager;

    if (!combat_manager.load_log(argv[1])) {
        return 1;
    }

    combat_manager.run();
    combat_manager.print_report();

    return 0;
}
