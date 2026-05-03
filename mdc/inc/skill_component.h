#pragma once

#include "component.h"
#include <string>
#include <vector>
#include <map>

namespace mdc {

/**
 * 스킬의 특정 레벨에서의 상세 수치입니다.
 */
struct SkillLevelStats {
    double damage = 0.0;
    int hit = 0;
    double duration = 0.0;
    double bonus_ignore_defense = 0.0;
    double bonus_boss_damage = 0.0;
    
    // 다단계 공격(Origin 등) 처리용
    struct Phase {
        std::string name;
        double damage;
        int hit;
        int count;
    };
    std::vector<Phase> phases;
};

/**
 * 레벨에 따른 수치 변화를 계산하기 위한 일차함수(y = ax + b) 파라미터입니다.
 */
struct LinearGrowth {
    double slope = 0.0;     // 기울기 (a)
    double intercept = 0.0; // 절편 (b)

    double eval(int level) const {
        if (level <= 0) return 0.0;
        return slope * level + intercept;
    }
};

struct SkillData {
    std::string name;
    std::string display_name;
    double cooldown = 0.0;
    double delay = 0.0;
    
    // 기본 수치 (비강화 상태)
    double damage = 0.0;
    int hit = 0;

    // 성장 공식 (HEXA 등)
    struct Growth {
        LinearGrowth damage;
        int hit = 0;
        std::map<int, double> bonus_ignore_defense;
        std::map<int, double> bonus_boss_damage;
    } growth;

    // 추가 공격 정보
    struct ExtraAttack {
        std::string name;
        double damage;
        int hit;
        int count;
    };
    std::vector<ExtraAttack> extra_attacks;
};

/**
 * 캐릭터의 강화 상태에 따라 적절한 수치를 선택하는 공격 컴포넌트입니다.
 */
class AttackSkillComponent : public Component {
public:
    AttackSkillComponent(const SkillData& data);
    
    std::string get_name() const override { return data_.name; }
    
    std::vector<maple_combat_calculator::shared::Event> process_action(
        const maple_combat_calculator::shared::Action& action,
        Entity& entity
    ) override;

    std::vector<maple_combat_calculator::shared::Event> elapse(
        double time_delta,
        Entity& entity
    ) override;

protected:
    SkillData data_;
};

/**
 * 버프 효과를 적용하는 컴포넌트입니다.
 */
class BuffSkillComponent : public AttackSkillComponent {
public:
    using AttackSkillComponent::AttackSkillComponent;

    std::vector<maple_combat_calculator::shared::Event> process_action(
        const maple_combat_calculator::shared::Action& action,
        Entity& entity
    ) override;
};

/**
 * 6차 전직(HEXA) 강화 여부를 판단하여 베이스/강화 스킬을 전환하는 컴포넌트입니다.
 */
class HexaSkillComponent : public Component {
public:
    HexaSkillComponent(std::shared_ptr<Component> base, std::shared_ptr<Component> upgraded);

    std::string get_name() const override { return base_->get_name(); }

    std::vector<maple_combat_calculator::shared::Event> process_action(
        const maple_combat_calculator::shared::Action& action,
        Entity& entity
    ) override;

    std::vector<maple_combat_calculator::shared::Event> elapse(
        double time_delta,
        Entity& entity
    ) override;

private:
    std::shared_ptr<Component> base_;
    std::shared_ptr<Component> upgraded_;
};

}
