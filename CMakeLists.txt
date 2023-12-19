cmake_minimum_required(VERSION 3.5)
project(parson C)

set(C_STANDARD 89)

include (GNUInstallDirs)

set(PARSON_VERSION 1.5.3)
add_library(parson parson.c)
target_include_directories(parson PUBLIC $<INSTALL_INTERFACE:include>)

set_target_properties(parson PROPERTIES PUBLIC_HEADER "parson.h")
set_target_properties(parson PROPERTIES VERSION ${PARSON_VERSION})
set_target_properties(parson PROPERTIES SOVERSION ${PARSON_VERSION})

install(
    TARGETS parson
    EXPORT parsonTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT shlib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT parsonTargets
    FILE parsonConfig.cmake
    NAMESPACE parson::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

enable_testing()
add_executable(json_test ${CMAKE_SOURCE_DIR}/tests.c)
add_definitions(-DTESTS_MAIN)
target_link_libraries(json_test PUBLIC parson)
add_test(NAME json_tests COMMAND json_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
