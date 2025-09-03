# Configurable Board Initialization System

## Overview

The Einstein game now supports configurable board initialization, allowing users and AI to customize starting positions for strategic gameplay optimization. This system provides multiple predefined setups and AI-optimized configurations based on difficulty levels.

## Features

### 1. Predefined Setup Types

#### Standard Triangle (`STANDARD_TRIANGLE`)
- **Description**: Original Einstein game triangular formation
- **Left-Top Player**: Triangular formation in top-left corner
  - Row 0: pieces 1,2,3 in columns 0,1,2
  - Row 1: pieces 4,5 in columns 0,1
  - Row 2: piece 6 in column 0
- **Right-Bottom Player**: Triangular formation in bottom-right corner
  - Row 3: pieces 6,5 in columns 3,4
  - Row 4: pieces 3,2,1 in columns 2,3,4

#### Balanced Setup (`BALANCED`)
- **Description**: Pieces more spread out for balanced gameplay
- **Strategy**: Provides equal offensive and defensive positioning
- **Use Case**: Medium difficulty games, balanced AI vs human matches

#### Aggressive Setup (`AGGRESSIVE`)
- **Description**: Pieces positioned closer to center for forward gameplay
- **Strategy**: Fast-paced, attack-oriented positioning
- **Use Case**: Expert difficulty, aggressive AI behavior

#### Defensive Setup (`DEFENSIVE`)
- **Description**: Pieces positioned closer to edges for defensive strategy
- **Strategy**: Conservative, protection-focused positioning
- **Use Case**: Easy difficulty, defensive AI behavior

#### Custom Setup (`CUSTOM`)
- **Description**: User-defined piece positions
- **Implementation**: Via `InitializeCustom()` method
- **Use Case**: Tournament play, specific scenario testing

### 2. AI-Optimized Configuration

The system includes AI-based optimal setup calculation that considers:
- Player type (LEFT_TOP vs RIGHT_BOTTOM)
- Difficulty level (EASY, MEDIUM, HARD, EXPERT)
- Strategic countering (defensive vs aggressive positioning)

#### Optimization Logic
```cpp
// For LEFT_TOP player
EASY → DEFENSIVE      // Conservative approach
MEDIUM → BALANCED     // Balanced strategy
HARD/EXPERT → AGGRESSIVE  // Aggressive positioning

// For RIGHT_BOTTOM player (counter-strategy)
EASY → AGGRESSIVE     // Counter defensive with aggressive
MEDIUM → BALANCED     // Mirror balanced approach
HARD/EXPERT → DEFENSIVE   // Counter aggressive with defensive
```

## API Reference

### Core Methods

#### `ChessBoard::Initialize(InitialSetup setup)`
```cpp
void Initialize(InitialSetup setup);
```
Initialize board with predefined setup type.

**Parameters:**
- `setup`: One of `STANDARD_TRIANGLE`, `BALANCED`, `AGGRESSIVE`, `DEFENSIVE`, `CUSTOM`

**Example:**
```cpp
ChessBoard board;
board.Initialize(InitialSetup::AGGRESSIVE);
```

#### `ChessBoard::InitializeCustom(leftTop, rightBottom)`
```cpp
void InitializeCustom(const std::vector<Position>& leftTop, 
                     const std::vector<Position>& rightBottom);
```
Initialize board with custom piece positions.

**Parameters:**
- `leftTop`: Vector of 6 positions for left-top player pieces
- `rightBottom`: Vector of 6 positions for right-bottom player pieces

**Example:**
```cpp
std::vector<Position> custom_lt = {{0,0}, {0,1}, {1,0}, {1,1}, {2,0}, {2,1}};
std::vector<Position> custom_rb = {{2,3}, {2,4}, {3,2}, {3,3}, {4,2}, {4,3}};
board.InitializeCustom(custom_lt, custom_rb);
```

#### `ChessBoard::InitializeFromConfig(const GameConfig& config)`
```cpp
void InitializeFromConfig(const GameConfig& config);
```
Initialize board based on game configuration settings.

#### `ChessBoard::CalculateOptimalSetup(player, difficulty)`
```cpp
InitialSetup CalculateOptimalSetup(Player player, Difficulty difficulty);
```
Calculate AI-optimized setup for given player and difficulty.

**Returns:** Recommended `InitialSetup` enum value

#### `ChessBoard::GetPredefinedSetups()`
```cpp
std::vector<InitialSetup> GetPredefinedSetups() const;
```
Get list of all available predefined setups.

