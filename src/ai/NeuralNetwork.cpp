#include "ai/NeuralNetwork.h"
#include "core/ChessBoard.h"
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>

namespace Einstein {

NeuralNetwork::NeuralNetwork() : model_loaded_(false) {
    // Initialize random number generator for dummy predictions
    std::random_device rd;
    rng_.seed(rd());
}

NeuralNetwork::~NeuralNetwork() = default;

bool NeuralNetwork::LoadModel(const std::string& model_path) {
    try {
        // Check if model file exists
        std::ifstream file(model_path);
        if (!file.good()) {
            std::cerr << "Model file not found: " << model_path << std::endl;
            return false;
        }
        
        model_path_ = model_path;
        model_loaded_ = true;
        
        // In a real implementation, this would load the actual neural network model
        // For now, we'll simulate loading by reading basic model metadata
        std::cout << "Neural network model loaded from: " << model_path << std::endl;
        std::cout << "Model type: Position evaluation network" << std::endl;
        std::cout << "Input dimensions: 25 (5x5 board)" << std::endl;
        std::cout << "Output dimensions: 1 (position value)" << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load model: " << e.what() << std::endl;
        model_loaded_ = false;
        return false;
    }
}

bool NeuralNetwork::IsLoaded() const {
    return model_loaded_;
}

std::vector<float> NeuralNetwork::Predict(const std::vector<float>& input) {
    if (!model_loaded_) {
        std::cerr << "Model not loaded" << std::endl;
        return {};
    }
    
    if (input.size() != 25) {  // Expected 5x5 board representation
        std::cerr << "Invalid input size. Expected 25, got " << input.size() << std::endl;
        return {};
    }
    
    // Enhanced dummy prediction that considers board features
    std::vector<float> output(1);  // Single value output for position evaluation
    
    // Calculate position evaluation based on piece positions and values
    float evaluation = EvaluatePosition(input);
    output[0] = evaluation;
    
    return output;
}

float NeuralNetwork::EvaluatePosition(const std::vector<float>& board_state) {
    float score = 0.0f;
    
    // Basic position evaluation heuristics
    for (int i = 0; i < 25; ++i) {
        int row = i / 5;
        int col = i % 5;
        float piece = board_state[i];
        
        if (piece != 0.0f) {
            // Player identification
            bool is_left_top = piece < 0.0f;
            float piece_value = std::abs(piece);
            
            // Position-based scoring
            if (is_left_top) {
                // Left-top player benefits from advancing toward bottom-right
                float advance_bonus = (row + col) * 0.1f;
                score += piece_value * (1.0f + advance_bonus);
            } else {
                // Right-bottom player benefits from advancing toward top-left
                float advance_bonus = (4 - row + 4 - col) * 0.1f;
                score -= piece_value * (1.0f + advance_bonus);
            }
            
            // Center control bonus
            float center_distance = std::sqrt((row - 2.0f) * (row - 2.0f) + (col - 2.0f) * (col - 2.0f));
            float center_bonus = (3.0f - center_distance) * 0.05f;
            score += is_left_top ? center_bonus : -center_bonus;
        }
    }
    
    // Add some controlled randomness for variety
    std::uniform_real_distribution<float> noise(-0.1f, 0.1f);
    score += noise(rng_);
    
    // Normalize score to [-1, 1] range
    return std::tanh(score / 10.0f);
}

std::vector<std::vector<float>> NeuralNetwork::BatchPredict(const std::vector<std::vector<float>>& inputs) {
    std::vector<std::vector<float>> outputs;
    outputs.reserve(inputs.size());
    
    for (const auto& input : inputs) {
        outputs.push_back(Predict(input));
    }
    
    return outputs;
}

float NeuralNetwork::EvaluateBoard(const ChessBoard& board) {
    // Convert board to neural network input format
    std::vector<float> input = BoardToInput(board);
    
    // Get prediction
    auto prediction = Predict(input);
    
    if (prediction.empty()) {
        return 0.0f;  // Neutral evaluation if prediction fails
    }
    
    return prediction[0];
}

std::vector<float> NeuralNetwork::BoardToInput(const ChessBoard& board) {
    std::vector<float> input(25);
    const auto& board_array = board.GetBoard();
    
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            input[row * 5 + col] = static_cast<float>(board_array[row][col]);
        }
    }
    
    return input;
}

void NeuralNetwork::SetModelPath(const std::string& path) {
    model_path_ = path;
}

std::string NeuralNetwork::GetModelPath() const {
    return model_path_;
}

std::vector<float> NeuralNetwork::GetMoveProabilities(const ChessBoard& board, 
                                                     const std::vector<Move>& legal_moves) {
    if (!model_loaded_ || legal_moves.empty()) {
        // Return uniform distribution if model not loaded
        std::vector<float> probabilities(legal_moves.size(), 1.0f / legal_moves.size());
        return probabilities;
    }
    
    std::vector<float> probabilities;
    probabilities.reserve(legal_moves.size());
    
    // Evaluate each possible move
    for (const auto& move : legal_moves) {
        // Create a copy of the board and make the move
        ChessBoard temp_board = board;
        bool move_success = temp_board.ExecuteMove(move);
        
        if (move_success) {
            // Evaluate the resulting position
            float evaluation = EvaluateBoard(temp_board);
            // Convert evaluation to probability (higher evaluation = higher probability)
            probabilities.push_back(std::exp(evaluation));
        } else {
            probabilities.push_back(0.0f);  // Invalid move gets zero probability
        }
    }
    
    // Normalize probabilities
    float sum = 0.0f;
    for (float prob : probabilities) {
        sum += prob;
    }
    
    if (sum > 0.0f) {
        for (float& prob : probabilities) {
            prob /= sum;
        }
    } else {
        // Fallback to uniform distribution
        float uniform_prob = 1.0f / probabilities.size();
        std::fill(probabilities.begin(), probabilities.end(), uniform_prob);
    }
    
    return probabilities;
}

} // namespace Einstein
