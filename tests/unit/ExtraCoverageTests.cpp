// ExtraCoverageTests.cpp
#include "TestFramework.h"
#include "utils/GameSnapshot.h"

using namespace Einstein;

// Tests to exercise edge branches not covered by earlier tests
TEST(GameSnapshot_LoadNonexistentFile) {
    GameSnapshot snap;
    bool ok = snap.LoadFromFile("/nonexistent/path/this_file_does_not_exist.snapshot");
    ASSERT_EQ(0, ok);
}

TEST(GameSnapshot_ParseValidMoves) {
    GameSnapshot snap;
    std::string j = R"({"valid_moves": [[1,2,3,4]]})";
    snap.FromString(j);
    ASSERT_EQ(1, (int)snap.current_valid_moves.size());
    ASSERT_EQ(1, snap.current_valid_moves[0].first.first);
}

TEST(AIThinkingSnapshot_ParseMoveEvaluationsAndDebug) {
    AIThinkingSnapshot a;
    std::string j = R"({"move_evaluations": [[1,2,3,4,0.55]], "debug_info": ["d1","d2"]})";
    a.FromString(j);
    ASSERT_EQ(1, (int)a.move_evaluations.size());
    ASSERT_EQ(0.55, a.move_evaluations[0].second);
    ASSERT_EQ(2, (int)a.debug_info.size());
}

TEST(MCTSSnapshot_FromMalformedJsonDoesNotThrow) {
    MCTSSnapshot s;
    // malformed JSON
    std::string bad = "{ this is : not json ";
    // should not throw
    s.FromString(bad);
    // defaults should remain (total_iterations default 0)
    ASSERT_EQ(0, s.total_iterations);
}
