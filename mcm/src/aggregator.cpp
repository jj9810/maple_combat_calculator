#include "mcm/inc/aggregator.h"
#include <iostream>
#include <iomanip>
#include <numeric>
#include <vector>
#include <algorithm>

namespace mcm {

void DamageAggregator::set_combat_time(double start, double end) {
    start_time_ = start;
    end_time_ = end;
}

void DamageAggregator::record_damage(double timestamp, const std::string& skill_name, long long damage, long long hit_count) {
    total_damage_ += damage * hit_count;

    auto& record = skill_records_[skill_name];
    record.name = skill_name;
    record.total_damage += damage * hit_count;
    record.use_count += 1; // 정확한 집계를 위해선 Action과 연동 필요
    record.hit_count += hit_count;
}

double DamageAggregator::get_dps() const {
    double duration = end_time_ - start_time_;
    if (duration <= 0) {
        return 0;
    }
    return static_cast<double>(total_damage_) / duration;
}

void DamageAggregator::print_report() const {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "========================================\n";
    std::cout << "           MCM Damage Report            \n";
    std::cout << "========================================\n";
    std::cout << "Total Damage: " << total_damage_ << "\n";
    std::cout << "Combat Duration: " << (end_time_ - start_time_) << "s\n";
    std::cout << "DPS: " << get_dps() << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Skill Breakdown:\n";

    // 점유율 계산을 위해 정렬
    std::vector<std::pair<std::string, SkillRecord>> sorted_records(skill_records_.begin(), skill_records_.end());
    std::sort(sorted_records.begin(), sorted_records.end(),
        [](const auto& a, const auto& b) {
            return a.second.total_damage > b.second.total_damage;
        });

    for (const auto& pair : sorted_records) {
        const auto& record = pair.second;
        double percentage = (static_cast<double>(record.total_damage) / total_damage_) * 100.0;
        std::cout << std::setw(20) << std::left << record.name
                  << std::setw(15) << std::right << record.total_damage
                  << " (" << percentage << "%)\n";
    }
    std::cout << "========================================\n";
}

}
