#include <string>
#include <fstream>
#include <cctype>

#include "src/resources_info.h"
#include "utils.h"

constexpr char IMPLEMENTATIONS_TEMPLATE[] = R"(
int {{function_prefix}}_get_resource(char const* key,
                                           uint8_t const** resource_data,
                                           size_t* resource_size) {
    try {
        auto it = resource_map.find(key);
        if (it == std::end(resource_map)) {
            return {{upper_case_function_prefix}}_NOT_FOUND_ERROR;
        }

        *resource_data = it->second.first;
        *resource_size = it->second.second;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return {{upper_case_function_prefix}}_UNKNOWN_ERROR;
    }

    return {{upper_case_function_prefix}}_NO_ERROR;
}

int {{function_prefix}}_save_resource(char const* key,
                                            char const* filename) {
    using namespace std::string_literals;
    int ret = {{upper_case_function_prefix}}_NOT_FOUND_ERROR;
    try {
        uint8_t const* resource_data;
        size_t resource_size;
        ret = {{function_prefix}}_get_resource(key, &resource_data,
                                                     &resource_size);
        if (ret != {{upper_case_function_prefix}}_NO_ERROR) {
            return ret;
        }

        std::ofstream ofs{filename, std::ios::binary};
        if (!ofs.is_open()) {
            ret = {{upper_case_function_prefix}}_FILE_OPEN_ERROR;
            throw std::runtime_error("Failed to open file "s + filename);
        }

        ofs.write(reinterpret_cast<char const*>(resource_data), resource_size);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        if (ret == {{upper_case_function_prefix}}_NO_ERROR) {
            ret = {{upper_case_function_prefix}}_UNKNOWN_ERROR;
        }
    }

    return ret;
}
)";

inline void WriteCppSource(
    const std::string& cpp_source_path, const std::string& private_header_path,
    const std::vector<ResourceInfo>& resources_info,
    std::string_view function_prefix = "seyeon_compiled_resources") {
    std::ofstream cpp_source_ofs{cpp_source_path};

    cpp_source_ofs << "#include \"seyeon/compiled_resources.h\"\n\n";

    cpp_source_ofs << "#include <map>\n"
                      "#include <iostream>\n"
                      "#include <fstream>\n\n";

    cpp_source_ofs << "#include \"" << private_header_path << "\"\n\n";

    cpp_source_ofs << "static std::map<std::string, std::pair<uint8_t const*, "
                      "size_t>> resource_map{\n";
    for (auto& resource_info : resources_info) {
        cpp_source_ofs << "{\"" << resource_info.key << "\", {"
                       << resource_info.define_name << ", sizeof("
                       << resource_info.define_name << ")}},\n";
    }
    cpp_source_ofs << "};\n\n";

    std::string uppercase_function_prefix(function_prefix);
    std::transform(std::begin(uppercase_function_prefix),
                   std::end(uppercase_function_prefix),
                   std::begin(uppercase_function_prefix), [](unsigned char c) {
                       return static_cast<char>(std::toupper(c));
                   });

    std::string implementations = ReplaceAll(
        IMPLEMENTATIONS_TEMPLATE, "{{function_prefix}}", function_prefix);
    implementations =
        ReplaceAll(implementations, "{{upper_case_function_prefix}}",
                   uppercase_function_prefix);

    cpp_source_ofs << implementations;
}