

function(compile_resources target_lib_name target_lib_pub_header resources_file)
    list(LENGTH ARGN num_varargs)

    if(${num_varargs} LESS 1)
        message(FATAL_ERROR "No resources are passed to compile.")
        endif()
        
    math(EXPR remainder "${num_varargs} % 2")
    if(NOT (${remainder} EQUAL 0))
        message(FATAL_ERROR "Resources' key and path are not matched.")
    endif()

    message("target_lib_name = ${target_lib_name}")
    message("target_lib_pub_header = ${target_lib_pub_header}")
    message("resources_file = ${resources_file}")

    string(RANDOM LENGTH 64 random_filename)
    set(tmp_resources_file "${CMAKE_CURRENT_BINARY_DIR}/${random_filename}")

    set(resources "[]")

    set(resource_paths "")

    math(EXPR stop "${num_varargs} / 2 - 1")
    foreach(res_idx RANGE ${stop})
        math(EXPR key_idx "${res_idx} * 2")
        math(EXPR path_idx "${res_idx} * 2 + 1")

        list(GET ARGN ${key_idx} res_key)
        list(GET ARGN ${path_idx} res_path)

        get_filename_component(res_path ${res_path} ABSOLUTE)

        string(JSON resource SET "{}" "key" "\"${res_key}\"")
        string(JSON resource SET ${resource} "resource_path" "\"${res_path}\"")
        string(JSON resources SET ${resources} ${res_idx} ${resource})

        list(APPEND ${resource_paths} ${res_path})
    endforeach()

    string(JSON resources_json SET "{}" "resources" ${resources})

    file(WRITE ${tmp_resources_file} ${resources_json})
    file(COPY_FILE ${tmp_resources_file} ${resources_file} ONLY_IF_DIFFERENT)
    file(REMOVE ${tmp_resources_file})

    set(priv_header "${CMAKE_CURRENT_BINARY_DIR}/${random_filename}.h")
    set(cpp_source "${CMAKE_CURRENT_BINARY_DIR}/${random_filename}.cc")

    add_custom_command(OUTPUT ${priv_header} ${cpp_source} ${target_lib_pub_header}
        COMMAND resource_compiler ${resources_json} ${priv_header} ${cpp_source} ${target_lib_pub_header}
        DEPENDS resource_compiler ${resources_json} ${resource_paths})
endfunction()

compile_resources(lib header "${PROJECT_BINARY_DIR}/rcfile.json"
    key1 path1 
    key2 C:/a/b/c/path2)