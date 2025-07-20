/**
 * @file main.cpp
 * @brief Main entry point for MiniDB CLI application
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "minidb/minidb.h"

using namespace minidb;

/**
 * @brief Print usage information
 */
void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n";
    std::cout << "\nMiniDB - Lightweight C++ Database Engine\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -h, --help                    Show this help message\n";
    std::cout << "  -v, --version                 Show version information\n";
    std::cout << "  -f, --file <filename>         Execute SQL commands from file\n";
    std::cout << "  -c, --command <sql>           Execute single SQL command\n";
    std::cout << "  --format <format>             Output format (table, json, csv)\n";
    std::cout << "  --no-header                   Don't show column headers\n";
    std::cout << "  --quiet                       Suppress informational messages\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << "                          # Start interactive mode\n";
    std::cout << "  " << program_name << " -f queries.sql          # Execute file\n";
    std::cout << "  " << program_name << " -c \"SELECT * FROM users;\"  # Execute command\n";
    std::cout << "\nFor more information, visit: https://github.com/minidb/minidb\n";
}

/**
 * @brief Print version information
 */
void print_version() {
    std::cout << "MiniDB " << get_version() << "\n";
    std::cout << "Copyright (C) 2025 MiniDB Team\n";
    std::cout << "This is free software; see the source for copying conditions.\n";
}

/**
 * @brief Parse command line arguments
 * @param argc Argument count
 * @param argv Argument vector
 * @param config CLI configuration to populate
 * @return true if should continue execution, false if should exit
 */
bool parse_arguments(int argc, char* argv[], utils::CLIConfig& config,
                    std::string& file_to_execute, std::string& command_to_execute) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return false;
        } else if (arg == "-v" || arg == "--version") {
            print_version();
            return false;
        } else if (arg == "-f" || arg == "--file") {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << arg << " requires a filename\n";
                return false;
            }
            file_to_execute = argv[++i];
        } else if (arg == "-c" || arg == "--command") {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << arg << " requires a SQL command\n";
                return false;
            }
            command_to_execute = argv[++i];
        } else if (arg == "--format") {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << arg << " requires a format (table, json, csv)\n";
                return false;
            }
            std::string format = argv[++i];
            // Format will be handled by CLI class
        } else if (arg == "--no-header") {
            // Option will be handled by CLI class
        } else if (arg == "--quiet") {
            config.welcome_message = "";
            config.goodbye_message = "";
        } else {
            std::cerr << "Error: Unknown option " << arg << "\n";
            print_usage(argv[0]);
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Main application entry point
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code (0 for success)
 */
int main(int argc, char* argv[]) {
    try {
        // Initialize MiniDB library
        if (!initialize()) {
            std::cerr << "Error: Failed to initialize MiniDB library\n";
            return 1;
        }
        
        // Parse command line arguments
        utils::CLIConfig config;
        std::string file_to_execute;
        std::string command_to_execute;
        
        if (!parse_arguments(argc, argv, config, file_to_execute, command_to_execute)) {
            cleanup();
            return 0;  // Help or version was shown, exit successfully
        }
        
        // Create CLI instance
        utils::CLI cli(config);
        
        // Handle different execution modes
        if (!file_to_execute.empty()) {
            // Execute file mode
            if (!config.welcome_message.empty()) {
                std::cout << config.welcome_message << "\n";
            }
            
            bool success = cli.execute_file(file_to_execute);
            if (!success) {
                std::cerr << "Error: Failed to execute file " << file_to_execute << "\n";
                cleanup();
                return 1;
            }
            
            if (!config.goodbye_message.empty()) {
                std::cout << config.goodbye_message << "\n";
            }
            
        } else if (!command_to_execute.empty()) {
            // Execute single command mode
            cli.execute_single(command_to_execute);
            
        } else {
            // Interactive mode
            cli.run();
        }
        
        // Cleanup library
        cleanup();
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        cleanup();
        return 1;
    } catch (...) {
        std::cerr << "Error: Unknown exception occurred\n";
        cleanup();
        return 1;
    }
}
