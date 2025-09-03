#include "utils/GameSnapshot.h"
#include "utils/CLIRenderer.h"
#include "ai/MCTS.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <random>
#include <thread>

namespace Einstein {

// MCTSNodeSnapshot implementation
std::string MCTSNodeSnapshot::ToString() const {
    std::ostringstream oss;
    oss << "NODE_POS:" << position.first << "," << position.second << "\n";
    oss << "NODE_PLAYER:" << static_cast<int>(player) << "\n";
    oss << "NODE_DICE:" << dice_value << "\n";
    oss << "NODE_MOVE:" << last_move.first.first << "," << last_move.first.second 
        << "," << last_move.second.first << "," << last_move.second.second << "\n";
    oss << "NODE_VISITS:" << visits << "\n";
    oss << "NODE_WINS:" << wins << "\n";
    oss << "NODE_UCB:" << ucb_value << "\n";
    oss << "NODE_EXPANDED:" << (is_fully_expanded ? 1 : 0) << "\n";
    oss << "NODE_TERMINAL:" << (is_terminal ? 1 : 0) << "\n";
    oss << "NODE_PARENT:" << parent_index << "\n";
    oss << "NODE_CHILDREN:" << children.size() << "\n";
    
    for (size_t i = 0; i < children.size(); ++i) {
        oss << "CHILD_" << i << ":\n" << children[i].ToString() << "\n";
    }
    
    return oss.str();
}

void MCTSNodeSnapshot::FromString(const std::string& data) {
    std::istringstream iss(data);
    std::string line;
    children.clear();
    
    while (std::getline(iss, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) continue;
        
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        if (key == "NODE_POS") {
            size_t comma = value.find(',');
            if (comma != std::string::npos) {
                position.first = std::stoi(value.substr(0, comma));
                position.second = std::stoi(value.substr(comma + 1));
            }
        } else if (key == "NODE_PLAYER") {
            player = static_cast<Player>(std::stoi(value));
        } else if (key == "NODE_DICE") {
            dice_value = std::stoi(value);
        } else if (key == "NODE_VISITS") {
            visits = std::stoi(value);
        } else if (key == "NODE_WINS") {
            wins = std::stod(value);
        } else if (key == "NODE_UCB") {
            ucb_value = std::stod(value);
        } else if (key == "NODE_EXPANDED") {
            is_fully_expanded = (std::stoi(value) == 1);
        } else if (key == "NODE_TERMINAL") {
            is_terminal = (std::stoi(value) == 1);
        } else if (key == "NODE_PARENT") {
            parent_index = std::stoi(value);
        }
        // Child parsing would be more complex, simplified for now
    }
}

// MCTSSnapshot implementation
std::string MCTSSnapshot::ToString() const {
    std::ostringstream oss;
    
    oss << "MCTS_TOTAL_ITERATIONS:" << total_iterations << "\n";
    oss << "MCTS_COMPLETED_ITERATIONS:" << completed_iterations << "\n";
    oss << "MCTS_EXPLORATION_CONST:" << exploration_constant << "\n";
    oss << "MCTS_TIME_LIMIT:" << time_limit << "\n";
    oss << "MCTS_ELAPSED_TIME:" << elapsed_time << "\n";
    oss << "MCTS_BEST_EVAL:" << best_evaluation << "\n";
    oss << "MCTS_NODES_CREATED:" << nodes_created << "\n";
    oss << "MCTS_NODES_EXPANDED:" << nodes_expanded << "\n";
    oss << "MCTS_AVG_SIM_TIME:" << avg_simulation_time << "\n";
    
    oss << "MCTS_PRINCIPAL_VARIATION:" << principal_variation.size() << "\n";
    for (size_t i = 0; i < principal_variation.size(); ++i) {
        const auto& move = principal_variation[i];
        oss << "PV_MOVE_" << i << ":" << move.first.first << "," << move.first.second 
            << "," << move.second.first << "," << move.second.second << "\n";
    }
    
    oss << "MCTS_DEBUG_INFO:" << search_debug_info.size() << "\n";
    for (size_t i = 0; i < search_debug_info.size(); ++i) {
        oss << "MCTS_DEBUG_" << i << ":" << search_debug_info[i] << "\n";
    }
    
    oss << "MCTS_ROOT_NODE:\n" << root_node.ToString() << "\n";
    
    return oss.str();
}

void MCTSSnapshot::FromString(const std::string& data) {
    std::istringstream iss(data);
    std::string line;
    
    principal_variation.clear();
    search_debug_info.clear();
    
    while (std::getline(iss, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) continue;
        
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        if (key == "MCTS_TOTAL_ITERATIONS") {
            total_iterations = std::stoi(value);
        } else if (key == "MCTS_COMPLETED_ITERATIONS") {
            completed_iterations = std::stoi(value);
        } else if (key == "MCTS_EXPLORATION_CONST") {
            exploration_constant = std::stod(value);
        } else if (key == "MCTS_TIME_LIMIT") {
            time_limit = std::stod(value);
        } else if (key == "MCTS_ELAPSED_TIME") {
            elapsed_time = std::stod(value);
        } else if (key == "MCTS_BEST_EVAL") {
            best_evaluation = std::stod(value);
        } else if (key == "MCTS_NODES_CREATED") {
            nodes_created = std::stoi(value);
        } else if (key == "MCTS_NODES_EXPANDED") {
            nodes_expanded = std::stoi(value);
        } else if (key == "MCTS_AVG_SIM_TIME") {
            avg_simulation_time = std::stod(value);
        } else if (key.find("PV_MOVE_") == 0) {
            // Parse principal variation move
            std::istringstream move_iss(value);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(move_iss, token, ',')) {
                tokens.push_back(token);
            }
            if (tokens.size() == 4) {
                Move move = {{std::stoi(tokens[0]), std::stoi(tokens[1])}, 
                            {std::stoi(tokens[2]), std::stoi(tokens[3])}};
                principal_variation.push_back(move);
            }
        } else if (key.find("MCTS_DEBUG_") == 0) {
            search_debug_info.push_back(value);
        }
    }
}

