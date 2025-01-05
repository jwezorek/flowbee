#pragma once

#include <vector>
#include <cstdint>
#include "types.hpp"
#include "third-party/mixbox.h"

namespace flo {

    struct pigment {
        mixbox_latent impl;
    };

    struct paint_particle {
        pigment color;
        double volume;
    };

    pigment rgb_to_pigment(const rgb_color& rgb);
    pigment rgb_to_pigment(uint8_t r, uint8_t g, uint8_t b);

    void pigment_to_rgb_values(const pigment& p, uint8_t& r, uint8_t& g, uint8_t& b);
    rgb_color pigment_to_rgb(const pigment& p);

    pigment mix_paint_particles(const std::vector<paint_particle>& particles);

}