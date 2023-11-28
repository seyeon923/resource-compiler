cmake_minimum_required(VERSION 3.26)

project(resource_compiler VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(include_resource_compiler_tests "Include tests for resource_compiler" ON)

# Add nlohmann_json extern project
find_package(nlohmann_json 3.11.2...<4.0.0)

if(NOT ${nlohmann_json_FOUND})
    message("Package nlohmann_json was not found, so include nlohmann_json by FetchContent")

    include(FetchContent)

    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
    FetchContent_MakeAvailable(json)
endif()

add_executable(resource_compiler resource_compiler.cc)

target_link_libraries(resource_compiler PRIVATE nlohmann_json::nlohmann_json)

if(${include_resource_compiler_tests})
    enable_testing()
    add_subdirectory(tests)
endif()