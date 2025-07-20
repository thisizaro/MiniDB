/**
 * @file btree.cpp
 * @brief B-Tree implementation
 */

#include "minidb/core/btree.h"
#include <algorithm>
#include <iostream>

namespace minidb {
namespace core {

    // Explicit template instantiations for common types
    template class BTree<int, 5>;
    template class BTree<std::string, 5>;
    template class BTree<std::pair<int, uint64_t>, 5>;
    template class BTree<std::pair<std::string, uint64_t>, 5>;
    
    // BTree implementation
    template<typename T, size_t Order>
    bool BTree<T, Order>::insert(const T& key) {
        // Check if key already exists
        if (search(key)) {
            return false;  // Key already exists
        }
        
        NodePtr root = root_;
        
        // If root is full, create new root and split
        if (root->is_full()) {
            NodePtr new_root = std::make_shared<BTreeNode<T, Order>>(false);
            new_root->children.push_back(root_);
            root_->parent = new_root;
            
            split_child(new_root, 0);
            root_ = new_root;
        }
        
        insert_non_full(root_, key);
        size_++;
        return true;
    }
    
    template<typename T, size_t Order>
    bool BTree<T, Order>::search(const T& key) const {
        return search_helper(root_, key) != nullptr;
    }
    
    template<typename T, size_t Order>
    void BTree<T, Order>::split_child(NodePtr parent, size_t child_index) {
        NodePtr full_child = parent->children[child_index];
        NodePtr new_child = std::make_shared<BTreeNode<T, Order>>(full_child->is_leaf);
        
        size_t mid_index = (Order - 1) / 2;
        
        // Copy second half of keys to new child
        for (size_t i = mid_index + 1; i < Order - 1; i++) {
            new_child->keys.push_back(full_child->keys[i]);
            new_child->key_count++;
        }
        
        // If not leaf, copy second half of children
        if (!full_child->is_leaf) {
            for (size_t i = mid_index + 1; i < Order; i++) {
                new_child->children.push_back(full_child->children[i]);
                full_child->children[i]->parent = new_child;
            }
            full_child->children.resize(mid_index + 1);
        }
        
        // Shrink original child
        full_child->keys.resize(mid_index);
        full_child->key_count = mid_index;
        
        // Insert middle key into parent
        T middle_key = full_child->keys.back();
        full_child->keys.pop_back();
        full_child->key_count--;
        
        // Insert new child into parent
        parent->children.insert(parent->children.begin() + child_index + 1, new_child);
        parent->keys.insert(parent->keys.begin() + child_index, middle_key);
        parent->key_count++;
        
        // Set parent pointers
        new_child->parent = parent;
        full_child->parent = parent;
    }
    
    template<typename T, size_t Order>
    void BTree<T, Order>::insert_non_full(NodePtr node, const T& key) {
        int i = static_cast<int>(node->key_count) - 1;
        
        if (node->is_leaf) {
            // Insert into leaf node
            node->keys.resize(node->key_count + 1);
            
            // Shift keys to make room
            while (i >= 0 && compare_(key, node->keys[i]) < 0) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            
            node->keys[i + 1] = key;
            node->key_count++;
        } else {
            // Find child to insert into
            while (i >= 0 && compare_(key, node->keys[i]) < 0) {
                i--;
            }
            i++;
            
            // If child is full, split it
            if (node->children[i]->is_full()) {
                split_child(node, i);
                if (compare_(key, node->keys[i]) > 0) {
                    i++;
                }
            }
            
            insert_non_full(node->children[i], key);
        }
    }
    
    template<typename T, size_t Order>
    auto BTree<T, Order>::search_helper(NodePtr node, const T& key) const -> NodePtr {
        size_t i = 0;
        
        // Find first key >= search key
        while (i < node->key_count && compare_(key, node->keys[i]) > 0) {
            i++;
        }
        
        // If key found
        if (i < node->key_count && compare_(key, node->keys[i]) == 0) {
            return node;
        }
        
        // If leaf, key not found
        if (node->is_leaf) {
            return nullptr;
        }
        
        // Recurse to appropriate child
        return search_helper(node->children[i], key);
    }
    
    template<typename T, size_t Order>
    bool BTree<T, Order>::remove(const T& key) {
        // Simplified remove implementation
        // In a complete implementation, this would handle all cases
        // including merging and borrowing
        
        // For now, just mark as not implemented
        return false;
    }
    
    template<typename T, size_t Order>
    void BTree<T, Order>::clear() {
        root_ = std::make_shared<BTreeNode<T, Order>>(true);
        size_ = 0;
    }
    
    template<typename T, size_t Order>
    void BTree<T, Order>::print() const {
        if (root_) {
            print_helper(root_);
        }
    }
    
    template<typename T, size_t Order>
    void BTree<T, Order>::print_helper(NodePtr node, int level) const {
        if (!node) return;
        
        std::cout << "Level " << level << ": ";
        for (size_t i = 0; i < node->key_count; i++) {
            std::cout << node->keys[i] << " ";
        }
        std::cout << "\n";
        
        if (!node->is_leaf) {
            for (size_t i = 0; i <= node->key_count; i++) {
                if (i < node->children.size()) {
                    print_helper(node->children[i], level + 1);
                }
            }
        }
    }
    
    template<typename T, size_t Order>
    template<typename Visitor>
    void BTree<T, Order>::traverse(Visitor visitor) const {
        if (root_) {
            traverse_helper(root_, visitor);
        }
    }
    
    template<typename T, size_t Order>
    template<typename Visitor>
    void BTree<T, Order>::traverse_helper(NodePtr node, Visitor visitor) const {
        if (!node) return;
        
        size_t i;
        for (i = 0; i < node->key_count; i++) {
            if (!node->is_leaf) {
                traverse_helper(node->children[i], visitor);
            }
            visitor(node->keys[i]);
        }
        
        if (!node->is_leaf) {
            traverse_helper(node->children[i], visitor);
        }
    }
    
    template<typename T, size_t Order>
    std::vector<T> BTree<T, Order>::range_query(const T& start, const T& end) const {
        std::vector<T> result;
        
        traverse([&](const T& key) {
            if (compare_(key, start) >= 0 && compare_(key, end) <= 0) {
                result.push_back(key);
            }
        });
        
        return result;
    }
    
    template<typename T, size_t Order>
    T BTree<T, Order>::min() const {
        if (empty()) {
            return T{};
        }
        
        NodePtr current = root_;
        while (!current->is_leaf) {
            current = current->children[0];
        }
        
        return current->keys[0];
    }
    
    template<typename T, size_t Order>
    T BTree<T, Order>::max() const {
        if (empty()) {
            return T{};
        }
        
        NodePtr current = root_;
        while (!current->is_leaf) {
            current = current->children[current->key_count];
        }
        
        return current->keys[current->key_count - 1];
    }

} // namespace core
} // namespace minidb
