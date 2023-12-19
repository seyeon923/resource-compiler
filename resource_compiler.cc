#include <iostream>
#include <filesystem>
#include <cctype>
#include <algorithm>

#include <nlohmann/json.hpp>

#include "resources_info.h"
#include "write_private_header.h"
#include "write_cpp_source.h"
#include "write_public_header.h"

namespace fs = std::filesystem;

using json = nlohmann::json;

inline std::string NormalizePath(const std::string& path) {
    std::string ret = fs::weakly_canonical(path).string();
    std::replace(std::begin(ret), std::end(ret), '\\', '/');
    return ret;
}

inline std::string FixCppExtension(const std::string& cpp_path_str) {
    auto cpp_path = fs::path{cpp_path_str};
    std::string ext = cpp_path.extension().string();
    std::string upper_ext(ext.size(), ' ');
    std::transform(std::begin(ext), std::end(ext), std::begin(upper_ext),
                   [](unsigned char ch) { return std::toupper(ch); });
    if (upper_ext != ".CC" && upper_ext != ".CPP" && upper_ext != ".CXX") {
        std::string filename = cpp_path.filename().string();

        std::string new_path;
        if (ext.empty()) {
            filename += ".cc";
        } else {
            filename = filename.substr(0, filename.find_last_of(ext)) + ".cc";
        }
        new_path = cpp_path.parent_path().string() + "/" + filename;

        std::cerr << "C++ source file \"" << cpp_path_str
                  << "\" doesn't have C++ extension, "
                  << "so C++ source will be written to " << new_path
                  << std::endl;
        return new_path;
    } else {
        return cpp_path_str;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <resources json file>"
                  << " <private resource header file>"
                  << " <c++ source file>"
                  << " <public header path>" << std::endl;
        return 1;
    }

    const std::string resources_json_path{argv[1]};
    const std::string private_header_path{NormalizePath(argv[2])};
    const std::string cpp_source_path{FixCppExtension(argv[3])};
    const std::string public_header_path{NormalizePath(argv[4])};

    try {
        auto resources_info = GetResourcesInfo(resources_json_path);

        WritePrivateHeader(private_header_path, resources_info);
        WriteCppSource(cpp_source_path, private_header_path, public_header_path,
                       resources_info);
        WritePublicHeader(public_header_path);

    } catch (std::exception& ex) {
        std::cerr << "Unexpected Error: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}