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
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file for writing: " + filename);
        }
        
        // Save game state in a simple format
        file << "# Einstein Game Save File\n";
        file << "version=2.0\n";
        file << "turn=" << static_cast<int>(game_state_.GetCurrentPlayer()) << "\n";
        file << "moves=" << game_state_.GetMoveCount() << "\n";
        
        // Save board state
        file << "board=";
        const auto& board = game_state_.GetBoard();
        const auto& board_array = board.GetBoard();
        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 5; ++col) {
                file << static_cast<int>(board_array[row][col]);
                if (row < 4 || col < 4) file << ",";
            }
        }
        file << "\n";
        
        // Save move history
        const auto& history = game_state_.GetMoveHistory();
        file << "history_count=" << history.size() << "\n";
        for (size_t i = 0; i < history.size(); ++i) {
            const auto& move = history[i];
            file << "move" << i << "=" 
                 << move.first.first << "," << move.first.second << ","
                 << move.second.first << "," << move.second.second << "\n";
        }
        
        std::cout << "Game saved to: " << filename << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Failed to save game: " << e.what() << std::endl;
    }
}

void Game::LoadGame(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open save file: " << filename << std::endl;
            return;
        }
        
        std::string line;
        ChessBoard board;
        std::vector<Move> move_history;
        Player current_player = Player::LEFT_TOP;
        
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            size_t eq_pos = line.find('=');
            if (eq_pos == std::string::npos) continue;
            
            std::string key = line.substr(0, eq_pos);
            std::string value = line.substr(eq_pos + 1);
            
            if (key == "turn") {
                current_player = static_cast<Player>(std::stoi(value));
            } else if (key == "board") {
                // Parse board state
                board.Clear();
                std::stringstream ss(value);
                std::string piece_str;
                int row = 0, col = 0;
                
                while (std::getline(ss, piece_str, ',') && row < 5) {
                    int piece_value = std::stoi(piece_str);
                    board.SetPiece(row, col, static_cast<int8_t>(piece_value));
                    
                    col++;
                    if (col >= 5) {
                        col = 0;
                        row++;
                    }
                }
            } else if (key.substr(0, 4) == "move") {
                // Parse move history
                std::stringstream ss(value);
                std::string coord;
                std::vector<int> coords;
                
                while (std::getline(ss, coord, ',')) {
                    coords.push_back(std::stoi(coord));
                }
                
                if (coords.size() == 4) {
                    Move move = {{coords[0], coords[1]}, {coords[2], coords[3]}};
                    move_history.push_back(move);
                }
            }
        }
        
        // Apply loaded state
        game_state_.SetBoard(board);
        game_state_.SetCurrentPlayer(current_player);
        
        // Replay move history to restore game state
        for (const auto& move : move_history) {
            game_state_.MakeMove(move);
        }
        
        std::cout << "Game loaded from: " << filename << "\n";
        std::cout << "Moves loaded: " << move_history.size() << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to load game: " << e.what() << std::endl;
    }
}

int Game::GetCurrentDiceValue() const {
    return dice_value_;
}

} // namespace Einstein
