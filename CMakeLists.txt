cmake_minimum_required(VERSION 3.26)

project(resource_compiler VERSION 0.0.0)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")

add_subdirectory(extern/json)

add_executable(resource_compiler resource_compiler.cc)

target_link_libraries(resource_compiler PRIVATE nlohmann_json::nlohmann_json)