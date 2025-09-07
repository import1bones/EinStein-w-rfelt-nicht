#pragma once

#include "core/Types.h"
#include "core/ChessBoard.h"
#include "game/GameState.h"
#include <string>
#include <vector>
#include <chrono>
#include <map>
#include <sstream>

namespace Einstein {

// Forward declarations
struct MCTSNode;
class MCTS;

/**
 * MCTS Tree snapshot for debugging and analysis
 */
struct MCTSNodeSnapshot {
    Position position;
    Player player;
    int dice_value;
    Move last_move;
    
    // Statistics
    int visits = 0;
    double wins = 0.0;
    double ucb_value = 0.0;
    bool is_fully_expanded = false;
    bool is_terminal = false;
    
    // Tree structure (simplified)
    std::vector<MCTSNodeSnapshot> children;
    int parent_index = -1; // Index in flattened tree
    
    std::string ToString() const;
    void FromString(const std::string& data);
};

struct MCTSSnapshot {
    // Tree state
    MCTSNodeSnapshot root_node;
    std::vector<MCTSNodeSnapshot> all_nodes; // Flattened tree
    
    // Search parameters
    int total_iterations = 0;
    int completed_iterations = 0;
    double exploration_constant = 1.414;
    double time_limit = 5.0;
    double elapsed_time = 0.0;
    
    // Current search state
    std::vector<int> current_path; // Node indices from root to current
    std::vector<Move> principal_variation; // Best line found
    double best_evaluation = 0.0;
    
    // Performance stats
    int nodes_created = 0;
    int nodes_expanded = 0;
    double avg_simulation_time = 0.0;
    std::vector<std::string> search_debug_info;
    
    std::string ToString() const;
    void FromString(const std::string& data);
    
    // Analysis methods
    std::vector<Move> GetBestMoves(int count = 5) const;
    std::string GetSearchSummary() const;
    void AnalyzeSearchTree(std::vector<std::string>& analysis) const;
};

/**
 * Comprehensive game snapshot system for debugging and development
 * Now includes MCTS state for complete AI debugging
 */

struct AIThinkingSnapshot {
    int mcts_iterations = 0;
    double thinking_time = 0.0;
    std::vector<std::pair<Move, double>> move_evaluations;
    std::string best_move_reasoning;
    double position_evaluation = 0.0;
    int nodes_explored = 0;
    std::vector<std::string> debug_info;
    
    // MCTS state
    MCTSSnapshot mcts_state;
    bool has_mcts_data = false;
    
    std::string ToString() const;
    void FromString(const std::string& data);
};

struct GameSnapshot {
    // Basic game state
    ChessBoard board;
    Player current_player = Player::LEFT_TOP;
    int current_dice = 1;
    GameMode game_mode = GameMode::AI_VS_AI;
    GameResult game_result = GameResult::ONGOING;
    
    // Move history
    std::vector<GameMove> move_history;
    size_t current_move_index = 0;
    
    // AI state
    AIThinkingSnapshot ai_thinking;
    std::vector<Move> current_valid_moves;
    Move suggested_move = {{-1, -1}, {-1, -1}};
    
    // Metadata
    std::chrono::system_clock::time_point timestamp;
    std::string snapshot_id;
    int turn_number = 1;
    double total_game_time = 0.0;
    
    // Debug information
    std::vector<std::string> debug_log;
    std::string current_phase; // "thinking", "moving", "analyzing", etc.
    
    GameSnapshot();
    
    // Serialization
    std::string ToString() const;
    void FromString(const std::string& data);
    
    // File operations
    bool SaveToFile(const std::string& filename) const;
    bool LoadFromFile(const std::string& filename);
    
    // Utility methods
    std::string GetSnapshotSummary() const;
    void AddDebugInfo(const std::string& info);
    void UpdateFromGameState(const GameState& game_state);
};

/**
 * Snapshot manager for handling multiple snapshots and debugging workflow
 */
class SnapshotManager {
public:
    SnapshotManager();
    
    // Snapshot operations
    std::string CreateSnapshot(const GameState& game_state, const std::string& phase = "");
    bool LoadSnapshot(const std::string& snapshot_id, GameState& game_state);
    bool SaveSnapshot(const std::string& snapshot_id, const GameSnapshot& snapshot);
    
