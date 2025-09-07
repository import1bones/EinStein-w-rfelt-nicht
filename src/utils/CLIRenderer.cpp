#include "utils/CLIRenderer.h"
#include "utils/Logger.h"
#include "ai/MCTS.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <fstream>

namespace Einstein {

// Color constants
const std::string CLIRenderer::RESET = "\033[0m";
const std::string CLIRenderer::RED = "\033[31m";
const std::string CLIRenderer::GREEN = "\033[32m";
const std::string CLIRenderer::YELLOW = "\033[33m";
const std::string CLIRenderer::BLUE = "\033[34m";
const std::string CLIRenderer::MAGENTA = "\033[35m";
const std::string CLIRenderer::CYAN = "\033[36m";
const std::string CLIRenderer::WHITE = "\033[37m";
const std::string CLIRenderer::BOLD = "\033[1m";

// Background colors for highlighting
const std::string YELLOW_BG = "\033[43m";

CLIRenderer::CLIRenderer() : use_colors_(true), verbose_(false) {}

void CLIRenderer::RenderGame(const GameState& game_state) {
    Clear();
    PrintTitle("Einstein Game - CLI Mode");
    RenderGameInfo(game_state);
    std::cout << std::endl;
    
    // Get the last move for highlighting, if available
    Move last_move = {{-1, -1}, {-1, -1}};
    const GameMove* last_game_move = game_state.GetLastMove();
    if (last_game_move != nullptr) {
        last_move = last_game_move->move;
    }
    
    RenderBoard(game_state.GetBoard(), last_move);
    std::cout << std::endl;
    if (verbose_) {
        RenderMoveHistory(game_state);
        std::cout << std::endl;
    }
}

void CLIRenderer::RenderBoard(const ChessBoard& board, const Move& last_move) {
    PrintSeparator('-', 25);
    std::cout << "   ";
    for (int x = 0; x < BOARD_SIZE; ++x) {
        char col = 'a' + x;
        std::cout << " " << col << " ";
    }
    std::cout << std::endl;
    
    PrintSeparator('-', 25);
    
    for (int y = 0; y < BOARD_SIZE; ++y) {
        std::cout << " " << (y + 1) << " |";
        for (int x = 0; x < BOARD_SIZE; ++x) {
            int8_t piece = board.GetPiece(x, y);
            std::string symbol = GetPieceSymbol(piece);
            
            // Check if this position is part of the last move (from or to position)
            bool is_last_move_position = (last_move.first.first != -1) && 
                                       ((last_move.first.first == x && last_move.first.second == y) ||
                                        (last_move.second.first == x && last_move.second.second == y));
            
            if (piece > 0) {
                if (is_last_move_position) {
                    std::cout << YELLOW_BG << ColorText(symbol, GREEN) << RESET;
                } else {
                    std::cout << ColorText(symbol, GREEN);
                }
            } else if (piece < 0) {
                if (is_last_move_position) {
                    std::cout << YELLOW_BG << ColorText(symbol, RED) << RESET;
                } else {
                    std::cout << ColorText(symbol, RED);
                }
            } else {
                if (is_last_move_position) {
                    std::cout << YELLOW_BG << " . " << RESET;
                } else {
                    std::cout << " . ";
                }
            }
        }
        std::cout << "|" << std::endl;
    }
    
    PrintSeparator('-', 25);
    
    // Legend
    std::cout << "Legend: " << ColorText("Red(-)", RED) << " = Left-Top Player, " 
              << ColorText("Green(+)", GREEN) << " = Right-Bottom Player" << std::endl;
}

void CLIRenderer::RenderGameInfo(const GameState& game_state) {
    std::cout << "Game Mode: " << GetGameModeString(game_state.GetGameMode()) << std::endl;
    std::cout << "Current Player: " << ColorText(GetPlayerName(game_state.GetCurrentPlayer()), 
                                                 game_state.GetCurrentPlayer() == Player::LEFT_TOP ? RED : GREEN) << std::endl;
    std::cout << "Dice Value: " << ColorText(std::to_string(game_state.GetCurrentDice()), YELLOW) << std::endl;
    std::cout << "Move #" << game_state.GetMoveCount() << std::endl;
    std::cout << "Status: " << GetGameResultString(game_state.GetGameResult()) << std::endl;
}

void CLIRenderer::RenderMoveHistory(const GameState& game_state, int last_moves) {
    PrintTitle("Recent Moves");
    const auto& history = game_state.GetMoveHistory();
    
    if (history.empty()) {
        std::cout << "No moves yet." << std::endl;
        return;
    }
    
    int start_idx = std::max(0, static_cast<int>(history.size()) - last_moves);
    
    for (int i = start_idx; i < static_cast<int>(history.size()); ++i) {
        const auto& game_move = history[i];
        std::cout << std::setw(3) << (i + 1) << ". ";
        std::cout << ColorText(GetPlayerName(game_move.player), 
                              game_move.player == Player::LEFT_TOP ? RED : GREEN);
        std::cout << " dice:" << game_move.dice_value;
        std::cout << " " << FormatMove(game_move.move);
        if (game_move.thinking_time > 0) {
            std::cout << " (" << FormatTime(game_move.thinking_time) << ")";
        }
        std::cout << std::endl;
    }
}

void CLIRenderer::RenderAIThinking(const std::string& ai_info) {
    std::cout << ColorText("AI Thinking: ", CYAN) << ai_info << std::flush;
}

void CLIRenderer::RenderGameStatistics(const GameState& game_state) {
    PrintTitle("Game Statistics");
    const auto& stats = game_state.GetStatistics();
    
    std::cout << "Total Moves: " << stats.total_moves << std::endl;
    std::cout << "Total Time: " << FormatTime(stats.total_thinking_time) << std::endl;
    if (stats.total_moves > 0) {
        std::cout << "Avg Time/Move: " << FormatTime(stats.total_thinking_time / stats.total_moves) << std::endl;
    }
    std::cout << "MCTS Simulations: " << stats.total_mcts_iterations << std::endl;
}

Move CLIRenderer::GetMoveFromUser(const ChessBoard& board, Player player, int dice) {
    std::cout << std::endl;
    PrintInfo("Your turn, " + GetPlayerName(player) + "!");
    std::cout << "Dice rolled: " << ColorText(std::to_string(dice), YELLOW) << std::endl;
    
    // Show valid moves
    auto valid_moves = board.GetValidMoves(player, dice);
    if (valid_moves.empty()) {
        PrintWarning("No valid moves available!");
        return {{-1, -1}, {-1, -1}};
    }
    
    std::cout << "Valid moves:" << std::endl;
    for (size_t i = 0; i < valid_moves.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << FormatMove(valid_moves[i]) << std::endl;
    }
    
    while (true) {
        std::cout << "Enter move (e.g., 'a1 b2' or move number): ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        // Try to parse as move number
        try {
            int move_num = std::stoi(input);
            if (move_num >= 1 && move_num <= static_cast<int>(valid_moves.size())) {
                return valid_moves[move_num - 1];
            }
        } catch (...) {
            // Try to parse as move notation
            Move move = ParseMove(input);
            if (move.first.first != -1) {
                // Validate move
                auto it = std::find(valid_moves.begin(), valid_moves.end(), move);
                if (it != valid_moves.end()) {
                    return move;
                }
                PrintError("Invalid move: " + input);
            } else {
                PrintError("Could not parse move: " + input);
            }
        }
        
        PrintError("Please enter a valid move number (1-" + std::to_string(valid_moves.size()) + ") or move notation.");
    }
}

void CLIRenderer::ShowMenu() {
    PrintTitle("Einstein Game - Main Menu");
    std::cout << "1. Human vs AI" << std::endl;
    std::cout << "2. AI vs AI" << std::endl;
    std::cout << "3. Training Mode" << std::endl;
    std::cout << "4. Analysis Mode" << std::endl;
    std::cout << "5. Settings" << std::endl;
    std::cout << "6. Help" << std::endl;
    std::cout << "7. Exit" << std::endl;
    PrintSeparator();
}

void CLIRenderer::Clear() {
    // Clear screen for Unix-like systems
    std::cout << "\033[2J\033[H";
}

void CLIRenderer::PrintSeparator(char c, int length) {
    std::cout << std::string(length, c) << std::endl;
}

void CLIRenderer::PrintTitle(const std::string& title) {
    std::cout << ColorText(BOLD + title, CYAN) << std::endl;
    PrintSeparator('=', title.length());
}

void CLIRenderer::PrintInfo(const std::string& message) {
    std::cout << ColorText("[INFO] ", BLUE) << message << std::endl;
}

void CLIRenderer::PrintWarning(const std::string& message) {
    std::cout << ColorText("[WARN] ", YELLOW) << message << std::endl;
}

void CLIRenderer::PrintError(const std::string& message) {
    std::cout << ColorText("[ERROR] ", RED) << message << std::endl;
}

void CLIRenderer::PrintSuccess(const std::string& message) {
    std::cout << ColorText("[SUCCESS] ", GREEN) << message << std::endl;
}

std::string CLIRenderer::ColorText(const std::string& text, const std::string& color) {
    if (!use_colors_) return text;
    return color + text + RESET;
}

std::string CLIRenderer::GetPieceSymbol(int8_t piece) {
    if (piece == 0) return " . ";
    
    int abs_piece = std::abs(piece);
    std::string symbol = " " + std::to_string(abs_piece) + " ";
    return symbol;
}

std::string CLIRenderer::GetPlayerName(Player player) {
    switch (player) {
        case Player::LEFT_TOP: return "Left-Top";
        case Player::RIGHT_BOTTOM: return "Right-Bottom";
        default: return "None";
    }
}

std::string CLIRenderer::GetGameModeString(GameMode mode) {
    switch (mode) {
        case GameMode::HUMAN_VS_AI: return "Human vs AI";
        case GameMode::AI_VS_AI: return "AI vs AI";
        case GameMode::HUMAN_VS_HUMAN: return "Human vs Human";
        default: return "Unknown";
    }
}

std::string CLIRenderer::GetGameResultString(GameResult result) {
    switch (result) {
        case GameResult::ONGOING: return ColorText("In Progress", YELLOW);
        case GameResult::LT_WINS: return ColorText("Left-Top Wins!", RED);
        case GameResult::RB_WINS: return ColorText("Right-Bottom Wins!", GREEN);
        case GameResult::DRAW: return ColorText("Draw", BLUE);
        default: return "Unknown";
    }
}

std::string CLIRenderer::FormatTime(double seconds) {
    if (seconds < 1.0) {
        return std::to_string(static_cast<int>(seconds * 1000)) + "ms";
    } else {
        return std::to_string(seconds) + "s";
    }
}

std::string CLIRenderer::FormatMove(const Move& move) {
    return FormatPosition(move.first) + " -> " + FormatPosition(move.second);
}

std::string CLIRenderer::FormatPosition(const Position& pos) {
    char col = 'a' + pos.first;
    return std::string(1, col) + std::to_string(pos.second + 1);
}

Position CLIRenderer::ParsePosition(const std::string& input) {
    if (input.length() < 2) return {-1, -1};
    
    char col = std::tolower(input[0]);
    if (col < 'a' || col > 'e') return {-1, -1};
    
    int row = input[1] - '1';
    if (row < 0 || row >= BOARD_SIZE) return {-1, -1};
    
    return {col - 'a', row};
}

Move CLIRenderer::ParseMove(const std::string& input) {
    // Expected format: "a1 b2" or "a1-b2" or "a1b2"
    std::string cleaned = input;
    std::replace(cleaned.begin(), cleaned.end(), '-', ' ');
    
    std::istringstream iss(cleaned);
    std::string from_str, to_str;
    
    if (cleaned.find(' ') != std::string::npos) {
        iss >> from_str >> to_str;
    } else if (cleaned.length() == 4) {
        from_str = cleaned.substr(0, 2);
        to_str = cleaned.substr(2, 2);
    } else {
        return {{-1, -1}, {-1, -1}};
    }
    
    Position from = ParsePosition(from_str);
    Position to = ParsePosition(to_str);
    
    if (from.first == -1 || to.first == -1) {
        return {{-1, -1}, {-1, -1}};
    }
    
    return {from, to};
}

void CLIRenderer::Pause(const std::string& message) {
    std::cout << message;
    std::cin.ignore();
}

bool CLIRenderer::GetYesNoInput(const std::string& prompt) {
    while (true) {
        std::cout << prompt << " (y/n): ";
        std::string input;
        std::getline(std::cin, input);
        
        if (!input.empty()) {
            char c = std::tolower(input[0]);
            if (c == 'y') return true;
            if (c == 'n') return false;
        }
        
        PrintError("Please enter 'y' for yes or 'n' for no.");
    }
}

std::string CLIRenderer::GetStringInput(const std::string& prompt) {
    std::cout << prompt << ": ";
    std::string input;
    std::getline(std::cin, input);
    return input;
}

// CLIGameController implementation
CLIGameController::CLIGameController() 
    : verbose_(true), ai_difficulty_(3), time_limit_(5.0) {}

int CLIGameController::RunInteractiveGame() {
    renderer_.Clear();
    renderer_.ShowMenu();
    
    while (true) {
        std::string choice = renderer_.GetStringInput("Select option (1-7)");
        
        if (choice == "1") {
            return RunHumanvsAI();
        } else if (choice == "2") {
            int games = 1;
            if (renderer_.GetYesNoInput("Multiple games?")) {
                std::string games_str = renderer_.GetStringInput("Number of games");
                try {
                    games = std::stoi(games_str);
                } catch (...) {
                    games = 1;
                }
            }
            return RunAIvsAI(games, verbose_);
        } else if (choice == "3") {
            int games = 100;
            std::string games_str = renderer_.GetStringInput("Number of training games (default 100)");
            if (!games_str.empty()) {
                try {
                    games = std::stoi(games_str);
                } catch (...) {
                    games = 100;
                }
            }
            return RunTrainingMode(games);
        } else if (choice == "4") {
            std::string file = renderer_.GetStringInput("Game file to analyze (empty for new analysis)");
            return RunAnalysisMode(file);
        } else if (choice == "5") {
            // Settings menu
            ShowSimpleSettingsMenu();
        } else if (choice == "6") {
            ShowHelp();
        } else if (choice == "7") {
            renderer_.PrintInfo("Goodbye!");
            return 0;
        } else {
            renderer_.PrintError("Invalid choice. Please select 1-7.");
        }
    }
}

int CLIGameController::RunHumanvsAI(bool human_starts) {
    GameState game_state;
    game_state.NewGame(GameMode::HUMAN_VS_AI, human_starts);
    
    renderer_.PrintTitle("Human vs AI Game");
    renderer_.PrintInfo(std::string("You are ") + (human_starts ? "Left-Top (Red)" : "Right-Bottom (Green)"));
    renderer_.Pause();
    
    while (game_state.GetGameResult() == GameResult::ONGOING) {
        if (!ProcessGameLoop(game_state)) {
            break;
        }
    }
    
    ShowGameResult(game_state);
    return 0;
}

int CLIGameController::RunAIvsAI(int games, bool verbose) {
    renderer_.PrintTitle("AI vs AI Mode");
    renderer_.PrintInfo("Running " + std::to_string(games) + " games...");
    
    int ai1_wins = 0, ai2_wins = 0, draws = 0;
    
    for (int i = 0; i < games; ++i) {
        GameState game_state;
        game_state.NewGame(GameMode::AI_VS_AI, i % 2 == 0);
        
        if (verbose && games == 1) {
            renderer_.PrintInfo("Game " + std::to_string(i + 1) + "/" + std::to_string(games));
        }
        
        while (game_state.GetGameResult() == GameResult::ONGOING) {
            if (verbose && games == 1) {
                renderer_.RenderGame(game_state);
                ProcessAITurn(game_state);
                renderer_.Pause("Press Enter for next move...");
            } else {
                ProcessAITurn(game_state);
            }
        }
        
        // Count results
        switch (game_state.GetGameResult()) {
            case GameResult::LT_WINS: ai1_wins++; break;
            case GameResult::RB_WINS: ai2_wins++; break;
            case GameResult::DRAW: draws++; break;
            default: break;
        }
        
        if (games > 1 && (i + 1) % 10 == 0) {
            renderer_.PrintInfo("Progress: " + std::to_string(i + 1) + "/" + std::to_string(games));
        }
    }
    
    // Show final results
    renderer_.PrintTitle("AI vs AI Results");
    std::cout << "AI 1 (Left-Top) wins: " << ai1_wins << std::endl;
    std::cout << "AI 2 (Right-Bottom) wins: " << ai2_wins << std::endl;
    std::cout << "Draws: " << draws << std::endl;
    std::cout << "Total games: " << games << std::endl;
    
    return 0;
}

bool CLIGameController::ProcessGameLoop(GameState& game_state) {
    renderer_.RenderGame(game_state);
    
    Player current_player = game_state.GetCurrentPlayer();
    bool is_human = (game_state.GetGameMode() == GameMode::HUMAN_VS_AI && 
                     current_player == Player::LEFT_TOP);
    
    if (is_human) {
        return ProcessHumanTurn(game_state);
    } else {
        return ProcessAITurn(game_state);
    }
}

bool CLIGameController::ProcessHumanTurn(GameState& game_state) {
    const ChessBoard& board = game_state.GetBoard();
    Player player = game_state.GetCurrentPlayer();
    int dice = game_state.GetCurrentDice();
    
    Move move = renderer_.GetMoveFromUser(board, player, dice);
    if (move.first.first == -1) {
        renderer_.PrintWarning("No valid moves - turn skipped");
        Logger::Instance().Info("Human player " + std::to_string(static_cast<int>(player)) + " skipped turn (no valid moves)");
        game_state.SkipTurn();
        return true;
    }
    
    if (game_state.MakeMove(move)) {
        std::string move_str = renderer_.FormatMove(move);
        renderer_.PrintSuccess("Move executed: " + move_str);
        Logger::Instance().Info("Human player " + std::to_string(static_cast<int>(player)) + " moved: " + move_str);
        return true;
    } else {
        renderer_.PrintError("Failed to execute move");
        Logger::Instance().Error("Human player " + std::to_string(static_cast<int>(player)) + " failed to execute move: " + renderer_.FormatMove(move));
        return false;
    }
}

bool CLIGameController::ProcessAITurn(GameState& game_state) {
    renderer_.PrintInfo("AI is thinking...");
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Use real MCTS AI instead of simulation
    Move ai_move = GetAIMove(game_state);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    double thinking_time = std::chrono::duration<double>(end_time - start_time).count();
    
    if (ai_move.first.first == -1) {
        renderer_.PrintWarning("AI has no valid moves - turn skipped");
        Logger::Instance().Info("AI player " + std::to_string(static_cast<int>(game_state.GetCurrentPlayer())) + " skipped turn (no valid moves)");
        game_state.SkipTurn();
        return true;
    }
    
    if (game_state.MakeMove(ai_move)) {
        std::string move_str = renderer_.FormatMove(ai_move);
        std::string time_str = renderer_.FormatTime(thinking_time);
        renderer_.PrintSuccess("AI move: " + move_str + " (" + time_str + ")");
        Logger::Instance().Info("AI player " + std::to_string(static_cast<int>(game_state.GetCurrentPlayer())) + " moved: " + move_str + " (thinking time: " + time_str + ")");
        return true;
    } else {
        renderer_.PrintError("AI failed to execute move");
        Logger::Instance().Error("AI player " + std::to_string(static_cast<int>(game_state.GetCurrentPlayer())) + " failed to execute move: " + renderer_.FormatMove(ai_move));
        return false;
    }
}

Move CLIGameController::GetAIMove(const GameState& game_state) {
    const ChessBoard& board = game_state.GetBoard();
    Player player = game_state.GetCurrentPlayer();
    int dice = game_state.GetCurrentDice();
    
    auto valid_moves = board.GetValidMoves(player, dice);
    if (valid_moves.empty()) {
        return {{-1, -1}, {-1, -1}};
    }
    
    // Use MCTS AI for intelligent move selection
    AIConfig ai_config;
    ai_config.mcts_iterations = 1000 * ai_difficulty_;
    ai_config.thinking_time = time_limit_;
    ai_config.exploration_constant = 1.414;
    ai_config.enable_multithreading = true;
    ai_config.thread_count = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) / 2);
    
    MCTS mcts(ai_config);
    
    // Get best move from MCTS
    Move best_move = mcts.FindBestMove(board, player, dice);
    
    // Fallback to random if MCTS fails
    if (best_move.first.first == -1) {
        std::srand(std::time(nullptr));
        int random_index = std::rand() % valid_moves.size();
        return valid_moves[random_index];
    }
    
    return best_move;
}

