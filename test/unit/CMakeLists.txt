add_executable(unit_tests
    test_memory.cpp
    test_hazards.cpp
)

target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include ${source_dir})
target_link_libraries(unit_tests PRIVATE phlego_core gtest_main)

add_test(NAME unit_tests COMMAND unit_tests)
