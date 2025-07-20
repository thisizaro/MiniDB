/**
 * @file executor.cpp
 * @brief Basic query executor implementation
 */

#include "minidb/query/executor.h"
#include "minidb/query/parser.h"
#include <algorithm>

namespace minidb {
namespace query {

    // QueryResult implementation (constructors already in header)
    
    // Plan node implementations
    QueryResult TableScanNode::execute() {
        std::vector<storage::Row> result_rows;
        const auto& all_rows = table_->get_all_rows();
        
        for (const auto& row : all_rows) {
            bool include = true;
            
            if (filter_) {
                storage::Value filter_result = filter_->evaluate(row, table_->get_schema());
                include = (filter_result.get_type() == storage::ColumnType::INTEGER && 
                          filter_result.get_int() != 0);
            }
            
            if (include) {
                result_rows.push_back(row);
            }
        }
        
        // Create column names for all columns
        std::vector<std::string> column_names;
        const auto& schema = table_->get_schema();
        for (size_t i = 0; i < schema.column_count(); i++) {
            column_names.push_back(schema.get_column(i).name);
        }
        
        return QueryResult(result_rows, column_names);
    }
    
    double TableScanNode::get_cost() const {
        return static_cast<double>(table_->row_count());  // Linear scan cost
    }
    
    QueryResult ProjectionNode::execute() {
        QueryResult child_result = child_->execute();
        
        if (!child_result.is_success()) {
            return child_result;
        }
        
        const auto& input_rows = child_result.get_rows();
        const auto& input_columns = child_result.get_column_names();
        std::vector<storage::Row> result_rows;
        
        // Determine column indices to project
        std::vector<size_t> column_indices;
        std::vector<std::string> result_columns;
        
        if (columns_.empty()) {
            // Project all columns
            for (size_t i = 0; i < input_columns.size(); i++) {
                column_indices.push_back(i);
                result_columns.push_back(input_columns[i]);
            }
        } else {
            // Project specified columns
            const auto& schema = table_->get_schema();
            for (const auto& col_name : columns_) {
                size_t index = schema.get_column_index(col_name);
                if (index != SIZE_MAX) {
                    column_indices.push_back(index);
                    result_columns.push_back(col_name);
                }
            }
        }
        
        // Project rows
        for (const auto& input_row : input_rows) {
            storage::Row result_row;
            result_row.set_id(input_row.get_id());
            
            for (size_t col_idx : column_indices) {
                if (col_idx < input_row.size()) {
                    result_row.add_value(input_row.get_value(col_idx));
                } else {
                    result_row.add_value(storage::Value());  // NULL
                }
            }
            
            result_rows.push_back(result_row);
        }
        
        return QueryResult(result_rows, result_columns);
    }
    
    double ProjectionNode::get_cost() const {
        return child_->get_cost();  // Same as child cost
    }
    
    QueryResult InsertNode::execute() {
        uint64_t row_id = table_->insert_row(row_);
        if (row_id == 0) {
            return QueryResult("Failed to insert row");
        }
        
        return QueryResult(1);  // 1 row affected
    }
    
    double InsertNode::get_cost() const {
        return 1.0;  // Constant cost
    }
    
    // Query planner implementation
    std::unique_ptr<PlanNode> QueryPlanner::create_plan(const Statement* stmt) {
        switch (stmt->get_type()) {
            case StatementType::SELECT:
                return plan_select(static_cast<const SelectStatement*>(stmt));
            case StatementType::INSERT:
                return plan_insert(static_cast<const InsertStatement*>(stmt));
            case StatementType::UPDATE:
                return plan_update(static_cast<const UpdateStatement*>(stmt));
            case StatementType::DELETE:
                return plan_delete(static_cast<const DeleteStatement*>(stmt));
            default:
                return nullptr;
        }
    }
    
