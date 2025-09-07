#pragma once

#include <string>
#include <vector>
#include <random>

namespace Einstein {

// Forward declarations
class ChessBoard;
using Move = std::pair<std::pair<int, int>, std::pair<int, int>>;

class NeuralNetwork {
public:
    NeuralNetwork();
    ~NeuralNetwork();
    
    // Model management
    bool LoadModel(const std::string& model_path);
    bool IsLoaded() const;
    void SetModelPath(const std::string& path);
    std::string GetModelPath() const;
    
    // Prediction
    std::vector<float> Predict(const std::vector<float>& input);
    std::vector<std::vector<float>> BatchPredict(const std::vector<std::vector<float>>& inputs);
    
    // Board evaluation
    float EvaluateBoard(const ChessBoard& board);
    std::vector<float> GetMoveProabilities(const ChessBoard& board, const std::vector<Move>& legal_moves);
    
private:
    std::string model_path_;
    bool model_loaded_;
    mutable std::mt19937 rng_;  // Random number generator for stochastic elements
    
    // Helper methods
    float EvaluatePosition(const std::vector<float>& board_state);
    std::vector<float> BoardToInput(const ChessBoard& board);
};

} // namespace Einstein
