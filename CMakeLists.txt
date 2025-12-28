cmake_minimum_required(VERSION 3.20)
project(brain_replica VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warning flags
if(MSVC)
    add_compile_options(/W3 /permissive- /volatile:iso)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -mavx2 -mfma)
endif()

# Include vendor libraries
include_directories(include)

# Threading/Parallelism support
find_package(Threads REQUIRED)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found")
endif()

# Find libpq
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(PQ libpq)
endif()

if(NOT PQ_FOUND)
    find_path(LIBPQ_INCLUDE_DIR libpq-fe.h PATH_SUFFIXES postgresql)
    find_library(LIBPQ_LIBRARY pq)
    if(NOT LIBPQ_INCLUDE_DIR OR NOT LIBPQ_LIBRARY)
        message(FATAL_ERROR "libpq not found")
    endif()
else()
    set(LIBPQ_INCLUDE_DIR ${PQ_INCLUDE_DIRS})
    set(LIBPQ_LIBRARY ${PQ_LIBRARIES})
    link_directories(${PQ_LIBRARY_DIRS})
endif()

include_directories(${LIBPQ_INCLUDE_DIR})

# The main executable
# The main executable
add_executable(brain_replica main.cpp dnn.cpp brain.cpp memory_store.cpp server.cpp reflex.cpp task_manager.cpp planning_unit.cpp postgres_client.cpp postgres_storage.cpp crash_reporter.cpp)

# Enable Precompiled Headers (PCH) for faster builds
target_precompile_headers(brain_replica PRIVATE stable.hpp)

# Link libraries
target_link_libraries(brain_replica PRIVATE Threads::Threads sqlite3 hiredis ${LIBPQ_LIBRARY})

# TBB is often required for parallel algorithms on GCC/Clang
# We'll try to find it, but won't hard fail if not present (implementation might vary)
# dnn.hpp uses std::execution, which often relies on TBB on Linux.
find_package(TBB)
if(TBB_FOUND)
    target_link_libraries(brain_replica PRIVATE TBB::tbb)
endif()

if(OpenMP_CXX_FOUND)
    target_link_libraries(brain_replica PRIVATE OpenMP::OpenMP_CXX)
endif()

# Tests
add_subdirectory(tests)

# Benchmarks
add_executable(benchmark_simd tests/benchmark_simd.cpp)
target_include_directories(benchmark_simd PRIVATE .)
if(OpenMP_CXX_FOUND)
    target_link_libraries(benchmark_simd PRIVATE OpenMP::OpenMP_CXX)
endif()
