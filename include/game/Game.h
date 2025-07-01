#pragma once

#include "core/Types.h"
#include "game/GameState.h"
#include <memory>
#include <string>

namespace Einstein {

// Forward declarations
class Renderer;
class InputHandler;
class MCTS;
class Config;

class Game {
public:
    Game();
    ~Game();
    
    // Game lifecycle
    bool Initialize(const std::string& config_file = "config.json");
    void Run();
    void Shutdown();
    
    // Game control
    void NewGame(GameMode mode = GameMode::HUMAN_VS_AI);
    void PauseGame();
    void ResumeGame();
    void QuitGame();
    
    // Settings
    void LoadConfig(const std::string& config_file);
    void SaveConfig(const std::string& config_file) const;
    void ApplySettings(const GameConfig& config);
    
    // Game state access
    const GameState& GetGameState() const { return *game_state_; }
    GameState& GetMutableGameState() { return *game_state_; }
    bool IsRunning() const { return running_; }
    bool IsPaused() const { return paused_; }
    
    // Events
    void OnMouseClick(int x, int y);
    void OnMouseMove(int x, int y);
    void OnKeyPress(int key);
    void OnWindowResize(int width, int height);
    
private:
    // Core systems
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<InputHandler> input_handler_;
    std::unique_ptr<GameState> game_state_;
    std::unique_ptr<MCTS> ai_;
    std::unique_ptr<Config> config_;
    
    // Game state
    bool running_;
    bool paused_;
    bool initialized_;
    
    // Configuration
    GameConfig game_config_;
    
    // Timing
    double last_frame_time_;
    double accumulated_time_;
    const double target_frame_time_ = 1.0 / 60.0; // 60 FPS
    
    // UI state
    Position selected_position_;
    std::vector<Move> highlighted_moves_;
    bool show_move_hints_;
    bool show_analysis_;
    
    // Private methods
    void Update(double delta_time);
    void Render();
    void HandleInput();
    void ProcessAITurn();
    void UpdateUI();
    
    // Event handlers
    void OnGameMove(const GameMove& move);
    void OnGameEnd(GameResult result);
    
    // UI helpers
    void RenderBoard();
    void RenderPieces();
    void RenderUI();
    void RenderMoveHints();
    void RenderAnalysis();
    void RenderGameInfo();
    
    // Input processing
    Position ScreenToBoard(int screen_x, int screen_y) const;
    bool HandleBoardClick(const Position& board_pos);
    void HandleMenuAction(const std::string& action);
    
    // Game logic helpers
    void SelectPosition(const Position& pos);
    void DeselectPosition();
    bool IsMoveValid(const Move& move) const;
    void HighlightValidMoves(const Position& from);
    void ClearHighlights();
};

} // namespace Einstein
