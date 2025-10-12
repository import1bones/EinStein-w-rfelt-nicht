#include "utils/GameSnapshot.h"
#include "utils/CLIRenderer.h"
#include "ai/MCTS.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdint>
#include <random>
#include <thread>
#include <sstream>
#include <algorithm>
#include <functional>
#include <utility>
#include <string>
#include <chrono>
#include "third_party/json_adapter.hpp"
using ein_json::Value;
using ein_json::Object;
using ein_json::Array;
#ifdef USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

namespace Einstein {

// NOLINTBEGIN(readability-identifier-naming,readability-identifier-length)

// Local constants to avoid magic-number diagnostics
static constexpr int DEFAULT_STEP_DELAY_MS = 1000;
static constexpr int DEFAULT_MAX_STEPS = 100;
static constexpr int DEFAULT_MCTS_ITERATIONS = 1000;
static constexpr int SNAPSHOT_RANDOM_MIN = 1000;
static constexpr int SNAPSHOT_RANDOM_MAX = 9999;
static constexpr double DEFAULT_AI_THINKING_TIME = 2.0;
static constexpr double DEFAULT_MCTS_EXPLORATION_CONST = 1.4;
static constexpr int DEFAULT_MCTS_EXPORT_DEPTH = 2;
static constexpr int DEFAULT_MCTS_EXPORT_WIDTH = 5;
static constexpr int MS_PER_SECOND = 1000;
static constexpr int MOVE_EVAL_SIZE = 5;

// Internal helpers to decompose large parsing logic and reduce function complexity
namespace {
void ParseBoardValue(const Value& board_val, ChessBoard& out_board) {
    if (!board_val.is_array()) { return; }
    const auto& rows = board_val.as_array();
    for (int rIdx = 0; rIdx < BOARD_SIZE && rIdx < static_cast<int>(rows.size()); ++rIdx) {
        if (!rows[rIdx].is_array()) { continue; }
        const auto& cols = rows[rIdx].as_array();
        for (int cIdx = 0; cIdx < BOARD_SIZE && cIdx < static_cast<int>(cols.size()); ++cIdx) {
            if (cols[cIdx].is_number()) {
                out_board.SetPiece(cIdx, rIdx, static_cast<int8_t>(cols[cIdx].as_int()));
            }
        }
    }
}

void ParseValidMovesValue(const Value& vm_val, std::vector<Move>& out_moves) {
    if (!vm_val.is_array()) { return; }
    for (const auto& item : vm_val.as_array()) {
        if (!item.is_array()) { continue; }
        const auto& arr = item.as_array();
        if (arr.size() == 4) {
            out_moves.emplace_back(Move{{arr[0].as_int(), arr[1].as_int()}, {arr[2].as_int(), arr[3].as_int()}});
        }
    }
}

void ParseMoveHistoryValue(const Value& mh_val, std::vector<GameMove>& out_history) {
    if (!mh_val.is_array()) { return; }
    for (const auto& item : mh_val.as_array()) {
        if (!item.is_array()) { continue; }
        const auto& arr = item.as_array();
        if (arr.size() == 4) {
            Move move = {{arr[0].as_int(), arr[1].as_int()}, {arr[2].as_int(), arr[3].as_int()}};
            out_history.emplace_back(move, Player::LEFT_TOP, 0);
        }
    }
}

void ParseDebugLogValue(const Value& dbg_val, std::vector<std::string>& out_debug) {
    if (!dbg_val.is_array()) { return; }
    for (const auto& item : dbg_val.as_array()) {
    if (item.is_string()) { out_debug.emplace_back(item.as_string()); }
    }
}
} // anonymous namespace


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

// Simple JSON serialization for MCTSSnapshot (no external deps)
std::string MCTSSnapshot::ToJson() const {
    // Produce a compact, safe JSON structure limited to a small depth to avoid
    // emitting malformed nested strings. We include essential stats only.
    Object json_obj;
    json_obj["total_iterations"] = Value(total_iterations);
    json_obj["completed_iterations"] = Value(completed_iterations);
    json_obj["exploration_constant"] = Value(exploration_constant);
    json_obj["time_limit"] = Value(time_limit);
    json_obj["elapsed_time"] = Value(elapsed_time);
    json_obj["best_evaluation"] = Value(best_evaluation);
    json_obj["nodes_created"] = Value(nodes_created);
    json_obj["nodes_expanded"] = Value(nodes_expanded);
    json_obj["avg_simulation_time"] = Value(avg_simulation_time);

    Array pv;
    for (const auto& pv_move : principal_variation) {
        Array tmp = {Value(pv_move.first.first), Value(pv_move.first.second), Value(pv_move.second.first), Value(pv_move.second.second)};
        pv.emplace_back(std::move(tmp));
    }
    json_obj["principal_variation"] = Value(pv);

    // Depth-limited node serializer: include only numeric/boolean fields and arrays
    const int MAX_DEPTH = 2;
    const size_t MAX_CHILDREN = 5;
    std::function<Value(const MCTSNodeSnapshot&, int)> node_to_json = [&](const MCTSNodeSnapshot& node, int depth) -> Value {
        Object node_json;
        // last_move
        Array last_move_arr;
        last_move_arr.emplace_back(Value(node.last_move.first.first));
        last_move_arr.emplace_back(Value(node.last_move.first.second));
        last_move_arr.emplace_back(Value(node.last_move.second.first));
        last_move_arr.emplace_back(Value(node.last_move.second.second));
        node_json["last_move"] = Value(last_move_arr);

    node_json["visits"] = Value(node.visits);
    node_json["ucb"] = Value(node.ucb_value);
    node_json["is_terminal"] = Value(node.is_terminal);
    // prior/virtual_loss not present in MCTSNodeSnapshot; include zeros for compatibility
    node_json["prior"] = Value(0.0);
    node_json["virtual_loss"] = Value(0);

        Array children;
        if (depth > 0 && !node.children.empty()) {
            size_t count = std::min(node.children.size(), MAX_CHILDREN);
            for (size_t i = 0; i < count; ++i) {
                children.emplace_back(node_to_json(node.children[i], depth - 1));
            }
        }
        node_json["children"] = Value(children);
        return Value(node_json);
    };

    json_obj["root_node"] = node_to_json(root_node, MAX_DEPTH);
    return Value(json_obj).dump(2);
}

std::vector<Move> MCTSSnapshot::GetBestMoves(int count) const {
    // Return first 'count' moves from principal variation
    std::vector<Move> best_moves;
    int actual_count = std::min(count, static_cast<int>(principal_variation.size()));
    best_moves.reserve(static_cast<size_t>(actual_count));
    for (int i = 0; i < actual_count; ++i) {
        best_moves.emplace_back(principal_variation[i]);
    }
    return best_moves;
}

std::string MCTSSnapshot::GetSearchSummary() const {
    std::ostringstream oss;
    oss << "MCTS Search: " << completed_iterations << "/" << total_iterations << " iterations";
    oss << " (" << (elapsed_time * MS_PER_SECOND) << "ms)";
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
    // Minimal MCTS tree (top 3 children depth 2) flattened
    if (has_mcts_data) {
        oss << "MCTS_TREE_ENABLED:1\n";
        // Root summary
        oss << "MCTS_ROOT_VISITS:" << mcts_state.root_node.visits << "\n";
        oss << "MCTS_ROOT_CHILDREN:" << mcts_state.root_node.children.size() << "\n";
        // Serialize first-level children limited to 3 already
        for (size_t i = 0; i < mcts_state.root_node.children.size(); ++i) {
            const auto& c = mcts_state.root_node.children[i];
            oss << "MCTS_C1_" << i << ":" << c.last_move.first.first << "," << c.last_move.first.second
                << "," << c.last_move.second.first << "," << c.last_move.second.second
                << "," << c.visits << "," << c.wins << "," << c.ucb_value << "\n";
            // Second level for each child
            for (size_t j = 0; j < c.children.size(); ++j) {
                const auto& gc = c.children[j];
                oss << "MCTS_C2_" << i << "_" << j << ":" << gc.last_move.first.first << "," << gc.last_move.first.second
                    << "," << gc.last_move.second.first << "," << gc.last_move.second.second
                    << "," << gc.visits << "," << gc.wins << "," << gc.ucb_value << "\n";
            }
        }
    } else {
        oss << "MCTS_TREE_ENABLED:0\n";
    }
    
    return oss.str();
}

std::string AIThinkingSnapshot::ToJson() const {
    Object json_obj;
    json_obj["mcts_iterations"] = Value(mcts_iterations);
    json_obj["thinking_time"] = Value(thinking_time);
    json_obj["position_evaluation"] = Value(position_evaluation);
    json_obj["nodes_explored"] = Value(nodes_explored);
    json_obj["best_move_reasoning"] = Value(best_move_reasoning);

    Array move_eval_arr;
    for (const auto& meval : move_evaluations) {
        move_eval_arr.emplace_back(Array{Value(meval.first.first.first), Value(meval.first.first.second), Value(meval.first.second.first), Value(meval.first.second.second), Value(meval.second)});
    }
    json_obj["move_evaluations"] = Value(move_eval_arr);
    json_obj["debug_info"] = Value(Array());
    if (!debug_info.empty()) {
        Array debug_arr;
        for (const auto& dbg_str : debug_info) {
            debug_arr.emplace_back(Value(dbg_str));
        }
        json_obj["debug_info"] = Value(debug_arr);
    }

    if (has_mcts_data) {
        try {
            Value mcts_val = ein_json::parse(mcts_state.ToJson());
            json_obj["mcts_state"] = mcts_val;
        } catch (const std::exception& /*e*/) {
            // omit on failure
        }
    }
    return Value(json_obj).dump(2);
}

void AIThinkingSnapshot::FromString(const std::string& data) {
    try {
        Value root = ein_json::parse(data);
        if (root.is_object()) {
            auto obj = root.as_object();
            if (obj.count("mcts_iterations") != 0) { mcts_iterations = obj.at("mcts_iterations").as_int(); }
            if (obj.count("thinking_time") != 0) { thinking_time = obj.at("thinking_time").as_double(); }
            if (obj.count("position_evaluation") != 0) { position_evaluation = obj.at("position_evaluation").as_double(); }
            if (obj.count("nodes_explored") != 0) { nodes_explored = obj.at("nodes_explored").as_int(); }
            if (obj.count("best_move_reasoning") != 0 && obj.at("best_move_reasoning").is_string()) { best_move_reasoning = obj.at("best_move_reasoning").as_string(); }


            move_evaluations.clear();
            if (obj.count("move_evaluations") != 0 && obj.at("move_evaluations").is_array()) {
                for (const auto& entry : obj.at("move_evaluations").as_array()) {
                    if (entry.is_array()) {
                        const auto& arr = entry.as_array();
                        if (arr.size() == static_cast<size_t>(MOVE_EVAL_SIZE)) {
                            Move mv = {{arr[0].as_int(), arr[1].as_int()}, {arr[2].as_int(), arr[3].as_int()}};
                            double ev = arr[4].as_double();
                            move_evaluations.emplace_back(mv, ev);
                        }
                    }
                }
            }

            debug_info.clear();
            if (obj.count("debug_info") != 0 && obj.at("debug_info").is_array()) {
                for (const auto& entry : obj.at("debug_info").as_array()) {
                    if (entry.is_string()) debug_info.emplace_back(entry.as_string());
                }
            }

            if (obj.count("mcts_state") != 0) {
                try {
                    mcts_state.FromString(obj.at("mcts_state").dump());
                    has_mcts_data = true;
                    } catch (const std::exception& e) {
                        (void)e;
                        has_mcts_data = false;
                    }
            }
        }
    } catch (const std::exception& e) { (void)e; }
}

// GameSnapshot implementation
GameSnapshot::GameSnapshot() {
    timestamp = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tm{};
#if defined(_MSC_VER)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    snapshot_id = oss.str();
}

std::string GameSnapshot::ToString() const {
    Object j;
    j["snapshot_id"] = Value(snapshot_id);
    j["turn_number"] = Value(turn_number);
    j["current_player"] = Value(static_cast<int>(current_player));
    j["current_dice"] = Value(current_dice);
    j["game_mode"] = Value(static_cast<int>(game_mode));
    j["game_result"] = Value(static_cast<int>(game_result));
    j["current_move_index"] = Value(static_cast<int>(current_move_index));
    j["total_game_time"] = Value(total_game_time);
    j["current_phase"] = Value(current_phase);

    // board
    Array board_arr;
    for (int y = 0; y < BOARD_SIZE; ++y) {
        Array row;
        for (int x = 0; x < BOARD_SIZE; ++x) {
            row.emplace_back(static_cast<int>(board.GetPiece(x, y)));
        }
        board_arr.emplace_back(std::move(row));
    }
    j["board"] = Value(board_arr);

    // valid moves
    Array vm;
    for (const auto& move_item : current_valid_moves) {
        Array tmp = {move_item.first.first, move_item.first.second, move_item.second.first, move_item.second.second};
        vm.emplace_back(std::move(tmp));
    }
    j["valid_moves"] = Value(vm);

    j["suggested_move"] = Value(Array{Value(suggested_move.first.first), Value(suggested_move.first.second), Value(suggested_move.second.first), Value(suggested_move.second.second)});

    Array mh;
    for (const auto& gm_item : move_history) {
        Array tmp = {gm_item.move.first.first, gm_item.move.first.second, gm_item.move.second.first, gm_item.move.second.second};
        mh.emplace_back(std::move(tmp));
    }
    j["move_history"] = Value(mh);

    try {
        Value ai_val = ein_json::parse(ai_thinking.ToJson());
        j["ai_thinking"] = ai_val;
    } catch (const std::exception& e) {
        // Parsing AI thinking JSON failed; include empty object
        (void)e;
        j["ai_thinking"] = Value();
    }

    Array dbg;
    for (const auto& dbg_str : debug_log) {
        dbg.emplace_back(dbg_str);
    }
    j["debug_log"] = Value(dbg);
    return Value(j).dump(2);
}
void GameSnapshot::FromString(const std::string& data) {
    try {
        Value root = ein_json::parse(data);
        if (root.is_object()) {
            auto obj = root.as_object();
            if (obj.count("snapshot_id") > 0 && obj.at("snapshot_id").is_string()) {
                snapshot_id = obj.at("snapshot_id").as_string();
            }
            if (obj.count("turn_number") > 0) { turn_number = obj.at("turn_number").as_int(); }
            if (obj.count("current_player") > 0) { current_player = static_cast<Player>(obj.at("current_player").as_int()); }
            if (obj.count("current_dice") > 0) { current_dice = obj.at("current_dice").as_int(); }
            if (obj.count("game_mode") > 0) { game_mode = static_cast<GameMode>(obj.at("game_mode").as_int()); }
            if (obj.count("game_result") > 0) { game_result = static_cast<GameResult>(obj.at("game_result").as_int()); }
            if (obj.count("current_move_index") > 0) { current_move_index = obj.at("current_move_index").as_int(); }
            if (obj.count("total_game_time") > 0) { total_game_time = obj.at("total_game_time").as_double(); }
            if (obj.count("current_phase") > 0 && obj.at("current_phase").is_string()) { current_phase = obj.at("current_phase").as_string(); }

            if (obj.count("board") > 0 && obj.at("board").is_array()) {
                const auto& barr = obj.at("board").as_array();
                for (int y = 0; y < BOARD_SIZE && y < static_cast<int>(barr.size()); ++y) {
                    if (!barr[y].is_array()) { continue; }
                    const auto& row = barr[y].as_array();
                    for (int x = 0; x < BOARD_SIZE && x < static_cast<int>(row.size()); ++x) {
                        if (row[x].is_number()) {
                            board.SetPiece(x, y, static_cast<int8_t>(row[x].as_int()));
                        }
                    }
                }
            }

            current_valid_moves.clear();
            if (obj.count("valid_moves") > 0 && obj.at("valid_moves").is_array()) {
                for (const auto& it : obj.at("valid_moves").as_array()) {
                    if (it.is_array()) {
                        const auto& arr = it.as_array();
                        if (arr.size() == 4) {
                            current_valid_moves.emplace_back(Move{{arr[0].as_int(), arr[1].as_int()}, {arr[2].as_int(), arr[3].as_int()}});
                        }
                    }
                }
            }

            if (obj.count("suggested_move") > 0 && obj.at("suggested_move").is_array()) {
                const auto& sm = obj.at("suggested_move").as_array();
                if (sm.size() == 4) {
                    suggested_move = {{sm[0].as_int(), sm[1].as_int()}, {sm[2].as_int(), sm[3].as_int()}};
                }
            }

            move_history.clear();
            if (obj.count("move_history") > 0 && obj.at("move_history").is_array()) {
                for (const auto& it : obj.at("move_history").as_array()) {
                    if (it.is_array()) {
                        const auto& arr = it.as_array();
                        if (arr.size() == 4) {
                            Move mv = {{arr[0].as_int(), arr[1].as_int()}, {arr[2].as_int(), arr[3].as_int()}};
                            GameMove gm(mv, Player::LEFT_TOP, 0);
                            move_history.emplace_back(std::move(gm));
                        }
                    }
                }
            }

            if (obj.count("ai_thinking") > 0) {
                try {
                    ai_thinking.FromString(obj.at("ai_thinking").dump());
                } catch (const std::exception& e) {
                    (void)e;
                }
            }

            debug_log.clear();
            if (obj.count("debug_log") > 0 && obj.at("debug_log").is_array()) {
                for (const auto& it : obj.at("debug_log").as_array()) {
                    if (it.is_string()) { debug_log.emplace_back(it.as_string()); }
                }
            }
        }
    } catch (const std::exception& e) { (void)e; }
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
    if (!file.is_open()) { return false; }
    std::ostringstream ss;
    ss << file.rdbuf();
    try {
        FromString(ss.str());
    } catch (...) {
        return false;
    }
    return true;
}

std::string GameSnapshot::GetSnapshotSummary() const {
    std::ostringstream oss;
    oss << snapshot_id << " (turn=" << turn_number << ", player=" << static_cast<int>(current_player) << ")";
    return oss.str();
}

void GameSnapshot::UpdateFromGameState(const GameState& game_state) {
    board = game_state.GetBoard();
    current_player = game_state.GetCurrentPlayer();
    current_dice = game_state.GetCurrentDice();
    game_mode = game_state.GetGameMode();
    game_result = game_state.GetGameResult();
    move_history = game_state.GetMoveHistory();
    current_move_index = game_state.GetMoveCount();
    total_game_time = game_state.GetStatistics().GetGameDurationSeconds();
}

void MCTSSnapshot::FromString(const std::string& data) {
#ifdef USE_NLOHMANN_JSON
    try {
        nlohmann::json nj = nlohmann::json::parse(data);
        if (nj.is_object()) {
            if (nj.contains("total_iterations")) total_iterations = nj["total_iterations"].get<int>();
            if (nj.contains("completed_iterations")) completed_iterations = nj["completed_iterations"].get<int>();
            if (nj.contains("exploration_constant")) exploration_constant = nj["exploration_constant"].get<double>();
            if (nj.contains("time_limit")) time_limit = nj["time_limit"].get<double>();
            if (nj.contains("elapsed_time")) elapsed_time = nj["elapsed_time"].get<double>();
            if (nj.contains("best_evaluation")) best_evaluation = nj["best_evaluation"].get<double>();
            if (nj.contains("nodes_created")) nodes_created = nj["nodes_created"].get<int>();
            if (nj.contains("nodes_expanded")) nodes_expanded = nj["nodes_expanded"].get<int>();
            if (nj.contains("avg_simulation_time")) avg_simulation_time = nj["avg_simulation_time"].get<double>();

            principal_variation.clear();
            if (nj.contains("principal_variation") && nj["principal_variation"].is_array()) {
                for (const auto& item : nj["principal_variation"]) {
                    if (item.is_array() && item.size() == 4) {
                        Move m = {{item[0].get<int>(), item[1].get<int>()}, {item[2].get<int>(), item[3].get<int>()}};
                        principal_variation.emplace_back(std::move(m));
                    }
                }
            }

            std::function<void(const nlohmann::json&, MCTSNodeSnapshot&)> conv = [&](const nlohmann::json& node, MCTSNodeSnapshot& out) {
                if (!node.is_object()) return;
                if (node.contains("last_move") && node["last_move"].is_array() && node["last_move"].size() == 4) {
                    out.last_move = {{node["last_move"][0].get<int>(), node["last_move"][1].get<int>()}, {node["last_move"][2].get<int>(), node["last_move"][3].get<int>()}};
                }
                if (node.contains("visits")) out.visits = node["visits"].get<int>();
                if (node.contains("wins")) out.wins = node["wins"].get<double>();
                if (node.contains("ucb")) out.ucb_value = node["ucb"].get<double>();
                if (node.contains("is_terminal")) out.is_terminal = node["is_terminal"].get<bool>();
                out.children.clear();
                if (node.contains("children") && node["children"].is_array()) {
                    for (const auto& child_json : node["children"]) {
                        MCTSNodeSnapshot child;
                        conv(child_json, child);
                        out.children.emplace_back(std::move(child));
                    }
                }
            };

            if (nj.contains("root_node")) {
                conv(nj["root_node"], root_node);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "MCTSSnapshot::FromString parse error: " << e.what() << "\n";
    }
#else
    // Fallback to existing ein_json-based parsing
    try {
        Value root = ein_json::parse(data);
        if (root.is_object()) {
            auto obj = root.as_object();
            if (obj.count("total_iterations") != 0) { total_iterations = obj.at("total_iterations").as_int(); }
            if (obj.count("completed_iterations") != 0) { completed_iterations = obj.at("completed_iterations").as_int(); }
            if (obj.count("exploration_constant") != 0) { exploration_constant = obj.at("exploration_constant").as_double(); }
            if (obj.count("time_limit") != 0) { time_limit = obj.at("time_limit").as_double(); }
            if (obj.count("elapsed_time") != 0) { elapsed_time = obj.at("elapsed_time").as_double(); }
            if (obj.count("best_evaluation") != 0) { best_evaluation = obj.at("best_evaluation").as_double(); }
            if (obj.count("nodes_created") != 0) { nodes_created = obj.at("nodes_created").as_int(); }
            if (obj.count("nodes_expanded") != 0) { nodes_expanded = obj.at("nodes_expanded").as_int(); }
            if (obj.count("avg_simulation_time") != 0) { avg_simulation_time = obj.at("avg_simulation_time").as_double(); }

            principal_variation.clear();
            if (obj.count("principal_variation") && obj.at("principal_variation").is_array()) {
                for (const auto& item : obj.at("principal_variation").as_array()) {
                    if (item.is_array()) {
                        const auto& arr = item.as_array();
                        if (arr.size() == 4) {
                            Move m = {{arr[0].as_int(), arr[1].as_int()}, {arr[2].as_int(), arr[3].as_int()}};
                            principal_variation.emplace_back(std::move(m));
                        }
                    }
                }
            }

            if (obj.count("root_node")) {
                std::function<void(const Value&, MCTSNodeSnapshot&)> conv;
                conv = [&](const Value& nj, MCTSNodeSnapshot& out) {
                    if (nj.is_object()) {
                        auto nobj = nj.as_object();
                        if (nobj.count("last_move") != 0 && nobj.at("last_move").is_array()) {
                            auto lastMoveArr = nobj.at("last_move").as_array();
                            if (lastMoveArr.size() == 4) {
                                out.last_move = {{lastMoveArr[0].as_int(), lastMoveArr[1].as_int()}, {lastMoveArr[2].as_int(), lastMoveArr[3].as_int()}};
                            }
                        }
                        if (nobj.count("visits") != 0) { out.visits = nobj.at("visits").as_int(); }
                        if (nobj.count("wins") != 0) { out.wins = nobj.at("wins").as_double(); }
                        if (nobj.count("ucb") != 0) { out.ucb_value = nobj.at("ucb").as_double(); }
                        if (nobj.count("is_terminal") != 0) { out.is_terminal = nobj.at("is_terminal").as_int() != 0; }
                        out.children.clear();
                        for (const auto& kv : nobj) {
                            if (kv.first == "children") {
                                const Value& maybeChildren = kv.second;
                                if (maybeChildren.is_array()) {
                                    auto childrenArr = maybeChildren.as_array();
                                    for (const auto& elem : childrenArr) {
                                        MCTSNodeSnapshot childNode;
                                        conv(elem, childNode);
                                        out.children.emplace_back(std::move(childNode));
                                    }
                                }
                            }
                        }
                    }
                };
                conv(obj.at("root_node"), root_node);
            }
        }
    } catch (const std::exception& e) {
        (void)e;
    }
#endif
}
// Load a snapshot into a live GameState using setters on GameState
bool SnapshotManager::LoadSnapshot(const std::string& snapshot_id, GameState& game_state) {
    GameSnapshot snapshot;
    if (!snapshot.LoadFromFile(GetSnapshotFilename(snapshot_id))) {
        return false;
    }
    // This requires GameState to expose setters (added to GameState.h)
    // Note: We only update core fields here; callbacks are not preserved
    try {
        // Apply snapshot fields to the provided GameState using the new setters
        game_state.SetBoard(snapshot.board);
        game_state.SetCurrentPlayer(snapshot.current_player);
        game_state.SetCurrentDice(snapshot.current_dice);
        game_state.SetGameMode(snapshot.game_mode);
        game_state.SetGameResult(snapshot.game_result);
        game_state.SetMoveHistory(snapshot.move_history);
        game_state.SetCurrentMoveIndex(snapshot.current_move_index);
        // Note: we intentionally do not reattach callbacks or AI pointers here
    } catch (...) {
        // Fallback: do nothing
    }
    return true;
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
                snapshots.emplace_back(std::move(filename));
            }
        }
    } catch (const std::exception& e) {
        // Directory doesn't exist or other error
    }
    