    // File management
    std::vector<std::string> ListSnapshots() const;
    bool DeleteSnapshot(const std::string& snapshot_id);
    void CleanOldSnapshots(int keep_count = 10);
    
    // Debugging utilities
    std::string GetLatestSnapshotId() const;
    bool HasSnapshot(const std::string& snapshot_id) const;
    GameSnapshot GetSnapshot(const std::string& snapshot_id) const;
    
    // Auto-save functionality
    void EnableAutoSave(bool enabled, int interval_moves = 1) { 
        auto_save_enabled_ = enabled; 
        auto_save_interval_ = interval_moves;
    }
    
    // Analysis
    std::vector<std::string> AnalyzeSnapshots() const;
    std::string CompareSnapshots(const std::string& id1, const std::string& id2) const;
    
private:
    std::string snapshots_dir_;
    std::string latest_snapshot_id_;
    bool auto_save_enabled_;
    int auto_save_interval_;
    int move_counter_;
    
    std::string GenerateSnapshotId() const;
    std::string GetSnapshotFilename(const std::string& snapshot_id) const;
    void EnsureSnapshotsDirectory() const;
};

/**
 * Non-blocking game runner that uses snapshots for development and debugging
 */
class SnapshotGameRunner {
public:
    SnapshotGameRunner();
    
    // Main execution modes
    int RunFromSnapshot(const std::string& snapshot_id = "");
    int RunSingleStep(const std::string& snapshot_id = "");
    int RunUntilCondition(const std::string& condition, const std::string& snapshot_id = "");
    int AnalyzePosition(const std::string& snapshot_id = "");
    
    // Configuration
    void SetStepDelay(int milliseconds) { step_delay_ms_ = milliseconds; }
    void SetVerbose(bool enabled) { verbose_ = enabled; }
    void SetMaxSteps(int max_steps) { max_steps_ = max_steps; }
    void SetOutputFile(const std::string& filename) { output_file_ = filename; }
    
    // AI integration
    void SetAIThinkingTime(double seconds) { ai_thinking_time_ = seconds; }
    void SetMCTSIterations(int iterations) { mcts_iterations_ = iterations; }
    
    // MCTS debugging
    void SetMCTSInstance(std::shared_ptr<MCTS> mcts) { mcts_instance_ = mcts; }
    bool CaptureMCTSState(MCTSSnapshot& mcts_snapshot) const;
    bool RestoreMCTSState(const MCTSSnapshot& mcts_snapshot);
    
    // Extended MCTS snapshot methods
    MCTSSnapshot CaptureMCTSStateForDebug() const;
    bool RunWithMCTSSnapshots(int max_moves = 50);
    
    // Analysis and debugging
    std::string GetCurrentStatus() const;
    std::vector<std::string> GetDebugInfo() const;
    void DumpAnalysis(const std::string& filename = "") const;
    
private:
    SnapshotManager snapshot_manager_;
    bool verbose_;
    int step_delay_ms_;
    int max_steps_;
    std::string output_file_;
    double ai_thinking_time_;
    int mcts_iterations_;
    
    // MCTS integration
    std::shared_ptr<MCTS> mcts_instance_;
    MCTSSnapshot stored_mcts_snapshot_;
    
    // Game state tracking
    GameState current_game_state_;
    Player current_player_;
    int current_move_index_;
    
    std::vector<std::string> debug_log_;
    std::string current_snapshot_id_;
    
    // Internal methods
    bool ExecuteOneStep(GameState& game_state);
    void LogDebug(const std::string& message);
    void RenderGameState(const GameState& game_state) const;
    Move GetAIMove(const GameState& game_state, AIThinkingSnapshot& ai_thinking);
    void AnalyzeCurrentPosition(const GameState& game_state) const;
    bool CheckStopCondition(const std::string& condition, const GameState& game_state) const;
    
    // Helper methods
    GameSnapshot CaptureState() const;
    bool SaveSnapshot(const GameSnapshot& snapshot, const std::string& name = "");
    bool Initialize();
};

} // namespace Einstein
