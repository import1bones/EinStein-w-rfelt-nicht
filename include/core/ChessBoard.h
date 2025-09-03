#pragma once

#include "Types.h"
#include <array>
#include <vector>
#include <optional>
#include <cstdint>

namespace Einstein {

// Forward declarations
class GameConfig;

// Configuration enums for initialization
enum class InitialSetup {
    STANDARD_TRIANGLE,     // Original Einstein triangular layout
    BALANCED,              // Balanced defensive formation
    AGGRESSIVE,            // Forward-positioned pieces
    DEFENSIVE,             // Back-positioned pieces
    CUSTOM                 // User-defined positions
};

class ChessBoard {
public:
    ChessBoard();
    ChessBoard(const ChessBoard& other) = default;
    ChessBoard& operator=(const ChessBoard& other) = default;
    
    // Board initialization
    void Initialize();
    void Initialize(InitialSetup setup);
    void InitializeCustom(const std::vector<Position>& leftTop, const std::vector<Position>& rightBottom);
    void InitializeFromConfig(const GameConfig& config);
    void Clear();
    
    // AI-optimized initialization
    InitialSetup CalculateOptimalSetup(Player player, Difficulty difficulty = Difficulty::MEDIUM);
    std::vector<InitialSetup> GetPredefinedSetups() const;
    
    // Piece management
    int8_t GetPiece(int x, int y) const;
    void SetPiece(int x, int y, int8_t piece);
    bool IsEmpty(int x, int y) const;
    bool IsValidPosition(int x, int y) const;
    
    // Player pieces
    std::vector<Position> GetPlayerPieces(Player player) const;
    std::optional<Position> FindPiece(int piece_number, Player player) const;
    
    // Move validation and execution
    bool IsValidMove(const Move& move, Player player) const;
    bool ExecuteMove(const Move& move);
    void UndoMove(const Move& move, int8_t captured_piece = 0);
    
    // Game state queries
    bool HasPlayerWon(Player player) const;
    bool CanPlayerMove(Player player, int dice) const;
    std::vector<Move> GetValidMoves(Player player, int dice) const;
    
    // Board state
    const BoardArray& GetBoard() const { return board_; }
    void SetBoard(const BoardArray& board) { board_ = board; }
    
    // Utility functions
    void Print() const;
    uint64_t GetHash() const;
    bool operator==(const ChessBoard& other) const;
    
private:
    BoardArray board_;
    
    // Helper functions
    bool IsInTargetZone(const Position& pos, Player player) const;
    std::vector<Position> GetAdjacentPositions(const Position& pos) const;
    bool CanPieceMove(int piece_number, Player player, int dice) const;
    Player GetPieceOwner(int8_t piece) const;
    int GetPieceNumber(int8_t piece) const;
};

} // namespace Einstein
