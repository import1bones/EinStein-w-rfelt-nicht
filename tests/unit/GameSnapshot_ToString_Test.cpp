// ...existing code...
#include "TestFramework.h"
#include "utils/GameSnapshot.h"
#include "game/GameState.h"

using namespace Einstein;

TEST(GameSnapshot_MCTSSnapshot_ToString) {
    MCTSSnapshot s;
    s.total_iterations = 10;
    s.completed_iterations = 5;
    s.avg_simulation_time = 0.01;
    // add a principal variation
    s.principal_variation.push_back({{0,0},{1,1}});
    std::string out = s.ToString();
    ASSERT_TRUE(!out.empty());
}

TEST(GameSnapshot_GameSnapshot_ToString) {
    GameSnapshot gs;
    gs.turn_number = 2;
    gs.current_player = Player::LEFT_TOP;
    std::string out = gs.ToString();
    ASSERT_TRUE(!out.empty());
}