void CLIGameController::ShowGameResult(const GameState& game_state) {
    renderer_.Clear();
    renderer_.PrintTitle("Game Over!");
    renderer_.RenderBoard(game_state.GetBoard());
    std::cout << std::endl;
    
    GameResult result = game_state.GetGameResult();
    switch (result) {
        case GameResult::LT_WINS:
            renderer_.PrintSuccess("Left-Top Player Wins!");
            break;
        case GameResult::RB_WINS:
            renderer_.PrintSuccess("Right-Bottom Player Wins!");
            break;
        case GameResult::DRAW:
            renderer_.PrintInfo("Game ended in a draw!");
            break;
        default:
            renderer_.PrintWarning("Game ended unexpectedly");
            break;
    }
    
    renderer_.RenderGameStatistics(game_state);
    renderer_.Pause();
}

int CLIGameController::RunTrainingMode(int games) {
    renderer_.PrintTitle("Training Mode");
    renderer_.PrintInfo("Running " + std::to_string(games) + " training games...");
    
    // This would implement the training loop
    // For now, just run AI vs AI games
    return RunAIvsAI(games, false);
}

int CLIGameController::RunAnalysisMode(const std::string& game_file) {
    renderer_.PrintTitle("Analysis Mode");
    if (game_file.empty()) {
        renderer_.PrintInfo("Running new game analysis...");
        return RunHumanvsAI();
    } else {
        renderer_.PrintInfo("Analyzing game file: " + game_file);
        
        // Basic file analysis
        std::ifstream file(game_file);
        if (!file.is_open()) {
            renderer_.PrintError("Cannot open file: " + game_file);
            return 1;
        }
        
        int line_count = 0;
        std::string line;
        while (std::getline(file, line)) {
            line_count++;
        }
        
        renderer_.PrintSuccess("File analysis complete:");
        renderer_.PrintInfo("Total lines: " + std::to_string(line_count));
        return 0;
    }
}

void CLIGameController::ShowHelp() {
    renderer_.PrintTitle("Einstein Game Help");
    std::cout << "Game Rules:" << std::endl;
    std::cout << "- 5x5 board with 6 pieces per player" << std::endl;
    std::cout << "- Roll dice to determine which piece can move" << std::endl;
    std::cout << "- Win by reaching opponent's corner or capturing all pieces" << std::endl;
    std::cout << std::endl;
    std::cout << "Move notation: 'a1 b2' or just select from numbered list" << std::endl;
    std::cout << "Commands: Type move or select number, 'help', 'quit'" << std::endl;
    renderer_.Pause();
}

void CLIGameController::ShowSimpleSettingsMenu() {
    renderer_.PrintTitle("Game Settings");
    renderer_.PrintInfo("Current settings:");
    renderer_.PrintInfo("- Verbose mode: " + std::string(verbose_ ? "ON" : "OFF"));
    renderer_.PrintInfo("- AI Difficulty: " + std::to_string(ai_difficulty_));
    renderer_.PrintInfo("- Time limit: " + std::to_string(time_limit_) + "s");
    renderer_.PrintInfo("Settings modification in future versions.");
    renderer_.Pause();
}

} // namespace Einstein
