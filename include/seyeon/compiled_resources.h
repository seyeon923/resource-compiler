#ifndef SEYEON_RESOURCE_COMPILER_COMPILED_RESOURCES_H_
#define SEYEON_RESOURCE_COMPILER_COMPILED_RESOURCES_H_

#if defined(_WIN32)
#    if defined(EXPORT_SEYEON_COMPILED_RESOURCES)
#        define SEYEON_COMPILED_RESOURCES_API __declspec(dllexport)
#    elif defined(IMPORT_SEYEON_COMPILED_RESOURCES)
#        define SEYEON_COMPILED_RESOURCES_API __declspec(dllimport)
#    else
#        define SEYEON_COMPILED_RESOURCES_API
#    endif
#else  // non windows
#    define COMPILED_RESOURCES_API
#endif

#include <wchar.h>
#include <stdint.h>

#define SEYEON_COMPILED_RESOURCES_NO_ERROR 0
#define SEYEON_COMPILED_RESOURCES_UNKNOWN_ERROR -1
#define SEYEON_COMPILED_RESOURCES_NOT_FOUND_ERROR 1
#define SEYEON_COMPILED_RESOURCES_FILE_OPEN_ERROR 2

#ifdef __cplusplus
extern "C" {
#endif

SEYEON_COMPILED_RESOURCES_API int seyeon_compiled_resources_get_resource(
    char const* key, uint8_t const** resource_data, size_t* resource_size);
SEYEON_COMPILED_RESOURCES_API int seyeon_compiled_resources_save_resource(
    char const* key, char const* filename);

#ifdef __cplusplus
}

#    include <string>

namespace seyeon {
namespace compiled_resources {
int GetResource(const std::string& key, uint8_t const*& resource_data,
                size_t& resource_size) {
    return seyeon_compiled_resources_get_resource(key.c_str(), &resource_data,
                                                  &resource_size);
}
int SaveResource(const std::string& key, const std::string& filename) {
    return seyeon_compiled_resources_save_resource(key.c_str(),
                                                   filename.c_str());
}
}  // namespace compiled_resources
}  // namespace seyeon
#endif

#endif  // SEYEON_RESOURCE_COMPILER_COMPILED_RESOURCES_H_