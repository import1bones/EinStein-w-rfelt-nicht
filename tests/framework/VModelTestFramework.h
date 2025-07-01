#pragma once

#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <fstream>
#include <sstream>

namespace VModel {

// V-Model Test Levels (left side of V)
enum class TestLevel {
    Unit,           // Component testing
    Integration,    // Module integration testing
    System,         // System testing
    Acceptance      // User acceptance testing
};

// V-Model Verification Levels (right side of V)
enum class VerificationLevel {
    UnitVerification,        // Code reviews, static analysis
    IntegrationVerification, // Interface testing
    SystemVerification,      // Performance, security testing
    AcceptanceVerification   // User validation
};

struct TestMetrics {
    std::string test_name;
    TestLevel level;
    bool passed;
    double execution_time_ms;
    std::string error_message;
    std::string component_tested;
    std::chrono::system_clock::time_point timestamp;
};

struct CoverageInfo {
    std::string component;
    int lines_covered;
    int total_lines;
    double coverage_percentage;
};

class VModelTestFramework {
public:
    static VModelTestFramework& Instance() {
        static VModelTestFramework instance;
        return instance;
    }

    // Test Registration
    void RegisterUnitTest(const std::string& name, const std::string& component, 
                         std::function<void()> test_func);
    void RegisterIntegrationTest(const std::string& name, const std::string& component,
                               std::function<void()> test_func);
    void RegisterSystemTest(const std::string& name, std::function<void()> test_func);
    void RegisterAcceptanceTest(const std::string& name, std::function<void()> test_func);

    // Test Execution
    void RunUnitTests();
    void RunIntegrationTests();
    void RunSystemTests();
    void RunAcceptanceTests();
    void RunAllTests();
    void RunTestsForComponent(const std::string& component);

    // Reporting
    void GenerateReport(const std::string& output_file = "");
    void PrintSummary();
    void PrintDetailedResults();
    void GenerateVModelReport();

    // Coverage Analysis
    void SetCoverageData(const std::vector<CoverageInfo>& coverage);
    double GetOverallCoverage() const;

    // Performance Monitoring
    void SetPerformanceBenchmark(const std::string& test_name, double max_time_ms);
    void ValidatePerformance();

    // Continuous Integration Support
    int GetExitCode() const { return total_failed_ > 0 ? 1 : 0; }
    
private:
    struct TestEntry {
        std::string name;
        std::string component;
        TestLevel level;
        std::function<void()> test_func;
    };

    std::vector<TestEntry> tests_;
    std::vector<TestMetrics> results_;
    std::vector<CoverageInfo> coverage_data_;
    std::map<std::string, double> performance_benchmarks_;
    
    int total_passed_ = 0;
    int total_failed_ = 0;
    
    void RunTestsAtLevel(TestLevel level);
    void ExecuteTest(const TestEntry& test);
    std::string TestLevelToString(TestLevel level) const;
    void WriteHTMLReport(const std::string& filename);
    void WriteJSONReport(const std::string& filename);
};

// Test Assertion Macros
#define V_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + message + \
                                   " at " + __FILE__ + ":" + std::to_string(__LINE__)); \
        } \
    } while(0)

#define V_ASSERT_EQ(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            std::stringstream ss; \
            ss << "Expected: " << (expected) << ", Actual: " << (actual) << " - " << message; \
            throw std::runtime_error(ss.str()); \
        } \
    } while(0)

#define V_ASSERT_NEAR(expected, actual, tolerance, message) \
    do { \
        if (std::abs((expected) - (actual)) > (tolerance)) { \
            std::stringstream ss; \
            ss << "Expected: " << (expected) << " ± " << (tolerance) \
               << ", Actual: " << (actual) << " - " << message; \
            throw std::runtime_error(ss.str()); \
        } \
    } while(0)

// Test Registration Helpers
#define REGISTER_UNIT_TEST(name, component, test_func) \
    VModel::VModelTestFramework::Instance().RegisterUnitTest(name, component, test_func)

#define REGISTER_INTEGRATION_TEST(name, component, test_func) \
    VModel::VModelTestFramework::Instance().RegisterIntegrationTest(name, component, test_func)

#define REGISTER_SYSTEM_TEST(name, test_func) \
    VModel::VModelTestFramework::Instance().RegisterSystemTest(name, test_func)

#define REGISTER_ACCEPTANCE_TEST(name, test_func) \
    VModel::VModelTestFramework::Instance().RegisterAcceptanceTest(name, test_func)

} // namespace VModel
