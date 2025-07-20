/**
 * @file test_btree.cpp
 * @brief B-Tree tests
 */

#include "minidb/core/btree.h"
#include <iostream>

using namespace minidb::core;

bool test_btree_basic() {
    BTree<int, 5> btree;
    
    // Test empty tree
    if (!btree.empty()) return false;
    if (btree.size() != 0) return false;
    
    // Test single insert
    if (!btree.insert(10)) return false;
    if (btree.empty()) return false;
    if (btree.size() != 1) return false;
    
    return true;
}

bool test_btree_insert_search() {
    BTree<int, 5> btree;
    
    // Insert some values
    std::vector<int> values = {5, 2, 8, 1, 3, 7, 9, 4, 6, 10};
    
    for (int val : values) {
        if (!btree.insert(val)) return false;
    }
    
    // Search for all values
    for (int val : values) {
        if (!btree.search(val)) return false;
    }
    
    // Search for non-existent values
    if (btree.search(0)) return false;
    if (btree.search(11)) return false;
    
    // Check size
    if (btree.size() != values.size()) return false;
    
    return true;
}
