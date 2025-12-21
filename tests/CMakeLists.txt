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
    test_main.cpp
    test_dnn.cpp
    test_memory.cpp
    test_nlu.cpp
    ../dnn.cpp
    ../brain.cpp
    ../memory_store.cpp
    ../reflex.cpp
    ../task_manager.cpp
)

target_include_directories(brain_tests PRIVATE ..)
target_link_libraries(brain_tests PRIVATE gtest_main Threads::Threads sqlite3)

find_package(TBB)
if(TBB_FOUND)
    target_link_libraries(brain_tests PRIVATE TBB::tbb)
endif()

add_test(NAME BrainTests COMMAND brain_tests)
