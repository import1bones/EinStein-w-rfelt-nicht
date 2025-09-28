#include "TestFramework.h"
#include "utils/GameSnapshot.h"
#include "game/GameState.h"
#include <filesystem>

using namespace Einstein;

TEST(GameSnapshot_ToJson_FromString_Rich) {
    // Build a rich MCTSSnapshot with children
    MCTSSnapshot ms;
    ms.total_iterations = 10;
    ms.completed_iterations = 5;
    ms.avg_simulation_time = 0.02;
    // create a child tree
    MCTSNodeSnapshot root;
    root.last_move = {{0,0},{1,1}};
    root.visits = 100;
    MCTSNodeSnapshot child;
    child.last_move = {{1,1},{2,2}};
    child.visits = 50;
    root.children.push_back(child);
    ms.root_node = root;

    // AIThinkingSnapshot with mcts_state
    AIThinkingSnapshot ats;
    ats.mcts_iterations = 7;
    ats.thinking_time = 0.01;
    ats.has_mcts_data = true;
    ats.mcts_state = ms;
    ats.debug_info.push_back("dbg1");

    std::string json = ats.ToJson();
    ASSERT_FALSE(json.empty());

    AIThinkingSnapshot parsed;
    parsed.FromString(json);
    ASSERT_EQ(parsed.mcts_iterations, ats.mcts_iterations);
    ASSERT_TRUE(parsed.has_mcts_data == ats.has_mcts_data);
}

TEST(GameSnapshot_GameSnapshot_RoundTrip_Rich) {
    GameState gs;
    gs.NewGame(GameMode::AI_VS_AI, true);
    // mutate board a bit
    auto board = gs.GetBoard();
    board.SetPiece(0,0, 1);
    gs.SetBoard(board);

    GameSnapshot snap;
    snap.UpdateFromGameState(gs);
    snap.turn_number = 3;
    snap.current_phase = "moving";
    snap.debug_log.push_back("line1");

    // attach AI thinking with mcts snapshot
    MCTSSnapshot ms;
    ms.total_iterations = 3;
    ms.root_node.last_move = {{-1,-1},{-1,-1}};
    snap.ai_thinking.has_mcts_data = true;
    snap.ai_thinking.mcts_state = ms;

    std::string s = snap.ToString();
    ASSERT_FALSE(s.empty());

    GameSnapshot loaded;
    loaded.FromString(s);
    ASSERT_EQ(loaded.turn_number, snap.turn_number);
    ASSERT_EQ(loaded.current_phase, snap.current_phase);
}

TEST(SnapshotManager_CleanOldSnapshots_Latest) {
    SnapshotManager mgr;
    // create 4 snapshots
    GameState gs;
    gs.NewGame(GameMode::AI_VS_AI, true);
    std::vector<std::string> ids;
    for (int i = 0; i < 4; ++i) {
        auto id = mgr.CreateSnapshot(gs, "test", nullptr);
        ASSERT_FALSE(id.empty());
        ids.push_back(id);
    }

    auto list_before = mgr.ListSnapshots();
    ASSERT_TRUE(list_before.size() >= 4);

    mgr.CleanOldSnapshots(2);
    auto list_after = mgr.ListSnapshots();
    ASSERT_TRUE(list_after.size() <= 2);

    // cleanup
    for (auto &id : ids) mgr.DeleteSnapshot(id);
    try { std::filesystem::remove_all("snapshots"); } catch (...) {}
}

TEST(SnapshotGameRunner_HelperMethods) {
    SnapshotGameRunner runner;
    runner.SetVerbose(false);
    // CaptureMCTSStateForDebug should return a lightweight snapshot
    MCTSSnapshot dbg = runner.CaptureMCTSStateForDebug();
    ASSERT_TRUE(dbg.total_iterations > 0);

    // GetCurrentStatus should work even without explicit initialization
    std::string status = runner.GetCurrentStatus();
    ASSERT_FALSE(status.empty());

    // DumpAnalysis writes a file
    std::string fname = "analysis_test.txt";
    runner.DumpAnalysis(fname);
    ASSERT_TRUE(std::filesystem::exists(fname));
    try { std::filesystem::remove(fname); } catch(...) {}
}
