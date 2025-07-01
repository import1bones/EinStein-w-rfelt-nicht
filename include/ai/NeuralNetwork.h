#pragma once

#include <string>
#include <vector>

namespace Einstein {

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
    
private:
    std::string model_path_;
    bool model_loaded_;
};

} // namespace Einstein