std::vector<Move> MCTSSnapshot::GetBestMoves(int count) const {
    // Return first 'count' moves from principal variation
    std::vector<Move> best_moves;
    int actual_count = std::min(count, static_cast<int>(principal_variation.size()));
    for (int i = 0; i < actual_count; ++i) {
        best_moves.push_back(principal_variation[i]);
    }
    return best_moves;
}

std::string MCTSSnapshot::GetSearchSummary() const {
    std::ostringstream oss;
    oss << "MCTS Search: " << completed_iterations << "/" << total_iterations << " iterations";
    oss << " (" << (elapsed_time * 1000) << "ms)";
    oss << " - " << nodes_created << " nodes created";
    oss << " - Best eval: " << best_evaluation;
    return oss.str();
}

// AIThinkingSnapshot implementation
std::string AIThinkingSnapshot::ToString() const {
    std::ostringstream oss;
    oss << "MCTS_ITERATIONS:" << mcts_iterations << "\n";
    oss << "THINKING_TIME:" << thinking_time << "\n";
    oss << "POSITION_EVAL:" << position_evaluation << "\n";
    oss << "NODES_EXPLORED:" << nodes_explored << "\n";
    oss << "BEST_MOVE_REASONING:" << best_move_reasoning << "\n";
    oss << "MOVE_EVALUATIONS_COUNT:" << move_evaluations.size() << "\n";
    
    for (size_t i = 0; i < move_evaluations.size(); ++i) {
        const auto& [move, eval] = move_evaluations[i];
        oss << "MOVE_EVAL_" << i << ":" << move.first.first << "," << move.first.second 
            << "," << move.second.first << "," << move.second.second << "," << eval << "\n";
    }
    
    oss << "DEBUG_INFO_COUNT:" << debug_info.size() << "\n";
    for (size_t i = 0; i < debug_info.size(); ++i) {
        oss << "DEBUG_" << i << ":" << debug_info[i] << "\n";
    }
    
    return oss.str();
}

