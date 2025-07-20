-- MiniDB Demo Script
-- Demonstrates various database operations

-- Show welcome message (this is a comment)
-- Let's start by creating our first table

CREATE TABLE employees (
    employee_id INTEGER,
    first_name TEXT,
    last_name TEXT,
    department TEXT,
    salary REAL
);

-- Insert employee data
INSERT INTO employees VALUES (1, 'John', 'Doe', 'Engineering', 75000.0);
INSERT INTO employees VALUES (2, 'Jane', 'Smith', 'Marketing', 65000.0);
INSERT INTO employees VALUES (3, 'Mike', 'Johnson', 'Engineering', 80000.0);
INSERT INTO employees VALUES (4, 'Sarah', 'Wilson', 'HR', 55000.0);
INSERT INTO employees VALUES (5, 'David', 'Brown', 'Engineering', 90000.0);

-- View all employees
SELECT * FROM employees;

-- Find engineering employees
SELECT first_name, last_name, salary FROM employees WHERE department = 'Engineering';

-- Create a projects table
CREATE TABLE projects (
    project_id INTEGER,
    project_name TEXT,
    lead_engineer TEXT
);

INSERT INTO projects VALUES (1, 'Database System', 'John Doe');
INSERT INTO projects VALUES (2, 'Web Application', 'David Brown');
INSERT INTO projects VALUES (3, 'Mobile App', 'Mike Johnson');

SELECT * FROM projects;
