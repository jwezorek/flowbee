#include "image.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third-party/stb_image_write.h"
#include <ranges>
#include <filesystem>
#include <stdexcept>
#include <format>

namespace fs = std::filesystem;
namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

void flo::write_to_file(const std::string& fname, const image& img) {
    auto extension = fs::path(fname).extension().string();
    if (extension != ".png" && extension != ".bmp") {
        throw std::runtime_error("unknown output image format");
    }
    int result = 0;
    if (extension == ".png") {
        result = stbi_write_png(
            fname.c_str(), img.cols(), img.rows(), 4, img.data(), 4 * img.cols()
        );
    }
    else {
        result = stbi_write_bmp(
            fname.c_str(), img.cols(), img.rows(), 4, img.data()
        );
    }

    if (!result) {
        throw std::runtime_error(
            std::format("unknown error while writing {}", extension)
        );
    }
}