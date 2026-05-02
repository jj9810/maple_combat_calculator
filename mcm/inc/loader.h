#pragma once

#include "internal/combat_log.pb.h"
#include <string>
#include <google/protobuf/util/json_util.h>

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

    /**
     * @brief 넥슨 API JSON 문자열들을 직접 받아 CombatLog로 변환합니다.
     */
    static bool load_nexon_json_from_string(
        const std::string& char_json,
        const std::string& timeline_json,
        maple_combat_calculator::shared::CombatLog& combat_log
    );

    /**
     * @brief JSON 응답을 Protobuf 메시지 객체로 변환하는 유틸리티입니다.
     */
    template <typename T>
    static bool json_to_proto(const std::string& json, T& proto) {
        if (json.empty()) return false;
        google::protobuf::util::JsonParseOptions options;
        options.ignore_unknown_fields = true;
        auto status = google::protobuf::util::JsonStringToMessage(json, &proto, options);
        return status.ok();
    }
};

}
