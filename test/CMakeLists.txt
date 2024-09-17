# Set CMP0079 to allow linking from other directories
cmake_policy(SET CMP0079 NEW)

# Unit tests
file(GLOB UNIT_TESTS_SOURCES ${CMAKE_SOURCE_DIR}/test/unit/*.cpp)

foreach(UNIT_TEST ${UNIT_TESTS_SOURCES})
    get_filename_component(UNIT_TEST_NAME ${UNIT_TEST} NAME_WE)
    set(TEST_EXECUTABLE_NAME "${UNIT_TEST_NAME}_test")
    add_executable(${TEST_EXECUTABLE_NAME} ${UNIT_TEST})
    target_link_libraries(${TEST_EXECUTABLE_NAME} libhttpserver)
    add_test(NAME ${UNIT_TEST_NAME} COMMAND ${TEST_EXECUTABLE_NAME})
endforeach()

# Integration tests
file(GLOB INTEG_TESTS_SOURCES ${CMAKE_SOURCE_DIR}/test/integ/*.cpp)

foreach(INTEG_TEST ${INTEG_TESTS_SOURCES})
    get_filename_component(INTEG_TEST_NAME ${INTEG_TEST} NAME_WE)
    set(TEST_EXECUTABLE_NAME "${INTEG_TEST_NAME}_test")
    add_executable(${TEST_EXECUTABLE_NAME} ${INTEG_TEST})
    target_link_libraries(${TEST_EXECUTABLE_NAME} libhttpserver)
    add_test(NAME ${INTEG_TEST_NAME} COMMAND ${TEST_EXECUTABLE_NAME})
endforeach()
