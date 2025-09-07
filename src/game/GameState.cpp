#include "game/GameState.h"
#include "ai/MCTS.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>

namespace Einstein {

GameState::GameState() 
    : current_player_(Player::LEFT_TOP), current_dice_(1), game_mode_(GameMode::HUMAN_VS_AI),
      game_result_(GameResult::ONGOING), current_move_index_(0) {
    board_.Initialize();
    statistics_.game_start_time = std::chrono::system_clock::now();
}

void GameState::NewGame(GameMode mode, bool lt_starts) {
    board_.Initialize();
    current_player_ = lt_starts ? Player::LEFT_TOP : Player::RIGHT_BOTTOM;
    current_dice_ = RollDice();
    game_mode_ = mode;
    game_result_ = GameResult::ONGOING;
    
    move_history_.clear();
    current_move_index_ = 0;
    
    statistics_ = GameStatistics();
}

void GameState::ResetGame() {
    NewGame(game_mode_, true);
}

bool GameState::IsGameOver() const {
    return game_result_ != GameResult::ONGOING;
}

GameResult GameState::GetGameResult() const {
    return game_result_;
}

bool GameState::MakeMove(const Move& move) {
    if (!ValidateMove(move)) {
        return false;
    }
    
    // Create game move with metadata
    auto start_time = std::chrono::high_resolution_clock::now();
    GameMove game_move(move, current_player_, current_dice_);
    
    // Execute the move on the board
    if (!board_.ExecuteMove(move)) {
        return false;
    }
    
    // Calculate thinking time (simplified for now)
    auto end_time = std::chrono::high_resolution_clock::now();
    game_move.thinking_time = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    
    // Truncate history if we're in the middle and making a new move
    TruncateHistoryFromCurrentPosition();
    
    // Add to history
    move_history_.push_back(game_move);
    current_move_index_++;
    
    // Update statistics
    UpdateStatistics(game_move);
    
    // Notify callback
    if (move_callback_) {
        move_callback_(game_move);
    }
    
    // Switch player and roll new dice
    SwitchPlayer();
    current_dice_ = RollDice();
    
    // Check for game end
    CheckGameEnd();
    
    return true;
}

bool GameState::CanUndo() const {
    return current_move_index_ > 0;
}

bool GameState::UndoMove() {
    if (!CanUndo()) {
        return false;
    }
    
    current_move_index_--;
    const GameMove& last_move = move_history_[current_move_index_];
    
    // Undo the move on the board (simplified - would need captured piece info)
    board_.UndoMove(last_move.move, 0); // Simplified undo
    
    // Switch back to previous player
    current_player_ = last_move.player;
    current_dice_ = last_move.dice_value;
    
    // Reset game result if it was over
    if (game_result_ != GameResult::ONGOING) {
        game_result_ = GameResult::ONGOING;
    }
    
    return true;
}

bool GameState::CanRedo() const {
    return current_move_index_ < move_history_.size();
}

bool GameState::RedoMove() {
    if (!CanRedo()) {
        return false;
    }
    
    const GameMove& redo_move = move_history_[current_move_index_];
    current_move_index_++;
    
    // Re-execute the move
    board_.ExecuteMove(redo_move.move);
    
    // Restore state
    current_player_ = redo_move.player;
    current_dice_ = redo_move.dice_value;
    SwitchPlayer();
    
    CheckGameEnd();
    
    return true;
}

const GameMove* GameState::GetLastMove() const {
    if (move_history_.empty() || current_move_index_ == 0) {
        return nullptr;
    }
    return &move_history_[current_move_index_ - 1];
}

std::vector<Move> GameState::GetValidMoves() const {
    return board_.GetValidMoves(current_player_, current_dice_);
}

Move GameState::GetAIMove() {
    if (!ai_ || !IsAITurn()) {
        return {{-1, -1}, {-1, -1}}; // Invalid move
    }
    
    // Use MCTS AI to select the best move
    auto valid_moves = GetValidMoves();
    if (valid_moves.empty()) {
        return {{-1, -1}, {-1, -1}};
    }
    
    // If only one move available, return it immediately
    if (valid_moves.size() == 1) {
        return valid_moves[0];
    }
    
    // Use MCTS to find the best move
    auto best_move = ai_->FindBestMove(board_, current_player_, current_dice_);
    
    // Validate the AI's move is actually valid
    for (const auto& move : valid_moves) {
        if (move.first == best_move.first && move.second == best_move.second) {
            return move;
        }
    }
    
    // Fallback: return first valid move if AI move is invalid
    return valid_moves[0];
}

bool GameState::IsAITurn() const {
    if (game_mode_ == GameMode::HUMAN_VS_HUMAN) {
        return false;
    } else if (game_mode_ == GameMode::AI_VS_AI) {
        return true;
    } else if (game_mode_ == GameMode::HUMAN_VS_AI) {
        return current_player_ == Player::RIGHT_BOTTOM; // AI plays as RB
    }
    return false;
}

double GameState::EvaluateCurrentPosition() const {
    // Simple evaluation based on piece count and positions
    auto lt_pieces = board_.GetPlayerPieces(Player::LEFT_TOP);
    auto rb_pieces = board_.GetPlayerPieces(Player::RIGHT_BOTTOM);
    
    double score = static_cast<double>(lt_pieces.size()) - static_cast<double>(rb_pieces.size());
    
    // Add positional evaluation (pieces closer to target get bonus)
    for (const auto& pos : lt_pieces) {
        score += (pos.first + pos.second) * 0.1; // Closer to bottom-right
    }
    
    for (const auto& pos : rb_pieces) {
        score -= (4 - pos.first + 4 - pos.second) * 0.1; // Closer to top-left
    }
    
    return score;
}

std::vector<std::pair<Move, double>> GameState::AnalyzeMoves() const {
    std::vector<std::pair<Move, double>> analyzed_moves;
    auto valid_moves = GetValidMoves();
    
    for (const auto& move : valid_moves) {
        // Create a copy to test the move
        ChessBoard test_board = board_;
        test_board.ExecuteMove(move);
        
        // Simple evaluation (could be improved with deeper analysis)
        double score = EvaluateCurrentPosition();
        analyzed_moves.emplace_back(move, score);
    }
    
    // Sort by score (best first)
    std::sort(analyzed_moves.begin(), analyzed_moves.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    return analyzed_moves;
}

bool GameState::SaveGame(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Save game state in a simple format
    file << "Einstein Game Save\n";
    file << "Mode: " << static_cast<int>(game_mode_) << "\n";
    file << "Current Player: " << static_cast<int>(current_player_) << "\n";
    file << "Current Dice: " << current_dice_ << "\n";
    file << "Game Result: " << static_cast<int>(game_result_) << "\n";
    file << "Move Count: " << current_move_index_ << "\n";
    
    // Save move history
    for (size_t i = 0; i < current_move_index_; ++i) {
        const auto& game_move = move_history_[i];
        file << "Move: " << game_move.move.first.first << " " << game_move.move.first.second
             << " " << game_move.move.second.first << " " << game_move.move.second.second
             << " " << static_cast<int>(game_move.player) << " " << game_move.dice_value << "\n";
    }
    
    return true;
}

bool GameState::LoadGame(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    if (!std::getline(file, line) || line != "Einstein Game Save") {
        return false;
    }
    
    // Load game data from simple format
    while (std::getline(file, line)) {
        if (line.find("current_player:") != std::string::npos) {
            int player_val = std::stoi(line.substr(line.find(":") + 1));
            current_player_ = static_cast<Player>(player_val);
        } else if (line.find("game_mode:") != std::string::npos) {
            int mode_val = std::stoi(line.substr(line.find(":") + 1));
            game_mode_ = static_cast<GameMode>(mode_val);
        } else if (line.find("board:") != std::string::npos) {
            // Board loading would be implemented here - simplified for now
            board_.Initialize();
        }
    }
    
    return true;
}

std::string GameState::ExportPGN() const {
    std::stringstream pgn;
    
    pgn << "[Event \"Einstein Game\"]\n";
    pgn << "[Date \"" << std::chrono::system_clock::now().time_since_epoch().count() << "\"]\n";
    pgn << "[White \"Player LT\"]\n";
    pgn << "[Black \"Player RB\"]\n";
    pgn << "[Result \"";
    
    switch (game_result_) {
        case GameResult::LT_WINS: pgn << "1-0"; break;
        case GameResult::RB_WINS: pgn << "0-1"; break;
        case GameResult::DRAW: pgn << "1/2-1/2"; break;
        default: pgn << "*"; break;
    }
    
    pgn << "\"]\n\n";
    
    // Export moves (simplified notation)
    for (size_t i = 0; i < current_move_index_; ++i) {
        const auto& move = move_history_[i];
        if (i % 2 == 0) {
            pgn << (i / 2 + 1) << ". ";
        }
        
        pgn << "(" << move.move.first.first << "," << move.move.first.second << ")->"
            << "(" << move.move.second.first << "," << move.move.second.second << ") ";
        
        if (i % 2 == 1) {
            pgn << "\n";
        }
    }
    
    return pgn.str();
}

// Private helper methods

void GameState::SwitchPlayer() {
    current_player_ = (current_player_ == Player::LEFT_TOP) ? 
                      Player::RIGHT_BOTTOM : Player::LEFT_TOP;
}

int GameState::RollDice() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1, 6);
    return dis(gen);
}

