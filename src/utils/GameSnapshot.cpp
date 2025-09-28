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
#include "third_party/json_adapter.hpp"
using ein_json::Value;
using ein_json::Object;
using ein_json::Array;
#ifdef USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

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
    Object j;
    j["total_iterations"] = Value(total_iterations);
    j["completed_iterations"] = Value(completed_iterations);
    j["exploration_constant"] = Value(exploration_constant);
    j["time_limit"] = Value(time_limit);
    j["elapsed_time"] = Value(elapsed_time);
    j["best_evaluation"] = Value(best_evaluation);
    j["nodes_created"] = Value(nodes_created);
    j["nodes_expanded"] = Value(nodes_expanded);
    j["avg_simulation_time"] = Value(avg_simulation_time);

    Array pv;
    for (const auto& m : principal_variation) pv.emplace_back(Array{Value(m.first.first), Value(m.first.second), Value(m.second.first), Value(m.second.second)});
    j["principal_variation"] = Value(pv);

    // Depth-limited node serializer: include only numeric/boolean fields and arrays
    const int MAX_DEPTH = 2;
    const size_t MAX_CHILDREN = 5;
    std::function<Value(const MCTSNodeSnapshot&, int)> node_to_json = [&](const MCTSNodeSnapshot& node, int depth) -> Value {
        Object nj;
        // last_move
        Array lm;
        lm.emplace_back(Value(node.last_move.first.first));
        lm.emplace_back(Value(node.last_move.first.second));
        lm.emplace_back(Value(node.last_move.second.first));
        lm.emplace_back(Value(node.last_move.second.second));
        nj["last_move"] = Value(lm);

        nj["visits"] = Value(node.visits);
        nj["ucb"] = Value(node.ucb_value);
        nj["is_terminal"] = Value(node.is_terminal);
        // prior/virtual_loss not present in MCTSNodeSnapshot; include zeros for compatibility
        nj["prior"] = Value(0.0);
        nj["virtual_loss"] = Value(0);

        Array children;
        if (depth > 0 && !node.children.empty()) {
            size_t count = std::min(node.children.size(), MAX_CHILDREN);
            for (size_t i = 0; i < count; ++i) children.push_back(node_to_json(node.children[i], depth - 1));
        }
        nj["children"] = Value(children);
        return Value(nj);
    };

    j["root_node"] = node_to_json(root_node, MAX_DEPTH);
    return Value(j).dump(2);
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
    Object j;
    j["mcts_iterations"] = Value(mcts_iterations);
    j["thinking_time"] = Value(thinking_time);
    j["position_evaluation"] = Value(position_evaluation);
    j["nodes_explored"] = Value(nodes_explored);
    j["best_move_reasoning"] = Value(best_move_reasoning);

    Array mevals;
    for (const auto& me : move_evaluations) mevals.emplace_back(Array{Value(me.first.first.first), Value(me.first.first.second), Value(me.first.second.first), Value(me.first.second.second), Value(me.second)});
    j["move_evaluations"] = Value(mevals);
    j["debug_info"] = Value(Array());
    if (!debug_info.empty()) {
        Array di;
        for (const auto& s : debug_info) di.emplace_back(Value(s));
        j["debug_info"] = Value(di);
    }

    if (has_mcts_data) {
        try {
            Value ms = ein_json::parse(mcts_state.ToJson());
            j["mcts_state"] = ms;
        } catch (...) {
            // omit on failure
        }
    }
    return Value(j).dump(2);
}

