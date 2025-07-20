-- Sample SQL queries for MiniDB
-- This file demonstrates the basic functionality

-- Create a users table
CREATE TABLE users (
    id INTEGER,
    name TEXT,
    age INTEGER
);

-- Insert some sample data
INSERT INTO users VALUES (1, 'Alice', 25);
INSERT INTO users VALUES (2, 'Bob', 30);
INSERT INTO users VALUES (3, 'Charlie', 35);
INSERT INTO users VALUES (4, 'Diana', 28);

-- Query all users
SELECT * FROM users;

-- Query specific columns
SELECT name, age FROM users;

-- Query with conditions
SELECT name FROM users WHERE age > 28;

-- Create another table for products
CREATE TABLE products (
    id INTEGER,
    name TEXT,
    price REAL
);

-- Insert products
INSERT INTO products VALUES (1, 'Laptop', 999.99);
INSERT INTO products VALUES (2, 'Mouse', 25.50);
INSERT INTO products VALUES (3, 'Keyboard', 75.00);

-- Query products
SELECT * FROM products;
SELECT name FROM products WHERE price < 100;
