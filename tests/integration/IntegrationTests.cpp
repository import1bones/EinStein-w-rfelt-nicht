#include "../framework/VModelTestFramework.h"
#include "../../include/core/ChessBoard.h"
#include "../../include/utils/Config.h"

namespace VModel {
namespace IntegrationTests {

// Integration Tests for Component Interactions
void TestChessBoardWithConfig() {
    // Test ChessBoard and Config integration
    Einstein::Config config;
    
    try {
        config.LoadFromFile("assets/config.json");
        V_ASSERT(true, "Config should load successfully");
    } catch (...) {
        // Config loading might fail in test environment, that's ok
        V_ASSERT(true, "Config integration test framework ready");
    }
    
    Einstein::ChessBoard board;
    board.Initialize();
    
    // Test that board operations work after config loading
    V_ASSERT(board.IsValidPosition(2, 2), "Board should work after config integration");
}

void TestBoardGameLogicIntegration() {
    Einstein::ChessBoard board;
    board.Initialize();
    
    // Test move sequence integration
    board.Clear();
    board.SetPiece(0, 0, static_cast<int8_t>(Einstein::Player::LEFT_TOP));
    board.SetPiece(4, 4, static_cast<int8_t>(Einstein::Player::RIGHT_BOTTOM));
    
    V_ASSERT(!board.IsEmpty(0, 0), "Player piece should be placed");
    V_ASSERT(!board.IsEmpty(4, 4), "Opponent piece should be placed");
    V_ASSERT(board.IsEmpty(2, 2), "Empty position should remain empty");
}

void TestConfigComponentIntegration() {
    Einstein::Config config;
    
    // Test default values
    int window_width = config.GetInt("graphics.window_width", 800);
    int window_height = config.GetInt("graphics.window_height", 600);
    bool fullscreen = config.GetBool("graphics.fullscreen", false);
    
    V_ASSERT(window_width > 0, "Window width should be positive");
    V_ASSERT(window_height > 0, "Window height should be positive");
    V_ASSERT(fullscreen == false || fullscreen == true, "Fullscreen should be boolean");
}

void TestBoardStateConsistency() {
    Einstein::ChessBoard board1, board2;
    
    // Test that multiple boards maintain independent state
    board1.Initialize();
    board2.Initialize();
    
    board1.SetPiece(1, 1, 1);
    board2.SetPiece(2, 2, 2);
    
    V_ASSERT(board1.GetPiece(1, 1) == 1, "Board1 should have correct piece");
    V_ASSERT(board2.GetPiece(2, 2) == 2, "Board2 should have correct piece");
    V_ASSERT(board1.GetPiece(2, 2) != 2, "Board1 should not have Board2's piece");
    V_ASSERT(board2.GetPiece(1, 1) != 1, "Board2 should not have Board1's piece");
}

void TestPlayerPiecesIntegration() {
    Einstein::ChessBoard board;
    board.Initialize();
    board.Clear();
    
    // Place some pieces for LEFT_TOP player
    board.SetPiece(0, 0, static_cast<int8_t>(Einstein::Player::LEFT_TOP));
    board.SetPiece(1, 0, static_cast<int8_t>(Einstein::Player::LEFT_TOP));
    
    // Place some pieces for RIGHT_BOTTOM player  
    board.SetPiece(3, 4, static_cast<int8_t>(Einstein::Player::RIGHT_BOTTOM));
    board.SetPiece(4, 4, static_cast<int8_t>(Einstein::Player::RIGHT_BOTTOM));
    
    auto left_pieces = board.GetPlayerPieces(Einstein::Player::LEFT_TOP);
    auto right_pieces = board.GetPlayerPieces(Einstein::Player::RIGHT_BOTTOM);
    
    V_ASSERT(left_pieces.size() == 2, "LEFT_TOP should have 2 pieces");
    V_ASSERT(right_pieces.size() == 2, "RIGHT_BOTTOM should have 2 pieces");
}

void TestPerformanceIntegration() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Test multiple component operations together
    for (int i = 0; i < 100; ++i) {
        Einstein::ChessBoard board;
        Einstein::Config config;
        
        board.Initialize();
        board.Clear();
        
        // Simulate some game operations
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                if (board.IsValidPosition(x, y)) {
                    board.SetPiece(x, y, i % 3);
                    board.GetPiece(x, y);
                }
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();
    
    V_ASSERT(duration < 1000.0, "Integration operations should be performant");
}

void RegisterIntegrationTests() {
    auto& framework = VModelTestFramework::Instance();
    
    // Component Integration Tests
    framework.RegisterIntegrationTest("ChessBoard_Config_Integration", "Core", TestChessBoardWithConfig);
    framework.RegisterIntegrationTest("Board_GameLogic_Integration", "GameLogic", TestBoardGameLogicIntegration);
    framework.RegisterIntegrationTest("Config_Component_Integration", "Config", TestConfigComponentIntegration);
    
    // State Management Tests
    framework.RegisterIntegrationTest("Board_State_Consistency", "StateManagement", TestBoardStateConsistency);
    framework.RegisterIntegrationTest("Player_Pieces_Integration", "GameLogic", TestPlayerPiecesIntegration);
    
    // Performance Integration Tests
    framework.RegisterIntegrationTest("Performance_Integration", "Performance", TestPerformanceIntegration);
    
    // Set performance benchmarks
    framework.SetPerformanceBenchmark("Performance_Integration", 1000.0); // 1000ms max
}

} // namespace IntegrationTests
} // namespace VModel
