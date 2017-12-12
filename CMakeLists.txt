cmake_minimum_required(VERSION 2.8)
project(parson)

# Version ######################################################################
set(PARSON_VERSION_MAJOR 0)
set(PARSON_VERSION_MINOR 0)
set(PARSON_VERSION_PATCH 0)

# Build Options ################################################################
option(BUILD_TESTS "Build unit tests" OFF)

# Build library ################################################################
set(SOURCE_FILES ${CMAKE_SOURCE_DIR}/parson.c ${CMAKE_SOURCE_DIR}/parson.h)
add_library(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

# Build options ################################################################
if(WIN32 AND MSVC AND BUILD_SHARED_LIBS)
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif(WIN32 AND MSVC AND BUILD_SHARED_LIBS)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -std=c89 -pedantic-errors)
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

# Install targets ##############################################################
install(TARGETS ${CMAKE_PROJECT_NAME}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)

install(FILES parson.h DESTINATION include
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)

# Build and Execute Unit Test ##################################################
if(BUILD_TESTS)
    enable_testing()

    set(TARGET_TEST test-all)
    add_executable(${TARGET_TEST} ${CMAKE_SOURCE_DIR}/tests.c)
    target_link_libraries(${TARGET_TEST} ${CMAKE_PROJECT_NAME})
    add_test(NAME ${TARGET_TEST} COMMAND ${TARGET_TEST})
endif(BUILD_TESTS)
