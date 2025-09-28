#include "TestFramework.h"
#include "utils/GameSnapshot.h"
#include "game/GameState.h"
#include <memory>

using namespace Einstein;

TEST(GameSnapshot_GetAIMove_FastMCTS) {
    SnapshotGameRunner runner;
    runner.SetVerbose(false);
    runner.SetStepDelay(0);
    runner.SetMCTSIterations(1);
    runner.SetAIThinkingTime(0.001);

    // Create a game state and ensure GetAIMove returns a valid move
    GameState gs;
    gs.NewGame(GameMode::AI_VS_AI, true);
    AIThinkingSnapshot ai;
    Move mv = runner.GetAIMove(gs, ai);
    // Either a valid move or fallback valid_moves[0]; check structure
    ASSERT_TRUE(mv.first.first >= -1 && mv.second.first >= -1);
    // ai should have some basic fields set
    ASSERT_TRUE(ai.thinking_time >= 0.0);
}

TEST(SnapshotGameRunner_RunFromSnapshot_NoSnapshotsCreatesOne) {
    SnapshotGameRunner runner;
    runner.SetVerbose(false);
    runner.SetStepDelay(0);
    runner.SetMCTSIterations(1);
    runner.SetAIThinkingTime(0.001);

    // Ensure running from empty snapshot creates initial snapshot and returns 0
    int rc = runner.RunFromSnapshot("");
    ASSERT_EQ(rc, 0);
}
