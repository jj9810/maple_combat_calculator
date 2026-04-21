#pragma once

#include "battle_practice_character_info.pb.h"
#include "component.h"
#include "policy.h"
#include <string>
#include <vector>
#include <memory>

namespace mdc {

class DataLoader {
public:
    // 프리셋 파일에서 캐릭터 정보 로드 (BattlePracticeCharacterInfo 형식으로 통합)
    static maple_combat_calculator::shared::BattlePracticeCharacterInfo load_character_info(const std::string& filepath);

    // 프리셋 파일에서 캐릭터 기본 스탯 로드 (BattlePracticeCharacterInfo 내부에 포함되도록 처리)
    static maple_combat_calculator::shared::BattlePracticeCharacterInfo load_character_preset(const std::string& filepath);

    // 몬스터 프리셋 파일에서 몬스터 정보 로드
    static maple_combat_calculator::shared::MonsterInfo load_monster_preset(const std::string& filepath);

    // 스킬 데이터 파일에서 컴포넌트 목록 로드
    static std::vector<std::shared_ptr<Component>> load_skills(const std::string& filepath);

    // 정책 파일에서 Policy 로드
    static std::unique_ptr<Policy> load_policy(const std::string& filepath);
};

}
