#include "framework/VModelTestFramework.h"
#include "unit/CoreUnitTests.cpp"
#include "integration/IntegrationTests.cpp"
#include "system/SystemTests.cpp"
#include "acceptance/AcceptanceTests.cpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "*** V-Model Test Framework for Einstein Game Refactor ***" << std::endl;
    std::cout << "====================================================" << std::endl;
    
    // Register all tests
    VModel::UnitTests::RegisterUnitTests();
    VModel::IntegrationTests::RegisterIntegrationTests();
    VModel::SystemTests::RegisterSystemTests();
    VModel::AcceptanceTests::RegisterAcceptanceTests();
    
    auto& framework = VModel::VModelTestFramework::Instance();
    
    // Set up coverage data (mock data for demonstration)
    std::vector<VModel::CoverageInfo> coverage = {
        {"Core", 85, 100, 85.0},
        {"ChessBoard", 78, 90, 86.7},
        {"Config", 65, 75, 86.7},
        {"AI", 40, 80, 50.0},
        {"Graphics", 20, 120, 16.7}
    };
    framework.SetCoverageData(coverage);
    
    // Parse command line arguments
    bool run_specific_level = false;
    bool generate_report = false;
    bool run_component = false;
    std::string report_file;
    std::string component_name;
    
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
        } else if (arg == "--component" && i + 1 < argc) {
            run_component = true;
            component_name = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --unit                Run only unit tests" << std::endl;
            std::cout << "  --integration         Run only integration tests" << std::endl;
            std::cout << "  --system             Run only system tests" << std::endl;
            std::cout << "  --acceptance         Run only acceptance tests" << std::endl;
            std::cout << "  --component <name>   Run tests for specific component" << std::endl;
            std::cout << "  --report <file>      Generate report to file (.html, .json, or .txt)" << std::endl;
            std::cout << "  --help, -h           Show this help message" << std::endl;
            std::cout << std::endl;
            std::cout << "V-Model Testing Levels:" << std::endl;
            std::cout << "  Unit         - Individual component testing" << std::endl;
            std::cout << "  Integration  - Module interaction testing" << std::endl;
            std::cout << "  System       - End-to-end system testing" << std::endl;
            std::cout << "  Acceptance   - Requirements validation testing" << std::endl;
            return 0;
        }
    }
    
    // Run tests based on arguments
    if (run_component) {
        framework.RunTestsForComponent(component_name);
    } else if (!run_specific_level) {
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
        std::cout << "[SUCCESS] All tests passed - Refactor validation successful!" << std::endl;
        std::cout << "\nNext Steps:" << std::endl;
        std::cout << "  1. Fix any remaining implementation issues" << std::endl;
        std::cout << "  2. Improve code coverage in AI and Graphics components" << std::endl;
        std::cout << "  3. Add more integration tests for complex scenarios" << std::endl;
        std::cout << "  4. Performance optimization based on benchmarks" << std::endl;
    } else {
        std::cout << "[FAILED] Some tests failed - Review results and fix issues" << std::endl;
        std::cout << "\nActions Required:" << std::endl;
        std::cout << "  1. Review failed test details above" << std::endl;
        std::cout << "  2. Fix implementation or design issues" << std::endl;
        std::cout << "  3. Re-run tests to validate fixes" << std::endl;
        std::cout << "  4. Ensure all V-Model levels are satisfied" << std::endl;
    }
    
    std::cout << "\nFor detailed analysis, run with --report output.html" << std::endl;
    
    return exit_code;
}

// Version and build information
extern "C" const char* GetTestFrameworkVersion() {
    return "V-Model Test Framework v1.0.0 for Einstein Game Refactor";
}

extern "C" const char* GetTestFrameworkInfo() {
    return "Comprehensive V-Model testing framework implementing:\n"
           "- Unit Tests: Component-level validation\n"
           "- Integration Tests: Module interaction validation\n"
           "- System Tests: End-to-end functionality validation\n"
           "- Acceptance Tests: Requirements compliance validation\n"
           "\nBuilt for cross-platform C++17 compatibility with performance monitoring.";
}