void GameState::CheckGameEnd() {
    if (board_.HasPlayerWon(Player::LEFT_TOP)) {
        game_result_ = GameResult::LT_WINS;
        statistics_.game_end_time = std::chrono::system_clock::now();
        if (game_end_callback_) {
            game_end_callback_(game_result_);
        }
    } else if (board_.HasPlayerWon(Player::RIGHT_BOTTOM)) {
        game_result_ = GameResult::RB_WINS;
        statistics_.game_end_time = std::chrono::system_clock::now();
        if (game_end_callback_) {
            game_end_callback_(game_result_);
        }
    } else if (move_history_.size() > 300) {
        game_result_ = GameResult::DRAW;
        statistics_.game_end_time = std::chrono::system_clock::now();
        if (game_end_callback_) {
            game_end_callback_(game_result_);
        }
    }
}

void GameState::UpdateStatistics(const GameMove& move) {
    statistics_.total_moves++;
    statistics_.total_thinking_time += move.thinking_time;
    statistics_.total_mcts_iterations += move.mcts_iterations;
}

bool GameState::ValidateMove(const Move& move) const {
    if (game_result_ != GameResult::ONGOING) {
        return false;
    }
    
    return board_.IsValidMove(move, current_player_);
}

void GameState::TruncateHistoryFromCurrentPosition() {
    if (current_move_index_ < move_history_.size()) {
        move_history_.erase(move_history_.begin() + current_move_index_, move_history_.end());
    }
}

} // namespace Einstein
