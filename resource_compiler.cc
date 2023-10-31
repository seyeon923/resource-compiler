#include <iostream>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <ios>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

using json = nlohmann::json;

inline std::ofstream& WriteResource(
    std::ofstream& ofs, const std::string& key, const json& resource,
    const std::filesystem::path& resources_base_dir = {}) {
    const std::string resource_id{"resources[" + key + "]"};
    auto define_name_it = resource.find("define_name");
    if (define_name_it == std::end(resource)) {
        throw std::runtime_error("Cannot find \"define_name\" entry from " +
                                 resource_id);
    }
    auto resource_path_it = resource.find("resource_path");
    if (resource_path_it == std::end(resource)) {
        throw std::runtime_error("Cannot find \"resource_path\" entry from" +
                                 resource_id);
    }

    const std::string define_name{define_name_it->get<std::string>()};
    const fs::path resource_path{resources_base_dir /
                                 resource_path_it->get<fs::path>()};

    std::ifstream resource_ifs(resource_path, std::ios::binary);
    if (!resource_ifs.is_open()) {
        throw std::runtime_error("Failed to open resource file " +
                                 resource_path.string() + " of " + resource_id);
    }

    resource_ifs.seekg(0, std::ios::end);
    size_t filesize = resource_ifs.tellg();
    resource_ifs.seekg(0);

    std::vector<uint8_t> resource_data(filesize);
    resource_ifs.read(reinterpret_cast<char*>(resource_data.data()), filesize);

    ofs << "constexpr uint8_t " << define_name << "[] = {" << std::hex;
    for (auto byte : resource_data) {
        ofs << "0x" << static_cast<int>(byte) << ", ";
    }
    ofs << "};\n";

    return ofs;
}

inline int WriteResources(
    std::ofstream& ofs, const json& resources_json,
    const std::filesystem::path& resources_base_dir = {}) {
    try {
        auto header_guard_it = resources_json.find("header_guard");
        if (header_guard_it == std::end(resources_json)) {
            throw std::runtime_error(
                "Cannot find \"header_guard\" entry from resources json");
        }
        auto resources_it = resources_json.find("resources");
        if (resources_it == std::end(resources_json)) {
            throw std::runtime_error(
                "Cannot find \"resources\" entry from resources json");
        }
        const std::string header_guard{header_guard_it->get<std::string>()};
        ofs << "#ifndef " << header_guard << '\n';
        ofs << "#define " << header_guard << '\n';
        ofs << '\n';
        ofs << "#include <cstdint>\n";
        ofs << '\n';

        for (auto& [key, resource] : resources_it->items()) {
            WriteResource(ofs, key, resource, resources_base_dir) << '\n';
        }

        ofs << '\n';
        ofs << "#endif // " << header_guard;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <resources json file> <out header file>" << std::endl;
        return 1;
    }

    const std::string resources_json_path{argv[1]}, out_header_path{argv[2]};

    std::ifstream json_ifs{resources_json_path};
    if (!json_ifs.is_open()) {
        std::cerr << "Failed to open resources json file "
                  << resources_json_path << std::endl;
        return -1;
    }

    json resources_json;
    try {
        try {
            resources_json = json::parse(json_ifs);
        } catch (std::exception& ex) {
            std::cerr << "Failed to parse resources json file "
                      << resources_json_path << " - " << ex.what() << std::endl;
            return -1;
        }
        json_ifs.close();

        std::ofstream header_ofs{out_header_path};
        if (!header_ofs.is_open()) {
            std::cerr << "Failed to open out header file " << out_header_path
                      << std::endl;
            return -1;
        }

        if (int ret =
                WriteResources(header_ofs, resources_json,
                               fs::path{resources_json_path}.parent_path())) {
            return ret;
        }

        header_ofs.close();
        std::cout << "Resources header file is written to " << out_header_path
                  << std::endl;
    } catch (std::exception& ex) {
        std::cerr << "Unexpected Error: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}