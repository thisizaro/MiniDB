/**
 * @file parser.cpp
 * @brief Basic SQL parser implementation
 */

#include "minidb/query/parser.h"
#include <algorithm>
#include <cctype>

namespace minidb {
namespace query {

    // Expression implementations
    storage::Value LiteralExpression::evaluate(const storage::Row& row, 
                                              const storage::TableSchema& schema) const {
        return value_;
    }
    
    std::unique_ptr<Expression> LiteralExpression::clone() const {
        return std::make_unique<LiteralExpression>(value_);
    }
    
    storage::Value ColumnExpression::evaluate(const storage::Row& row,
                                            const storage::TableSchema& schema) const {
        size_t column_index = schema.get_column_index(column_name_);
        if (column_index != SIZE_MAX && column_index < row.size()) {
            return row.get_value(column_index);
        }
        return storage::Value();  // NULL value
    }
    
    std::unique_ptr<Expression> ColumnExpression::clone() const {
        return std::make_unique<ColumnExpression>(column_name_);
    }
    
    storage::Value BinaryExpression::evaluate(const storage::Row& row,
                                            const storage::TableSchema& schema) const {
        storage::Value left_val = left_->evaluate(row, schema);
        storage::Value right_val = right_->evaluate(row, schema);
        
        switch (op_) {
            case Operator::EQUAL:
                return storage::Value(static_cast<int64_t>(left_val == right_val));
            case Operator::NOT_EQUAL:
                return storage::Value(static_cast<int64_t>(left_val != right_val));
            case Operator::LESS_THAN:
                return storage::Value(static_cast<int64_t>(left_val < right_val));
            case Operator::LESS_EQUAL:
                return storage::Value(static_cast<int64_t>(left_val <= right_val));
            case Operator::GREATER_THAN:
                return storage::Value(static_cast<int64_t>(left_val > right_val));
            case Operator::GREATER_EQUAL:
                return storage::Value(static_cast<int64_t>(left_val >= right_val));
            default:
                return storage::Value();
        }
    }
    
    std::unique_ptr<Expression> BinaryExpression::clone() const {
        return std::make_unique<BinaryExpression>(left_->clone(), right_->clone(), op_);
    }
    
    // Tokenizer implementation
    bool Tokenizer::is_whitespace(char c) const {
        return std::isspace(c);
    }
    
    bool Tokenizer::is_alpha(char c) const {
        return std::isalpha(c) || c == '_';
    }
    
    bool Tokenizer::is_digit(char c) const {
        return std::isdigit(c);
    }
    
    bool Tokenizer::is_alphanumeric(char c) const {
        return is_alpha(c) || is_digit(c);
    }
    
    bool Tokenizer::tokenize(const std::string& sql) {
        tokens_.clear();
        current_pos_ = 0;
        
        size_t i = 0;
        while (i < sql.length()) {
            char c = sql[i];
            
            // Skip whitespace
            if (is_whitespace(c)) {
                i++;
                continue;
            }
            
            // Handle quoted strings
            if (c == '\'' || c == '\"') {
                char quote = c;
                std::string token;
                token += c;
                i++;
                
                while (i < sql.length() && sql[i] != quote) {
                    token += sql[i];
                    i++;
                }
                
                if (i < sql.length()) {
                    token += sql[i];  // Add closing quote
                    i++;
                }
                
                tokens_.push_back(token);
                continue;
            }
            
            // Handle operators and punctuation
            if (c == '=' || c == '<' || c == '>' || c == '!' || 
                c == '(' || c == ')' || c == ',' || c == ';' || c == '*') {
                
                std::string token;
                token += c;
                
                // Handle two-character operators
                if (i + 1 < sql.length()) {
                    char next = sql[i + 1];
                    if ((c == '<' && next == '=') ||
                        (c == '>' && next == '=') ||
                        (c == '!' && next == '=')) {
                        token += next;
                        i++;
                    }
                }
                
                tokens_.push_back(token);
                i++;
                continue;
            }
            
            // Handle identifiers and keywords
            if (is_alpha(c)) {
                std::string token;
                while (i < sql.length() && is_alphanumeric(sql[i])) {
                    token += sql[i];
                    i++;
                }
                
                // Convert to uppercase for keywords
                std::string upper_token = token;
                std::transform(upper_token.begin(), upper_token.end(), 
                             upper_token.begin(), ::toupper);
                tokens_.push_back(upper_token);
                continue;
            }
            
            // Handle numbers
            if (is_digit(c)) {
                std::string token;
                while (i < sql.length() && (is_digit(sql[i]) || sql[i] == '.')) {
                    token += sql[i];
                    i++;
                }
                tokens_.push_back(token);
                continue;
            }
            
            // Unknown character, skip
            i++;
        }
        
        return true;
    }
    
