#include "TestFramework.h"
#include "utils/GameSnapshot.h"

using namespace Einstein;

TEST(MCTSSnapshot_Json_RoundTrip)
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
    ASSERT_FALSE(json_str.empty());

    // Should parse with ein_json and produce a similar object on FromString
    MCTSSnapshot parsed;
    parsed.FromString(json_str);

    ASSERT_EQ(parsed.total_iterations, ms.total_iterations);
    ASSERT_EQ(parsed.root_node.visits, ms.root_node.visits);
    ASSERT_EQ(parsed.root_node.children.size(), ms.root_node.children.size());
}
