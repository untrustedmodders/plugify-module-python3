cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()


project(py3-12-lang-module VERSION 1.0.0.0  DESCRIPTION "Python 3.12 Language Module" HOMEPAGE_URL "https://github.com/untrustedmodders/py3-12-lang-module" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

#
# Plugify
#
set(PLUGIFY_BUILD_SHARED_LIB ON CACHE INTERNAL "")
set(PLUGIFY_BUILD_FUNCTION ON CACHE INTERNAL "")
if(LINUX)
    set(PLUGIFY_USE_STATIC_STDLIB ON CACHE INTERNAL "")
endif()
add_subdirectory(external/plugify)

#
# dyncall
#
include(cmake/dyncall.cmake)

#
# Python 3 Language Module for Plugify
#
set(PY3LM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/module.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/module.cpp")
add_library(${PROJECT_NAME} SHARED ${PY3LM_SOURCES})

set(PY3LM_LINK_LIBRARIES plugify::plugify plugify::plugify-function asmjit::asmjit dyncall_s)

if(NOT COMPILER_SUPPORTS_FORMAT)
    set(PY3LM_LINK_LIBRARIES ${PY3LM_LINK_LIBRARIES} fmt::fmt-header-only)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${PY3LM_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${dyncall_SOURCE_DIR})

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_MACRO_NAME PY3LM_EXPORT EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/module_export.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/exports)

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

if(MSVC)
    # Linked by #pragma
    target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/windows/${PYTHON_ABSTRACT_BUILD_TYPE_LOWER}/bin")
else()
    if(WIN32)
        set_property(TARGET python3 PROPERTY IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/windows/debug/bin/python312_d.dll")
        set_property(TARGET python3 PROPERTY IMPORTED_IMPLIB_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/windows/debug/bin/python312_d.lib")
        set_property(TARGET python3 PROPERTY IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/windows/release/bin/python312.dll")
        set_property(TARGET python3 PROPERTY IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/windows/release/bin/python312.lib")
    elseif(LINUX)
        set_property(TARGET python3 PROPERTY IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/linux/debug/bin/libpython3.12d.so.1.0")
        set_property(TARGET python3 PROPERTY IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/linux/release/bin/libpython3.12.so.1.0")
        set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_FLAGS "-Wl,-rpath,\\\$ORIGIN/../python3.12")
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE python3)
endif()
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/include/python3.12" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/_pyinclude")
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${SYSTEM_NAME_LOWER}/${PYTHON_ABSTRACT_BUILD_TYPE_LOWER}/include/pyconfig.h" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/_pyinclude/python3.12")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/_pyinclude/python3.12")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PY3LM_PLATFORM_WINDOWS=$<BOOL:${WIN32}>
    PY3LM_PLATFORM_APPLE=$<BOOL:${APPLE}>
    PY3LM_PLATFORM_LINUX=$<BOOL:${LINUX}>
    PY3LM_IS_DEBUG=$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>)

set(PY3LM_VERSION "0" CACHE STRING "Set version name")
set(PY3LM_PACKAGE "${PROJECT_NAME}" CACHE STRING "Set package name")

configure_file(
    ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.pmodule.in
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pmodule
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
