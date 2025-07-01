#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace Einstein {

/**
 * Command-Line Interface for automated testing and game operations
 * 
 * Supports:
 * - Interactive game mode
 * - Automated testing with output formats
 * - AI self-play for training/evaluation
 * - Performance benchmarking
 * - Configuration validation
 * - CI/CD integration
 */

struct CLICommand {
    std::string name;
    std::string description;
    std::function<int(const std::vector<std::string>&)> handler;
    std::vector<std::string> usage_examples;
};

class CLIInterface {
public:
    CLIInterface();
    
    /**
     * Execute CLI command based on command-line arguments
     * @param argc Number of arguments
     * @param argv Array of argument strings
     * @return Exit code (0 = success, non-zero = error)
     */
    int Execute(int argc, char* argv[]);
    
    /**
     * Show help information for all commands or a specific command
     * @param command_name Optional specific command to show help for
     */
    void ShowHelp(const std::string& command_name = "");
    
private:
    std::map<std::string, CLICommand> commands_;
    
    void SetupCommands();
    int RunInteractiveMode();
    
    // Command handlers
    int HandleHelp(const std::vector<std::string>& args);
    int HandleRun(const std::vector<std::string>& args);
    int HandleTest(const std::vector<std::string>& args);
    int HandleSelfPlay(const std::vector<std::string>& args);
    int HandleBenchmark(const std::vector<std::string>& args);
    int HandleValidate(const std::vector<std::string>& args);
    int HandleVersion(const std::vector<std::string>& args);
    
    // Utility methods
    bool ParseArgument(const std::vector<std::string>& args, const std::string& flag, std::string& value);
    bool ParseArgument(const std::vector<std::string>& args, const std::string& flag, int& value);
    bool HasFlag(const std::vector<std::string>& args, const std::string& flag);
};

/**
 * Output formats for automated testing and CI integration
 */
namespace OutputFormat {
    void WriteTestResults(const std::string& filename, int passed, int failed, const std::vector<std::string>& details = {});
    void WriteBenchmarkResults(const std::string& filename, double avg_time, int iterations, const std::map<std::string, double>& metrics = {});
    void WriteSelfPlayResults(const std::string& filename, int ai1_wins, int ai2_wins, int draws, const std::vector<std::string>& game_data = {});
    void WriteJSONOutput(const std::string& filename, const std::map<std::string, std::string>& data);
}

} // namespace Einstein
