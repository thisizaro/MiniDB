# Source CMakeLists.txt for MiniDB

# Core library sources
set(MINIDB_SOURCES
    minidb.cpp
    core/btree.cpp
    core/hashmap.cpp
    storage/page_manager.cpp
    storage/table.cpp
    query/parser.cpp
    query/executor.cpp
    utils/cli.cpp
)

# Create static library
add_library(minidb_static STATIC ${MINIDB_SOURCES})

# Set library properties
set_target_properties(minidb_static PROPERTIES
    OUTPUT_NAME minidb
    POSITION_INDEPENDENT_CODE ON
)

# Include directories for the library
target_include_directories(minidb_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler features
target_compile_features(minidb_static PUBLIC cxx_std_17)

# Create main executable
add_executable(minidb cli/main.cpp)

# Link executable with library
target_link_libraries(minidb
    PRIVATE
        minidb_static
)

# Set executable properties
set_target_properties(minidb PROPERTIES
    OUTPUT_NAME minidb
)

# Install targets
install(TARGETS minidb minidb_static
    EXPORT MiniDBTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

# Install headers
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/minidb
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
