#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <chrono>
#include <sstream>

namespace Einstein {

enum class LogLevel : std::uint8_t {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
};

class Logger {
public:
    static Logger& Instance();
    
    void Initialize(const std::string& config_file = "config.json");
    void Shutdown();
    
    void Log(LogLevel level, const std::string& message);
    void Debug(const std::string& message);
    void Info(const std::string& message);
    void Warning(const std::string& message);
    void Error(const std::string& message);
    
    void SetLogLevel(LogLevel level) { log_level_ = level; }
    void SetFileOutput(bool enabled) { file_output_ = enabled; }
    void SetConsoleOutput(bool enabled) { console_output_ = enabled; }
    void SetLogFile(const std::string& filename);
    
private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void WriteToFile(const std::string& formatted_message);
    void WriteToConsole(const std::string& formatted_message);
    std::string FormatMessage(LogLevel level, const std::string& message);
    std::string GetTimestamp();
    std::string LogLevelToString(LogLevel level);
    
    LogLevel log_level_ = LogLevel::LOG_INFO;
    bool file_output_ = false;
    bool console_output_ = true;
    std::string log_file_ = "logs/einstein.log";
    
    std::unique_ptr<std::ofstream> file_stream_;
    std::mutex log_mutex_;
    bool initialized_ = false;
};

} // namespace Einstein