    // Sort by modification time (newest first)
    std::sort(snapshots.begin(), snapshots.end(), std::greater<>());
    
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

// Minimal SnapshotManager constructor and CreateSnapshot implementation
SnapshotManager::SnapshotManager()
    : snapshots_dir_("snapshots"), latest_snapshot_id_(), auto_save_enabled_(false), auto_save_interval_(1), move_counter_(0) {
    EnsureSnapshotsDirectory();
}

std::string SnapshotManager::CreateSnapshot(const GameState& game_state, const std::string& phase, const AIThinkingSnapshot* ai_thinking) {
    GameSnapshot snapshot;
    snapshot.UpdateFromGameState(game_state);
    if (ai_thinking != nullptr) { snapshot.ai_thinking = *ai_thinking; }
    std::string id = GenerateSnapshotId();
    snapshot.snapshot_id = id;
    if (snapshot.SaveToFile(GetSnapshotFilename(id))) {
        latest_snapshot_id_ = id;
        return id;
    }
    return std::string();
}

std::string SnapshotManager::GenerateSnapshotId() const {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_MSC_VER)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    
    // Add some randomness to avoid collisions
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(SNAPSHOT_RANDOM_MIN, SNAPSHOT_RANDOM_MAX);
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
    : verbose_(true), step_delay_ms_(DEFAULT_STEP_DELAY_MS), max_steps_(DEFAULT_MAX_STEPS), ai_thinking_time_(DEFAULT_AI_THINKING_TIME), mcts_iterations_(DEFAULT_MCTS_ITERATIONS) {}

int SnapshotGameRunner::RunFromSnapshot(const std::string& snapshot_id) {
    std::string actual_snapshot_id = snapshot_id.empty() ? 
        snapshot_manager_.GetLatestSnapshotId() : snapshot_id;
    
    if (actual_snapshot_id.empty()) {
        LogDebug("No snapshots found, creating new game");
        
        // Create new game
        GameState game_state;
        game_state.NewGame(GameMode::AI_VS_AI, true);
    actual_snapshot_id = snapshot_manager_.CreateSnapshot(game_state, "initial", nullptr);
        
        if (actual_snapshot_id.empty()) {
            LogDebug("Failed to create initial snapshot");
            return 1;
        }
    }
    
    current_snapshot_id_ = actual_snapshot_id;
    LogDebug("Running from snapshot: " + actual_snapshot_id);
    
    GameSnapshot snapshot = snapshot_manager_.GetSnapshot(actual_snapshot_id);
    LogDebug("Loaded snapshot: " + snapshot.GetSnapshotSummary());
    
    // Create game state from snapshot (attempt to load snapshot into game_state)
    GameState game_state;
    if (!snapshot_manager_.LoadSnapshot(actual_snapshot_id, game_state)) {
        LogDebug("Failed to load snapshot into game state; initializing new game instead");
        game_state.NewGame(snapshot.game_mode, true);
    }
    
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
        // If we have AI thinking info from last step, include it; otherwise pass nullptr
        const AIThinkingSnapshot* ai_ptr = last_ai_thinking_.has_mcts_data ? &last_ai_thinking_ : nullptr;
        std::string new_snapshot_id = snapshot_manager_.CreateSnapshot(game_state, "step_" + std::to_string(steps), ai_ptr);
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
        LogDebug("No valid moves available for dice=" + std::to_string(dice) + ", performing SkipTurn() to advance state");
        // Advance turn properly so dice changes and game progresses
        game_state.SkipTurn();
        // Clear last AI thinking since no AI ran this turn
        last_ai_thinking_ = AIThinkingSnapshot();
        return true; // Not a failure; just a skipped turn
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
    
    // Save AI thinking for snapshot saving and debugging
    last_ai_thinking_ = ai_thinking;

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
        return Move{{-1, -1}, {-1, -1}};
    }
    // Use real MCTS instance
    AIConfig config;
    config.mcts_iterations = mcts_iterations_;
    config.thinking_time = ai_thinking_time_;
    config.enable_multithreading = true;
    config.thread_count = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) / 2);

    MCTS mcts(config);
    mcts.EnableTreePersistence(true);

    auto start = std::chrono::steady_clock::now();
    Move best_move = mcts.FindBestMove(board, player, dice);
    auto end = std::chrono::steady_clock::now();

    ai_thinking.mcts_iterations = mcts.GetIterationsPerformed();
    ai_thinking.thinking_time = std::chrono::duration<double>(end - start).count();
    ai_thinking.nodes_explored = ai_thinking.mcts_iterations; // approximate
    ai_thinking.position_evaluation = mcts.EvaluatePosition(board, player);
    ai_thinking.best_move_reasoning = "MCTS selection";

    // Export trimmed tree (depth 2, top 3 children)
    const int kExportDepthLocal = 2;
    const int kExportWidthLocal = 3;
    auto export_root = mcts.ExportSearchTree(kExportDepthLocal, kExportWidthLocal);
    // Populate MCTSSnapshot root node
    MCTSSnapshot snapshot;
    snapshot.total_iterations = ai_thinking.mcts_iterations;
    snapshot.time_limit = config.thinking_time;
    snapshot.elapsed_time = ai_thinking.thinking_time;
    snapshot.best_evaluation = ai_thinking.position_evaluation;
    snapshot.nodes_created = 0;

    // Convert export_root into MCTSNodeSnapshot structure
    MCTSNodeSnapshot root_node;
    root_node.position = {-1, -1};
    root_node.player = player;
    root_node.dice_value = dice;
    root_node.last_move = export_root.move;
    root_node.visits = export_root.visits;
    root_node.wins = export_root.win_rate;
    root_node.ucb_value = export_root.ucb;
    root_node.is_fully_expanded = export_root.terminal;
    root_node.is_terminal = export_root.terminal;

    // children (top 3) and all their grandchildren
    for (const auto& child_export : export_root.children) {
        MCTSNodeSnapshot child_node;
        child_node.last_move = child_export.move;
        child_node.visits = child_export.visits;
        child_node.wins = child_export.win_rate;
        child_node.ucb_value = child_export.ucb;
        child_node.is_terminal = child_export.terminal;
        // grandchildren
        for (const auto& grand_export : child_export.children) {
            MCTSNodeSnapshot grand;
            grand.last_move = grand_export.move;
            grand.visits = grand_export.visits;
            grand.wins = grand_export.win_rate;
            grand.ucb_value = grand_export.ucb;
            grand.is_terminal = grand_export.terminal;
            child_node.children.emplace_back(std::move(grand));
        }
        root_node.children.emplace_back(std::move(child_node));
    }

    snapshot.root_node = root_node;
    ai_thinking.mcts_state = snapshot;
    ai_thinking.has_mcts_data = true;

    if (best_move.first.first == -1) {
        return valid_moves[0];
    }
    return best_move;
}

