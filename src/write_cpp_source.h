#include <string>
#include <fstream>
#include <cctype>
#include <filesystem>

#include "src/resources_info.h"
#include "utils.h"

namespace cc_source {

constexpr char INCLUDE[] = R"(
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstdint>
)";

constexpr char GET_RESOURCE_IMPLEMENTATION_OPEN_TEMPLATE[] = R"(
int {{function_prefix}}_get_resource(char const* key,
                                           uint8_t** resource_data,
                                           size_t* resource_size,
                                           void* (*malloc_fn)(size_t),
                                           void (*free_fn)(void*)) {
    using namespace std::string_literals;

    int ret = {{define_prefix}}_NO_ERROR;
    *resource_data = nullptr;
    *resource_size = 0;

    try {
        std::string key_str{key};
)";
constexpr char GET_RESOURCE_IMPLEMENTATION_CLOSE_TEMPLATE[] = R"(
        else {
            ret = {{define_prefix}}_NOT_FOUND_ERROR;
        }
    } catch (std::exception& e) {
        if (ret == {{define_prefix}}_NO_ERROR) {
            ret = {{define_prefix}}_UNKNOWN_ERROR;
        }
        if (free_fn != nullptr) {
            free_fn(*resource_data);
        }
        *resource_data = nullptr;
        *resource_size = 0;
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return ret;
}
)";
constexpr char SAVE_RESOURCE_IMPLEMENTATION_TEMPLATE[] = R"(
int {{function_prefix}}_save_resource(char const* key,
                                            char const* filename) {
    using namespace std::string_literals;
    int ret = {{define_prefix}}_NOT_FOUND_ERROR;

    uint8_t* resource_data = nullptr;
    size_t resource_size;
    try {
        ret = {{function_prefix}}_get_resource(
            key, &resource_data, &resource_size, &malloc, &free);
        if (ret != {{define_prefix}}_NO_ERROR) {
            resource_data = nullptr;
            throw std::runtime_error(
                "{{function_prefix}}_get_resource failed");
        }

        std::ofstream ofs{filename, std::ios::binary};
        if (!ofs.is_open()) {
            ret = {{define_prefix}}_FILE_OPEN_ERROR;
            throw std::runtime_error("Failed to open file "s + filename);
        }

        ofs.write(reinterpret_cast<char const*>(resource_data), resource_size);
    } catch (std::exception& e) {
        if (ret == {{define_prefix}}_NO_ERROR) {
            ret = {{define_prefix}}_UNKNOWN_ERROR;
        }
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    free(resource_data);

    return ret;
}
)";

inline void WriteResourceData(std::ofstream& cpp_source_ofs,
                              const ResourceInfo& resource_info,
                              int base_indent = 12) {
    auto& resource_path = resource_info.resource_path;

    std::ifstream resource_ifs(resource_path, std::ios::binary);
    if (!resource_ifs.is_open()) {
        throw std::runtime_error("Failed to open resource file " +
                                 resource_path);
    }

    resource_ifs.seekg(0, std::ios::end);
    size_t filesize = resource_ifs.tellg();
    resource_ifs.seekg(0);

    std::vector<uint8_t> resource_data(filesize);
    resource_ifs.read(reinterpret_cast<char*>(resource_data.data()), filesize);

    std::string indent(base_indent, ' ');

    cpp_source_ofs << indent << "*resource_size = " << filesize << ";\n";
    cpp_source_ofs << indent
                   << "*resource_data = "
                      "static_cast<uint8_t*>(malloc_fn(*resource_size));\n\n";

    cpp_source_ofs << indent;
    for (size_t i = 0; i < filesize; ++i) {
        uint8_t byte = resource_data[i];

        cpp_source_ofs << "(*resource_data)[" << i
                       << "] = " << static_cast<int>(byte) << "; ";

        if ((i + 1) % 5 == 0) {
            cpp_source_ofs << '\n' << indent;
        }
    }
    cpp_source_ofs << '\n';
}

inline void WriteCppSource(const std::string& cpp_source_path,
                           const std::string& pub_header_path,
                           const std::vector<ResourceInfo>& resources_info,
                           const std::string& function_prefix,
                           const std::string& define_prefix) {
    namespace fs = std::filesystem;

    std::ofstream cpp_source_ofs{cpp_source_path};
    if (!cpp_source_ofs.is_open()) {
        throw std::runtime_error("Failed to open file " + cpp_source_path +
                                 " to write C++ source");
    }

    cpp_source_ofs << "#include \"" << fs::absolute(pub_header_path).string()
                   << "\"\n";

    cpp_source_ofs << INCLUDE << '\n';

    cpp_source_ofs << ReplaceAll(
        ReplaceAll(GET_RESOURCE_IMPLEMENTATION_OPEN_TEMPLATE,
                   "{{function_prefix}}", function_prefix),
        "{{define_prefix}}", define_prefix);

    for (size_t i = 0; i < resources_info.size(); ++i) {
        auto& resource_info = resources_info[i];

        cpp_source_ofs << "        ";
        if (i > 0) {
            cpp_source_ofs << "else ";
        }
        cpp_source_ofs << "if (key_str == \"" << resource_info.key
                       << "\"s) {\n";
        WriteResourceData(cpp_source_ofs, resource_info);
        cpp_source_ofs << "        }\n";
    }

    cpp_source_ofs << ReplaceAll(
                          ReplaceAll(GET_RESOURCE_IMPLEMENTATION_CLOSE_TEMPLATE,
                                     "{{function_prefix}}", function_prefix),
                          "{{define_prefix}}", define_prefix)
                   << '\n';

    cpp_source_ofs << ReplaceAll(
        ReplaceAll(SAVE_RESOURCE_IMPLEMENTATION_TEMPLATE, "{{function_prefix}}",
                   function_prefix),
        "{{define_prefix}}", define_prefix);
}
}  // namespace cc_source