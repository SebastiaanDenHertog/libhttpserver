cmake_minimum_required(VERSION 3.10)
project(libhttpserver VERSION 0.19.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(ENABLE_DEBUG "Enable debug mode" OFF)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(ENABLE_EXAMPLES "Build examples" OFF)
option(ENABLE_FASTOPEN "Enable TCP_FASTOPEN support" ON)
option(ENABLE_STATIC "Enable static linking" ON)
option(WITH_TLS "Enable TLS support" ON)
option(ENABLE_TESTS "Build tests" OFF) # Add option to control whether to build tests

# Compiler flags
if(ENABLE_DEBUG)
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Werror -pedantic")
else()
    add_definitions(-DNDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
endif()

# External libraries
find_package(Threads REQUIRED)

# Find libmicrohttpd
find_path(LIBMICROHTTPD_INCLUDE_DIR NAMES microhttpd.h)
find_library(LIBMICROHTTPD_LIBRARY NAMES microhttpd)

if(NOT LIBMICROHTTPD_INCLUDE_DIR OR NOT LIBMICROHTTPD_LIBRARY)
    message(FATAL_ERROR "libmicrohttpd >= 0.9.64 not found")
endif()

# Include directories for src and headers
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${LIBMICROHTTPD_INCLUDE_DIR})

# Collect source files
file(GLOB_RECURSE LIBHTTPSERVER_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Create the libhttpserver library
add_library(libhttpserver SHARED ${LIBHTTPSERVER_SOURCES})

target_compile_definitions(libhttpserver PRIVATE HTTPSERVER_COMPILATION)

# Link libraries
target_link_libraries(libhttpserver PRIVATE ${LIBMICROHTTPD_LIBRARY} Threads::Threads)

# Install the library and headers
install(TARGETS libhttpserver DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/src/httpserver.h DESTINATION include)

# Examples
if(ENABLE_EXAMPLES)
    file(GLOB EXAMPLES_SOURCES ${CMAKE_SOURCE_DIR}/examples/*.cpp)

    foreach(EXAMPLE_FILE ${EXAMPLES_SOURCES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})
        target_link_libraries(${EXAMPLE_NAME} libhttpserver)
        install(TARGETS ${EXAMPLE_NAME} DESTINATION bin)
    endforeach()
endif()

# Tests
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
