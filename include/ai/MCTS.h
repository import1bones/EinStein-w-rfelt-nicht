#pragma once

#include "core/Types.h"
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
    
    std::shared_ptr<MCTSNode> parent;
    std::vector<std::shared_ptr<MCTSNode>> children;
    
    std::atomic<int> visits{0};
    std::atomic<double> wins{0.0};
    
    bool is_fully_expanded = false;
    bool is_terminal = false;
    GameResult result = GameResult::ONGOING;
    
    MCTSNode(const ChessBoard& board_state, Player player, int dice = 1, 
             const Move& move = {{-1, -1}, {-1, -1}});
    
    double GetUCBValue(double exploration_constant = 1.414) const;
    bool IsFullyExpanded() const;
    std::shared_ptr<MCTSNode> SelectBestChild(double exploration_constant) const;
    void Backpropagate(GameResult result, Player winner);
};

class MCTS {
public:
    explicit MCTS(const AIConfig& config = AIConfig{});
    ~MCTS() = default;
    
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
    
private:
    AIConfig config_;
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<int> dice_dist_;
    
    mutable std::mutex tree_mutex_;
    std::atomic<bool> search_cancelled_{false};
    std::atomic<int> iterations_performed_{0};
    double last_search_time_ = 0.0;
    
    // MCTS phases
    std::shared_ptr<MCTSNode> Selection(std::shared_ptr<MCTSNode> root);
    std::shared_ptr<MCTSNode> Expansion(std::shared_ptr<MCTSNode> node);
    GameResult Simulation(const ChessBoard& board, Player current_player);
    void Backpropagation(std::shared_ptr<MCTSNode> node, GameResult result, Player winner);
    
    // Helper functions
    std::vector<Move> GetAllPossibleMoves(const ChessBoard& board, Player player, int dice);
    GameResult EvaluateGameState(const ChessBoard& board, Player current_player);
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
