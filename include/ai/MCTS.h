#pragma once

#include "core/Types.h"
#include "core/ChessBoard.h"
#include <memory>
#include <vector>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <chrono>

namespace Einstein {

// Forward declarations
class ChessBoard;

struct MCTSNode {
    ChessBoard board;
    Player current_player;
    int dice_value;
    Move last_move;
    
    std::weak_ptr<MCTSNode> parent;  // Use weak_ptr to avoid circular reference
    std::vector<std::shared_ptr<MCTSNode>> children;
    mutable std::mutex children_mutex; // protect children vector & expansion flags
    
    std::atomic<int> visits{0};
    std::atomic<int> wins_int{0};  // Store wins * 1000 to avoid floating point atomics
    mutable std::mutex wins_mutex;
    
    bool is_fully_expanded = false;
    bool is_terminal = false;
    GameResult result = GameResult::ONGOING;
    
    MCTSNode(const ChessBoard& board_state, Player player, int dice = 1, 
             const Move& move = {{-1, -1}, {-1, -1}});
    
    double GetUCBValue(double exploration_constant = 1.414) const;
    bool IsFullyExpanded() const;
    std::shared_ptr<MCTSNode> SelectBestChild(double exploration_constant) const;
    void Backpropagate(GameResult result, Player winner);
    double GetWinRate() const;
};

class MCTS {
public:
    explicit MCTS(const AIConfig& config = AIConfig{});
    ~MCTS() = default;
    // Public constants (needed by node methods for scaling without friendship)
    static constexpr int WIN_SCALE = 1000;
    static constexpr int DRAW_SCORE = WIN_SCALE/2;
    static constexpr int SIMULATION_MAX_PLIES = 200;
    
    // Main MCTS interface
    Move FindBestMove(const ChessBoard& board, Player player, int dice);
    void SetConfig(const AIConfig& config);
    
    // Performance monitoring
    int GetIterationsPerformed() const { return iterations_performed_; }
    double GetLastSearchTime() const { return last_search_time_; }
    void ClearTree();
    
    // Analysis features
    std::vector<std::pair<Move, double>> GetMoveAnalysis(const ChessBoard& board, Player player, int dice);
    double EvaluatePosition(const ChessBoard& board, Player player);

    // Snapshot / tree export (trimmed) - exports a shallow copy of current search tree after FindBestMove
    // Parameters: max_depth (inclusive, root depth=0), max_children per node (top by visits)
    struct ExportNode {
        Move move;
        int visits = 0;
        double win_rate = 0.0; // 0..1
        double ucb = 0.0;
        bool terminal = false;
        std::vector<ExportNode> children;
    };
    ExportNode ExportSearchTree(int max_depth = 2, int max_children = 3) const; // Returns empty if no root persisted
    void EnableTreePersistence(bool enable) { persist_last_root_ = enable; }
    // Import a previously exported tree into the live MCTS instance (for resuming searches)
    void ImportSearchTree(const ExportNode& root_export);
    
private:

    AIConfig config_;
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<int> dice_dist_;
    
    mutable std::mutex tree_mutex_;
    std::atomic<bool> search_cancelled_{false};
    std::atomic<int> iterations_performed_{0};
    double last_search_time_ = 0.0;
    bool debug_enabled_ = false; // toggled if log level / config indicates
    bool persist_last_root_ = false;
    std::shared_ptr<MCTSNode> last_root_; // kept for snapshot export when enabled
    
    // MCTS phases
    std::shared_ptr<MCTSNode> Selection(std::shared_ptr<MCTSNode> root);
    std::shared_ptr<MCTSNode> Expansion(std::shared_ptr<MCTSNode> node);
    GameResult Simulation(const ChessBoard& board, Player current_player);
    void Backpropagation(std::shared_ptr<MCTSNode> node, GameResult result, Player winner);
    
    // Helper functions
    std::vector<Move> GetAllPossibleMoves(const ChessBoard& board, Player player, int dice);
    GameResult EvaluateGameState(const ChessBoard& board);
    Player GetOpponent(Player player) const;
    int RollDice();
    
    // Threading support
    void RunIterations(std::shared_ptr<MCTSNode> root, int iterations);
    void SingleThreadIteration(std::shared_ptr<MCTSNode> root);
    void ParallelSearch(std::shared_ptr<MCTSNode> root, const std::chrono::steady_clock::time_point& end_time);
    
    // Neural network integration
    double GetNeuralNetworkEvaluation(const ChessBoard& board, Player player);
    std::vector<double> GetMoveProbabilities(const ChessBoard& board, Player player, int dice);
};

} // namespace Einstein
