#include "ai/MCTS.h"
#include "core/ChessBoard.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>

namespace Einstein {

// MCTSNode implementation
MCTSNode::MCTSNode(const ChessBoard& board_state, Player player, int dice, const Move& move)
    : board(board_state), current_player(player), dice_value(dice), last_move(move) {
    visits.store(0);
    wins_int.store(0);
}

double MCTSNode::GetWinRate() const {
    int visit_count = visits.load();
    if (visit_count == 0) return 0.0;
    return static_cast<double>(wins_int.load()) / (visit_count * MCTS::WIN_SCALE);
}

double MCTSNode::GetUCBValue(double exploration_constant) const {
    int visit_count = visits.load();
    if (visit_count == 0) return std::numeric_limits<double>::max();
    
    auto parent_ptr = parent.lock();
    if (parent_ptr == nullptr) return 0.0;
    
    double exploitation = GetWinRate();
    double exploration = std::sqrt(std::log(parent_ptr->visits.load()) / visit_count);
    
    return exploitation + exploration_constant * exploration;
}

bool MCTSNode::IsFullyExpanded() const {
    return is_fully_expanded;
}

std::shared_ptr<MCTSNode> MCTSNode::SelectBestChild(double exploration_constant) const {
    if (children.empty()) {
        return nullptr;
    }
    
    auto best_child = std::max_element(children.begin(), children.end(),
        [exploration_constant](const std::shared_ptr<MCTSNode>& a, const std::shared_ptr<MCTSNode>& b) {
            return a->GetUCBValue(exploration_constant) < b->GetUCBValue(exploration_constant);
        });
    
    return *best_child;
}

void MCTSNode::Backpropagate(GameResult result, Player winner) {
    visits.fetch_add(1, std::memory_order_relaxed);
    {
        std::lock_guard<std::mutex> lock(wins_mutex);
        if (result == GameResult::DRAW) {
            wins_int.fetch_add(MCTS::DRAW_SCORE, std::memory_order_relaxed);
        } else if ((winner == current_player && result != GameResult::ONGOING) ||
                   (current_player == Player::LEFT_TOP && result == GameResult::LT_WINS) ||
                   (current_player == Player::RIGHT_BOTTOM && result == GameResult::RB_WINS)) {
            wins_int.fetch_add(MCTS::WIN_SCALE, std::memory_order_relaxed);
        }
    }
    
    // Recursively propagate up the tree
    auto parent_ptr = parent.lock();
    if (parent_ptr != nullptr) {
        parent_ptr->Backpropagate(result, winner);
    }
}

// MCTS implementation
MCTS::MCTS(const AIConfig& config) 
    : config_(config), gen_(rd_()), dice_dist_(1, 6) {
}

Move MCTS::FindBestMove(const ChessBoard& board, Player player, int dice) {
    auto start_time = std::chrono::steady_clock::now();
    iterations_performed_ = 0;
    search_cancelled_ = false;
    
    // Create root node
    auto root = std::make_shared<MCTSNode>(board, player, dice);
    if (persist_last_root_) {
        std::lock_guard<std::mutex> lock(tree_mutex_);
        last_root_ = root;
    }
    
    // Calculate end time based on configuration
    auto end_time = start_time + std::chrono::milliseconds(
        static_cast<int>(config_.thinking_time * 1000));
    
    // Run MCTS search
    if (config_.thread_count > 1 && config_.enable_multithreading) {
        ParallelSearch(root, end_time);
    } else {
        // Respect time limit also in single-thread mode
        int safety_iterations = std::max(1, config_.mcts_iterations);
        for (int i = 0; i < safety_iterations && std::chrono::steady_clock::now() < end_time && !search_cancelled_; ++i) {
            SingleThreadIteration(root);
            iterations_performed_++;
        }
    }
    
    // Calculate search time
    auto search_end = std::chrono::steady_clock::now();
    last_search_time_ = std::chrono::duration<double>(search_end - start_time).count();
    
    // Select best move
    if (root->children.empty()) {
        // Fallback: return any valid move
        auto valid_moves = GetAllPossibleMoves(board, player, dice);
        if (!valid_moves.empty()) {
            return valid_moves[0];
        }
        return {{0, 0}, {0, 0}}; // Invalid move if no moves available
    }
    
    // Find child with most visits (most robust choice)
    auto best_child = std::max_element(root->children.begin(), root->children.end(),
        [](const std::shared_ptr<MCTSNode>& a, const std::shared_ptr<MCTSNode>& b) {
            return a->visits.load() < b->visits.load();
        });
    
    return (*best_child)->last_move;
}

// Helper to sort and take top K children by visits
static void TrimChildren(std::vector<std::shared_ptr<MCTSNode>>& children, int max_children) {
    std::sort(children.begin(), children.end(), [](const auto& a, const auto& b) {
        return a->visits.load() > b->visits.load();
    });
    if ((int)children.size() > max_children) {
        children.resize(max_children);
    }
}

MCTS::ExportNode MCTS::ExportSearchTree(int max_depth, int max_children) const {
    ExportNode root_export;
    if (!persist_last_root_) return root_export;
    std::shared_ptr<MCTSNode> root_copy;
    {
        std::lock_guard<std::mutex> lock(tree_mutex_);
        root_copy = last_root_;
    }
    if (!root_copy) return root_export;

    std::function<void(const std::shared_ptr<MCTSNode>&, ExportNode&, int)> dfs;
    dfs = [&](const std::shared_ptr<MCTSNode>& node, ExportNode& out, int depth) {
        if (!node) return;
        out.move = node->last_move;
        out.visits = node->visits.load();
        out.win_rate = node->GetWinRate();
        out.ucb = node->GetUCBValue(1.414); // exploration constant snapshot
        out.terminal = node->is_terminal;
        if (depth >= max_depth) return;
        std::vector<std::shared_ptr<MCTSNode>> kids;
        {
            std::lock_guard<std::mutex> lock(node->children_mutex);
            kids = node->children; // copy
        }
        // Only trim at root level; include all grandchildren for selected top children
        if (depth == 0) {
            TrimChildren(kids, max_children);
        }
        for (auto& child : kids) {
            ExportNode child_out;
            dfs(child, child_out, depth + 1);
            out.children.push_back(std::move(child_out));
        }
    };
    dfs(root_copy, root_export, 0);
    return root_export;
}

// Import an exported tree back into the MCTS structure. This creates a shallow
// live tree where visits/wins/ucb are restored into node counters so searches
// can continue (best-effort). This is intentionally conservative: we rebuild
// nodes with board states based on last_move application where possible.
void MCTS::ImportSearchTree(const ExportNode& root_export) {
    if (!persist_last_root_) return;

    // Helper to recursively build nodes
    std::function<std::shared_ptr<MCTSNode>(const ExportNode&, const std::shared_ptr<MCTSNode>&)> build;
    build = [&](const ExportNode& exp, const std::shared_ptr<MCTSNode>& parent) -> std::shared_ptr<MCTSNode> {
        // The root export move may be default; for children we should apply move to parent's board
        ChessBoard board_state;
        Player player = Player::LEFT_TOP;
        int dice = 1;
        Move move = exp.move;

        std::shared_ptr<MCTSNode> node;
        if (!parent) {
            // Root: unknown board, create an empty placeholder
            node = std::make_shared<MCTSNode>(board_state, player, dice, move);
        } else {
            ChessBoard new_board = parent->board;
            if (move.first.first != -1) {
                new_board.ExecuteMove(move);
            }
            player = GetOpponent(parent->current_player);
            dice = parent->dice_value; // not precise but acceptable as placeholder
            node = std::make_shared<MCTSNode>(new_board, player, dice, move);
            node->parent = parent;
            parent->children.push_back(node);
        }

        // Restore counters (approximate): wins_int stored as fraction of WIN_SCALE
        node->visits.store(exp.visits);
        int wins_scaled = static_cast<int>(std::round(exp.win_rate * MCTS::WIN_SCALE));
        node->wins_int.store(wins_scaled);
        node->is_terminal = exp.terminal;

        // Recurse children
        for (const auto& child_exp : exp.children) {
            build(child_exp, node);
        }

        return node;
    };

    std::lock_guard<std::mutex> lock(tree_mutex_);
    last_root_.reset();
    last_root_ = build(root_export, nullptr);
}

void MCTS::SetConfig(const AIConfig& config) {
    config_ = config;
}

void MCTS::ClearTree() {
    // Trees will be automatically cleaned up by shared_ptr
}

std::vector<std::pair<Move, double>> MCTS::GetMoveAnalysis(const ChessBoard& board, Player player, int dice) {
    std::vector<std::pair<Move, double>> analysis;
    
    auto valid_moves = GetAllPossibleMoves(board, player, dice);
    for (const auto& move : valid_moves) {
        ChessBoard temp_board = board;
        temp_board.ExecuteMove(move);
        double evaluation = EvaluatePosition(temp_board, GetOpponent(player));
        analysis.emplace_back(move, evaluation);
    }
    
    // Sort by evaluation (best first)
    std::sort(analysis.begin(), analysis.end(),
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
    
    return analysis;
}

double MCTS::EvaluatePosition(const ChessBoard& board, Player player) {
    // Simple heuristic evaluation
    // In a real implementation, this would use the neural network
    
    // Check for immediate win/loss
    if (board.HasPlayerWon(player)) {
        return 1.0;
    }
    if (board.HasPlayerWon(GetOpponent(player))) {
        return -1.0;
    }
    
    // Positional evaluation based on piece positions
    auto player_pieces = board.GetPlayerPieces(player);
    auto opponent_pieces = board.GetPlayerPieces(GetOpponent(player));
    
    double score = 0.0;
    
    // Piece count advantage
    score += (player_pieces.size() - opponent_pieces.size()) * 0.2;
    
    // Position advancement (closer to target corner)
    for (const auto& pos : player_pieces) {
        if (player == Player::LEFT_TOP) {
            // LT player wants to reach bottom-right
            score += (pos.first + pos.second) * 0.1;
        } else {
            // RB player wants to reach top-left
            score += (8 - pos.first - pos.second) * 0.1;
        }
    }
    
    return std::tanh(score); // Normalize to [-1, 1]
}

// Private methods
std::shared_ptr<MCTSNode> MCTS::Selection(std::shared_ptr<MCTSNode> root) {
    auto current = root;
    while (true) {
        // Snapshot children safely
        std::vector<std::shared_ptr<MCTSNode>> snapshot;
        {
            std::lock_guard<std::mutex> lock(current->children_mutex);
            if (current->children.empty() || !current->IsFullyExpanded()) break;
            snapshot = current->children; // copy
        }
        auto best = current->SelectBestChild(config_.exploration_constant);
        if (!best) break;
        current = best;
    }
    return current;
}

std::shared_ptr<MCTSNode> MCTS::Expansion(std::shared_ptr<MCTSNode> node) {
    if (node->is_terminal) return node;
    auto valid_moves = GetAllPossibleMoves(node->board, node->current_player, node->dice_value);
    if (valid_moves.empty()) {
        node->is_terminal = true;
        node->result = EvaluateGameState(node->board);
        return node;
    }
    {
        std::lock_guard<std::mutex> lock(node->children_mutex);
        if (node->children.size() < valid_moves.size()) {
            // Expand ONE child only (typical incremental expansion)
            size_t index = node->children.size();
            const auto& move = valid_moves[index];
            ChessBoard new_board = node->board;
            new_board.ExecuteMove(move);
            Player next_player = GetOpponent(node->current_player);
            int next_dice = RollDice();
            auto child = std::make_shared<MCTSNode>(new_board, next_player, next_dice, move);
            child->parent = node;
            node->children.push_back(child);
            if (node->children.size() == valid_moves.size()) node->is_fully_expanded = true;
            return child; // newly expanded
        }
    }
    // Pick an unvisited child if any (no lock after snapshot)
    {
        std::lock_guard<std::mutex> lock(node->children_mutex);
        for (auto& child : node->children) {
            if (child->visits.load(std::memory_order_relaxed) == 0) return child;
        }
        if (!node->children.empty()) return node->children[0];
    }
    return node;
}

GameResult MCTS::Simulation(const ChessBoard& board, Player current_player) {
    ChessBoard sim_board = board;
    Player sim_player = current_player;
    int moves_played = 0;
    while (moves_played < SIMULATION_MAX_PLIES) {
        // Check for win condition
        if (sim_board.HasPlayerWon(Player::LEFT_TOP)) {
            return GameResult::LT_WINS;
        }
        if (sim_board.HasPlayerWon(Player::RIGHT_BOTTOM)) {
            return GameResult::RB_WINS;
        }
        
        int dice = RollDice();
        auto valid_moves = GetAllPossibleMoves(sim_board, sim_player, dice);
        
        if (valid_moves.empty()) {
            // No moves available - game ends
            break;
        }
        
        // Select random move
        std::uniform_int_distribution<size_t> move_dist(0, valid_moves.size() - 1);
        const auto& selected_move = valid_moves[move_dist(gen_)];
        
        sim_board.ExecuteMove(selected_move);
    sim_player = GetOpponent(sim_player);
    ++moves_played;
    }
    
    // If no winner found, use position evaluation
    double eval = EvaluatePosition(sim_board, Player::LEFT_TOP);
    if (eval > 0.1) return GameResult::LT_WINS;
    if (eval < -0.1) return GameResult::RB_WINS;
    return GameResult::DRAW;
}

void MCTS::Backpropagation(std::shared_ptr<MCTSNode> node, GameResult result, Player winner) {
    node->Backpropagate(result, winner);
}

std::vector<Move> MCTS::GetAllPossibleMoves(const ChessBoard& board, Player player, int dice) {
    return board.GetValidMoves(player, dice);
}

GameResult MCTS::EvaluateGameState(const ChessBoard& board) {
    if (board.HasPlayerWon(Player::LEFT_TOP)) {
        return GameResult::LT_WINS;
    }
    if (board.HasPlayerWon(Player::RIGHT_BOTTOM)) {
        return GameResult::RB_WINS;
    }
    return GameResult::ONGOING;
}

Player MCTS::GetOpponent(Player player) const {
    return (player == Player::LEFT_TOP) ? Player::RIGHT_BOTTOM : Player::LEFT_TOP;
}

int MCTS::RollDice() {
    return dice_dist_(gen_);
}

void MCTS::RunIterations(std::shared_ptr<MCTSNode> root, int iterations) {
    for (int i = 0; i < iterations && !search_cancelled_; ++i) {
        SingleThreadIteration(root);
        iterations_performed_++;
    }
}

void MCTS::SingleThreadIteration(std::shared_ptr<MCTSNode> root) {
    // Selection
    auto node = Selection(root);
    
    // Expansion
    auto leaf = Expansion(node);
    
    // Simulation
    GameResult result = Simulation(leaf->board, leaf->current_player);
    Player winner = (result == GameResult::LT_WINS) ? Player::LEFT_TOP :
                   (result == GameResult::RB_WINS) ? Player::RIGHT_BOTTOM : Player::NONE;
    
    // Backpropagation
    Backpropagation(leaf, result, winner);
}

void MCTS::ParallelSearch(std::shared_ptr<MCTSNode> root, const std::chrono::steady_clock::time_point& end_time) {
    std::vector<std::future<void>> futures;
    
    // Launch worker threads
    for (int t = 0; t < config_.thread_count; ++t) {
        futures.emplace_back(std::async(std::launch::async, [this, root, end_time]() {
            while (std::chrono::steady_clock::now() < end_time && !search_cancelled_) {
                SingleThreadIteration(root);
                iterations_performed_++;
            }
        }));
    }
    
    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }
}

double MCTS::GetNeuralNetworkEvaluation(const ChessBoard& board, Player player) {
    // Placeholder for neural network integration
    // In a complete implementation, this would call the Python neural network
    return EvaluatePosition(board, player);
}

std::vector<double> MCTS::GetMoveProbabilities(const ChessBoard& board, Player player, int dice) {
    // Placeholder for neural network move probability prediction
    auto valid_moves = GetAllPossibleMoves(board, player, dice);
    std::vector<double> probabilities(valid_moves.size(), 1.0 / valid_moves.size());
    return probabilities;
}

} // namespace Einstein
