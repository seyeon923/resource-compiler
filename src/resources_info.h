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

// inline std::vector<ResourceInfo> GetResourcesInfo(
//     const std::string& json_path) {
//     auto resources_base_dir = fs::path{json_path}.parent_path();

//     std::ifstream json_ifs{json_path};
//     if (!json_ifs.is_open()) {
//         throw std::runtime_error("Failed to open resources json file " +
//                                  json_path);
//     }

//     json resources_json = json::parse(json_ifs);

//     auto resources_it = resources_json.find("resources");
//     if (resources_it == std::end(resources_json)) {
//         throw std::runtime_error(
//             "Cannot find \"resources\" entry from resources json");
//     }

//     std::vector<ResourceInfo> ret;
//     for (auto& [idx, resource] : resources_it->items()) {
//         const std::string resource_id{"resources[" + idx + "]"};

//         std::string key, define_name, resource_path;

//         auto key_it = resource.find("key");
//         if (key_it == std::end(resource)) {
//             throw std::runtime_error("Cannot find \"key\" entry from " +
//                                      resource_id);
//         }
//         key = Trim(key_it->get<std::string>());

//         auto define_name_it = resource.find("define_name");
//         if (define_name_it == std::end(resource)) {
//             define_name = GetRandomDefineName();
//         } else {
//             define_name = Trim(define_name_it->get<std::string>());
//         }

//         auto resource_path_it = resource.find("resource_path");
//         if (resource_path_it == std::end(resource)) {
//             throw std::runtime_error(
//                 "Cannot find \"resource_path\" entry from" + resource_id);
//         }
//         resource_path = fs::canonical(resources_base_dir /
//                                       resource_path_it->get<fs::path>())
//                             .string();

//         ret.emplace_back(std::move(key), std::move(define_name),
//                          std::move(resource_path));
//     }
//     return ret;
// }


inline ResourceInfo GetResourcesInfo(
    const std::string process_name, const std::string model_name,
    const std::string idx, json resource,
    const fs::path& resources_base_dir) {
    
    const std::string resource_id{process_name + ":" + model_name + "[" + idx + "]"};

    std::string key, define_name, resource_path;

    auto key_it = resource.find("key");
    if (key_it == std::end(resource)) {
        throw std::runtime_error("Cannot find \"key\" entry from " +
                                 resource_id);
    }

    auto key_post_fix = ':' + process_name + ':' + model_name;
    key = Trim(key_it->get<std::string>() + key_post_fix);

    auto define_name_it = resource.find("define_name");
    if (define_name_it == std::end(resource)) {
        define_name = GetRandomDefineName();
    } else {
        define_name = Trim(define_name_it->get<std::string>());
    }

    auto resource_path_it = resource.find("resource_path");
    if (resource_path_it == std::end(resource)) {
        throw std::runtime_error("Cannot find \"resource_path\" entry from" +
                                 resource_id);
    }
    resource_path =
        fs::canonical(resources_base_dir / resource_path_it->get<fs::path>())
            .string();

    return ResourceInfo(key, define_name, resource_path);
}

inline std::vector<ResourceInfo> GetResourcesInfo(
    const std::string& json_path) {
    std::ifstream json_ifs{json_path};
    if (!json_ifs.is_open()) {
        throw std::runtime_error("Failed to open resources json file " +
                                 json_path);
    }
    auto resources_base_dir = fs::path{json_path}.parent_path();

    json resources_json = json::parse(json_ifs);
    if (resources_json.empty()) {
        throw std::runtime_error(
            "Cannot find processes entry from resources json");
    }

    std::vector<ResourceInfo> ret{};

    for (auto it_proc = resources_json.begin(); it_proc != resources_json.end(); ++it_proc) {                
        const auto& process_name = it_proc.key();
        const auto& process_json = it_proc.value();
        if (process_json.empty()) {
            throw std::runtime_error(
                "Empty model name entry from \"process_name\" json");
        }
        
        for (auto it_model = process_json.begin(); it_model != process_json.end(); ++it_model) {            

            for (auto& [idx, resource] : it_model->items()) {
                auto resource_info = GetResourcesInfo(process_name, it_model.key(),
                                                      idx, resource,
                                                      resources_base_dir);                                                 
                ret.emplace_back(std::move(resource_info));
            }
        }
    }

    // std::string build_info[][2] = {
    //     {"solder", "all"},
    //     {"dispenser", "bwi_a"},
    //     {"dispenser", "bwi_b"},
    //     {"dispenser", "mw"}
    // };

    // std::vector<ResourceInfo> ret{};

    // for (const auto& infos : build_info) {
    //     const auto&[process_name, model_name] = infos;
        
    //     if (resources_json.find(process_name) == std::end(resources_json)) {
    //         throw std::runtime_error(
    //             "Cannot find \"process_name\" entry from resources json");
    //     }
    //     json process_json = resources_json[process_name];
        
    //     auto model_it = process_json.find(model_name);
    //     if (model_it == std::end(process_json)) {
    //         throw std::runtime_error(
    //             "Cannot find \"model_name\" entry from " + process_name);
    //     }

    //     for (auto& [idx, resource] : model_it->items()) {
    //         auto resource_info = GetResourcesInfo(json_path, 
    //                                               idx, resource,
    //                                               process_name, model_name);
    //         ret.emplace_back(std::move(resource_info));
    //     }
    // }

    return ret;
}

#endif  // RESOURCE_COMPILER_SRC_RESOURCES_INFO_H_