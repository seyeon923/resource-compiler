#ifndef RESOURCE_COMPILER_SRC_RESOURCES_INFO_H_
#define RESOURCE_COMPILER_SRC_RESOURCES_INFO_H_

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

#include <nlohmann/json.hpp>

#include "src/utils.h"

class ResourceInfo {
public:
    std::string key;
    std::string resource_path;

    ResourceInfo(const std::string& key, const std::string& resource_path)
        : key(key), resource_path(resource_path) {}
    ResourceInfo(std::string&& key, std::string&& resource_path)
        : key(std::move(key)), resource_path(std::move(resource_path)) {}
};

inline std::vector<ResourceInfo> GetResourcesInfo(
    const std::string& json_path) {
    namespace fs = std::filesystem;
    using json = nlohmann::json;

    auto resources_base_dir = fs::path{json_path}.parent_path();

    std::ifstream json_ifs{json_path};
    if (!json_ifs.is_open()) {
        throw std::runtime_error("Failed to open resources json file " +
                                 json_path);
    }

    json resources_json = json::parse(json_ifs);

    auto resources_it = resources_json.find("resources");
    if (resources_it == std::end(resources_json)) {
        throw std::runtime_error(
            "Cannot find \"resources\" entry from resources json");
    }

    std::vector<ResourceInfo> ret;
    for (auto& [idx, resource] : resources_it->items()) {
        const std::string resource_id{"resources[" + idx + "]"};

        std::string key, resource_path;

        auto key_it = resource.find("key");
        if (key_it == std::end(resource)) {
            throw std::runtime_error("Cannot find \"key\" entry from " +
                                     resource_id);
        }
        key = Trim(key_it->get<std::string>());

        auto resource_path_it = resource.find("resource_path");
        if (resource_path_it == std::end(resource)) {
            throw std::runtime_error(
                "Cannot find \"resource_path\" entry from" + resource_id);
        }
        resource_path = fs::canonical(resources_base_dir /
                                      resource_path_it->get<fs::path>())
                            .string();

        ret.emplace_back(std::move(key), std::move(resource_path));
    }
    return ret;
}

#endif  // RESOURCE_COMPILER_SRC_RESOURCES_INFO_H_