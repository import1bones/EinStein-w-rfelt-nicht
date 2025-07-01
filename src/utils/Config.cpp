#include "utils/Config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>

namespace Einstein {

// Config implementation
Config::Config() = default;

bool Config::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    Clear();
    
    std::string line;
    std::string current_section;
    std::shared_ptr<Config> current_config = std::make_shared<Config>();
    
    while (std::getline(file, line)) {
        line = Trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Check for section header [section_name]
        if (line[0] == '[' && line.back() == ']') {
            // Save previous section
            if (!current_section.empty()) {
                sections_[current_section] = current_config;
            }
            
            // Start new section
            current_section = line.substr(1, line.length() - 2);
            current_config = std::make_shared<Config>();
            continue;
        }
        
        // Parse key-value pair
        std::string key, value;
        if (ParseLine(line, key, value)) {
            if (current_section.empty()) {
                // Global setting
                string_values_[key] = value;
            } else {
                // Section setting
                current_config->string_values_[key] = value;
            }
        }
    }
    
    // Save last section
    if (!current_section.empty()) {
        sections_[current_section] = current_config;
    }
    
    return true;
}

bool Config::SaveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Write global settings first
    for (const auto& pair : string_values_) {
        file << pair.first << " = " << pair.second << std::endl;
    }
    
    if (!string_values_.empty() && !sections_.empty()) {
        file << std::endl;
    }
    
    // Write sections
    for (const auto& section_pair : sections_) {
        file << "[" << section_pair.first << "]" << std::endl;
        
        for (const auto& value_pair : section_pair.second->string_values_) {
            file << value_pair.first << " = " << value_pair.second << std::endl;
        }
        
        file << std::endl;
    }
    
    return true;
}

std::string Config::GetString(const std::string& key, const std::string& default_value) const {
    auto it = string_values_.find(key);
    return (it != string_values_.end()) ? it->second : default_value;
}

int Config::GetInt(const std::string& key, int default_value) const {
    auto value = GetString(key);
    if (value.empty()) return default_value;
    
    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

double Config::GetDouble(const std::string& key, double default_value) const {
    auto value = GetString(key);
    if (value.empty()) return default_value;
    
    try {
        return std::stod(value);
    } catch (...) {
        return default_value;
    }
}

bool Config::GetBool(const std::string& key, bool default_value) const {
    auto value = GetString(key);
    if (value.empty()) return default_value;
    
    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
    
    return (lower_value == "true" || lower_value == "yes" || lower_value == "1");
}

void Config::SetString(const std::string& key, const std::string& value) {
    string_values_[key] = value;
}

void Config::SetInt(const std::string& key, int value) {
    string_values_[key] = std::to_string(value);
}

void Config::SetDouble(const std::string& key, double value) {
    string_values_[key] = std::to_string(value);
}

void Config::SetBool(const std::string& key, bool value) {
    string_values_[key] = value ? "true" : "false";
}

std::vector<std::string> Config::GetStringArray(const std::string& key) const {
    auto value = GetString(key);
    if (value.empty()) return {};
    
    return Split(value, ',');
}

std::vector<int> Config::GetIntArray(const std::string& key) const {
    auto string_array = GetStringArray(key);
    std::vector<int> int_array;
    
    for (const auto& str : string_array) {
        try {
            int_array.push_back(std::stoi(Trim(str)));
        } catch (...) {
            // Skip invalid values
        }
    }
    
    return int_array;
}

std::vector<double> Config::GetDoubleArray(const std::string& key) const {
    auto string_array = GetStringArray(key);
    std::vector<double> double_array;
    
    for (const auto& str : string_array) {
        try {
            double_array.push_back(std::stod(Trim(str)));
        } catch (...) {
            // Skip invalid values
        }
    }
    
    return double_array;
}

std::shared_ptr<Config> Config::GetSection(const std::string& section_name) const {
    auto it = sections_.find(section_name);
    return (it != sections_.end()) ? it->second : nullptr;
}

void Config::SetSection(const std::string& section_name, std::shared_ptr<Config> section) {
    sections_[section_name] = section;
}

bool Config::HasKey(const std::string& key) const {
    return string_values_.find(key) != string_values_.end();
}

std::vector<std::string> Config::GetAllKeys() const {
    std::vector<std::string> keys;
    for (const auto& pair : string_values_) {
        keys.push_back(pair.first);
    }
    return keys;
}

void Config::Clear() {
    string_values_.clear();
    sections_.clear();
}

// Private helper methods
std::string Config::Trim(const std::string& str) const {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> Config::Split(const std::string& str, char delimiter) const {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(Trim(token));
    }
    
    return tokens;
}

bool Config::ParseLine(const std::string& line, std::string& key, std::string& value) const {
    size_t pos = line.find('=');
    if (pos == std::string::npos) {
        return false;
    }
    
    key = Trim(line.substr(0, pos));
    value = Trim(line.substr(pos + 1));
    
    // Remove quotes if present
    if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.length() - 2);
    }
    
    return !key.empty();
}

// Logger implementation
Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

void Logger::Initialize(bool console_output, bool file_output, const std::string& log_file) {
    console_output_ = console_output;
    file_output_ = file_output;
    log_filename_ = log_file;
    
    if (file_output_ && !log_filename_.empty()) {
        log_file_ = std::make_unique<std::ofstream>(log_filename_, std::ios::app);
        if (log_file_->is_open()) {
            WriteMessage(LogLevel::INFO, "=== Log Session Started ===");
        }
    }
}

void Logger::Shutdown() {
    if (log_file_ && log_file_->is_open()) {
        WriteMessage(LogLevel::INFO, "=== Log Session Ended ===");
        log_file_->close();
    }
}

void Logger::Debug(const std::string& message) {
    if (current_level_ <= LogLevel::DEBUG) {
        WriteMessage(LogLevel::DEBUG, message);
    }
}

void Logger::Info(const std::string& message) {
    if (current_level_ <= LogLevel::INFO) {
        WriteMessage(LogLevel::INFO, message);
    }
}

void Logger::Warning(const std::string& message) {
    if (current_level_ <= LogLevel::WARNING) {
        WriteMessage(LogLevel::WARNING, message);
    }
}

void Logger::Error(const std::string& message) {
    if (current_level_ <= LogLevel::ERROR) {
        WriteMessage(LogLevel::ERROR, message);
    }
}

void Logger::SetLogLevel(LogLevel level) {
    current_level_ = level;
}

void Logger::SetConsoleOutput(bool enabled) {
    console_output_ = enabled;
}

void Logger::SetFileOutput(bool enabled, const std::string& filename) {
    file_output_ = enabled;
    if (!filename.empty()) {
        log_filename_ = filename;
    }
    
    if (file_output_ && !log_filename_.empty()) {
        log_file_ = std::make_unique<std::ofstream>(log_filename_, std::ios::app);
    } else if (log_file_) {
        log_file_->close();
        log_file_.reset();
    }
}

void Logger::WriteMessage(LogLevel level, const std::string& message) {
    std::string log_line = "[" + GetTimestamp() + "] [" + LevelToString(level) + "] " + message;
    
    if (console_output_) {
        if (level == LogLevel::ERROR || level == LogLevel::WARNING) {
            std::cerr << log_line << std::endl;
        } else {
            std::cout << log_line << std::endl;
        }
    }
    
    if (file_output_ && log_file_ && log_file_->is_open()) {
        *log_file_ << log_line << std::endl;
        log_file_->flush();
    }
}

std::string Logger::GetTimestamp() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::LevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

} // namespace Einstein
