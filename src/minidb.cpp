/**
 * @file minidb.cpp
 * @brief Main MiniDB library implementation
 */

#include "minidb/minidb.h"
#include <iostream>

namespace minidb {

    // Database implementation
    Database::Database(const std::string& name) 
        : db_name_(name), is_open_(false) {
    }
    
    bool Database::open() {
        if (is_open_) {
            return true;
        }
        
        // Initialize components
        page_manager_ = std::make_unique<PageManager>();
        executor_ = std::make_unique<QueryExecutor>(page_manager_.get());
        
        is_open_ = true;
        return true;
    }
    
    void Database::close() {
        if (is_open_) {
            executor_.reset();
            page_manager_.reset();
            tables_.clear();
            is_open_ = false;
        }
    }
    
    QueryResult Database::execute_query(const std::string& query) {
        if (!is_open_) {
            return query::QueryResult("Database is not open");
        }
        
        return executor_->execute_sql(query);
    }
    
    Table* Database::get_table(const std::string& name) {
        if (!is_open_) {
            return nullptr;
        }
        
        return executor_->get_table(name);
    }
    
    bool Database::create_table(const std::string& name, const TableSchema& schema) {
        if (!is_open_) {
            return false;
        }
        
        return executor_->create_table(name, schema);
    }
    
    bool Database::drop_table(const std::string& name) {
        if (!is_open_) {
            return false;
        }
        
        return executor_->drop_table(name);
    }
    
    // Library functions
    bool initialize() {
        // Initialize any global state if needed
        return true;
    }
    
    void cleanup() {
        // Cleanup any global state if needed
    }
    
    std::string get_version() {
        return "1.0.0";
    }

} // namespace minidb
