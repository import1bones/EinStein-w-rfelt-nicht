#pragma once

#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <sstream>
#include <type_traits>

namespace Test {

class TestRunner {
public:
    struct TestResult {
        std::string name;
        bool passed;
        std::string error_message;
        double execution_time_ms;
    };

    // Inline singleton instance so it's available to all translation units that include this header
    static TestRunner& Instance() {
        static TestRunner instance;
        return instance;
    }

    void AddTest(const std::string& name, std::function<void()> test_func) {
        tests_.emplace_back(name, test_func);
    }

    void RunAllTests() {
        std::cout << "Running " << tests_.size() << " tests...\n" << std::endl;

        int passed = 0;
        int failed = 0;

        for (const auto& test : tests_) {
            auto start = std::chrono::high_resolution_clock::now();

            try {
                test.second(); // Run test function

                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration<double, std::milli>(end - start).count();

                std::cout << "[PASS] " << test.first << " (" << duration << "ms)" << std::endl;

                results_.push_back({test.first, true, "", duration});
                passed++;

            } catch (const std::exception& e) {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration<double, std::milli>(end - start).count();

                std::cout << "[FAIL] " << test.first << " - " << e.what() << " (" << duration << "ms)" << std::endl;

                results_.push_back({test.first, false, e.what(), duration});
                failed++;

            } catch (...) {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration<double, std::milli>(end - start).count();

                std::cout << "[FAIL] " << test.first << " - Unknown exception (" << duration << "ms)" << std::endl;

                results_.push_back({test.first, false, "Unknown exception", duration});
                failed++;
            }
        }

        std::cout << "\nTest Results:" << std::endl;
        std::cout << "  Passed: " << passed << std::endl;
        std::cout << "  Failed: " << failed << std::endl;
        std::cout << "  Total:  " << (passed + failed) << std::endl;

        if (failed > 0) {
            std::cout << "\nFailed tests:" << std::endl;
            for (const auto& result : results_) {
                if (!result.passed) {
                    std::cout << "  - " << result.name << ": " << result.error_message << std::endl;
                }
            }
        }
    }

    const std::vector<TestResult>& GetResults() const { return results_; }

private:
    std::vector<std::pair<std::string, std::function<void()>>> tests_;
    std::vector<TestResult> results_;
};

} // namespace Test

// Lightweight ToString helpers so assertions can stringify any printable type
namespace TestInternal {
    inline std::string ToString(const std::string& s) { return s; }
    inline std::string ToString(const char* s) { return s ? std::string(s) : std::string("(null)"); }
    template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    inline std::string ToString(T v) { return std::to_string(v); }
    template <typename T>
    inline std::string ToStringFallback(const T& v) {
        std::ostringstream oss; oss << v; return oss.str();
    }
    template <typename T>
    inline std::string ToStringGeneric(const T& v) {
        if constexpr (std::is_arithmetic_v<T>) return ToString(v);
        else if constexpr (std::is_same_v<T, std::string>) return ToString(v);
        else return ToStringFallback(v);
    }
}

// Assertion macros
#define ASSERT_TRUE(condition) if (!(condition)) throw std::runtime_error("Assertion failed: " #condition);
#define ASSERT_FALSE(condition) if (condition) throw std::runtime_error("Assertion failed: " #condition " should be false");
#define ASSERT_EQ(expected, actual) if ((expected) != (actual)) throw std::runtime_error(std::string("Assertion failed: expected ") + TestInternal::ToStringGeneric(expected) + std::string(" but got ") + TestInternal::ToStringGeneric(actual));
#define ASSERT_NE(expected, actual) if ((expected) == (actual)) throw std::runtime_error(std::string("Assertion failed: ") + TestInternal::ToStringGeneric(expected) + std::string(" should not equal ") + TestInternal::ToStringGeneric(actual));
#define ASSERT_STREQ(expected, actual) if (std::string(expected) != std::string(actual)) throw std::runtime_error(std::string("Assertion failed: expected \"") + std::string(expected) + std::string("\" but got \"") + std::string(actual) + std::string("\""));

// Test registration macro
#define TEST(test_name) \
    void test_name(); \
    struct test_name##_registrar { \
        test_name##_registrar() { Test::TestRunner::Instance().AddTest(#test_name, test_name); } \
    }; \
    static test_name##_registrar test_name##_reg; \
    void test_name()
