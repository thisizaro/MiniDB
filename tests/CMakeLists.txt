# Tests CMakeLists.txt for MiniDB

# Find or build a simple test framework (using basic assertions for now)
# In a real project, you might use Google Test, Catch2, etc.

# Test sources
set(TEST_SOURCES
    test_main.cpp
    test_btree.cpp
    test_hashmap.cpp
)

# Create test executable
add_executable(minidb_tests ${TEST_SOURCES})

# Link with the main library
target_link_libraries(minidb_tests
    PRIVATE
        minidb_static
)

# Include directories
target_include_directories(minidb_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Add tests to CTest
add_test(NAME MiniDBTests COMMAND minidb_tests)

# Set test properties
set_target_properties(minidb_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