void SnapshotGameRunner::RenderGameState(const GameState& game_state) const {
    CLIRenderer renderer;
    renderer.SetColors(true);
    renderer.SetVerbose(verbose_);
    renderer.RenderGame(game_state);
    
    // Show snapshot info
    std::cout << "\n[SNAPSHOT] Current: " << current_snapshot_id_ << '\n';
    std::cout << "[DEBUG] " << debug_log_.size() << " debug messages" << '\n';
}

void SnapshotGameRunner::LogDebug(const std::string& message) {
    debug_log_.emplace_back(message);
    if (verbose_) {
        std::cout << "[DEBUG] " << message << '\n';
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

auto SnapshotGameRunner::GetDebugInfo() const -> std::vector<std::string> {
    return debug_log_;
}

void SnapshotGameRunner::DumpAnalysis(const std::string& filename) const {
    const std::string actualFilename = filename.empty() ? "game_analysis.txt" : filename;

    std::ofstream file(actualFilename);
    if (!file.is_open()) {
        std::cout << "[DEBUG] Failed to open analysis file: " + actualFilename << '\n';
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
    for (const auto& snapshotId : snapshots) {
        const GameSnapshot snapshot = snapshot_manager_.GetSnapshot(snapshotId);
        file << snapshot.GetSnapshotSummary() << "\n";
    }

    std::cout << "[DEBUG] Analysis saved to: " + actualFilename << '\n';
}

// MCTS Snapshot helper methods - simplified version
auto SnapshotGameRunner::CaptureMCTSStateForDebug() const -> MCTSSnapshot {
    MCTSSnapshot snapshot;
    
    // Fill with default/demo values for now
    snapshot.exploration_constant = DEFAULT_MCTS_EXPLORATION_CONST;
    snapshot.total_iterations = mcts_iterations_;
    snapshot.completed_iterations = 0;
    snapshot.time_limit = ai_thinking_time_;
    snapshot.elapsed_time = 0.0;
    snapshot.best_evaluation = 0.0;
    snapshot.nodes_created = 0;
    snapshot.nodes_expanded = 0;
    snapshot.avg_simulation_time = 0.0;
    
    // Add debug info
    snapshot.search_debug_info.emplace_back("MCTS state captured for debugging");
    snapshot.search_debug_info.emplace_back("Using " + std::to_string(mcts_iterations_) + " iterations");
    
    return snapshot;
}

// Export live MCTS state into MCTSSnapshot if an MCTS instance is attached
auto SnapshotGameRunner::CaptureMCTSState(MCTSSnapshot& mcts_snapshot) const -> bool {
    if (mcts_instance_ == nullptr) {
        return false;
    }
    // Export a moderately deep tree for debugging
    const int kExportDepth = DEFAULT_MCTS_EXPORT_DEPTH; // levels deep
    const int kExportWidth = DEFAULT_MCTS_EXPORT_WIDTH; // top children per node
    auto exportNode = mcts_instance_->ExportSearchTree(kExportDepth, kExportWidth);
    // Fill snapshot minimally
    mcts_snapshot.total_iterations = mcts_instance_->GetIterationsPerformed();
    mcts_snapshot.completed_iterations = mcts_instance_->GetIterationsPerformed();
    mcts_snapshot.time_limit = ai_thinking_time_;
    mcts_snapshot.elapsed_time = mcts_instance_->GetLastSearchTime();
    mcts_snapshot.best_evaluation = 0.0;
    // Convert export_node recursively into MCTSNodeSnapshot structure
    std::function<void(const MCTS::ExportNode&, MCTSNodeSnapshot&)> conv;
    conv = [&](const MCTS::ExportNode& exportNodeIn, MCTSNodeSnapshot& out) -> void {
        out.last_move = exportNodeIn.move;
        out.visits = exportNodeIn.visits;
        out.wins = exportNodeIn.win_rate;
        out.ucb_value = exportNodeIn.ucb;
        out.is_terminal = exportNodeIn.terminal;
        out.children.clear();
        out.children.reserve(exportNodeIn.children.size());
        for (const auto& childExport : exportNodeIn.children) {
            MCTSNodeSnapshot child;
            conv(childExport, child);
            out.children.emplace_back(std::move(child));
        }
    };
    conv(exportNode, mcts_snapshot.root_node);
    return true;
}

// Restore an MCTS snapshot into the attached MCTS instance (best-effort)
auto SnapshotGameRunner::RestoreMCTSState(const MCTSSnapshot& mcts_snapshot) -> bool {
    if (mcts_instance_ == nullptr) {
        return false;
    }
    // Convert MCTSSnapshot.root_node into ExportNode for import
    std::function<MCTS::ExportNode(const MCTSNodeSnapshot&)> conv;
    conv = [&](const MCTSNodeSnapshot& nodeIn) -> MCTS::ExportNode {
        MCTS::ExportNode out;
        out.move = nodeIn.last_move;
        out.visits = nodeIn.visits;
        out.win_rate = nodeIn.wins;
        out.ucb = nodeIn.ucb_value;
        out.terminal = nodeIn.is_terminal;
        out.children.reserve(nodeIn.children.size());
        for (const auto& childSnapshot : nodeIn.children) {
            out.children.emplace_back(conv(childSnapshot));
        }
        return out;
    };

    MCTS::ExportNode rootExport = conv(mcts_snapshot.root_node);
    mcts_instance_->EnableTreePersistence(true);
    mcts_instance_->ImportSearchTree(rootExport);
    return true;
}

auto SnapshotGameRunner::RunWithMCTSSnapshots(int max_moves) -> bool {
    std::cout << "Starting game with MCTS snapshots enabled...\n";
    std::cout << "Max moves: " << max_moves << "\n";
    
    // Simple demonstration of MCTS snapshot functionality
    MCTSSnapshot demoSnapshot = CaptureMCTSStateForDebug();
    std::cout << "MCTS Snapshot created:\n" << demoSnapshot.GetSearchSummary() << "\n";
    
    std::cout << "MCTS snapshot integration successful!\n";
    std::cout << "This feature allows capturing complete MCTS tree state for debugging.\n";
    
    return true;
}

auto SnapshotGameRunner::CaptureState() const -> GameSnapshot {
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

auto SnapshotGameRunner::SaveSnapshot(const GameSnapshot& snapshot, const std::string& name) -> bool {
    const std::string snapshotId = name.empty() ? snapshot.snapshot_id : name;
    return snapshot_manager_.SaveSnapshot(snapshotId, snapshot);
}

auto SnapshotGameRunner::Initialize() -> bool {
    // Simple initialization for demo
    debug_log_.clear();
    return true;
}

} // namespace Einstein