## CLI Integration

### Configuration Demo Command

```bash
# View all setup types
./EinStein_Game config-demo

# View specific setup
./EinStein_Game config-demo --setup aggressive

# View AI-optimized setups
./EinStein_Game config-demo --ai-optimal

# Verbose mode
./EinStein_Game config-demo --verbose
```

### Command Line Options

| Option | Description | Example |
|--------|-------------|---------|
| `--setup <type>` | Show specific setup type | `--setup balanced` |
| `--ai-optimal` | Demonstrate AI optimization | `--ai-optimal` |
| `--verbose` | Enable verbose output | `--verbose` |

### Supported Setup Types
- `all` (default): Show all setup types
- `standard`: Standard triangular formation
- `balanced`: Balanced formation
- `aggressive`: Aggressive formation
- `defensive`: Defensive formation

## Usage Examples

### Basic Usage
```cpp
#include "core/ChessBoard.h"

// Create board with aggressive setup
ChessBoard board;
board.Initialize(InitialSetup::AGGRESSIVE);

// Get AI recommendation
auto optimal = board.CalculateOptimalSetup(Player::LEFT_TOP, Difficulty::HARD);
board.Initialize(optimal);
```

### Custom Configuration
```cpp
// Define custom positions
std::vector<Position> my_left_positions = {
    {0,0}, {0,1}, {1,0}, {1,1}, {2,0}, {2,1}
};
std::vector<Position> my_right_positions = {
    {2,3}, {2,4}, {3,2}, {3,3}, {4,2}, {4,3}
};

// Apply custom setup
board.InitializeCustom(my_left_positions, my_right_positions);
```

### Game Integration
```cpp
// Initialize game with difficulty-based setup
GameConfig config;
config.difficulty = Difficulty::EXPERT;

ChessBoard board;
board.InitializeFromConfig(config);
// Board now uses aggressive setup for expert difficulty
```

## Visual Examples

### Standard Triangle Setup
```
 1  2  3  .  .
 4  5  .  .  .
 6  .  .  .  .
 .  .  .  6  5
 .  .  3  2  1
```

### Balanced Setup
```
 .  1  .  2  .
 3  .  4  .  5
 .  6  .  6  .
 5  .  4  .  3
 .  2  .  1  .
```

### Aggressive Setup
```
 .  .  1  2  .
 .  3  4  5  .
 .  .  6  .  .
 .  6  5  4  .
 .  3  2  .  .
```

### Defensive Setup
```
 1  2  3  .  .
 4  5  .  .  .
 6  .  .  .  6
 .  .  .  5  4
 .  .  3  2  1
```

## Integration with MCTS

The configurable initialization system works seamlessly with the MCTS snapshot system:

```cpp
// Initialize with custom setup
board.Initialize(InitialSetup::BALANCED);

// Create MCTS snapshot with custom initial state
SnapshotGameRunner runner;
auto snapshot = runner.CaptureMCTSStateForDebug();
// MCTS will analyze from the custom initial position
```

## Performance Considerations

- **Setup Time**: All initialization methods are O(1) constant time
- **Memory Usage**: No additional memory overhead
- **AI Impact**: Different setups may affect MCTS search tree complexity
- **Validation**: Custom positions are validated for rule compliance

## Future Enhancements

1. **Machine Learning Integration**: Train neural networks to predict optimal setups
2. **Tournament Mode**: Save/load setup configurations for competitive play
3. **Setup Analytics**: Track win rates for different setup combinations
4. **Dynamic Adaptation**: AI learns and adapts setup preferences during gameplay
5. **Configuration Files**: JSON-based setup definitions for easy customization

## Testing

The configuration system includes comprehensive testing via:

```bash
# Test all configurations
./EinStein_Game config-demo

# Test AI optimization
./EinStein_Game config-demo --ai-optimal

# Test with MCTS integration
./EinStein_Game mcts-demo --moves 10
```

## Troubleshooting

### Common Issues

1. **Invalid Custom Positions**: Ensure positions are within board bounds (0-4, 0-4)
2. **Piece Overlap**: Custom positions must not overlap between players
3. **Missing Pieces**: Each player must have exactly 6 pieces
4. **Compilation Errors**: Ensure all headers are included correctly

### Debug Output
Enable verbose mode to see detailed setup information:
```bash
./EinStein_Game config-demo --verbose
```

This documentation covers the complete configurable initialization system, providing both technical reference and practical usage examples for developers and users.
