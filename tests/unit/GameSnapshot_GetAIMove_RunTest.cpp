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
    // Run a single step to exercise GetAIMove internally and ensure it completes
    int rc = runner.RunSingleStep("");
    ASSERT_EQ(rc, 0);
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
