#include "TestFramework.h"
#include "utils/GameSnapshot.h"
#include <iostream>
#include "third_party/json_adapter.hpp"

using namespace Einstein;

TEST(MCTSSnapshot_Json_Debug)
{
    MCTSSnapshot ms;
    ms.total_iterations = 1234;
    ms.completed_iterations = 1234;
    ms.exploration_constant = 1.23;
    ms.time_limit = 2.0;
    ms.elapsed_time = 0.5;
    ms.best_evaluation = 0.42;
    ms.nodes_created = 10;
    ms.nodes_expanded = 8;

    // Build a small tree
    MCTSNodeSnapshot root;
    root.last_move = {{-1,-1},{-1,-1}};
    root.visits = 100;
    root.wins = 0.5;

    MCTSNodeSnapshot child;
    child.last_move = {{0,0},{1,1}};
    child.visits = 50;
    child.wins = 0.6;

    root.children.push_back(child);
    ms.root_node = root;

    std::string json_str = ms.ToJson();
    // Print JSON and original values for debugging
    std::cout << "--- MCTSSnapshot JSON ---\n" << json_str << "\n--- end JSON ---\n";
    std::cout << "Original total_iterations=" << ms.total_iterations << " root.visits=" << ms.root_node.visits << " child_count=" << ms.root_node.children.size() << "\n";

    // Direct parse using ein_json to inspect structure before invoking FromString
    try {
        auto root = ein_json::parse(json_str);
        if (root.is_object()) {
            auto obj = root.as_object();
            if (obj.count("root_node")) {
                auto rn = obj.at("root_node");
                std::cout << "direct parse: root_node is_object=" << (rn.is_object() ? 1 : 0) << "\n";
                if (rn.is_object()) {
                    auto rn_obj = rn.as_object();
                    if (rn_obj.count("children")) {
                        auto children_val = rn_obj.at("children");
                        std::cout << "direct parse: children is_array=" << (children_val.is_array() ? 1 : 0) << "\n";
                        if (children_val.is_array()) {
                            auto carr = children_val.as_array();
                            std::cout << "direct parse: children size=" << carr.size() << "\n";
                        }
                    }
                }
            }
        }
    } catch (...) {
        std::cout << "direct parse failed" << std::endl;
    }

    MCTSSnapshot parsed;
    parsed.FromString(json_str);

    std::cout << "Parsed total_iterations=" << parsed.total_iterations << " root.visits=" << parsed.root_node.visits << " child_count=" << parsed.root_node.children.size() << "\n";

    ASSERT_EQ(parsed.total_iterations, ms.total_iterations);
    ASSERT_EQ(parsed.root_node.visits, ms.root_node.visits);
    ASSERT_EQ(parsed.root_node.children.size(), ms.root_node.children.size());
}
