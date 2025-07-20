/**
 * @file simple_example.cpp
 * @brief Simple example of using MiniDB programmatically
 */

#include "minidb/minidb.h"
#include <iostream>

int main() {
    try {
        // Initialize MiniDB library
        if (!minidb::initialize()) {
            std::cerr << "Failed to initialize MiniDB" << std::endl;
            return 1;
        }
        
        // Create database instance
        minidb::Database db("example_db");
        
        // Open the database
        if (!db.open()) {
            std::cerr << "Failed to open database" << std::endl;
            return 1;
        }
        
        std::cout << "Database opened successfully!" << std::endl;
        std::cout << "Database name: " << db.get_name() << std::endl;
        
        // Execute some SQL commands
        std::cout << "\nCreating table..." << std::endl;
        auto result1 = db.execute_query("CREATE TABLE test (id INTEGER, name TEXT)");
        if (!result1.is_success()) {
            std::cout << "Error: " << result1.get_error() << std::endl;
        }
        
        std::cout << "\nInserting data..." << std::endl;
        auto result2 = db.execute_query("INSERT INTO test VALUES (1, 'Hello')");
        if (!result2.is_success()) {
            std::cout << "Error: " << result2.get_error() << std::endl;
        }
        
        auto result3 = db.execute_query("INSERT INTO test VALUES (2, 'World')");
        if (!result3.is_success()) {
            std::cout << "Error: " << result3.get_error() << std::endl;
        }
        
        std::cout << "\nQuerying data..." << std::endl;
        auto result4 = db.execute_query("SELECT * FROM test");
        if (result4.is_success()) {
            std::cout << "Query returned " << result4.row_count() << " rows" << std::endl;
            
            // Print results (simplified)
            const auto& rows = result4.get_rows();
            const auto& columns = result4.get_column_names();
            
            // Print column headers
            for (const auto& col : columns) {
                std::cout << col << "\t";
            }
            std::cout << std::endl;
            
            // Print rows
            for (const auto& row : rows) {
                for (size_t i = 0; i < row.size(); i++) {
                    std::cout << row.get_value(i).to_string() << "\t";
                }
                std::cout << std::endl;
            }
        } else {
            std::cout << "Error: " << result4.get_error() << std::endl;
        }
        
        // Close database
        db.close();
        std::cout << "\nDatabase closed." << std::endl;
        
        // Cleanup
        minidb::cleanup();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