    std::unique_ptr<PlanNode> QueryPlanner::plan_select(const SelectStatement* stmt) {
        auto table_it = tables_->find(stmt->get_table_name());
        if (table_it == tables_->end()) {
            return nullptr;  // Table not found
        }
        
        storage::Table* table = table_it->second;
        
        // Create table scan node
        std::unique_ptr<Expression> filter;
        if (stmt->get_where_clause()) {
            filter = stmt->get_where_clause()->clone();
        }
        
        auto scan_node = std::make_unique<TableScanNode>(table, std::move(filter));
        
        // Add projection if specific columns requested
        if (!stmt->is_select_all()) {
            return std::make_unique<ProjectionNode>(
                std::move(scan_node), stmt->get_columns(), table);
        }
        
        return scan_node;
    }
    
    std::unique_ptr<PlanNode> QueryPlanner::plan_insert(const InsertStatement* stmt) {
        auto table_it = tables_->find(stmt->get_table_name());
        if (table_it == tables_->end()) {
            return nullptr;  // Table not found
        }
        
        storage::Table* table = table_it->second;
        const auto& values = stmt->get_values();
        
        // Create row from values
        storage::Row row;
        for (const auto& value : values) {
            row.add_value(value);
        }
        
        return std::make_unique<InsertNode>(table, row);
    }
    
    std::unique_ptr<PlanNode> QueryPlanner::plan_update(const UpdateStatement* stmt) {
        // Not implemented for simplicity
        return nullptr;
    }
    
    std::unique_ptr<PlanNode> QueryPlanner::plan_delete(const DeleteStatement* stmt) {
        // Not implemented for simplicity
        return nullptr;
    }
    
    // Query executor implementation
    QueryResult QueryExecutor::execute(const Statement* stmt) {
        switch (stmt->get_type()) {
            case StatementType::CREATE_TABLE: {
                const auto* create_stmt = static_cast<const CreateTableStatement*>(stmt);
                storage::TableSchema schema(create_stmt->get_table_name());
                
                for (const auto& column : create_stmt->get_columns()) {
                    schema.add_column(column);
                }
                
                if (create_table(create_stmt->get_table_name(), schema)) {
                    return QueryResult(0);  // Success, 0 rows affected
                } else {
                    return QueryResult("Failed to create table");
                }
            }
            
            case StatementType::DROP_TABLE: {
                const auto* drop_stmt = static_cast<const DropTableStatement*>(stmt);
                if (drop_table(drop_stmt->get_table_name())) {
                    return QueryResult(0);  // Success, 0 rows affected
                } else {
                    return QueryResult("Failed to drop table");
                }
            }
            
            default: {
                // For other statements, use the planner
                auto plan = planner_.create_plan(stmt);
                if (!plan) {
                    return QueryResult("Failed to create execution plan");
                }
                
                return plan->execute();
            }
        }
    }
    
    QueryResult QueryExecutor::execute_sql(const std::string& sql) {
        Parser parser;
        auto stmt = parser.parse(sql);
        
        if (!stmt) {
            return QueryResult("Parse error: " + parser.get_error());
        }
        
        return execute(stmt.get());
    }
    
    bool QueryExecutor::create_table(const std::string& name, const storage::TableSchema& schema) {
        if (table_refs_.find(name) != table_refs_.end()) {
            return false;  // Table already exists
        }
        
        auto table = std::make_unique<storage::Table>(schema, page_manager_);
        storage::Table* table_ptr = table.get();
        
        tables_[name] = std::move(table);
        table_refs_[name] = table_ptr;
        
        return true;
    }
    
    bool QueryExecutor::drop_table(const std::string& name) {
        auto it = tables_.find(name);
        if (it != tables_.end()) {
            table_refs_.erase(name);
            tables_.erase(it);
            return true;
        }
        return false;
    }
    
    storage::Table* QueryExecutor::get_table(const std::string& name) {
        auto it = table_refs_.find(name);
        if (it != table_refs_.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    std::vector<std::string> QueryExecutor::get_table_names() const {
        std::vector<std::string> names;
        for (const auto& [name, table] : tables_) {
            names.push_back(name);
        }
        return names;
    }
    
    void QueryExecutor::clear_all_tables() {
        tables_.clear();
        table_refs_.clear();
    }

} // namespace query
} // namespace minidb