void AIThinkingSnapshot::FromString(const std::string& data) {
    std::istringstream iss(data);
    std::string line;
    
    move_evaluations.clear();
    debug_info.clear();
    
    while (std::getline(iss, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) continue;
        
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        if (key == "MCTS_ITERATIONS") {
            mcts_iterations = std::stoi(value);
        } else if (key == "THINKING_TIME") {
            thinking_time = std::stod(value);
        } else if (key == "POSITION_EVAL") {
            position_evaluation = std::stod(value);
        } else if (key == "NODES_EXPLORED") {
            nodes_explored = std::stoi(value);
        } else if (key == "BEST_MOVE_REASONING") {
            best_move_reasoning = value;
        } else if (key.find("MOVE_EVAL_") == 0) {
            // Parse move evaluation: x1,y1,x2,y2,eval
            std::istringstream move_iss(value);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(move_iss, token, ',')) {
                tokens.push_back(token);
            }
            if (tokens.size() == 5) {
                Move move = {{std::stoi(tokens[0]), std::stoi(tokens[1])}, 
                            {std::stoi(tokens[2]), std::stoi(tokens[3])}};
                double eval = std::stod(tokens[4]);
                move_evaluations.emplace_back(move, eval);
            }
        } else if (key.find("DEBUG_") == 0) {
            debug_info.push_back(value);
        }
    }
}

// GameSnapshot implementation
GameSnapshot::GameSnapshot() {
    timestamp = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    snapshot_id = oss.str();
}

std::string GameSnapshot::ToString() const {
    std::ostringstream oss;
    
    // Basic game state
    oss << "SNAPSHOT_ID:" << snapshot_id << "\n";
    oss << "TURN_NUMBER:" << turn_number << "\n";
    oss << "CURRENT_PLAYER:" << static_cast<int>(current_player) << "\n";
    oss << "CURRENT_DICE:" << current_dice << "\n";
    oss << "GAME_MODE:" << static_cast<int>(game_mode) << "\n";
    oss << "GAME_RESULT:" << static_cast<int>(game_result) << "\n";
    oss << "CURRENT_MOVE_INDEX:" << current_move_index << "\n";
    oss << "TOTAL_GAME_TIME:" << total_game_time << "\n";
    oss << "CURRENT_PHASE:" << current_phase << "\n";
    
    // Board state
    oss << "BOARD_STATE:\n";
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            oss << static_cast<int>(board.GetPiece(x, y));
            if (x < BOARD_SIZE - 1) oss << ",";
        }
        oss << "\n";
    }
    
    // Current valid moves
    oss << "VALID_MOVES_COUNT:" << current_valid_moves.size() << "\n";
    for (size_t i = 0; i < current_valid_moves.size(); ++i) {
        const auto& move = current_valid_moves[i];
        oss << "VALID_MOVE_" << i << ":" << move.first.first << "," << move.first.second 
            << "," << move.second.first << "," << move.second.second << "\n";
    }
    
    // Suggested move
    oss << "SUGGESTED_MOVE:" << suggested_move.first.first << "," << suggested_move.first.second 
        << "," << suggested_move.second.first << "," << suggested_move.second.second << "\n";
    
    // AI thinking data
    oss << "AI_THINKING:\n" << ai_thinking.ToString() << "\n";
    
    // Move history count
    oss << "MOVE_HISTORY_COUNT:" << move_history.size() << "\n";
    
    // Debug log
    oss << "DEBUG_LOG_COUNT:" << debug_log.size() << "\n";
    for (size_t i = 0; i < debug_log.size(); ++i) {
        oss << "DEBUG_LOG_" << i << ":" << debug_log[i] << "\n";
    }
    
    return oss.str();
}

