#pragma once

#include "types.hpp"
#include <string>

namespace flo {

    void write_to_file(const std::string& fname, const image& img);
    scalar_field normalize(const scalar_field& s);
    image to_gray_scale_image(const scalar_field& sf, bool invert = false);
    scalar_field white_noise(int wd, int hgt);
    scalar_field perlin_noise(const flo::dimensions& sz, uint32_t seed, int octaves, double freq);
    vector_field perlin_vector_field(const flo::dimensions& sz, uint32_t seed1, uint32_t seed2, int octaves, double freq);
}