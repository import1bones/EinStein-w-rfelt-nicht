#include "TestFramework.h"
#include "utils/GameSnapshot.h"
#include "game/GameState.h"
#include <filesystem>

using namespace Einstein;

TEST(MCTSSnapshot_GetBestMoves_And_Summary) {
    MCTSSnapshot ms;
    ms.total_iterations = 100;
    ms.completed_iterations = 40;
    ms.principal_variation.push_back({{0,0},{1,1}});
    ms.principal_variation.push_back({{1,1},{2,2}});

    auto best = ms.GetBestMoves(1);
    ASSERT_EQ(best.size(), 1);
    auto sum = ms.GetSearchSummary();
    ASSERT_TRUE(sum.find("40/100") != std::string::npos || sum.find("iterations") != std::string::npos);
}

TEST(GameSnapshot_Heavy_ToString) {
    GameState gs;
    gs.NewGame(GameMode::AI_VS_AI, true);

    // Change several pieces
    auto b = gs.GetBoard();
    b.SetPiece(0,0, 1);
    b.SetPiece(1,1, -2);
    b.SetPiece(2,2, 3);
    gs.SetBoard(b);

    GameSnapshot snap;
    snap.UpdateFromGameState(gs);
    snap.turn_number = 7;
    snap.current_phase = "analyzing";

    // valid moves
    snap.current_valid_moves.push_back({{0,0},{1,0}});
    snap.current_valid_moves.push_back({{1,1},{2,1}});

    // suggested move
    snap.suggested_move = {{0,0},{1,0}};

    // move history
    GameMove gm({{0,0},{1,0}}, Player::LEFT_TOP, 3);
    snap.move_history.push_back(gm);

    // AI thinking with debug and mcts state
    AIThinkingSnapshot ats;
    ats.mcts_iterations = 5;
    ats.thinking_time = 0.02;
    ats.debug_info.push_back("dbgA");
    ats.debug_info.push_back("dbgB");

    MCTSSnapshot ms;
    ms.root_node.visits = 12;
    ms.search_debug_info.push_back("sdebug");
    ms.principal_variation.push_back({{0,0},{1,1}});
    ats.mcts_state = ms;
    ats.has_mcts_data = true;

    snap.ai_thinking = ats;
    snap.debug_log.push_back("line1");
    snap.debug_log.push_back("line2");

    std::string out = snap.ToString();
    ASSERT_FALSE(out.empty());
    ASSERT_TRUE(out.find("valid_moves") != std::string::npos);
    ASSERT_TRUE(out.find("ai_thinking") != std::string::npos);
}

TEST(SnapshotManager_MultipleSnapshots_And_DumpAnalysis) {
    SnapshotManager mgr;
    GameState gs;
    gs.NewGame(GameMode::AI_VS_AI, true);

    std::vector<std::string> ids;
    for (int i = 0; i < 3; ++i) {
        auto id = mgr.CreateSnapshot(gs, "run", nullptr);
        ASSERT_FALSE(id.empty());
        ids.push_back(id);
    }

    SnapshotGameRunner runner;
    runner.SetVerbose(false);
    std::string fname = "analysis_multi.txt";
    runner.DumpAnalysis(fname);
    ASSERT_TRUE(std::filesystem::exists(fname));

    // cleanup
    for (auto &id : ids) mgr.DeleteSnapshot(id);
    try { std::filesystem::remove(fname); } catch(...) {}
    try { std::filesystem::remove_all("snapshots"); } catch(...) {}
}