    std::string Tokenizer::current_token() const {
        if (current_pos_ < tokens_.size()) {
            return tokens_[current_pos_];
        }
        return "";
    }
    
    bool Tokenizer::next_token() {
        if (current_pos_ < tokens_.size()) {
            current_pos_++;
            return current_pos_ < tokens_.size();
        }
        return false;
    }
    
    std::string Tokenizer::peek_token() const {
        if (current_pos_ + 1 < tokens_.size()) {
            return tokens_[current_pos_ + 1];
        }
        return "";
    }
    
    bool Tokenizer::at_end() const {
        return current_pos_ >= tokens_.size();
    }
    
    void Tokenizer::reset() {
        current_pos_ = 0;
    }
    
    // Parser implementation
    std::unique_ptr<Statement> Parser::parse(const std::string& sql) {
        error_message_.clear();
        
        if (!tokenizer_.tokenize(sql)) {
            error_message_ = "Tokenization failed";
            return nullptr;
        }
        
        tokenizer_.reset();
        
        if (tokenizer_.at_end()) {
            error_message_ = "Empty query";
            return nullptr;
        }
        
        std::string first_token = tokenizer_.current_token();
        
        if (first_token == "SELECT") {
            return parse_select();
        } else if (first_token == "INSERT") {
            return parse_insert();
        } else if (first_token == "UPDATE") {
            return parse_update();
        } else if (first_token == "DELETE") {
            return parse_delete();
        } else if (first_token == "CREATE") {
            return parse_create_table();
        } else if (first_token == "DROP") {
            return parse_drop_table();
        } else {
            error_message_ = "Unsupported statement type: " + first_token;
            return nullptr;
        }
    }
    
    std::unique_ptr<Statement> Parser::parse_select() {
        // SELECT columns FROM table [WHERE condition]
        
        if (!expect_token("SELECT")) {
            return nullptr;
        }
        
        std::vector<std::string> columns;
        
        // Parse column list or *
        if (tokenizer_.current_token() == "*") {
            // Select all columns (empty vector indicates this)
            tokenizer_.next_token();
        } else {
            do {
                std::string column = tokenizer_.current_token();
                if (column.empty()) {
                    error_message_ = "Expected column name";
                    return nullptr;
                }
                columns.push_back(column);
                tokenizer_.next_token();
                
                if (tokenizer_.current_token() == ",") {
                    tokenizer_.next_token();
                } else {
                    break;
                }
            } while (!tokenizer_.at_end());
        }
        
        // Parse FROM clause
        if (!expect_token("FROM")) {
            return nullptr;
        }
        
        std::string table_name = tokenizer_.current_token();
        if (table_name.empty()) {
            error_message_ = "Expected table name";
            return nullptr;
        }
        tokenizer_.next_token();
        
        // Parse optional WHERE clause
        std::unique_ptr<Expression> where_clause;
        if (tokenizer_.current_token() == "WHERE") {
            tokenizer_.next_token();
            where_clause = parse_expression();
            if (!where_clause) {
                return nullptr;
            }
        }
        
        return std::make_unique<SelectStatement>(columns, table_name, std::move(where_clause));
    }
    
    std::unique_ptr<Statement> Parser::parse_insert() {
        // INSERT INTO table VALUES (value1, value2, ...)
        
        if (!expect_token("INSERT") || !expect_token("INTO")) {
            return nullptr;
        }
        
        std::string table_name = tokenizer_.current_token();
        if (table_name.empty()) {
            error_message_ = "Expected table name";
            return nullptr;
        }
        tokenizer_.next_token();
        
        if (!expect_token("VALUES") || !expect_token("(")) {
            return nullptr;
        }
        
        std::vector<storage::Value> values;
        std::vector<std::string> columns;  // Empty for now, assume all columns
        
        do {
            storage::Value value = parse_literal();
            values.push_back(value);
            
            if (tokenizer_.current_token() == ",") {
                tokenizer_.next_token();
            } else {
                break;
            }
        } while (!tokenizer_.at_end());
        
        if (!expect_token(")")) {
            return nullptr;
        }
        
        return std::make_unique<InsertStatement>(table_name, columns, values);
    }
    
    std::unique_ptr<Statement> Parser::parse_create_table() {
        // CREATE TABLE name (column1 type1, column2 type2, ...)
        
        if (!expect_token("CREATE") || !expect_token("TABLE")) {
            return nullptr;
        }
        
        std::string table_name = tokenizer_.current_token();
        if (table_name.empty()) {
            error_message_ = "Expected table name";
            return nullptr;
        }
        tokenizer_.next_token();
        
        if (!expect_token("(")) {
            return nullptr;
        }
        
        std::vector<storage::Column> columns;
        
        do {
            std::string column_name = tokenizer_.current_token();
            if (column_name.empty()) {
                error_message_ = "Expected column name";
                return nullptr;
            }
            tokenizer_.next_token();
            
            std::string type_str = tokenizer_.current_token();
            storage::ColumnType type = parse_column_type(type_str);
            tokenizer_.next_token();
            
            columns.emplace_back(column_name, type);
            
            if (tokenizer_.current_token() == ",") {
                tokenizer_.next_token();
            } else {
                break;
            }
        } while (!tokenizer_.at_end());
        
        if (!expect_token(")")) {
            return nullptr;
        }
        
        return std::make_unique<CreateTableStatement>(table_name, columns);
    }
    
