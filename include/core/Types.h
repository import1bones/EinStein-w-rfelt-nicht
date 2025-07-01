#pragma once

#include <array>
#include <vector>
#include <memory>
#include <cstdint>
#include <utility>

namespace Einstein {

// Type definitions
using BoardArray = std::array<std::array<int8_t, 5>, 5>;
using Position = std::pair<int, int>;
using Move = std::pair<Position, Position>;

// Game constants
constexpr int BOARD_SIZE = 5;
constexpr int NUM_PIECES = 6;
constexpr int MAX_DICE = 6;
constexpr int MIN_DICE = 1;

// Player types
enum class Player : int8_t {
    NONE = 0,
    LEFT_TOP = -1,    // LT player (AI)
    RIGHT_BOTTOM = 1  // RB player
};

// Game modes
enum class GameMode {
    HUMAN_VS_AI,
    AI_VS_AI,
    HUMAN_VS_HUMAN
};

// AI difficulty levels
enum class Difficulty {
    EASY,
    MEDIUM,
    HARD,
    EXPERT
};

// Game result
enum class GameResult {
    ONGOING,
    LT_WINS,
    RB_WINS,
    DRAW
};

// Move direction
enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    UP_LEFT,
    UP_RIGHT,
    DOWN_LEFT,
    DOWN_RIGHT
};

// AI settings
struct AIConfig {
    int mcts_iterations = 1000;
    double exploration_constant = 1.414;  // sqrt(2)
    bool use_neural_network = true;
    int thread_count = 4;
    double thinking_time = 5.0;  // seconds
};

// Graphics settings
struct GraphicsConfig {
    int window_width = 800;
    int window_height = 600;
    int board_size = 500;
    bool fullscreen = false;
    bool vsync = true;
};

// Game configuration
struct GameConfig {
    GameMode mode = GameMode::HUMAN_VS_AI;
    Difficulty difficulty = Difficulty::MEDIUM;
    bool sound_enabled = true;
    bool animations_enabled = true;
    AIConfig ai_config;
    GraphicsConfig graphics_config;
};

} // namespace Einstein
