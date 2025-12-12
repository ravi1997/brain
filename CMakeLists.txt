cmake_minimum_required(VERSION 3.20)
project(brain_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(brain main.cpp)
target_include_directories(brain PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(brain PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable)

add_executable(test_brain test_brain_simulation.cpp)
target_include_directories(test_brain PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(test_brain PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable)
target_compile_features(brain PRIVATE cxx_std_17)
target_compile_features(test_brain PRIVATE cxx_std_17)
