cmake_minimum_required(VERSION 3.10)

# Define project name and version
project(LIBHTTPSERVER VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(src)
include_directories(src/httpserver)

# Add source files
file(GLOB SRC_FILES
    "src/*.cpp"
    "src/httpserver/*.cpp"
    "src/details/*.cpp"
)

# Option to build examples and tests
option(BUILD_EXAMPLES "Build the example programs" OFF)
option(BUILD_TESTING "Build the tests" OFF)

# Create a library from the source files
add_library(httpserver_lib STATIC ${SRC_FILES})

# Conditionally build examples if BUILD_EXAMPLES is ON
if(BUILD_EXAMPLES)
    # Add the examples
    file(GLOB EXAMPLES
        "examples/*.cpp"
    )
    
    # Add example executables for each example and prepend "example_" to avoid name conflicts
    foreach(example_file ${EXAMPLES})
        get_filename_component(example_name ${example_file} NAME_WE)
        set(example_target "example_${example_name}")
        add_executable(${example_target} ${example_file})
        target_link_libraries(${example_target} httpserver_lib)
    endforeach()
endif()

# Conditionally build tests if BUILD_TESTING is ON
if(BUILD_TESTING)
    # Add the test files
    file(GLOB UNIT_TESTS
        "test/unit/*.cpp"
    )

    # Add the integration test files
    file(GLOB INTEG_TESTS
        "test/integ/*.cpp"
    )

    # Define the unit tests
    foreach(test_file ${UNIT_TESTS})
        get_filename_component(test_name ${test_file} NAME_WE)
        add_executable(${test_name} ${test_file})
        target_link_libraries(${test_name} httpserver_lib)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()

    # Define the integration tests
    foreach(test_file ${INTEG_TESTS})
        get_filename_component(test_name ${test_file} NAME_WE)
        add_executable(${test_name} ${test_file})
        target_link_libraries(${test_name} httpserver_lib)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()

    # Enable testing
    enable_testing()
endif()

