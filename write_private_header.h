#include <fstream>
#include <string>
#include <iomanip>

#include "resources_info.h"

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
    const std::string& private_header_path,
    const std::vector<ResourceInfo>& resources_info) {
    std::ofstream header_ofs{private_header_path};
    if (!header_ofs.is_open()) {
        throw std::runtime_error("Failed to open out header file " +
                                 private_header_path);
    }

    constexpr char HEADER_GUARD[] =
        "SEYEON_RESOURCE_COMPILER_PRIVATE_RESOURCES_H_";

    header_ofs << "#ifndef " << HEADER_GUARD << '\n';
    header_ofs << "#define " << HEADER_GUARD << '\n';
    header_ofs << '\n';
    header_ofs << "#include <cstdint>\n";
    header_ofs << '\n';

    for (auto& resource_info : resources_info) {
        WriteResourceToPrivateHeader(header_ofs, resource_info);
    }

    header_ofs << '\n';
    header_ofs << "#endif // " << HEADER_GUARD;

    header_ofs.close();
}