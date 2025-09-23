#pragma once

#include <string>
#include <functional>
#include <vector>

namespace Test {
    class TestRunner {
    public:
        struct TestResult { std::string name; bool passed; std::string error_message; double execution_time_ms; };
        static TestRunner& Instance();
        void AddTest(const std::string& name, std::function<void()> test_func);
        void RunAllTests();
        const std::vector<TestResult>& GetResults() const;
    };
}

// Reuse the same macros as test_main.cpp
#define ASSERT_TRUE(condition) if (!(condition)) throw std::runtime_error("Assertion failed: " #condition);
#define ASSERT_FALSE(condition) if (condition) throw std::runtime_error("Assertion failed: " #condition " should be false");
#define ASSERT_EQ(expected, actual) if ((expected) != (actual)) throw std::runtime_error("Assertion failed: expected mismatch");
#define ASSERT_NE(expected, actual) if ((expected) == (actual)) throw std::runtime_error("Assertion failed: should not equal");
#define ASSERT_STREQ(expected, actual) if (std::string(expected) != std::string(actual)) throw std::runtime_error("Assertion failed: string mismatch");

#define TEST(test_name) \
    void test_name(); \
    struct test_name##_registrar { \
        test_name##_registrar() { Test::TestRunner::Instance().AddTest(#test_name, test_name); } \
    }; \
    static test_name##_registrar test_name##_reg; \
    void test_name()
