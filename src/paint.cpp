#include "paint.hpp"
#include <stdexcept>
#include <cstring> // For memset

flo::pigment flo::rgb_to_pigment(uint8_t r, uint8_t g, uint8_t b) {
    pigment p;
    mixbox_rgb_to_latent(r, g, b, p.impl);
    return p;
}

flo::pigment flo::rgb_to_pigment(const rgb_color& rgb)
{
    return rgb_to_pigment(rgb.red, rgb.green, rgb.blue);
}

void flo::pigment_to_rgb_values(const pigment& p, uint8_t& r, uint8_t& g, uint8_t& b) {
    mixbox_latent_to_rgb(const_cast<float*>(p.impl), &r, &g, &b);
}

flo::rgb_color flo::pigment_to_rgb(const pigment& p) {
    flo::rgb_color rgb;
    pigment_to_rgb_values(p, rgb.red, rgb.green, rgb.blue);
    return rgb;
}

flo::pigment flo::mix_paint_particles(const std::vector<paint_particle>& particles) {
    if (particles.empty()) {
        throw std::invalid_argument("Cannot mix an empty set of paint particles.");
    }

    pigment mixed_pigment;
    std::memset(mixed_pigment.impl, 0, sizeof(mixbox_latent)); // Initialize to zeros
    double total_volume = 0.0f;

    for (const auto& particle : particles) {
        for (int i = 0; i < MIXBOX_LATENT_SIZE; ++i) {
            mixed_pigment.impl[i] += particle.color.impl[i] * particle.volume;
        }
        total_volume += particle.volume;
    }

    // Normalize the mixed pigment by the total volume
    if (total_volume > 0.0f) {
        for (int i = 0; i < MIXBOX_LATENT_SIZE; ++i) {
            mixed_pigment.impl[i] /= total_volume;
        }
    }

    return mixed_pigment;
}