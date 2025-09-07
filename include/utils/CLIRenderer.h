#pragma once

#include "core/Types.h"
#include "core/ChessBoard.h"
#include "game/GameState.h"
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace Einstein {

/**
 * CLI-based game renderer for debugging and testing without GUI
 * 
 * Features:
 * - ASCII art board visualization
 * - Move history display
 * - AI thinking process visualization
 * - Game statistics and analysis
 * - Interactive command interface
 * - Colored output support
 */
class CLIRenderer {
public:
    CLIRenderer();
    ~CLIRenderer() = default;

    // Core rendering methods
    void RenderGame(const GameState& game_state);
    void RenderBoard(const ChessBoard& board, const Move& last_move = {{-1, -1}, {-1, -1}});
    void RenderGameInfo(const GameState& game_state);
    void RenderMoveHistory(const GameState& game_state, int last_moves = 10);
    void RenderAIThinking(const std::string& ai_info);
    void RenderGameStatistics(const GameState& game_state);
    
    // Interactive CLI methods
    void ShowMenu();
    void ShowHelp();
    void ShowCommands();
    Move GetMoveFromUser(const ChessBoard& board, Player player, int dice);
    bool GetYesNoInput(const std::string& prompt);
    std::string GetStringInput(const std::string& prompt);
    
    // Utility methods
    void Clear();
    void SetColors(bool enabled) { use_colors_ = enabled; }
    void SetVerbose(bool enabled) { verbose_ = enabled; }
    void Pause(const std::string& message = "Press Enter to continue...");
    
    // Output formatting
    void PrintSeparator(char c = '=', int length = 60);
    void PrintTitle(const std::string& title);
    void PrintInfo(const std::string& message);
    void PrintWarning(const std::string& message);
    void PrintError(const std::string& message);
    void PrintSuccess(const std::string& message);
    
    // Public formatting helpers
    std::string FormatTime(double seconds);
    std::string FormatMove(const Move& move);
    std::string FormatPosition(const Position& pos);

private:
    bool use_colors_;
    bool verbose_;
    
    // Color codes
    static const std::string RESET;
    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string MAGENTA;
    static const std::string CYAN;
    static const std::string WHITE;
    static const std::string BOLD;
    
    // Helper methods
    std::string ColorText(const std::string& text, const std::string& color);
    std::string GetPieceSymbol(int8_t piece);
    std::string GetPlayerName(Player player);
    std::string GetGameModeString(GameMode mode);
    std::string GetGameResultString(GameResult result);
    
    // Board rendering helpers
    void RenderBoardHeader();
    void RenderBoardRow(const ChessBoard& board, int row);
    void RenderBoardFooter();
    void RenderCoordinates();
    
    // Input validation
    bool IsValidInput(const std::string& input);
    Position ParsePosition(const std::string& input);
    Move ParseMove(const std::string& input);
};

/**
 * Interactive CLI Game Controller
 * Handles the complete game flow with CLI rendering
 */
class CLIGameController {
public:
    CLIGameController();
    ~CLIGameController() = default;
    
    // Main game modes
    int RunInteractiveGame();
    int RunAIvsAI(int games = 1, bool verbose = true);
    int RunHumanvsAI(bool human_starts = true);
    int RunTrainingMode(int games = 100);
    int RunAnalysisMode(const std::string& game_file = "");
    
    // Configuration
    void SetAIDifficulty(int level) { ai_difficulty_ = level; }
    void SetTimeLimit(double seconds) { time_limit_ = seconds; }
    void SetVerbose(bool enabled) { verbose_ = enabled; }
    
    // Help
    void ShowHelp();
    void ShowSimpleSettingsMenu();
    
private:
    CLIRenderer renderer_;
    bool verbose_;
    int ai_difficulty_;
    double time_limit_;
    
    // Game flow methods
    bool ProcessGameLoop(GameState& game_state);
    bool ProcessHumanTurn(GameState& game_state);
    bool ProcessAITurn(GameState& game_state);
    void ShowGameResult(const GameState& game_state);
    
    // AI integration methods
    Move GetAIMove(const GameState& game_state);
    std::string GetAIThinkingInfo(const GameState& game_state);
    
    // Utility methods
    void SaveGame(const GameState& game_state, const std::string& filename);
    bool LoadGame(GameState& game_state, const std::string& filename);
};

} // namespace Einstein
