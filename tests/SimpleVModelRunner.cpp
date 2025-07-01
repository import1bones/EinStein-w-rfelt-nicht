#include "framework/VModelTestFramework.h"
#include <iostream>
#include <string>

// Simple test implementations to verify the framework works
namespace VModel {
namespace SimpleTests {

void TestFrameworkBasics() {
    V_ASSERT(true, "Basic assertion should work");
    V_ASSERT_EQ(1, 1, "Equality assertion should work");
    V_ASSERT_NEAR(1.0, 1.1, 0.2, "Near assertion should work");
}

void TestStringOperations() {
    std::string test = "hello";
    V_ASSERT(test.length() == 5, "String length should be correct");
    V_ASSERT(test == "hello", "String equality should work");
}

void TestMathOperations() {
    int a = 5;
    int b = 3;
    V_ASSERT(a + b == 8, "Addition should work");
    V_ASSERT(a * b == 15, "Multiplication should work");
}

void RegisterSimpleTests() {
    auto& framework = VModelTestFramework::Instance();
    
    framework.RegisterUnitTest("Framework_Basics", "Framework", TestFrameworkBasics);
    framework.RegisterUnitTest("String_Operations", "Utility", TestStringOperations);
    framework.RegisterUnitTest("Math_Operations", "Utility", TestMathOperations);
    
    // Performance benchmarks
    framework.SetPerformanceBenchmark("Framework_Basics", 10.0);
    framework.SetPerformanceBenchmark("String_Operations", 5.0);
    framework.SetPerformanceBenchmark("Math_Operations", 5.0);
}

} // namespace SimpleTests
} // namespace VModel

int main(int argc, char* argv[]) {
    std::cout << "*** V-Model Test Framework for Einstein Game Refactor ***" << std::endl;
    std::cout << "====================================================" << std::endl;
    
    // Register simple tests to verify framework works
    VModel::SimpleTests::RegisterSimpleTests();
    
    auto& framework = VModel::VModelTestFramework::Instance();
    
    // Set up mock coverage data
    std::vector<VModel::CoverageInfo> coverage = {
        {"Framework", 10, 10, 100.0},
        {"Utility", 8, 10, 80.0},
        {"Core", 0, 50, 0.0}
    };
    framework.SetCoverageData(coverage);
    
    // Parse command line arguments
    bool run_specific_level = false;
    bool generate_report = false;
    std::string report_file;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--unit") {
            framework.RunUnitTests();
            run_specific_level = true;
        } else if (arg == "--integration") {
            framework.RunIntegrationTests();
            run_specific_level = true;
        } else if (arg == "--system") {
            framework.RunSystemTests();
            run_specific_level = true;
        } else if (arg == "--acceptance") {
            framework.RunAcceptanceTests();
            run_specific_level = true;
        } else if (arg == "--report" && i + 1 < argc) {
            generate_report = true;
            report_file = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --unit                Run only unit tests" << std::endl;
            std::cout << "  --integration         Run only integration tests" << std::endl;
            std::cout << "  --system             Run only system tests" << std::endl;
            std::cout << "  --acceptance         Run only acceptance tests" << std::endl;
            std::cout << "  --report <file>      Generate report to file" << std::endl;
            std::cout << "  --help, -h           Show this help message" << std::endl;
            return 0;
        }
    }
    
    // Run tests based on arguments
    if (!run_specific_level) {
        // Run all tests if no specific level was requested
        framework.RunAllTests();
    }
    
    // Generate report if requested
    if (generate_report) {
        framework.GenerateReport(report_file);
    } else {
        // Always show V-Model validation report
        framework.GenerateVModelReport();
    }
    
    // Print final summary
    std::cout << "\n*** V-Model Testing Complete ***" << std::endl;
    std::cout << "============================" << std::endl;
    
    int exit_code = framework.GetExitCode();
    if (exit_code == 0) {
        std::cout << "[SUCCESS] Framework validation successful!" << std::endl;
        std::cout << "\nNext Steps:" << std::endl;
        std::cout << "  1. Implement ChessBoard and core components" << std::endl;
        std::cout << "  2. Add more comprehensive unit tests" << std::endl;
        std::cout << "  3. Implement integration and system tests" << std::endl;
        std::cout << "  4. Add acceptance tests for requirements validation" << std::endl;
    } else {
        std::cout << "[FAILED] Some tests failed - Review results and fix issues" << std::endl;
    }
    
    return exit_code;
}
