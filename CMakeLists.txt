cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()


file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" VERSION_FILE_CONTENTS)
string(STRIP "${VERSION_FILE_CONTENTS}" VERSION_FILE_CONTENTS)
set(PY3LM_VERSION "${VERSION_FILE_CONTENTS}" CACHE STRING "Set version name")
set(PY3LM_PACKAGE "python3_module" CACHE STRING "Set package name")
string(REPLACE "v" "" PY3LM_VERSION "${PY3LM_VERSION}")
string(REGEX REPLACE "[.+-]" ";" PY3LM_VERSION_LIST ${PY3LM_VERSION})
list(GET PY3LM_VERSION_LIST 0 PY3LM_VERSION_MAJOR)
list(GET PY3LM_VERSION_LIST 1 PY3LM_VERSION_MINOR)
list(GET PY3LM_VERSION_LIST 2 PY3LM_VERSION_PATCH)

project(plugify-module-python3
        VERSION "${PY3LM_VERSION_MAJOR}.${PY3LM_VERSION_MINOR}.${PY3LM_VERSION_PATCH}"
        DESCRIPTION "Python 3 Language Module"
        HOMEPAGE_URL "https://github.com/untrustedmodders/plugify-module-python3"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE MATCHES "Debug|Devel|MinSizeRel|RelWithDebInfo|Release")
    message(STATUS "CMAKE_BUILD_TYPE not set, defaulting to Debug.")
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(BUILD_SHARED_LIBS ON CACHE INTERNAL "")

#
# Plugify
#
set(PLUGIFY_BUILD_SHARED_LIB ON CACHE INTERNAL "")
set(PLUGIFY_BUILD_TESTS OFF CACHE INTERNAL "")
if(LINUX)
	set(PLUGIFY_USE_STATIC_STDLIB ON CACHE INTERNAL "")
	set(PLUGIFY_USE_ABI0 OFF CACHE INTERNAL "")
endif()
add_subdirectory(external/plugify)

#
# Python 3 Language Module for Plugify
#
set(PY3LM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/module.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/module.cpp")
add_library(${PROJECT_NAME} SHARED ${PY3LM_SOURCES})

set(PY3LM_LINK_LIBRARIES plugify::plugify)

if(NOT COMPILER_SUPPORTS_FORMAT)
    set(PY3LM_LINK_LIBRARIES ${PY3LM_LINK_LIBRARIES} fmt::fmt-header-only)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${PY3LM_LINK_LIBRARIES})

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_MACRO_NAME PY3LM_EXPORT EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/module_export.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/exports)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wextra -Wconversion -Werror -Wshadow -Wpedantic)
endif()

