#pragma once

#include "nexon/skill_timeline.pb.h"
#include "nexon/battle_practice_character_info.pb.h"
#include "nexon/battle_practice_result.pb.h"
#include "nexon/replay_id.pb.h"

#include <string>
#include <vector>
#include <optional>
#include <google/protobuf/util/json_util.h>

namespace nexon_api {

/**
 * @class NexonApiLoader
 * @brief 넥슨 오픈 API와 통신하여 메이플스토리 전투 데이터를 가져오는 클래스입니다.
 */
class NexonApiLoader {
public:
    /**
     * @brief 생성자
     * @param api_key 넥슨 오픈 API 센터에서 발급받은 API 키
     */
    explicit NexonApiLoader(const std::string& api_key);
    ~NexonApiLoader();

    /**
     * @brief 캐릭터의 ocid를 기반으로 가장 최근의 replay_id를 획득합니다.
     */
    std::optional<std::string> get_replay_id(const std::string& ocid);

    /**
     * @brief 특정 전투의 최종 결과를 가져옵니다.
     */
    std::optional<maple_combat_calculator::shared::BattlePracticeResult> 
    get_battle_result(const std::string& replay_id);

    /**
     * @brief 특정 전투의 스킬 사용 타임라인 기록을 가져옵니다.
     */
    std::optional<maple_combat_calculator::shared::SkillTimeline> 
    get_skill_timeline(const std::string& replay_id, int page_no = 1);

    /**
     * @brief 특정 전투 시점의 캐릭터 상세 스탯 및 장비 정보를 가져옵니다.
     */
    std::optional<maple_combat_calculator::shared::BattlePracticeCharacterInfo> 
    get_character_info(const std::string& replay_id);

    /**
     * @brief JSON 응답을 Protobuf 메시지 객체로 변환하는 헬퍼 템플릿입니다.
     */
    template <typename T>
    bool json_to_proto(const std::string& json, T& proto) {
        if (json.empty()) return false;
        google::protobuf::util::JsonParseOptions options;
        options.ignore_unknown_fields = true;
        auto status = google::protobuf::util::JsonStringToMessage(json, &proto, options);
        return status.ok();
    }

private:
    /**
     * @brief 지정된 URL로 HTTP GET 요청을 보내고 응답 바디를 반환합니다.
     */
    std::string fetch_url(const std::string& url);

    std::string api_key_;
    const std::string base_url_ = "https://open.api.nexon.com/maplestory/v1/battle-practice/";
};

}
