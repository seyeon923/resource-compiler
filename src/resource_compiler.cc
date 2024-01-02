#include <iostream>
#include <filesystem>
#include <cctype>
#include <algorithm>

#include <nlohmann/json.hpp>

#include "src/resources_info.h"
#include "src/write_cpp_source.h"
#include "src/write_public_header.h"

namespace fs = std::filesystem;

using json = nlohmann::json;

inline std::string NormalizePath(const std::string& path) {
    std::string ret = fs::weakly_canonical(fs::absolute(path)).string();
    std::replace(std::begin(ret), std::end(ret), '\\', '/');
    return ret;
}

inline std::string FixCppExtension(const std::string& cpp_path_str) {
    auto cpp_path = fs::path{cpp_path_str};
    std::string ext = cpp_path.extension().string();
    std::string upper_ext(ext.size(), ' ');
    std::transform(
        std::begin(ext), std::end(ext), std::begin(upper_ext),
        [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    if (upper_ext != ".CC" && upper_ext != ".CPP" && upper_ext != ".CXX") {
        std::string filename = cpp_path.filename().string();

        std::string new_path;
        if (ext.empty()) {
            filename += ".cc";
        } else {
            filename = filename.substr(0, filename.find_last_of('.')) + ".cc";
        }
        auto parent_path = cpp_path.parent_path().string();
        if (parent_path.empty()) {
            new_path = std::move(filename);
        } else {
            new_path = parent_path + "/" + filename;
        }

        std::cerr << "C++ source file \"" << cpp_path_str
                  << "\" doesn't have C++ extension, "
                  << "so C++ source will be written to \"" << new_path << '"'
                  << std::endl;
        return new_path;
    } else {
        return cpp_path_str;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <resources json file>"
                  << " <c++ source file> <public header file> <library name> "
                     "[extra c++ namespaces ...]"
                  << std::endl;
        return 1;
    }

    try {
        const std::string resources_json_path{argv[1]};
        const std::string cpp_source_path{FixCppExtension(argv[2])};
        const std::string pub_header_path{argv[3]};
        const std::string_view libname{argv[4]};

        std::vector<std::string> extra_cc_namespaces;
        if (argc > 5) {
            for (int i = 5; i < argc; ++i) {
                extra_cc_namespaces.push_back(argv[i]);
            }
        }

        auto resources_info = GetResourcesInfo(resources_json_path);

        auto [function_prefix, define_prefix] = GetPrefixes(libname);

        cc_source::WriteCppSource(cpp_source_path, pub_header_path,
                                  resources_info, function_prefix,
                                  define_prefix);
        public_header::WritePublicHeader(pub_header_path, function_prefix,
                                         define_prefix, extra_cc_namespaces);

    } catch (std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}