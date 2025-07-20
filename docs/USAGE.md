# MiniDB Usage Guide

## Building the Project

### Using the Build Script (Recommended)

**Linux/macOS:**
```bash
chmod +x build.sh
./build.sh
```

**Windows:**
```cmd
build.bat
```

### Manual Build with CMake

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

## Running MiniDB

### Interactive Mode
```bash
./minidb
```

This starts the interactive CLI where you can type SQL commands and built-in commands.

### Execute SQL File
```bash
./minidb -f data/sample_queries.sql
```

### Execute Single Command
```bash
./minidb -c "SELECT * FROM users;"
```

## SQL Syntax Supported

### Table Management
```sql
-- Create table
CREATE TABLE users (id INTEGER, name TEXT, age INTEGER);

-- Drop table
DROP TABLE users;
```

### Data Operations
```sql
-- Insert data
INSERT INTO users VALUES (1, 'Alice', 25);

-- Select all columns
SELECT * FROM users;

-- Select specific columns
SELECT name, age FROM users;

-- Select with conditions
SELECT * FROM users WHERE age > 25;
```

### Data Types
- `INTEGER`: 64-bit signed integers
- `TEXT`: Variable-length strings
- `REAL`: Double-precision floating-point numbers

## Built-in CLI Commands

- `help` - Show available commands
- `tables` - List all tables
- `describe <table>` - Show table structure
- `clear` - Clear screen
- `quit` or `exit` - Exit the program

## Examples

### Basic Usage
```sql
minidb> CREATE TABLE products (id INTEGER, name TEXT, price REAL);
Query executed successfully. 0 rows affected.

minidb> INSERT INTO products VALUES (1, 'Laptop', 999.99);
Query executed successfully. 1 rows affected.

minidb> SELECT * FROM products;
+----+--------+--------+
| id | name   | price  |
+----+--------+--------+
| 1  | Laptop | 999.99 |
+----+--------+--------+
(1 row)
```

### File Execution
Create a file `my_queries.sql`:
```sql
CREATE TABLE test (id INTEGER, value TEXT);
INSERT INTO test VALUES (1, 'Hello');
INSERT INTO test VALUES (2, 'World');
SELECT * FROM test;
```

Then run:
```bash
./minidb -f my_queries.sql
```

## Performance Notes

- B-Tree operations: O(log n)
- Hash map lookups: O(1) average case
- Table scans: O(n)
- Memory-based storage (no persistence)

## Limitations

- Single-threaded operation
- No transaction support
- Limited SQL syntax
- In-memory storage only
- No concurrent access control

## Troubleshooting

### Build Issues
- Ensure you have CMake 3.10+ installed
- Verify C++17 compiler support
- Check that all source files are present

### Runtime Issues
- Verify table exists before querying
- Check SQL syntax for typos
- Ensure proper data types in INSERT statements

For more examples, see the `data/` directory.
