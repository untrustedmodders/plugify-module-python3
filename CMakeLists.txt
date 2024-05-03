cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(py3-12-lang-module VERSION 1.0.0.0  DESCRIPTION "Python 3.12 Language Module" HOMEPAGE_URL "https://github.com/untrustedmodders/py3-12-lang-module" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
# Plugify
#
set(PLUGIFY_BUILD_SHARED_LIB ON CACHE INTERNAL "")
set(PLUGIFY_BUILD_FUNCTION ON CACHE INTERNAL "")
add_subdirectory(external/plugify)

#
# dyncall
#
# Must be last. Overwise, fail feature tests from Plugify
add_subdirectory(external/dyncall)

#
# Python 3 Language Module for Plugify
#
set(PY3LM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/module.cpp")
add_library(${PROJECT_NAME} SHARED ${PY3LM_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE plugify::plugify plugify::plugify-function asmjit::asmjit dyncall_s)
include_directories(external/dyncall/)

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
if(MSVC)
    # Linked by #pragma
    target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/windows/debug")
else()
    if(WIN32)
        set_property(TARGET python3 PROPERTY IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/windows/debug/python312_d.dll")
        set_property(TARGET python3 PROPERTY IMPORTED_IMPLIB_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/windows/debug/python312_d.lib")
        set_property(TARGET python3 PROPERTY IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/windows/release/python312.dll")
        set_property(TARGET python3 PROPERTY IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/windows/release/python312.lib")
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE python3)
endif()
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/${SYSTEM_NAME_LOWER}/include/pyconfig.h" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/include/python3.12")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/python3.12/include/python3.12")
