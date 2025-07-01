#include "VModelTestFramework.h"
#include <algorithm>
#include <iomanip>

namespace VModel {

void VModelTestFramework::RegisterUnitTest(const std::string& name, const std::string& component, 
                                         std::function<void()> test_func) {
    tests_.push_back({name, component, TestLevel::Unit, test_func});
}

void VModelTestFramework::RegisterIntegrationTest(const std::string& name, const std::string& component,
                                                 std::function<void()> test_func) {
    tests_.push_back({name, component, TestLevel::Integration, test_func});
}

void VModelTestFramework::RegisterSystemTest(const std::string& name, std::function<void()> test_func) {
    tests_.push_back({name, "System", TestLevel::System, test_func});
}

void VModelTestFramework::RegisterAcceptanceTest(const std::string& name, std::function<void()> test_func) {
    tests_.push_back({name, "Acceptance", TestLevel::Acceptance, test_func});
}

void VModelTestFramework::RunUnitTests() {
    std::cout << "\n=== V-Model: Unit Tests ===" << std::endl;
    RunTestsAtLevel(TestLevel::Unit);
}

void VModelTestFramework::RunIntegrationTests() {
    std::cout << "\n=== V-Model: Integration Tests ===" << std::endl;
    RunTestsAtLevel(TestLevel::Integration);
}

void VModelTestFramework::RunSystemTests() {
    std::cout << "\n=== V-Model: System Tests ===" << std::endl;
    RunTestsAtLevel(TestLevel::System);
}

void VModelTestFramework::RunAcceptanceTests() {
    std::cout << "\n=== V-Model: Acceptance Tests ===" << std::endl;
    RunTestsAtLevel(TestLevel::Acceptance);
}

void VModelTestFramework::RunAllTests() {
    std::cout << "🔬 V-Model Test Framework - Einstein Game Refactor Validation" << std::endl;
    std::cout << "=================================================================" << std::endl;
    
    // Clear previous results
    results_.clear();
    total_passed_ = 0;
    total_failed_ = 0;
    
    // Run tests in V-Model order
    RunUnitTests();
    RunIntegrationTests();
    RunSystemTests();
    RunAcceptanceTests();
    
    // Generate summary
    PrintSummary();
    ValidatePerformance();
}

void VModelTestFramework::RunTestsForComponent(const std::string& component) {
    std::cout << "\n=== Testing Component: " << component << " ===" << std::endl;
    
    int component_passed = 0;
    int component_failed = 0;
    
    for (const auto& test : tests_) {
        if (test.component == component) {
            auto start_time = std::chrono::high_resolution_clock::now();
            TestMetrics metrics;
            metrics.test_name = test.name;
            metrics.level = test.level;
            metrics.component_tested = test.component;
            metrics.timestamp = std::chrono::system_clock::now();
            
            try {
                test.test_func();
                auto end_time = std::chrono::high_resolution_clock::now();
                metrics.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
                metrics.passed = true;
                
                std::cout << "  [PASS] [" << TestLevelToString(test.level) << "] " << test.name 
                         << " (" << std::fixed << std::setprecision(2) << metrics.execution_time_ms << "ms)" << std::endl;
                
                component_passed++;
                total_passed_++;
                
            } catch (const std::exception& e) {
                auto end_time = std::chrono::high_resolution_clock::now();
                metrics.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
                metrics.passed = false;
                metrics.error_message = e.what();
                
                std::cout << "  [FAIL] [" << TestLevelToString(test.level) << "] " << test.name 
                         << " - " << e.what() << std::endl;
                
                component_failed++;
                total_failed_++;
            }
            
            results_.push_back(metrics);
        }
    }
    
    std::cout << "Component Summary: " << component_passed << " passed, " 
              << component_failed << " failed" << std::endl;
}

void VModelTestFramework::RunTestsAtLevel(TestLevel level) {
    std::vector<std::string> components;
    
    // Collect unique components for this level
    for (const auto& test : tests_) {
        if (test.level == level && 
            std::find(components.begin(), components.end(), test.component) == components.end()) {
            components.push_back(test.component);
        }
    }
    
    // Run tests grouped by component
    for (const auto& component : components) {
        std::cout << "\n--- Component: " << component << " ---" << std::endl;
        
        for (const auto& test : tests_) {
            if (test.level == level && test.component == component) {
                ExecuteTest(test);
            }
        }
    }
}

void VModelTestFramework::ExecuteTest(const TestEntry& test) {
    auto start_time = std::chrono::high_resolution_clock::now();
    TestMetrics metrics;
    metrics.test_name = test.name;
    metrics.level = test.level;
    metrics.component_tested = test.component;
    metrics.timestamp = std::chrono::system_clock::now();
    
    try {
        test.test_func();
        auto end_time = std::chrono::high_resolution_clock::now();
        metrics.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        metrics.passed = true;
        
        std::cout << "  [PASS] " << test.name 
                 << " (" << std::fixed << std::setprecision(2) << metrics.execution_time_ms << "ms)" << std::endl;
        
        total_passed_++;
        
    } catch (const std::exception& e) {
        auto end_time = std::chrono::high_resolution_clock::now();
        metrics.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        metrics.passed = false;
        metrics.error_message = e.what();
        
        std::cout << "  [FAIL] " << test.name << " - " << e.what() << std::endl;
        
        total_failed_++;
    }
    
    results_.push_back(metrics);
}

void VModelTestFramework::PrintSummary() {
    std::cout << "\n📊 V-Model Test Summary" << std::endl;
    std::cout << "========================" << std::endl;
    
    // Count tests by level
    std::map<TestLevel, std::pair<int, int>> level_stats; // passed, failed
    
    for (const auto& result : results_) {
        if (result.passed) {
            level_stats[result.level].first++;
        } else {
            level_stats[result.level].second++;
        }
    }
    
    // Print level-wise summary
    for (const auto& level_stat : level_stats) {
        TestLevel level = level_stat.first;
        auto stats = level_stat.second;
        int passed = stats.first;
        int failed = stats.second;
        int total = passed + failed;
        double pass_rate = total > 0 ? (double)passed / total * 100.0 : 0.0;
        
        std::cout << TestLevelToString(level) << ": " 
                 << passed << "/" << total << " passed (" 
                 << std::fixed << std::setprecision(1) << pass_rate << "%)" << std::endl;
    }
    
    // Overall summary
    std::cout << "\nOverall: " << total_passed_ << "/" << (total_passed_ + total_failed_) 
              << " tests passed";
    
    if (total_passed_ + total_failed_ > 0) {
        double overall_rate = (double)total_passed_ / (total_passed_ + total_failed_) * 100.0;
        std::cout << " (" << std::fixed << std::setprecision(1) << overall_rate << "%)";
    }
    
    std::cout << std::endl;
    
    // Coverage information
    if (!coverage_data_.empty()) {
        double overall_coverage = GetOverallCoverage();
        std::cout << "Code Coverage: " << std::fixed << std::setprecision(1) 
                 << overall_coverage << "%" << std::endl;
    }
    
    std::cout << "\nTest Status: " << (total_failed_ == 0 ? "[PASS]" : "[FAIL]") << std::endl;
}

void VModelTestFramework::PrintDetailedResults() {
    std::cout << "\n📋 Detailed Test Results" << std::endl;
    std::cout << "=========================" << std::endl;
    
    for (const auto& result : results_) {
        std::cout << "[" << TestLevelToString(result.level) << "] " 
                 << result.component_tested << "::" << result.test_name << " - ";
        
        if (result.passed) {
            std::cout << "[PASS] (" << std::fixed << std::setprecision(2) 
                     << result.execution_time_ms << "ms)";
        } else {
            std::cout << "[FAIL] - " << result.error_message;
        }
        std::cout << std::endl;
    }
}

void VModelTestFramework::GenerateVModelReport() {
    std::cout << "\n🔄 V-Model Validation Report" << std::endl;
    std::cout << "=============================" << std::endl;
    
    std::cout << "Left Side (Development):     Right Side (Testing):" << std::endl;
    std::cout << "Requirements Analysis   <--> Acceptance Testing" << std::endl;
    std::cout << "System Design          <--> System Testing" << std::endl;
    std::cout << "Module Design          <--> Integration Testing" << std::endl;
    std::cout << "Implementation         <--> Unit Testing" << std::endl;
    
    std::cout << "\nValidation Status:" << std::endl;
    
    // Check each V-Model level
    auto check_level = [this](TestLevel level, const std::string& name) {
        int passed = 0, total = 0;
        for (const auto& result : results_) {
            if (result.level == level) {
                total++;
                if (result.passed) passed++;
            }
        }
        
        std::cout << "  " << name << ": ";
        if (total == 0) {
            std::cout << "[NONE] No tests defined";
        } else if (passed == total) {
            std::cout << "[PASS] Validated (" << passed << "/" << total << ")";
        } else {
            std::cout << "[FAIL] Issues found (" << passed << "/" << total << ")";
        }
        std::cout << std::endl;
    };
    
    check_level(TestLevel::Acceptance, "Requirements Validation");
    check_level(TestLevel::System, "System Validation");
    check_level(TestLevel::Integration, "Integration Validation");
    check_level(TestLevel::Unit, "Unit Validation");
}

std::string VModelTestFramework::TestLevelToString(TestLevel level) const {
    switch (level) {
        case TestLevel::Unit: return "UNIT";
        case TestLevel::Integration: return "INTEGRATION";
        case TestLevel::System: return "SYSTEM";
        case TestLevel::Acceptance: return "ACCEPTANCE";
        default: return "UNKNOWN";
    }
}

void VModelTestFramework::SetCoverageData(const std::vector<CoverageInfo>& coverage) {
    coverage_data_ = coverage;
}

double VModelTestFramework::GetOverallCoverage() const {
    if (coverage_data_.empty()) return 0.0;
    
    int total_lines = 0;
    int covered_lines = 0;
    
    for (const auto& info : coverage_data_) {
        total_lines += info.total_lines;
        covered_lines += info.lines_covered;
    }
    
    return total_lines > 0 ? (double)covered_lines / total_lines * 100.0 : 0.0;
}

void VModelTestFramework::SetPerformanceBenchmark(const std::string& test_name, double max_time_ms) {
    performance_benchmarks_[test_name] = max_time_ms;
}

void VModelTestFramework::ValidatePerformance() {
    if (performance_benchmarks_.empty()) return;
    
    std::cout << "\n⚡ Performance Validation" << std::endl;
    std::cout << "=========================" << std::endl;
    
    bool all_passed = true;
    
    for (const auto& benchmark : performance_benchmarks_) {
        const std::string& test_name = benchmark.first;
        double max_time = benchmark.second;
        auto it = std::find_if(results_.begin(), results_.end(), 
                              [&test_name](const TestMetrics& m) { return m.test_name == test_name; });
        
        if (it != results_.end()) {
            bool within_benchmark = it->execution_time_ms <= max_time;
            std::cout << "  " << test_name << ": " 
                     << std::fixed << std::setprecision(2) << it->execution_time_ms 
                     << "ms (max: " << max_time << "ms) ";
            
            if (within_benchmark) {
                std::cout << "[OK]";
            } else {
                std::cout << "[SLOW]";
                all_passed = false;
            }
            std::cout << std::endl;
        }
    }
    
    std::cout << "Performance Status: " << (all_passed ? "[PASS]" : "[FAIL]") << std::endl;
}

void VModelTestFramework::GenerateReport(const std::string& output_file) {
    if (output_file.empty()) {
        PrintDetailedResults();
        GenerateVModelReport();
        return;
    }
    
    if (output_file.length() >= 5 && output_file.substr(output_file.length() - 5) == ".html") {
        WriteHTMLReport(output_file);
    } else if (output_file.length() >= 5 && output_file.substr(output_file.length() - 5) == ".json") {
        WriteJSONReport(output_file);
    } else {
        // Default to text report
        std::ofstream file(output_file);
        if (file.is_open()) {
            // Redirect cout to file temporarily
            std::streambuf* orig = std::cout.rdbuf();
            std::cout.rdbuf(file.rdbuf());
            
            PrintDetailedResults();
            GenerateVModelReport();
            
            std::cout.rdbuf(orig);
            file.close();
            
            std::cout << "Report generated: " << output_file << std::endl;
        }
    }
}

void VModelTestFramework::WriteHTMLReport(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    
    file << "<!DOCTYPE html>\n<html>\n<head>\n"
         << "<title>V-Model Test Report - Einstein Game</title>\n"
         << "<style>\n"
         << "body { font-family: Arial, sans-serif; margin: 20px; }\n"
         << ".passed { color: green; }\n"
         << ".failed { color: red; }\n"
         << "table { border-collapse: collapse; width: 100%; }\n"
         << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n"
         << "th { background-color: #f2f2f2; }\n"
         << "</style>\n</head>\n<body>\n";
    
    file << "<h1>V-Model Test Report</h1>\n";
    file << "<h2>Summary</h2>\n";
    file << "<p>Total: " << (total_passed_ + total_failed_) << " tests, "
         << "Passed: " << total_passed_ << ", Failed: " << total_failed_ << "</p>\n";
    
    file << "<h2>Test Results</h2>\n";
    file << "<table>\n<tr><th>Level</th><th>Component</th><th>Test</th><th>Status</th><th>Time (ms)</th><th>Error</th></tr>\n";
    
    for (const auto& result : results_) {
        file << "<tr><td>" << TestLevelToString(result.level) << "</td>"
             << "<td>" << result.component_tested << "</td>"
             << "<td>" << result.test_name << "</td>"
             << "<td class=\"" << (result.passed ? "passed" : "failed") << "\">"
             << (result.passed ? "PASS" : "FAIL") << "</td>"
             << "<td>" << std::fixed << std::setprecision(2) << result.execution_time_ms << "</td>"
             << "<td>" << result.error_message << "</td></tr>\n";
    }
    
    file << "</table>\n</body>\n</html>\n";
    file.close();
    
    std::cout << "HTML report generated: " << filename << std::endl;
}

void VModelTestFramework::WriteJSONReport(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    
    file << "{\n";
    file << "  \"summary\": {\n";
    file << "    \"total\": " << (total_passed_ + total_failed_) << ",\n";
    file << "    \"passed\": " << total_passed_ << ",\n";
    file << "    \"failed\": " << total_failed_ << ",\n";
    file << "    \"coverage\": " << std::fixed << std::setprecision(2) << GetOverallCoverage() << "\n";
    file << "  },\n";
    file << "  \"results\": [\n";
    
    for (size_t i = 0; i < results_.size(); ++i) {
        const auto& result = results_[i];
        file << "    {\n";
        file << "      \"name\": \"" << result.test_name << "\",\n";
        file << "      \"level\": \"" << TestLevelToString(result.level) << "\",\n";
        file << "      \"component\": \"" << result.component_tested << "\",\n";
        file << "      \"passed\": " << (result.passed ? "true" : "false") << ",\n";
        file << "      \"execution_time_ms\": " << std::fixed << std::setprecision(2) << result.execution_time_ms << ",\n";
        file << "      \"error_message\": \"" << result.error_message << "\"\n";
        file << "    }" << (i < results_.size() - 1 ? "," : "") << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    file.close();
    
    std::cout << "JSON report generated: " << filename << std::endl;
}

} // namespace VModel
