#include "mdc/inc/engine.h"
#include "mdc/inc/loader.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <char_preset.yaml> <mob_preset.yaml> <skills.yaml> <policy.yaml>" << std::endl;
        return 1;
    }

    std::string char_preset_path = argv[1];
    std::string mob_preset_path = argv[2];
    std::string skills_path = argv[3];
    std::string policy_path = argv[4];

    // 1. 데이터 로드
    auto char_info = mdc::DataLoader::load_character_info(char_preset_path);
    auto base_stat = mdc::DataLoader::load_character_preset(char_preset_path);
    auto monster_info = mdc::DataLoader::load_monster_preset(mob_preset_path);
    auto components = mdc::DataLoader::load_skills(skills_path);
    auto policy = mdc::DataLoader::load_policy(policy_path);

    // 2. 초기 CombatLog 설정
    maple_combat_calculator::shared::CombatLog initial_log;
    *initial_log.mutable_character_info() = char_info;
    *initial_log.mutable_base_stat() = base_stat;
    *initial_log.mutable_monster_info() = monster_info;

    // 3. 엔진 생성
    mdc::Engine engine(initial_log);

    // 4. 컴포넌트 및 정책 설정
    for (const auto& comp : components) {
        engine.add_component(comp);
    }
    engine.set_policy(std::move(policy));

    // 5. 시뮬레이션 실행
    engine.run(60.0); // 60초 동안 실행

    // 6. 결과 저장
    const auto& result_log = engine.get_combat_log();
    std::ofstream output("combat_log.bin", std::ios::binary);
    if (!result_log.SerializeToOstream(&output)) {
        std::cerr << "Failed to write combat log." << std::endl;
        return 1;
    }

    std::cout << "MDC simulation complete. combat_log.bin generated." << std::endl;
    std::cout << "Total operations: " << result_log.operation_logs_size() << std::endl;

    return 0;
}
