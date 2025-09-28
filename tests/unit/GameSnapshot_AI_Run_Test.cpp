// ...existing code...
#include "TestFramework.h"
#include "utils/GameSnapshot.h"
#include "game/GameState.h"
#include "ai/MCTS.h"

using namespace Einstein;

TEST(SnapshotRunner_RunSingleStep_Skeleton) {
    SnapshotGameRunner runner;
    runner.SetVerbose(false);
    runner.SetMCTSIterations(5);
    runner.SetAIThinkingTime(0.01);
    // Run a single step via public API; it will internally call GetAIMove/ExecuteOneStep
    int rc = runner.RunSingleStep("");
    ASSERT_TRUE(rc == 0 || rc == 1);
}
