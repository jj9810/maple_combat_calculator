#include "combat_power.h"
#include "skill_damage.h"
#include <iostream>
#include <cassert>

void test_combat_power() {
    std::cout << "Testing calculateCombatPower..." << std::endl;
    
    // Simple test case with dummy values
    int cp = calculateCombatPower(
        10000, // mainStat
        2000,  // subStat
        1000,  // flatAtt
        500,   // weaponBaseAtt
        100,   // weaponSfAtt
        600,   // convertedWeaponAtt
        20.0,  // attPercent
        50.0,  // critDmg
        35.0,  // innateCritDmg
        100.0, // bossDmg
        0.0,   // innateBossDmg
        50.0,  // dmg
        0.0,   // innateDmg
        20.0,  // finalDmg
        0.0    // innateFinalDmg
    );
    
    std::cout << "Calculated Combat Power: " << cp << std::endl;
    assert(cp > 0);
    std::cout << "Combat Power Test Success!" << std::endl;
}

void test_skill_damage() {
    std::cout << "Testing calcSkillDamageRaw..." << std::endl;
    
    long long damage = calcSkillDamageRaw(
        500.0,  // skillDamage
        10000.0, // mainStat
        2000.0,  // subStat
        1000.0,  // attack
        0.95,    // mastery
        50.0,    // damagePercent
        20.0,    // finalDamagePercent
        1.0,     // critRate
        50.0,    // critDamagePercent
        90.0,    // ignoreDefense
        1.0,     // elementalAdjust
        300.0,   // mobDefense
        0.5,     // mobElemRes
        1.5,     // weaponConst
        1.1,     // levelAdjust
        1.0      // forceAdjust
    );
    
    std::cout << "Calculated Skill Damage: " << damage << std::endl;
    assert(damage > 0);
    std::cout << "Skill Damage Test Success!" << std::endl;
}

int main() {
    test_combat_power();
    test_skill_damage();
    return 0;
}
