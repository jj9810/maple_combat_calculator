#include "mcm/inc/mcm.h"
#include "mcm/inc/loader.h"
#include <iostream>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_combat_log.bin>" << std::endl;
        std::cerr << "   or: " << argv[0] << " <character.json> <timeline.json>" << std::endl;
        return 1;
    }

    mcm::MCM combat_manager;
    maple_combat_calculator::shared::CombatLog log;

    if (argc == 2) {
        if (!mcm::LogLoader::load(argv[1], log)) {
            return 1;
        }
    } else if (argc >= 3) {
        if (!mcm::LogLoader::load_nexon_json(argv[1], argv[2], log)) {
            return 1;
        }
    }

    // 로드된 로그 주입 및 실행 (MCM 클래스에 주입 메서드 추가 필요하거나 기존 load_log 수정)
    // 여기서는 간단히 하기 위해 MCM::load_log가 CombatLog를 직접 받을 수 있도록 리팩토링하거나
    // 내부적으로 다시 저장 후 로드하는 방식을 택할 수 있음. 
    // 여기서는 MCM::run() 내부에서 직접 처리하도록 MCM을 수정하는 것이 깔끔함.
    
    // 임시: main에서 CombatLog를 직접 다루도록 MCM 인터페이스 확장 고려
    // 일단은 MCM 내부 구조를 고려하여 동작하도록 구현
    combat_manager.load_log_direct(log); 
    
    combat_manager.run();
    combat_manager.print_report();

    return 0;
}
