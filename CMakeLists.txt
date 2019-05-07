cmake_minimum_required(VERSION 3.5)
project(parson C)

add_library(parson parson.c)
target_include_directories(parson PUBLIC $<INSTALL_INTERFACE:include>)

set_target_properties(parson PROPERTIES PUBLIC_HEADER "parson.h")

install(
    TARGETS parson
    EXPORT parsonTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

install(
    EXPORT parsonTargets
    FILE parsonConfig.cmake
    NAMESPACE parson::
    DESTINATION cmake
)

