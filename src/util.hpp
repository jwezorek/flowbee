#pragma once

#include "types.hpp"
#include <string>

namespace flo {

    void write_to_file(const std::string& fname, const image& img);
    scalar_field normalize(const scalar_field& s);
    image to_gray_scale_image(const scalar_field& sf, bool invert = false);
    scalar_field white_noise(int wd, int hgt);

}