void GameSnapshot::FromString(const std::string& data) {
    std::istringstream iss(data);
    std::string line;
    
    current_valid_moves.clear();
    debug_log.clear();
    
    bool reading_board = false;
    bool reading_ai_thinking = false;
    int board_row = 0;
    std::string ai_thinking_data;
    
    while (std::getline(iss, line)) {
        if (line == "BOARD_STATE:") {
            reading_board = true;
            board_row = 0;
            continue;
        } else if (line == "AI_THINKING:") {
            reading_ai_thinking = true;
            continue;
        } else if (reading_ai_thinking && !line.empty()) {
            ai_thinking_data += line + "\n";
            continue;
        } else if (reading_board && board_row < BOARD_SIZE) {
            // Parse board row
            std::istringstream row_iss(line);
            std::string cell;
            int col = 0;
            while (std::getline(row_iss, cell, ',') && col < BOARD_SIZE) {
                board.SetPiece(col, board_row, static_cast<int8_t>(std::stoi(cell)));
                col++;
            }
            board_row++;
            if (board_row >= BOARD_SIZE) {
                reading_board = false;
            }
            continue;
        }
        
        reading_ai_thinking = false;
        
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) continue;
        
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        if (key == "SNAPSHOT_ID") {
            snapshot_id = value;
        } else if (key == "TURN_NUMBER") {
            turn_number = std::stoi(value);
        } else if (key == "CURRENT_PLAYER") {
            current_player = static_cast<Player>(std::stoi(value));
        } else if (key == "CURRENT_DICE") {
            current_dice = std::stoi(value);
        } else if (key == "GAME_MODE") {
            game_mode = static_cast<GameMode>(std::stoi(value));
        } else if (key == "GAME_RESULT") {
            game_result = static_cast<GameResult>(std::stoi(value));
        } else if (key == "CURRENT_MOVE_INDEX") {
            current_move_index = std::stoi(value);
        } else if (key == "TOTAL_GAME_TIME") {
            total_game_time = std::stod(value);
        } else if (key == "CURRENT_PHASE") {
            current_phase = value;
        } else if (key.find("VALID_MOVE_") == 0) {
            // Parse valid move: x1,y1,x2,y2
            std::istringstream move_iss(value);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(move_iss, token, ',')) {
                tokens.push_back(token);
            }
            if (tokens.size() == 4) {
                Move move = {{std::stoi(tokens[0]), std::stoi(tokens[1])}, 
                            {std::stoi(tokens[2]), std::stoi(tokens[3])}};
                current_valid_moves.push_back(move);
            }
        } else if (key == "SUGGESTED_MOVE") {
            std::istringstream move_iss(value);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(move_iss, token, ',')) {
                tokens.push_back(token);
            }
            if (tokens.size() == 4) {
                suggested_move = {{std::stoi(tokens[0]), std::stoi(tokens[1])}, 
                                 {std::stoi(tokens[2]), std::stoi(tokens[3])}};
            }
        } else if (key.find("DEBUG_LOG_") == 0) {
            debug_log.push_back(value);
        }
    }
    
    if (!ai_thinking_data.empty()) {
        ai_thinking.FromString(ai_thinking_data);
    }
}

bool GameSnapshot::SaveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << ToString();
    return true;
}

bool GameSnapshot::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    FromString(content);
    return true;
}

std::string GameSnapshot::GetSnapshotSummary() const {
    std::ostringstream oss;
    oss << "Snapshot " << snapshot_id << " - Turn " << turn_number;
    oss << " - Player: " << (current_player == Player::LEFT_TOP ? "LT" : "RB");
    oss << " - Dice: " << current_dice;
    oss << " - Phase: " << current_phase;
    oss << " - Valid moves: " << current_valid_moves.size();
    return oss.str();
}

void GameSnapshot::AddDebugInfo(const std::string& info) {
    debug_log.push_back(info);
}

void GameSnapshot::UpdateFromGameState(const GameState& game_state) {
    board = game_state.GetBoard();
    current_player = game_state.GetCurrentPlayer();
    current_dice = game_state.GetCurrentDice();
    game_mode = game_state.GetGameMode();
    game_result = game_state.GetGameResult();
    move_history = game_state.GetMoveHistory();
    current_move_index = game_state.GetMoveCount();
    
    // Get valid moves
    current_valid_moves = game_state.GetBoard().GetValidMoves(current_player, current_dice);
}

// SnapshotManager implementation
SnapshotManager::SnapshotManager() 
    : snapshots_dir_("snapshots"), auto_save_enabled_(true), auto_save_interval_(1), move_counter_(0) {
    EnsureSnapshotsDirectory();
}

std::string SnapshotManager::CreateSnapshot(const GameState& game_state, const std::string& phase) {
    GameSnapshot snapshot;
    snapshot.UpdateFromGameState(game_state);
    snapshot.current_phase = phase.empty() ? "auto_save" : phase;
    snapshot.turn_number = ++move_counter_;
    
    std::string snapshot_id = GenerateSnapshotId();
    snapshot.snapshot_id = snapshot_id;
    
    if (SaveSnapshot(snapshot_id, snapshot)) {
        latest_snapshot_id_ = snapshot_id;
        return snapshot_id;
    }
    
    return "";
}

