cmake_minimum_required (VERSION 2.8)

project (libparson)

include (version.cmake.in)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=c89 -pedantic-errors")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include_directories (${PROJECT_SOURCE_DIR})

add_library (parson SHARED parson.c)

set_target_properties (parson PROPERTIES
	SOVERSION ${so_version}
	VERSION ${PARSON_VERSION}
)

install (FILES parson.h DESTINATION include/parson)
install (TARGETS parson DESTINATION lib)

add_custom_target (uninstall
	COMMAND ${CMAKE_COMMAND} -E remove
		`cat ${PROJECT_BINARY_DIR}/install_manifest.txt`
)

if (${CMAKE_BUILD_TYPE} MATCHES Debug)
	include (test.cmake.in)
endif ()
