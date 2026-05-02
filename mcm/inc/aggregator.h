#pragma once

#include <string>
#include <map>

namespace mcm {

struct SkillRecord {
    std::string name;
    long long total_damage = 0;
    long long use_count = 0;
    long long hit_count = 0;
};

class DamageAggregator {
public:
    void set_combat_time(double start, double end);
    void record_damage(double timestamp, const std::string& skill_name, long long damage, long long hit_count);

    double get_dps() const;
    long long get_total_damage() const { return total_damage_; }
    const std::map<std::string, SkillRecord>& get_skill_records() const { return skill_records_; }
    void print_report() const;

private:
    double start_time_ = 0.0;
    double end_time_ = 0.0;
    long long total_damage_ = 0;
    std::map<std::string, SkillRecord> skill_records_;
};

}
