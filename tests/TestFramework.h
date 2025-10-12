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
    static auto Instance() -> TestRunner& {
        static TestRunner instance;
        return instance;
    }

    void AddTest(const std::string& name, std::function<void()> test_func) {
        tests_.emplace_back(name, test_func);
    }

    void RunAllTests() {
        std::cout << "Running " << tests_.size() << " tests...\n";

        int passed = 0;
        int failed = 0;

        for (const auto& test : tests_) {
            auto start = std::chrono::high_resolution_clock::now();

            try {
                test.second(); // Run test function

                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration<double, std::milli>(end - start).count();

                std::cout << "[PASS] " << test.first << " (" << duration << "ms)\n";

                results_.push_back({test.first, true, "", duration});
                passed++;

            } catch (const std::exception& e) {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration<double, std::milli>(end - start).count();

                std::cout << "[FAIL] " << test.first << " - " << e.what() << " (" << duration << "ms)\n";

                results_.push_back({test.first, false, e.what(), duration});
                failed++;

            } catch (...) {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration<double, std::milli>(end - start).count();

                std::cout << "[FAIL] " << test.first << " - Unknown exception (" << duration << "ms)\n";

                results_.push_back({test.first, false, "Unknown exception", duration});
                failed++;
            }
        }

    std::cout << "\nTest Results:\n";
    std::cout << "  Passed: " << passed << "\n";
    std::cout << "  Failed: " << failed << "\n";
    std::cout << "  Total:  " << (passed + failed) << "\n";

        if (failed > 0) {
            std::cout << "\nFailed tests:\n";
            for (const auto& result : results_) {
                if (!result.passed) {
                    std::cout << "  - " << result.name << ": " << result.error_message << "\n";
                }
            }
        }
    }

    [[nodiscard]] auto GetResults() const -> const std::vector<TestResult>& { return results_; }

private:
    std::vector<std::pair<std::string, std::function<void()>>> tests_;
    std::vector<TestResult> results_;
};

} // namespace Test

// Lightweight ToString helpers so assertions can stringify any printable type
namespace TestInternal {
    inline auto ToString(const std::string& str) -> std::string { return str; }
    inline auto ToString(const char* cstr) -> std::string { return cstr ? std::string(cstr) : std::string("(null)"); }
    template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    inline auto ToString(T value) -> std::string { return std::to_string(value); }
    template <typename T>
    inline auto ToStringFallback(const T& val) -> std::string {
        std::ostringstream oss; oss << val; return oss.str();
    }
    template <typename T>
    inline auto ToStringGeneric(const T& val) -> std::string {
        if constexpr (std::is_arithmetic_v<T>) return ToString(val);
        else if constexpr (std::is_same_v<T, std::string>) return ToString(val);
        else return ToStringFallback(val);
    }
}

// Typed assertion helpers (constexpr/template where applicable)
namespace Test {
    inline void AssertTrue(bool condition, const char* expr, const char* file, int line) {
        if (!condition) {
            throw std::runtime_error(std::string("Assertion failed: ") + expr + " at " + file + ":" + std::to_string(line));
        }
    }

    inline void AssertFalse(bool condition, const char* expr, const char* file, int line) {
        if (condition) {
            throw std::runtime_error(std::string("Assertion failed: ") + expr + " should be false at " + file + ":" + std::to_string(line));
        }
    }

    template <typename T, typename U>
    inline void AssertEq(const T& expected, const U& actual, const char* expected_expr, const char* actual_expr, const char* file, int line) {
        if (!(expected == actual)) {
            throw std::runtime_error(std::string("Assertion failed: expected ") + TestInternal::ToStringGeneric(expected)
                + std::string(" but got ") + TestInternal::ToStringGeneric(actual)
                + std::string(" (") + expected_expr + std::string(" vs ") + actual_expr + std::string(") at ") + file + std::string(":") + std::to_string(line));
        }
    }

    template <typename T, typename U>
    inline void AssertNe(const T& expected, const U& actual, const char* expected_expr, const char* actual_expr, const char* file, int line) {
        if (expected == actual) {
            throw std::runtime_error(std::string("Assertion failed: ") + TestInternal::ToStringGeneric(expected)
                + std::string(" should not equal ") + TestInternal::ToStringGeneric(actual)
                + std::string(" (") + expected_expr + std::string(" vs ") + actual_expr + std::string(") at ") + file + std::string(":") + std::to_string(line));
        }
    }

    inline void AssertStreq(const std::string& expected, const std::string& actual, const char* expected_expr, const char* actual_expr, const char* file, int line) {
        if (expected != actual) {
            throw std::runtime_error(std::string("Assertion failed: expected \"") + expected + std::string("\" but got \"") + actual + std::string("\" (") + expected_expr + std::string(" vs ") + actual_expr + std::string(") at ") + file + std::string(":") + std::to_string(line));
        }
    }
}

// Thin macro wrappers that forward to typed helpers (preserve expression stringification)
#define ASSERT_TRUE(condition) ::Test::AssertTrue((condition), #condition, __FILE__, __LINE__)
#define ASSERT_FALSE(condition) ::Test::AssertFalse((condition), #condition, __FILE__, __LINE__)
#define ASSERT_EQ(expected, actual) ::Test::AssertEq((expected), (actual), #expected, #actual, __FILE__, __LINE__)
#define ASSERT_NE(expected, actual) ::Test::AssertNe((expected), (actual), #expected, #actual, __FILE__, __LINE__)
#define ASSERT_STREQ(expected, actual) ::Test::AssertStreq(std::string(expected), std::string(actual), #expected, #actual, __FILE__, __LINE__)

// Test registration macro
#define TEST(test_name) \
    void test_name(); \
    struct test_name##_registrar { \
        test_name##_registrar() { Test::TestRunner::Instance().AddTest(#test_name, test_name); } \
    }; \
    static test_name##_registrar test_name##_reg; \
    void test_name()
