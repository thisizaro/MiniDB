/**
 * @file cli.cpp
 * @brief Basic CLI implementation
 */

#include "minidb/utils/cli.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <algorithm>

namespace minidb {
namespace utils {

    // TableFormatter implementation
    std::vector<size_t> TableFormatter::calculate_column_widths(
        const query::QueryResult& result) const {
        std::vector<size_t> widths;
        
        if (!result.has_data()) {
            return widths;
        }
        
        const auto& column_names = result.get_column_names();
        const auto& rows = result.get_rows();
        
        // Initialize with column name lengths
        for (const auto& name : column_names) {
            widths.push_back(std::min(name.length(), max_column_width_));
        }
        
        // Check data widths
        size_t rows_to_check = std::min(rows.size(), max_rows_);
        for (size_t i = 0; i < rows_to_check; i++) {
            const auto& row = rows[i];
            for (size_t j = 0; j < std::min(widths.size(), row.size()); j++) {
                std::string value_str = row.get_value(j).to_string();
                widths[j] = std::max(widths[j], 
                    std::min(value_str.length(), max_column_width_));
            }
        }
        
        return widths;
    }
    
    void TableFormatter::print_separator(const std::vector<size_t>& widths, 
                                       std::ostream& output) const {
        output << "+";
        for (size_t width : widths) {
            output << std::string(width + 2, '-') << "+";
        }
        output << "\n";
    }
    
    std::string TableFormatter::truncate_text(const std::string& text, size_t width) const {
        if (text.length() <= width) {
            return text;
        }
        return text.substr(0, width - 3) + "...";
    }
    
    void TableFormatter::format(const query::QueryResult& result, std::ostream& output) const {
        if (!result.is_success()) {
            output << "Error: " << result.get_error() << "\n";
            return;
        }
        
        if (result.get_affected_rows() > 0 && !result.has_data()) {
            output << "Query executed successfully. " 
                   << result.get_affected_rows() << " rows affected.\n";
            return;
        }
        
        if (!result.has_data()) {
            output << "No results.\n";
            return;
        }
        
        const auto& column_names = result.get_column_names();
        const auto& rows = result.get_rows();
        std::vector<size_t> widths = calculate_column_widths(result);
        
        if (widths.empty()) {
            output << "No data to display.\n";
            return;
        }
        
        // Print top border
        print_separator(widths, output);
        
        // Print headers
        output << "|";
        for (size_t i = 0; i < column_names.size() && i < widths.size(); i++) {
            std::string header = truncate_text(column_names[i], widths[i]);
            output << " " << std::left << std::setw(widths[i]) << header << " |";
        }
        output << "\n";
        
        // Print header separator
        print_separator(widths, output);
        
        // Print data rows
        size_t rows_to_print = std::min(rows.size(), max_rows_);
        for (size_t i = 0; i < rows_to_print; i++) {
            const auto& row = rows[i];
            output << "|";
            
            for (size_t j = 0; j < column_names.size() && j < widths.size(); j++) {
                std::string value = "";
                if (j < row.size()) {
                    value = truncate_text(row.get_value(j).to_string(), widths[j]);
                }
                output << " " << std::left << std::setw(widths[j]) << value << " |";
            }
            output << "\n";
        }
        
        // Print bottom border
        print_separator(widths, output);
        
        // Show row count
        output << "(" << rows.size() << " row";
        if (rows.size() != 1) output << "s";
        output << ")\n";
        
        if (rows.size() > max_rows_) {
            output << "... and " << (rows.size() - max_rows_) << " more rows\n";
        }
    }
    
    // CLI implementation
    CLI::CLI(const CLIConfig& config) 
        : config_(config), running_(false) {
        
        // Initialize page manager and executor
        page_manager_ = std::make_unique<storage::PageManager>();
        executor_ = std::make_unique<query::QueryExecutor>(page_manager_.get());
        
        // Initialize formatter
        formatter_ = std::make_unique<TableFormatter>();
        
        // Initialize built-in commands
        initialize_commands();
        
        // Load history if enabled
        if (config_.enable_history) {
            load_history();
        }
    }
    
    CLI::~CLI() {
        if (config_.enable_history) {
            save_history();
        }
    }
    
    void CLI::run() {
        running_ = true;
        
        // Print welcome message
        if (!config_.welcome_message.empty()) {
            std::cout << config_.welcome_message << "\n";
            std::cout << "Type 'help' for available commands or enter SQL queries.\n";
            std::cout << "Type 'quit' or 'exit' to exit.\n\n";
        }
        
        while (running_) {
            std::cout << config_.prompt;
            std::string input = read_input();
            
            if (input.empty()) {
                continue;
            }
            
            // Add to history
            add_to_history(input);
            
            execute_single(input);
        }
        
        // Print goodbye message
        if (!config_.goodbye_message.empty()) {
            std::cout << config_.goodbye_message << "\n";
        }
    }
    
    void CLI::execute_single(const std::string& input) {
        std::vector<std::string> tokens = parse_input(input);
        if (tokens.empty()) {
            return;
        }
        
        std::string command_name = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        
        // Try to execute as built-in command first
        if (execute_command(command_name, args)) {
            return;
        }
        
        // Otherwise, execute as SQL
        execute_sql(input);
    }
    
