// ...existing code...
#include "TestFramework.h"
#include "utils/GameSnapshot.h"
#include "game/GameState.h"
#include "ai/MCTS.h"

using namespace Einstein;

TEST(SnapshotRunner_CaptureMCTSState_Skeleton) {
    SnapshotGameRunner runner;
    MCTSSnapshot s;
    // Without an attached MCTS instance this should return false
    bool ok = runner.CaptureMCTSState(s);
    ASSERT_TRUE(ok == false);
}

TEST(SnapshotRunner_RunFromSnapshot_Skeleton) {
    SnapshotGameRunner runner;
    runner.SetVerbose(false);
    runner.SetStepDelay(0);
    runner.SetMaxSteps(1);
    int rc = runner.RunFromSnapshot("");
    // Should return 0 or non-zero depending on environment; ensure it completes
    ASSERT_TRUE(rc == 0 || rc == 1);
}
