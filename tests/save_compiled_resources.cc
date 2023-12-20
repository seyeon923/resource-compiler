#include <iostream>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <stdexcept>

#include "seyeon/compiled_resources.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <resources save dir>"
                  << std::endl;
        return 1;
    }

    fs::path save_dir{argv[1]};
    if (!fs::exists(save_dir)) {
        fs::create_directories(save_dir);
    }

    try {
        seyeon::compiled_resources::SaveResource(
            "1.sample_text", (save_dir / "sample_text.txt").string());
        seyeon::compiled_resources::SaveResource(
            "2.lena", (save_dir / "lena.png").string());
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    return 0;
}