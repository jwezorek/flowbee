#include "util.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third-party/stb_image_write.h"
#include <ranges>
#include <filesystem>
#include <stdexcept>
#include <format>
#include <random>


namespace fs = std::filesystem;
namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    std::random_device rd;
    std::mt19937 g_generator(rd());

    double normalize(double value, double min, double max) {
        return (value - min) / (max - min);
    }

    uint32_t to_grayscale_pixel(double val) {
        uint32_t gray = static_cast<uint8_t>(std::round(val * 255.0));
        return 0xFF000000 | (gray << 16) | (gray << 8) | gray;
    }

    double uniform_rand(double low = 0.0, double high = 1.0) {
        static std::uniform_real_distribution<double> distribution(low, high);
        return distribution(g_generator);
    }

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

flo::scalar_field flo::normalize(const scalar_field& s) {
    auto [min, max] = r::minmax(s.entries());
    return s.transform_to(
        [&](double val) {
            return ::normalize(val, min, max);
        }
    );
}

flo::image flo::to_gray_scale_image(const scalar_field& sf, bool invert) {
    auto s = normalize(sf);
    if (invert) {
        s.transform([](auto v) {return 1.0 - v; });
    }
    return s.transform_to<uint32_t>(to_grayscale_pixel);
}

flo::scalar_field flo::white_noise(int wd, int hgt) { 

    scalar_field noise(wd, hgt);
    noise.transform(
        [](double) {
            return uniform_rand();
        }
    );

    return noise;
}
