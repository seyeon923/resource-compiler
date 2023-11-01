set(resources_header "${CMAKE_CURRENT_BINARY_DIR}/resources.h")
set(resources_json "${CMAKE_SOURCE_DIR}/sample_data/resources.json")

add_custom_command(OUTPUT ${resources_header}
    COMMAND resource_compiler ${resources_json} ${resources_header}
    DEPENDS resource_compiler ${resources_json})

add_executable(save_compiled_resources 
    save_compiled_resources.cc
    ${resources_header})

target_include_directories(save_compiled_resources PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME run_save_compiled_resources 
    COMMAND save_compiled_resources "${CMAKE_CURRENT_BINARY_DIR}/compiled_resources")