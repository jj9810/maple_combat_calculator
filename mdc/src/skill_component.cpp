#include "mdc/inc/skill_component.h"
#include "mdc/inc/entity.h"
#include "shared/proto/internal/combat_log.pb.h"
#include <iostream>

namespace mdc {

AttackSkillComponent::AttackSkillComponent(const SkillData& data) : data_(data) {}

std::vector<maple_combat_calculator::shared::Event> AttackSkillComponent::process_action(
    const maple_combat_calculator::shared::Action& action,
    Entity& entity
) {
    std::vector<maple_combat_calculator::shared::Event> events;

    if (action.skill_name() != data_.name) {
        return events;
    }

    if (!entity.is_available(data_.name)) {
        return events;
    }

    int level = entity.get_skill_level(data_.name);
    
    // 데미지 결정 (레벨 정보가 있으면 성장 공식 사용, 없으면 기본값)
    double current_damage = (level > 0) ? data_.growth.damage.eval(level) : data_.damage;
    int current_hit = (level > 0 && data_.growth.hit > 0) ? data_.growth.hit : data_.hit;

    // 1. 메인 공격 이벤트 생성
    maple_combat_calculator::shared::Event main_event;
    main_event.set_method(maple_combat_calculator::shared::Event::DAMAGE);
    main_event.set_name(data_.display_name);
    main_event.set_damage(current_damage);
    main_event.set_hit(current_hit);
    
    // 보너스 스탯 처리 (방무, 보공 등)
    if (level > 0) {
        auto* payload = main_event.mutable_payload();
        
        // 해당 레벨 이하의 가장 높은 보너스 적용 (메이플 특성상 특정 구간마다 해금됨)
        double bid = 0;
        for (auto const& [lv, val] : data_.growth.bonus_ignore_defense) {
            if (level >= lv) bid = std::max(bid, val);
        }
        if (bid > 0) (*payload->mutable_fields())["bonus_ignore_defense"] = google::protobuf::Value().set_number_value(bid);

        double bbd = 0;
        for (auto const& [lv, val] : data_.growth.bonus_boss_damage) {
            if (level >= lv) bbd = std::max(bbd, val);
        }
        if (bbd > 0) (*payload->mutable_fields())["bonus_boss_damage"] = google::protobuf::Value().set_number_value(bbd);
    }
    
    events.push_back(main_event);

    // 2. 추가 공격(Extra Attacks) 처리
    for (const auto& extra : data_.extra_attacks) {
        for (int i = 0; i < extra.count; ++i) {
            maple_combat_calculator::shared::Event extra_event;
            extra_event.set_method(maple_combat_calculator::shared::Event::DAMAGE);
            extra_event.set_name(extra.name);
            extra_event.set_damage(extra.damage);
            extra_event.set_hit(extra.hit);
            events.push_back(extra_event);
        }
    }

    if (data_.cooldown > 0) {
        entity.start_cooldown(data_.name, data_.cooldown);
    }

    return events;
}

std::vector<maple_combat_calculator::shared::Event> AttackSkillComponent::elapse(
    double time_delta,
    Entity& entity
) {
    // 일반 공격 스킬은 시간 경과에 따른 자동 이벤트가 없음 (Summon 제외)
    return {};
}

std::vector<maple_combat_calculator::shared::Event> BuffSkillComponent::process_action(
    const maple_combat_calculator::shared::Action& action,
    Entity& entity
) {
    auto events = AttackSkillComponent::process_action(action, entity);
    if (events.empty()) return events;

    // 버프 적용 이벤트 추가 (실제로는 duration 관리가 필요하지만 현재는 적용/해제 이벤트만 처리)
    maple_combat_calculator::shared::Event buff_event;
    buff_event.set_method(maple_combat_calculator::shared::Event::BUFF);
    buff_event.set_name(data_.display_name);
    buff_event.set_hit(1); // 1: 적용
    // payload 설정은 생략 (추후 데이터에서 파싱)
    events.push_back(buff_event);

    return events;
}

HexaSkillComponent::HexaSkillComponent(std::shared_ptr<Component> base, std::shared_ptr<Component> upgraded)
    : base_(base), upgraded_(upgraded) {}

std::vector<maple_combat_calculator::shared::Event> HexaSkillComponent::process_action(
    const maple_combat_calculator::shared::Action& action,
    Entity& entity
) {
    // 엔티티 상태에 따라 분기
    if (entity.is_hexa_active()) {
        return upgraded_->process_action(action, entity);
    }
    return base_->process_action(action, entity);
}

std::vector<maple_combat_calculator::shared::Event> HexaSkillComponent::elapse(
    double time_delta,
    Entity& entity
) {
    if (entity.is_hexa_active()) {
        return upgraded_->elapse(time_delta, entity);
    }
    return base_->elapse(time_delta, entity);
}

}
