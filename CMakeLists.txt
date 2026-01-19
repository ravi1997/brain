cmake_minimum_required(VERSION 3.20)
project(brain_replica VERSION 1.0.0 LANGUAGES CXX)

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

option(ENABLE_POSTGRES "Enable PostgreSQL support" ON)
option(ENABLE_REDIS "Enable Redis support" ON)

# Find libpq
if(ENABLE_POSTGRES)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(PQ libpq)
    endif()

    if(NOT PQ_FOUND)
        find_path(LIBPQ_INCLUDE_DIR libpq-fe.h PATH_SUFFIXES postgresql)
        find_library(LIBPQ_LIBRARY pq)
        if(NOT LIBPQ_INCLUDE_DIR OR NOT LIBPQ_LIBRARY)
            message(WARNING "libpq not found - PostgreSQL support will be disabled")
            set(ENABLE_POSTGRES OFF)
        endif()
    else()
        set(LIBPQ_INCLUDE_DIR ${PQ_INCLUDE_DIRS})
        set(LIBPQ_LIBRARY ${PQ_LIBRARIES})
        link_directories(${PQ_LIBRARY_DIRS})
    endif()
endif()

# Find hiredis
if(ENABLE_REDIS)
    find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
    find_library(HIREDIS_LIBRARY hiredis)
    if(NOT HIREDIS_INCLUDE_DIR OR NOT HIREDIS_LIBRARY)
        message(WARNING "hiredis not found - Redis support will be disabled")
        set(ENABLE_REDIS OFF)
    endif()
endif()

# Find sqlite3
find_package(SQLite3)

if(ENABLE_POSTGRES)
    add_definitions(-DUSE_POSTGRES)
    include_directories(${LIBPQ_INCLUDE_DIR})
endif()

if(ENABLE_REDIS)
    add_definitions(-DUSE_REDIS)
    include_directories(${HIREDIS_INCLUDE_DIR})
endif()

# The main executable
add_executable(brain_replica 
    src/main.cpp 
    src/dnn.cpp 
    src/brain.cpp 
    src/memory_store.cpp 
    src/server.cpp 
    src/reflex.cpp 
    src/task_manager.cpp 
    src/planning_unit.cpp 
    src/postgres_client.cpp 
    src/postgres_storage.cpp 
    src/crash_reporter.cpp
    src/cognitive_engine.cpp
    src/skill_manager.cpp
)

# Enable Precompiled Headers (PCH) for faster builds
target_precompile_headers(brain_replica PRIVATE include/stable.hpp)

# Link libraries
target_link_libraries(brain_replica PRIVATE Threads::Threads)

if(SQLite3_FOUND)
    target_link_libraries(brain_replica PRIVATE ${SQLite3_LIBRARIES})
    include_directories(${SQLite3_INCLUDE_DIRS})
else()
    target_link_libraries(brain_replica PRIVATE sqlite3) # Fallback
endif()

if(ENABLE_REDIS)
    target_link_libraries(brain_replica PRIVATE ${HIREDIS_LIBRARY})
endif()

if(ENABLE_POSTGRES)
    target_link_libraries(brain_replica PRIVATE ${LIBPQ_LIBRARY})
endif()

# TBB is often required for parallel algorithms on GCC/Clang
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
target_include_directories(benchmark_simd PRIVATE include src)
if(OpenMP_CXX_FOUND)
    target_link_libraries(benchmark_simd PRIVATE OpenMP::OpenMP_CXX)
endif()

# Test RL Engine
add_executable(test_rl_engine tests/test_rl_engine.cpp src/cognitive_engine.cpp src/dnn.cpp)
target_include_directories(test_rl_engine PRIVATE include src)
# Test Skill Manager
add_executable(test_skill_manager tests/test_skill_manager.cpp src/skill_manager.cpp src/dnn.cpp)
target_include_directories(test_skill_manager PRIVATE include src)
if(OpenMP_CXX_FOUND)
    target_link_libraries(test_skill_manager PRIVATE OpenMP::OpenMP_CXX)
endif()
