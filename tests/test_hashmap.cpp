/**
 * @file test_hashmap.cpp
 * @brief HashMap tests
 */

#include "minidb/core/hashmap.h"
#include <iostream>

using namespace minidb::core;

bool test_hashmap_basic() {
    HashMap<std::string, int> hashmap;
    
    // Test empty map
    if (!hashmap.empty()) return false;
    if (hashmap.size() != 0) return false;
    
    // Test single insert
    if (!hashmap.insert("key1", 100)) return false;
    if (hashmap.empty()) return false;
    if (hashmap.size() != 1) return false;
    
    return true;
}

bool test_hashmap_operations() {
    HashMap<std::string, int> hashmap;
    
    // Insert some key-value pairs
    if (!hashmap.insert("one", 1)) return false;
    if (!hashmap.insert("two", 2)) return false;
    if (!hashmap.insert("three", 3)) return false;
    
    // Test find
    const int* val = hashmap.find("two");
    if (!val || *val != 2) return false;
    
    // Test contains
    if (!hashmap.contains("one")) return false;
    if (hashmap.contains("four")) return false;
    
    // Test update
    if (!hashmap.update("one", 10)) return false;
    val = hashmap.find("one");
    if (!val || *val != 10) return false;
    
    // Test remove
    if (!hashmap.remove("two")) return false;
    if (hashmap.contains("two")) return false;
    if (hashmap.size() != 2) return false;
    
    return true;
}
