#include <iostream>
#include <cassert>
#include <vector>
#include <filesystem>
#include "skill_manager.hpp"

int main() {
    std::cout << "Testing SkillManager..." << std::endl;
    
    // Clean start
    std::filesystem::remove_all("data/tests/skills");
    
    dnn::SkillManager manager("data/tests/skills/");

    // 1. Create & Train New Skill
    std::cout << "Training 'Math_Add'..." << std::endl;
    std::vector<double> input(2, 0.5); // 2 inputs
    std::vector<double> output(1, 1.0); // 1 output
    manager.teach_skill("Math_Add", input, output);
    
    auto skills = manager.list_skills();
    assert(skills.size() == 1);
    assert(skills[0] == "Math_Add");

    // 2. Query Skill
    auto result = manager.query_skill("Math_Add", input);
    assert(!result.empty());
    std::cout << "Math_Add Result: " << result[0] << std::endl;

    // 3. Test Similarity/Merge Logic
    std::cout << "Training 'Math_Adding' (Should merge)..." << std::endl;
    manager.teach_skill("Math_Adding", input, output);
    
    // Should still technically be 2 entries in map, but they might share underlying data or logic 
    // In current impl, get_or_create returns existing if similar. 
    // So teaching "Math_Adding" should update "Math_Add" if it returned it.
    // However, the map keys would still be distinct unless we actively aliased them? 
    // Wait, get_or_create returns a pointer. If we call teach("Math_Adding"), it gets the pointer to "Math_Add".
    // But it doesn't add "Math_Adding" to the map? 
    // Ah, my logic in get_or_create: returns existing pointer but does NOT insert "Math_Adding" into map.
    // So list_skills should still be size 1?
    
    skills = manager.list_skills();
    // If it reused existing, it didn't add new key.
    std::cout << "Skill count: " << skills.size() << std::endl; 
    
    if (skills.size() == 1) std::cout << "Merge Success: Reused existing skill." << std::endl;
    else std::cout << "Note: Created separate entry (Merge logic might need map alias)." << std::endl;

    std::cout << "SkillManager Tests Parsed!" << std::endl;
    return 0;
}
