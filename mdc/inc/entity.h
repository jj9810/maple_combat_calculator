#pragma once

#include "shared/proto/internal/mcc_stat.pb.h"
#include <map>
#include <string>
#include <vector>

namespace mdc {

// Entity는 시뮬레이션 동안의 캐릭터 상태를 저장합니다.
class Entity {
public:
    Entity(const maple_combat_calculator::shared::Stat& initial_stat);

    const maple_combat_calculator::shared::Stat& get_stat() const { return current_stat_; }
    
    // 쿨타임 관리
    bool is_available(const std::string& skill_name) const;
    void start_cooldown(const std::string& skill_name, double duration);
    void elapse(double time_delta);

    // 6차 전직 및 스킬 레벨 관리
    bool is_hexa_active() const { return hexa_active_; }
    void set_hexa_active(bool active) { hexa_active_ = active; }
    
    int get_skill_level(const std::string& skill_name) const;
    void set_skill_level(const std::string& skill_name, int level);

private:
    maple_combat_calculator::shared::Stat current_stat_;
    std::map<std::string, double> cooldowns_;
    std::map<std::string, int> skill_levels_;
    bool hexa_active_ = true; // 기본적으로 활성화 상태로 가정
};

}
