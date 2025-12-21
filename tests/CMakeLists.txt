project(BrainTests)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(brain_tests
    ../brain.cpp
    ../dnn.cpp
    ../memory_store.cpp
    ../task_manager.cpp
    ../reflex.cpp
    test_memory.cpp
    test_nlu.cpp
    test_context.cpp
    test_memory_optimization.cpp
    test_redis.cpp
)

target_link_libraries(brain_tests
    gtest_main
    sqlite3
    tbb
    hiredis
)

include(GoogleTest)
gtest_discover_tests(brain_tests)