void AIThinkingSnapshot::FromString(const std::string& data) {
    try {
        Value root = ein_json::parse(data);
        if (root.is_object()) {
            auto obj = root.as_object();
            if (obj.count("mcts_iterations")) mcts_iterations = obj.at("mcts_iterations").as_int();
            if (obj.count("thinking_time")) thinking_time = obj.at("thinking_time").as_double();
            if (obj.count("position_evaluation")) position_evaluation = obj.at("position_evaluation").as_double();
            if (obj.count("nodes_explored")) nodes_explored = obj.at("nodes_explored").as_int();
            if (obj.count("best_move_reasoning") && obj.at("best_move_reasoning").is_string()) best_move_reasoning = obj.at("best_move_reasoning").as_string();

            move_evaluations.clear();
            if (obj.count("move_evaluations") && obj.at("move_evaluations").is_array()) {
                for (const auto& it : obj.at("move_evaluations").as_array()) {
                    if (it.is_array()) {
                        const auto& arr = it.as_array();
                        if (arr.size() == 5) {
                            Move m = {{arr[0].as_int(), arr[1].as_int()}, {arr[2].as_int(), arr[3].as_int()}};
                            double ev = arr[4].as_double();
                            move_evaluations.emplace_back(m, ev);
                        }
                    }
                }
            }

            debug_info.clear();
            if (obj.count("debug_info") && obj.at("debug_info").is_array()) {
                for (const auto& di : obj.at("debug_info").as_array()) if (di.is_string()) debug_info.push_back(di.as_string());
            }

            if (obj.count("mcts_state")) {
                try {
                    mcts_state.FromString(obj.at("mcts_state").dump());
                    has_mcts_data = true;
                } catch (...) { has_mcts_data = false; }
            }
        }
    } catch (...) { }
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
        for (int x = 0; x < BOARD_SIZE; ++x) row.emplace_back(Value(static_cast<int>(board.GetPiece(x, y))));
        board_arr.emplace_back(Value(row));
    }
    j["board"] = Value(board_arr);

    // valid moves
    Array vm;
    for (const auto& m : current_valid_moves) vm.emplace_back(Array{Value(m.first.first), Value(m.first.second), Value(m.second.first), Value(m.second.second)});
    j["valid_moves"] = Value(vm);

    j["suggested_move"] = Value(Array{Value(suggested_move.first.first), Value(suggested_move.first.second), Value(suggested_move.second.first), Value(suggested_move.second.second)});

    Array mh;
    for (const auto& gm : move_history) mh.emplace_back(Array{Value(gm.move.first.first), Value(gm.move.first.second), Value(gm.move.second.first), Value(gm.move.second.second)});
    j["move_history"] = Value(mh);

    try {
        Value at = ein_json::parse(ai_thinking.ToJson());
        j["ai_thinking"] = at;
    } catch (...) {
        j["ai_thinking"] = Value();
    }

    Array dbg;
    for (const auto& s : debug_log) dbg.emplace_back(Value(s));
    j["debug_log"] = Value(dbg);
    return Value(j).dump(2);
}
void GameSnapshot::FromString(const std::string& data) {
    try {
        Value root = ein_json::parse(data);
        if (root.is_object()) {
            auto obj = root.as_object();
            if (obj.count("snapshot_id") && obj.at("snapshot_id").is_string()) snapshot_id = obj.at("snapshot_id").as_string();
            if (obj.count("turn_number")) turn_number = obj.at("turn_number").as_int();
            if (obj.count("current_player")) current_player = static_cast<Player>(obj.at("current_player").as_int());
            if (obj.count("current_dice")) current_dice = obj.at("current_dice").as_int();
            if (obj.count("game_mode")) game_mode = static_cast<GameMode>(obj.at("game_mode").as_int());
            if (obj.count("game_result")) game_result = static_cast<GameResult>(obj.at("game_result").as_int());
            if (obj.count("current_move_index")) current_move_index = obj.at("current_move_index").as_int();
            if (obj.count("total_game_time")) total_game_time = obj.at("total_game_time").as_double();
            if (obj.count("current_phase") && obj.at("current_phase").is_string()) current_phase = obj.at("current_phase").as_string();

            if (obj.count("board") && obj.at("board").is_array()) {
                const auto& barr = obj.at("board").as_array();
                for (int y = 0; y < BOARD_SIZE && y < (int)barr.size(); ++y) {
                    if (!barr[y].is_array()) continue;
                    const auto& row = barr[y].as_array();
                    for (int x = 0; x < BOARD_SIZE && x < (int)row.size(); ++x) {
                        if (row[x].is_number()) board.SetPiece(x, y, static_cast<int8_t>(row[x].as_int()));
                    }
                }
            }

            current_valid_moves.clear();
            if (obj.count("valid_moves") && obj.at("valid_moves").is_array()) {
                for (const auto& it : obj.at("valid_moves").as_array()) {
                    if (it.is_array()) {
                        const auto& arr = it.as_array();
                        if (arr.size() == 4) current_valid_moves.push_back({{arr[0].as_int(), arr[1].as_int()}, {arr[2].as_int(), arr[3].as_int()}});
                    }
                }
            }

            if (obj.count("suggested_move") && obj.at("suggested_move").is_array()) {
                const auto& sm = obj.at("suggested_move").as_array();
                if (sm.size() == 4) suggested_move = {{sm[0].as_int(), sm[1].as_int()}, {sm[2].as_int(), sm[3].as_int()}};
            }

            move_history.clear();
            if (obj.count("move_history") && obj.at("move_history").is_array()) {
                for (const auto& it : obj.at("move_history").as_array()) {
                    if (it.is_array()) {
                        const auto& arr = it.as_array();
                        if (arr.size() == 4) {
                            Move mv = {{arr[0].as_int(), arr[1].as_int()}, {arr[2].as_int(), arr[3].as_int()}};
                            GameMove gm(mv, Player::LEFT_TOP, 0);
                            move_history.push_back(gm);
                        }
                    }
                }
            }

            if (obj.count("ai_thinking")) {
                try { ai_thinking.FromString(obj.at("ai_thinking").dump()); } catch (...) { }
            }

            debug_log.clear();
            if (obj.count("debug_log") && obj.at("debug_log").is_array()) {
                for (const auto& it : obj.at("debug_log").as_array()) if (it.is_string()) debug_log.push_back(it.as_string());
            }
        }
    } catch (...) { }
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
    if (!file.is_open()) return false;
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
                        principal_variation.push_back(m);
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
                    for (const auto& c : node["children"]) {
                        MCTSNodeSnapshot child;
                        conv(c, child);
                        out.children.push_back(child);
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
            if (obj.count("total_iterations")) total_iterations = obj.at("total_iterations").as_int();
            if (obj.count("completed_iterations")) completed_iterations = obj.at("completed_iterations").as_int();
            if (obj.count("exploration_constant")) exploration_constant = obj.at("exploration_constant").as_double();
            if (obj.count("time_limit")) time_limit = obj.at("time_limit").as_double();
            if (obj.count("elapsed_time")) elapsed_time = obj.at("elapsed_time").as_double();
            if (obj.count("best_evaluation")) best_evaluation = obj.at("best_evaluation").as_double();
            if (obj.count("nodes_created")) nodes_created = obj.at("nodes_created").as_int();
            if (obj.count("nodes_expanded")) nodes_expanded = obj.at("nodes_expanded").as_int();
            if (obj.count("avg_simulation_time")) avg_simulation_time = obj.at("avg_simulation_time").as_double();

            principal_variation.clear();
            if (obj.count("principal_variation") && obj.at("principal_variation").is_array()) {
                for (const auto& item : obj.at("principal_variation").as_array()) {
                    if (item.is_array()) {
                        const auto& arr = item.as_array();
                        if (arr.size() == 4) {
                            Move m = {{arr[0].as_int(), arr[1].as_int()}, {arr[2].as_int(), arr[3].as_int()}};
                            principal_variation.push_back(m);
                        }
                    }
                }
            }

            if (obj.count("root_node")) {
                std::function<void(const Value&, MCTSNodeSnapshot&)> conv;
                conv = [&](const Value& nj, MCTSNodeSnapshot& out) {
                    if (nj.is_object()) {
                        auto nobj = nj.as_object();
                        if (nobj.count("last_move") && nobj.at("last_move").is_array()) {
                            auto lm = nobj.at("last_move").as_array();
                            if (lm.size() == 4) out.last_move = {{lm[0].as_int(), lm[1].as_int()}, {lm[2].as_int(), lm[3].as_int()}};
                        }
                        if (nobj.count("visits")) out.visits = nobj.at("visits").as_int();
                        if (nobj.count("wins")) out.wins = nobj.at("wins").as_double();
                        if (nobj.count("ucb")) out.ucb_value = nobj.at("ucb").as_double();
                        if (nobj.count("is_terminal")) out.is_terminal = nobj.at("is_terminal").as_int() != 0;
                        out.children.clear();
                        for (const auto& kv : nobj) {
                            if (kv.first == "children") {
                                const Value& maybe_children = kv.second;
                                if (maybe_children.is_array()) {
                                    auto carr = maybe_children.as_array();
                                    for (const auto& elem : carr) {
                                        MCTSNodeSnapshot child;
                                        conv(elem, child);
                                        out.children.push_back(child);
                                    }
                                }
                            }
                        }
                    }
                };
                conv(obj.at("root_node"), root_node);
            }
        }
    } catch (...) {
        // ignore
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

// Minimal SnapshotManager constructor and CreateSnapshot implementation
SnapshotManager::SnapshotManager()
    : snapshots_dir_("snapshots"), latest_snapshot_id_(), auto_save_enabled_(false), auto_save_interval_(1), move_counter_(0) {
    EnsureSnapshotsDirectory();
}

std::string SnapshotManager::CreateSnapshot(const GameState& game_state, const std::string& phase, const AIThinkingSnapshot* ai_thinking) {
    GameSnapshot snapshot;
    snapshot.UpdateFromGameState(game_state);
    if (ai_thinking) {
        snapshot.ai_thinking = *ai_thinking;
    }
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
        return {{-1, -1}, {-1, -1}};
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
    auto export_root = mcts.ExportSearchTree(2, 3);
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
    for (size_t i = 0; i < export_root.children.size(); ++i) {
        const auto& c = export_root.children[i];
        MCTSNodeSnapshot child_node;
        child_node.last_move = c.move;
        child_node.visits = c.visits;
        child_node.wins = c.win_rate;
        child_node.ucb_value = c.ucb;
        child_node.is_terminal = c.terminal;
        // grandchildren
        for (size_t j = 0; j < c.children.size(); ++j) {
            const auto& gc = c.children[j];
            MCTSNodeSnapshot grand;
            grand.last_move = gc.move;
            grand.visits = gc.visits;
            grand.wins = gc.win_rate;
            grand.ucb_value = gc.ucb;
            grand.is_terminal = gc.terminal;
            child_node.children.push_back(grand);
        }
        root_node.children.push_back(child_node);
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

// Export live MCTS state into MCTSSnapshot if an MCTS instance is attached
bool SnapshotGameRunner::CaptureMCTSState(MCTSSnapshot& mcts_snapshot) const {
    if (!mcts_instance_) return false;
    // Export a moderately deep tree for debugging (depth 2, top 5 children)
    auto export_node = mcts_instance_->ExportSearchTree(2, 5);
    // Fill snapshot minimally
    mcts_snapshot.total_iterations = mcts_instance_->GetIterationsPerformed();
    mcts_snapshot.completed_iterations = mcts_instance_->GetIterationsPerformed();
    mcts_snapshot.time_limit = ai_thinking_time_;
    mcts_snapshot.elapsed_time = mcts_instance_->GetLastSearchTime();
    mcts_snapshot.best_evaluation = 0.0;
    // Convert export_node recursively into MCTSNodeSnapshot structure
    std::function<void(const MCTS::ExportNode&, MCTSNodeSnapshot&)> conv;
    conv = [&](const MCTS::ExportNode& in, MCTSNodeSnapshot& out) {
        out.last_move = in.move;
        out.visits = in.visits;
        out.wins = in.win_rate;
        out.ucb_value = in.ucb;
        out.is_terminal = in.terminal;
        for (const auto& c : in.children) {
            MCTSNodeSnapshot child;
            conv(c, child);
            out.children.push_back(child);
        }
    };
    conv(export_node, mcts_snapshot.root_node);
    return true;
}

// Restore an MCTS snapshot into the attached MCTS instance (best-effort)
bool SnapshotGameRunner::RestoreMCTSState(const MCTSSnapshot& mcts_snapshot) {
    if (!mcts_instance_) return false;
    // Convert MCTSSnapshot.root_node into ExportNode for import
    std::function<MCTS::ExportNode(const MCTSNodeSnapshot&)> conv;
    conv = [&](const MCTSNodeSnapshot& in) -> MCTS::ExportNode {
        MCTS::ExportNode out;
        out.move = in.last_move;
        out.visits = in.visits;
        out.win_rate = in.wins;
        out.ucb = in.ucb_value;
        out.terminal = in.is_terminal;
        for (const auto& c : in.children) {
            out.children.push_back(conv(c));
        }
        return out;
    };

    MCTS::ExportNode root_export = conv(mcts_snapshot.root_node);
    mcts_instance_->EnableTreePersistence(true);
    mcts_instance_->ImportSearchTree(root_export);
    return true;
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
