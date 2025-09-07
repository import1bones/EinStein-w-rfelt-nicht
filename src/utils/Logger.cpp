#include "utils/Logger.h"
#include "utils/Config.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <filesystem>

namespace Einstein {

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

void Logger::Initialize(const std::string& config_file) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (initialized_) {
        return;
    }
    
    // Try to load configuration
    Config config;
    if (config.LoadFromFile(config_file)) {
        // Read logging configuration
        std::string level_str = config.GetString("logging.level", "info");
        file_output_ = config.GetBool("logging.file_output", true);
        console_output_ = config.GetBool("logging.console_output", true);
        log_file_ = config.GetString("logging.log_file", "logs/einstein.log");
        
        // Set log level
        if (level_str == "debug") {
            log_level_ = LogLevel::LOG_DEBUG;
        } else if (level_str == "info") {
            log_level_ = LogLevel::LOG_INFO;
        } else if (level_str == "warning") {
            log_level_ = LogLevel::LOG_WARNING;
        } else if (level_str == "error") {
            log_level_ = LogLevel::LOG_ERROR;
        }
    }
    
    // Create logs directory if it doesn't exist
    if (file_output_) {
        std::filesystem::path log_path(log_file_);
        std::filesystem::create_directories(log_path.parent_path());
        
        file_stream_ = std::make_unique<std::ofstream>(log_file_, std::ios::app);
        if (!file_stream_->is_open()) {
            std::cerr << "Warning: Could not open log file: " << log_file_ << std::endl;
            file_output_ = false;
        }
    }
    
    initialized_ = true;
    Info("Logger initialized");
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (file_stream_ && file_stream_->is_open()) {
        Info("Logger shutting down");
        file_stream_->close();
    }
    
    initialized_ = false;
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (!initialized_ || level < log_level_) {
        return;
    }
    
    std::string formatted = FormatMessage(level, message);
    
    if (console_output_) {
        WriteToConsole(formatted);
    }
    
    if (file_output_) {
        WriteToFile(formatted);
    }
}

void Logger::Debug(const std::string& message) {
    Log(LogLevel::LOG_DEBUG, message);
}

void Logger::Info(const std::string& message) {
    Log(LogLevel::LOG_INFO, message);
}

void Logger::Warning(const std::string& message) {
    Log(LogLevel::LOG_WARNING, message);
}

void Logger::Error(const std::string& message) {
    Log(LogLevel::LOG_ERROR, message);
}

void Logger::SetLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (file_stream_ && file_stream_->is_open()) {
        file_stream_->close();
    }
    
    log_file_ = filename;
    
    if (file_output_) {
        std::filesystem::path log_path(log_file_);
        std::filesystem::create_directories(log_path.parent_path());
        
        file_stream_ = std::make_unique<std::ofstream>(log_file_, std::ios::app);
        if (!file_stream_->is_open()) {
            std::cerr << "Warning: Could not open log file: " << log_file_ << std::endl;
            file_output_ = false;
        }
    }
}

void Logger::WriteToFile(const std::string& formatted_message) {
    if (file_stream_ && file_stream_->is_open()) {
        std::lock_guard<std::mutex> lock(log_mutex_);
        *file_stream_ << formatted_message << std::endl;
        file_stream_->flush();
    }
}

void Logger::WriteToConsole(const std::string& formatted_message) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    std::cout << formatted_message << std::endl;
}

std::string Logger::FormatMessage(LogLevel level, const std::string& message) {
    std::ostringstream oss;
    oss << "[" << GetTimestamp() << "] "
        << "[" << LogLevelToString(level) << "] "
        << message;
    return oss.str();
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::LOG_DEBUG: return "DEBUG";
        case LogLevel::LOG_INFO: return "INFO";
        case LogLevel::LOG_WARNING: return "WARN";
        case LogLevel::LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

} // namespace Einstein
