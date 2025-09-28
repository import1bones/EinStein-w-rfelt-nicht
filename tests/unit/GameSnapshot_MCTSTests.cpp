#include "TestFramework.h"
#include "utils/GameSnapshot.h"
#include "game/GameState.h"
#include "ai/MCTS.h"
#include <memory>

using namespace Einstein;

TEST(MCTSSnapshot_Node_ToString_Roundtrip) {
    // Create node with children to exercise ToString loops
    MCTSNodeSnapshot root;
    root.last_move = {{0,0},{1,1}};
    root.visits = 10;
    root.wins = 2.5;
    root.ucb_value = 0.75;
    root.is_terminal = false;

    MCTSNodeSnapshot child;
    child.last_move = {{1,1},{2,2}};
    child.visits = 4;
    child.wins = 1.0;
    child.is_terminal = true;

    root.children.push_back(child);

    std::string out = root.ToString();
    ASSERT_FALSE(out.empty());
    // basic contents
    ASSERT_TRUE(out.find("NODE_VISITS") != std::string::npos);
    ASSERT_TRUE(out.find("CHILD_0:") != std::string::npos);
}

TEST(SnapshotGameRunner_CaptureRestore_MCTS_Instance) {
    SnapshotGameRunner runner;
    runner.SetVerbose(false);
    runner.SetMCTSIterations(1);
    runner.SetAIThinkingTime(0.01);

    // Without instance, CaptureMCTSState should return false
    MCTSSnapshot snapshot;
    bool res_no = runner.CaptureMCTSState(snapshot);
    ASSERT_FALSE(res_no);

    // Create a lightweight real MCTS instance and attach
    AIConfig cfg;
    cfg.mcts_iterations = 1;
    cfg.thinking_time = 0.01;
    cfg.enable_multithreading = false;
    auto mcts = std::make_shared<MCTS>(cfg);
    runner.SetMCTSInstance(mcts);

    // Now capture should succeed (returns true when instance attached)
    MCTSSnapshot captured;
    bool res_yes = runner.CaptureMCTSState(captured);
    ASSERT_TRUE(res_yes);

    // Restoring without instance cleared on a fresh runner should fail
    SnapshotGameRunner runner2;
    bool restore_no = runner2.RestoreMCTSState(captured);
    ASSERT_FALSE(restore_no);

    // Attach instance and restore should succeed
    runner2.SetMCTSInstance(mcts);
    bool restore_yes = runner2.RestoreMCTSState(captured);
    ASSERT_TRUE(restore_yes);
}

TEST(SnapshotGameRunner_RunSingleStep_GetAIMove_Path) {
    SnapshotGameRunner runner;
    runner.SetVerbose(false);
    runner.SetStepDelay(0);
    runner.SetMCTSIterations(1);
    runner.SetAIThinkingTime(0.005);

    // Run a single step from an empty snapshot (runner will create an initial game)
    int rc = runner.RunSingleStep("");
    // Expect success (0)
    ASSERT_EQ(rc, 0);
}
