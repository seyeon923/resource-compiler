#include <string>
#include <fstream>

#include "src/resources_info.h"

constexpr char IMPLEMENTATIONS[] = R"(
int seyeon_compiled_resources_get_resource(char const* key,
                                           uint8_t const** resource_data,
                                           size_t* resource_size) {
    try {
        auto it = resource_map.find(key);
        if (it == std::end(resource_map)) {
            return SEYEON_COMPILED_RESOURCES_NOT_FOUND_ERROR;
        }

        *resource_data = it->second.first;
        *resource_size = it->second.second;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return SEYEON_COMPILED_RESOURCES_UNKNOWN_ERROR;
    }

    return SEYEON_COMPILED_RESOURCES_NO_ERROR;
}

int seyeon_compiled_resources_save_resource(char const* key,
                                            char const* filename) {
    using namespace std::string_literals;
    int ret = SEYEON_COMPILED_RESOURCES_NOT_FOUND_ERROR;
    try {
        uint8_t const* resource_data;
        size_t resource_size;
        ret = seyeon_compiled_resources_get_resource(key, &resource_data,
                                                     &resource_size);
        if (ret != SEYEON_COMPILED_RESOURCES_NO_ERROR) {
            return ret;
        }

        std::ofstream ofs{filename, std::ios::binary};
        if (!ofs.is_open()) {
            ret = SEYEON_COMPILED_RESOURCES_FILE_OPEN_ERROR;
            throw std::runtime_error("Failed to open file "s + filename);
        }

        ofs.write(reinterpret_cast<char const*>(resource_data), resource_size);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        if (ret == SEYEON_COMPILED_RESOURCES_NO_ERROR) {
            ret = SEYEON_COMPILED_RESOURCES_UNKNOWN_ERROR;
        }
    }

    return ret;
}
)";

inline void WriteCppSource(const std::string& cpp_source_path,
                           const std::string& private_header_path,
                           const std::string& public_header_path,
                           const std::vector<ResourceInfo>& resources_info) {
    std::ofstream cpp_source_ofs{cpp_source_path};

    cpp_source_ofs << "#include \"" << public_header_path << "\"\n\n";

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

    cpp_source_ofs << IMPLEMENTATIONS;
}