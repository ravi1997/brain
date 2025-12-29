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
    ../planning_unit.cpp
    ../postgres_client.cpp
    ../postgres_storage.cpp
    ../crash_reporter.cpp
    test_memory.cpp
    test_nlu.cpp
    test_context.cpp
    test_memory_optimization.cpp
    test_redis.cpp
    test_nlu_improved.cpp
    test_emotion_unit.cpp
    test_reflex_rl.cpp
    test_reflex_fuzzy.cpp
    test_emotion_logic.cpp
    test_nlu_context.cpp
    test_planning_dynamic.cpp
    test_postgres.cpp
    test_nlu_advanced.cpp
    test_cognition_features.cpp
    test_sensory.cpp
    test_sensory_fusion.cpp
    test_spatial_autonomy.cpp
    test_mega_batch.cpp
    benchmark_suite.cpp
    test_cognition_sprint.cpp
    stress_test_suite.cpp
    memory_profiling_suite.cpp
    nlu_test_suite.cpp
    qa_complete_suite.cpp
    nextgen_foundation_suite.cpp
    phase1_batch_a_suite.cpp
    phase1_batch_b_suite.cpp
    phase1_batch_c_suite.cpp
    phase1_batch_d_suite.cpp
    phase1_batch_e_suite.cpp
    phase1_batch_f_suite.cpp
    phase2_batch_a_suite_part1.cpp
    phase2_batch_a_suite_part2.cpp
    brain_integration_test.cpp
)

target_link_libraries(brain_tests
    gtest_main
    sqlite3
    tbb
    hiredis
    ${LIBPQ_LIBRARY}
)

include(GoogleTest)
gtest_discover_tests(brain_tests)