if(APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/sym/exported_symbols.lds")
elseif(UNIX)
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/sym/version_script.lds")
endif()

#
# Python 3.12
#
add_library(python3 SHARED IMPORTED)

if(${CMAKE_BUILD_TYPE} EQUAL "Debug")
    set(PYTHON_ABSTRACT_BUILD_TYPE_LOWER "debug")
else()
    set(PYTHON_ABSTRACT_BUILD_TYPE_LOWER "release")
endif()

detect_system()

if(WIN32)
    set(PYTHON_ARCH "win-64")
    set(PYTHON_DEBUG_LIB "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${PYTHON_ARCH}/debug/bin/python312_d.dll")
    set(PYTHON_DEBUG_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${PYTHON_ARCH}/debug/bin/python312_d.lib")
    set(PYTHON_RELEASE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${PYTHON_ARCH}/release/bin/python312.dll")
    set(PYTHON_RELEASE_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${PYTHON_ARCH}/release/bin/python312.lib")

    set_target_properties(python3 PROPERTIES
            IMPORTED_LOCATION_DEBUG ${PYTHON_DEBUG_LIB}
            IMPORTED_IMPLIB_DEBUG ${PYTHON_DEBUG_IMPLIB}
            IMPORTED_LOCATION_RELEASE ${PYTHON_RELEASE_LIB}
            IMPORTED_IMPLIB_RELEASE ${PYTHON_RELEASE_IMPLIB}
            IMPORTED_LOCATION_RELWITHDEBINFO ${PYTHON_RELEASE_LIB}
            IMPORTED_IMPLIB_RELWITHDEBINFO ${PYTHON_RELEASE_IMPLIB}
            IMPORTED_LOCATION_MINSIZEREL ${PYTHON_RELEASE_LIB}
            IMPORTED_IMPLIB_MINSIZEREL ${PYTHON_RELEASE_IMPLIB}
    )
elseif(LINUX)
    set(PYTHON_ARCH "linux-64")
    set(PYTHON_DEBUG_LIB "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${PYTHON_ARCH}/debug/bin/libpython3.12d.so.1.0")
    set(PYTHON_RELEASE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${PYTHON_ARCH}/release/bin/libpython3.12.so.1.0")

    set_target_properties(python3 PROPERTIES
            IMPORTED_LOCATION_DEBUG ${PYTHON_DEBUG_LIB}
            IMPORTED_LOCATION_RELEASE ${PYTHON_RELEASE_LIB}
            IMPORTED_LOCATION_RELWITHDEBINFO ${PYTHON_RELEASE_LIB}
            IMPORTED_LOCATION_MINSIZEREL ${PYTHON_RELEASE_LIB}
    )

    set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_FLAGS "-Wl,-rpath,\\\$ORIGIN/../python3.12")
elseif(APPLE)
    set(PYTHON_ARCH "osx-64")
    set(PYTHON_DEBUG_LIB "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${PYTHON_ARCH}/debug/bin/libpython3.12d.dylib")
    set(PYTHON_RELEASE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${PYTHON_ARCH}/release/bin/libpython3.12.dylib")

    set_target_properties(python3 PROPERTIES
            IMPORTED_LOCATION_DEBUG ${PYTHON_DEBUG_LIB}
            IMPORTED_LOCATION_RELEASE ${PYTHON_RELEASE_LIB}
            IMPORTED_LOCATION_RELWITHDEBINFO ${PYTHON_RELEASE_LIB}
            IMPORTED_LOCATION_MINSIZEREL ${PYTHON_RELEASE_LIB}
    )

    # TODO: Fix mac rpath
#    set_target_properties(${PROJECT_NAME} PROPERTIES
#            LINKER_LANGUAGE CXX
#            LINK_FLAGS "-Wl,-rpath,@loader_path/../python3.12/"
#            MACOSX_RPATH TRUE
#            SKIP_BUILD_RPATH FALSE
#            BUILD_WITH_INSTALL_RPATH TRUE
#            INSTALL_RPATH_USE_LINK_PATH TRUE
#            INSTALL_NAME_DIR "@rpath/")


endif()

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/include/python3.12" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/_pyinclude")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${PYTHON_ARCH}/${PYTHON_ABSTRACT_BUILD_TYPE_LOWER}/include/pyconfig.h" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/_pyinclude/python3.12")
add_library(pyinclude INTERFACE)
target_include_directories(pyinclude INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/_pyinclude/python3.12")
if(MSVC)
    target_compile_options(pyinclude INTERFACE /wd4100)
else()
    target_compile_options(pyinclude INTERFACE -Wno-unused-parameter)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE python3 pyinclude)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PY3LM_PLATFORM_WINDOWS=$<BOOL:${WIN32}>
    PY3LM_PLATFORM_APPLE=$<BOOL:${APPLE}>
    PY3LM_PLATFORM_LINUX=$<BOOL:${LINUX}>
    PY3LM_IS_DEBUG=$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pmodule.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pmodule
)

set(PY3LM_GITHUB_ACTIONS "0" CACHE STRING "Build with github actions")

if(PY3LM_GITHUB_ACTIONS)
    execute_process(COMMAND cmake -E create_symlink
        "${CMAKE_SOURCE_DIR}/lib"
        "${CMAKE_BINARY_DIR}/lib"
    )
    execute_process(COMMAND cmake -E create_symlink
        "${CMAKE_SOURCE_DIR}/python3.12"
        "${CMAKE_BINARY_DIR}/python3.12"
    )
endif()
