/**
 * @file test_main.cpp
 * @brief Simple test runner for MiniDB
 */

#include <iostream>
#include <string>
#include <functional>
#include <vector>

// Simple test framework
struct TestCase {
    std::string name;
    std::function<bool()> test_func;
};

static std::vector<TestCase> tests;

void add_test(const std::string& name, std::function<bool()> test_func) {
    tests.push_back({name, test_func});
}

#define TEST(name) \
    static bool test_##name(); \
    static bool __unused_##name = (add_test(#name, test_##name), true); \
    static bool test_##name()

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::cout << "ASSERT_TRUE failed: " << #condition << std::endl; \
            return false; \
        } \
    } while (0)

#define ASSERT_FALSE(condition) \
    do { \
        if ((condition)) { \
            std::cout << "ASSERT_FALSE failed: " << #condition << std::endl; \
            return false; \
        } \
    } while (0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::cout << "ASSERT_EQ failed: expected " << (expected) \
                     << ", got " << (actual) << std::endl; \
            return false; \
        } \
    } while (0)

// Forward declarations for test functions
extern bool test_btree_basic();
extern bool test_btree_insert_search();
extern bool test_hashmap_basic();
extern bool test_hashmap_operations();

int main() {
    std::cout << "Running MiniDB tests...\n\n";
    
    // Register external tests
    add_test("btree_basic", test_btree_basic);
    add_test("btree_insert_search", test_btree_insert_search);
    add_test("hashmap_basic", test_hashmap_basic);
    add_test("hashmap_operations", test_hashmap_operations);
    
    int passed = 0;
    int failed = 0;
    
    for (const auto& test : tests) {
        std::cout << "Running test: " << test.name << "... ";
        
        try {
            if (test.test_func()) {
                std::cout << "PASSED\n";
                passed++;
            } else {
                std::cout << "FAILED\n";
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "FAILED (exception: " << e.what() << ")\n";
            failed++;
        } catch (...) {
            std::cout << "FAILED (unknown exception)\n";
            failed++;
        }
    }
    
    std::cout << "\nTest Results:\n";
    std::cout << "  Passed: " << passed << "\n";
    std::cout << "  Failed: " << failed << "\n";
    std::cout << "  Total:  " << (passed + failed) << "\n";
    
    return (failed == 0) ? 0 : 1;
}

// Basic library test
TEST(library_initialization) {
    ASSERT_TRUE(true);  // Placeholder
    return true;
}
