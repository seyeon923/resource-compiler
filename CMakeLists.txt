cmake_minimum_required(VERSION 3.26)

project(resource_compiler VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(msvc_c "$<COMPILE_LANG_AND_ID:C,MSVC>")

# compiler warning options
add_compile_options(
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:/W4>>")

# MSVC /permissive- compile option
add_compile_options(
    "$<${msvc_cxx}:$<BUILD_INTERFACE:/permissive->>")

# MSVC utf-8 option
add_compile_options(
    "$<${msvc_cxx}:$<BUILD_INTERFACE:/utf-8>>"
    "$<${msvc_c}:$<BUILD_INTERFACE:/utf-8>>"
)

option(include_resource_compiler_tests "Include tests for resource_compiler" ON)

set(FETCHCONTENT_QUIET OFF)

include(FetchContent)

# Add nlohmann_json extern project
find_package(nlohmann_json 3.11.2...<4.0.0 QUIET)

if(NOT ${nlohmann_json_FOUND})
    message("Package nlohmann_json was not found, so include nlohmann_json by FetchContent")

    FetchContent_Declare(json 
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d)
    FetchContent_MakeAvailable(json)
endif()

include_directories(${PROJECT_SOURCE_DIR})

add_executable(resource_compiler 
    src/resource_compiler.cc
    src/write_cpp_source.h
    src/resources_info.h)

target_link_libraries(resource_compiler 
    PRIVATE nlohmann_json::nlohmann_json)

#include(compile_resources.cmake)

if(${include_resource_compiler_tests})
    enable_testing()
    add_subdirectory(tests)
endif()