    std::unique_ptr<Statement> Parser::parse_drop_table() {
        if (!expect_token("DROP") || !expect_token("TABLE")) {
            return nullptr;
        }
        
        std::string table_name = tokenizer_.current_token();
        if (table_name.empty()) {
            error_message_ = "Expected table name";
            return nullptr;
        }
        
        return std::make_unique<DropTableStatement>(table_name);
    }
    
    // Simplified implementations for update and delete
    std::unique_ptr<Statement> Parser::parse_update() {
        error_message_ = "UPDATE not yet implemented";
        return nullptr;
    }
    
    std::unique_ptr<Statement> Parser::parse_delete() {
        error_message_ = "DELETE not yet implemented";
        return nullptr;
    }
    
    std::unique_ptr<Expression> Parser::parse_expression() {
        return parse_comparison_expression();
    }
    
    std::unique_ptr<Expression> Parser::parse_comparison_expression() {
        auto left = parse_primary_expression();
        if (!left) return nullptr;
        
        std::string op_token = tokenizer_.current_token();
        Operator op;
        
        if (op_token == "=") {
            op = Operator::EQUAL;
        } else if (op_token == "!=") {
            op = Operator::NOT_EQUAL;
        } else if (op_token == "<") {
            op = Operator::LESS_THAN;
        } else if (op_token == "<=") {
            op = Operator::LESS_EQUAL;
        } else if (op_token == ">") {
            op = Operator::GREATER_THAN;
        } else if (op_token == ">=") {
            op = Operator::GREATER_EQUAL;
        } else {
            return left;  // No comparison operator
        }
        
        tokenizer_.next_token();
        auto right = parse_primary_expression();
        if (!right) return nullptr;
        
        return std::make_unique<BinaryExpression>(std::move(left), std::move(right), op);
    }
    
    std::unique_ptr<Expression> Parser::parse_primary_expression() {
        std::string token = tokenizer_.current_token();
        
        if (token.empty()) {
            error_message_ = "Unexpected end of expression";
            return nullptr;
        }
        
        // Check if it's a literal
        if ((token[0] >= '0' && token[0] <= '9') || 
            token[0] == '\'' || token[0] == '\"') {
            storage::Value value = parse_literal();
            return std::make_unique<LiteralExpression>(value);
        }
        
        // Otherwise, treat as column name
        std::string column_name = token;
        tokenizer_.next_token();
        return std::make_unique<ColumnExpression>(column_name);
    }
    
    storage::ColumnType Parser::parse_column_type(const std::string& type_str) {
        std::string upper_type = type_str;
        std::transform(upper_type.begin(), upper_type.end(), 
                      upper_type.begin(), ::toupper);
        
        if (upper_type == "INTEGER" || upper_type == "INT") {
            return storage::ColumnType::INTEGER;
        } else if (upper_type == "TEXT" || upper_type == "VARCHAR") {
            return storage::ColumnType::TEXT;
        } else if (upper_type == "REAL" || upper_type == "FLOAT" || upper_type == "DOUBLE") {
            return storage::ColumnType::REAL;
        } else {
            return storage::ColumnType::TEXT;  // Default
        }
    }
    
    storage::Value Parser::parse_literal() {
        std::string token = tokenizer_.current_token();
        tokenizer_.next_token();
        
        if (token.empty()) {
            return storage::Value();  // NULL
        }
        
        // String literal
        if (token[0] == '\'' || token[0] == '\"') {
            std::string str = token.substr(1, token.length() - 2);  // Remove quotes
            return storage::Value(str);
        }
        
        // Number literal
        if (token[0] >= '0' && token[0] <= '9') {
            if (token.find('.') != std::string::npos) {
                return storage::Value(std::stod(token));
            } else {
                return storage::Value(static_cast<int64_t>(std::stoll(token)));
            }
        }
        
        return storage::Value();  // NULL
    }
    
    bool Parser::expect_token(const std::string& expected) {
        if (tokenizer_.current_token() == expected) {
            tokenizer_.next_token();
            return true;
        }
        error_message_ = "Expected '" + expected + "', got '" + tokenizer_.current_token() + "'";
        return false;
    }

} // namespace query
} // namespace minidb
