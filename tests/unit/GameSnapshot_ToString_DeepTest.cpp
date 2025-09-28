#include "TestFramework.h"
#include "utils/GameSnapshot.h"

using namespace Einstein;

TEST(GameSnapshot_MCTSSnapshot_ToString_Deep) {
    // Build a deeper tree structure to exercise recursive ToString loops
    MCTSSnapshot ms;
    ms.total_iterations = 42;
    ms.completed_iterations = 21;
    ms.best_evaluation = 3.14;

    MCTSNodeSnapshot root;
    root.last_move = {{0,0},{1,1}};
    root.visits = 1000;
    root.wins = 250.5;

    // Create several children and grandchildren
    for (int i = 0; i < 4; ++i) {
        MCTSNodeSnapshot c;
        c.last_move = {{i,i},{i+1,i+1}};
        c.visits = 100 - i*10;
        c.wins = 10.0 + i;
        for (int j = 0; j < 3; ++j) {
            MCTSNodeSnapshot g;
            g.last_move = {{i,j},{j,i}};
            g.visits = j * 2 + 1;
            g.wins = j * 0.5;
            c.children.push_back(g);
        }
        root.children.push_back(c);
    }

    ms.root_node = root;

    std::string s = ms.ToString();
    ASSERT_FALSE(s.empty());
    // Ensure some expected markers are present
    ASSERT_TRUE(s.find("MCTS_ROOT_NODE") != std::string::npos || s.find("NODE_POS") != std::string::npos);
    ASSERT_TRUE(s.find("NODE_CHILDREN") != std::string::npos);
}
