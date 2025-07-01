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

void TestChessBoardPlayerPieces() {
    Einstein::ChessBoard board;
    
    auto lt_pieces = board.GetPlayerPieces(Einstein::Player::LEFT_TOP);
    auto rb_pieces = board.GetPlayerPieces(Einstein::Player::RIGHT_BOTTOM);
    
    V_ASSERT(lt_pieces.size() == 6, "LT player should have 6 pieces initially");
    V_ASSERT(rb_pieces.size() == 6, "RB player should have 6 pieces initially");
    
    // Test piece finding
    auto piece1_pos = board.FindPiece(1, Einstein::Player::LEFT_TOP);
    V_ASSERT(piece1_pos.has_value(), "Should find LT piece 1");
    V_ASSERT(piece1_pos->first == 0, "LT piece 1 should be at x=0");
    V_ASSERT(piece1_pos->second == 0, "LT piece 1 should be at y=0");
    
    auto piece1_rb_pos = board.FindPiece(1, Einstein::Player::RIGHT_BOTTOM);
    V_ASSERT(piece1_rb_pos.has_value(), "Should find RB piece 1");
    V_ASSERT(piece1_rb_pos->first == 4, "RB piece 1 should be at x=4");
    V_ASSERT(piece1_rb_pos->second == 4, "RB piece 1 should be at y=4");
}

void TestChessBoardMoveValidation() {
    Einstein::ChessBoard board;
    
    // Test valid move
    Einstein::Move valid_move = {{0, 0}, {1, 0}};
    // This should be invalid because position (1,0) is already occupied
    V_ASSERT(!board.IsValidMove(valid_move, Einstein::Player::LEFT_TOP), 
              "Move to occupied position should be invalid");
    
    // Test move to empty adjacent position
    Einstein::Move valid_empty_move = {{0, 0}, {0, 2}};
    V_ASSERT(!board.IsValidMove(valid_empty_move, Einstein::Player::LEFT_TOP), 
              "Move more than one step should be invalid");
    
    // Test adjacent empty move
    Einstein::Move adjacent_move = {{2, 1}, {2, 2}};
    V_ASSERT(board.IsValidMove(adjacent_move, Einstein::Player::LEFT_TOP), 
              "Move to adjacent empty position should be valid");
    
    // Test invalid player move
    V_ASSERT(!board.IsValidMove(adjacent_move, Einstein::Player::RIGHT_BOTTOM), 
              "Moving opponent's piece should be invalid");
    
    // Test out of bounds move
    Einstein::Move oob_move = {{0, 0}, {-1, 0}};
    V_ASSERT(!board.IsValidMove(oob_move, Einstein::Player::LEFT_TOP), 
              "Move out of bounds should be invalid");
}

void TestChessBoardMoveExecution() {
    Einstein::ChessBoard board;
    
    // Test basic move execution
    Einstein::Move move = {{2, 1}, {2, 2}};
    V_ASSERT(board.IsValidMove(move, Einstein::Player::LEFT_TOP), "Move should be valid");
    
    int8_t piece_before = board.GetPiece(2, 1);
    V_ASSERT(board.ExecuteMove(move), "Move execution should succeed");
    
    V_ASSERT(board.GetPiece(2, 1) == 0, "Source position should be empty after move");
    V_ASSERT(board.GetPiece(2, 2) == piece_before, "Piece should be at destination");
    
    // Test capture move
    Einstein::Move capture_move = {{2, 2}, {2, 3}};
    int8_t captured_piece = board.GetPiece(2, 3);
    V_ASSERT(captured_piece != 0, "Target position should have a piece to capture");
    
    V_ASSERT(board.ExecuteMove(capture_move), "Capture move should succeed");
    V_ASSERT(board.GetPiece(2, 3) == piece_before, "Capturing piece should be at destination");
    
    // Test undo move
    board.UndoMove(capture_move, captured_piece);
    V_ASSERT(board.GetPiece(2, 2) == piece_before, "Piece should be back at original position");
    V_ASSERT(board.GetPiece(2, 3) == captured_piece, "Captured piece should be restored");
}

void TestChessBoardWinConditions() {
    Einstein::ChessBoard board;
    
    // Initially, no player should have won
    V_ASSERT(!board.HasPlayerWon(Einstein::Player::LEFT_TOP), "LT should not have won initially");
    V_ASSERT(!board.HasPlayerWon(Einstein::Player::RIGHT_BOTTOM), "RB should not have won initially");
    
    // Test win condition by reaching target zone
    board.Clear();
    board.SetPiece(4, 4, -1); // LT piece at RB corner
    V_ASSERT(board.HasPlayerWon(Einstein::Player::LEFT_TOP), "LT should win by reaching target");
    
    board.Clear();
    board.SetPiece(0, 0, 1); // RB piece at LT corner
    V_ASSERT(board.HasPlayerWon(Einstein::Player::RIGHT_BOTTOM), "RB should win by reaching target");
}

void TestChessBoardDiceBasedMoves() {
    Einstein::ChessBoard board;
    
    // Test getting valid moves with dice
    auto moves_dice_1 = board.GetValidMoves(Einstein::Player::LEFT_TOP, 1);
    auto moves_dice_6 = board.GetValidMoves(Einstein::Player::LEFT_TOP, 6);
    
    // Should have some valid moves for both dice values
    V_ASSERT(!moves_dice_1.empty(), "Should have valid moves for dice 1");
    V_ASSERT(!moves_dice_6.empty(), "Should have valid moves for dice 6");
    
    // Test CanPlayerMove
    V_ASSERT(board.CanPlayerMove(Einstein::Player::LEFT_TOP, 1), "LT should be able to move with dice 1");
    V_ASSERT(board.CanPlayerMove(Einstein::Player::LEFT_TOP, 6), "LT should be able to move with dice 6");
    V_ASSERT(board.CanPlayerMove(Einstein::Player::RIGHT_BOTTOM, 1), "RB should be able to move with dice 1");
}

void TestChessBoardHashing() {
    Einstein::ChessBoard board1, board2;
    
    // Initially, boards should be equal and have same hash
    V_ASSERT(board1 == board2, "Initial boards should be equal");
    V_ASSERT(board1.GetHash() == board2.GetHash(), "Equal boards should have same hash");
    
    // After a move, boards should be different
    Einstein::Move move = {{2, 1}, {2, 2}};
    board1.ExecuteMove(move);
    
    V_ASSERT(!(board1 == board2), "Boards should be different after move");
    V_ASSERT(board1.GetHash() != board2.GetHash(), "Different boards should have different hashes");
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
    framework.RegisterUnitTest("ChessBoard_PlayerPieces", "ChessBoard", TestChessBoardPlayerPieces);
    framework.RegisterUnitTest("ChessBoard_MoveValidation", "ChessBoard", TestChessBoardMoveValidation);
    framework.RegisterUnitTest("ChessBoard_MoveExecution", "ChessBoard", TestChessBoardMoveExecution);
    framework.RegisterUnitTest("ChessBoard_WinConditions", "ChessBoard", TestChessBoardWinConditions);
    framework.RegisterUnitTest("ChessBoard_DiceBasedMoves", "ChessBoard", TestChessBoardDiceBasedMoves);
    framework.RegisterUnitTest("ChessBoard_Hashing", "ChessBoard", TestChessBoardHashing);
    
    // Performance Tests
    framework.RegisterUnitTest("ChessBoard_Performance", "Performance", TestChessBoardPerformance);
    
    // Set performance benchmarks
    framework.SetPerformanceBenchmark("ChessBoard_Performance", 100.0); // 100ms max
}

} // namespace UnitTests
} // namespace VModel
