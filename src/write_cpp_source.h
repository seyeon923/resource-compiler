#include <string>
#include <fstream>
#include <cctype>

#include "src/resources_info.h"
#include "utils.h"

namespace cc_source {
constexpr char IMPLEMENTATIONS_TEMPLATE[] = R"(
int {{function_prefix}}_get_resource(char const* key,
                                           uint8_t const** resource_data,
                                           size_t* resource_size) {
    try {
        auto it = resource_map.find(key);
        if (it == std::end(resource_map)) {
            return {{define_prefix}}_NOT_FOUND_ERROR;
        }

        *resource_data = it->second.first;
        *resource_size = it->second.second;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return {{define_prefix}}_UNKNOWN_ERROR;
    }

    return {{define_prefix}}_NO_ERROR;
}

int {{function_prefix}}_save_resource(char const* key,
                                            char const* filename) {
    using namespace std::string_literals;
    int ret = {{define_prefix}}_NOT_FOUND_ERROR;
    try {
        uint8_t const* resource_data;
        size_t resource_size;
        ret = {{function_prefix}}_get_resource(key, &resource_data,
                                                     &resource_size);
        if (ret != {{define_prefix}}_NO_ERROR) {
            return ret;
        }

        std::ofstream ofs{filename, std::ios::binary};
        if (!ofs.is_open()) {
            ret = {{define_prefix}}_FILE_OPEN_ERROR;
            throw std::runtime_error("Failed to open file "s + filename);
        }

        ofs.write(reinterpret_cast<char const*>(resource_data), resource_size);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        if (ret == {{define_prefix}}_NO_ERROR) {
            ret = {{define_prefix}}_UNKNOWN_ERROR;
        }
    }

    return ret;
}
)";

inline std::ofstream& WriteResourceToPrivateHeader(
    std::ofstream& ofs, const ResourceInfo& resource_info) {
    auto& resource_path = resource_info.resource_path;
    auto& define_name = resource_info.define_name;

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

    ofs << "constexpr uint8_t " << define_name << "[] = {\n" << std::hex;
    int cnt = 0;
    for (auto byte : resource_data) {
        if (cnt == 0) {
            ofs << '\t';
        }
        ofs << "0x" << std::setfill('0') << std::setw(2)
            << static_cast<int>(byte) << ", ";

        if (++cnt == 10) {
            cnt = 0;
            ofs << '\n';
        };
    }
    ofs << "\n};\n";

    return ofs;
}

inline void WritePrivateHeader(
    std::ofstream& header_ofs,
    const std::vector<ResourceInfo>& resources_info) {
    header_ofs << "#include <cstdint>\n";
    header_ofs << '\n';

    for (auto& resource_info : resources_info) {
        WriteResourceToPrivateHeader(header_ofs, resource_info);
    }
}

inline void WriteCppSource(const std::string& cpp_source_path,
                           const std::vector<ResourceInfo>& resources_info,
                           const std::string& function_prefix,
                           const std::string& define_prefix) {
    std::ofstream cpp_source_ofs{cpp_source_path};
    if (!cpp_source_ofs.is_open()) {
        throw std::runtime_error("Failed to open file " + cpp_source_path +
                                 " to write C++ source");
    }

    cpp_source_ofs << "#include \"seyeon/compiled_resources.h\"\n\n";

    cpp_source_ofs << "#include <map>\n"
                      "#include <iostream>\n"
                      "#include <fstream>\n\n";

    WritePrivateHeader(cpp_source_ofs, resources_info);

    cpp_source_ofs << "static std::map<std::string, std::pair<uint8_t const*, "
                      "size_t>> resource_map{\n";
    for (auto& resource_info : resources_info) {
        cpp_source_ofs << "{\"" << resource_info.key << "\", {"
                       << resource_info.define_name << ", sizeof("
                       << resource_info.define_name << ")}},\n";
    }
    cpp_source_ofs << "};\n\n";

    std::string implementations = ReplaceAll(
        IMPLEMENTATIONS_TEMPLATE, "{{function_prefix}}", function_prefix);
    implementations =
        ReplaceAll(implementations, "{{define_prefix}}", define_prefix);

    cpp_source_ofs << implementations;
}
}  // namespace cc_source