set(resources_json "${PROJECT_SOURCE_DIR}/sample_data/resources.json")
set(resources_cpp_path "${CMAKE_CURRENT_BINARY_DIR}/__resources.cc")
set(resources_pub_header "${CMAKE_CURRENT_BINARY_DIR}/__include/seyeon/sample_compiled_resources.h")

add_custom_command(OUTPUT ${resources_cpp_path} ${resources_pub_header}
    COMMAND resource_compiler ${resources_json} ${resources_cpp_path} ${resources_pub_header} sample_compiled_resources seyeon dummy_ns
    DEPENDS resource_compiler ${resources_json}
            "${PROJECT_SOURCE_DIR}/sample_data/sample_text.txt"
            "${PROJECT_SOURCE_DIR}/sample_data/lena.png")

add_library(sample_compiled_resources SHARED
    ${resources_cpp_path}
    ${resources_pub_header})
target_include_directories(sample_compiled_resources PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/__include")
target_compile_definitions(sample_compiled_resources 
    PRIVATE EXPORT_SAMPLE_COMPILED_RESOURCES
    INTERFACE IMPORT_SAMPLE_COMPILED_RESOURCES)

add_executable(save_compiled_resources 
    save_compiled_resources.cc)
target_include_directories(save_compiled_resources PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(save_compiled_resources PRIVATE sample_compiled_resources)

add_test(NAME run_save_compiled_resources 
    COMMAND save_compiled_resources "${CMAKE_CURRENT_BINARY_DIR}/sample_compiled_resources")