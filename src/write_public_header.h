#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

#include "utils.h"

namespace public_header {

constexpr char HEADER_GUARD_OPEN_TEMPLATE[] =
    R"(
#ifndef {{define_prefix}}_COMPILED_RESOURCES_H_
#define {{define_prefix}}_COMPILED_RESOURCES_H_
)";
constexpr char HEADER_GUARD_CLOSE_TEMPLATE[] =
    R"(
#endif  // {{define_prefix}}_COMPILED_RESOURCES_H_
)";

constexpr char API_DEFINE_TEMPLATE[] = R"(
#if defined(_WIN32)
#    if defined(EXPORT_{{define_prefix}})
#        define {{define_prefix}}_API __declspec(dllexport)
#    elif defined(IMPORT_{{define_prefix}})
#        define {{define_prefix}}_API __declspec(dllimport)
#    else
#        define {{define_prefix}}_API
#    endif
#else  // non windows
#    define COMPILED_RESOURCES_API
#endif
)";

constexpr char C_INCLUDE[] = R"(
#include <wchar.h>
#include <stdint.h>
#include <stdlib.h>
)";

constexpr char CC_INCLUDE[] = R"(
#include <string>
)";

constexpr char C_DEFINE_TEMPLATE[] =
    R"(
#define {{define_prefix}}_NO_ERROR 0
#define {{define_prefix}}_UNKNOWN_ERROR -1
#define {{define_prefix}}_NOT_FOUND_ERROR 1
#define {{define_prefix}}_FILE_OPEN_ERROR 2
)";

constexpr char C_FUNCTION_TEMPLATE[] =
    R"(
// Get resources data identified by `key`
//
// - `key`: Resource ID
// - `resource_data`: Pointer to allocated memory pointer of resource data
// - `resource_size`: Pointer to resource data size
// - `malloc_fn` : Memory allocation function for allocating memory
//                 for the resource data
// - `free_fn` : Memory deallocation function in case of failing this function
{{define_prefix}}_API int {{function_prefix}}_get_resource(
    char const* key, uint8_t** resource_data, size_t* resource_size,
    void* (*malloc_fn)(size_t), void (*free_fn)(void*));

// Save resources identified by `key` to `filename` file
{{define_prefix}}_API int {{function_prefix}}_save_resource(
    char const* key, char const* filename);
)";

constexpr char CC_FUNCTION_TEMPLATE[] =
    R"(
int GetResource(const std::string& key, uint8_t*& resource_data,
                size_t& resource_size, void* (*malloc_fn)(size_t),
                void (*free_fn)(void*)) {
    return sample_compiled_resources_get_resource(
        key.c_str(), &resource_data, &resource_size, malloc_fn, free_fn);
}
int SaveResource(const std::string& key, const std::string& filename) {
    return sample_compiled_resources_save_resource(key.c_str(),
                                                   filename.c_str());
}
)";

inline void WritePublicHeader(
    const std::string& header_path, const std::string& function_prefix,
    const std::string& define_prefix,
    const std::vector<std::string>& extra_cc_namespaces) {
    namespace fs = std::filesystem;

    auto header_dir = fs::path{header_path}.parent_path();
    if (!fs::exists(header_dir)) {
        fs::create_directories(header_dir);
    }

    std::ofstream header_ofs{header_path};
    if (!header_ofs.is_open()) {
        throw std::runtime_error("Failed to open " + header_path);
    }

    const std::string define_prefix_placeholder = "{{define_prefix}}";
    const std::string function_prefix_placeholder = "{{function_prefix}}";

    header_ofs << ReplaceAll(HEADER_GUARD_OPEN_TEMPLATE,
                             define_prefix_placeholder, define_prefix)
               << '\n';

    header_ofs << ReplaceAll(API_DEFINE_TEMPLATE, define_prefix_placeholder,
                             define_prefix)
               << '\n';

    header_ofs << C_INCLUDE << '\n';

    header_ofs << "#ifdef __cplusplus\n";
    header_ofs << CC_INCLUDE << '\n';
    header_ofs << "#endif // __cplusplus\n";

    header_ofs << ReplaceAll(C_DEFINE_TEMPLATE, define_prefix_placeholder,
                             define_prefix)
               << '\n';

    header_ofs << "#ifdef __cplusplus\n"
                  "extern \"C\" {\n"
                  "#endif // __cplusplus\n";
    header_ofs << ReplaceAll(
                      ReplaceAll(C_FUNCTION_TEMPLATE, define_prefix_placeholder,
                                 define_prefix),
                      function_prefix_placeholder, function_prefix)
               << '\n';
    header_ofs << "#ifdef __cplusplus\n"
                  "}\n"
                  "#endif // __cplusplus\n";

    header_ofs << "#ifdef __cplusplus\n";

    for (const auto& cc_namespace : extra_cc_namespaces) {
        header_ofs << "namespace " << cc_namespace << " {\n";
    }
    header_ofs << "namespace " << function_prefix << " {\n";
    header_ofs << ReplaceAll(CC_FUNCTION_TEMPLATE, function_prefix_placeholder,
                             function_prefix);
    header_ofs << "} // namespace " << function_prefix << '\n';
    for (auto it = extra_cc_namespaces.rbegin();
         it != extra_cc_namespaces.rend(); ++it) {
        header_ofs << "} // namespace " << *it << '\n';
    }
    header_ofs << "#endif // __cplusplus";

    header_ofs << ReplaceAll(HEADER_GUARD_CLOSE_TEMPLATE,
                             define_prefix_placeholder, define_prefix);
}

}  // namespace public_header