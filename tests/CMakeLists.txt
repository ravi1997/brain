enable_testing()
find_package(GTest REQUIRED)

add_executable(brain_tests
    test_main.cpp
    test_dnn.cpp
    test_memory.cpp
    ../dnn.cpp
    ../brain.cpp
    ../memory_store.cpp
    ../reflex.cpp
    ../task_manager.cpp
)

target_include_directories(brain_tests PRIVATE ..)
target_link_libraries(brain_tests PRIVATE GTest::GTest GTest::Main Threads::Threads sqlite3)

find_package(TBB)
if(TBB_FOUND)
    target_link_libraries(brain_tests PRIVATE TBB::tbb)
endif()

add_test(NAME BrainTests COMMAND brain_tests)