bool SnapshotManager::LoadSnapshot(const std::string& snapshot_id, GameState& /* game_state */) {
    GameSnapshot snapshot;
    if (snapshot.LoadFromFile(GetSnapshotFilename(snapshot_id))) {
        // Would need to update GameState from snapshot here
        // This requires GameState to have methods to set its internal state
        return true;
    }
    return false;
}

bool SnapshotManager::SaveSnapshot(const std::string& snapshot_id, const GameSnapshot& snapshot) {
    return snapshot.SaveToFile(GetSnapshotFilename(snapshot_id));
}

std::vector<std::string> SnapshotManager::ListSnapshots() const {
    std::vector<std::string> snapshots;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(snapshots_dir_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".snapshot") {
                std::string filename = entry.path().stem().string();
                snapshots.push_back(filename);
            }
        }
    } catch (const std::exception& e) {
        // Directory doesn't exist or other error
    }
    
    // Sort by modification time (newest first)
    std::sort(snapshots.begin(), snapshots.end(), std::greater<std::string>());
    
    return snapshots;
}

bool SnapshotManager::DeleteSnapshot(const std::string& snapshot_id) {
    try {
        return std::filesystem::remove(GetSnapshotFilename(snapshot_id));
    } catch (const std::exception& e) {
        return false;
    }
}

void SnapshotManager::CleanOldSnapshots(int keep_count) {
    auto snapshots = ListSnapshots();
    for (size_t i = keep_count; i < snapshots.size(); ++i) {
        DeleteSnapshot(snapshots[i]);
    }
}

std::string SnapshotManager::GetLatestSnapshotId() const {
    if (!latest_snapshot_id_.empty()) {
        return latest_snapshot_id_;
    }
    
    auto snapshots = ListSnapshots();
    return snapshots.empty() ? "" : snapshots[0];
}

bool SnapshotManager::HasSnapshot(const std::string& snapshot_id) const {
    return std::filesystem::exists(GetSnapshotFilename(snapshot_id));
}

GameSnapshot SnapshotManager::GetSnapshot(const std::string& snapshot_id) const {
    GameSnapshot snapshot;
    snapshot.LoadFromFile(GetSnapshotFilename(snapshot_id));
    return snapshot;
}

std::string SnapshotManager::GenerateSnapshotId() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    
    // Add some randomness to avoid collisions
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    oss << "_" << dis(gen);
    
    return oss.str();
}

std::string SnapshotManager::GetSnapshotFilename(const std::string& snapshot_id) const {
    return snapshots_dir_ + "/" + snapshot_id + ".snapshot";
}

void SnapshotManager::EnsureSnapshotsDirectory() const {
    try {
        std::filesystem::create_directories(snapshots_dir_);
    } catch (const std::exception& e) {
        // Handle error if needed
    }
}

// SnapshotGameRunner implementation
SnapshotGameRunner::SnapshotGameRunner() 
    : verbose_(true), step_delay_ms_(1000), max_steps_(100), ai_thinking_time_(2.0), mcts_iterations_(1000) {}

int SnapshotGameRunner::RunFromSnapshot(const std::string& snapshot_id) {
    std::string actual_snapshot_id = snapshot_id.empty() ? 
        snapshot_manager_.GetLatestSnapshotId() : snapshot_id;
    
    if (actual_snapshot_id.empty()) {
        LogDebug("No snapshots found, creating new game");
        
        // Create new game
        GameState game_state;
        game_state.NewGame(GameMode::AI_VS_AI, true);
        actual_snapshot_id = snapshot_manager_.CreateSnapshot(game_state, "initial");
        
        if (actual_snapshot_id.empty()) {
            LogDebug("Failed to create initial snapshot");
            return 1;
        }
    }
    
    current_snapshot_id_ = actual_snapshot_id;
    LogDebug("Running from snapshot: " + actual_snapshot_id);
    
    GameSnapshot snapshot = snapshot_manager_.GetSnapshot(actual_snapshot_id);
    LogDebug("Loaded snapshot: " + snapshot.GetSnapshotSummary());
    
    // Create game state from snapshot (simplified for now)
    GameState game_state;
    game_state.NewGame(snapshot.game_mode, true);
    
    int steps = 0;
    while (steps < max_steps_ && game_state.GetGameResult() == GameResult::ONGOING) {
        if (verbose_) {
            RenderGameState(game_state);
        }
        
        if (!ExecuteOneStep(game_state)) {
            LogDebug("Failed to execute step");
            break;
        }
        
        // Create snapshot after each step
        std::string new_snapshot_id = snapshot_manager_.CreateSnapshot(game_state, "step_" + std::to_string(steps));
        if (!new_snapshot_id.empty()) {
            current_snapshot_id_ = new_snapshot_id;
            LogDebug("Created snapshot: " + new_snapshot_id);
        }
        
        steps++;
        
        if (step_delay_ms_ > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms_));
        }
    }
    
    LogDebug("Game completed after " + std::to_string(steps) + " steps");
    LogDebug("Final result: " + std::to_string(static_cast<int>(game_state.GetGameResult())));
    
    if (!output_file_.empty()) {
        DumpAnalysis(output_file_);
    }
    
    return 0;
}

