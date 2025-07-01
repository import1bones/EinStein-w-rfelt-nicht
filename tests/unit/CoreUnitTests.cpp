#include "../framework/VModelTestFramework.h"
#include "../../include/core/Types.h"
#include "../../include/core/ChessBoard.h"

namespace VModel {
namespace UnitTests {

// Unit Tests for Core Components
void TestTypesDefinitions() {
    // Test basic type definitions
    V_ASSERT(sizeof(Einstein::Position) == sizeof(std::pair<int, int>), "Position should be defined as pair<int, int>");
    V_ASSERT(sizeof(Einstein::Move) == sizeof(std::pair<Einstein::Position, Einstein::Position>), "Move should be defined as pair of positions");
    V_ASSERT(sizeof(Einstein::Player) == sizeof(int8_t), "Player should be int8_t enum");
    
    // Test type ranges
    Einstein::Position pos = std::make_pair(0, 0);
    V_ASSERT(pos.first == 0 && pos.second == 0, "Position initialization should work");
    
    // Test Player enum values
    V_ASSERT(static_cast<int>(Einstein::Player::LEFT_TOP) != static_cast<int>(Einstein::Player::RIGHT_BOTTOM), 
             "Player enum values should be different");
    V_ASSERT(static_cast<int>(Einstein::Player::NONE) == 0, "NONE player should be 0");
}

void TestChessBoardConstruction() {
    try {
        Einstein::ChessBoard board;
        V_ASSERT(true, "ChessBoard should construct without throwing");
    } catch (...) {
        V_ASSERT(false, "ChessBoard construction should not throw");
    }
}

void TestChessBoardInitialization() {
    Einstein::ChessBoard board;
    board.Initialize();
    
    // Test that board is properly initialized
    // This test will need to be updated when ChessBoard implementation is complete
    V_ASSERT(true, "Board initialization should complete without error");
}

void TestChessBoardBasicOperations() {
    Einstein::ChessBoard board;
    board.Initialize();
    
    // Test valid position checking
    V_ASSERT(board.IsValidPosition(0, 0), "Corner position should be valid");
    V_ASSERT(board.IsValidPosition(4, 4), "Center position should be valid");
    V_ASSERT(!board.IsValidPosition(-1, 0), "Negative position should be invalid");
    V_ASSERT(!board.IsValidPosition(0, 5), "Out-of-bounds position should be invalid");
    
    // Test piece operations
    board.Clear();
    V_ASSERT(board.IsEmpty(2, 2), "Cleared board position should be empty");
}

void TestPositionOperations() {
    Einstein::Position pos1 = std::make_pair(1, 2);
    Einstein::Position pos2 = std::make_pair(1, 2);
    Einstein::Position pos3 = std::make_pair(2, 1);
    
    V_ASSERT(pos1 == pos2, "Equal positions should compare equal");
    V_ASSERT(!(pos1 == pos3), "Different positions should not compare equal");
    V_ASSERT(pos1.first == 1 && pos1.second == 2, "Position components should be accessible");
}

void TestMoveOperations() {
    Einstein::Position from = std::make_pair(0, 0);
    Einstein::Position to = std::make_pair(1, 1);
    Einstein::Move move = std::make_pair(from, to);
    
    V_ASSERT(move.first == from, "Move from position should be set correctly");
    V_ASSERT(move.second == to, "Move to position should be set correctly");
    
    // Test move components
    V_ASSERT(move.first.first == 0 && move.first.second == 0, "Move from components should be correct");
    V_ASSERT(move.second.first == 1 && move.second.second == 1, "Move to components should be correct");
}

// Performance unit tests
void TestChessBoardPerformance() {
    auto start = std::chrono::high_resolution_clock::now();
    
    Einstein::ChessBoard board;
    for (int i = 0; i < 1000; ++i) {
        board.Initialize();
        board.Clear();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();
    
    V_ASSERT(duration < 100.0, "Board operations should be fast");
}

void RegisterUnitTests() {
    auto& framework = VModelTestFramework::Instance();
    
    // Core Types Tests
    framework.RegisterUnitTest("Types_Definitions", "Core", TestTypesDefinitions);
    framework.RegisterUnitTest("Position_Operations", "Core", TestPositionOperations);
    framework.RegisterUnitTest("Move_Operations", "Core", TestMoveOperations);
    
    // ChessBoard Tests
    framework.RegisterUnitTest("ChessBoard_Construction", "ChessBoard", TestChessBoardConstruction);
    framework.RegisterUnitTest("ChessBoard_Initialization", "ChessBoard", TestChessBoardInitialization);
    framework.RegisterUnitTest("ChessBoard_BasicOperations", "ChessBoard", TestChessBoardBasicOperations);
    
    // Performance Tests
    framework.RegisterUnitTest("ChessBoard_Performance", "Performance", TestChessBoardPerformance);
    
    // Set performance benchmarks
    framework.SetPerformanceBenchmark("ChessBoard_Performance", 100.0); // 100ms max
}

} // namespace UnitTests
} // namespace VModel
