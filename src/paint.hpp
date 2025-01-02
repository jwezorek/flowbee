#pragma once

#include <vector>
#include <cstdint>
#include "types.hpp"
#include "third-party/mixbox.h"

namespace flo {

    // Pigment Struct: Encapsulates Mixbox's latent color representation
    struct pigment {
        mixbox_latent impl;
    };

    // Paint Particle: Pigment with Volume
    struct paint_particle {
        pigment color;
        float volume;
    };

    pigment rgb_to_pigment(const rgb_color& rgb);
    pigment rgb_to_pigment(uint8_t r, uint8_t g, uint8_t b);

    // Convert Pigment to RGB
    void pigment_to_rgb_values(const pigment& p, uint8_t& r, uint8_t& g, uint8_t& b);
    rgb_color pigment_to_rgb(const pigment& p);

    // Mix a collection of paint particles into a single pigment
    pigment mix_paint_particles(const std::vector<paint_particle>& particles);

}