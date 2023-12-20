#ifndef RESOURCE_COMPILER_SRC_RESOURCES_INFO_H_
#define RESOURCE_COMPILER_SRC_RESOURCES_INFO_H_

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

#include <nlohmann/json.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "src/utils.h"

using json = nlohmann::json;

namespace fs = std::filesystem;

class ResourceInfo {
public:
    std::string key;
    std::string define_name;
    std::string resource_path;

    ResourceInfo(const std::string& key, const std::string& define_name,
                 const std::string& resource_path)
        : key(key), define_name(define_name), resource_path(resource_path) {}
    ResourceInfo(std::string&& key, std::string&& define_name,
                 std::string&& resource_path)
        : key(std::move(key)),
          define_name(std::move(define_name)),
          resource_path(std::move(resource_path)) {}
};

inline std::string GetRandomDefineName() {
    static boost::uuids::random_generator uuid_gen;
    boost::uuids::uuid uuid = uuid_gen();
    std::string define_name = "_" + boost::uuids::to_string(uuid);
    std::transform(std::begin(define_name), std::end(define_name),
                   std::begin(define_name), [](char ch) {
                       if (ch == '-') {
                           return '_';
                       } else {
                           return static_cast<char>(
                               std::toupper(static_cast<unsigned char>(ch)));
                       }
                   });
    return define_name;
}

inline std::vector<ResourceInfo> GetResourcesInfo(
    const std::string& json_path) {
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

        std::string key, define_name, resource_path;

        auto key_it = resource.find("key");
        if (key_it == std::end(resource)) {
            throw std::runtime_error("Cannot find \"key\" entry from " +
                                     resource_id);
        }
        key = Trim(key_it->get<std::string>());

        auto define_name_it = resource.find("define_name");
        if (define_name_it == std::end(resource)) {
            define_name = GetRandomDefineName();
        } else {
            define_name = Trim(define_name_it->get<std::string>());
        }

        auto resource_path_it = resource.find("resource_path");
        if (resource_path_it == std::end(resource)) {
            throw std::runtime_error(
                "Cannot find \"resource_path\" entry from" + resource_id);
        }
        resource_path = fs::canonical(resources_base_dir /
                                      resource_path_it->get<fs::path>())
                            .string();

        ret.emplace_back(std::move(key), std::move(define_name),
                         std::move(resource_path));
    }
    return ret;
}

#endif  // RESOURCE_COMPILER_SRC_RESOURCES_INFO_H_