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
#include <stdexcept>

namespace fs = std::filesystem;
namespace r = std::ranges;
namespace rv = std::ranges::views;

/*------------------------------------------------------------------------------------------------*/

namespace {

    std::random_device rd;
    std::mt19937 g_generator( rd() );

    double normalize(double value, double min, double max) {
        return (value - min) / (max - min);
    }

    uint32_t to_grayscale_pixel(double val) {
        uint32_t gray = static_cast<uint8_t>(std::round(val * 255.0));
        return 0xFF000000 | (gray << 16) | (gray << 8) | gray;
    }

}

void flo::set_rand_seed(uint32_t seed) {
    g_generator = std::mt19937{ seed };
}

uint32_t flo::rgb_to_pixel(const rgb_color& rgb) {
    return 0xFF000000 | (rgb.blue << 16) | (rgb.green << 8) | rgb.red;
}

flo::rgb_color flo::pixel_to_rgb(uint32_t pixel) {
    rgb_color rgb;
    rgb.red = pixel & 0xFF;           
    rgb.green = (pixel >> 8) & 0xFF;  
    rgb.blue = (pixel >> 16) & 0xFF;  
    return rgb;
}

void flo::img_to_file(const std::string& fname, const image& img) {
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

flo::image flo::img_from_file(const std::string& fname)
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

flo::scalar_field flo::to_gray_scale(const image& img) {
    scalar_field gray(img.cols(), img.rows());
    for (int y = 0; y < img.rows(); ++y) {
        for (int x = 0; x < img.cols(); ++x) {
            rgb_color color = pixel_to_rgb(img[x, y]);
            gray[x, y] = (0.299 * color.red + 0.587 * color.green + 0.114 * color.blue) / 255.0;
        }
    }
    return gray;
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

flo::scalar_field flo::perlin_noise(const flo::dimensions& sz, int octaves, double freq) {
    auto noise = scalar_field(sz.wd, sz.hgt, 0.0);

    siv::PerlinNoise perlin{ g_generator() };
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

flo::rgb_color flo::random_rgb_color() {
    flo::rgb_color color;
    color.red = static_cast<uint8_t>(std::round(uniform_rand(0.0, 255.0)));
    color.green = static_cast<uint8_t>(std::round(uniform_rand(0.0, 255.0)));
    color.blue = static_cast<uint8_t>(std::round(uniform_rand(0.0, 255.0)));
    return color;
}

int flo::rand_number(int min, int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(g_generator);
}

double flo::normal_rand(double mean, double stddev) {
    if (stddev == 0.0) {
        return mean;
    }
    std::normal_distribution<double> distribution(mean, stddev);
    return distribution(g_generator);
}

double flo::uniform_rand(double low, double high) {
    static std::uniform_real_distribution<double> distribution(low, high);
    return distribution(g_generator);
}

bool flo::in_bounds(const point& p, const dimensions& dim) {
    return p.x >= 0.0 && p.x < static_cast<double>(dim.wd) &&
        p.y >= 0.0 && p.y < static_cast<double>(dim.hgt);
}


bool flo::in_bounds(const coords& p, const dimensions& dim) {
    return p.x >= 0 && p.x < dim.wd && p.y >= 0 && p.y < dim.hgt;
}

flo::dimensions flo::convex_hull_bounds(std::span<point> pts) {

    if (pts.empty()) return dimensions{ 0, 0 };

    double min_x = pts[0].x, max_x = pts[0].x;
    double min_y = pts[0].y, max_y = pts[0].y;

    for (const auto& p : pts) {
        min_x = std::min(min_x, p.x);
        max_x = std::max(max_x, p.x);
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }

    int width = static_cast<int>(std::ceil(max_x - min_x));
    int height = static_cast<int>(std::ceil(max_y - min_y));

    return dimensions{ width, height };

}

flo::coords flo::to_coords(const point& pt) {

    return { static_cast<int>(pt.x), static_cast<int>(pt.y) };

}

flo::point flo::to_point(const coords& cds)
{
    return { static_cast<double>(cds.x), static_cast<double>(cds.y) };
}

flo::rgb_color flo::hex_str_to_rgb(const std::string& hex) {

    std::string hex_code = hex;
    if (hex_code[0] == '#') {
        hex_code = hex_code.substr(1);
    }

    if (hex_code.length() != 6) {
        throw std::invalid_argument("Hex string must be in the format RRGGBB or #RRGGBB");
    }

    auto hex_to_uint8 = [](const std::string& str) -> uint8_t {
        unsigned int value;
        std::stringstream ss;
        ss << std::hex << str;
        ss >> value;
        return static_cast<uint8_t>(value);
        };

    std::string red_str = hex_code.substr(0, 2);
    std::string green_str = hex_code.substr(2, 2);
    std::string blue_str = hex_code.substr(4, 2);

    // Convert to lowercase for case insensitivity
    for (auto& c : red_str) c = std::tolower(c);
    for (auto& c : green_str) c = std::tolower(c);
    for (auto& c : blue_str) c = std::tolower(c);

    rgb_color color;
    color.red = hex_to_uint8(red_str);
    color.green = hex_to_uint8(green_str);
    color.blue = hex_to_uint8(blue_str);

    return color;

}

double flo::distance(const point& p1, const point& p2) {
    auto x_diff = p1.x - p2.x;
    auto y_diff = p1.y - p2.y;
    return std::sqrt(x_diff * x_diff + y_diff * y_diff);
}
