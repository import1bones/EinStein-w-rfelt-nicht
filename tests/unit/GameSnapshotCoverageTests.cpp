#include "TestFramework.h"
#include "utils/GameSnapshot.h"
#include "game/GameState.h"
#include <filesystem>

using namespace Einstein;

TEST(GameSnapshot_SaveLoad_FileRoundTrip)
{
    // Create a temp snapshot and save to file, then load it back
    GameState gs;
    gs.NewGame(GameMode::AI_VS_AI, true);
    GameSnapshot snap;
    snap.UpdateFromGameState(gs);
    snap.snapshot_id = "test_snapshot";
    snap.turn_number = 2;

    std::string fname = "temp_test.snapshot";
    ASSERT_TRUE(snap.SaveToFile(fname));

    GameSnapshot loaded;
    ASSERT_TRUE(loaded.LoadFromFile(fname));
    ASSERT_EQ(loaded.turn_number, snap.turn_number);

    // cleanup
    std::filesystem::remove(fname);
}

TEST(AIThinkingSnapshot_Serialization_WithMCTS)
{
    AIThinkingSnapshot ats;
    ats.mcts_iterations = 42;
    ats.thinking_time = 0.01;
    ats.position_evaluation = 3.14;

    // Attach a small MCTSSnapshot
    MCTSSnapshot ms;
    ms.total_iterations = 5;
    MCTSNodeSnapshot r;
    r.last_move = {{-1,-1},{-1,-1}};
    r.visits = 10;
    ms.root_node = r;
    ats.mcts_state = ms;
    ats.has_mcts_data = true;

    std::string s = ats.ToJson();
    ASSERT_FALSE(s.empty());

    AIThinkingSnapshot parsed;
    parsed.FromString(s);
    ASSERT_EQ(parsed.mcts_iterations, ats.mcts_iterations);
    ASSERT_TRUE(parsed.has_mcts_data == ats.has_mcts_data);
    ASSERT_EQ(parsed.mcts_state.root_node.visits, ms.root_node.visits);
}

TEST(SnapshotManager_CreateListDelete)
{
    // Use default SnapshotManager which writes into ./snapshots
    SnapshotManager mgr;
    GameState gs;
    gs.NewGame(GameMode::AI_VS_AI, true);

    std::string id = mgr.CreateSnapshot(gs, "unit_test", nullptr);
    ASSERT_FALSE(id.empty());

    auto list = mgr.ListSnapshots();
    ASSERT_TRUE(std::find(list.begin(), list.end(), id) != list.end());

    ASSERT_TRUE(mgr.HasSnapshot(id));
    auto snap = mgr.GetSnapshot(id);
    ASSERT_FALSE(snap.snapshot_id.empty());

    ASSERT_TRUE(mgr.DeleteSnapshot(id));
    // cleanup directory if empty
    try { std::filesystem::remove_all("snapshots"); } catch (...) {}
}

TEST(MCTSSnapshot_DeepTree_RoundTrip)
{
    MCTSSnapshot ms;
    ms.total_iterations = 999;
    // Build depth 2 tree
    MCTSNodeSnapshot root;
    root.last_move = {{-1,-1},{-1,-1}};
    root.visits = 1;
    for (int i = 0; i < 3; ++i) {
        MCTSNodeSnapshot c;
        c.last_move = {{i,i},{i+1,i+1}};
        c.visits = i+2;
        for (int j = 0; j < 2; ++j) {
            MCTSNodeSnapshot gc;
            gc.last_move = {{j,j},{j,j}};
            gc.visits = j+1;
            c.children.push_back(gc);
        }
        root.children.push_back(c);
    }
    ms.root_node = root;

    std::string s = ms.ToJson();
    ASSERT_FALSE(s.empty());

    MCTSSnapshot parsed;
    parsed.FromString(s);
    ASSERT_EQ(parsed.total_iterations, ms.total_iterations);
    ASSERT_EQ(parsed.root_node.children.size(), ms.root_node.children.size());
    for (size_t i = 0; i < parsed.root_node.children.size(); ++i) {
        ASSERT_EQ(parsed.root_node.children[i].children.size(), ms.root_node.children[i].children.size());
    }
}
