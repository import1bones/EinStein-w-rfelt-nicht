#include "TestFramework.h"
#include "utils/GameSnapshot.h"

using namespace Einstein;

TEST(Snapshot_RoundTrip)
{
    GameSnapshot s;
    s.snapshot_id = "test_snap";
    s.turn_number = 5;
    s.current_player = Player::RIGHT_BOTTOM;
    s.current_dice = 3;
    s.current_phase = "test_phase";

    // Add a simple move history
    Move mv = {{0,0},{1,1}};
    GameMove gm(mv, Player::LEFT_TOP, 2);
    s.move_history.push_back(gm);

    // AI thinking snapshot
    AIThinkingSnapshot ai;
    ai.mcts_iterations = 10;
    ai.thinking_time = 0.5;
    ai.position_evaluation = 1.23;

    MCTSSnapshot ms;
    ms.total_iterations = 10;
    ms.best_evaluation = 1.23;
    ms.principal_variation.push_back(mv);
    ai.mcts_state = ms;
    ai.has_mcts_data = true;

    s.ai_thinking = ai;

    std::string dumped = s.ToString();
    GameSnapshot parsed;
    parsed.FromString(dumped);

    ASSERT_STREQ(s.snapshot_id.c_str(), parsed.snapshot_id.c_str());
    ASSERT_EQ(s.turn_number, parsed.turn_number);
    ASSERT_EQ(static_cast<int>(s.current_player), static_cast<int>(parsed.current_player));
    ASSERT_EQ(s.current_dice, parsed.current_dice);
    ASSERT_EQ(s.move_history.size(), parsed.move_history.size());
    ASSERT_TRUE(parsed.ai_thinking.has_mcts_data);
    ASSERT_EQ(parsed.ai_thinking.mcts_iterations, ai.mcts_iterations);
}
