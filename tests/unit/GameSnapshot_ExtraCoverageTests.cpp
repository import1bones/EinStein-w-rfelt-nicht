#include "TestFramework.h"
#include "utils/GameSnapshot.h"
#include "game/GameState.h"
#include <filesystem>

using namespace Einstein;

TEST(MCTSSnapshot_PrincipalVariation_And_Debug_ToJson) {
    MCTSSnapshot ms;
    // principal variation with two moves
    ms.principal_variation.push_back({{0,0},{1,1}});
    ms.principal_variation.push_back({{1,1},{2,2}});
    ms.search_debug_info.push_back("d1");
    ms.search_debug_info.push_back("d2");

    // root node with two children to hit children serialization
    MCTSNodeSnapshot root;
    root.last_move = {{-1,-1},{-1,-1}};
    root.visits = 200;
    MCTSNodeSnapshot c1;
    c1.last_move = {{0,0},{1,1}};
    c1.visits = 50;
    MCTSNodeSnapshot c2;
    c2.last_move = {{1,1},{2,2}};
    c2.visits = 30;
    root.children.push_back(c1);
    root.children.push_back(c2);
    ms.root_node = root;

    std::string json = ms.ToJson();
    ASSERT_FALSE(json.empty());
    ASSERT_TRUE(json.find("principal_variation") != std::string::npos);
    ASSERT_TRUE(json.find("root_node") != std::string::npos);
}

TEST(AIThinkingSnapshot_ToString_WithMCTSTree) {
    AIThinkingSnapshot ats;
    ats.mcts_iterations = 3;
    ats.thinking_time = 0.01;
    ats.position_evaluation = 0.5;
    ats.best_move_reasoning = "best";

    // Build mcts tree with grandchildren to hit second-level output
    MCTSSnapshot ms;
    MCTSNodeSnapshot root;
    root.visits = 10;
    MCTSNodeSnapshot child;
    child.last_move = {{0,0},{1,1}};
    child.visits = 5;
    MCTSNodeSnapshot grand;
    grand.last_move = {{1,1},{2,2}};
    grand.visits = 2;
    child.children.push_back(grand);
    root.children.push_back(child);
    ms.root_node = root;

    ats.has_mcts_data = true;
    ats.mcts_state = ms;

    std::string s = ats.ToString();
    ASSERT_TRUE(s.find("MCTS_TREE_ENABLED:1") != std::string::npos);
    ASSERT_TRUE(s.find("MCTS_C1_0:") != std::string::npos || s.find("MCTS_C2_0_0:") != std::string::npos);
}

TEST(GameSnapshot_ToString_FromString_WithMoves) {
    GameState gs;
    gs.NewGame(GameMode::AI_VS_AI, true);
    auto board = gs.GetBoard();
    board.SetPiece(2,2, 2);
    gs.SetBoard(board);

    GameSnapshot snap;
    snap.UpdateFromGameState(gs);
    snap.turn_number = 5;
    snap.current_phase = "analysis";
    snap.current_valid_moves.push_back({{0,0},{1,1}});
    snap.suggested_move = {{0,0},{1,1}};
    snap.debug_log.push_back("x");

    std::string out = snap.ToString();
    ASSERT_FALSE(out.empty());

    GameSnapshot loaded;
    loaded.FromString(out);
    ASSERT_EQ(loaded.turn_number, snap.turn_number);
    ASSERT_EQ(loaded.current_phase, snap.current_phase);
    ASSERT_FALSE(loaded.current_valid_moves.empty());
}

TEST(SnapshotManager_SaveLoad_List_Delete) {
    SnapshotManager mgr;
    GameState gs;
    gs.NewGame(GameMode::AI_VS_AI, true);

    auto id = mgr.CreateSnapshot(gs, "t1", nullptr);
    ASSERT_FALSE(id.empty());
    ASSERT_TRUE(mgr.HasSnapshot(id));

    auto list = mgr.ListSnapshots();
    ASSERT_TRUE(!list.empty());

    GameSnapshot s = mgr.GetSnapshot(id);
    ASSERT_FALSE(s.snapshot_id.empty());

    bool del = mgr.DeleteSnapshot(id);
    ASSERT_TRUE(del);
    // cleanup snapshots directory if empty
    try { std::filesystem::remove_all("snapshots"); } catch(...) {}
}
