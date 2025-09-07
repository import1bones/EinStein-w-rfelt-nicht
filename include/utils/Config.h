#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace Einstein {

// Simple JSON-like configuration system
class Config {
public:
    Config();
    ~Config() = default;
    
    // File operations
    bool LoadFromFile(const std::string& filename);
    bool SaveToFile(const std::string& filename) const;
    
    // Value getters with defaults
    std::string GetString(const std::string& key, const std::string& default_value = "") const;
    int GetInt(const std::string& key, int default_value = 0) const;
    double GetDouble(const std::string& key, double default_value = 0.0) const;
    bool GetBool(const std::string& key, bool default_value = false) const;
    
    // Value setters
    void SetString(const std::string& key, const std::string& value);
    void SetInt(const std::string& key, int value);
    void SetDouble(const std::string& key, double value);
    void SetBool(const std::string& key, bool value);
    
    // Array operations
    std::vector<std::string> GetStringArray(const std::string& key) const;
    std::vector<int> GetIntArray(const std::string& key) const;
    std::vector<double> GetDoubleArray(const std::string& key) const;
    
    // Nested configuration
    std::shared_ptr<Config> GetSection(const std::string& section_name) const;
    void SetSection(const std::string& section_name, std::shared_ptr<Config> section);
    
    // Utility
    bool HasKey(const std::string& key) const;
    std::vector<std::string> GetAllKeys() const;
    void Clear();
    
private:
    std::map<std::string, std::string> string_values_;
    std::map<std::string, std::shared_ptr<Config>> sections_;
    
    // Helper methods
    std::string Trim(const std::string& str) const;
    std::vector<std::string> Split(const std::string& str, char delimiter) const;
    bool ParseLine(const std::string& line, std::string& key, std::string& value) const;
};

} // namespace Einstein
