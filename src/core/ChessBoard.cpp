#include "core/ChessBoard.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <optional>
#include <cassert>

namespace Einstein {

ChessBoard::ChessBoard() {
    Initialize();
}

void ChessBoard::Initialize() {
    Clear();
    
    // Initialize Left-Top player pieces (negative values)
    board_[0][0] = -1;
    board_[1][0] = -2;
    board_[2][0] = -3;
    board_[0][1] = -4;
    board_[1][1] = -5;
    board_[2][1] = -6;
    
    // Initialize Right-Bottom player pieces (positive values)
    board_[4][4] = 1;
    board_[3][4] = 2;
    board_[2][4] = 3;
    board_[4][3] = 4;
    board_[3][3] = 5;
    board_[2][3] = 6;
}

void ChessBoard::Clear() {
    for (auto& row : board_) {
        row.fill(0);
    }
}

int8_t ChessBoard::GetPiece(int x, int y) const {
    if (!IsValidPosition(x, y)) return 0;
    return board_[x][y];
}

void ChessBoard::SetPiece(int x, int y, int8_t piece) {
    if (IsValidPosition(x, y)) {
        board_[x][y] = piece;
    }
}

bool ChessBoard::IsEmpty(int x, int y) const {
    return GetPiece(x, y) == 0;
}

bool ChessBoard::IsValidPosition(int x, int y) const {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

std::vector<Position> ChessBoard::GetPlayerPieces(Player player) const {
    std::vector<Position> pieces;
    
    for (int x = 0; x < BOARD_SIZE; ++x) {
        for (int y = 0; y < BOARD_SIZE; ++y) {
            int8_t piece = board_[x][y];
            if (GetPieceOwner(piece) == player) {
                pieces.emplace_back(x, y);
            }
        }
    }
    
    return pieces;
}

std::optional<Position> ChessBoard::FindPiece(int piece_number, Player player) const {
    int8_t target_piece = (player == Player::LEFT_TOP) ? -piece_number : piece_number;
    
    for (int x = 0; x < BOARD_SIZE; ++x) {
        for (int y = 0; y < BOARD_SIZE; ++y) {
            if (board_[x][y] == target_piece) {
                return Position{x, y};
            }
        }
    }
    
    return std::nullopt;
}

bool ChessBoard::IsValidMove(const Move& move, Player player) const {
    const auto& [from, to] = move;
    const auto& [from_x, from_y] = from;
    const auto& [to_x, to_y] = to;
    
    // Check if positions are valid
    if (!IsValidPosition(from_x, from_y) || !IsValidPosition(to_x, to_y)) {
        return false;
    }
    
    // Check if there's a piece at the source position
    int8_t piece = board_[from_x][from_y];
    if (piece == 0) {
        return false;
    }
    
    // Check if the piece belongs to the current player
    if (GetPieceOwner(piece) != player) {
        return false;
    }
    
    // Check if destination is not occupied by own piece
    int8_t dest_piece = board_[to_x][to_y];
    if (dest_piece != 0 && GetPieceOwner(dest_piece) == player) {
        return false;
    }
    
    // Check if move is adjacent (only one step allowed)
    int dx = std::abs(to_x - from_x);
    int dy = std::abs(to_y - from_y);
    if (dx > 1 || dy > 1 || (dx == 0 && dy == 0)) {
        return false;
    }
    
    return true;
}

bool ChessBoard::ExecuteMove(const Move& move) {
    const auto& [from, to] = move;
    const auto& [from_x, from_y] = from;
    const auto& [to_x, to_y] = to;
    
    int8_t piece = board_[from_x][from_y];
    board_[from_x][from_y] = 0;
    board_[to_x][to_y] = piece;
    
    return true;
}

void ChessBoard::UndoMove(const Move& move, int8_t captured_piece) {
    const auto& [from, to] = move;
    const auto& [from_x, from_y] = from;
    const auto& [to_x, to_y] = to;
    
    int8_t piece = board_[to_x][to_y];
    board_[to_x][to_y] = captured_piece;
    board_[from_x][from_y] = piece;
}

bool ChessBoard::HasPlayerWon(Player player) const {
    // Einstein game win conditions:
    // 1. Get any piece to the opposite corner
    // 2. Capture all opponent pieces
    
    auto pieces = GetPlayerPieces(player);
    
    // Check if no pieces left (lost condition for this player)
    if (pieces.empty()) {
        return false;
    }
    
    // Check if any piece reached the target zone
    for (const auto& pos : pieces) {
        if (IsInTargetZone(pos, player)) {
            return true;
        }
    }
    
    // Check if opponent has no pieces left
    Player opponent = (player == Player::LEFT_TOP) ? Player::RIGHT_BOTTOM : Player::LEFT_TOP;
    auto opponent_pieces = GetPlayerPieces(opponent);
    if (opponent_pieces.empty()) {
        return true;
    }
    
    return false;
}

bool ChessBoard::CanPlayerMove(Player player, int dice) const {
    // Check if any piece can move with the given dice
    auto valid_moves = GetValidMoves(player, dice);
    return !valid_moves.empty();
}

std::vector<Move> ChessBoard::GetValidMoves(Player player, int dice) const {
    std::vector<Move> valid_moves;
    
    // In Einstein game, you can move pieces with numbers that match dice or are multiples
    std::vector<int> movable_pieces;
    
    // Find which piece numbers can move with this dice value
    for (int piece_num = 1; piece_num <= NUM_PIECES; ++piece_num) {
        if (CanPieceMove(piece_num, player, dice)) {
            auto piece_pos = FindPiece(piece_num, player);
            if (piece_pos.has_value()) {
                movable_pieces.push_back(piece_num);
            }
        }
    }
    
    // If no exact match, find the closest available piece
    if (movable_pieces.empty()) {
        // Look for the closest smaller piece
        for (int piece_num = dice - 1; piece_num >= 1; --piece_num) {
            auto piece_pos = FindPiece(piece_num, player);
            if (piece_pos.has_value()) {
                movable_pieces.push_back(piece_num);
                break;
            }
        }
        
        // If still no piece found, look for the closest larger piece
        if (movable_pieces.empty()) {
            for (int piece_num = dice + 1; piece_num <= NUM_PIECES; ++piece_num) {
                auto piece_pos = FindPiece(piece_num, player);
                if (piece_pos.has_value()) {
                    movable_pieces.push_back(piece_num);
                    break;
                }
            }
        }
    }
    
    // Generate moves for each movable piece
    for (int piece_num : movable_pieces) {
        auto piece_pos = FindPiece(piece_num, player);
        if (piece_pos.has_value()) {
            auto adjacent_positions = GetAdjacentPositions(piece_pos.value());
            for (const auto& adj_pos : adjacent_positions) {
                Move test_move{piece_pos.value(), adj_pos};
                if (IsValidMove(test_move, player)) {
                    valid_moves.push_back(test_move);
                }
            }
        }
    }
    
    return valid_moves;
}

void ChessBoard::Print() const {
    std::cout << "  ";
    for (int x = 0; x < BOARD_SIZE; ++x) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
    
    for (int y = 0; y < BOARD_SIZE; ++y) {
        std::cout << y << " ";
        for (int x = 0; x < BOARD_SIZE; ++x) {
            int8_t piece = board_[x][y];
            if (piece == 0) {
                std::cout << ". ";
            } else {
                std::cout << static_cast<int>(piece) << " ";
            }
        }
        std::cout << std::endl;
    }
}

uint64_t ChessBoard::GetHash() const {
    std::hash<int8_t> hasher;
    uint64_t hash = 0;
    
    for (int x = 0; x < BOARD_SIZE; ++x) {
        for (int y = 0; y < BOARD_SIZE; ++y) {
            hash ^= hasher(board_[x][y]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
    }
    
    return hash;
}

bool ChessBoard::operator==(const ChessBoard& other) const {
    return board_ == other.board_;
}

// Private helper functions

bool ChessBoard::IsInTargetZone(const Position& pos, Player player) const {
    const auto& [x, y] = pos;
    
    if (player == Player::LEFT_TOP) {
        // LT player wins by reaching the bottom-right corner area
        return (x == BOARD_SIZE - 1 && y == BOARD_SIZE - 1);
    } else {
        // RB player wins by reaching the top-left corner area
        return (x == 0 && y == 0);
    }
}

std::vector<Position> ChessBoard::GetAdjacentPositions(const Position& pos) const {
    std::vector<Position> adjacent;
    const auto& [x, y] = pos;
    
    // Check all 8 directions
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;  // Skip current position
            
            int new_x = x + dx;
            int new_y = y + dy;
            
            if (IsValidPosition(new_x, new_y)) {
                adjacent.emplace_back(new_x, new_y);
            }
        }
    }
    
    return adjacent;
}

bool ChessBoard::CanPieceMove(int piece_number, Player player, int dice) const {
    // In Einstein game, exact dice match or closest available piece
    return piece_number == dice;
}

Player ChessBoard::GetPieceOwner(int8_t piece) const {
    if (piece > 0) return Player::RIGHT_BOTTOM;
    if (piece < 0) return Player::LEFT_TOP;
    return Player::NONE;
}

int ChessBoard::GetPieceNumber(int8_t piece) const {
    return std::abs(piece);
}

} // namespace Einstein