    void CLI::execute_sql(const std::string& sql) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        query::QueryResult result = executor_->execute_sql(sql);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);
        
        // Format and display result
        formatter_->format(result, std::cout);
        
        // Show query time if enabled
        if (config_.show_query_time) {
            std::cout << "(Time: " << duration.count() << " ms)\n";
        }
        
        std::cout << "\n";
    }
    
    void CLI::initialize_commands() {
        commands_.emplace_back("help", "Show available commands", 
            [this](const auto& args) { handle_help(args); });
        commands_.emplace_back("quit", "Exit the application", 
            [this](const auto& args) { handle_quit(args); });
        commands_.emplace_back("exit", "Exit the application", 
            [this](const auto& args) { handle_quit(args); });
        commands_.emplace_back("clear", "Clear the screen", 
            [this](const auto& args) { handle_clear(args); });
        commands_.emplace_back("tables", "List all tables", 
            [this](const auto& args) { handle_tables(args); });
        commands_.emplace_back("describe", "Describe table structure", 
            [this](const auto& args) { handle_describe(args); });
    }
    
    void CLI::handle_help(const std::vector<std::string>& args) {
        std::cout << "Available commands:\n";
        for (const auto& cmd : commands_) {
            std::cout << "  " << std::left << std::setw(12) << cmd.name 
                     << " - " << cmd.description << "\n";
        }
        std::cout << "\nSQL Commands supported:\n";
        std::cout << "  CREATE TABLE - Create a new table\n";
        std::cout << "  DROP TABLE   - Drop an existing table\n";
        std::cout << "  INSERT       - Insert data into table\n";
        std::cout << "  SELECT       - Query data from table\n";
        std::cout << "  UPDATE       - Update existing data\n";
        std::cout << "  DELETE       - Delete data from table\n";
        std::cout << "\n";
    }
    
    void CLI::handle_quit(const std::vector<std::string>& args) {
        running_ = false;
    }
    
    void CLI::handle_clear(const std::vector<std::string>& args) {
        // Clear screen (simplified)
        std::cout << "\033[2J\033[H";
    }
    
    void CLI::handle_tables(const std::vector<std::string>& args) {
        auto table_names = executor_->get_table_names();
        if (table_names.empty()) {
            std::cout << "No tables found.\n";
        } else {
            std::cout << "Tables:\n";
            for (const auto& name : table_names) {
                std::cout << "  " << name << "\n";
            }
        }
        std::cout << "\n";
    }
    
    void CLI::handle_describe(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Usage: describe <table_name>\n";
            return;
        }
        
        std::string table_name = args[0];
        storage::Table* table = executor_->get_table(table_name);
        
        if (!table) {
            std::cout << "Table '" << table_name << "' not found.\n";
            return;
        }
        
        const auto& schema = table->get_schema();
        std::cout << "Table: " << table_name << "\n";
        std::cout << "Columns:\n";
        
        for (size_t i = 0; i < schema.column_count(); i++) {
            const auto& column = schema.get_column(i);
            std::cout << "  " << column.name << " " 
                     << column_type_to_string(column.type);
            if (column.primary_key) std::cout << " PRIMARY KEY";
            if (column.not_null) std::cout << " NOT NULL";
            if (column.unique) std::cout << " UNIQUE";
            std::cout << "\n";
        }
        
        std::cout << "Rows: " << table->row_count() << "\n\n";
    }
    
    std::vector<std::string> CLI::parse_input(const std::string& input) {
        std::vector<std::string> tokens;
        std::istringstream iss(input);
        std::string token;
        
        while (iss >> token) {
            // Convert to lowercase for command matching
            std::transform(token.begin(), token.end(), token.begin(), ::tolower);
            tokens.push_back(token);
        }
        
        return tokens;
    }
    
    bool CLI::execute_command(const std::string& command_name, 
                             const std::vector<std::string>& args) {
        for (const auto& cmd : commands_) {
            if (cmd.name == command_name) {
                cmd.handler(args);
                return true;
            }
        }
        return false;
    }
    
    std::string CLI::read_input() {
        std::string input;
        std::getline(std::cin, input);
        return input;
    }
    
    void CLI::load_history() {
        // Simplified history loading
        std::ifstream file(config_.history_file);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                history_.push_back(line);
            }
        }
    }
    
    void CLI::save_history() {
        // Simplified history saving
        std::ofstream file(config_.history_file);
        for (const auto& line : history_) {
            file << line << "\n";
        }
    }
    
    void CLI::add_to_history(const std::string& command) {
        if (config_.enable_history && !command.empty()) {
            history_.push_back(command);
            
            // Keep history size reasonable
            if (history_.size() > 1000) {
                history_.erase(history_.begin());
            }
        }
    }
    
    bool CLI::execute_file(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << filename << "\n";
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;  // Skip empty lines and comments
            }
            
            std::cout << config_.prompt << line << "\n";
            execute_single(line);
        }
        
        return true;
    }
    
    void CLI::set_formatter(std::unique_ptr<ResultFormatter> formatter) {
        formatter_ = std::move(formatter);
    }

} // namespace utils
} // namespace minidb
