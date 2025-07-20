# MiniDB - Lightweight C++ Database Engine

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)]()

MiniDB is a lightweight, embeddable database engine implemented in C++ with support for basic SQL operations. It features custom implementations of B-Trees, hash maps, memory paging, and a simple SQL query processor.

## Features

### Core Data Structures
- **B-Tree**: Self-balancing tree for efficient indexing and range queries
- **Hash Map**: Fast key-value lookups with collision handling
- **Custom Vector**: Dynamic arrays for flexible data storage

### Storage Layer
- **Memory Paging**: Efficient memory management with configurable page sizes
- **Table Management**: Support for creating, dropping, and managing tables
- **Index Support**: Automatic indexing for improved query performance

### Query Processing
- **SQL Parser**: Basic SQL syntax support for common operations
- **Query Executor**: Efficient query execution engine
- **CRUD Operations**: Complete Create, Read, Update, Delete functionality

### CLI Interface
- Interactive command-line interface
- Batch query execution
- Table inspection and debugging tools

## Quick Start

### Building with Make

```bash
# Clone the repository
git clone <repository-url>
cd minidb

# Build the project
make all

# Run the CLI
./minidb
```

### Building with CMake

```bash
mkdir build && cd build
cmake ..
make
./minidb
```

## Usage Examples

### Basic Operations

```sql
-- Create a table
CREATE TABLE users (id INTEGER, name TEXT, age INTEGER);

-- Insert data
INSERT INTO users VALUES (1, 'Alice', 25);
INSERT INTO users VALUES (2, 'Bob', 30);

-- Query data
SELECT * FROM users;
SELECT name, age FROM users WHERE age > 25;

-- Update data
UPDATE users SET age = 26 WHERE id = 1;

-- Delete data
DELETE FROM users WHERE id = 2;

-- Drop table
DROP TABLE users;
```

### CLI Commands

```bash
# Start interactive mode
./minidb

# Execute SQL file
./minidb --file queries.sql

# Show help
./minidb --help
```

## Project Structure

```
minidb/
├── include/minidb/          # Public header files
│   ├── core/               # Core data structures
│   ├── storage/            # Storage management
│   ├── query/              # Query processing
│   └── utils/              # Utilities and CLI
├── src/                    # Source files
│   ├── core/              # Core implementations
│   ├── storage/           # Storage implementations
│   ├── query/             # Query implementations
│   ├── utils/             # Utility implementations
│   └── cli/               # Main CLI application
├── tests/                  # Unit tests
├── cmake/                  # CMake modules
├── docs/                   # Documentation
└── data/                   # Sample data files
```

## Dependencies

- C++17 compiler (GCC 7+, Clang 5+, MSVC 2019+)
- CMake 3.10+ (optional, for CMake builds)
- Make (optional, for Make builds)

## Building Tests

```bash
# With Make
make test

# With CMake
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
make
ctest
```

## Performance Notes

- B-Tree operations: O(log n) time complexity
- Hash map lookups: O(1) average case
- Memory paging: Configurable page size (default 4KB)
- Query parsing: Linear time in query length

## Limitations

This is a educational/demonstration database engine with the following limitations:

- Single-user, single-threaded operation
- In-memory storage (persistence planned for future versions)
- Limited SQL syntax support
- No transaction support
- No concurrent access control

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Architecture

### Core Components

1. **Data Structures**: Custom B-Tree and Hash Map implementations
2. **Storage**: Page-based memory management system
3. **Parser**: Recursive descent SQL parser
4. **Executor**: Query execution engine with optimization
5. **CLI**: Interactive command-line interface

### Design Principles

- **Modularity**: Clean separation of concerns
- **Performance**: Optimized data structures and algorithms
- **Maintainability**: Well-documented, tested code
- **Extensibility**: Plugin-friendly architecture

For detailed architecture documentation, see the `docs/` directory.
