/**
 * @file table.cpp
 * @brief Table implementation - basic version
 */

#include "minidb/storage/table.h"
#include <algorithm>
#include <sstream>

namespace minidb {
namespace storage {

    // Column type to string conversion
    std::string column_type_to_string(ColumnType type) {
        switch (type) {
            case ColumnType::INTEGER: return "INTEGER";
            case ColumnType::TEXT: return "TEXT";
            case ColumnType::REAL: return "REAL";
            case ColumnType::BLOB: return "BLOB";
            case ColumnType::NULL_TYPE: return "NULL";
            default: return "UNKNOWN";
        }
    }
    
    // TableSchema implementation
    bool TableSchema::add_column(const Column& column) {
        // Check for duplicate column names
        if (column_indices_.find(column.name) != column_indices_.end()) {
            return false;
        }
        
        column_indices_[column.name] = columns_.size();
        columns_.push_back(column);
        return true;
    }
    
    const Column& TableSchema::get_column(size_t index) const {
        return columns_.at(index);
    }
    
    const Column* TableSchema::get_column(const std::string& name) const {
        auto it = column_indices_.find(name);
        if (it != column_indices_.end()) {
            return &columns_[it->second];
        }
        return nullptr;
    }
    
    size_t TableSchema::get_column_index(const std::string& name) const {
        auto it = column_indices_.find(name);
        if (it != column_indices_.end()) {
            return it->second;
        }
        return SIZE_MAX;
    }
    
    bool TableSchema::validate() const {
        if (columns_.empty()) {
            return false;
        }
        
        // Check for at most one primary key
        size_t primary_key_count = 0;
        for (const auto& column : columns_) {
            if (column.primary_key) {
                primary_key_count++;
            }
        }
        
        return primary_key_count <= 1;
    }
    
    // Value implementation
    std::string Value::to_string() const {
        if (is_null_) {
            return "NULL";
        }
        
        switch (type_) {
            case ColumnType::INTEGER:
                return std::to_string(int_val_);
            case ColumnType::TEXT:
                return str_val_;
            case ColumnType::REAL:
                return std::to_string(real_val_);
            case ColumnType::NULL_TYPE:
                return "NULL";
            default:
                return "";
        }
    }
    
    int Value::compare(const Value& other) const {
        // Handle null values
        if (is_null_ && other.is_null_) return 0;
        if (is_null_) return -1;
        if (other.is_null_) return 1;
        
        // Type mismatch comparison (simplified)
        if (type_ != other.type_) {
            return static_cast<int>(type_) - static_cast<int>(other.type_);
        }
        
        switch (type_) {
            case ColumnType::INTEGER:
                if (int_val_ < other.int_val_) return -1;
                if (int_val_ > other.int_val_) return 1;
                return 0;
                
            case ColumnType::TEXT:
                return str_val_.compare(other.str_val_);
                
            case ColumnType::REAL:
                if (real_val_ < other.real_val_) return -1;
                if (real_val_ > other.real_val_) return 1;
                return 0;
                
            default:
                return 0;
        }
    }
    
    // Index implementations (basic versions)
    bool BTreeIndex::insert(const Value& key, uint64_t row_id) {
        return btree_.insert(std::make_pair(key, row_id));
    }
    
    bool BTreeIndex::remove(const Value& key) {
        // Simplified - in full implementation would need to find exact pair
        return false;  // Not implemented for simplicity
    }
    
    uint64_t BTreeIndex::find(const Value& key) {
        // Simplified search
        bool found = btree_.search(std::make_pair(key, 0));
        return found ? 1 : 0;  // Simplified return
    }
    
    std::vector<uint64_t> BTreeIndex::range_query(const Value& start, const Value& end) {
        std::vector<uint64_t> results;
        // Simplified implementation
        return results;
    }
    
    bool HashIndex::insert(const Value& key, uint64_t row_id) {
        return hashmap_.insert(key, row_id);
    }
    
    bool HashIndex::remove(const Value& key) {
        return hashmap_.remove(key);
    }
    