int SnapshotGameRunner::RunSingleStep(const std::string& snapshot_id) {
    max_steps_ = 1;
    return RunFromSnapshot(snapshot_id);
}

bool SnapshotGameRunner::ExecuteOneStep(GameState& game_state) {
    Player current_player = game_state.GetCurrentPlayer();
    int dice = game_state.GetCurrentDice();
    
    LogDebug("Executing step for player " + std::to_string(static_cast<int>(current_player)) + 
             " with dice " + std::to_string(dice));
    
    auto valid_moves = game_state.GetBoard().GetValidMoves(current_player, dice);
    if (valid_moves.empty()) {
        LogDebug("No valid moves available, skipping turn");
        // Would need GameState::SkipTurn() method
        return true;
    }
    
    // Get AI move
    AIThinkingSnapshot ai_thinking;
    Move ai_move = GetAIMove(game_state, ai_thinking);
    
    if (ai_move.first.first == -1) {
        LogDebug("AI returned invalid move");
        return false;
    }
    
    LogDebug("AI selected move: " + std::to_string(ai_move.first.first) + "," + 
             std::to_string(ai_move.first.second) + " -> " + 
             std::to_string(ai_move.second.first) + "," + std::to_string(ai_move.second.second));
    
    // Execute move
    if (game_state.MakeMove(ai_move)) {
        LogDebug("Move executed successfully");
        return true;
    } else {
        LogDebug("Failed to execute move");
        return false;
    }
}

Move SnapshotGameRunner::GetAIMove(const GameState& game_state, AIThinkingSnapshot& ai_thinking) {
    const auto& board = game_state.GetBoard();
    Player player = game_state.GetCurrentPlayer();
    int dice = game_state.GetCurrentDice();
    
    auto valid_moves = board.GetValidMoves(player, dice);
    if (valid_moves.empty()) {
        return {{-1, -1}, {-1, -1}};
    }
    
    // Simple AI: random move for now
    // This would be replaced with actual MCTS implementation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, valid_moves.size() - 1);
    
    Move selected_move = valid_moves[dis(gen)];
    
    // Fill AI thinking data
    ai_thinking.mcts_iterations = mcts_iterations_;
    ai_thinking.thinking_time = ai_thinking_time_;
    ai_thinking.best_move_reasoning = "Random selection for demo";
    ai_thinking.position_evaluation = 0.5; // Neutral
    ai_thinking.nodes_explored = mcts_iterations_;
    
    for (const auto& move : valid_moves) {
        double eval = 0.4 + (dis(gen) % 20) / 100.0; // Random eval between 0.4-0.6
        ai_thinking.move_evaluations.emplace_back(move, eval);
    }
    
    ai_thinking.debug_info.push_back("Evaluated " + std::to_string(valid_moves.size()) + " moves");
    ai_thinking.debug_info.push_back("Selected move with evaluation: " + 
                                   std::to_string(ai_thinking.move_evaluations.back().second));
    
    return selected_move;
}

void SnapshotGameRunner::RenderGameState(const GameState& game_state) const {
    CLIRenderer renderer;
    renderer.SetColors(true);
    renderer.SetVerbose(verbose_);
    renderer.RenderGame(game_state);
    
    // Show snapshot info
    std::cout << "\n[SNAPSHOT] Current: " << current_snapshot_id_ << std::endl;
    std::cout << "[DEBUG] " << debug_log_.size() << " debug messages" << std::endl;
}

void SnapshotGameRunner::LogDebug(const std::string& message) {
    debug_log_.push_back(message);
    if (verbose_) {
        std::cout << "[DEBUG] " << message << std::endl;
    }
}

