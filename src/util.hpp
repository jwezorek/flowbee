#pragma once

#include "types.hpp"
#include <string>
#include <span>

/*------------------------------------------------------------------------------------------------*/

namespace flo {

    uint32_t rgb_to_pixel(const rgb_color& rgb);
    rgb_color pixel_to_rgb(uint32_t pix);
    void img_to_file(const std::string& fname, const image& img);
    image img_from_file(const std::string& fname);
    scalar_field normalize(const scalar_field& s);
    image to_gray_scale_image(const scalar_field& sf, bool invert = false);
    scalar_field white_noise(int wd, int hgt);
    scalar_field perlin_noise(const flo::dimensions& sz, uint32_t seed, int octaves, double freq);
    rgb_color random_rgb_color();
    int rand_number(int min, int max);
    double normal_rand(double mean, double stddev);
    double uniform_rand(double low = 0.0, double high = 1.0);
    bool in_bounds(const point& p, const dimensions& dim);
    bool in_bounds(const coords& p, const dimensions& dim);
    dimensions convex_hull_bounds(std::span<point> pts);
    coords to_coords(const point& pt);
    point to_point(const coords& cds);
    rgb_color hex_str_to_rgb(const std::string& hex);

    template <typename T> int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }
}