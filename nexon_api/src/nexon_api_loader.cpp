#include "nexon_api_loader.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>

namespace nexon_api {

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

NexonApiLoader::NexonApiLoader(const std::string& api_key) : api_key_(api_key) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

NexonApiLoader::~NexonApiLoader() {
    curl_global_cleanup();
}

std::optional<std::string> NexonApiLoader::get_replay_id(const std::string& ocid) {
    std::string url = base_url_ + "replay-id?ocid=" + ocid; 
    std::string response = fetch_url(url);
    if (response.empty()) return std::nullopt;
    
    // TODO: JSON 파싱 로직을 통해 실제 replay_id만 추출해야 함
    return response; 
}

std::optional<maple_combat_calculator::shared::BattlePracticeResult> 
NexonApiLoader::get_battle_result(const std::string& replay_id) {
    std::string url = base_url_ + "battle-result?replay_id=" + replay_id;
    std::string response = fetch_url(url);
    maple_combat_calculator::shared::BattlePracticeResult result;
    if (json_to_proto(response, result)) return result;
    return std::nullopt;
}

std::optional<maple_combat_calculator::shared::SkillTimeline> 
NexonApiLoader::get_skill_timeline(const std::string& replay_id, int page_no) {
    std::string url = base_url_ + "skill-timeline?replay_id=" + replay_id + "&page_no=" + std::to_string(page_no);
    std::string response = fetch_url(url);
    maple_combat_calculator::shared::SkillTimeline timeline;
    if (json_to_proto(response, timeline)) return timeline;
    return std::nullopt;
}

std::optional<maple_combat_calculator::shared::BattlePracticeCharacterInfo> 
NexonApiLoader::get_character_info(const std::string& replay_id) {
    std::string url = base_url_ + "character-info?replay_id=" + replay_id;
    std::string response = fetch_url(url);
    maple_combat_calculator::shared::BattlePracticeCharacterInfo info;
    if (json_to_proto(response, info)) return info;
    return std::nullopt;
}

std::string NexonApiLoader::fetch_url(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("x-nxopen-api-key: " + api_key_).c_str());
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return readBuffer;
}

}
