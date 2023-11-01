#include <iostream>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <stdexcept>

#include <resources.h>

namespace fs = std::filesystem;

template <typename T>
void SaveResource(const T& bytes, const std::filesystem::path& save_path) {
    constexpr size_t filesize = sizeof(T);

    std::ofstream ofs{save_path, std::ios::binary | std::ios::trunc};
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open file " + save_path.string());
    }
    ofs.write(reinterpret_cast<const char*>(&bytes), filesize);
    std::cout << "Wrote " << filesize << " bytes to " << save_path << std::endl;
}

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
        SaveResource(SAMPLE_TEXT_TXT_, save_dir / "sample_text.txt");
        SaveResource(LENA_PNG_, save_dir / "lena.png");
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    return 0;
}