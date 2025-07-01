#include "game/Game.h"
#include <iostream>

namespace Einstein {

Game::Game() : game_state_(), dice_value_(1) {
}

Game::~Game() = default;

bool Game::Initialize() {
    try {
        // Initialize game state
        game_state_.Reset();
        
        // Initialize configuration
        config_.LoadFromFile("config.json");
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Game initialization failed: " << e.what() << std::endl;
        return false;
    }
}

void Game::Start() {
    if (!Initialize()) {
        std::cerr << "Failed to initialize game" << std::endl;
        return;
    }
    
    std::cout << "Einstein Game Started!" << std::endl;
    
    // Main game loop (simplified)
    while (!game_state_.IsGameOver()) {
        ProcessTurn();
    }
    
    std::cout << "Game Over! Result: " << static_cast<int>(game_state_.GetGameResult()) << std::endl;
}

void Game::ProcessTurn() {
    // Get current player
    Player current_player = game_state_.GetCurrentPlayer();
    
    // Roll dice (simplified)
    dice_value_ = (rand() % 6) + 1;
    
    // Get valid moves
    auto valid_moves = game_state_.GetValidMoves();
    
    if (valid_moves.empty()) {
        // No valid moves, skip turn
        game_state_.ExecuteMove(std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0))); // Dummy move
        return;
    }
    
    // For now, just execute the first valid move (AI integration will come later)
    if (!valid_moves.empty()) {
        game_state_.ExecuteMove(valid_moves[0]);
    }
}

bool Game::ExecuteMove(const Move& move) {
    return game_state_.ExecuteMove(move);
}

bool Game::UndoMove() {
    return game_state_.UndoLastMove();
}

const GameState& Game::GetGameState() const {
    return game_state_;
}

const Config& Game::GetConfig() const {
    return config_;
}

void Game::NewGame() {
    game_state_.Reset();
    dice_value_ = 1;
}

void Game::SaveGame(const std::string& filename) {
    // TODO: Implement game saving
    (void)filename; // Suppress unused parameter warning
}

void Game::LoadGame(const std::string& filename) {
    // TODO: Implement game loading
    (void)filename; // Suppress unused parameter warning
}

int Game::GetCurrentDiceValue() const {
    return dice_value_;
}

} // namespace Einstein
