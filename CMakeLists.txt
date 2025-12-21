cmake_minimum_required(VERSION 3.20)
project(brain_replica VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warning flags
if(MSVC)
    add_compile_options(/W3 /permissive- /volatile:iso)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

# Include vendor libraries
include_directories(include)

# Threading support (needed for dnn.hpp parallel execution)
find_package(Threads REQUIRED)

# The main executable
add_executable(brain_replica main.cpp dnn.cpp brain.cpp memory_store.cpp)

# Link threads (for std::execution::par_unseq in dnn.hpp)
target_link_libraries(brain_replica PRIVATE Threads::Threads sqlite3)

# TBB is often required for parallel algorithms on GCC/Clang
# We'll try to find it, but won't hard fail if not present (implementation might vary)
# dnn.hpp uses std::execution, which often relies on TBB on Linux.
find_package(TBB)
if(TBB_FOUND)
    target_link_libraries(brain_replica PRIVATE TBB::tbb)
endif()

# Tests
add_subdirectory(tests)