    uint64_t HashIndex::find(const Value& key) {
        const uint64_t* result = hashmap_.find(key);
        return result ? *result : 0;
    }
    
    std::vector<uint64_t> HashIndex::range_query(const Value& start, const Value& end) {
        // Hash index doesn't support efficient range queries
        return std::vector<uint64_t>();
    }
    
    // Table implementation
    Table::Table(const TableSchema& schema, PageManager* page_manager)
        : schema_(schema), page_manager_(page_manager), next_row_id_(1) {
    }
    
    uint64_t Table::insert_row(const Row& row) {
        // Validate row has correct number of columns
        if (row.size() != schema_.column_count()) {
            return 0;  // Invalid row
        }
        
        // Create new row with assigned ID
        Row new_row = row;
        new_row.set_id(next_row_id_++);
        
        // Add to storage
        rows_.push_back(new_row);
        
        // Update indices
        for (auto& [column_name, index] : indices_) {
            size_t column_index = schema_.get_column_index(column_name);
            if (column_index != SIZE_MAX && column_index < row.size()) {
                index->insert(row.get_value(column_index), new_row.get_id());
            }
        }
        
        return new_row.get_id();
    }
    
    bool Table::update_row(uint64_t row_id, const Row& new_row) {
        // Find existing row
        auto it = std::find_if(rows_.begin(), rows_.end(),
            [row_id](const Row& r) { return r.get_id() == row_id; });
        
        if (it == rows_.end()) {
            return false;  // Row not found
        }
        
        // Update indices (remove old, add new)
        for (auto& [column_name, index] : indices_) {
            size_t column_index = schema_.get_column_index(column_name);
            if (column_index != SIZE_MAX) {
                if (column_index < it->size()) {
                    index->remove(it->get_value(column_index));
                }
                if (column_index < new_row.size()) {
                    index->insert(new_row.get_value(column_index), row_id);
                }
            }
        }
        
        // Update the row
        *it = new_row;
        it->set_id(row_id);  // Preserve row ID
        
        return true;
    }
    
    bool Table::delete_row(uint64_t row_id) {
        auto it = std::find_if(rows_.begin(), rows_.end(),
            [row_id](const Row& r) { return r.get_id() == row_id; });
        
        if (it == rows_.end()) {
            return false;  // Row not found
        }
        
        // Remove from indices
        for (auto& [column_name, index] : indices_) {
            size_t column_index = schema_.get_column_index(column_name);
            if (column_index != SIZE_MAX && column_index < it->size()) {
                index->remove(it->get_value(column_index));
            }
        }
        
        // Remove from storage
        rows_.erase(it);
        return true;
    }
    
    const Row* Table::get_row(uint64_t row_id) const {
        auto it = std::find_if(rows_.begin(), rows_.end(),
            [row_id](const Row& r) { return r.get_id() == row_id; });
        
        return (it != rows_.end()) ? &(*it) : nullptr;
    }
    
    bool Table::create_index(const std::string& column_name, const std::string& index_type) {
        // Check if column exists
        if (schema_.get_column(column_name) == nullptr) {
            return false;
        }
        
        // Check if index already exists
        if (indices_.find(column_name) != indices_.end()) {
            return false;
        }
        
        // Create appropriate index type
        std::unique_ptr<Index> index;
        if (index_type == "btree") {
            index = std::make_unique<BTreeIndex>();
        } else if (index_type == "hash") {
            index = std::make_unique<HashIndex>();
        } else {
            return false;  // Unknown index type
        }
        
        // Build index from existing data
        size_t column_index = schema_.get_column_index(column_name);
        for (const auto& row : rows_) {
            if (column_index < row.size()) {
                index->insert(row.get_value(column_index), row.get_id());
            }
        }
        
        indices_[column_name] = std::move(index);
        return true;
    }
    
    bool Table::drop_index(const std::string& column_name) {
        auto it = indices_.find(column_name);
        if (it != indices_.end()) {
            indices_.erase(it);
            return true;
        }
        return false;
    }
    
    void Table::clear() {
        rows_.clear();
        indices_.clear();
        next_row_id_ = 1;
    }

} // namespace storage
} // namespace minidb
