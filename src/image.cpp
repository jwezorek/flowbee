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


flo::image::image(int cols, int rows) : cols_(cols), rows_(rows), impl_(cols* rows)
{
}

uint32_t& flo::image::operator[](int x, int y) {
    return impl_[y * cols_ + x];
}

const uint32_t& flo::image::operator[](int x, int y) const {
    return impl_[y * cols_ + x];
}

void* flo::image::data() const
{
    return reinterpret_cast<void*>(const_cast<uint32_t*>(impl_.data()));
}

int flo::image::cols() const
{
    return cols_;
}

int flo::image::rows() const
{
    return rows_;
}

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