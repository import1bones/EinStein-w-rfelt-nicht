#pragma once

#include "core/Types.h"
#include "core/ChessBoard.h"
#include <vector>
#include <memory>
#include <string>
#include <chrono>

namespace Einstein {

// Forward declarations
class MCTS;
class Renderer;

struct GameMove {
    Move move;
    Player player;
    int dice_value;
    std::chrono::system_clock::time_point timestamp;
    double thinking_time;
    int mcts_iterations;
    
    GameMove(const Move& m, Player p, int dice, double time = 0.0, int iterations = 0)
        : move(m), player(p), dice_value(dice), timestamp(std::chrono::system_clock::now())
        , thinking_time(time), mcts_iterations(iterations) {}
};

struct GameStatistics {
    int total_moves = 0;
    double total_thinking_time = 0.0;
    int total_mcts_iterations = 0;
    std::chrono::system_clock::time_point game_start_time;
    std::chrono::system_clock::time_point game_end_time;
    
    GameStatistics() : game_start_time(std::chrono::system_clock::now()) {}
    
    double GetAverageThinkingTime() const {
        return total_moves > 0 ? total_thinking_time / total_moves : 0.0;
    }
    
    double GetGameDurationSeconds() const {
        auto end = (game_end_time.time_since_epoch().count() == 0) ? 
                   std::chrono::system_clock::now() : game_end_time;
        return std::chrono::duration<double>(end - game_start_time).count();
    }
};

class GameState {
public:
    GameState();
    ~GameState() = default;
    
    // Game lifecycle
    void NewGame(GameMode mode = GameMode::HUMAN_VS_AI, bool lt_starts = true);
    void ResetGame();
    bool IsGameOver() const;
    GameResult GetGameResult() const;
    Player GetCurrentPlayer() const { return current_player_; }
    
    // Game state access
    const ChessBoard& GetBoard() const { return board_; }
    ChessBoard& GetMutableBoard() { return board_; }
    int GetCurrentDice() const { return current_dice_; }
    GameMode GetGameMode() const { return game_mode_; }
    
    // Move management
    bool MakeMove(const Move& move);
    bool CanUndo() const;
    bool UndoMove();
    bool CanRedo() const;
    bool RedoMove();
    
    // Move history and analysis
    const std::vector<GameMove>& GetMoveHistory() const { return move_history_; }
    size_t GetMoveCount() const { return current_move_index_; }
    const GameMove* GetLastMove() const;
    std::vector<Move> GetValidMoves() const;
    
    // AI interaction
    void SetAI(std::shared_ptr<MCTS> ai) { ai_ = ai; }
    Move GetAIMove();
    bool IsAITurn() const;
    
    // Game analysis
    const GameStatistics& GetStatistics() const { return statistics_; }
    double EvaluateCurrentPosition() const;
    std::vector<std::pair<Move, double>> AnalyzeMoves() const;
    
    // Save/Load functionality
    bool SaveGame(const std::string& filename) const;
    bool LoadGame(const std::string& filename);
    std::string ExportPGN() const; // Portable Game Notation style export
    
    // Events and notifications
    void SetMoveCallback(std::function<void(const GameMove&)> callback) { 
        move_callback_ = callback; 
    }
    void SetGameEndCallback(std::function<void(GameResult)> callback) { 
        game_end_callback_ = callback; 
    }
    
private:
    ChessBoard board_;
    Player current_player_;
    int current_dice_;
    GameMode game_mode_;
    GameResult game_result_;
    
    // Move history management
    std::vector<GameMove> move_history_;
    size_t current_move_index_;
    
    // AI
    std::shared_ptr<MCTS> ai_;
    
    // Statistics
    GameStatistics statistics_;
    
    // Callbacks
    std::function<void(const GameMove&)> move_callback_;
    std::function<void(GameResult)> game_end_callback_;
    
    // Helper methods
    void SwitchPlayer();
    int RollDice();
    void CheckGameEnd();
    void UpdateStatistics(const GameMove& move);
    bool ValidateMove(const Move& move) const;
    void TruncateHistoryFromCurrentPosition();
};

} // namespace Einstein
