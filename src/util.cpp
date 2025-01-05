#include "util.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "third-party/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third-party/stb_image_write.h"
#include "third-party/PerlinNoise.hpp"
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

uint32_t flo::rgb_to_pixel(const rgb_color& rgb)
{
    return 0xFF000000 | (rgb.blue << 16) | (rgb.green << 8) | rgb.red;
}

flo::rgb_color flo::pixel_to_rgb(uint32_t pixel) {
    rgb_color rgb;
    rgb.red = pixel & 0xFF;           
    rgb.green = (pixel >> 8) & 0xFF;  
    rgb.blue = (pixel >> 16) & 0xFF;  
    return rgb;
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

flo::image flo::read_from_file(const std::string& fname)
{
    int wd, hgt, n;
    unsigned char *data = stbi_load(fname.c_str(), &wd, &hgt, &n, 4);
    flo::image img(wd,hgt);
    for (int y = 0; y < hgt; ++y) {
        for (int x = 0; x < wd; ++x) {
            auto ptr = data + y * wd * 4 + x * 4;
            uint32_t pix = *reinterpret_cast<uint32_t*>(ptr);
            img[x, y] = pix;
        }
    }
    return img;
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

flo::scalar_field flo::perlin_noise(const flo::dimensions& sz, uint32_t seed, int octaves, double freq) {
    auto noise = scalar_field(sz.hgt, sz.wd, 0.0);

    siv::PerlinNoise perlin{ seed };
    auto dim = std::max(sz.wd, sz.hgt);
    double freq_per_pix = freq / dim;

    for (auto y = 0; y < sz.hgt; ++y) {
        for (auto x = 0; x < sz.wd; ++x) {
            auto value = perlin.octave2D_01(x * freq_per_pix, y * freq_per_pix, octaves);
            noise[x,y] = value;
        }
    }

    return noise;
}

flo::vector_field flo::perlin_vector_field(
        const flo::dimensions& sz, uint32_t seed1, uint32_t seed2,
        int octaves, double freq) {

    auto x_comp = perlin_noise(sz, seed1, octaves, freq);
    auto y_comp = perlin_noise(sz, seed2, octaves, freq);

    // Find the maximum magnitude using ranges::max and entries
    auto max_magnitude = std::ranges::max(
        std::views::zip(x_comp.entries(), y_comp.entries())
        | std::views::transform([](auto&& pair) {
            auto [v1, v2] = pair;
            return std::sqrt(v1 * v1 + v2 * v2);
            })
    );

    // Normalize vectors if max_magnitude > 1.0
    if (max_magnitude > 1.0) {
        x_comp.transform(
            [&, max_magnitude](double value) {
                return value / max_magnitude;
            }
        );

        y_comp.transform(
            [&, max_magnitude](double value) {
                return value / max_magnitude;
            }
        );
    }

    return { x_comp, y_comp };
}

flo::rgb_color flo::random_rgb_color() {
    flo::rgb_color color;
    color.red = static_cast<uint8_t>(std::round(uniform_rand(0.0, 255.0)));
    color.green = static_cast<uint8_t>(std::round(uniform_rand(0.0, 255.0)));
    color.blue = static_cast<uint8_t>(std::round(uniform_rand(0.0, 255.0)));
    return color;
}