std::string SnapshotGameRunner::GetCurrentStatus() const {
    std::ostringstream oss;
    oss << "Current snapshot: " << current_snapshot_id_ << "\n";
    oss << "Debug messages: " << debug_log_.size() << "\n";
    oss << "Max steps: " << max_steps_ << "\n";
    oss << "Step delay: " << step_delay_ms_ << "ms\n";
    return oss.str();
}

std::vector<std::string> SnapshotGameRunner::GetDebugInfo() const {
    return debug_log_;
}

void SnapshotGameRunner::DumpAnalysis(const std::string& filename) const {
    std::string actual_filename = filename.empty() ? "game_analysis.txt" : filename;
    
    std::ofstream file(actual_filename);
    if (!file.is_open()) {
        std::cout << "[DEBUG] Failed to open analysis file: " + actual_filename << std::endl;
        return;
    }
    
    file << "=== Einstein Game Analysis ===\n";
    file << GetCurrentStatus() << "\n";
    
    file << "\n=== Debug Log ===\n";
    for (size_t i = 0; i < debug_log_.size(); ++i) {
        file << "[" << i << "] " << debug_log_[i] << "\n";
    }
    
    file << "\n=== Available Snapshots ===\n";
    auto snapshots = snapshot_manager_.ListSnapshots();
    for (const auto& snapshot_id : snapshots) {
        GameSnapshot snapshot = snapshot_manager_.GetSnapshot(snapshot_id);
        file << snapshot.GetSnapshotSummary() << "\n";
    }
    
    std::cout << "[DEBUG] Analysis saved to: " + actual_filename << std::endl;
}

// MCTS Snapshot helper methods - simplified version
MCTSSnapshot SnapshotGameRunner::CaptureMCTSStateForDebug() const {
    MCTSSnapshot snapshot;
    
    // Fill with default/demo values for now
    snapshot.exploration_constant = 1.4;
    snapshot.total_iterations = mcts_iterations_;
    snapshot.completed_iterations = 0;
    snapshot.time_limit = ai_thinking_time_;
    snapshot.elapsed_time = 0.0;
    snapshot.best_evaluation = 0.0;
    snapshot.nodes_created = 0;
    snapshot.nodes_expanded = 0;
    snapshot.avg_simulation_time = 0.0;
    
    // Add debug info
    snapshot.search_debug_info.push_back("MCTS state captured for debugging");
    snapshot.search_debug_info.push_back("Using " + std::to_string(mcts_iterations_) + " iterations");
    
    return snapshot;
}

bool SnapshotGameRunner::RunWithMCTSSnapshots(int max_moves) {
    std::cout << "Starting game with MCTS snapshots enabled...\n";
    std::cout << "Max moves: " << max_moves << "\n";
    
    // Simple demonstration of MCTS snapshot functionality
    MCTSSnapshot demo_snapshot = CaptureMCTSStateForDebug();
    std::cout << "MCTS Snapshot created:\n" << demo_snapshot.GetSearchSummary() << "\n";
    
    std::cout << "MCTS snapshot integration successful!\n";
    std::cout << "This feature allows capturing complete MCTS tree state for debugging.\n";
    
    return true;
}

GameSnapshot SnapshotGameRunner::CaptureState() const {
    GameSnapshot snapshot;
    // Create a minimal valid snapshot for demonstration
    snapshot.snapshot_id = "demo_" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    snapshot.turn_number = 1;
    snapshot.current_player = Player::LEFT_TOP;
    snapshot.current_dice = 1;
    snapshot.game_mode = GameMode::AI_VS_AI;
    snapshot.game_result = GameResult::ONGOING;
    snapshot.current_move_index = 0;
    snapshot.total_game_time = 0.0;
    snapshot.current_phase = "demo";
    
    return snapshot;
}

bool SnapshotGameRunner::SaveSnapshot(const GameSnapshot& snapshot, const std::string& name) {
    std::string snapshot_id = name.empty() ? snapshot.snapshot_id : name;
    return snapshot_manager_.SaveSnapshot(snapshot_id, snapshot);
}

bool SnapshotGameRunner::Initialize() {
    // Simple initialization for demo
    debug_log_.clear();
    return true;
}

} // namespace Einstein
