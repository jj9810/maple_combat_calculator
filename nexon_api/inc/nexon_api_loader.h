#pragma once

#include "shared/proto/skill_timeline.pb.h"
#include "shared/proto/battle_practice_character_info.pb.h"
#include "shared/proto/battle_practice_result.pb.h"
#include "shared/proto/replay_id.pb.h"

#include <string>
#include <vector>
#include <optional>
#include <google/protobuf/util/json_util.h>

namespace nexon_api {

class NexonApiLoader {
public:
    explicit NexonApiLoader(const std::string& api_key);
    ~NexonApiLoader();

    // 1. 캐릭터 OCID를 기반으로 리플레이 식별자 조회
    std::optional<std::string> get_replay_id(const std::string& ocid);

    // 2. 리플레이 측정 결과 조회
    std::optional<maple_combat_calculator::shared::BattlePracticeResult> 
    get_battle_result(const std::string& replay_id);

    // 3. 리플레이 스킬 타임라인 조회
    std::optional<maple_combat_calculator::shared::SkillTimeline> 
    get_skill_timeline(const std::string& replay_id, int page_no = 1);

    // 4. 리플레이 캐릭터 능력치 정보 조회
    std::optional<maple_combat_calculator::shared::BattlePracticeCharacterInfo> 
    get_character_info(const std::string& replay_id);

    // JSON 응답을 Protobuf로 변환하는 헬퍼
    template <typename T>
    bool json_to_proto(const std::string& json, T& proto) {
        if (json.empty()) return false;
        google::protobuf::util::JsonParseOptions options;
        options.ignore_unknown_fields = true;
        auto status = google::protobuf::util::JsonStringToMessage(json, &proto, options);
        return status.ok();
    }

private:
    // 공통 HTTP GET 요청 메서드
    std::string fetch_url(const std::string& url);

    std::string api_key_;
    const std::string base_url_ = "https://open.api.nexon.com/maplestory/v1/battle-practice/";
};

}
